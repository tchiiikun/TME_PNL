#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <asm/io.h>
/* for the function to iterate and the structure */
#include <linux/fs.h>
/* for the list of time */
#include <linux/types.h>
/* time */
/* getnstimeofday*/
#include <linux/timekeeping.h>
/* kmalloc */
#include <linux/slab.h>
/* vmalloc */
#include <linux/vmalloc.h>

MODULE_DESCRIPTION("Module \"update_sb\" pour noyau linux");
MODULE_AUTHOR("William Fabre, LIP6");
MODULE_LICENSE("GPL");

#define container_of_tchi(ptr, type, member) ({                      \
		const typeof( ((type *)0)->member ) __mptr = (ptr);    \
		(type *)( (char *)__mptr - offsetof(type,member) );})

static char *my_type= "proc";
module_param(my_type, charp, 0644);
MODULE_PARM_DESC(my_type, " The type of the Structure to iterate over\n");

struct uuid_list {
	struct super_block *my_super_block;
	struct timespec *date;
	struct list_head *list;
};
struct uuid_list* my_list;
EXPORT_SYMBOL(my_list);

void my_print_block(struct uuid_list *s_b, void *unknow);
struct uuid_list* uuid_list_allocation(struct uuid_list* l);

void block_stamp(struct super_block *s_b, void *unknow)
{
	struct uuid_list *tmp;

	getnstimeofday(my_list->date);
	my_list->my_super_block = s_b;
	tmp = uuid_list_allocation(tmp);
	getnstimeofday(tmp->date);
	list_add((tmp->list), (my_list->list));
}


static int __init hello_init(void)
{
	struct list_head *pos;
	struct uuid_list *elem;

	pr_info("Polymorphic SuperBloc\n");
	if (my_list->list->next == NULL){
		pr_info("LA LISTE EST VIDE\n");
		my_list = uuid_list_allocation(my_list);
		INIT_LIST_HEAD(my_list->list);
		iterate_supers_type(get_fs_type(my_type), block_stamp, NULL);
	}

	list_for_each(pos, my_list->list)
	{
		elem = container_of_tchi(pos, struct uuid_list, list);
		my_print_block(elem, NULL);
	}

	return 0;
}
module_init(hello_init);


static void __exit hello_exit(void)
{
	struct list_head *pos;
	struct list_head *q;
	struct uuid_list *elem;

	pr_info("End Polymorphic SuperBloc\n");
	/* list_for_each_safe(pos, q, my_list->list)*/
	/* {*/
	/*         elem = container_of_tchi(pos, struct uuid_list, list);*/
	/*         list_del(pos);*/
	/*         kfree(elem);*/
	/* }*/
}
module_exit(hello_exit);


void my_print_block(struct uuid_list *toto, void *unknow)
{
	pr_info("uuid= %.2x%.2x%.2x%.2x %.2x%.2x %.2x%.2x %.2x%.2x %.2x%.2x %.2x%.2x%.2x%.2x temps=%ld.%ld type=%s\n",
			my_list->my_super_block->s_uuid[0],
			my_list->my_super_block->s_uuid[1],
			my_list->my_super_block->s_uuid[2],
			my_list->my_super_block->s_uuid[3],
			my_list->my_super_block->s_uuid[4],
			my_list->my_super_block->s_uuid[5],
			my_list->my_super_block->s_uuid[6],
			my_list->my_super_block->s_uuid[7],
			my_list->my_super_block->s_uuid[8],
			my_list->my_super_block->s_uuid[9],
			my_list->my_super_block->s_uuid[10],
			my_list->my_super_block->s_uuid[11],
			my_list->my_super_block->s_uuid[12],
			my_list->my_super_block->s_uuid[13],
			my_list->my_super_block->s_uuid[14],
			my_list->my_super_block->s_uuid[15],
			my_list->date->tv_sec,
			my_list->date->tv_nsec,
			my_list->my_super_block->s_type->name);
	return;
}

struct uuid_list* uuid_list_allocation(struct uuid_list* l)
{
	int j;
	if ((l = kmalloc(sizeof(struct uuid_list), GFP_KERNEL)) == NULL){
		pr_err("allocation problem\n");
	}
	if ((l->date = kmalloc(sizeof(struct timespec), GFP_KERNEL)) == NULL){
		pr_info("allocation problem");
	}	
	if ((l->date->tv_sec = kmalloc(sizeof(__kernel_time_t), GFP_KERNEL)) == NULL){
		pr_info("allocation problem");
	}
	if ((l->date->tv_nsec = kmalloc(sizeof(long), GFP_KERNEL)) == NULL){
		pr_info("allocation problem");
	}
	if ((l->list = kmalloc(sizeof(struct list_head), GFP_KERNEL)) == NULL){
		pr_info("allocation problem");
	}
	if ((l->list->next = kmalloc(sizeof(struct list_head), GFP_KERNEL)) == NULL){
		pr_info("allocation problem");
	}
	if ((l->list->prev = kmalloc(sizeof(struct list_head), GFP_KERNEL)) == NULL){
		pr_info("allocation problem");
	}
	if ((l->my_super_block = kmalloc(sizeof(struct super_block), GFP_KERNEL)) == NULL){
		pr_info("allocation problem");
	}

	return l;
}


