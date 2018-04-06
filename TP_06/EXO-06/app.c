#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "helloioctl.h"

int main()
{
	int fd;
	int32_t value, number;
	char string[256];
	struct task_sample* ts_info;
	ts_info = malloc(sizeof(ts_info));

	printf("*********************************\n");
	printf("*******marcalain*******\n");

	printf("\nOpening Driver\n");
	fd = open("/dev/taskmonitor", O_RDWR);
	if(fd < 0) {
		printf("Cannot open device file...\n");
		return 0;
	}

	/* printf("Enter the Value to send\n");*/
	/* scanf("%d",&number);*/
	/* printf("Writing Value to Driver\n");*/
	/* ioctl(fd, WR_VALUE, (int32_t*) &number);*/

	/* printf("Reading Value from Driver\n");*/
	/* ioctl(fd, RD_VALUE, (int32_t*) &value);*/
	/* printf("Value is %d\n", value);*/

	/* printf("Reading Value Hello from Driver\n");*/
	/* ioctl(fd, HELLO, (char*) string);*/
	/* printf("Value is %s\n", string);*/

	/* printf("Enter the Value to send to WHO\n");*/
	/* scanf("%s",&string);*/
	/* printf("Writing Value to Driver\n");*/
	/* ioctl(fd, WHO, (char*) &string); */

	printf("Reading value from string in Driver\n");
	ioctl(fd, GET_SAMPLE, (char*) string);
	printf(string);

	printf("Reading Value from struct in Driver\n");
	ioctl(fd, GET_SAMPLE_STRUCT, (struct task_sample*) ts_info);
	printf("usr %lu sys %lu\n", ts_info->utime, ts_info->stime);

	sleep(5);
	printf("Reading Value from struct in Driver to trigger stop thread\n");
	ioctl(fd, TASKMON_STOP, NULL);

	sleep(5);
	printf("Reading Value from struct in Driver to trigger start thread\n");
	ioctl(fd, TASKMON_START, NULL);

	printf("Enter the Value to send\n");
	scanf("%d", &number);
	printf("Writing Value to Driver\n");
	ioctl(fd, TASKMON_SET_PID, (int32_t*) &number);



	printf("Closing Driver\n");
	close(fd);
}
