/*
 * lunix-chrdev.c
 *
 * Implementation of character devices
 * for Lunix:TNG
 *
 * < Your name here >
 *
 */

#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/cdev.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/mmzone.h>
#include <linux/vmalloc.h>
#include <linux/spinlock.h>

#include "lunix.h"
#include "lunix-chrdev.h"
#include "lunix-lookup.h"

#define MSR_MAX_LEN	7

/*
 * Global data
 */
struct cdev lunix_chrdev_cdev;

int lunix_chrdev_parse( enum lunix_msr_enum type, uint32_t before, unsigned char *ret, int *cnt )
{
	long temp;
	unsigned char *curr = ret;

	if ( type == 0 ) temp = lookup_voltage[before];
	else if ( type == 1 ) temp = lookup_temperature[before];
	else if ( type == 2 ) temp = lookup_light[before];
	else return -1;

	/* FIXME : possible length > 6 */
	if ( temp/10000 != 0 ) *curr++ = temp/10000 + 48;
	else *curr++ = ' ';
	*curr++ = (temp/1000)%10 + 48;
	*curr++ = '.';
	*curr++ = (temp%1000)/100 + 48;
	*curr++ = (temp%100)/10 + 48;
	*curr++ = (temp%10) + 48;
	*curr = '\n';

	(*cnt) = MSR_MAX_LEN;
	return 0;	
}

/*
 * Just a quick [unlocked] check to see if the cached
 * chrdev state needs to be updated from sensor measurements.
 */
static int lunix_chrdev_state_needs_refresh(struct lunix_chrdev_state_struct *state)
{
	struct lunix_sensor_struct *sensor;
	
	WARN_ON ( !(sensor = state->sensor));

//	debug("I am need_refresh. state_timestamp=%d, sensor_timestamp=%d\n", state->buf_timestamp, sensor->msr_data[state->type]->last_update );
	if ( state->buf_timestamp < sensor->msr_data[state->type]->last_update ) return 1;
	return 0;
}

/*
 * Updates the cached state of a character device
 * based on sensor data. Must be called with the
 * character device state lock held.
 */
static int lunix_chrdev_state_update(struct lunix_chrdev_state_struct *state)
{
	uint32_t temp;

	/*
	 * Grab the raw data quickly, hold the
	 * spinlock for as little as possible.
	 */
	/* Why use spinlocks? See LDD3, p. 119 */
	spin_lock_irq(&state->sensor->lock);

	/*
	 * Any new data available?
	 */
	if ( lunix_chrdev_state_needs_refresh(state) )
	{
		debug("New data!\n");
		//copy new data to temp
		temp = state->sensor->msr_data[state->type]->values[0];
	}
	else
	{
		debug("No new data!\n");
		spin_unlock_irq(&state->sensor->lock);
		//debug("Leaving update, return -EAGAIN\n");
		return -EAGAIN;
	}

	spin_unlock_irq(&state->sensor->lock);

	/*
	 * Now we can take our time to format them,
	 * holding only the private state semaphore
	 */
	if ( lunix_chrdev_parse( state->type, temp, &state->buf_data[0], &state->buf_lim ) )
		debug("Some kind of Error\n");

	state->buf_timestamp = get_seconds();

	return 0;
}

/*************************************
 * Implementation of file operations
 * for the Lunix character device
 *************************************/

static int lunix_chrdev_open(struct inode *inode, struct file *filp)
{
	unsigned int my_major;
	unsigned int my_minor;
	int ret;
	struct lunix_chrdev_state_struct *state;

	debug("I am alive\n");
	ret = -ENODEV;
	if ((ret = nonseekable_open(inode, filp)) < 0)
		/* used by subsystems that don't want seekable file 
		descriptors. The function is not supposed to ever fail */
		goto out;

	/*
	 * Associate this open file with the relevant sensor based on
	 * the minor number of the device node [/dev/sensor<NO>-<TYPE>]
	 */
	my_major = imajor(inode);
	my_minor = iminor(inode);
	debug("major=%d minor=%d sensor=%d measurement=%d\n", my_major, my_minor, my_minor / 8, my_minor % 8);

	/* Allocate a new Lunix character device private state structure */
	state = kmalloc(sizeof(struct lunix_chrdev_state_struct), GFP_KERNEL);

	state->type = my_minor%8;
	state->sensor = &lunix_sensors[my_minor/8];
	state->buf_timestamp = get_seconds();
	init_MUTEX(&state->lock);

	filp->private_data = state;

out:
	debug("Leaving, with ret = %d\n\n", ret);
	return ret;
}

