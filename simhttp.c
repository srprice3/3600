/*	simhttp.c  by Stephen Price
	CpSc 3600 HW 3	Dr. Sekou Remy	Fall 2014

	This program will spawn a simple HTTP server and respond to GET and HEAD

	Program usage:	./simhttp <-p optional portNum> <optional path to files>

*/

#include "simhttp.h"

char *newline = "\r\n";
char *serverName = "Server: simhttp/1.0\r\n";
char *err400 = "HTTP/1.1 400 BAD REQUEST\r\n";
char *err403 = "HTTP/1.1 403 FORBIDDEN\r\n";
char *err404 = "HTTP/1.1 404 NOT FOUND\r\n";
char *err405 = "HTTP/1.1 405 METHOD NOT ALLOWED\r\nAllow: GET, HEAD\r\n";
char *connClose = "Connection: close\r\n";
char *cType = "Content-Type: ";   /* used for content-type header  */
char *cLength = "Content-Length: "; /* used for content-length header */

int main(int argc, char *argv[])
{
    int sock, connfd, fin;           /* Socket, connection file descriptor, finished flag */
    struct sockaddr_in servAddr;     /* Local address */
    struct sockaddr_in clientAddr;   /* address of client */
    unsigned int addrLen;            /* Length of address */
    char inBuffer[1024];             /* Buffer for incoming msg */
	char outBuffer[1024];            /* Buffer for outgoing msg */
	char fileBuffer[1001];           /* Buffer for sending file */
	FILE *fp;                        /* pointer to new file */
    unsigned short portNum = 8000;     /* Server port */
    int recvMsgSize;                 /* Size of received message */
    char *fileName = NULL;               /* name of file to transfer */
	char *path = "./";            /* optional path to files */
	char method[10];                 /* HEAD or GET strings    */
	struct stat *st = NULL;          /* used to get file status */
	http_request_t * req = NULL;
	

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
		path = realpath(argv[optind],NULL);
		if (path == NULL || chdir(path) == -1){
			fprintf(stderr, "Error changing to specified directory\n");
			exit(1);
		}
	}
	else {
		path = realpath(path,NULL);
	}

	fprintf(stderr, "Port Number = %d, path: %s\n", portNum, path);
	//addDate(outBuffer);
	// resp403(outBuffer);
