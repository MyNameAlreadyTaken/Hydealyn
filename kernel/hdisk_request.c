#include "hdisk_request.h"

extern struct HDISK_REQUESTS hdisk_requests;
extern struct HDISK_REQUEST hdisk_request;

void direct_read_hdisk(int sector) {
	hdisk_request.dev = 0;
	hdisk_request.cmd = 0;
	hdisk_request.error = 0;
	hdisk_request.sector = sector;
	hdisk_request.nr_sector = 1;
	do_hdisk_request();
}

void direct_write_hdisk(int sector) {
	hdisk_request.dev = 0;
	hdisk_request.cmd = 1;
	hdisk_request.error = 0;
	hdisk_request.sector = sector;
	hdisk_request.nr_sector = 1;
	/*int i;
	for (i = 0; i < 32; ++i) {
		hdisk_request.buffer[i] = 32 - i;
	}*/
	do_hdisk_request();
}

void add_hdisk_request(struct HDISK_REQUEST *request) {
	if (hdisk_requests.first_request != 0) {
		hdisk_requests.last_request->next = request;
		hdisk_requests.last_request = request;
	}
	else {
		hdisk_requests.first_request = request;
		hdisk_requests.last_request = request;
	}
}

void end_hdisk_request() {
	hdisk_requests.first_request = hdisk_requests.first_request->next;
	if (hdisk_requests.first_request == 0) {
		hdisk_requests.last_request = 0;
	}
	return;
}
