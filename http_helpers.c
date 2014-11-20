http_request_t * parseRequest(char * buf) {
	http_request_t * req = (http_request_t *)malloc(sizeof(http_request_t));
	char * fields = strtok(buf,"\r\n");

	req->version = 

	do {

	}

	return req;
}

http_response_t * generateResponse(http_request_t * req) {

}

uint8_t * packResponse(http_response_t * resp) {

}

void outputLog(http_request_t * req,http_response_t * resp) {

}

uint8_t * generateResponse(uint8_t * buf,char * dir) {
	return NULL;
}