#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include "pnlfs.h"

MODULE_LICENSE("GPL");

// struct file_system_type* p;
//
// TODO implementer le register, unregister fs.
// verifier dans /proc/filesystems s'il est present
//
/* struct file_system_type {*/
/* const char *name;*/
/* int fs_flags;*/
/* struct dentry *(*mount) (struct file_system_type *, int,*/
/* const char *, void *);*/
/* void (*kill_sb) (struct super_block *);*/
/* struct module *owner;*/
/* struct file_system_type *next;*/
/* };*/

static int monitor_init(void)
{
	pr_info("pnlfs init");
	// register_filesystem()
	return 0;
}
module_init(monitor_init);

static void monitor_exit(void)
{
	// unregister_filesystem()
	pr_info("pnlfs exit");
}

module_exit(monitor_exit);
