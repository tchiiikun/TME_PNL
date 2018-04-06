.PHONY: all
#.SECONDARY:

ifneq ($(KERNELRELEASE),)

obj-m := kcpustat.o

else
	
  #../../../sources/linux-4.9.85/lib/modules

  KERNELDIR ?= ../../../sources/linux-4.9.85
  #KERNELDIR ?= /lib/modules/$(shell uname -r)/build
  PWD := $(shell pwd)

all :
	$(info obj-m : $(obj-m))
	make -C $(KERNELDIR) M=$(PWD) modules

clean:
	make -C $(KERNELDIR) M=$(PWD) clean

endif
