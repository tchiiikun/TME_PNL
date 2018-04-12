#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/dcache.h>

#define MAX(a,b) a > b ? a : b\

MODULE_DESCRIPTION("Module Weasel pour noyau linux");
MODULE_AUTHOR("William Fabre, LIP6");
MODULE_LICENSE("GPL");

static int __init hello_init(void)
{
	struct hlist_bl_node *node;
	struct dentry *entry;
	int i;
	int entry_size;
	int cpt = 0;
	int maximum = 0;

	pr_info("Weasel\n");
	printk("d_hash_shift value : %d\n", d_hash_shift);
	printk("list size : %d\n", 1 << d_hash_shift);
	printk("table adress : %p\n", dentry_hashtable);

	hlist_bl_lock(dentry_hashtable);
	for (i = 0; i < 1 << d_hash_shift; i++) {
		hlist_bl_for_each_entry(entry, node, dentry_hashtable+i,
				d_hash) {
			entry_size++;
			cpt++;
		}
		maximum = MAX(maximum, entry_size);
		entry_size = 0;
	}
	entry = NULL;
	hlist_bl_unlock(dentry_hashtable);

	printk("value : %d\n", cpt);
	printk("size max in the list : %d\n", maximum);
	return 0;
}
module_init(hello_init);

static void __exit hello_exit(void)
{
	pr_info("Goodbye, cruel world\n");
}
module_exit(hello_exit);
