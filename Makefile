KERNELDIR ?= /usr/src/linux-headers-$(shell uname -r)
PWD := $(shell pwd)
MODULE := fs_info_module

obj-m += $(MODULE).o

all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
