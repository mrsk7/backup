#!/bin/bash
cp -t /home/user/qemu-1.2.0/hw cryptodev.h pci.h virtio-crypto.h virtio-crypto.c virtio-pci.c -u -v
cd /home/user/qemu-1.2.0/
./configure --prefix=/home/user/qemu --enable-kvm --target-list=x86_64-softmmu
make
make install
cd /home/user/Desktop/ex2/virtualization/qemu/hw
