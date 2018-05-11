#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/ipc.h> 
#include<time.h>
#include<semaphore.h>

#define SEMAPHORES 1

void delay(unsigned int mseconds)
{
    clock_t goal = mseconds + clock();
    while (goal > clock());
}


//Para intentar usar mmap más abajo para la variable compartido
//creo que es mejor usar un struct que tenga el char del equipo al que pertenece la bola y un int con el id del proceso que tiene la bola
struct ball
{
    int pID;
    char team;
} match_ball = {
    0,
    'A'
};

int main(){
    int childs = 10;
    int a_team[5];
    int b_team[5];
    int process_id;
    int parent_process_id;
    int scoreA = 0;
    int scoreB = 0;
   //Aquí se usa mmap. Apenas estuve investigando, todavía no sé como usarlo bien
    struct ball* match_ball = mmap(NULL, sizeof(struct ball), PROT_READ | PROT_WRITE, MAP_SHARED |MAP_ANONYMOUS, -1, 0);
    //Aquí se crea el semáforo de manera compartida
    sem_t*mutex=mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE,
                    MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    int msec = 0, trigger = 10; /* 10ms */
    clock_t before = clock();
    sem_init(mutex, 1, 1);
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
            //match_ball->team = 'A';
            //printf("%c\n", match_ball->team);
            
            //for(int i = 0; i < childs; i++){
                if(SEMAPHORES){
                  sem_wait(mutex);
                   if(process_id <= parent_process_id + 5)
                     match_ball -> team = 'A';   
                   else
                     match_ball -> team = 'B';   
                   match_ball -> pID = process_id;
                    printf("la bola pertenece a : %d, del equipo %c \n", match_ball->pID, match_ball->team);
                   //printf("i = %d, %d \n", i, process_id);
                   sleep(1);
                   sem_post(mutex);
                }
                //sleep(2);
                //sem_post(mutex);

                
            //}
            return 0;
        }
    }

    
    for(int i = 0; i < childs; i++){
        wait(NULL);

    }

    munmap(match_ball, sizeof(struct ball));
    return 0;
}
