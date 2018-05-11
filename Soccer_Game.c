/********************************************************************************
Códigos de referencia: https://gist.github.com/junfenglx/7412986 Autor: junfenglx
*/

//Para compilar se recomienda hacer gcc Soccer_Game.c -lpthread -o Soccer_Game
// y para ejecutar ./Soccer_Game

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/ipc.h> 
#include<time.h>
#include<semaphore.h>
#include <errno.h> /* errno, ECHILD            */

#define SEMAPHORES 1

void delay(unsigned int mseconds)
{
    clock_t goal = mseconds + clock();
    while (goal > clock());
}

//Estructuras para las variables compartidas
struct ball
{
    int pID;
    char team;
} match_ball = {
    0,
    'A'
};

struct goal{
    int pID;
}goal_team_A ={
    0,
},
goal_team_B = {
    0,
};


int main(){
	int childs = 10;
	pid_t child;
	int*scoreA = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE,
                    MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	int*scoreB = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE,
                    MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	scoreA = 0;
	scoreB = 0;
	//Se crean los recursos compartidos(la bola y las canchas)
    struct ball* match_ball = mmap(NULL, sizeof(struct ball), PROT_READ | PROT_WRITE, MAP_SHARED |MAP_ANONYMOUS, -1, 0);
    struct goal* goal_team_A = mmap(NULL, sizeof(struct goal), PROT_READ | PROT_WRITE, MAP_SHARED |MAP_ANONYMOUS, -1, 0);
    struct goal* goal_team_B = mmap(NULL, sizeof(struct goal), PROT_READ | PROT_WRITE, MAP_SHARED |MAP_ANONYMOUS, -1, 0);

    //Aquí se crean los semáforos de manera compartida
    sem_t*mutex=mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE,
                    MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    sem_t*mutex_goal_A=mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE,
                    MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    sem_t*mutex_goal_B=mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE,
                    MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    sem_init(mutex, 1, 1);
    sem_init(mutex_goal_A, 1, 1);
    sem_init(mutex_goal_B, 1, 1);
    //Ciclo para la creación de todos los hijos
    for(int i = 0; i < childs; i++){
    	child = fork();
    	if(child < 0){
    		printf("Fork error\n");
    	}
    	else if(child == 0){
            /* para saber en cual equipo irá se hace
              si(id del proceso < id del proceso + 5) then
                a equipo a
              sino
                a equipo b
            Por ejemplo: process_id = 18002
                        parent_process_id = 17988
                    process_id es menor que 18003*/
            
            if(getpid() <= getppid() + 5){
                printf("Proceso: %d va a equipo A\n", getpid());
            }
            else{
                printf("Proceso: %d va a equipo B\n", getpid());
            }
            sleep(1);
    		break;
    	}
    }

    if(child != 0){
    	while(child = waitpid(-1, NULL, 0)){
    		if(errno == ECHILD){
    			break;
    		}
    	}
    	printf("Todos los procesos hijos han acabado\n");
    	//Se destruyen las variables compartidas y los semáforos
    	munmap(match_ball, sizeof(struct ball));
    	munmap(goal_team_A, sizeof(struct goal));
    	munmap(goal_team_B, sizeof(struct goal));

    	munmap(mutex, sizeof(sem_t));
    	munmap(mutex_goal_A, sizeof(sem_t));
    	munmap(mutex_goal_B, sizeof(sem_t));

    	return 0;
    }
    else{
		sem_wait(mutex);
		if(getpid() <= getppid() + 5){
    		match_ball->pID = getpid();
    		match_ball->team = 'A';
    	}
    	else if(getpid() > getppid() + 5){
    		match_ball->pID = getpid();
    		match_ball->team = 'B';
    	}
    	sleep(1);
    	printf("El proceso %d del equipo %c tiene el balon\n", match_ball->pID, match_ball->team);
    	sem_post(mutex);

    	sem_wait(mutex_goal_A);
    	sem_wait(mutex_goal_B);
    	sleep(1);
    	if(getpid() == match_ball -> pID && match_ball->team == 'B'){
    		printf("Hello\n");
    		scoreB += 1;
    	}
    	else if(getpid() == match_ball -> pID && match_ball->team == 'A'){
    		scoreA += 1;
    	}
    	printf("Goool!! del equipo %c. Anotado por %d\n", match_ball->team, match_ball->pID);
    	printf("Marcador- A: %d - B: %d\n", scoreA, scoreB);
    	sem_post(mutex_goal_B);
    	sem_post(mutex_goal_A);
    	exit(0);
    }
}