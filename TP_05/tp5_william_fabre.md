# TP 05 – Débogage du noyau Linux

## Exercice 1 : Connection du débogueur KGDB


### Question 1
OK visible dans nconfig.

### Question 2

#si on met rien :1234 c'est localhost:1234

KDB=1  si on veut lancer kdb grace a :

if [ -n "${KDB}" ]; then
KGD_WAIT='kgdbwait'
fi



### Question 3
ensuite il faut aller dans les sources et lancer gdb vmlinux une fois dans
gdb il faut lancer "target remote :1234"


### Question 5
puis lancer "continue" si ca bloque.
ca permet de finir de lancer le noyau.

### Question 6
envoyer g dans /proc/sysrq-trigger
GDB SI ON APPUIE SUR ENTRER FAIT LA COMMANDE PRECEDENTE

### Question 4
attention question 4 en derniere et attention "info thread" et pas info-thread

info thread : info threads -- Display currently known thread
monitor ps : Send a command to the remote monitor (remote targets only).


## Exercice 2 : Prise en main de KGDB

### Question 1
init_uts_ns:
* kref.h - library routines for handling generic reference counted objects
* .name : 
* define UTS_SYSNAME "Linux"
* define UTS_NODENAME CONFIG_DEFAULT_HOSTNAME /* set by sethostname() */
* define UTS_DOMAINNAME "(none)"	/* set by setdomainname() */
* struct user_namespace *user_ns;
* struct ucounts *ucounts;
* struct ns_common ns;

### Question 2
(gdb) print init_uts_ns
$1 = {kref = {refcount = {counter = 4}}, name = {sysname = "Linux", '\000' <repeats 59 times>, nodename = "pnl-tp", '\000' <repeats 58 times>, 
	release = "4.9.85", '\000' <repeats 58 times>, version = "#6 SMP Thu Mar 8 14:53:50 CET 2018", '\000' <repeats 30 times>, machine = "x86_64", '\000' <repeats 58 times>, 
	domainname = "(none)", '\000' <repeats 58 times>}, user_ns = 0xffffffff8225cc40 <init_user_ns>, ucounts = 0x0 <irq_stack_union>, ns = {stashed = {counter = 0}, 
		ops = 0xffffffff81c30be0 <utsns_operations>, inum = 4026531838}}

		(gdb) set variable init_uts_ns.name.sysname = "toto"
		(gdb) continue

		[root@pnl-tp ~]# uname
		toto


## Exercice 3 : Mon premier bug

### Question 1
la fonction set_current_state va mettre dans la task_struct le champ :
volatile long state;	/* -1 unrunnable, 0 runnable, >0 stopped */
We have two separate sets of flags: task->state is about runnability.
define TASK_UNINTERRUPTIBLE	2
Va prevenir les interruptions pour cette thread.


schedule_timeout - sleep until timeout
@timeout: timeout value in jiffies
In computing, a jiffy was originally the time between two ticks of the system timer interrupt.[4] It is not an absolute time interval unit, since its duration depends on the clock interrupt frequency of the particular hardware platform.

Make the current task sleep until @timeout jiffies have
elapsed. The routine will return immediately unless
the current task state has been set (see set_current_state()).

You can set the task state as follows -

%TASK_UNINTERRUPTIBLE - at least @timeout jiffies are guaranteed to
pass before the routine returns. The routine will return 0

%TASK_INTERRUPTIBLE - the routine may return early if a signal is
delivered to the current task. In this case the remaining time
in jiffies will be returned, or 0 if the timer expired in time

The current task state is guaranteed to be TASK_RUNNING when this
routine returns.

Specifying a @timeout value of %MAX_SCHEDULE_TIMEOUT will schedule
the CPU away without a bound on the timeout. In this case the return
value will be %MAX_SCHEDULE_TIMEOUT.

In all cases the return value is guaranteed to be non-negative.

