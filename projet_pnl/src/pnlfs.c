#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include "pnlfs.h"

MODULE_LICENSE("GPL");

/* struct file_system_type* fst_pnl;*/

/* TODO implementer le register, unregister fs.*/
/* verifier dans /proc/filesystems s'il est present*/

/* Implementer mount :*/
/* struct dentry* mount(struct file_system_type *fs_type,*/
/*                 int flags, const char *dev_name, void *data);*/

static int monitor_init(void)
{
	pr_info("pnlfs init");
	/* register_filesystemregister_filesystem(struct file_system_type *);*/

	return 0;
}
module_init(monitor_init);

static void monitor_exit(void)
{
	/* unregister_filesystem(struct file_system_type *);*/
	pr_info("pnlfs exit");
}

module_exit(monitor_exit);


