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
#include <ctype.h>
#include <sys/time.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>     /* for memset() */
#include <netinet/in.h> /* for in_addr */
#include <sys/socket.h> /* for socket(), connect(), sendto(), and recvfrom() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <unistd.h>     /* for close(), getopt(), chdir() */


int readData(void);

int addDate(char *);  /* gets the local date and inserts a date header string at ptr 
						 returns number of chars added  */

int addMod(char *, FILE *); /*  gets the file modified date and inserts a Last-Modified 
								header string at ptr returns number of chars added */

void resp400(char *ptr); /* puts all response info for 400 error into ptr */
void resp403(char *ptr); /* puts all response info for 403 error into ptr */
void resp404(char *ptr); /* puts all response info for 404 error into ptr */
void resp405(char *ptr); /* puts all response info for 405 error into ptr */

void getMime(char *fname); /* gets mimetype from filename */

void printBuffer(char *buf, int); /* print contents of <int> lines in the buffer */

#endif
