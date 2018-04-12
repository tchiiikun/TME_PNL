.PHONY: all 
#.SECONDARY:

ifneq ($(KERNELRELEASE),)

  obj-m := HelloWorldParam.o

else
	
  # celui du kernel emule
  #KERNELDIR ?= /lib/modules/$(shell uname -r)/build
  KERNELDIR ?= ../../../sources/linux-4.9.85
  PWD := $(shell pwd)

all :
	make -C $(KERNELDIR) M=$(PWD) modules

clean:
	make -C $(KERNELDIR) M=$(PWD) clean

endif

