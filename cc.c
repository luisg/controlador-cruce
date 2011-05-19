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
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PRINT_HELP printf("CC Controlador de Cruce\n\
Uso: cc -n <serial> -s <servidor> \
-p <puerto servidor> -c <archivo configuracion>\n");

char *serial_name = "/dev/ttyS0";
char *tcp_server = "127.0.0.1";
unsigned short tcp_port = 2000;
char *conf_file_name = "conf";
int id_cruce = 0;

int time_flag = 0;
int cicle_time = 60;

struct termios oldsioc, newsioc;

int serial_open();
int serial_close(int fd);
void get_conf();
void sigint_handler(int sign);
void sigalrm_handler(int sign);
int tcp_sendmsg(struct s_mesg *msg);

int main(int argc, char *argv[])
{
	int tmp;
	int s_fd;
	int car_count = 0;
	char serial_buf[256];
	
	struct s_mesg msg;
	/*	
	if (argc < 5) { 
		PRINT_HELP
		exit(EXIT_FAILURE);
	}
	*/

	while ((tmp = getopt(argc, argv, "n:s:p:c:i:")) != -1) {
		switch (tmp) {
			case 'n':
				serial_name = optarg;
				break;
			case 's':
				tcp_server = optarg;
				break;
			case 'p':
				tcp_port = atoi(optarg);
				break;
			case 'c':
				conf_file_name = optarg;
				break;
			case 'i':
				break;
			default:
				PRINT_HELP;
				exit(EXIT_FAILURE);
				break;
		}
	}
	
	if (signal(SIGINT, sigint_handler) == SIG_ERR)
  		error(0, errno, "ERROR: Cannot set signal handler");

	if (signal(SIGALRM, sigalrm_handler) == SIG_ERR)
		error(EXIT_FAILURE, errno, "ERROR: Cannot set alarm signal handler");
	/*
	printf("sending: \n");
	if (tcp_sendmsg(&msg) < 0)
		perror("tcp_sendmsg");
	
	
	s_fd = serial_open();	
	alarm(cicle_time);
	
	while(1) {
		msg[0] = '\0';
		car_count = 0;
		time_flag = 0;
		
		while(!time_flag) {
			tmp = read(s_fd, serial_buf, sizeof(serial_buf));
			serial_buf[tmp] = '\0';
			printf("%s", serial_buf);
			car_count++;
		}	
			
		tcp_sendmsg(struct mesg);
		
		alarm(cicle_time);
	}	
	*/
		
	return 0;
}

int serial_open()
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

void sigalrm_handler(int sign)
{
	time_flag = 1;
	signal(sign, sigalrm_handler);
}

void get_conf()
{
	FILE *file = NULL;
	char line[50];

	if ((file = fopen(conf_file_name, "r")) == NULL)
		error(EXIT_FAILURE, errno, "Cannot open config file %s", conf_file_name);
	
	while(fgets(line, 50, file) != NULL) {
		if (line[0] != '\n')
			printf("%s", line);
	}

	if (fclose(file) == EOF)
		error(0, errno, "ERROR: Cannot close config file %s", conf_file_name);
}

int tcp_sendmsg(struct s_mesg *msg)
{
	int sockfd;
	int len;

	struct sockaddr_in s_addr;
	
	if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		return -1;
	}
	
	memset(&s_addr, 0, sizeof(s_addr));
	s_addr.sin_family = AF_INET;
	s_addr.sin_port = htons(tcp_port);
	s_addr.sin_addr.s_addr = inet_addr(tcp_server);
	
	if (connect(sockfd, (struct sockaddr *)&s_addr, sizeof(s_addr)) < 0) {
		perror("connect");
		return -1;
	}
	
	if ((len = send(sockfd, msg, sizeof(*msg), 0)) < 0) {
		perror("send");
		return -1;
	}
	
	/* recibir mensaje del servidor */
	
	close(sockfd);
	return 0;
}
