# TP 06 – Mécanismes de communication du noyau Linux


## Exercice 1 Système de fichiers virtuels sysfs

### Question 1
http://www.embetronicx.com/tutorials/linux/device-drivers/sysfs-in-linux-kernel/

Differents moyens de communication entre Userland et Kernelland :
    * IOCTL : communication avec des pilotes de peripheriques
    * Procfs : Information sur les Processus
    * Sysfs : intrinsequement lié au PILOTE de PERIPHERIQUE
    * Configfs
    * Debugfs : Information de debug
    * Sysctl
    * UDP Sockets
    * Netlink Sockets

Les Kobjects sont ce qui lie le kernel a sysfs. trouvable dans <linux/kobject.h>
une struct Kobject va representer un device (=peripherique) ils sont
generalement une partie d'une autre structure.


struct kobject {
	const char		*name; //nom du kobject qui sera cree dans sysfs
	struct list_head	entry; 
	struct kobject		*parent; //nom du directory parent.
	struct kset		*kset;// groupement de kobject dans un une struct
	struct kobj_type	*ktype; //type du kobject courant.

	struct kernfs_node	*sd; /* sysfs directory entry, points to a sysfs_dirent structure that represents this kobject in sysfs */

	struct kref		kref; //compteur de references
	#ifdef CONFIG_DEBUG_KOBJECT_RELEASE
		struct delayed_work	release;
	#endif
	unsigned int state_initialized:1;
	unsigned int state_in_sysfs:1;
	unsigned int state_add_uevent_sent:1;
	unsigned int state_remove_uevent_sent:1;
	unsigned int uevent_suppress:1;
};

Pour creer un repertoire dans sysfs de maniere dynamique et retourne NULL sinon:
struct kobject * kobject_create_and_add ( const char * name, struct kobject * parent);

Pour free cette structure il faut faire :
kobject_put(kobj_ref);

Maintenant on peut creer un fichier dans le sysfs, il contiendra une valeur par
fichier et doit utiliser sysfs attributes pour fonctionner.

