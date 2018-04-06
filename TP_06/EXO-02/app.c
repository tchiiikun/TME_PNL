#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include<sys/ioctl.h>

#include "helloioctl.h"

int main()
{
	int fd;
	int32_t value, number;
	char *string;
	printf("*********************************\n");
	printf("*******WWW.EmbeTronicX.com*******\n");

	printf("\nOpening Driver\n");
	fd = open("/dev/etx_device", O_RDWR);
	if(fd < 0) {
		printf("Cannot open device file...\n");
		return 0;
	}

	printf("Enter the Value to send\n");
	scanf("%d",&number);
	printf("Writing Value to Driver\n");
	ioctl(fd, WR_VALUE, (int32_t*) &number); 

	printf("Reading Value from Driver\n");
	ioctl(fd, RD_VALUE, (int32_t*) &value);
	printf("Value is %d\n", value);

	printf("Reading Value Hello from Driver\n");
	ioctl(fd, HELLO, (char*) string);
	printf("Value is %s\n", string);

	printf("Enter the Value to send to WHO\n");
	scanf("%s",&string);
	printf("Writing Value to Driver\n");
	ioctl(fd, WHO, (char*) &string); 


	printf("Closing Driver\n");
	close(fd);
}
