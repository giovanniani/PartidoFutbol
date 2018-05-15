/********************************************************************************
Instituto Tecnológico de Costa Rica
Principios de Sistemas Operativos
Profesor: Esteban Arias
Laboratorio de Forks
Estudiantes:
    Myron Camacho Brenes 2013034267
    Giovanni Villalobos

Códigos de referencia: https://gist.github.com/junfenglx/7412986 Autor: junfenglx
*/

//Para compilar se recomienda hacer gcc Soccer_Game2.c -lpthread -o Soccer_Game2
// y para ejecutar ./Soccer_Game2



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
#include <signal.h>

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
    time_t*start_time = mmap(NULL, sizeof(time_t), PROT_READ | PROT_WRITE,
                    MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    *start_time = time(NULL);
    int childs = 10;
    pid_t child;
    int*scoreA = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE,
                    MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    int*scoreB = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE,
                    MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    *scoreA = 0;
    *scoreB = 0;
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
                printf("Proceso: %d, hijo de %d, va a equipo A\n", getpid(), getppid());
            }
            else{
                printf("Proceso: %d, hijo de %d, va a equipo B\n", getpid(), getppid());
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

        munmap(scoreA, sizeof(int));
        munmap(scoreB, sizeof(int));

        return 0;
    }
    else{
        while(1){
            if(time(NULL) - *start_time == 300){
                printf("Juego terminado\n");
                printf("Marcador final-> A: %d - B: %d\n", *scoreA, *scoreB);
                munmap(match_ball, sizeof(struct ball));
                munmap(goal_team_A, sizeof(struct goal));
                munmap(goal_team_B, sizeof(struct goal));

                munmap(mutex, sizeof(sem_t));
                munmap(mutex_goal_A, sizeof(sem_t));
                munmap(mutex_goal_B, sizeof(sem_t));

                munmap(scoreA, sizeof(int));
                munmap(scoreB, sizeof(int));
                kill(child, SIGKILL);
                exit(0);
            }
        time_t t;
        srand((unsigned) time(&t));
        int randomnumber;
        randomnumber = rand() % 21;
        printf("Tiempo de espera: %d\n", randomnumber);
        //sleep(randomnumber);
        delay((randomnumber*10000));
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
        //printf("proceso actual = %d \n", getpid());
        printf("La bola le pertenece a  = %d del equipo %c\n", match_ball->pID, match_ball->team);
        for(int i = 0; i < 3; ++i){
            sem_wait(mutex_goal_B);
            //sem_wait(mutex_goal_B);
            if(match_ball->team == 'A' && match_ball->pID == getpid()){
                goal_team_B->pID = getpid();
            }
            else if(match_ball->team == 'B' && match_ball->pID == getpid()){
                goal_team_A->pID = getpid();
            }
            sem_post(mutex_goal_B);
            //sem_post(mutex_goal_B);
        }
        sleep(1);
        sem_post(mutex);
        if(match_ball->team == 'A' && goal_team_B->pID == match_ball->pID){
            printf("El proceso %d va a rematar a la cancha perteneciente a B\n", match_ball->pID);
            *scoreA += 1;
            printf("Gooooool! de %c anotado por %d\n", match_ball->team, match_ball->pID);
            printf("Marcador: A: %d - B: %d\n",*scoreA, *scoreB);
        }
        else if(match_ball->team == 'B' && goal_team_A->pID == match_ball->pID){
            printf("El proceso %d va a rematar a la cancha perteneciente a A\n", match_ball->pID);   
            *scoreB += 1;
            printf("Gooooool! de %c anotado por %d\n", match_ball->team, match_ball->pID);
            printf("Marcador: A: %d - B: %d\n",*scoreA, *scoreB);
        }
        else{
            printf("El proceso %d no pudo anotar\n", match_ball->pID);
        }
        sleep(5);
        }
        
    }
}