#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <asm/io.h>

#include <linux/proc_fs.h>
#include <linux/cred.h>
#include <linux/string.h>


MODULE_DESCRIPTION("Module \"hello word\" pour noyau linux");
MODULE_AUTHOR("William Fabre, LIP6");
MODULE_LICENSE("GPL");

static int __init hello_init(void)
{
	pr_info("Hello, world\n");
	return 0;
}
module_init(hello_init);

static void __exit hello_exit(void)
{
	printk(" You are : %s", whom);
}
module_init(hello_exit);
