.PHONY: all

ifneq ($(KERNELRELEASE),)

obj-m += helloioctl.o

else

KERNELDIR ?= ../../../sources/linux-4.9.85

all:
	make -C $(KERNELDIR) M=$(PWD) modules

clean:
	make -C $(KERNELDIR) M=$(PWD) clean

endif