static int lunix_chrdev_release(struct inode *inode, struct file *filp)
{
	//debug("release\n");
	kfree(filp->private_data);	
	debug("closed...\n\n");
	return 0;
}

static long lunix_chrdev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	return -EINVAL;
}

static ssize_t lunix_chrdev_read(struct file *filp, char __user *usrbuf, size_t cnt, loff_t *f_pos)
{
	ssize_t ret;
	struct lunix_sensor_struct *sensor;
	struct lunix_chrdev_state_struct *state;

	ret = 0;
	debug("I am alive!\n");

	state = filp->private_data;
	WARN_ON(!state);

	sensor = state->sensor;
	WARN_ON(!sensor);

	/* Lock? */
	// wait for the semaphore
	if ( down_interruptible(&state->lock) )
		return -ERESTARTSYS;

	if ( *f_pos < 0 ) {
		debug("userspace process called us with f_pos < 0. will return -EFAULT.\n");
		return -EFAULT;
	}

	/*
	 * If the cached character device state needs to be
	 * updated by actual sensor data (i.e. we need to report
	 * on a "fresh" measurement), do so
	 */
	if ( *f_pos == 0 ) {
		/* get fresh data, if any */
		while (lunix_chrdev_state_update(state) == -EAGAIN) {
			/* The process needs to sleep */
			/* See LDD3, page 153 for a hint */
			debug("The process needs to sleep!\n");
			up(&state->lock);	// release the lock
			if (filp->f_flags & O_NONBLOCK)
			{
				// this process doesn't want to block
				return -EAGAIN;
			}
			debug("\"%s\" reading: going to sleep\n", current->comm);
			// the following will be checked after waking up!
			if ( wait_event_interruptible( state->sensor->wq, (lunix_chrdev_state_needs_refresh(state) != 0) ) )
			{
				debug("\"%s\" woken up, BY SIGNAL\n", current->comm);
				return -ERESTARTSYS;	// signal: tell the fs layer to handle it
			}
			// otherwise loop, but first reacquire the lock
			debug("\"%s\" woken up, looping over again trying to update...\n", current->comm);
			if ( down_interruptible(&state->lock) )
				return -ERESTARTSYS;
		}
		// when we exit from the loop, we know that the semaphore is held and there are data we can use
		//debug("Update finished, my buf_data are %s and their length is %d\n", state->buf_data, state->buf_lim);
		debug("Update finished\n");
	}

	/* End of file */
	/* ? */
	
	/* Determine the number of cached bytes to copy to userspace */
	/* ? */
	ret = state->buf_lim - *f_pos;
	if ( cnt < ret ) {
		ret = cnt;
		*f_pos += cnt;
	}
	else {
		*f_pos = 0;
	}

	debug("been asked for %d bytes, I copy to user %d bytes\n", cnt, ret);
	if ( copy_to_user(usrbuf, &state->buf_data[filp->f_pos], ret) ) {
		up(&state->lock);
		return -EFAULT;
	}

	/* Auto-rewind on EOF mode? */
	/* ? */

	/* Unlock? */
	up(&state->lock);
	wake_up_interruptible(&state->sensor->wq);
//out:
	debug("\"%s\" did read %li bytes\n", current->comm, (long) ret);
	debug("Leaving with ret=%d\n", (int) ret);
	return ret;
}

static int lunix_chrdev_mmap(struct file *filp, struct vm_area_struct *vma)
{
	return -EINVAL;
}

static struct file_operations lunix_chrdev_fops = 
{
        .owner          = THIS_MODULE,
	.open           = lunix_chrdev_open,
	.release        = lunix_chrdev_release,
	.read           = lunix_chrdev_read,
	.unlocked_ioctl = lunix_chrdev_ioctl,
	.mmap           = lunix_chrdev_mmap
};

