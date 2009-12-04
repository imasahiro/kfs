ifneq ($(KERNELRELEASE),)
	obj-m := kfs.o
	kfs-objs := super.o file.o inode.o
else
KVER = $(shell uname -r)
KDIR = /lib/modules/$(KVER)/build
modules:
	$(MAKE) -C $(KDIR) M=$(shell pwd) modules

clean:
	$(MAKE) -C $(KDIR) M=$(shell pwd) clean
endif
