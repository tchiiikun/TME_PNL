#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/slab.h>
#include <linux/shrinker.h>
#include <linux/mempool.h>
#include <linux/debugfs.h>

/* MODULE DECLARATION*/
MODULE_DESCRIPTION("A process monitor");
MODULE_AUTHOR("William Fabre <contact@williamfabre.fr>");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.1");
static unsigned short target = 1; /* default pid to monitor */
static unsigned frequency = 1; /* sampling frequency */
module_param(target, ushort, 0400);
module_param(frequency, uint, 0600);

/******************************* STRUCTURE ***********************************/
/****************************** DECLARATION **********************************/

static struct task_sample {
	struct list_head list; 	// 16
	cputime_t utime;	// 8
	struct kref refcount;
	cputime_t stime;	// 8
};

static struct task_monitor {
	struct list_head tm_list;
	struct task_sample head;
	struct mutex lock;
	struct pid *pid;
	int nb_samples;
};

static struct task_struct *monitor_thread;

static struct task_sample  *ex5_crash_tester; // doing dirty stuff check kref.

static struct my_tasks{
	struct task_monitor tm_head;
} *all_tasks;

/* Mempool declaration */
static mempool_t *my_mempool;

static struct kmem_cache *cached_ts;

/* total for check at module rm */
static int total_created = 0;
static int total_destroyed = 0;

/* Sysfs file declaration */
static struct kobject *tm_obj;
static char my_string[256];
static bool run;

static ssize_t sysfs_store(struct kobject *kobj,
		struct kobj_attribute *attr, const char *buf, size_t count);

static ssize_t sysfs_show(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf);

struct kobj_attribute tm_attr =
__ATTR(taskmonitor, 0644, sysfs_show, sysfs_store);


/* Shrinker declaration */
unsigned long
my_shrink_scan(struct shrinker *shrink, struct shrink_control *sc);

unsigned long
my_shrink_count(struct shrinker *shrink, struct shrink_control *sc);

static struct shrinker my_shrinker = {
	.count_objects = my_shrink_count,
	.scan_objects = my_shrink_scan,
	.seeks = DEFAULT_SEEKS,
};


/* Kref declaration */
void task_sample_release(struct kref *kref);

/* Debugfs declration for dentry */
struct dentry* my_dentry;

static int debugfs_task_monitor_show(struct seq_file *m, void *v);
static int debugfs_taskmonitor_open(struct inode *inode, struct file *file);

static const struct file_operations taskmonitor_fops = {
	.open		= debugfs_taskmonitor_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};



	unsigned long
my_shrink_scan(struct shrinker *shrink, struct shrink_control *sc)
{
	int res;
	struct task_monitor *tmp_tm, *p;
	struct task_sample *ts, *tmp;
	pr_info("Start deletion\n");
	res = 0;

	if (!(sc->gfp_mask & __GFP_FS))
		return SHRINK_STOP;

	list_for_each_entry_safe(tmp_tm, p, &all_tasks->tm_head.tm_list,
			tm_list){
		if (mutex_is_locked(&tmp_tm->lock))
			return SHRINK_STOP;
		list_for_each_entry_safe(ts, tmp,  &(tmp_tm->head.list), list){
			/* kmem_cache_free(cached_ts, ts);*/
			kref_put(&ts->refcount, task_sample_release);
			res++;
		}
	}
	pr_info("End deletion\n");
	return res;
}

	unsigned long
my_shrink_count(struct shrinker *shrink, struct shrink_control *sc)
{
	int res = 0;
	struct task_monitor *tmp_tm, *p;

	list_for_each_entry_safe(tmp_tm, p, &all_tasks->tm_head.tm_list,
			tm_list){
		res += tmp_tm->nb_samples;
	}
	return res;
}


/************************************ CODE ***********************************/

bool get_sample(struct task_monitor *tm, struct task_sample *sample)
{
	struct task_struct *task;
	bool alive = false;

	task = get_pid_task(tm->pid, PIDTYPE_PID);
	if (!task) {
		pr_err("can't find task for pid %u\n", pid_nr(tm->pid));
		goto out;
	}

	task_lock(task);
	alive = pid_alive(task);
	if (alive)
		task_cputime(task, &sample->utime, &sample->stime);
	task_unlock(task);

	put_task_struct(task);
out:
	return alive;
}

