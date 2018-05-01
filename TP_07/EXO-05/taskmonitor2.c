#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/slab.h>
#include <linux/shrinker.h>
#include <linux/mempool.h>
#include <linux/debugfs.h>

#include "taskmonitor.h"
#include "my_shrinker.h"
#include "my_debugfs.h"
#include "my_sysfs.h"

/* MODULE DECLARATION*/
MODULE_DESCRIPTION("A process monitor");
MODULE_AUTHOR("William Fabre <contact@williamfabre.fr>");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.1");
static unsigned short target = 1; /* default pid to monitor */
static unsigned frequency = 1; /* sampling frequency */
module_param(target, ushort, 0400);
module_param(frequency, uint, 0600);

/* Structures */
static struct task_struct *monitor_thread;
static struct task_sample *ex5_crash_tester;
static struct my_tasks *all_tasks;

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
struct kobj_attribute tm_attr =
__ATTR(taskmonitor, 0644, sysfs_show, sysfs_store);

/* Shrinker declaration */
static struct shrinker my_shrinker = {
	.count_objects = my_shrink_count,
	.scan_objects = my_shrink_scan,
	.seeks = DEFAULT_SEEKS,
};

/* Kref declaration */
void task_sample_release(struct kref *kref);
void task_monitor_release(struct kref *kref);

