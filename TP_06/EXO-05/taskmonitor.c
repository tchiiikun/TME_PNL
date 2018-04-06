#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/slab.h>

#include <linux/fs.h>
#include <asm/io.h>
#include<linux/sysfs.h>
#include<linux/kobject.h> 

MODULE_DESCRIPTION("A process monitor");
MODULE_AUTHOR("Maxime Lorrillere <maxime.lorrillere@lip6.fr>");
MODULE_LICENSE("GPL");
MODULE_VERSION("1");

unsigned short target = 1; /* default pid to monitor */
unsigned frequency = 1; /* sampling frequency */

module_param(target, ushort, 0400);
module_param(frequency, uint, 0600);

struct task_monitor {
	struct pid *pid;
};

struct task_monitor *tm;

struct task_struct *monitor_thread;

struct task_sample {
	cputime_t utime;
	cputime_t stime;
};

struct kobject *tm_obj;
static char my_string[256];
static bool run;

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
		pr_err("%hd is dead\n",	pid);
	else
		pr_info("%hd usr %lu sys %lu\n", pid, ts.utime, ts.stime);
	// RELOU
}

int monitor_fn(void *data)
{
	while (!kthread_should_stop()) {
		set_current_state(TASK_INTERRUPTIBLE);
		if (schedule_timeout(max(HZ/frequency, 1U)))
			return -EINTR;

		print_sample(tm);
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
	struct task_sample ts;
	pid_t pid = pid_nr(tm->pid);
	bool alive;

	alive = get_sample(tm, &ts);

	if (!alive)
		pr_err("%hd is dead\n",	pid);
	else
		pr_info("%hd usr %lu sys %lu\n", pid, ts.utime, ts.stime);
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
	if (run && monitor_thread)
		kthread_stop(monitor_thread);

	put_pid(tm->pid);
	kfree(tm);

	kobject_put(tm_obj);
	sysfs_remove_file(kernel_kobj, &tm_attr.attr);
	run = false;

	pr_info("Monitoring module unloaded\n");
}

module_init(monitor_init);
module_exit(monitor_exit);
