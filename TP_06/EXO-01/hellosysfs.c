#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <asm/io.h>
#include<linux/sysfs.h>
#include<linux/kobject.h> 

MODULE_DESCRIPTION("Module \"hello word\" pour noyau linux");
MODULE_AUTHOR("William Fabre, LIP6");
MODULE_LICENSE("GPL");


char *my_name = "my_attribute";
char my_string[256];
struct kobject *my_kobj;

/* SHOW */
static ssize_t sysfs_show(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf)
{
	pr_info("Hello World %s\n", my_string);
	return 0;
}

/* STORE */
static ssize_t sysfs_store(struct kobject *kobj,
		struct kobj_attribute *attr, const char *buf, size_t count)
{
	pr_info("Storing something");
	return strlen(strncpy(my_string, buf, 255));
}

/* ATTR */
struct kobj_attribute my_attribute =
	__ATTR(my_name, 0644, sysfs_show, sysfs_store);

static int __init hello_init(void)
{
	pr_info("Hello, world\n");

	/*Creating a directory in /sys/kernel/ */
	my_kobj = kobject_create_and_add("my_helloworld", kernel_kobj);
	/*Creating sysfs file for etx_value*/
	if(sysfs_create_file(my_kobj, &my_attribute.attr)){
		printk(KERN_INFO"Cannot create sysfs file\n");
		goto r_sysfs;
	}

	return 0;
r_sysfs:
	kobject_put(my_kobj);
	return 0;

}
module_init(hello_init);

static void __exit hello_exit(void)
{
	pr_info("Goodbye, cruel world\n");
	/* DESTRUCTION */
	kobject_put(my_kobj);
	sysfs_remove_file(kernel_kobj, &my_attribute.attr);
}
module_exit(hello_exit);