/*
 * Register the character device with the kernel, asking for
 * a range of minor numbers ( number of sensors * (8 measurements/sensor) )
 * beginning with LINUX_CHRDEV_MAJOR:0
 */
int lunix_chrdev_init(void)
{
	int ret;
	dev_t dev_no;
	unsigned int lunix_minor_cnt = lunix_sensor_cnt << 3;	// this is lunix_sensor_cnt * 8
	
	debug("Initializing character device\n");

	/*
	 * cdev_init() - initialize a cdev structure
	 * @cdev: the structure to initialize
	 * @fops: the file_operations for this device
	 *
	 * Initializes @cdev, remembering @fops, making it ready to add to the
	 * system with cdev_add().
	 *
	 * http://lxr.linux.no/linux+v2.6.37.4/fs/char_dev.c#L554
	 */
	cdev_init(&lunix_chrdev_cdev, &lunix_chrdev_fops);
	lunix_chrdev_cdev.owner = THIS_MODULE;
	
	/*
	 * #define MKDEV(ma,mi)		(((ma) << MINORBITS) | (mi))
	 *
	 * http://lxr.linux.no/linux+v2.6.37.4/include/linux/kdev_t.h#L9
	 */
	dev_no = MKDEV(LUNIX_CHRDEV_MAJOR, 0);

	/*
	 * register_chrdev_region() - register a range of device numbers
	 * @from: the first in the desired range of device numbers; must include
	 *        the major number.
	 * @count: the number of consecutive device numbers required
	 * @name: the name of the device or driver.
	 *
	 * Return value is zero on success, a negative error code on failure.
	 *
	 * http://lxr.linux.no/linux+v2.6.37.4/fs/char_dev.c#L196
	 */
	ret = register_chrdev_region(dev_no, lunix_minor_cnt, "lunix");
	if (ret < 0) {
		debug("failed to register region, ret = %d\n", ret);
		goto out;
	}	

	/*
	 * cdev_add() - add a char device to the system
	 * @p: the cdev structure for the device
	 * @dev: the first device number for which this device is responsible
	 * @count: the number of consecutive minor numbers corresponding to this
	 *         device
	 *
	 * cdev_add() adds the device represented by @p to the system, making it
	 * live immediately.  A negative error code is returned on failure.
	 *
	 * http://lxr.linux.no/linux+v2.6.37.4/fs/char_dev.c#L484
	 */
	ret = cdev_add(&lunix_chrdev_cdev, dev_no, lunix_minor_cnt);
	if (ret < 0) {
		debug("failed to add character device\n");
		goto out_with_chrdev_region;
	}

	debug("Character device added to kernel and is alive...\n");
	return 0;

out_with_chrdev_region:
	/**
	 * unregister_chrdev_region() - return a range of device numbers
	 * @from: the first in the range of numbers to unregister
	 * @count: the number of device numbers to unregister
	 *
	 * This function will unregister a range of @count device numbers,
	 * starting with @from.  The caller should normally be the one who
	 * allocated those numbers in the first place...
	 *
	 * http://lxr.linux.no/linux+v2.6.37.4/fs/char_dev.c#L307
	 */
	unregister_chrdev_region(dev_no, lunix_minor_cnt);
out:
	return ret;
}

void lunix_chrdev_destroy(void)
{
	dev_t dev_no;
	unsigned int lunix_minor_cnt = lunix_sensor_cnt << 3;
		
	debug("destroying character device\n");
	dev_no = MKDEV(LUNIX_CHRDEV_MAJOR, 0);
	/*
	 * cdev_del() - remove a cdev from the system
	 * @p: the cdev structure to be removed
	 *
	 * cdev_del() removes @p from the system, possibly freeing the structure
	 * itself.
	 *
	 * http://lxr.linux.no/linux+v2.6.37.4/fs/char_dev.c#L503
	 */
	cdev_del(&lunix_chrdev_cdev);
	unregister_chrdev_region(dev_no, lunix_minor_cnt);
	debug("character device destroyed\n");
}