signed long __sched schedule_timeout(signed long timeout)
{
	struct timer_list timer;
	unsigned long expire;

	switch (timeout)
	{
		case MAX_SCHEDULE_TIMEOUT:
			/*
			 * These two special cases are useful to be comfortable
			 * in the caller. Nothing more. We could take
			 * MAX_SCHEDULE_TIMEOUT from one of the negative value
			 * but I' d like to return a valid offset (>=0) to allow
			 * the caller to do everything it want with the retval.
			 */
			schedule();
			goto out;
		default:
			/*
			 * Another bit of PARANOID. Note that the retval will be
			 * 0 since no piece of kernel is supposed to do a check
			 * for a negative retval of schedule_timeout() (since it
			 * should never happens anyway). You just have the printk()
			 * that will tell you if something is gone wrong and where.
			 */
			if (timeout < 0) {
				printk(KERN_ERR "schedule_timeout: wrong timeout "
						"value %lx\n", timeout);
				dump_stack();
				current->state = TASK_RUNNING;
				goto out;
			}
	}

	expire = timeout + jiffies;

	setup_timer_on_stack(&timer, process_timeout, (unsigned long)current);
	__mod_timer(&timer, expire, false);
	schedule();
	del_singleshot_timer_sync(&timer);

	/* Remove the timer from the object tracker */
	destroy_timer_on_stack(&timer);

	timeout = expire - jiffies;

out:
	return timeout < 0 ? 0 : timeout;
}
EXPORT_SYMBOL(schedule_timeout);

### Question 2
Kernel hacking --> Lookups and Hang --> Detect Hung Task, on peut corriger le
probleme avec le default timeout for hungtask detection in seconds = 30
car on utilise 60*HZ

### Question 3
Oui le programme est fautif : INFO: task my_hanging_fn:329 blocked for more than 30 seconds.

### Question 4
Non ce n'est pas le code de notre module c'est  :

5  0xffffffff812384b2 in panic (fmt=0xffffffff81ece8ee "hung_task: blocked tasks") at kernel/panic.c:213
6  0xffffffff811efba8 in check_hung_task (timeout=<optimized out>, t=<optimized out>) at kernel/hung_task.c:126
7  check_hung_uninterruptible_tasks (timeout=<optimized out>) at kernel/hung_task.c:182
8  watchdog (dummy=0xffffffff82277130 <kgdb_panic_event_nb>) at kernel/hung_task.c:239
9  0xffffffff8115e7b9 in kthread (_create=0xffff88003d8f9440) at kernel/kthread.c:211
10 0xffffffff81b42c77 in ret_from_fork () at arch/x86/entry/entry_64.S:374

Ils ont detectes un probleme a cause de notre timer trop long.

### Question 5
Avec la commande "monitor ps " on observe :
Task Addr               Pid   Parent [*] cpu State Thread             Command
0xffff88003d8a3200       15        2  1    0   R  0xffff88003d8a3b80 *khungtaskd
0xffff88003dbba580      329        2  0    0   D  0xffff88003dbbaf00  my_hanging_fn

Avec la commande "monitor btp 329" on observe :

Stack traceback for pid 329
0xffff88003dbba580      329        2  0    0   D  0xffff88003dbbaf00  my_hanging_fn
0000000000000000 ffff88003cad43c0 ffff88003dbba580 ffffffff822134c0
ffff88003fc19600 ffffc90000e0bdf0 ffffffff81b3d811 ffffffff81176e85
0000000000000001 ffff88003fc19600 ffffc90000e0be30 00000000ffffe2ac
Call Trace:
[<ffffffff81b3d811>] ? __schedule+0x231/0x6e0
[<ffffffff81176e85>] ? put_prev_entity+0x35/0x940
[<ffffffff81b3dcec>] schedule+0x2c/0x80
[<ffffffff81b41412>] schedule_timeout+0x82/0x450
[<ffffffff811aa780>] ? del_timer_sync+0x50/0x50
[<ffffffffa0000000>] ? 0xffffffffa0000000
[<ffffffff81144dd4>] ? SyS_exit_group+0x14/0x20
[<ffffffffa0000038>] my_hanging_fn+0x38/0x50 [hanging]
[<ffffffff8115e7b9>] kthread+0xd9/0xf0
[<ffffffff8115e6e0>] ? kthread_park+0x60/0x60
[<ffffffff81003ac3>] ? do_syscall_64+0x73/0xe0
[<ffffffff81b42c77>] ret_from_fork+0x57/0x70



