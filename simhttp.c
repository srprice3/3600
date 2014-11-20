/*	simhttp.c  by Stephen Price
	CpSc 3600 HW 3	Dr. Sekou Remy	Fall 2014

	This program will spawn a simple HTTP server and respond to GET and POST

	Program usage:	./simhttp <-p optional portNum> <optional path to files>

*/

#include "simhttp.h"

int fileNum = 0;                 /* Number of files transferred */

int main(int argc, char *argv[])
{
    int sock, connfd, fin;           /* Socket, connection file descriptor, finished flag */
    struct sockaddr_in servAddr;     /* Local address */
    struct sockaddr_in clientAddr;   /* address of client */
    unsigned int addrLen;            /* Length of address */
    char inBuffer[1024];             /* Buffer for incoming msg */
	char outBuffer[1024];            /* Buffer for outgoing msg */
	FILE *fp;                        /* pointer to new file */
    unsigned short portNum = 80;     /* Server port */
    int recvMsgSize;                 /* Size of received message */
    char fileName[30];               /* name of file to transfer */
	char path[50] = "./";            /* optional path to files */
	
	char *newline = "\r\n";
	char *serverName = "Server: simhttp/1.1\r\n";
	char *err404 = "HTTP/1.1 404 NOT FOUND\r\n";
	

	if (argc > 4)         /* Test for correct number of parameters */
    {
        fprintf(stderr,"Usage:  ./simhttp <-p optional port> <optional path>\n");
        exit(1);
    }

	/*  process options using getopt()  */
	
  	int option;

  	opterr = 0;               /* prevent getopt() from printing messages to stderr */
  	while ((option = getopt (argc, argv, "p:")) != -1){

    	switch (option) {
			//printf("option = %d\n", option);
      		case 'p':
        		portNum = (unsigned short) strtoul(optarg, NULL, 0);
        		break;
      		case '?':
        		if (isprint (optopt))
          		fprintf (stderr, "ERROR\tUnknown option `-%c'.\n", optopt);
        		else
          		fprintf (stderr, "ERROR\tUnknown option character `\\x%x'.\n", optopt);
				exit(1);
      		default:
				fprintf(stderr, "ERROR\tImproper arguments\n");
        		exit(1);
      	}
	}
	fprintf(stderr, "optind: %d, argc: %d\n", optind, argc);
	if (optind < argc) {
		strcpy(path, argv[optind]);
		if (chdir(path) == -1){
			fprintf(stderr, "Error changing to specified directory\n");
			exit(1);
		}
	}

	fprintf(stderr, "Port Number = %d, path: %s\n", portNum, path);
	addDate(outBuffer);

	/* Create socket */
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
        fprintf(stderr, "socket() failed");
		exit(1);
    }

	 /* Construct local address structure */
    memset(&servAddr, 0, sizeof(servAddr));           /* Zero out structure */
    servAddr.sin_family = AF_INET;                    /* Internet address family */
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);     /* Any incoming interface */
	servAddr.sin_port = htons(portNum);               /* Local port */

	 
	fprintf(stderr, "Binding to port %d\n", portNum);  
	/* Bind to the local address */  
	if (bind(sock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
	{
		fprintf(stderr, "bind() failed");
		exit(1);
	}

	listen(sock, 10);	

	for (;;)                     /* Run forever */
	{
		/* Set the size of the in-out parameter */
		addrLen = sizeof(clientAddr);
		connfd = accept(sock,(struct sockaddr *) &clientAddr, &addrLen);
		if (connfd < 0)
		{
			fprintf(stderr, "accept() failed\n");
			//exit(1);
		}
		else if (connfd == 0)
		{
			fprintf(stderr, "EOF\n");
		}
		else
		{
			fprintf(stderr, "connection\n");
			int fin = 1;
			fin = read(connfd, inBuffer, 1023);
			if (fin < 0) {
				fprintf(stderr, "error in transfer\n");
				close(connfd);
			}
			readData();
			close(connfd);
		
		}
	
	}
	return 0;
}

int readData(void) {
	// int n = 0;
// 	char *c = NULL;
// 	while ((n < 1024) && (*c != '\n')){
// 		c = inBuffer[n];
// 	}
	return 0;
}

int addDate(char *ptr) {
	int charsAdded = 0;
	//printf("addDate method\n");
	char tmp[256];
	char tmp2[256];
	time_t rawTime;
	struct tm *timeInfo;
	time(&rawTime);
	timeInfo = localtime(&rawTime);
	strftime(tmp, 100, "%a, %d %b %Y %X\r\n", timeInfo);
	strcpy(tmp2, "Date: ");
	strcat(tmp2, tmp);
	printf("%s", tmp2);
	charsAdded = strlen(tmp2);
	printf("added %d chars\n", charsAdded);
	return charsAdded;
	
}
