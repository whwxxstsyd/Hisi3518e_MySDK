#include "socket.h"
#include "debug.h"
#include "wifi.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void *session_proc(void *arg)
{
	int ret;
	char recv_buf[1024] = {0};
	int fd = (int)arg;

	while (1)
	{
		ret = recv(fd, recv_buf, sizeof(recv_buf), 0);
		if (ret <= 0)
		{
			printf("recv fail[%d]\n", ret);
			break;
		}
		else
			printf("recv data\n");
	}

	return NULL;
}

int main(int argc, char *argv[])
{
	pthread_t thread_id;
	struct sockaddr_in remode_addr;	
	int listen_fd, new_fd, addr_len = sizeof(struct sockaddr_in);
	

	listen_fd = tcp_create_and_listen(3322);
	while (1)
	{
		if ((new_fd = accept(listen_fd, (struct sockaddr *)&remode_addr, (socklen_t *)&addr_len)) > 0)
		pthread_create(&thread_id, NULL, session_proc, (void *)new_fd);
	}

	return 0;
}

