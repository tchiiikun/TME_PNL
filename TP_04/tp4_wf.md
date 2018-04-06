# TP 04 – Mes premiers modules

## Exercice 1 : Mes premiers modules


### Question 1
Il faudra insmod helloWorld.ko une fois make.

### Question 2
foo := foo
bar = bar

undefine foo
undefine bar

$(info $(origin foo))
$(info $(flavor bar))

This example will print “undefined” for both variables.
make -C /lib/modules/4.15.5-1-ARCH/build M=/home/tchi/Lecture/TME_PNL/TP_04/ex1 modules

### Question 3
- dmesg affiche le kernel ring buffer.
- lsmod pour voir les modules charges
- /proc/modules
- journalctl may be used to query the contents of the systemd(1) journal as written by systemd-journald.service

### Question 4

Declaration avec renommage
static char *whom = "Tchi";
module_param_named(my_little_whom, whom, charp, 0444);
MODULE_PARM_DESC(my_little_whom, " Who are you?");


Declaration sans renommage
static int howmany;
module_param(howmany, int, 0444);
MODULE_PARM_DESC(howmany, " How many do you want?");

### Question 5

filename:       /root/share/TP_04/ex1/HelloWorldParam.ko
license:        GPL
author:         William Fabre, LIP6
description:    Module "hello word" pour noyau linux
depends:
retpoline:      Y
vermagic:       4.9.85 SMP mod_unload
parm:           my_little_whom: Who are you? (charp)
parm:           howmany: How many do you want? (int)

### Question 6

[root@pnl-tp ex1]# insmod HelloWorldParam.ko
[  299.288085] Hello, world
[  299.288576]  You are : Tchi
[  299.289062]  You have : 0
[root@pnl-tp ex1]# echo 42 > /sys/module/HelloWorldParam/parameters/howmany 
[root@pnl-tp ex1]# rmmod HelloWorldParam
[  304.191155] Goodbye, cruel world
[  304.192535]  You are : 42
[  304.193592]  You have : Tchi

## Exercice 2 : Modification d’une variable du noyau à l’aide d’un module


### Question 1
Oui car on fait un export lors de son initialisation.
EXPORT_SYMBOL_GPL(init_uts_ns);

### Question 2
https://elixir.bootlin.com/linux/v4.9.85/source/init/version.c#L43
https://elixir.bootlin.com/linux/v4.9.85/source/include/uapi/linux/utsname.h#L24
https://elixir.bootlin.com/linux/v4.9.85/source/include/uapi/linux/utsname.h#L24
https://elixir.bootlin.com/linux/v4.9.85/source/include/linux/string.h#L138



http://ytliu.info/notes/linux/file_ops_in_kernel.html
https://stackoverflow.com/questions/1184274/how-to-read-write-files-within-a-linux-kernel-module

/proc/sys/kernel/ostype 

[root@pnl-tp kernel]# echo 4242 > /proc/sys/kernel/ostype              
-bash: /proc/sys/kernel/ostype: Permission denied

[root@pnl-tp ex1]# insmod uname_module.ko
[   22.237322] uname_module: loading out-of-tree module taints kernel.
[   22.238935] Changing the output of uname
[root@pnl-tp ex1]# uname
[   27.240538] random: crng init done
m_arch-alain
[root@pnl-tp ex1]# rmmod uname_module
[   35.059419] Changing the output of uname finished
[root@pnl-tp ex1]# uname
Linux
[root@pnl-tp ex1]#

### Question 3
Car si on ne le recharge pas on va le perdre a tout jamais.


## Exercice 3 : Les limites des modules 

### Question 1
OK Voir code.

### Question 2
DANS UPDATE_sb2.KO

Au final la solution etait de faire une timespec dans la structure directement.
ligne 1456 du noyau dans include/linux/fs.h
	struct timespec date;

voici le resultat :

[  421.237068] End Polymorphic SuperBloc
[root@pnl-tp ex1]# insmod update_sb2.ko my_type=proc
[  423.261225] Polymorphic SuperBloc
[  423.263162] uuid= 00000000 0000 0000 0000 0000 00000000 temps=1520517382.181856446 type=proc
[root@pnl-tp ex1]# rmmod update_sb2.ko
[  431.643824] End Polymorphic SuperBloc
[root@pnl-tp ex1]# insmod update_sb2.ko my_type=ext4
[  433.705475] Polymorphic SuperBloc
[  433.706264] uuid= 9af987b4 1a11 45f6 992b 0f71 6a3451dd temps=1520517735.503223821 type=ext4
[  433.708088] uuid= e2788fd7 6503 4665 9498 d6e1 d23ab937 temps=1520517735.505048943 type=ext4
[root@pnl-tp ex1]# rmmod update_sb2.ko
[  441.116162] End Polymorphic SuperBloc
[root@pnl-tp ex1]# insmod update_sb2.ko my_type=proc
[  443.985833] Polymorphic SuperBloc
[  443.986701] uuid= 00000000 0000 0000 0000 0000 00000000 temps=1520517382.181856446 type=proc
[root@pnl-tp ex1]# insmod update_sb2.ko my_type=ext4
[root@pnl-tp ex1]# rmmod update_sb2.ko
[  449.131950] End Polymorphic SuperBloc
[root@pnl-tp ex1]# insmod update_sb2.ko my_type=ext4
[  451.881479] Polymorphic SuperBloc
[  451.882073] uuid= 9af987b4 1a11 45f6 992b 0f71 6a3451dd temps=1520517735.503223821 type=ext4
[  451.883226] uuid= e2788fd7 6503 4665 9498 d6e1 d23ab937 temps=1520517735.505048943 type=ext4


## Exercice bonus 4 (rootkit 1) : Cacher un module

### Question 1
Oui il est bien chargé

### Question 2
il faut utiliser :
prev_module = THIS_MODULE->list.prev;
list_del(&THIS_MODULE->list);

[root@pnl-tp ex1]# insmod helloWorld.ko
[   62.890552] helloWorld: loading out-of-tree module taints kernel.
[   62.891834] Hello, world
[   62.892331]  You are : (null)[   62.892859]  You are : 0
[root@pnl-tp ex1]# rmmod helloWorld 
rmmod: ERROR: could not remove 'helloWorld': No such file or directory
rmmod: ERROR: could not remove module helloWorld: No such file or directory

### Question 3
[root@pnl-tp module]# ls helloWorld
coresize  initsize   notes	 refcnt    taint
holders   initstate  parameters  sections  uevent

### Question 4
https://www.win.tue.nl/~aeb/linux/lk/lk-13.html
https://github.com/bones-codes/the_colonel/blob/master/lkm/rootkit.ch

[root@pnl-tp ~]# insmod share/TP_04/ex1/helloWorld.ko 
[   31.499202] helloWorld: loading out-of-tree module taints kernel.
[   31.500801] Hello, world
[   31.501388]  You are : (null)[   31.502011]  You are : 0
[root@pnl-tp ~]# lsmod
Module                  Size  Used by
[root@pnl-tp ~]# ls /sys/module/
[root@pnl-tp ~]# ls /sys/module/helloWorld
ls: cannot access '/sys/module/helloWorld': No such file or directory
