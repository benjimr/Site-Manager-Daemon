#ifndef ISALREADYRUNNING_H_
#define ISALREADYRUNNING_H_

bool isalreadyrunning(char name[]);
char* getPids(char name[]);
bool killAll(char name[]);

#endif