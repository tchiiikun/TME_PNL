/* store function */

static ssize_t sysfs_store(struct kobject *kobj,
		struct kobj_attribute *attr, const char *buf, size_t count);
/* show function */
static ssize_t sysfs_show(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf);
