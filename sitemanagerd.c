#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <mqueue.h>
#include <signal.h>
#include "isalreadyrunning.h"
#include "watchtime.h"
#include "constants.h"
#include "bandt.h"
#include "daemonize.h"

mqd_t setupQueue()
{
	mqd_t q;
	struct mq_attr attr;

	attr.mq_flags = 0;
	attr.mq_maxmsg = 10;
	attr.mq_msgsize = CMDQMAXLEN;
	attr.mq_curmsgs = 0;

	q = mq_open("/sitemanagerdCMDQ", O_CREAT | O_RDONLY, 0644, &attr);
	
	if(q == -1)
	{
		syslog(LOG_ERR, "Could not setup queue");
		exit(EXIT_FAILURE);
	};
	
	return q;
};

bool backup()
{
	pid_t pid;

	pid = fork();

	if(pid > 0)
	{
		return true;
	}
	else if(pid == 0)//first fork backup
	{
		executeUpdate("/var/www/html/", "/var/www/backup");
	}
	else
	{
		exit(EXIT_FAILURE);	
	};
}

bool transfer()
{
	pid_t pid;
	
	pid = fork();

	if(pid > 0)
	{
		return true;
	}
	else if(pid == 0)// second fork Transfer
	{
		executeUpdate("/var/www/html/intranet/", "/var/www/html/live/");
	}
	else
	{
		exit(EXIT_FAILURE);
	};
};

void signalHandler(int signal)
{
	if(signal == SIGINT || signal == SIGTERM)
	{
		syslog(LOG_INFO, "Interrupt/Termination signal received. Exiting...");
		exit(EXIT_SUCCESS);
	}
};

bool beginAuditer()
{
	pid_t pid;
	
	pid = fork();
	
	if(pid > 0)
	{
		return true;	
	}
	else if(pid == 0)
	{
		char* args[] = {"auditctl", "-w",  "/var/www/" "-p" "rwxa", NULL};
		execvp(args[0], args);
		syslog(LOG_ERR, "Failed to start auditer");
		exit(EXIT_FAILURE);
	}
	else
	{
		exit(EXIT_FAILURE);	
	}
}

bool updateAuditLogs()
{
	pid_t pid;
	
	pid = fork();
	
	if(pid > 0)
	{
		return true;
	}
	else if (pid == 0)
	{
		char* args[] = {"ausearch", "-f", "/var/www/", "--interpret", ">>", "/var/www/html/accesslog.txt", NULL};
		execvp(args[0], args);
		syslog(LOG_ERR, "Failed to update audit logs");
	}
	else
	{
		exit(EXIT_FAILURE);	
	}
}

int main(int argc, char** argv)
{	
	int option;
	if(argc == 1)
	{
		option = 0; // assume start if not specified
	}
	else
	{
		option = atoi(argv[1]);	
	};
	
	
	char *name;
	name = argv[0];
	name += 10; // removing ./
	bool running = isalreadyrunning(name);
	
	syslog(LOG_INFO, "NAME %s", name);
	
	switch(option)
	{
		case 0: //start
		{
			if(running == true)
			{
				syslog(LOG_ERR, "Daemon already running");	
				exit(EXIT_FAILURE);
			};
			break;		
		}
		case 1: //stop
		{
			if(running  == false)
			{
				syslog(LOG_ERR, "Daemon not running");	
				exit(EXIT_FAILURE);
			}
			
			killAll(name);
			break;
		}
		case 2: //force backup
		{
			bool backingUp = backup();
			exit(EXIT_SUCCESS);
		}	
		case 3: //force transfer
 		{
			bool transfering = transfer();
			exit(EXIT_SUCCESS);
		};
		case 4: //force audit log update
		{
			bool updating = updateAuditLogs();
			exit(EXIT_SUCCESS);
		}
	};
	
	pid_t pid;

	pid = fork();
	
	if(pid > 0) //parent
	{
		printf("Daemon started\n");
		exit(EXIT_SUCCESS);
	}
	else if(pid == 0) //child
	{
		//basic daemon setup
		bool ready = daemonize();
		
		if(!ready)
		{
			exit(EXIT_FAILURE);
		};

		//get syslog ready
		openlog(NULL, LOG_NDELAY | LOG_PID, LOG_DAEMON);
		
		//signal handlers
		signal(SIGINT, signalHandler);
		signal(SIGTERM, signalHandler);
		
		
		//prep queue to receive commands/info
		mqd_t commandQueue;
		commandQueue = setupQueue();
		
		int watcherID = spawnTimeWatcher();
		
		beginAuditer();
		
		if(watcherID != -1)
		{
			char *buf;
			buf = (char *) malloc(CMDQMAXLEN + 1);

			while(1)
			{
				ssize_t amt_read;

				syslog(LOG_DEBUG, "Waiting for message");
				amt_read = mq_receive(commandQueue, buf, CMDQMAXLEN, NULL);
												
				if(strcmp(buf, BEGINBANDT) == 0)
			   	{
					bool backingUp = backup();
					bool transfering = transfer();
					bool updating = updateAuditLogs();
					
					if(backingUp)
					{
						syslog(LOG_INFO, "Backup started");
					};
					
					if(transfering)
					{
						syslog(LOG_INFO, "Transfer started");
					};	
					
					if(updating)
					{
						syslog(LOG_INFO, "Updating logs");	
					}
			   	}
				else if(strcmp(buf, UPDATESUCCESS) == 0)
				{
					syslog(LOG_INFO, "Update Successful");
				}
				else if(strcmp(buf, UPDATEFAILED) == 0)
				{
					syslog(LOG_ERR, "Update Failed");	
				}
				else if(strcmp(buf, WTCHECKIN) == 0)
				{
					syslog(LOG_INFO, "Watch time child checked in");	
				}
				else
				{
					syslog(LOG_ERR, "Unknown message on queue %s", buf);
				};	
			};	
		};		
	}
	else //failed
	{
		exit(EXIT_FAILURE);
	};
};