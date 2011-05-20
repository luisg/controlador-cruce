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
#include <pthread.h>

#define PRINT_HELP printf("CC Controlador de Cruce\n\
Uso: cc -n <serial> -s <servidor> \
-p <puerto servidor> -c <archivo configuracion>\n");

#define GREEN 0
#define RED 1
#define YELLOW 2

struct semaforo {
	int id_sem;
	int green_time;
	int red_time;
	int counter;
	int status;
};

struct cruce {
	int cicle_time;
	int n_sem;
	int id_cruce;
	int yellow_time;
	struct semaforo sems[4];
};

char *serial_name = "/dev/ttyS0";
char *tcp_server = "127.0.0.1";
unsigned short tcp_port = 2000;
char *conf_file_name = "conf";

int cicle_counter = 0;

struct termios oldsioc, newsioc;
struct cruce cr;

int serial_open();
int serial_close(int fd);

void get_conf();

void sigint_handler(int sign);
void sigalrm_handler(int sign);

int tcp_sendmsg(char *msg, char *recvmsg);

void doit(void);
void reconfigure(char *conf_cr);
void print_info(void);

int main(int argc, char *argv[])
{
	int tmp;
	int s_fd;
	char serial_buf[256];
	
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
	
	cr.n_sem = 4;
	cr.id_cruce = 1;
	cr.yellow_time = 2;
	
	reconfigure("10&10&10&10");
	
	alarm(1);
	
	//s_fd = serial_open();	
	while(1){
		;
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
	doit();
	signal(sign, sigalrm_handler);
	alarm(1);
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

int tcp_sendmsg(char *msg, char *recvmsg)
{
	int sockfd;
	int len;
	int recv_bytes;
	char recv_buf[256];
	
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
	
	recv_bytes = recv(sockfd, &recv_buf, sizeof recv_buf, 0);
	if (recv_bytes == 0) {
		printf("Connection closed by client\n");
		exit(-1);
	} else if (recv_bytes == -1)
		perror("recv");
	
	recvmsg = (char*)malloc(recv_bytes);
	
	close(sockfd);
	return 0;
}

void doit(void)
{
	int i = 0;
	
	if (cicle_counter == 0) {
		reconfigure("10&10&10&10");
		//cicle_counter = cr.cicle_time;
	} else {
		cicle_counter--;
		for (i = 0; i < cr.n_sem; ++i) {
			if (cr.sems[i].counter == 0) {
				switch(cr.sems[i].status) {
					case GREEN:
						cr.sems[i].status = YELLOW;
						cr.sems[i].counter = cr.yellow_time;
						break;
					case YELLOW:
						cr.sems[i].status = RED;
						cr.sems[i].counter = -1;
						break;
					case RED:
						cr.sems[i].status = GREEN;
						cr.sems[i].counter = cr.sems[i].green_time;
						break;
				}
			}
			if (cr.sems[i].counter != -1)
				cr.sems[i].counter--;
		}
	}
	
	print_info();
}	

void reconfigure(char *conf_cr) 
{
	int i = 1;
	int rt = 0;
	char *tmp_conf;
	char *cad;

	tmp_conf = malloc(sizeof(*conf_cr));
	strcpy(tmp_conf, conf_cr);
	printf("%s\n", tmp_conf);
	
	cad = strtok(tmp_conf, "&");
	rt = (cr.sems[0].green_time = atoi(cad)) + cr.yellow_time;
	
	while ((cad = strtok(NULL, "&")) != NULL){
		cr.sems[i].red_time = rt;
		rt += (cr.sems[i++].green_time = atoi(cad)) + cr.yellow_time;
	}
	
	cr.sems[0].red_time = rt;
	cr.sems[0].status = GREEN;
	cr.sems[0].counter = cr.sems[0].green_time;
	
	for (i = 1; i < cr.n_sem; ++i) {
		cr.sems[i].status = RED;
		cr.sems[i].counter = cr.sems[i].red_time;
	}	
	/*Calcula el tiempo de Ciclo (tiempos en verde + tiempos en amarillo)*/
	cr.cicle_time = 0;
	for (i = 0; i < cr.n_sem; ++i) {
		cr.cicle_time += cr.sems[i].green_time;
	}
	
	cr.cicle_time += (cr.yellow_time * cr.n_sem);
	cicle_counter = cr.cicle_time;
}

void print_info(void)
{
	int i;
	
	system("clear");
	printf("Tiempo de Ciclo = %d\n", cicle_counter);
	printf("  S0     S1      S2     S3 \n");
	
	for (i = 0; i < cr.n_sem; i++) {	
		switch (cr.sems[i].status) {
			case GREEN:
				printf("V : ");
				break;
			case RED: 
				printf("R : ");
				break;
			case YELLOW:
				printf("A : ");
				break;
		}
		printf("%d    ", cr.sems[i].counter);
	}
	
	printf("\n\n");
}
