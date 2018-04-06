#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <asm/io.h>
/* for the structure */
#include <linux/utsname.h>

/* string.h redefinition */

MODULE_DESCRIPTION("Module \"uname\" pour noyau linux");
MODULE_AUTHOR("William Fabre, LIP6");
MODULE_LICENSE("GPL");

static char *kernel_name= "m_arch-alain";
module_param_named(my_name, kernel_name, charp, 0644);
MODULE_PARM_DESC(my_name, " Who are you?");

/* trouvable dans include/uapi/linux/utsname.h */ 
char save[__NEW_UTS_LEN + 1];

/* pas obligatoire on a include */
extern struct uts_namespace init_uts_ns;

static int __init hello_init(void)
{
	/* save */
	strcpy(save, init_uts_ns.name.sysname);
	/* change */
	strcpy(init_uts_ns.name.sysname, kernel_name);
	//EXPORT_SYMBOL_GPL(init_uts_ns);
	pr_info("Changing the output of uname \n");

	return 0;
}
module_init(hello_init);

static void __exit hello_exit(void)
{
	/* restore */
	strcpy(init_uts_ns.name.sysname, save);
	pr_info("Changing the output of uname finished\n");
}
module_exit(hello_exit);


/* https://stackoverflow.com/questions/1184274/how-to-read-write-files-within-a-linux-kernel-module */

/********************************/
/** We need to define some I/O **/
/********************************/

/* IO */
/* #include <asm/segment.h>*/
/* #include <asm/uaccess.h>*/
/* #include <linux/buffer_head.h>*/



/* struct file *file_open(const char *path, int flags, int rights)*/
/* {*/
/*     struct file *filp = NULL;*/
/*     mm_segment_t oldfs;*/
/*     int err = 0;*/

/*     oldfs = get_fs();*/
/*     set_fs(get_ds());*/
/*     filp = filp_open(path, flags, rights);*/
/*     set_fs(oldfs);*/
/*     if (IS_ERR(filp)) {*/
/*         err = PTR_ERR(filp);*/
/*         return NULL;*/
/*     }*/
/*     return filp;*/
/* }*/

/* void file_close(struct file *file)*/
/* {*/
/*     filp_close(file, NULL);*/
/* }*/

/* int file_read(struct file *file, unsigned long long offset, unsigned char *data,*/
/*                 unsigned int size) */
/* {*/
/*     mm_segment_t oldfs;*/
/*     int ret;*/

/*     oldfs = get_fs();*/
/*     set_fs(get_ds());*/

/*     ret = vfs_read(file, data, size, &offset);*/

/*     set_fs(oldfs);*/
/*     return ret;*/
/* }*/

/* int file_write(struct file *file, unsigned long long offset,*/
/*                 unsigned char *data, unsigned int size) */
/* {*/
/*     mm_segment_t oldfs;*/
/*     int ret;*/

/*     oldfs = get_fs();*/
/*     set_fs(get_ds());*/

/*     ret = vfs_write(file, data, size, &offset);*/

/*     set_fs(oldfs);*/
/*     return ret;*/
/* }*/

/* int file_sync(struct file *file) */
/* {*/
/*     vfs_fsync(file, 0);*/
/*     return 0;*/
/* }*/
