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
char *server_port = "2000";
char *conf_file_name = "conf";

int time_flag = 0;
int cicle_time = 5;

struct termios oldsioc, newsioc;

int serial_open();
int serial_close(int fd);
void get_conf();
void sigint_handler(int sign);
void sigalrm_handler(int sign);

int main(int argc, char *argv[])
{
	int tmp;
	int s_fd;
	int sockfd;
	int car_count = 0;
	char serial_buf[256];
	char cad_count[10];
	char cad_cicle[10];
	char msg[256];
	char buf[256];
	struct sockaddr_in s_addr;
		
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
				tcp_server = optarg;
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

	if (signal(SIGALRM, sigalrm_handler) == SIG_ERR)
		error(EXIT_FAILURE, errno, "ERROR: Cannot set alarm signal handler");
	
	s_fd = serial_open();
		
	
	memset(&s_addr, 0, sizeof(s_addr));
	s_addr.sin_family = AF_INET;
	s_addr.sin_port = htons(atoi(server_port));
	s_addr.sin_addr.s_addr = inet_addr(tcp_server);
	
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
		sprintf(cad_count, "%d", car_count);
		sprintf(cad_cicle, "%d", cicle_time);
		strcat(msg, "%1&");
		strcat(msg, cad_count);
		strcat(msg, "#");
		printf("sending: %s\n", msg);
		
		if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1)
			error(EXIT_FAILURE, errno, "Cannot open socket");
		
		if (connect(sockfd, (struct sockaddr *)&s_addr, sizeof(s_addr)) == -1)
			error(0, errno, "ERROR: Cannot connect to te server");
		
		if (send(sockfd, msg, strlen(msg)+1, 0) < 0)
			error(0, errno, "ERROR: sending");
		
		recv(sockfd, buf, sizeof(buf), 0);
		printf("recv: %s\n", buf);
		
		close(sockfd);
		alarm(cicle_time);
	}	
	
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
