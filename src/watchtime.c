#include <stdio.h>
#include <time.h>
#include <string.h>
#include <mqueue.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <errno.h>
#include "constants.h"

pid_t spawnTimeWatcher()
{
	pid_t pid;
	
	pid = fork();
		
	if(pid > 0)
	{
		return pid;
	}
	else if(pid == 0)
	{
		struct tm transferTime;
		transferTime.tm_hour = 1;
		transferTime.tm_min = 0;
		transferTime.tm_sec = 0;	
		watchTime(transferTime);
	}
	else
	{
		return -1;	
	};
};

int watchTime(struct tm targetTime)
{
	mqd_t commandQueue;
	commandQueue = mq_open("/sitemanagerdCMDQ", O_WRONLY);
	int retval = mq_send(commandQueue, WTCHECKIN, CMDQMAXLEN, 0);
	
	if(retval == -1)
	{
		syslog(LOG_ERR, "Watch time could not check in");	
	}
	else
	{
		syslog(LOG_DEBUG, "Watch time check in sent");	
	};
	
	if(commandQueue == -1)
	{
		syslog(LOG_ERR, "watchTime() : Could not open queue %s", strerror(errno));	
		exit(-1);
	}
	
	syslog(LOG_INFO, "Backup and transfer scheduled for %d:%d:%d", targetTime.tm_hour, targetTime.tm_min, targetTime.tm_sec);

	while(1)
	{
		time_t now;
		time(&now);
		struct tm current;
		current = *localtime(&now);

		int hourdif = targetTime.tm_hour - current.tm_hour;

		if(hourdif < 0)
		{
			hourdif = 24 + hourdif;	
		};

		if(hourdif == 0)
		{
			int mindif = targetTime.tm_min - current.tm_min;	
			if(mindif < 0)
			{
				mindif = 60 + mindif;	
			};

			if(mindif == 0)
			{
				int secdif = targetTime.tm_sec - current.tm_sec;

				if(secdif < 0)
				{
					secdif = 60 + secdif;	
				};

				if(secdif == 0)
				{
					int retval = mq_send(commandQueue, BEGINBANDT, CMDQMAXLEN, 0);

					if(retval == -1)
					{
						syslog(LOG_ERR, "watchTime() : Message could not be sent %s", strerror(errno));	
					}
					else
					{
						syslog(LOG_DEBUG, "Backup message sent");	
					}
				}
			};		
		};
		
		sleep(1);
	}
};

