#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <error.h>
#include <unistd.h>
#include <getopt.h>
#include <termios.h>
#include <fcntl.h>
#include <signal.h>

#define PRINT_HELP printf("CC Controlador de Cruce\n\
Uso: cc -n <serial> -s <servidor> \
-p <puerto servidor> -c <archivo configuracion>\n");

struct termios oldsioc, newsioc;

int serial_open(char *serial_name);
int serial_close(int fd);
void get_conf(char *file_name);
void sigint_handler(int sign);
void tcp_server(void);
void tcp_client(void);

int main(int argc, char *argv[])
{
	char *serial_name = NULL;
	char *udp_server = NULL;
	char *server_port = NULL;
	char *conf_file_name = "conf";

	int s_fd;

	/*	
	if (argc < 5) { 
		PRINT_HELP
		exit(EXIT_FAILURE);
	}
	*/

	while ((tmp = getopt(argc, argv, "n:s:p:c:")) != -1) {
		switch (tmp) {
			case 'n':
				serial_name = optarg;
				break;
			case 's':
				udp_server = optarg;
				break;
			case 'p':
				server_port = optarg;
				break;
			case 'c':
				conf_file_name = optarg;
				break;
			default:
				PRINT_HELP;
				exit(EXIT_FAILURE);
				break;
		}
	}
	
	if (signal(SIGINT, sigint_handler) == SIG_ERR)
  		error(0, errno, "ERROR: Cannot set signal handler");

		
	return 0;
}

int serial_open(char *serial_name)
{
	int fd;
	
	if ((fd = open(serial_name, O_NOCTTY | O_RDWR)) == -1)
		error(EXIT_FAILURE, errno, "ERROR: Cannot open %s ", serial_name);
	
	if (tcgetattr(fd, &oldsioc) == -1)
		error(EXIT_FAILURE, errno, "ERROR: Cannot get terminal attributes");

	newsioc.c_iflag = 0;
	newsioc.c_oflag = 0;
	newsioc.c_cflag = B9600 | CS8 | CREAD | CLOCAL;
	newsioc.c_lflag |= ICANON;
	/*
	newsioc.c_cc[VMIN] = 0;
	newsioc.c_cc[VTIME] = 0;
	*/

	tcflush(fd, TCIOFLUSH);

	if (tcsetattr(fd, TCSANOW, &newsioc) == -1)
		error(EXIT_FAILURE, errno, "ERROR: Cannot set terminal atributes");
	
	return fd;
}

int serial_close(int fd)
{
	int tmp;
	
	if (tcsetattr(fd, TCSANOW, &oldsioc) == -1)
		printf("ERROR: Cannot set terminal attributes\n");

	if ((tmp = close(fd)) == -1) 
		printf("ERROR: Cannot close file descriptor %d\n", fd);
	
	return tmp;
}

void sigint_handler(int sign)
{
	printf("\nQuit? Try again\n");
	signal(SIGINT, SIG_DFL);
}

void get_conf(char *file_name)
{
	FILE *file = NULL;
	char line[50];

	if ((file = fopen(file_name, "r")) == NULL)
		error(EXIT_FAILURE, errno, "Cannot open config file %s", file_name);
	
	while(fgets(line, 50, file) != NULL) {
		if (line[0] != '\n')
			printf("%s", line);
	}

	if (fclose(file) == EOF)
		error(0, errno, "ERROR: Cannot close config file %s", file_name);
}

void tcp_server(void)
{
	int s_sockfd;
	int c_sockfd;
	socklen_t sin_size;
	struct sockaddr_in s_addr;
	struct sockaddr_in c_addr;
	
	s_sockfd = socket(PF_INET, SOCK_STREAM, 0);
	
	memset(&s_addr, 0, sizeof(struct sockaddr_in);
	
	s_addr.sin_family = AF_INET;
	s_addr.sin_port = htons(2000);
	s_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	bind(s_sockfd, (struct sockaddr *)&s_addr, sizeof(struct sockaddr));
	listen(s_sockfd, 1);
	
	sin_size = sizeof(struct sockaddr_in);
	c_sockfd = accept(s_sockfd, (struct sockaddr *)&c_addr, &sin_size);
	
	close(c_sockfd);
	close(s_sockfd);
	
}

void tcp_client(void)
{
	int s_sockfd;
	struct sockaddr_in s_addr;
	
	s_sockfd = socket(PF_INET, SOCK_STREAM, 0);
	
	memset(&s_addr, 0, sizeof(sockaddr_in));
	
	s_addr.sin_family = AF_INET;
	s_addr.sin_port = htons(2000);
	s_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	
	connect(s_sockfd, (struct sockaddr_in *)&s_addr, sizeof(struct sockaddr_in);
	
	close(s_sockfd);
}