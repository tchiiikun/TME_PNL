#include<stdio.h>
#include<signal.h>

#include"func.h"

ssize_t read(int fd, void* buf, size_t nbyte)
{
	char* s = (char*)buf;
	s[0] = 'e';
	printf("Tchao !!!\n");
	return nbyte;
}
