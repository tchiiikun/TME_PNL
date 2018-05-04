### Explications
	
## Struct file_system_type

	struct file_system_type {
		const char *name;
		int fs_flags;
        	struct dentry *(*mount) (struct file_system_type *, int,
        	               const char *, void *);
        	void (*kill_sb) (struct super_block *);
        	struct module *owner;
        	struct file_system_type * next;
        	struct list_head fs_supers;
		struct lock_class_key s_lock_key;
		struct lock_class_key s_umount_key;
	};

	// TODO
	#include <linux/fs.h>
	register_filesystemregister_filesystem(struct file_system_type *);
	unregister_filesystem(struct file_system_type *);

*  name: the name of the filesystem type, such as "ext2", "iso9660", "msdos"
   and so on 

*  fs_flags: various flags (i.e. FS_REQUIRES_DEV, FS_NO_DCACHE, etc.)
mount: the method to call when a new instance of this filesystem should be
mounted 

*  kill_sb: the method to call when an instance of this filesystem
should be shut down

*  owner: for internal VFS use: you should initialize this to THIS_MODULE in
most cases.

*  next: for internal VFS use: you should initialize this to NULL

*  s_lock_key, s_umount_key: lockdep-specific

*  The mount() method has the following arguments:

*  struct file_system_type *fs_type: describes the filesystem, partly initialized
by the specific filesystem code

*  int flags: mount flags

*  const char *dev_name: the device name we are mounting.

*  void *data: arbitrary mount options, usually comes as an ASCII
	string (see "Mount Options" section)






* Usually, a filesystem uses one of the generic mount() implementations
and provides a fill_super() callback instead. The generic variants are:

*  mount_bdev: mount a filesystem residing on a block device

*  mount_nodev: mount a filesystem that is not backed by a device

*  mount_single: mount a filesystem which shares the instance between
  	all mounts

* A fill_super() callback implementation has the following arguments:

*  struct super_block *sb: the superblock structure. The callback
  	must initialize this properly.

*  void *data: arbitrary mount options, usually comes as an ASCII
	string (see "Mount Options" section)

*  int silent: whether or not to be silent on error

