/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

/*
 * Copyright (c) 2018-2020 David Jackson
 */

#ifdef __linux__
#define _GNU_SOURCE
#endif /* __linux__ */

#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#include <limits.h>
#include <stdbool.h>
#include "mime.h"

#define MAX_CONNECTIONS 5
#define BUFSIZE 512
#define CRLF "\r\n"
#define ERROR_404 "404: File not found"
#define METHOD_BUFSIZE 10

typedef unsigned char byte;

static void
handle_request(int sockfd);

static void
send_response(char *method, int sockfd, char *path);

void
http_server(int port)
{
	int sockfd;
	int status;
	struct sockaddr_in addr;
	int c; /* client socket */
	socklen_t addrlen;
	int opt_true;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		perror("socket");
		abort();
	}
	opt_true = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt_true, sizeof(int));

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	status = bind(sockfd, (struct sockaddr *) &addr, sizeof(addr));
	if (status < 0) {
		perror("bind");
		abort();
	}

	status = listen(sockfd, MAX_CONNECTIONS);
	if (status < 0) {
		perror("listen");
		abort();
	}

	printf("Serving on 0.0.0.0:%d...\n", port);

	addrlen = sizeof(addr);
	while (1) {
		c = accept(sockfd, (struct sockaddr *) &addr, &addrlen);
		if (c < 0) {
			perror("accept");
			abort();
		}
		handle_request(c);
		close(c);
	}

	close(sockfd);
}

static void 
extract_path(const char *req, ssize_t size, char *path, size_t path_size)
{
	int path_len;
	int i;
	int ch;

	path_len = 0;
	for (i = 0; i < size; i++) {
		ch = req[i];
		if (path_len >= path_size) {
			fprintf(stderr, "Path length too long\n");
			abort();
		}
		if (ch == ' ') {
			break;
		}
		path[path_len++] = ch;
	}
	path[path_len] = '\0';
}

static int 
extract_method(char *method, char *req)
{
	int i;
	char ch;
	int path_len;
	char supported_methods[2][METHOD_BUFSIZE] = {
		"GET",
		"HEAD"
	};
	bool is_allowed = false;
	int method_len;

	for (i = 0; i < 10 - 1; i++) {
		ch = req[i];
		if (ch == ' ') {
			break;
		}
		method[i] = ch;
	}
	method[i] = '\0';
	i++;
	method_len = i;

	for (i = 0; i < 2; i++) {
		if (strcmp(method, supported_methods[i]) == 0) {
			is_allowed = true;
		}
	}

	if (!is_allowed) {
		fprintf(stderr, "Unsupported method (%s)\n", method);
		abort();
	}

	return method_len;
}

static void
handle_request(int sockfd)
{
	char buf[BUFSIZE];
	ssize_t size;
	char path[PATH_MAX - 1];
	char method[METHOD_BUFSIZE];
	int method_len;
	char *req;

	size = recv(sockfd, buf, BUFSIZE, 0);
	method_len = extract_method(method, buf);
	req = buf + method_len;
	extract_path(req, size, path, PATH_MAX - 1);
	if (strstr(path, "favicon.ico") != NULL) {
		/* Ignore favicon requests */
		return;
	}
	send_response(method, sockfd, path);
}

/* Return 1 if there are parts left */
static int
next_path_part(char *path_part, char **path_ptr)
{
	char ch;

	if (**path_ptr == '\0') {
		return 0;
	}
	
	/* Copy "next piece" of path into path_part, updating path as we go */
	while ((ch = *(*path_ptr)++) != '/' && ch != '\0') {
		*path_part++ = ch;
	}
	*path_part++ = '\0';
	if (ch == '/') {
		return 1;
	}
	return 0;
}

