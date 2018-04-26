#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/slab.h>
#include <linux/shrinker.h>

MODULE_DESCRIPTION("A process monitor");
MODULE_AUTHOR("Maxime Lorrillere <maxime.lorrillere@lip6.fr>");
MODULE_LICENSE("GPL");
MODULE_VERSION("1");

static unsigned short target = 1; /* default pid to monitor */
static unsigned frequency = 1; /* sampling frequency */

module_param(target, ushort, 0400);
module_param(frequency, uint, 0600);

struct task_sample {
	struct list_head list; // 16
	cputime_t utime; // 8
	int a;
	cputime_t stime; // 8
};

struct task_monitor {
	struct task_sample head;
	struct mutex lock;
	struct pid *pid;
	int nb_samples;
};

static struct task_monitor *tm;

static struct task_struct *monitor_thread;

static struct kobject *tm_obj;
static char my_string[256];
static bool run;

static int total_created = 0;
static int total_destroyed = 0;




unsigned long
my_shrink_scan(struct shrinker *shrink, struct shrink_control *sc)
{
	int res;
	struct task_sample *ts, *tmp;
	pr_info("Start deletion\n");
	res = 0;

	if (!(sc->gfp_mask & __GFP_FS))
		return SHRINK_STOP;

	if (mutex_is_locked(&tm->lock))
		return SHRINK_STOP;

	mutex_lock(&tm->lock);
	list_for_each_entry_safe(ts, tmp,  &(tm->head.list), list){
		list_del(&ts->list);
		kfree(ts);
		total_destroyed++;
		tm->nb_samples--;
		res++;
	}
	mutex_unlock(&tm->lock);
	pr_info("End deletion");
	return res;
}

unsigned long
my_shrink_count(struct shrinker *shrink, struct shrink_control *sc)
{
	return tm->nb_samples;
}

static struct shrinker my_shrinker = {
	.count_objects = my_shrink_count,
	.scan_objects = my_shrink_scan,
	.seeks = DEFAULT_SEEKS,
};


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

void save_sample(void)
{
	struct task_sample* ts;

	ts = kmalloc(sizeof(struct task_sample), GFP_KERNEL);
	if (!ts){
		pr_err("can't allocate for save_sample");
		goto out;
	}

	mutex_lock(&tm->lock);
	get_sample(tm, ts);
	list_add_tail(&ts->list, &(tm->head.list));
	total_created++;
	tm->nb_samples++;
	mutex_unlock(&tm->lock);

	return;
out:
	kfree(ts);
	return;

}

int monitor_fn(void *data)
{
	while (!kthread_should_stop()) {
		set_current_state(TASK_INTERRUPTIBLE);
		if (schedule_timeout(max(HZ/frequency/20, 1U/20)))
			return -EINTR;
		/* print_sample(tm);*/
		save_sample();
	}
	return 0;
}

int monitor_pid(pid_t pid)
{
	struct pid *p = find_get_pid(pid);

	if (!p) {
		pr_err("pid %hu not found\n", pid);
		return -ESRCH;
	}
	tm = kmalloc(sizeof(*tm), GFP_KERNEL);
	tm->pid = p;

	return 0;
}

static ssize_t sysfs_show(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf)
{
	int i = 0;
	struct task_sample* ts;
	pid_t pid = pid_nr(tm->pid);

	list_for_each_entry_reverse(ts, &(tm->head.list), list){
		if (i == PAGE_SIZE)
			return 0;
		pr_info("pid:%hu num:%d : usr %lu sys %lu\n",
				pid, i++, ts->utime, ts->stime);
	}
	return 0;
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
			pr_info("thread Stopped");
		}
	}

	if (strncmp("start", my_string, 4) == 0){
		if (run == false){
			monitor_thread = kthread_run(monitor_fn,
					NULL, "monitor_fn");
			run = true;
			pr_info("thread Started");
		}
	}
	return res;
}

struct kobj_attribute tm_attr =
__ATTR(taskmonitor, 0644, sysfs_show, sysfs_store);

static int monitor_init(void)
{
	int err = monitor_pid(target);
	mutex_init(&tm->lock);
	tm->nb_samples = 0;
	INIT_LIST_HEAD(&tm->head.list);

	if (err)
		return err;

	monitor_thread = kthread_run(monitor_fn, NULL, "monitor_fn");
	if (IS_ERR(monitor_thread)) {
		err = PTR_ERR(monitor_thread);
		goto abort;
	}

	tm_obj = kobject_create_and_add("directory_monitor", kernel_kobj);
	if(sysfs_create_file(tm_obj, &tm_attr.attr)){
		printk(KERN_INFO"Cannot create sysfs file\n");
		goto r_sysfs;
	}
	run = true;
	register_shrinker(&my_shrinker);

	pr_info("Monitoring module loaded\n");
	return 0;

r_sysfs:
	kobject_put(tm_obj);
abort:
	put_pid(tm->pid);
	kfree(tm);
	return err;
}

static void monitor_exit(void)
{
	int i;
	struct task_sample *ts, *tmp;

	i = 0;
	if (monitor_thread)
		kthread_stop(monitor_thread);

	/* pr_info("total size: %lu ---- sizeof struct : %lu\n",*/
	/*                 ksize(&tm->head),*/
	/*                 sizeof(struct task_sample));*/
	/* pr_info("nb instances : %d\n", tm->nb_samples);*/
	/* pr_info("C= %d, D= %d", total_created, total_destroyed);*/
	/* pr_info("task_sample:%lu\n list_head:%lu\n cputime_t:%lu\n cputime_t:%lu\n",*/
	/*         sizeof(struct task_sample),*/
	/*         sizeof(struct list_head),*/
	/*         sizeof(cputime_t),*/
	/*         sizeof(cputime_t));*/
	/* list_for_each_entry(ts, &(tm->head.list), list){*/
	/*         pr_info("%d : usr %lu sys %lu\n", i++, ts->utime, ts->stime);*/
	/* }*/
	struct task_sample *q = kmalloc(sizeof(struct task_sample), GFP_KERNEL);
	pr_info("sizeof(struct task_sample) :%lu\n", sizeof(struct task_sample));
	/* pr_info("sizeof(u64) :%lu\n", sizeof(u64));*/
	/* pr_info("sizeof(u32) :%lu\n", sizeof(u32));*/
	/* pr_info("sizeof(u8) :%lu\n", sizeof(u8));*/
	pr_info("ksize(q):%lu\n", ksize(q));
	kfree(q);



	list_for_each_entry_safe(ts, tmp,  &(tm->head.list), list){
		list_del(&ts->list);
		kfree(ts);
	}

	put_pid(tm->pid);
	kfree(tm);

	kobject_put(tm_obj);
	sysfs_remove_file(kernel_kobj, &tm_attr.attr);
	run = false;
	unregister_shrinker(&my_shrinker);

	pr_info("Monitoring module unloaded\n");
}

module_init(monitor_init);
module_exit(monitor_exit);
