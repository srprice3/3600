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
char cType[50];              /* used for content-type header  */
char cLength[50];            /* used for content-length header */

int main(int argc, char *argv[])
{
    int sock, connfd;                /* Socket, connection file descriptor, finished flag */
    struct sockaddr_in servAddr;     /* Local address */
    struct sockaddr_in clientAddr;   /* address of client */
    unsigned int addrLen;            /* Length of address */
    char *inBuffer;                  /* Buffer for incoming msg */
	char *outBuffer;                 /* Buffer for outgoing msg */
	FILE *fp;                         /* pointer to new file */
    unsigned short portNum = 8080;    /* Server port */
    char fileName[256];               /* name of file to transfer */
	char *path = "./";                /* optional path to files */
	struct stat *st;                  /* used to get file status */
	http_request_t *req = NULL;       /* struct with request info  */
	
	strcpy(cType, "Content-Type: ");
	strcpy(cLength, "Content-Length: ");

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
	//fprintf(stderr, "optind: %d, argc: %d\n", optind, argc);
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

	fprintf(stderr, "\nStarting server, Port Number = %d, path: %s\n", portNum, path);

	/* Create socket */
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
        fprintf(stderr, "socket() failed\n");
		exit(1);
    }

	 /* Construct local address structure */
    memset(&servAddr, 0, sizeof(servAddr));           /* Zero out structure */
    servAddr.sin_family = AF_INET;                    /* Internet address family */
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);     /* Any incoming interface */
	servAddr.sin_port = htons(portNum);               /* Local port */

	 
	//fprintf(stderr, "Binding to port %d\n", portNum);  
	/* Bind to the local address */  
	if (bind(sock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
	{
		fprintf(stderr, "bind() failed\n");
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
			fprintf(stderr, "new connection\n");
			inBuffer = (char *)calloc(1024, 1);
			outBuffer = (char *)calloc(5001, 1);
			st = (struct stat *)calloc(sizeof(struct stat), 1);
			strcpy(cType, "Content-Type: ");
			strcpy(cLength, "Content-Length: ");
			errno = 0;
			//fprintf(stderr, "ct: %s, cl: %s\n", cType, cLength);
			//printBuffer(outBuffer, 6);
			int fin = 1;
			fin = read(connfd, inBuffer, 1023);
			if (fin < 0) {
				fprintf(stderr, "error in transfer\n");
				close(connfd);
			}
			
			req = parseRequest(inBuffer, path);

			//fileName = realpath(req->full_path,NULL);
			strcpy(fileName, req->filename);
			
			stat(fileName, st);
			fprintf(stderr, "stat errno = %d\n", errno);
			
			//fprintf(stderr, "host flag %d\n", req->host_flag);
			if (req->host_flag == 0) {
				fprintf(stderr,"Returning 400\n");
				outputServerInfo(req->method,400,fileName);
				resp400(outBuffer);
				printBuffer(outBuffer, 6);
				write(connfd,outBuffer,128);
			}
			
			//else if (errno != ENOENT && fileName != NULL && (strstr(fileName,path) == NULL || errno == EACCES || access(fileName,R_OK) < 0)) {
			else if (errno  == EACCES){
				fprintf(stderr,"Returning 403\n");
				outputServerInfo(req->method,403,fileName);
				resp403(outBuffer);
				printBuffer(outBuffer, 6);
				write(connfd,outBuffer,128);
			}
			else if (errno == ENOENT) {
				fprintf(stderr,"Returning 404\n");
				outputServerInfo(req->method,404,fileName);
				resp404(outBuffer);
				printBuffer(outBuffer, 6);
				write(connfd,outBuffer,128);
			}
			else if ((strcmp(req->method, "GET") != 0) && (strcmp(req->method, "HEAD") != 0)) {
				fprintf(stderr,"Returning 405\n");
				outputServerInfo(req->method,405,fileName);
				resp405(outBuffer);
				printBuffer(outBuffer, 6);
				write(connfd,outBuffer,128);
			}
			else {
				fprintf(stderr,"Returning 200\n");
				outputServerInfo(req->method, 200, fileName);
				
				/* add # of bytes to cLength header */
				int fileSize = st->st_size;
				char sizeString[25];
				sprintf(sizeString, "%d", fileSize);
				strcat(cLength, sizeString);
				strcat(cLength, "\r\n");
				
				/* fill outBuffer header section */
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
				position += strlen(connClose);
				memcpy(position, newline, strlen(newline));
				position += strlen(newline);		
				printBuffer(outBuffer, 14);
				
				if (strcmp(req->method, "GET") == 0){
					/*  add file data to buffer */
					int r;
					char *fileBuffer = position;
					fp = fopen(fileName, "r");
					if (fp == NULL)
						{
							fprintf(stderr, "Error with fopen(), errno = %d\n", errno);
						}
					
					/*  Loop until EOF  */
					//for(x = 0; x < fileSize; x++)
					//{
						r = fread(fileBuffer, 1, fileSize, fp);
						fprintf(stderr, "%d bytes read\n", r);
						if (r != fileSize){
							fprintf(stderr, "error in fread()\n");
						}	
						//write(sock, fileBuffer, 10);
					//}
					fclose(fp);
				}

				int w;
				w = write(connfd, outBuffer, fileSize + 256);
				fprintf(stderr, "%d bytes sent\n", w);
				//fprintf(stderr, "file sent\n");
			}
		}
		
		close(connfd);
		free(inBuffer);
		free(outBuffer);
		free(st);	
		
		fprintf(stderr, "connection terminated\n");
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
	position += strlen(connClose);
	memcpy(position, newline, strlen(newline));
	//printBuffer(ptr, 6);
}


void resp403(char *ptr) {
	char *position = ptr;
	memcpy(position, err403, strlen(err403));
	position += strlen(err403);
	position += addDate(position);
	memcpy(position, serverName, strlen(serverName));
	position += strlen(serverName);
	memcpy(position, connClose, strlen(connClose));
	position += strlen(connClose);
	memcpy(position, newline, strlen(newline));
	//printBuffer(ptr, 6);
}

void resp404(char *ptr) {
	char *position = ptr;
	memcpy(position, err404, strlen(err404));
	position += strlen(err404);
	position += addDate(position);
	memcpy(position, serverName, strlen(serverName));
	position += strlen(serverName);
	memcpy(position, connClose, strlen(connClose));
	position += strlen(connClose);
	memcpy(position, newline, strlen(newline));
	//printBuffer(ptr, 6);
}

void resp405(char *ptr) {
	char *position = ptr;
	memcpy(position, err405, strlen(err405));
	position += strlen(err405);
	position += addDate(position);
	memcpy(position, serverName, strlen(serverName));
	position += strlen(serverName);
	memcpy(position, connClose, strlen(connClose));
	position += strlen(connClose);
	memcpy(position, newline, strlen(newline));
	//printBuffer(ptr, 7);
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
	//fprintf(stderr, "getMime function: %s", cType);
	memcpy(ptr, cType, strlen(cType));
	return strlen(cType);
}

void printBuffer(char *buf, int lines) {
	fprintf(stderr,"====================%d lines of BUFFER===============\n", lines);
	int nullNum = 0;
	while (nullNum < lines) {
		fprintf(stderr,"%s", buf);
		buf += strlen(buf);
		nullNum ++;
	}
	fprintf(stderr,"=====================================================\n");
}

http_request_t *parseRequest(char * buf, char * dir) {
	http_request_t *req = (http_request_t *)malloc(sizeof(http_request_t));
	bzero(req,sizeof(http_request_t));

	fprintf(stderr, "Tokenizing the first line: ");
	char *line = strtok(buf,"\r\n"), * temp_filename, * endptr;

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

	endptr = strchr(temp_filename, '/');
	//fprintf(stderr, "endptr + 1: %s\n", endptr + 1);

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
		strcpy(req->filename,endptr +1);
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

void outputServerInfo(char *method,int status,char *file) {
	time_t t = time(NULL);
	struct tm * datetime = localtime(&t);
	char timebuf[256];
	strftime(timebuf, 256, "%a, %d %b %Y %T %Z", datetime);
	fprintf(stdout,"%s\t%s\t%s\t%d\n",method,file,timebuf,status);
	fprintf(stderr,"%s\t%s\t%s\t%d\n",method,file,timebuf,status);
}
