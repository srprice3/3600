#ifndef SIMHTTP_H
#define SIMHTTP_H

/*********************************************************
*
* Stephen Price   CpSc3600  HW3
* Dr. Remy Sokou, Fall 2014
* File Name:    simhttp.h	
*
* Summary:
*  This file contains common stuff for the server
*
* Revisions:
*
*********************************************************/
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
//#include <getopt.h>
#include <ctype.h>
#include <sys/time.h>
#include <math.h>
#include <sys/types.h>
#include <string.h>     /* for memset() */
#include <netinet/in.h> /* for in_addr */
#include <sys/socket.h> /* for socket(), connect(), sendto(), and recvfrom() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <unistd.h>     /* for close(), getopt(), chdir() */

typedef struct {
    char * method;
    char * file;
    char * version;
    char * host;
    char * options;
} http_request_t;

typedef struct {
    char * version;
    char * code;
    char * msg;
    char ** options;
} http_response_t;

http_request_t * parseRequest(char * buf);
http_response_t * generateResponse(http_request_t * req);
char * packResponse(http_response_t * resp);
void outputLog(http_request_t * req,http_response_t * resp);
char * generateResponse(char * buf,char * dir);


int readData(void);

#endif
