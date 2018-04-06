#! /bin/bash
#gdb > target remote :1234
#si on met rien :1234 c'est localhost:1234

# Fixer les variables avec les chemins de vos fichiers
HDA="-hda ../sources/pnl-tp.img"
HDB="-hdb ../sources/myHome.img"
KERNEL=../sources/linux-4.9.85/arch/x86/boot/bzImage

if [ -n "${KDB}" ]; then
    KGD_WAIT='kgdbwait'
fi


#CMDLINE="kmemleak=on KDB=1 root=/dev/sda1 rw vga=792 console=ttyS0 kgdboc=ttyS1 ${KGD_WAIT}"
CMDLINE="root=/dev/sda1 rw vga=792 console=ttyS0 kgdboc=ttyS1 ${KGD_WAIT}"

FLAGS="--enable-kvm "
VIRTFS+=" --virtfs local,path=.,mount_tag=share,security_model=passthrough,id=share "

exec qemu-system-x86_64 ${FLAGS} \
     ${HDA} ${HDB} \
     ${VIRTFS} \
     -net user -net nic \
     -serial stdio -serial tcp::1234,server,nowait \
     -boot c -m 1G \
     -kernel "${KERNEL}" -append "${CMDLINE}"
