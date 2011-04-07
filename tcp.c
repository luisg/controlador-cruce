/* http://www.csc.villanova.edu/~mdamian/Sockets/TcpSockets.htm
 * http://www.chuidiang.com/clinux/sockets/sockets_simp.php
 * http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html#select
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <error.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdint.h>

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
	int recv_bytes;
	int i;
	char recv_buf[1024];

	socklen_t sin_size;
	struct sockaddr_in myaddr;
	struct sockaddr_in client_addr;
	
	memset(&myaddr, 0, sizeof(myaddr));
	
	if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1)
		error(EXIT_FAILURE, errno, "ERROR: Cannot open socket");
	
	myaddr.sin_family = AF_INET;
	myaddr.sin_port = htons(2000);
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	if ((bind(sockfd, (struct sockaddr *)&myaddr, sizeof(myaddr)) == -1))
		error(EXIT_FAILURE, errno, "ERROR: Cannot bind socket");
	
	if ((listen(sockfd, 1) == -1))
		error(EXIT_FAILURE, errno, "ERROR: Cannot listen");
	
	printf("listening on port 2000...\n"
		   "waiting for connections...\n");
	
	while(1) {
		sin_size = sizeof(client_addr);
		client_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &sin_size);
		if (client_sockfd == -1)
			error(0, errno, "ERROR: Cannot get new connection");
		
		recv_bytes = recv(client_sockfd, &recv_buf, sizeof recv_buf, 0);
		if (recv_bytes == 0) {
			printf("Connection closed by client\n");
			continue;
		}
		else if (recv_bytes == -1)
			error(EXIT_FAILURE, errno, "ERROR: reciving data");
		
		printf("recv %d bytes from: %s\n", recv_bytes, inet_ntoa(client_addr.sin_addr));
		printf("data: ");
		
		for (i = 0; i < recv_bytes; ++i) {
			printf("%d ", recv_buf[i]); 
		}
		printf("\n");
		close(client_sockfd);	
	}
	
	return 0;
}

int tcp_client(char *msg)
{	
	int sockfd;
	int num = 10;
	struct sockaddr_in raddr;
	
	memset(&raddr, 0, sizeof(raddr));
	
	if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1)
		error(EXIT_FAILURE, errno, "ERROR: Cannot open socket");
	
	raddr.sin_family = AF_INET;
	raddr.sin_port = htons(2000);
	raddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	
	if ((connect(sockfd, (struct sockaddr *)&raddr, sizeof raddr)) == -1)
		error(EXIT_FAILURE, errno, "ERROR: Cannot connect to remote server");
	
	if (send(sockfd, msg, sizeof(msg), 0) == -1)
		error(EXIT_FAILURE, errno, "ERROR: Cannot send message to remote host");
	
	close(sockfd);
	
	return 0;
}