### Question 6
Module                  Size  modstruct     Used by
hanging                 1436  0xffffffffa0000180    0  (Live) 0xffffffffa0000000 [ ]
La premiere correspond a l'adresse de la structure du moducle, la seconde
l'adresse correspond a l'adresse de base du module car :
sa structure et a 180 byte plus loin, et la fonction:
[<ffffffffa0000038>] my_hanging_fn+0x38/0x50 [hanging]
est non loin non plus.

### Question 7
La premiere solution deja evoque avant : changer dans le module le nombre de
seconde que l'on attend (nombre de HZ) deuxieme solution augmenter la taille
du maximum que l'on attend dans le kernel. C'est un thread endormi et non
interruptible donc il doit bien mourrir un jour.



## Exercice 4 : Affichages dynamiques

### Question 1
CONFIG_DYNAMIC_DEBUG: dans printk et dmesg option dans le menuconfig, on peut
voir l'option : enable dynamic printk() support. De plus, il faut l'option
debug dans le makefile et on peut desactiver l'option d'affichage au runtime.
C'est la variable : dynamic_debug/control qui controle l'activation runtime
dans le debugfs donc il ne faut pas oublier de le monter avant


### Question 2
 filename:lineno [module]function flags format│  

  │ filename : source file of the debug statement│  
  │ lineno : line number of the debug statement│  
  │ module : module that contains the debug statement│  
  │ function : function that contains the debug statement│  
  │ flags : '=p' means the line is turned 'on' for printing│  
  │ format : the format used for the debug statement│  
  │ From a live system:│  
  │       nullarbor:~ # cat <debugfs>/dynamic_debug/control│  
  │       # filename:lineno [module]function flags format│  
  │       fs/aio.c:222 [aio]__put_ioctx =_ "__put_ioctx:\040freeing\040%p\012"│  
  │       fs/aio.c:248 [aio]ioctx_alloc =_ "ENOMEM:\040nr_events\040too\040high\012"│
  │       fs/aio.c:1770 [aio]sys_io_cancel =_ "calling\040cancel\012"│  
  │ Example usage:│  
  │               │  
  │       // enable the message at line 1603 of file svcsock.c│  
  │       nullarbor:~ # echo -n 'file svcsock.c line 1603 +p' >│  
  │                                       <debugfs>/dynamic_debug/control│  
  │       // enable all the messages in file svcsock.c│  
  │       nullarbor:~ # echo -n 'file svcsock.c +p' >│  
  │                                       <debugfs>/dynamic_debug/control│  
  │       // enable all the messages in the NFS server module│  
  │       nullarbor:~ # echo -n 'module nfsd +p' >│  
  │                                       <debugfs>/dynamic_debug/control│  
  │       // enable all 12 messages in the function svc_process()│  
  │       nullarbor:~ # echo -n 'func svc_process +p' >│  
  │                                       <debugfs>/dynamic_debug/control│  
  │       // disable all 12 messages in the function svc_process()│  
  │       nullarbor:~ # echo -n 'func svc_process -p' >│  
  │                                       <debugfs>/dynamic_debug/control

la commande :
echo -n 'module prdebug +p' > /sys/kernel/debug/dynamic_debug/control
fait, un echo qui ne va pas output le retour a la ligne, et va ecrire tous les
pr_debug car on ne precise pas de ligne avec l'option p qui enables the pr_debug()
callsite.

### Question 3
echo -n 'module prdebug +pflmt' > /sys/kernel/debug/dynamic_debug/control


