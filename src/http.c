/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

/*
 * Copyright (c) 2018 David Jackson
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

#define MAX_CONNECTIONS 5
#define BUFSIZE 512
#define CRLF "\r\n"

static void
handle_request(int sockfd);

static void
send_response(int sockfd);

void
http_server(int port)
{
	int sockfd;
	int status;
	struct sockaddr_in addr;
	int c; /* client socket */
	socklen_t addrlen;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		perror("socket");
		abort();
	}

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
handle_request(int sockfd)
{
	char buf[BUFSIZE];
	ssize_t size;
	int i;
	size = recv(sockfd, buf, BUFSIZE, 0);
	send_response(sockfd);
}

static void
send_response(int sockfd)
{
	time_t now;
	struct tm now_tm;
	char timebuf[100];
	char buf_fmt[] = "HTTP/1.1 200 OK" CRLF
		"Date: %s" CRLF
		"Server: cyhttp" CRLF
		"Content-Length: %d" CRLF
		"Content-Type: text/html; charset=utf-8" CRLF CRLF
		"%s" CRLF CRLF;
	char *buf;
	char content[] = "<strong>TEST</strong>"; /* TODO */
	now = time(NULL);
	localtime_r(&now, &now_tm);
	strftime(timebuf, 100, "%a, %d %b %Y %H:%M:%S %Z", &now_tm);
	asprintf(&buf, buf_fmt, timebuf, strlen(content), content);
	send(sockfd, buf, strlen(buf), 0);
}