void print_sample(struct task_monitor *tm)
{
	struct task_sample ts;
	pid_t pid = pid_nr(tm->pid);
	bool alive;

	alive = get_sample(tm, &ts);

	if (!alive)
		pr_err("%hd is dead\n", pid);
	else
		pr_info("%hd usr %lu sys %lu\n", pid, ts.utime, ts.stime);
}

struct task_sample *save_sample(struct task_monitor* tm)
{
	struct task_sample* ts;

	ts = kmem_cache_alloc(cached_ts, GFP_NOFS);
	kref_init(&ts->refcount);

	if (!ts){
		pr_err("can't allocate for save_sample\n");
		goto out;
	}

	if (kref_get_unless_zero(&ts->refcount) > 0){
		get_sample(tm, ts);
		list_add_tail(&ts->list, &(tm->head.list));
		total_created++;
		tm->nb_samples++;
	}
	kref_put(&ts->refcount, task_sample_release);

	return ts;
out:
	return NULL;
}

void task_sample_release(struct kref *kref)
{
	struct task_sample *t;
	struct task_monitor* tm;

	t = container_of(kref, struct task_sample, refcount);
	tm = container_of(t, struct task_monitor, head);

	list_del(&t->list);
	kmem_cache_free(cached_ts, t);

	total_destroyed++;
	tm->nb_samples--;
}

int monitor_fn(void *data)
{
	struct task_monitor *tmp_tm, *p;

	while (!kthread_should_stop()) {

		set_current_state(TASK_INTERRUPTIBLE);

		if (schedule_timeout(max(HZ/frequency/20, 1U/20)))
			return -EINTR;
		list_for_each_entry_safe(tmp_tm, p, &all_tasks->tm_head.tm_list,
				tm_list){

			ex5_crash_tester = save_sample(tmp_tm);
			/* pr_info("usr %lu sys %lu\n",*/
			/*                 ex5_crash_tester->utime,*/
			/*                 ex5_crash_tester->stime);*/

		}
	}
	return 0;
}

int monitor_pid(pid_t pid)
{
	struct task_monitor *tmp;

	struct pid *p = find_get_pid(pid);

	if (!p) {
		pr_err("pid %hu not found\n", pid);
		return -ESRCH;
	}

	tmp = kmalloc(sizeof(*tmp), GFP_KERNEL);
	mutex_init(&tmp->lock);
	tmp->nb_samples = 0;
	tmp->pid = p;
	INIT_LIST_HEAD(&tmp->head.list);
	list_add_tail(&tmp->tm_list, &all_tasks->tm_head.tm_list);
	return 0;
}

static ssize_t sysfs_show(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf)
{
	int i = 0;
	int count = 0;
	struct task_sample* ts;
	struct task_monitor *tmp_tm, *tmp_p;

	list_for_each_entry_safe(tmp_tm, tmp_p, &all_tasks->tm_head.tm_list,
			tm_list){

		pid_t pid = pid_nr(tmp_tm->pid);

		list_for_each_entry_reverse(ts, &(tmp_tm->head.list), list){

			if (kref_get_unless_zero(&ts->refcount) > 0){
				if (count == PAGE_SIZE)
					return count;
				scnprintf(buf+count,
						PAGE_SIZE - count,
						"pid:%hu num:%d usr:%lu sys:%lu\n",
						pid,
						i++,
						ts->utime,
						ts->stime);
				count = strlen(buf);
				kref_put(&ts->refcount, task_sample_release);
			}
		}
	}
	return strlen(buf);
}

static ssize_t sysfs_store(struct kobject *kobj,
		struct kobj_attribute *attr, const char *buf, size_t count)
{
	int res;

	res = strlen(strncpy(my_string, buf, 255));
	if (strncmp("stop", my_string, 4) == 0){
		if (run == true){
			kthread_stop(monitor_thread);
			run = false;
			pr_info("thread Stopped\n");
		}
	}

	if (strncmp("start", my_string, 4) == 0){
		if (run == false){
			monitor_thread = kthread_run(monitor_fn,
					NULL, "monitor_fn");
			run = true;
			pr_info("thread Started\n");
		}
	}

	return res;
}