[52670.180717] reactivating timer
[52671.220591] nr irqs 19
[52671.220608] reactivating timer
[52672.260641] nr irqs 15
[52672.260699] reactivating timer
[52673.300703] nr irqs 96
[52673.300730] reactivating timer
[52674.340736] nr irqs 5
[52674.340740] reactivating timer
[52675.380698] <intr> prdebug:prdebug_timeout:13: nr irqs 14
[52675.380727] <intr> prdebug:prdebug_timeout:16: reactivating timer
[52676.420589] <intr> prdebug:prdebug_timeout:13: nr irqs 25
[52676.420608] <intr> prdebug:prdebug_timeout:16: reactivating timer
[52677.460587] <intr> prdebug:prdebug_timeout:13: nr irqs 14
[52677.460605] <intr> prdebug:prdebug_timeout:16: reactivating timer
[52678.500592] <intr> prdebug:prdebug_timeout:13: nr irqs 40
[52678.500611] <intr> prdebug:prdebug_timeout:16: reactivating timer
[52679.540718] <intr> prdebug:prdebug_timeout:13: nr irqs 32
[52679.540746] <intr> prdebug:prdebug_timeout:16: reactivating timer
[52680.580693] <intr> prdebug:prdebug_timeout:13: nr irqs 0
[52680.580722] <intr> prdebug:prdebug_timeout:16: reactivating timer
[52681.621633] <intr> prdebug:prdebug_timeout:13: nr irqs 278
[52681.621636] <intr> prdebug:prdebug_timeout:16: reactivating timer
[52682.660533] <intr> prdebug:prdebug_timeout:13: nr irqs 7488
[52682.660535] <intr> prdebug:prdebug_timeout:16: reactivating timer

### Question 4

echo -n 'module prdebug line 13 +pflmt' > /sys/kernel/debug/dynamic_debug/control

[52814.433851] prdebug module unloaded
[52820.031476] prdebug module loaded
[52823.140687] <intr> prdebug:prdebug_timeout:13: nr irqs 9
[52824.180633] <intr> prdebug:prdebug_timeout:13: nr irqs 0
[52825.220687] <intr> prdebug:prdebug_timeout:13: nr irqs 25
[52826.260619] <intr> prdebug:prdebug_timeout:13: nr irqs 7871


## Exercice 5 : Débogage d’un module


### Question 1


### Question 2
Dans gdb je recois :

[New Thread 362]
[New Thread 320]
[New Thread 331]
[New Thread 355]

Thread 103 received signal SIGSEGV, Segmentation fault.
[Switching to Thread 362]
0xffffffffa00081d6 in ?? ()


Task Addr               Pid   Parent [*] cpu State Thread             Command
0xffff88003b6ce400      362        2  1    0   R  0xffff88003b6ced80 *my_kcpustat_fn


Stack traceback for pid 362
0xffff88003b6ce400      362        2  1    0   R  0xffff88003b6ced80 *my_kcpustat_fn
 ffffc90000e93e38 0000000000000018 0000000000000167 0000000000000008
 000000000000003a 0000000000000000 0000000000000167 0000000000000008
 0000000000000000 00000000000137c2 000000000000008a 0000000000000015
Call Trace:
 [<ffffffffa0008310>] my_kcpustat_fn+0x70/0x80 [kcpustat]
 [<ffffffff8115e7b9>] kthread+0xd9/0xf0
 [<ffffffff8115e6e0>] ? kthread_park+0x60/0x60
 [<ffffffff81b42c77>] ret_from_fork+0x57/0x70


\* 103  Thread 362 (my_kcpustat_fn) 0xffffffffa00081d6 in ?? ()



### Question 3

Thread 37 received signal SIGSEGV, Segmentation fault.
[Switching to Thread 326]
0xffffffffa00001e7 in ?? ()
(gdb) bt
0  0xffffffffa00001e7 in ?? ()
1  0x0000000000000010 in irq_stack_union ()
2  0x0000000000000000 in ?? ()

(gdb) bt
0  print_stats () at /home/tchi/Lecture/TME_PNL/TP_05/EXO-05/kcpustat.c:75
1  0xffffffffa0000320 in my_kcpustat_fn (data=0xdead000000000100) at /home/tchi/Lecture/TME_PNL/TP_05/EXO-05/kcpustat.c:102
2  0xffffffff8115e7b9 in kthread (_create=0xffff88003cb6b500) at kernel/kthread.c:211
3  0xffffffff81b42c77 in ret_from_fork () at arch/x86/entry/entry_64.S:374
4  0x0000000000000000 in ?? ()




