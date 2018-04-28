### TP 08 – Mécanismes de gestion de la mémoire dans le noyau


## Exercice 1 : Monitoring de l’activité d’un processus (suite)
# Question 1
https://www.kernel.org/doc/html/v4.13/process/applying-patches.html
https://lwn.net/Articles/167034/


The -mm patches and the linux-next tree

The -mm patches are experimental patches released by Andrew Morton.

In the past, -mm tree were used to also test subsystem patches, but this function is now done via the linux-next <https://www.kernel.org/doc/man-pages/linux-next.html> tree. The Subsystem maintainers push their patches first to linux-next, and, during the merge window, sends them directly to Linus.

The -mm patches serve as a sort of proving ground for new features and other experimental patches that aren’t merged via a subsystem tree. Once such patches has proved its worth in -mm for a while Andrew pushes it on to Linus for inclusion in mainline.

The linux-next tree is daily updated, and includes the -mm patches. Both are in constant flux and contains many experimental features, a lot of debugging patches not appropriate for mainline etc., and is the most experimental of the branches described in this document.

These patches are not appropriate for use on systems that are supposed to be stable and they are more risky to run than any of the other branches (make sure you have up-to-date backups – that goes for any experimental kernel but even more so for -mm patches or using a Kernel from the linux-next tree).

Testing of -mm patches and linux-next is greatly appreciated since the whole point of those are to weed out regressions, crashes, data corruption bugs, build breakage (and any other bug in general) before changes are merged into the more stable mainline Linus tree.

But testers of -mm and linux-next should be aware that breakages are more common than in any other tree.


# Question 2
Voir code. ne pas oublier d'initialiser la liste et de supprimer a la fin.
ajout d'une methode qui print tous les elements a la fin dans le exit pour
tester.
# Question 3
Voir code. ne pas oublier le kobject put et le sys remove.
# Question 4
[root@pnl-tp ~]# ./ins.sh
[  879.864981] Monitoring module loaded
[root@pnl-tp ~]# cat /sys/kernel/directory_monitor/taskmonitor
[  884.683417] pid:267 num:0 : usr 1 sys 3
[  884.685308] pid:267 num:1 : usr 1 sys 3
[  884.686911] pid:267 num:2 : usr 1 sys 3
[  884.688221] pid:267 num:3 : usr 1 sys 3


## Exercice 2 : Récupération automatique de la mémoire, le shrinker
https://lwn.net/Articles/527210/
https://fr.wikipedia.org/wiki/Non_uniform_memory_access
https://elixir.bootlin.com/linux/v4.9.85/source/include/linux/shrinker.h#L49
# Question 1
/* Compte le nombre que l'on peut degommer */
unsigned long (*count_objects)(struct shrinker *,
				       struct shrink_control *sc);

/* Degomme et rend le nombre qu'on a degomme */
unsigned long (*scan_objects)(struct shrinker *,
				      struct shrink_control *sc);


int seeks;	/* seeks to recreate an obj */
long batch;	/* reclaim batch size, 0 = default */
unsigned long flags; /* pour le NUMA, non uniform memory access */

/* These are for internal use */
struct list_head list;
/* objs pending delete, per node */
atomic_long_t *nr_deferred;



# Question 2 et Question 3
On cree autant d'objet que l'on veut, des que la pression memoire arrive on
supprime tous les objets. On continue a creer des objets.
Affichage : Debut et fin de destruction.
Au desarmement du module : affichage du total cree et total detruit. et nomnbre
de vivants.



## Exercice 3 : Gestion efficace de la mémoire avec les slab
https://lwn.net/Articles/319686/
toto
# Question 1
Probleme :
struct task_sample {
	struct list_head list;
	cputime_t utime;
	cputime_t stime;
};
	struct task_sample *q = kmalloc(sizeof(struct task_sample), GFP_KERNEL);
	pr_info("sizeof(struct task_sample) :%lu\n", sizeof(struct task_sample)); // RENVOIE 32
	pr_info("ksize(q):%lu\n", ksize(q)); // RENVOIE 32
	kfree(q);

Il y avait un probleme cela ne fonctionnait pas au debut. il a fallut rajouter
un int dans la structure pou faire apparaitre le probleme.
struct task_sample {
	struct list_head list;
	cputime_t utime;
	cputime_t stime;
	int i;
};

On obtient desormais
[   56.034134] sizeof(struct task_sample) :40
[   56.036232] ksize(q):64
[   56.037509] Monitoring module unloaded

/**
 * ksize - get the actual amount of memory allocated for a given object
 * @objp: Pointer to the object
 *
 * kmalloc may internally round up allocations and return more memory
 * than requested. ksize() can be used to determine the actual amount of
 * memory allocated. The caller may use this additional memory, even though
 * a smaller amount of memory was initially specified with the kmalloc call.
 * The caller must guarantee that objp points to a valid object previously
 * allocated with either kmalloc() or kmem_cache_alloc(). The object
 * must not be freed during the duration of the call.
 */