// 	resp404(outBuffer);
// 	resp405(outBuffer);
// 	FILE  *newFile;
// 	newFile = fopen("test.txt", "r");
// 	addMod(inBuffer, newFile);
// 	printBuffer(inBuffer, 1);
// 	fclose(newFile);
	

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
			
			req = parseRequest(inBuffer, path);

			fileName = realpath(req->full_path,NULL);

			if (req->host_flag == 0) {
				fprintf(stderr,"Returning 400\n");
				outputServerInfo(req->method,400,fileName + strlen(path) + 1);
				resp400(outBuffer);
				write(connfd,outBuffer,1024);
			}
			else if (errno != ENOENT && fileName != NULL && (strstr(fileName,path) == NULL || errno == EACCES || access(fileName,R_OK) < 0)) {
				fprintf(stderr,"Returning 403\n");
				outputServerInfo(req->method,403,fileName + strlen(path) + 1);
				resp403(outBuffer);
				write(connfd,outBuffer,1024);
			}
			else if (errno == ENOENT) {
				fprintf(stderr,"Returning 404\n");
				outputServerInfo(req->method,404,fileName + strlen(path) + 1);
				resp404(outBuffer);
				write(connfd,outBuffer,1024);
			}
			else if (strcmp(req->method, "GET") != 0 && strcmp(req->method, "GET") != 0) {
				fprintf(stderr,"Returning 405\n");
				outputServerInfo(req->method,405,fileName + strlen(path) + 1);
				resp405(outBuffer);
				write(connfd,outBuffer,1024);
			}
			else {
				fprintf(stderr,"Returning 200\n");
				outputServerInfo(req->method,200,fileName + strlen(path) + 1);
				
				/* get filename from request */
				/* fill outBuffer with headers */
				/* send outBuffer and /r/f */
				/* send file requested */
				//strcpy(fileName, resp->full_path);  // change to real filename
				fp = fopen(fileName, "r");
				if (fp == NULL)
					{
						perror ("Error with fopen()");
					}
				stat(fileName, st);
				int fileSize = st->st_size;
				//printf("file size: %d\n", fileSize);
				
				/* add # of bytes to cLength header */
				char sizeString[10];
				sprintf(sizeString, "%d", fileSize);
				strcat(cLength, sizeString);
				strcat(cLength, "\r\n");
				
				/* fill outBuffer and send */
				char *position = outBuffer;
				memcpy(position, "HTTP/1.1 200 OK\r\n", 17);
				position += 17;
				position += addDate(position);
				memcpy(position, serverName, strlen(serverName));
				position += strlen(serverName);
				position += addMod(position, fp);
				position += getMime(position, fileName);
				memcpy(position, cLength, strlen(cLength));
				position += strlen(cLength);
				memcpy(position, connClose, strlen(connClose));
				printBuffer(outBuffer, 7);
				
				write(sock,outBuffer, 1000);
				if (strcmp(method, "GET") == 0){
					int x;
					
					/*  Loop until EOF  */
					for(x = 0; x < fileSize/1000; x++)
					{
						fread(fileBuffer, 1, 1000, fp);
						//printf("x: %d\n", x);
						write(sock, fileBuffer, 1000);
					}
					fclose(fp);
				}

				free(fileName);
			}
			
			if (req->filename != NULL) {
				free(req->filename);
			}
			if (req->filepath != NULL) {
				free(req->filepath);
			}
			if (req->full_path != NULL) {
				free(req->full_path);
			}

			free(req);

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
	timeInfo = gmtime(&rawTime);
	strftime(tmp, 100, "%a, %d %b %Y %X GMT\r\n", timeInfo);
	strcpy(tmp2, "Date: ");
	strcat(tmp2, tmp);
	//printf("%s", tmp2);
	charsAdded = strlen(tmp2);
	memcpy(ptr, tmp2, charsAdded);
	return charsAdded;
	
}

int addMod(char *ptr, FILE *fp) {
	int charsAdded = 0;
	char tmp[256];
	char tmp2[256];
	time_t rawTime;
	struct tm *timeInfo;
	struct stat fileInfo;
	int fd;
	fd = fileno(fp);
	fstat(fd, &fileInfo);
	rawTime = fileInfo.st_mtime;
	timeInfo = gmtime(&rawTime);
	strftime(tmp, 100, "%a, %d %b %Y %X GMT\r\n", timeInfo);
	strcpy(tmp2, "Last-Modified: ");
	strcat(tmp2, tmp);
	//printf("%s", tmp2);
	charsAdded = strlen(tmp2);
	memcpy(ptr, tmp2, charsAdded);
	return charsAdded;
	
}

void resp400(char *ptr) {
	char *position = ptr;
	memcpy(position, err400, strlen(err400));
	position += strlen(err400);
	position += addDate(position);
	memcpy(position, serverName, strlen(serverName));
	position += strlen(serverName);
	memcpy(position, connClose, strlen(connClose));
	printBuffer(ptr, 5);
}


void resp403(char *ptr) {
	char *position = ptr;
	memcpy(position, err403, strlen(err403));
	position += strlen(err403);
	position += addDate(position);
	memcpy(position, serverName, strlen(serverName));
	position += strlen(serverName);
	memcpy(position, connClose, strlen(connClose));
	printBuffer(ptr, 4);
}

void resp404(char *ptr) {
	char *position = ptr;
	memcpy(position, err404, strlen(err404));
	position += strlen(err404);
	position += addDate(position);
	memcpy(position, serverName, strlen(serverName));
	position += strlen(serverName);
	memcpy(position, connClose, strlen(connClose));
	printBuffer(ptr, 4);
}

void resp405(char *ptr) {
	char *position = ptr;
	memcpy(position, err405, strlen(err405));
	position += strlen(err405);
	position += addDate(position);
	memcpy(position, serverName, strlen(serverName));
	position += strlen(serverName);
	memcpy(position, connClose, strlen(connClose));
	printBuffer(ptr, 5);
}

