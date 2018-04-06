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

static char *my_type= "proc";
module_param(my_type, charp, 0644);
MODULE_PARM_DESC(my_type, " The type of the Structure to iterate over\n");

void my_print_block(struct super_block*s_b, void *unknow);

void block_stamp(struct super_block *s_b, void *unknow)
{
	if (!s_b->date.tv_nsec || !s_b->date.tv_sec){
		getnstimeofday(&s_b->date);
	}
	my_print_block(s_b, NULL);

	return;
}


static int __init hello_init(void)
{
	pr_info("Polymorphic SuperBloc\n");
	iterate_supers_type(get_fs_type(my_type), block_stamp, NULL);


	return 0;
}
module_init(hello_init);


static void __exit hello_exit(void)
{
	pr_info("End Polymorphic SuperBloc\n");
}
module_exit(hello_exit);


void my_print_block(struct super_block *my_super_block, void *unknow)
{
	pr_info("uuid= %.2x%.2x%.2x%.2x %.2x%.2x %.2x%.2x %.2x%.2x %.2x%.2x %.2x%.2x%.2x%.2x temps=%ld.%ld type=%s\n",
			my_super_block->s_uuid[0],
			my_super_block->s_uuid[1],
			my_super_block->s_uuid[2],
			my_super_block->s_uuid[3],
			my_super_block->s_uuid[4],
			my_super_block->s_uuid[5],
			my_super_block->s_uuid[6],
			my_super_block->s_uuid[7],
			my_super_block->s_uuid[8],
			my_super_block->s_uuid[9],
			my_super_block->s_uuid[10],
			my_super_block->s_uuid[11],
			my_super_block->s_uuid[12],
			my_super_block->s_uuid[13],
			my_super_block->s_uuid[14],
			my_super_block->s_uuid[15],
			my_super_block->date.tv_sec,
			my_super_block->date.tv_nsec,
			my_super_block->s_type->name);
	return;
}
