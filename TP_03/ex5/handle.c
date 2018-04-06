#define _GNU_SOURCE
#include <dlfcn.h>
#include <unistd.h>
#include <string.h>
#include "func.h"

ssize_t read(int fd, void *buf, size_t count)
{
	char c;
	char i = 'i';
	ssize_t (*true_read)(int, void *, size_t);
	*(void **) (&true_read) = dlsym(RTLD_DEFAULT, "read");
	*(void **) (&true_read) = dlsym(RTLD_NEXT, "read");
	(*true_read)(fd, &c, count);
	if(c == 'r')
		memcpy(buf, (void*) &i, 1);
	else
		memcpy(buf, (void*) &c, 1);
	return count;
}
