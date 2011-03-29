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
int serial_read(int fd);
void sigint_handler(int sign);

int main(int argc, char *argv[])
{
	char *serial_name = NULL;
	char *udp_server = NULL;
	char *server_port = NULL;
	char *config_file = cc.conf;
	
	int sfd, tmp;
	
	FILE *conf_file = NULL;

	if (argc < 5) { 
		PRINT_HELP
		exit(EXIT_FAILURE);
	}
	
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
				config_file = optarg;
				break;
			default:
				PRINT_HELP;
				exit(EXIT_FAILURE);
				break;
		}
	}
	
	printf("Serial Name = %s \n", serial_name);
	printf("UDP Server = %s\n", udp_server);
	printf("Server Port = %s\n", server_port);
	
	if (signal(SIGINT, sigint_handler) == SIG_ERR)
  		error(0, errno, "ERROR: Cannot set signal handler");
	
	if ((conf_file = fopen("config", "r")) == NULL)
		error(-1, errno, "Cannot open config file %s\n", config_file);
	
	while(!feof(conf_file)) {
		
	}


	if (fclose(conf_file) == EOF)
		error(0, errno, "ERROR: Cannot close config file %s\n", conf_file);
	
	/*
	if ((sfd = serial_open(serial_name)) == -1);
		exit(EXIT_FAILURE);
	*/
	
	return 0;
}

int serial_open(char *serial_name)
{
	int fd;
	
	if ((fd = open(serial_name, O_NOCTTY | O_RDWR)) == -1) {
		error(0, errno, "ERROR: Cannot open %s", serial_name);
		return fd;
	}
	
	if (tcgetattr(fd, &oldsioc) == -1) {
		error(0, errno, "ERROR: Cannot get terminal attributes")
		return -1;
	}

	newsioc.c_iflag = 0;
	newsioc.c_oflag = 0;
	newsioc.c_cflag = B9600 | CS8 | CREAD | CLOCAL;
	newsioc.c_lflag |= ICANON;
	/*
	newsioc.c_cc[VMIN] = 0;
	newsioc.c_cc[VTIME] = 0;
	*/

	tcflush(fd, TCIOFLUSH);

	if (tcsetattr(fd, TCSANOW, &newsioc) == -1) {
		error(0, errno, "ERROR: Cannot set terminal atributes");
		return -1;
	}

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
