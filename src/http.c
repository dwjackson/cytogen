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

#define MAX_CONNECTIONS 5
#define BUFSIZE 512
#define CRLF "\r\n"
#define ERROR_404 "404: File not found"

static void
handle_request(int sockfd);

static void
send_response(int sockfd, char *path);

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
	char method[10];
	int i;
	char ch;
	int path_len;

	for (i = 0; i < 10 - 1; i++) {
		ch = req[i];
		if (ch == ' ') {
			break;
		}
		method[i] = ch;
	}
	method[i] = '\0';
	i++;

	if (strcmp(method, "GET") != 0) {
		fprintf(stderr, "Only GET requests are supported (%s)\n", method);
		abort();
	}
 
	path_len = 0;
	for (; i < size; i++) {
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

static void
handle_request(int sockfd)
{
	char buf[BUFSIZE];
	ssize_t size;
	char path[PATH_MAX - 1];

	size = recv(sockfd, buf, BUFSIZE, 0);
	extract_path(buf, size, path, PATH_MAX - 1);
	if (strstr(path, "favicon.ico") != NULL) {
		/* Ignore favicon requests */
		return;
	}
	send_response(sockfd, path);
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
send_response(int sockfd, char *path)
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
	char buf_fmt[] = "HTTP/1.1 %d %s" CRLF
		"Date: %s" CRLF
		"Server: cyhttp" CRLF
		"Content-Length: %d" CRLF
		"Content-Type: %s; charset=utf-8" CRLF CRLF
		"%s" CRLF CRLF;
	char *buf;
	char *content;
	int content_len;
	char topdir[PATH_MAX - 1];
	char path_part[PATH_MAX - 1];
	char *orig_path = path;

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
			strncpy(file_name, path_part, PATH_MAX - 3);
		} else if (S_ISDIR(statbuf.st_mode)) {
			chdir(path_part);
		} else if (S_ISREG(statbuf.st_mode)) {
			strncpy(file_name, path_part, PATH_MAX - 3);
			file_name[PATH_MAX - 2] = '\0';
			if (strstr(file_name, ".css") != NULL) {
				strcpy(content_type, "text/css");
			} else if (strstr(file_name, ".xml") != NULL) {
				strcpy(content_type, "application/xml");
			} else if (strstr(file_name, ".js") != NULL) {
				strcpy(content_type, "text/javascript");
			}
		}
	}

	if (strlen(file_name) == 0) {
		strcpy(file_name, "index.html");
	}

	if (stat(file_name, &statbuf) == 0) {
		printf("200: %s\n", file_name);
		file_size = statbuf.st_size;
		content = malloc(file_size + 1);
		fp = fopen(file_name, "r");
		fread(content, 1, file_size, fp);
		content[file_size] = '\0';
		content_len = (int)file_size + 1;
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
	asprintf(&buf, buf_fmt, status, status_string, timebuf, content_len, 
			content_type, content);
	send(sockfd, buf, strlen(buf), 0);

	free(buf);
	free(content);

	chdir(topdir);
}
