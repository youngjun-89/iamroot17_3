#include <stdio.h>
#include "ioctl.h"
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#define DEVFILENAME "/dev/iamroot"
 
#define IOCTL_CODE _IOR(0, IOCTL_PID, int)

int main(void)
{
	int fd;
	int ret;
	int result = -1;

	fd = open(DEVFILENAME, O_RDWR);
	
	if (fd == -1) {
			perror("open device file error");
			return -1;
	}
	
	if (ioctl(fd, IOCTL_CODE, &result)) {
		perror("ioctl error");
		ret = -1;
	} else
			printf("ioctl success pid:%d\n", result);

	close(fd);
	return ret;
}