struct kobj_attribute {
    struct attribute attr; // attribue aui correspond au file a creer.
    ssize_t (*show)(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
    ssize_t (*store)(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count);
};


show : pointeur sur la fonction qui sera appele lorsqu'on fait un read dans sysfs
RENVOIE LE NOMBRE DE CARAC ECRIT

store : pointeur qui sera appele lorsqu'on fait un write dans sysfs
RENVOIE LE NOMBRE DE CARAC LU

il faut utiliser la macro:
__ATTR(name, permission, show_ptr, store_ptr);
pour creer des attributs

__ATTR_RO permet de créer un attribut qui ne sera accessible qu’en lecture seule ;
__ATTR_WO permet de créer un attribut qui ne sera accessible qu’en écriture ;
__ATTR_RW permet de créer un attribut qui sera accessible en lecture et en écriture.

Une fois l’attribut défini, sa création passe par l’utilisation de la fonction sysfs_create_file :
int sysfs_create_file ( struct kobject *  kobj, const struct attribute * attr);

La suppression du fichier passe par la fonction sysfs_remove_file. Cette opération est importante et doit impérativement être effectuée lors du déchargement de votre module.

### Question 2
Voir le code mais en gros pour le strore il ne faut pas oublier la taille sinon
si on renvoie 0 on va boucler et du coup faire : 
	return strlen(strncpy(my_string, buf, 255));

## Exercice 2 Initiation aux ioctl
voir : http://embetronicx.com/tutorials/linux/device-drivers/ioctl-tutorial-in-linux/
### Question 1
FAIT voir code
### Question 2
FAIT voir code
### Question 3
FAIT voir code, je n'ai pas detaille l'exercice 2 car il est entierement code
dans le tutoriel et je n'ai fait que recopier le tutoriel et adapte au driver
de type character + pas besoin de mknod on le fait dans le code C.
Particularite on passe pas un pointeur sur string mais on passe juste la string
du coup c'est trompeur.


## Exercice 3 Manipulation des processus dans le noyau Linux
### Question 1
la struct pid est une reference sur un processus, on ne prend pas un struct
task de la user space mais directement un struct pid. On met ca dans une table
de hash pou y acceder plus rapidement.
### Question 2
task_struct :
Each task_struct data structure describes a process or task in the system.

task_struct->utime
task_struct->stime 
sont des extern cputime_t cputime_one_jiffy;
d'apres linus torvald :
u64 utime;
u64 stime;

/**
 * struct task_cputime - collected CPU time counts
 * @utime:		time spent in user mode, in nanoseconds
 * @stime:		time spent in kernel mode, in nanoseconds
 * @sum_exec_runtime:	total time spent on the CPU, in nanoseconds
 *
 * This structure groups together three kinds of CPU time that are tracked for
 * threads and thread groups.  Most things considering CPU time want to group
 * these counts together and treat all three of them in parallel.
 */


### Question 3
dans la task struct on a :
/* PID/PID hash table linkage. */
struct pid_link pids[PIDTYPE_MAX];

qui correspond a :
struct pid_link
{
	struct hlist_node node;
	struct pid *pid;
};


dans la struct pid on a :
struct hlist_head tasks[PIDTYPE_MAX];


pour s'interfacer on a ces fonctions:

static inline struct pid *task_pid(struct task_struct *task)
{
	return task->pids[PIDTYPE_PID].pid;
}

static inline struct pid *task_tgid(struct task_struct *task)
{
	return task->group_leader->pids[PIDTYPE_PID].pid;
}

/*
 * Without tasklist or rcu lock it is not safe to dereference
 * the result of task_pgrp/task_session even if task == current,
 * we can race with another thread doing sys_setsid/sys_setpgid.
 */
static inline struct pid *task_pgrp(struct task_struct *task)
{
	return task->group_leader->pids[PIDTYPE_PGID].pid;
}

static inline struct pid *task_session(struct task_struct *task)
{
	return task->group_leader->pids[PIDTYPE_SID].pid;
}


/*
 * the helpers to get the task's different pids as they are seen
 * from various namespaces
 *
 * task_xid_nr()     : global id, i.e. the id seen from the init namespace;
 * task_xid_vnr()    : virtual id, i.e. the id seen from the pid namespace of
 *                     current.
 * task_xid_nr_ns()  : id seen from the ns specified;
 *
 * set_task_vxid()   : assigns a virtual id to a task;
 *
 * see also pid_nr() etc in include/linux/pid.h
 */


## Exercice 4 Manipulation des processus dans le noyau Linux
Petit probleme dans le Makefile, une tabulation en trop
### Question 1
/*
 * Lookup a PID in the hash table, and return with it's count elevated.
 */
extern struct pid *find_get_pid(int nr);
extern void put_pid(struct pid *pid);
extern struct task_struct *pid_task(struct pid *pid, enum pid_type);
extern struct task_struct *get_pid_task(struct pid *pid, enum
pid_type);

### Question 2
On doit rendre la reference dans le init si on rate et que la cration de thread
ne fonctionne pas. Et on doit rendre dans le exit. Sans oublier de kfree.

### Question 3
static inline void task_lock(struct task_struct *p)
{
	spin_lock(&p->alloc_lock);
}
static inline void put_task_struct(struct task_struct *t)
{
	if (atomic_dec_and_test(&t->usage))
		__put_task_struct(t);
}

Resultat :
[13209.609399] 1 usr 11 sys 151

## Exercice 5 Monitoring avec le sysfs
### Question 1
fait deja.
### Question 2
utilisation de strncmp sinon ca fait des choses bizarres.


## Exercice 6 Module de monitoring avec ioctl
### Question 1
Copier coller de l'exo 2.
### Question 2
Opening Driver
[ 1417.927358] Device File Opened...!!!
Reading value from string in Driver
usr 5 sys 146
Reading Value Hello from struct in Driver
usr 5 sys 146
Closing Driver
[ 1417.930884] Device File Closed...!!!

### Question 3

Opening Driver
[ 2807.539921] Device File Opened...!!!
Reading value from string in Driver
usr 5 sys 146
Reading Value from struct in Driver
usr 5 sys 146
[ 2807.792478] 1 usr 5 sys 146
[ 2808.832459] 1 usr 5 sys 146
[ 2809.872471] 1 usr 5 sys 146
[ 2810.912478] 1 usr 5 sys 146
[ 2811.952488] 1 usr 5 sys 146
Reading Value from struct in Driver to trigger stop thread
[ 2812.544443] thread StoppedReading Value from struct in Driver to trigger start thread
[ 2817.546193] thread Started
Closing Driver
[ 2817.547069] Device File Closed...!!!
[root@pnl-tp EXO-06]# [ 2818.592468] 1 usr 5 sys 146
[ 2819.632479] 1 usr 5 sys 146
[ 2822.752475] 1 usr 5 sys 146
[ 2823.792467] 1 usr 5 sys 146

### Question 4
*********************************
*******marcalain*******

Opening Driver
[  230.943133] Device File Opened...!!!
Reading value from string in Driver
usr 8 sys 172
Reading Value from struct in Driver
usr 8 sys 172
[  231.898615] 1 usr 8 sys 172
[  232.938584] 1 usr 8 sys 172
[  233.978584] 1 usr 8 sys 172
[  235.018586] 1 usr 8 sys 172
Reading Value from struct in Driver to trigger stop thread
[  235.948288] thread StoppedReading Value from struct in Driver to trigger start thread
[  240.950192] thread Started
Enter the Value to send
[  241.978568] 1 usr 8 sys 172
2[  243.018569] 1 usr 8 sys 172

Writing Value to Driver
Closing Driver
[  243.347067] Device File Closed...!!!
[root@pnl-tp EXO-06]# [  244.378585] 2 usr 0 sys 2
[  245.418618] 2 usr 0 sys 2
[  246.458588] 2 usr 0 sys 2
[  247.498577] 2 usr 0 sys 2
[  248.538583] 2 usr 0 sys 2
[  249.578587] 2 usr 0 sys 2
[  250.618585] 2 usr 0 sys 2
[  251.658582] 2 usr 0 sys 2
[  252.698576] 2 usr 0 sys 2
[  253.738571] 2 usr 0 sys 2



