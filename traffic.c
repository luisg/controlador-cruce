#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

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

struct cruce cr;

int cicle_counter = 0;

void sigalrm_handler(int sign);
void doit(void);
void reconfigure(char *conf_cr);
void print_info(void);

int main(int argc, char *argv[])
{	
	int i = 0;
	
	cr.n_sem = 4;
	cr.id_cruce = 1;
	cr.yellow_time = 2;
	
	reconfigure("10&10&10&10");
	
	cr.sems[0].id_sem = 1;
	cr.sems[0].status = GREEN;
	cr.sems[0].counter = cr.sems[i].green_time;
	
	for (i = 1; i < cr.n_sem; ++i) {
		cr.sems[i].id_sem = i;
		cr.sems[i].status = RED;
		cr.sems[i].counter = cr.sems[i].red_time;
	}
	
	
	
	if (signal(SIGALRM, sigalrm_handler) == SIG_ERR)
		printf("ERROR: Cannot set alarm signal handler\n");
	
	alarm(1);
	
	while(1) {
		
	}
		
	return 0;
}

void sigalrm_handler(int sign)
{
	doit();
	signal(sign, sigalrm_handler);
	alarm(1);
}

void doit(void)
{
	int i = 0;
	
	if (cicle_counter == 0) {
		cicle_counter = cr.cicle_time;
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
						cr.sems[i].counter = cr.sems[i].red_time;
						break;
					case RED:
						cr.sems[i].status = GREEN;
						cr.sems[i].counter = cr.sems[i].green_time;
						break;
				}
			}
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
	
	/*
	cr.sems[1].id_sem = 1;
	cr.sems[1].green_time = 10;
	cr.sems[1].red_time = 12;
	cr.sems[1].status = RED;
	cr.sems[1].counter = cr.sems[1].red_time;
	*/	
	
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