static int debugfs_task_monitor_show(struct seq_file *m, void *v)
{
	int i = 0;
	struct task_sample* ts;
	struct task_monitor *tmp_tm, *tmp_p;

	list_for_each_entry_safe(tmp_tm, tmp_p, &all_tasks->tm_head.tm_list,
			tm_list){
		pid_t pid = pid_nr(tmp_tm->pid);
		list_for_each_entry_reverse(ts, &(tmp_tm->head.list), list){
			if (kref_get_unless_zero(&ts->refcount) > 0){
				seq_printf(m, "pid:%hu num:%d : usr %lu sys %lu\n",
						pid,
						i++,
						ts->utime, 
						ts->stime);
				kref_put(&ts->refcount,
						task_sample_release);
			}
		}
	}
	return 0;
}


static int debugfs_taskmonitor_open(struct inode *inode, struct file *file)
{
	return single_open(file, debugfs_task_monitor_show, NULL);
}

static int monitor_init(void)
{
	struct task_monitor *tmp, *p;
	int err;
	
	INIT_LIST_HEAD(&all_tasks->tm_head.tm_list);

	err = monitor_pid(target);
	if (err)
		return err;

	cached_ts = KMEM_CACHE(task_sample, 0);

	my_mempool = mempool_create(5,
			mempool_alloc_slab,
			mempool_free_slab,
			cached_ts);

	/* INIT_LIST_HEAD(&tm->head.list);*/

	/* list_for_each_entry(tmp, &all_tasks->tm_head.tm_list, tm_list){*/
	/*         tmp = kmalloc(sizeof(*tmp), GFP_KERNEL);*/
	/*         INIT_LIST_HEAD(&tmp->head.list);*/
	/*         mutex_init(&tmp->lock);*/
	/*         tmp->nb_samples = 0;*/
	/* }*/

	/* INIT_LIST_HEAD(&tm->head.list);*/

	monitor_thread = kthread_run(monitor_fn, NULL, "monitor_fn");
	if (IS_ERR(monitor_thread)) {
		err = PTR_ERR(monitor_thread);
		goto r_thread;
	}

	my_dentry = debugfs_create_file("taskmonitor", S_IRUGO, NULL, NULL,
			&taskmonitor_fops);

	if (!my_dentry)
		pr_warn("Failed to create the debugfs taskmonitor file\n");

	tm_obj = kobject_create_and_add("directory_monitor", kernel_kobj);
	if (sysfs_create_file(tm_obj, &tm_attr.attr)){
		printk(KERN_INFO"Cannot create sysfs file\n");
		goto r_sysfs;
	}
	run = true;
	if (register_shrinker(&my_shrinker))
		pr_warn("bcache: %s: could not register shrinker\n",
				__func__);

	pr_info("Monitoring module loaded\n");
	return 0;
r_sysfs:
	kobject_put(tm_obj);
r_thread:
	list_for_each_entry_safe(tmp, p, &all_tasks->tm_head.tm_list,
			tm_list){
		put_pid(tmp->pid);
		kfree(tmp);
	}
	return err;

}
module_init(monitor_init);

static void monitor_exit(void)
{
	int i;
	struct task_monitor *tmp_tm, *p;
	struct task_sample *ts, *tmp;

	i = 0;
	if (monitor_thread)
		kthread_stop(monitor_thread);

	/* pr_info("nb instances : %d\n", tm->nb_samples);*/
	pr_info("C= %d, D= %d\n", total_created, total_destroyed);

	list_for_each_entry_safe(tmp_tm, p, &all_tasks->tm_head.tm_list,
			tm_list){
		list_for_each_entry_safe(ts, tmp,  &(tmp_tm->head.list), list){
			/* kmem_cache_free(cached_ts, ts);*/
			kref_put(&ts->refcount, task_sample_release);
		}
	}

	list_for_each_entry_safe(tmp_tm, p, &all_tasks->tm_head.tm_list,
			tm_list){
		put_pid(tmp_tm->pid);
		kfree(tmp_tm);
	}
	kobject_put(tm_obj);
	sysfs_remove_file(kernel_kobj, &tm_attr.attr);
	run = false;
	unregister_shrinker(&my_shrinker);
	mempool_destroy(my_mempool);
	kmem_cache_destroy(cached_ts);

	debugfs_remove(my_dentry);

	pr_info("Monitoring module unloaded\n");
}
module_exit(monitor_exit);
