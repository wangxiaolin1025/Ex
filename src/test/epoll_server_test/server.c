#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "http_epoll.h"


void connfd_callback(int epfd, int epoll_fd, struct event *ev)
{
	struct sockaddr_in *addr;
	char buff[1024] = {0};
	int n;
	addr = (struct sockaddr_in *)ev->arg;
	char *ip;
	ip = inet_ntoa(addr->sin_addr);
	
	n = read(epoll_fd, buff, 1024);
	printf("from ip:%s =========>%s", ip, buff);
	return;
}

void listen_callback(int epfd, int epoll_fd, struct event *ev, void *arg)
{
	struct event *tmp;
	int conn_fd;
	struct sockaddr_in *client_addr;
	socklen_t len;
	len = sizeof(struct sockaddr);
	
	client_addr = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));
	if(client_addr == NULL){
		return;
	}
		
	if((conn_fd = accept(epoll_fd, (struct sockaddr *)client_addr, &len)) < 0) {
			fprintf(stderr, "accept error!");
			return ;
	}

	tmp = event_set(epfd, conn_fd, EPOLLIN, connfd_callback, (void *)&client_addr);
}



int main(int argc, char *argv[])
{
	int listen_fd;
	int epfd;
	
	struct event *e;
	
	struct sockaddr_in server_addr;
	socklen_t len;
	if(3 != argc) {
		fprintf(stderr, "Usage: %s <server_ip> <server port>", argv[0]);
		exit(1);
	}

	listen_fd = socket(AF_INET, SOCK_STREAM, 0);	
	if(listen_fd < 0) {
		fprintf(stderr, "create socket error!");
		exit(1);
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr =  
	server_addr.sin_port = htons(argv[2]);
	
	len = sizeof(server_addr);

	if(bind(listen_fd, (struct sockaddr *)&server_addr, len) < 0) {
		fprintf(stderr, "bind error!");
		exit(1);
	}
	
	epfd = event_init();
	
	e = event_set(epfd, listen_fd, EPOLLIN, listen_callback);	
	
	event_dispatch_loop(epfd);
	
	return 0;
}