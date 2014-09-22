#ifndef _DOGSTATSD_H
#define _DOGSTATSD_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#define BUFFSIZE 2048
#define PORT 8125

void startServer();

int getUntil(char *string, char pattern, char* result);

void setUnknowMetric(char *metric[5], char *unknownMetric);

void initMetric(char *metric[5], int lineSize);

#endif