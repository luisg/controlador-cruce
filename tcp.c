/* http://www.csc.villanova.edu/~mdamian/Sockets/TcpSockets.htm
 * http://www.chuidiang.com/clinux/sockets/sockets_simp.php
 * http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html#select
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int tcp_client(char *msg);
int tcp_server();

int main(int argc, char *argv[])
{
	
	if (argc == 1) {
		tcp_server();
	} 
	
	tcp_client(argv[1]);
		
	return 0;
}

int tcp_server()
{
	int sockfd;
	int client_sockfd;
	char buf[256];
	socklen_t sin_size;
	struct sockaddr_in myaddr;
	struct sockaddr_in client_addr;
	
	memset(&myaddr, 0, sizeof(myaddr));
	
	sockfd = socket(PF_INET, SOCK_STREAM, 0);
	
	myaddr.sin_family = AF_INET;
	myaddr.sin_port = htons(2000);
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	bind(sockfd, (struct sockaddr *)&myaddr, sizeof(myaddr));
	
	listen(sockfd, 1);
	
	printf("listening on port 2000...\n\
	waiting for connections...\n");
	
	while(1) {
		sin_size = sizeof(client_addr);
		client_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &sin_size);
		recv(client_sockfd, buf, sizeof(buf), 0);
		printf("recv: %s from %s\n", buf, inet_ntoa(client_addr.sin_addr));
		
		send(client_sockfd, buf, sizeof(buf), 0);	
		close(client_sockfd);	
	}
	
	return 0;
}

int tcp_client(char *msg)
{	
	int sockfd;
	struct sockaddr_in raddr;
	
	memset(&raddr, 0, sizeof(raddr));
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
	raddr.sin_family = AF_INET;
	raddr.sin_port = htons(2000);
	raddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	
	connect(sockfd, (struct sockaddr *)&raddr, sizeof(raddr));
	
	send(sockfd, msg, strlen(msg) + 1, 0);
	
	close(sockfd);
	
	return 0;
}