static void
send_response(char *method, int sockfd, char *path)
{
	char file_name[PATH_MAX - 1];
	off_t file_size;
	FILE *fp;
	struct stat statbuf;
	time_t now;
	struct tm now_tm;
	char timebuf[100];
	char content_type[100] = "text/html";
	int status = 200;
	char status_string[100] = "OK";
	char header_fmt[] = "HTTP/1.1 %d %s" CRLF
		"Date: %s" CRLF
		"Server: cyhttp" CRLF
		"Content-Length: %d" CRLF
		"Content-Type: %s" CRLF CRLF;
	char *header = NULL;
	byte *buf = NULL;
	byte *content = NULL;
	int content_len;
	char topdir[PATH_MAX - 1];
	char path_part[PATH_MAX - 1];
	char *orig_path = path;
	bool is_binary = false;
	int i;
	size_t header_len;
	size_t response_len = 0;
	char *mime_type;

	memset(file_name, 0, PATH_MAX - 1);
	memset(timebuf, 0, 100);
	memset(topdir, 0, PATH_MAX - 1);
	memset(path_part, 0, PATH_MAX - 1);

	getcwd(topdir, PATH_MAX - 1);
	if (!(strlen(path) == 1 && path[0] == '/')) {
		path++; /* Skip the initial "/" */
		while (next_path_part(path_part, &path)) {
			if (chdir(path_part) < 0) {
				printf("404: %s\n", orig_path);
				status = 404;
				strcpy(status_string, "Not Found");
				content_len = strlen(ERROR_404);
				content = malloc(content_len + 1);
				strcpy(content, ERROR_404);
				break;
			}
		}
		if (stat(path_part, &statbuf) < 0) {
			strcpy(file_name, "");
		} else if (S_ISDIR(statbuf.st_mode)) {
			chdir(path_part);
		} else if (S_ISREG(statbuf.st_mode)) {
			strncpy(file_name, path_part, PATH_MAX - 3);
			file_name[PATH_MAX - 2] = '\0';
			is_binary = false;
			mime_type = mime_type_of(file_name);
			if (mime_type != NULL) {
				strcpy(content_type, mime_type);
			} else {
				fprintf(stderr, "Unsupported filetype: %s\n",
						file_name);
			}
		}
	}

	if (strlen(file_name) == 0) {
		strcpy(file_name, "index.html");
	}

	if (stat(file_name, &statbuf) == 0) {
		printf("200: %s\n", orig_path);
		file_size = statbuf.st_size;
		content = malloc(file_size + 1);
		if (is_binary) {
			fp = fopen(file_name, "rb");
		} else {
			fp = fopen(file_name, "r");
		}
		if (fread(content, 1, file_size, fp) != file_size) {
			perror("fread");
			abort();
		}
		content_len = (int)file_size;
		if (content_len < 0) {
			fprintf(stderr, "Negative content length\n");
			abort();
		}
	} else {
		printf("404: %s\n", orig_path);
		status = 404;
		strcpy(status_string, "Not Found");
		content_len = strlen(ERROR_404);
		content = malloc(content_len + 1);
		strcpy(content, ERROR_404);
	}

	now = time(NULL);
	localtime_r(&now, &now_tm);
	strftime(timebuf, 100, "%a, %d %b %Y %H:%M:%S %Z", &now_tm);
	asprintf(&header, header_fmt, status, status_string, timebuf,
			content_len, content_type, content);
	header_len = strlen(header);
	if (strcmp(method, "GET") == 0) {
		response_len = header_len + content_len + 2;
		buf = malloc(response_len);
		if (buf == NULL) {
			perror("malloc");
			abort();
		}
		memcpy(buf, header, header_len);
		memcpy(buf + header_len, content, content_len);
		memcpy(buf + header_len + content_len, CRLF, 2);
	} else if (strcmp(method, "HEAD") == 0) {
		response_len = header_len;
		buf = malloc(header_len);
		if (buf == NULL) {
			perror("malloc");
			abort();
		}
		memcpy(buf, header, header_len);
	} else {
		fprintf(stderr, "Unsupported method: %s\n", method);
	}

	ssize_t sent = 0;
	while ((sent = send(sockfd, buf, response_len, 0)) < response_len 
			&& sent != -1) {
	}
	if (sent == -1) {
		perror("send");
	}

	free(buf);
	free(content);
	free(header);

	chdir(topdir);
}
