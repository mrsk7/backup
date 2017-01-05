/*
 * crypto-ioctl.c
 *
 * Implementation of ioctl for 
 * virtio-crypto (guest side) 
 *
 * Stefanos Gerangelos <sgerag@cslab.ece.ntua.gr>
 *                                                                               
 */

#include <linux/cdev.h>
#include <linux/completion.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/freezer.h>
#include <linux/list.h>
#include <linux/poll.h>
#include <linux/sched.h>
#include <linux/spinlock.h>
#include <linux/virtio.h>
#include <linux/wait.h>
#include <linux/workqueue.h>
#include <linux/module.h>
#include <linux/ioctl.h>

#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include "crypto.h"
#include "crypto-vq.h"
#include "crypto-chrdev.h"

long crypto_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct crypto_device *crdev;
	struct crypto_data *cr_data;
	ssize_t ret;
	int i=0;
	__u8 * ptr_to_dst;
	__u8 * ptr_to_iv;
	crdev = filp->private_data;

	cr_data = kzalloc(sizeof(crypto_data), GFP_KERNEL);
	if (!cr_data)
		return -ENOMEM;

	cr_data->cmd = cmd;

	switch (cmd) {

        /* get the metadata for every operation 
	 * from userspace and send the buffer 
         * to the host */

	case CIOCGSESSION:
		/* ? */
		printk(KERN_INFO "IN CIOGSESSION\n");
		ret=copy_from_user(&cr_data->op.sess,(void __user *)arg,sizeof(struct session_op));
		send_buf(crdev,cr_data,sizeof(struct crypto_data),false);

		if (!device_has_data(crdev)) {
			debug("sleeping in CIOCGSESSION\n");
			if (filp->f_flags & O_NONBLOCK)
				return -EAGAIN;

			/* Go to sleep unti we have data. */
			ret = wait_event_interruptible(crdev->i_wq,
			                               device_has_data(crdev));

			if (ret < 0) 
				goto free_buf;
		}
		printk(KERN_ALERT "woke up in CIOCGSESSION\n");

		if (!device_has_data(crdev))
			return -ENOTTY;

		ret = fill_readbuf(crdev, (char *)cr_data, sizeof(crypto_data));

		/* copy the response to userspace */
		/* ? */
		ret = copy_to_user((void __user *)arg, &cr_data->op.sess, 
                                   sizeof(struct session_op));
		if (ret)
			goto free_buf;
		break;

	case CIOCCRYPT: 
		/* ? */
		printk(KERN_INFO "IN CIOCCRYPT\n");
		ret=copy_from_user(&cr_data->op.crypt,(void __user *)arg,sizeof(struct crypt_op));
		ptr_to_dst = cr_data->op.crypt.dst;
		ptr_to_iv = cr_data->op.crypt.iv;
		printk(KERN_INFO "\n1\n");
		printk(KERN_INFO "LEN %u\n",cr_data->op.crypt.len);
		memcpy(cr_data->srcp,cr_data->op.crypt.src,cr_data->op.crypt.len);
		printk(KERN_INFO "\n3\n");
		memcpy(cr_data->dstp,cr_data->op.crypt.dst,cr_data->op.crypt.len);
		printk(KERN_INFO "\nDSTP\n");
		printk(KERN_INFO "\n4\n");
		for(i=0;i<sizeof(cr_data->ivp);i++) cr_data->ivp[i]=cr_data->op.crypt.iv[i];
		printk(KERN_INFO "\n5\n");
                send_buf(crdev,cr_data,sizeof(struct crypto_data),false);

		if (!device_has_data(crdev)) {
			printk(KERN_WARNING "sleeping in CIOCCRYPTO\n");	
			if (filp->f_flags & O_NONBLOCK)
			return -EAGAIN;
			
			/* Go to sleep until we have data. */
			ret = wait_event_interruptible(crdev->i_wq,
			device_has_data(crdev));
			
			if (ret < 0)
			goto free_buf;
		}

		ret = fill_readbuf(crdev, (char *)cr_data, sizeof(crypto_data));
		printk(KERN_INFO "before\n");
		memcpy(ptr_to_dst,cr_data->dstp,cr_data->op.crypt.len);
		printk(KERN_INFO "after\n");
		cr_data->op.crypt.iv=ptr_to_iv;
		printk(KERN_INFO "after\n");
		ret = copy_to_user((void __user *)arg, &cr_data->op.crypt, 
		                   sizeof(struct crypt_op));
		if (ret)
			goto free_buf;

		/* copy the response to userspace */
		/* ? */

		break;

	case CIOCFSESSION:

		/* ? */
		ret=copy_from_user(&cr_data->op.sess,(void __user *)arg,sizeof(cr_data->op.sess));
		printk(KERN_INFO "IN CIOCFSESSION\n");
                send_buf(crdev,cr_data,sizeof(struct crypto_data),false);

		if (!device_has_data(crdev)) {
			printk(KERN_WARNING "PORT HAS NOT DATA!!!\n");
			if (filp->f_flags & O_NONBLOCK)
				return -EAGAIN;

			/* Go to sleep until we have data. */
			ret = wait_event_interruptible(crdev->i_wq,
			                               device_has_data(crdev));

			if (ret < 0)
				goto free_buf;
		}

		ret = fill_readbuf(crdev, (char *)cr_data, sizeof(crypto_data));

		/* copy the response to userspace */
		/* ? */
		ret = copy_to_user((void __user *)arg, &cr_data->op.sess, 
                                   sizeof(struct session_op));
		if (ret)
			goto free_buf;	
		break;

	default:
		return -EINVAL;
	}
	//return 0;

free_buf:
	printk(KERN_INFO "FREE BUF\n");
	kfree(cr_data);
	return ret;	
}
