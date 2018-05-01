struct task_sample {
	struct list_head list; 	// 16
	cputime_t utime;	// 8
	struct kref refcount;
	cputime_t stime;	// 8
};

struct task_monitor {
	struct list_head tm_list;
	struct task_sample head;
	struct mutex lock;
	struct pid *pid;
	int nb_samples;
};

struct my_tasks{
	struct task_monitor tm_head;
};
