#! /bin/bash

# Fixer les variables avec les chemins de vos fichiers
HDA="-hda ../sources/pnl-tp.img"
HDB="-hdb ../sources/myHome.img"
KERNEL=../sources/linux-4.9.85/arch/x86/boot/bzImage
#KERNEL=sources/linux-4.9.83/arch/x86/boot/bzImage

CMDLINE='root=/dev/sda1 rw vga=792 console=ttyS0 kgdboc=ttyS1'

FLAGS="--enable-kvm "
VIRTFS+=" --virtfs local,path=.,mount_tag=share,security_model=passthrough,id=share "

exec qemu-system-x86_64 ${FLAGS} \
     ${HDA} ${HDB} \
     ${VIRTFS} \
     -net user -net nic \
     -serial stdio \
     -boot c -m 2G \
     -kernel "${KERNEL}" -append "${CMDLINE}"