int getMime(char *ptr, char *fname) {
	
	char *dot = NULL;
	dot = strrchr(fname, '.');

	if (strncmp(dot, ".css", 4) == 0){
		strcat(cType, "text/css\r\n");
	}
	else if (strncmp(dot, ".htm", 4) == 0){
		strcat(cType, "text/html\r\n");
	}
	else if (strncmp(dot, ".js", 3) == 0){
		strcat(cType, "application/javascript\r\n");
	}
	else if (strncmp(dot, ".txt", 4) == 0){
		strcat(cType, "text/plain\r\n");
	}
	else if (strncmp(dot, ".jpg", 4) == 0){
		strcat(cType, "image/jpeg\r\n");
	}
	else if (strncmp(dot, ".pdf", 4) == 0){
		strcat(cType, "application/pdf\r\n");
	}
	else {
		strcat(cType, "application/octet-stream\r\n");
	}
	fprintf(stderr, "MIME Type: %s\n", cType);
	strcat(ptr, cType);
	return strlen(cType);
}

void printBuffer(char *buf, int lines) {
	fprintf(stderr,"======================BUFFER=========================\n");
	int nullNum = 0;
	while (nullNum < lines) {
		fprintf(stderr,"%s", buf);
		buf += strlen(buf);
		nullNum ++;
	}
	fprintf(stderr,"=====================================================\n");
}

http_request_t * parseRequest(char * buf, char * dir) {
	http_request_t * req = (http_request_t *)malloc(sizeof(http_request_t));
	bzero(req,sizeof(http_request_t));

	fprintf(stderr, "Tokenizing the first line: ");
	char * line = strtok(buf,"\r\n"), * temp_filename, * endptr;

	req->method = (char *) malloc(strlen(line));
	req->filename = (char *) malloc(strlen(line));
	req->version = (char *) malloc(strlen(line));
	req->filepath = (char *) malloc(strlen(line) + strlen(dir) + 1);
	req->full_path = (char *) malloc(strlen(line) + strlen(dir));
	req->host_flag = 0;
	temp_filename = (char *) malloc(strlen(line));

	bzero(temp_filename,strlen(line));

	fprintf(stderr, "%s\n", line);

	if (line == NULL || sscanf(line, "%s %s %s", req->method, temp_filename, req->version) != 3) {
		fprintf(stderr, "Failed to scan the first line\n");
		return req;
	}

	fprintf(stderr, "Method: %s\nFilename: %s\nVersion: %s\n",req->method,temp_filename,req->version);

	endptr = strrchr(temp_filename, '/');

	if (endptr == NULL) {
		endptr = temp_filename;
	}

	if (temp_filename[strlen(temp_filename) - 1] == '/') {
		req->filename = (char *)malloc(strlen("index.html") + 1);
		bzero(req->filename,strlen("index.html") + 1);
		strcpy(req->filename,"index.html");
	}
	else {
		req->filename = (char *)malloc(strlen(endptr) + 1);
		bzero(req->filename,strlen(endptr) + 1);
		strcpy(req->filename,endptr);
	}

	strcpy(req->filepath, dir);
	req->filepath = strcat(req->filepath,"/");
	memcpy(req->filepath + strlen(dir) + 1,temp_filename, strlen(temp_filename) - strlen(endptr));

	req->full_path = (char *)malloc(strlen(req->filepath) + strlen(req->filename));
	bzero(req->full_path, strlen(req->filepath) + strlen(req->filename));
	strcpy(req->full_path, req->filepath);
	strcat(req->full_path, req->filename);

	free(temp_filename);

	while ((line = strtok(NULL, "\r\n")) != NULL && req->host_flag == 0) {
		if (strstr(line, "Host:") != NULL) {
			req->host_flag = 1;
		}
	}

	return req;
}

void outputServerInfo(char * method,int status,char * file) {
	time_t t = time(NULL);
	struct tm * datetime = localtime(&t);
	char timebuf[256];
	strftime(timebuf, 256, "%a, %d %b %Y %T %Z", datetime);
	fprintf(stdout,"%s\t%s\t%s\t%d\n",method,file,timebuf,status);
	fprintf(stderr,"%s\t%s\t%s\t%d\n",method,file,timebuf,status);
}