### Question 4

Il faut utiliser list_for_each_entry_safe sinon on va supprimer le pointeur sur
lequel on itere et tout va planter. LFEES va nous permettre d'avoir un pointeur
n qui reinitialisera au prochain tour de boucle notre pointeur actuel et nous
permet de supprimer le pointeur sur la position actuel.

### Question 5

Le probleme vient du fait que l'on utilise list_del_init (c'est la liste qui
pose probleme) et list_del juste apres. list_del_init est censee virer
reinitialiser les pointeurs sur lui meme et faire un INIT_LIST_HEAD.

### Question 6

Debug Linked List Manipulation
Debug priority Linked List Manipulation
Probleme regle dans la question 5.

### Question 7

Activation de l'option kmemleak dans le noyau:
1) Activer kmemleak=on dans la commande de lancement du script qemu
2) Activer l'option kmemleak et recompiler le noyau (kernel hacking).
3) monter debugfs :
  # mount -t debugfs nodev /sys/kernel/debug/
  # cat /sys/kernel/debug/kmemleak
4) trigger un scan :
  # echo scan > /sys/kernel/debug/kmemleak
5) clear la list des memory leaks :
  # echo clear > /sys/kernel/debug/kmemleak
6) changer la taille du maximum early log sinon on explose le compteur,
 avec un dmesg | grep -i kmem on tombe sur 1700 leak en early log et le
 fichier est detruit.

Autres options:
- off
    disable kmemleak (irreversible)
- stack=on
    enable the task stacks scanning (default)
- stack=off
    disable the tasks stacks scanning
- scan=on
    start the automatic memory scanning thread (default)
- scan=off
    stop the automatic memory scanning thread
- scan=<secs>
    set the automatic memory scanning period in seconds
    (default 600, 0 to stop the automatic scanning)
- scan
    trigger a memory scan
- clear
    clear list of current memory leak suspects, done by
    marking all current reported unreferenced objects grey,
    or free all kmemleak objects if kmemleak has been disabled.
- dump=<addr>
    dump information about the object found at <addr>

visible dans le fichier dev-tools/kmemleak.rst
https://access.redhat.com/documentation/en-US/Red_Hat_Enterprise_MRG/2/html/Realtime_Tuning_Guide/Mount_debugfs.html
Auto mount de debugfs.

Sur la VM pour check si kmemleak est present :
zcat /proc/config.gz | grep KMEMLEAK

### Question 8

On doit faire un echo scan > /sys/kernel/debug/kmemleak pour declencher le scan
et apres on aura :
kmemleak: 20 new suspected memory leaks (see /sys/kernel/debug/kmemleak)
on a juste a faire un cat de /sys/kernel/debug/kmemleak


unreferenced object 0xffff88003c220ba0 (size 96):
  comm "my_kcpustat_fn", pid 307, jiffies 4294945432 (age 67.780s)
  hex dump (first 32 bytes):
    00 01 00 00 00 00 ad de 00 02 00 00 00 00 ad de  ................
    00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
  backtrace:
    [<ffffffff81b3a16a>] kmemleak_alloc+0x4a/0xa0
    [<ffffffff8129ce45>] kmem_cache_alloc_trace+0x165/0x1c0
    [<ffffffffa000002c>] save_stats+0x2c/0x140 [kcpustat]
    [<ffffffffa0000273>] my_kcpustat_fn+0x53/0x80 [kcpustat]
    [<ffffffff8115e7b9>] kthread+0xd9/0xf0
    [<ffffffff81b45537>] ret_from_fork+0x57/0x70
    [<ffffffffffffffff>] 0xffffffffffffffff

En fait on peut voir ce message pour chaque element de la liste car tous les
elements de la liste sont mal desalloues. listdel ne fait que faire pointer sur
lui meme un element et remet corrctement le pointeur du suivant et du precedent
pour ne pas faire un trou dans la liste.  il faut donc faire deux kfree, apres
chaque list del et tout fonctionne.
