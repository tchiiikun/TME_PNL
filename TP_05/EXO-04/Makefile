.PHONY: all 
#.SECONDARY:

ifneq ($(KERNELRELEASE),)

obj-m := prdebug.o
CFLAGS_pr_debuf.o := -DDEBUG

else
	
  #KERNELDIR ?= /lib/modules/$(shell uname -r)/build
  KERNELDIR ?= ../../../sources/linux-4.9.85
  PWD := $(shell pwd)

all :
	$(info obj-m : $(obj-m))
	make -C $(KERNELDIR) M=$(PWD) modules

clean:
	make -C $(KERNELDIR) M=$(PWD) clean

endif
