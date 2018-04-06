#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <asm/io.h>

MODULE_DESCRIPTION("Module \"hello word\" pour noyau linux");
MODULE_AUTHOR("William Fabre, LIP6");
MODULE_LICENSE("GPL");

static char *whom = "Tchi";
module_param_named(my_little_whom, whom, charp, 0644);
MODULE_PARM_DESC(my_little_whom, " Who are you?");

/* static int answer = 42 ;*/
/* module_param_named(readable_answer, answer, int, 0444);*/
/* MODULE_PARM_DESC(readable_answer, "The readable answer");*/

static int howmany;
module_param(howmany, int, 0644);
MODULE_PARM_DESC(howmany, " How many do you want?");


static int __init hello_init(void)
{
	pr_info("Hello, world\n");
	printk(" You are : %s\n", whom);
	printk(" You have : %d\n", howmany);
	return 0;
}
module_init(hello_init);

static void __exit hello_exit(void)
{
	pr_info("Goodbye, cruel world\n");
	printk(" You are : %d\n", howmany);
	printk(" You have : %s\n", whom);
}
module_exit(hello_exit);
