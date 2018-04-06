#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/slab.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include<linux/uaccess.h>              //copy_to/from_user()
#include <linux/ioctl.h>

#include "helloioctl.h"

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

int32_t value = 0;
char* string = "toto";
char* info = "toto";
struct task_sample ts_info;
/* dev_t dev = 0;*/
/* static struct class *dev_class;*/
/* static struct cdev etx_cdev;*/

bool run;
int major;

static int monitor_init(void);
static void monitor_exit(void);
static int etx_open(struct inode *inode, struct file *file);
static int etx_release(struct inode *inode, struct file *file);
static ssize_t etx_read(struct file *filp, char __user *buf,
		size_t len,loff_t * off);
static ssize_t etx_write(struct file *filp, const char *buf,
		size_t len, loff_t * off);
static long etx_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

static struct file_operations fops =
{
	.owner          = THIS_MODULE,
	.read           = etx_read,
	.write          = etx_write,
	.open           = etx_open,
	.unlocked_ioctl = etx_ioctl,
	.release        = etx_release,
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
		pr_err("%hd is dead\n",	pid);
	else
		pr_info("%hd usr %lu sys %lu\n", pid, ts.utime, ts.stime);
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

static int etx_open(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "Device File Opened...!!!\n");
	return 0;
}

static int etx_release(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "Device File Closed...!!!\n");
	return 0;
}

static ssize_t etx_read(struct file *filp, char __user *buf,
		size_t len, loff_t *off)
{
	printk(KERN_INFO "Read Function\n");
	return 0;
}
static ssize_t etx_write(struct file *filp, const char __user *buf,
		size_t len, loff_t *off)
{
	printk(KERN_INFO "Write function\n");
	return 0;
}

static long etx_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct task_sample ts;
	bool alive;
	int res = 0;
	int err;

	switch(cmd) {
		case WR_VALUE:
			res = copy_from_user(&value ,(int32_t*) arg, sizeof(value));
			printk(KERN_INFO "Value = %d\n", value);
			break;
		case RD_VALUE:
			res = copy_to_user((int32_t*) arg, &value, sizeof(value));
			break;
		case HELLO:
			res = copy_to_user((char*) arg, string, strlen(string));
			break;
		case WHO:
			res = copy_from_user(string, (char*) arg, sizeof(string));
			printk(KERN_INFO "Value = %s\n", string);
			break;
		case GET_SAMPLE:
			pid_nr(tm->pid);
			alive = get_sample(tm, &ts);
			snprintf(info, 256, "usr %lu sys %lu\n",
					ts.utime, ts.stime);
			res = copy_to_user((char*) arg, info, strlen(info));
			break;
		case GET_SAMPLE_STRUCT:
			get_sample(tm, &ts_info);
			res = copy_to_user((struct task_sample*) arg,
					&ts_info,
					sizeof(u64)*2);
			break;
		case TASKMON_STOP:
			if (run == true){
				kthread_stop(monitor_thread);
				run = false;
				pr_info("thread Stopped");
			}
			break;
		case TASKMON_START:
			if (run == false){
				monitor_thread = kthread_run(monitor_fn,
						NULL, "monitor_fn");
				run = true;
				pr_info("thread Started");
			}
			break;
		case TASKMON_SET_PID:
			kthread_stop(monitor_thread);
			put_pid(tm->pid);
			kfree(tm);
			res = copy_from_user(&value ,
					(int32_t*) arg, sizeof(value));
			err = monitor_pid(value);
			if (err){
				pr_info("WRONG PID");
				put_pid(tm->pid);
				kfree(tm);
			} else {
				target = value;
				monitor_thread = kthread_run(monitor_fn,
						NULL, "monitor_fn");
			}
			break;
	}
	return res;
}

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

	run = true;
	major = register_chrdev(0, "taskmonitor", &fops);
	pr_info("%d", major);
	printk(KERN_INFO "Device Driver Insert...Done!!!\n");
	return 0;

abort:
	put_pid(tm->pid);
	kfree(tm);
	return err;
}
module_init(monitor_init);

static void monitor_exit(void)
{
	if (monitor_thread)
		kthread_stop(monitor_thread);
	unregister_chrdev(major, "taskmonitor");
	printk(KERN_INFO "Device Driver Remove...Done!!!\n");

	put_pid(tm->pid);
	kfree(tm);
	pr_info("Monitoring module unloaded\n");
}
module_exit(monitor_exit);