size_t ksize(const void *objp);

On aligne sur les puissances de 2 donc vu que notre strcture est desaligne et
fait 40 oct on va realigner sur du 64 pour qu'en memoire ce soit toujours
aligne sur une puissance de 2.


# Question 2
https://argp.github.io/2012/01/03/linux-kernel-heap-exploitation/
http://phrack.org/issues/66/8.html#article
http://phrack.org/issues/66/15.html#article
https://stackoverflow.com/questions/14450133/slab-memory-management
https://www.amazon.com/dp/0672329468/?tag=stackoverflow17-20
http://docs.zephyrproject.org/kernel/memory/slabs.html
http://www.secretmango.com/jimb/Whitepapers/slabs/slab.html
http://students.mimuw.edu.pl/ZSO/Wyklady/05_pamiec/5_pamiec_en.html

Un acces exclisif existe entre les 3 types d'allocateurs, ils sont dans la
gestion de la memoire juste au dessus de l'allocateur de page du systeme.
(i.e. you can only have one of them enabled/compiled in your kernel)

WIKIPEDIA: Slab allocation is a memory management mechanism intended for the efficient memory allocation of kernel objects. It eliminates fragmentation caused by allocations and deallocations. The technique is used to retain allocated memory that contains a data object of a certain type for reuse upon subsequent allocations of objects of the same type. 

	"A slab consists of one or more pages of virtually contiguous
        memory carved up into equal-size chunks, with a reference count
        indicating how many of those chunks have been allocated."
	Page 5, 3.2 Slabs. [1]

Le but est de limiter la fragmentation memoire. et grandement simplifier les
allocation, et retrait en cas de pression memoire.
SLAB: Le bon vieux.
SLOB: Le maigrichon.
SLUB: L'actuel.
SLQB: Perf HPC

# Question 3
	#define KMEM_CACHE(__struct, __flags) kmem_cache_create(#__struct,\
		sizeof(struct __struct), __alignof__(struct __struct),\
		(__flags), NULL)

/**
 * kmem_cache_create - Create a cache.
 * @name: A string which is used in /proc/slabinfo to identify this cache.
 * @size: The size of objects to be created in this cache.
 * @align: The required alignment for the objects.
 * @flags: SLAB flags
 * @ctor: A constructor for the objects.
 */

struct kmem_cache *
kmem_cache_create (const char *name, size_t size,
       size_t align, slab_flags_t flags, void (*ctor)(void*))


## Exercice 4 : Pré-allocation de la mémoire avec les mempools

# Question 1
https://lwn.net/Articles/22909/

# Question 2
Il ne nous faut pas implementer grand chose grace a :
" Creators of mempools will often want to use the slab allocator to do the actual object allocation and deallocation. To do that, create the slab, pass it in to mempool_create() as the pool_data value, and give mempool_alloc_slab and mempool_free_slab as the allocation and deallocation functions."


## Exercice 5 : Récupération « au besoin » de la mémoire : kref
https://elixir.bootlin.com/linux/v4.9.85/source/Documentation/kref.txt

# Question 1
struct task_sample {
	struct list_head list; 	// 16
	cputime_t utime;	// 8
	struct kref refcount;
	cputime_t stime;	// 8
};

kref_init(&ts->refcount);

# Question 2
http://www.kroah.com/linux/talks/ols_2004_kref_talk/
(THX TO KROAH)
Il faut utiliser kref_get, kref_put et release.

# Question 3
1 seul reference sinon il passe dans le destructeur et comme ca au besoin il
est supprime.


## Exercice 6 : Accès aux informations de monitoring depuis l’espace utilisateur
# Question 1
-Pour creer un directory, si parent est NULL alors racine == debuhgfs:
struct dentry *debugfs_create_dir(const char *name, struct dentry *parent);

-Pour creer un fichier :
struct dentry *debugfs_create_file_size(const char *name, umode_t mode,
				struct dentry *parent, void *data,
				const struct file_operations *fops,
				loff_t file_size);

-Pour detruire:
void debugfs_remove(struct dentry *dentry);

# Question 2
struct dentry* my_dentry;

static int debugfs_task_monitor_show(struct seq_file *m, void *v);
static int debugfs_taskmonitor_open(struct inode *inode, struct file *file);

static const struct file_operations taskmonitor_fops = {
	.open		= debugfs_taskmonitor_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};


# Question 3
seq_printf(m, "pid:%hu num:%d : usr %lu sys %lu\n",
				pid, i++, ts->utime, ts->stime);

Du coup quand on cat ca affiche bien la liste et quand on vi dans le fichier
aussi.


## Exercice 7 : Monitoring de plusieurs processus simultanément
# Question 1
# Question 2
# Question 3
