#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <syslog.h>

char* getPids(char name[])
{
	int namelen = strlen(name);
	
	if(namelen == 0)
	{
		return false;
	}
	else
	{
		int maxlen = 50;
		char command[maxlen];
		char part1[] = "ps -e | grep";
		char part2[] = "| awk \'{print $1}\'";
		
		snprintf(command, maxlen, "%s %s %s", part1, name, part2);
		
		FILE *fp = popen(command, "r");

		if(fp == NULL)
		{
			return NULL;
		}
		else
		{
			int filesize = ftell(fp);
			char *buf = malloc(filesize+1);
			fread(buf, filesize, 1, fp);
			return buf;
		};
	};
}

bool isalreadyrunning(char name[])
{	
	char *buf = getPids(name);
	int filelen = strlen(buf);
	syslog(LOG_INFO, "%s", buf);

	char *token = strtok(buf, "\n\r");
	pid_t pid;	
		
	int counter = 1;
	while(token != NULL)
 	{
		counter++;
		token = strtok(NULL, "\n\r");
	};
	
	
	if(counter > 4)
	{
		syslog(LOG_DEBUG, "Daemon already running");
		return true;
	}
	else
	{
		syslog(LOG_DEBUG, "Daemon not running, yet");
		return false;
	};
};

bool killAll(char name[])
{
	char *buf = getPids(name);
	
	char *token = strtok(buf, "\n\r");
	pid_t pid;	
		
	while(token != NULL)
 	{
		pid = atoi(token);
		kill(pid, SIGTERM);
		token = strtok(NULL, "\n\r");
	};
						 
	return true;
}
						 
						 
						 
						 
						 
						 
						 
						 
						 
						 
						 
						 
						 
						 
						 
						 