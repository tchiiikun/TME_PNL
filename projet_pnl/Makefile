.PHONY: all clean dirs

ifneq ($(KERNELRELEASE),)

obj-m := pnlfs.o
pnlfs-y := src/pnlfs.o
ccflags-y := -DDEBUG -O1 -I $(PWD)/include/

else

KERNELDIR ?= ../../sources/linux-4.9.83
PWD := $(shell pwd)

all: dirs
	make -C $(KERNELDIR) M=$(PWD) modules

clean:
	make -C $(KERNELDIR) M=$(PWD) clean
	-rm -rf build
	-rm disk.img

build/mkfs-pnlfs.elf: src/mkfs-pnlfs.o dirs
	$(CC) $< -o $@ -I include/

disk.img: build/mkfs-pnlfs.elf
	dd if=/dev/zero of=disk.img bs=1M count=30
	./build/mkfs-pnlfs.elf disk.img

dirs:
	@if [ ! -d build ]; then mkdir build; fi

endif

# — Compilation séparée d’un module noyau (dans le Makefile) :
# obj-m += foo.o bar.o # création de 2 modules (foo.ko et bar.ko)
# foo-objs := toto.o fifi.o # dépendances de foo.ko (et donc toto.c et fifi.c)
# bar-objs := tutu.o fifi.o # dépendances de bar.ko
# ccflags-y := -DDEBUG -O1 # flags passés à gcc à la compilation
