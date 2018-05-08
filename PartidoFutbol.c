#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/ipc.h> 
#include<time.h>
#include<semaphore.h>

void delay(unsigned int mseconds)
{
    clock_t goal = mseconds + clock();
    while (goal > clock());
}


//Para intentar usar mmap más abajo para la variable compartido
//creo que es mejor usar un struct que tenga el char del equipo al que pertenece la bola y un int con el id del proceso que tiene la bola
static char *ball;

int main(){
    sem_t mutex;
    int childs = 10;
    int a_team[5];
    int b_team[5];
    int process_id;
    int parent_process_id;
    int scoreA = 0;
    int scoreB = 0;
   //Aquí se usa mmap. Apenas estuve investigando, todavía no sé como usarlo bien
    ball = mmap(NULL, sizeof*ball, PROT_READ | PROT_WRITE, MAP_SHARED |MAP_ANONYMOUS, -1, 0);
    *ball = 'A';
    //printf("Childs number: ");
    //scanf("%d", &childs);
    int msec = 0, trigger = 10; /* 10ms */
    clock_t before = clock();
    for(int i = 0; i < childs; i++){
        if(fork() == 0){
            int iterations;
            process_id = getpid();
            parent_process_id = getppid();
            /* para saber en cual equipo irá se hace
              si(id del proceso < id del proceso + 5) then
                a equipo a
              sino
                a equipo b
            Por ejemplo: process_id = 18002
                        parent_process_id = 17988
                    process_id es menor que 18003*/
            
            if(process_id <= parent_process_id + 5){
                a_team[((process_id - parent_process_id) % 5) -1] = process_id;
                printf("Proceso: %d va a equipo A\n", a_team[((process_id - parent_process_id) % 5) -1]);
            }
            else{
                b_team[((process_id - parent_process_id) % 5) -1] = process_id;
                printf("Proceso: %d va a equipo B\n", b_team[((process_id - parent_process_id) % 5) -1]);
            }

            for(int i = 0; i < childs; i++){
                delay(100);
                sem_wait(&mutex);
                printf("\nEntered..\n");
                printf("%d\n", i);
                printf("\nJust Exiting...\n");
                sem_post(&mutex);
            }
            return 0;
        }
    }

    
    for(int i = 0; i < childs; i++){
        wait(NULL);

    }

    return 0;
}
