#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

bool daemonize()
{
	umask(0);
		
	pid_t sid;
	sid = setsid();

	if(sid < 0)
	{
		exit(EXIT_FAILURE);
	};

	int retval = chdir("/");

	if(retval < 0)
	{
		exit(EXIT_FAILURE);
	};

	for(int fd = sysconf(_SC_OPEN_MAX); fd >= 0; fd--)
	{
		close(fd);	
	};
	
	return true;
};