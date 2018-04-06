struct task_sample {
	long unsigned int utime;
	long unsigned int stime;
};

#define WR_VALUE 		_IOW('l','l',int32_t*)
#define RD_VALUE 		_IOR('a','a',int32_t*)
#define HELLO    		_IOR('r','r',char**)
#define WHO			_IOW('e','e',char**)
#define GET_SAMPLE 		_IOR('p','p',char**)
#define GET_SAMPLE_STRUCT 	_IOR('o','o', struct task_sample*)
#define TASKMON_STOP 		_IOR('n','n', struct task_sample*)
#define TASKMON_START 		_IOR('4','4', struct task_sample*)
#define TASKMON_SET_PID 	_IOR('2','2', struct task_sample*)