/* Debugfs declration for dentry */
struct dentry* my_dentry;
static const struct file_operations taskmonitor_fops = {
	.open		= debugfs_taskmonitor_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

unsigned long my_shrink_scan(struct shrinker *shrink,
		struct shrink_control *sc)
{
	int res;
	struct task_monitor *p_tm, *q_tm;
	struct task_sample *p_ts, *q_ts;
	struct list_head *s_list;
	struct list_head *m_list = &(all_tasks->tm_head.tm_list);

	pr_info("Start deletion\n");
	res = 0;

	if (!(sc->gfp_mask & __GFP_FS))
		return SHRINK_STOP;

	list_for_each_entry_safe(p_tm, q_tm, m_list, tm_list){
		if (kref_get_unless_zero(&p_tm->refcount) > 0){
			s_list = &(p_tm->head.list);
			list_for_each_entry_safe(p_ts, q_ts, s_list, list){
				kref_put(&p_ts->refcount, task_sample_release);
				res++;
			}
			kref_put(&p_tm->refcount, task_monitor_release);
		}
	}
	pr_info("End deletion\n");
	return res;
}

unsigned long my_shrink_count(struct shrinker *shrink,
		struct shrink_control *sc)
{
	int res = 0;
	struct task_monitor *p_tm;
	struct list_head *m_list = &(all_tasks->tm_head.tm_list);

	list_for_each_entry(p_tm, m_list, tm_list){
		if (kref_get_unless_zero(&p_tm->refcount) > 0){
			res += p_tm->nb_samples;
			kref_put(&p_tm->refcount, task_monitor_release);
		}
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
		kref_put(&ts->refcount, task_sample_release);
	}
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

void task_monitor_release(struct kref *kref)
{
	struct task_monitor* tm;

	tm = container_of(kref, struct task_monitor, refcount);

	list_del(&tm->tm_list);
	kfree(tm);
}

int monitor_fn(void *data)
{
	struct task_monitor *p_tm;
	struct list_head *m_list = &(all_tasks->tm_head.tm_list);

	while (!kthread_should_stop()) {
		set_current_state(TASK_INTERRUPTIBLE);
		if (schedule_timeout(max(HZ/frequency/20, 1U/20)))
			return -EINTR;
		list_for_each_entry(p_tm, m_list, tm_list){
			ex5_crash_tester = save_sample(p_tm);
		}
	}
	return 0;
}

int monitor_pid(pid_t pid, struct task_monitor *tm)
{
	struct pid *p = find_get_pid(pid);

	if (!p) {
		pr_err("pid %hu not found\n", pid);
		return -ESRCH;
	}

	tm->pid = p;

	return 0;
}

static ssize_t sysfs_show(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf)
{
	pid_t pid;
	struct task_monitor *p_tm;
	struct task_sample *p_ts;
	struct list_head *s_list;
	struct list_head *task_head = &(all_tasks->tm_head.tm_list);

	int i = 0;
	int count = 0;

	list_for_each_entry(p_tm, task_head, tm_list){
		if (kref_get_unless_zero(&p_tm->refcount) > 0){
			pid = pid_nr(p_tm->pid);
			s_list = &(p_tm->head.list);
			list_for_each_entry_reverse(p_ts, s_list, list){
				if (kref_get_unless_zero(&p_ts->refcount) > 0){
					if (count == PAGE_SIZE)
						return count;
					scnprintf(buf+count, PAGE_SIZE - count,
							"pid:%hu num:%d usr:%lu sys:%lu\n",
							pid, i++, p_ts->utime,
							p_ts->stime);
					count = strlen(buf);
					kref_put(&p_ts->refcount, task_sample_release);
				}
			}
			kref_put(&p_tm->refcount, task_monitor_release);
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
	pid_t pid;
	struct task_monitor *p_tm;
	struct task_sample *p_ts;
	struct list_head *s_list;
	struct list_head *task_head = &(all_tasks->tm_head.tm_list);
	int i = 0;

	list_for_each_entry(p_tm, task_head, tm_list){
		if (kref_get_unless_zero(&p_tm->refcount) > 0){
			pid = pid_nr(p_tm->pid);
			s_list = &(p_tm->head.list);
			list_for_each_entry_reverse(p_ts, s_list, list){
				if (kref_get_unless_zero(&p_ts->refcount) > 0){
					seq_printf(m, "pid:%hu num:%d : usr %lu sys %lu\n",
							pid, i++, p_ts->utime,
							p_ts->stime);
					kref_put(&p_ts->refcount, task_sample_release);
				}
			}
			kref_put(&p_tm->refcount, task_monitor_release);
		}
	}
	return 0;
}

static int debugfs_taskmonitor_open(struct inode *inode, struct file *file)
{
	return single_open(file, debugfs_task_monitor_show, NULL);
}

void tm_allocator(int pid){

	struct task_monitor* tm;
	struct list_head *task_head = &(all_tasks->tm_head.tm_list);

	tm = kmalloc(sizeof(struct task_monitor), GFP_KERNEL);

	if (monitor_pid(pid, tm) != 0){
		kfree(tm);
		return;
	}

	INIT_LIST_HEAD(&tm->head.list);
	tm->nb_samples = 0;
	kref_init(&tm->refcount);
	list_add_tail(&tm->tm_list, task_head);
	return;
}

static int monitor_init(void)
{
	int i = 1;
	int err = 0;

	/* allocation of the main structure */
	all_tasks = kmalloc(sizeof(all_tasks), GFP_KERNEL);
	mutex_init(&all_tasks->lock);
	INIT_LIST_HEAD(&(all_tasks->tm_head.tm_list));

	mutex_lock(&all_tasks->lock);
	/* for (i = 1; i < 5; i++) {*/
		tm_allocator(i);
	/* }*/
	mutex_unlock(&all_tasks->lock);

	/* allocation of the kmemcache */
	cached_ts = KMEM_CACHE(task_sample, 0);

	/* preparation of the pool */
	my_mempool = mempool_create(5,
			mempool_alloc_slab,
			mempool_free_slab,
			cached_ts);

	/* checking if error with the pid */

	/* starting the threads */
	monitor_thread = kthread_run(monitor_fn, NULL, "monitor_fn");
	if (IS_ERR(monitor_thread)) {
		err = PTR_ERR(monitor_thread);
		goto r_thread;
	}

	/* preparting the debugfs */
	my_dentry = debugfs_create_file("taskmonitor", S_IRUGO, NULL, NULL,
			&taskmonitor_fops);

	if (!my_dentry)
		pr_warn("Failed to create the debugfs taskmonitor file\n");

	/* creating the directory for sysfs */
	tm_obj = kobject_create_and_add("directory_monitor", kernel_kobj);
	if (sysfs_create_file(tm_obj, &tm_attr.attr)){
		printk(KERN_INFO"Cannot create sysfs file\n");
		goto r_sysfs;
	}

	/* variable for sysfs write */
	run = true;

	/* registering the shrinker */
	if (register_shrinker(&my_shrinker))
		pr_warn("bcache: %s: could not register shrinker\n",
				__func__);

	pr_info("Monitoring module loaded\n");
	return 0;
r_sysfs:
	kobject_put(tm_obj);
r_thread:
	return err;

}
module_init(monitor_init);

static void monitor_exit(void)
{
	int i;
	struct task_monitor *p_tm, *q_tm;
	struct task_sample *p_ts, *q_ts;
	struct list_head *s_list;
	struct list_head *task_head = &(all_tasks->tm_head.tm_list);

	i = 0;

	if (monitor_thread)
		kthread_stop(monitor_thread);

	pr_info("C= %d, D= %d\n", total_created, total_destroyed);

	list_for_each_entry_safe(p_tm, q_tm, task_head, tm_list){
		s_list = &(p_tm->head.list);
		list_for_each_entry_safe(p_ts, q_ts, s_list, list){
			kmem_cache_free(cached_ts, p_ts);
		}
		put_pid(p_tm->pid);
		kref_put(&p_tm->refcount, task_monitor_release);
	}

	/* directory sysfs destroy */
	kobject_put(tm_obj);
	/* removing the file from sysfs */
	sysfs_remove_file(kernel_kobj, &tm_attr.attr);
	/* useless but everything check */
	run = false;
	/* unregister the shrinker */
	unregister_shrinker(&my_shrinker);
	/* destro the pool */
	mempool_destroy(my_mempool);
	/* destroy the cache */
	kmem_cache_destroy(cached_ts);
	/* remove debugfs */
	debugfs_remove(my_dentry);
	/* destroy the mutex */
	mutex_destroy(&all_tasks->lock);
	/* removing the tasks list */
	kfree(all_tasks);
	pr_info("Monitoring module unloaded\n");

}
module_exit(monitor_exit);
