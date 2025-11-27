#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <time.h>
#include <string.h>

#define NUMBER_OF_CHATTIES 20
#define SHARED_MEMORY_NAME "/chatterboxes_shm"

typedef enum { //here we set the conditions of a chatterbox(a chatty person)
    WAITING,
    CALLING, 
    TALKING
} State;

typedef struct { //the structure for keeping the one person status
    State state; //current status
    int talkingTo; //here struct will store sobesednik's ID
    int numberOfCalls;
} PersonStatus;

typedef struct { //struct for data between processes
    int numberOfPeople;
    int running; //flazhok of a working programm, where 1 = works, 0 = not
    PersonStatus people[NUMBER_OF_CHATTIES]; //an array of chatty people's statuses
} SharedData;

//shared resources
SharedData *shared_data = NULL; //pointer to a piece of a shared memory
sem_t *sync_sem = NULL; //semaphore for synchronisation
int shm_fd = -1; //file descriptor for shared memory

void cleanup() {
    printf("Now I am going to clean my memory from yur digits, bits, programms :)\n");
    if(shared_data != NULL) {
        munmap(shared_data, sizeof(SharedData));
        shared_data = NULL;
        printf("Shared memory has been unmapped :)\n");
    }

    if(shm_fd != -1) {
        close(shm_fd);
        shm_unlink(SHARED_MEMORY_NAME);
        shm_fd = -1;
        printf("Shared memory closed up a shop :)\n"); //zakryli lavochky, vse
    }

    if (sync_sem != NULL) {
        sem_destroy(sync_sem);
        free(sync_sem);
        sync_sem = NULL;
        printf("Semaphore has been destroyed :)\n");
    }
}
    //this block is for handling exiting the program via Ctrl+C
    void signal_handler(int sig) {
        printf("\nRecieved signal %d which stops the programm :)\n", sig);
        if (shared_data != NULL) {
            shared_data -> running = 0; //stores the stop flazhok
        }
    }

    void init_shared_memory(int numberOfPeople) { //inits the shared memory
        shm_fd = shm_open(SHARED_MEMORY_NAME, O_CREAT | O_RDWR, 0666); //creates or opens the shared memory object
        if (shm_fd == -1) {
            perror("shm_open failed...ups");
            exit(1);
        }
        if(ftruncate(shm_fd, sizeof(SharedData)) == -1) { //sets the size of a shared memory
            perror("ftruncate failed...ups");
            exit(1);
        }
        
        //mirror the shared memory to the address space of a process
        shared_data = mmap(NULL, sizeof(SharedData), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
        if (shared_data == MAP_FAILED) {
            perror("mmap failed...ups...I'm so sorry...");
            exit(1);
        }

        //inits the data in shared memory
        shared_data->numberOfPeople = numberOfPeople;
        shared_data->running = 1; //prog is working

        //inits the status of all the shatterboxes
        for(int i = 0; i < numberOfPeople; i++) {
            shared_data->people[i].state = WAITING; //starts in condition of waiting
            shared_data->people[i].talkingTo = -1; //nobody talks
            shared_data->people[i].numberOfCalls = 0;
        }
        printf("Shared memory initialized for %d people :))\n", numberOfPeople);
    }

    void init_semaphores() {
        sync_sem = malloc(sizeof(sem_t));
        if (sync_sem == NULL) {
            perror("Malloc for semaphores failed...ups...");
            exit(1);
        }

        if (sem_init(sync_sem, 1, 1) == -1) { //1 = semaphore is alive, 0 = not
            perror("sem_init failed...ups...");
            exit(1);
        }
        printf("Semaphore initialised :))\n");
    }

    int get_random_person(int exclude, int numberOfPeople) {
        int target;
        do {
            target = rand() % numberOfPeople;
        } while (target == exclude);
        return target;
    }

    void chatty_person_process(int id, int numberOfPeople) {
        srand(time(NULL)+ id); //generating random numbers for all processes
        printf("Person %d: process started (PID: %d) :)\n", id, getpid());

        while(1) {
            sem_wait(sync_sem); //catch the semaphore for reading
            int running = shared_data->running;
            sem_post(sync_sem); //DOBBY (semaphore) IS FREE!!

            if (!running) break;

            usleep((rand() % 1000000) + 500000);

            sem_wait(sync_sem);

            if(shared_data->people[id].state == TALKING) {
                sem_post(sync_sem);
                usleep(2000000); //2 seconds of chatting
                continue;
            }

            if(rand() % 2 == 0) {
                shared_data->people[id].state = WAITING;
                printf("Person %d is waiting for calls :)\n", id);
            }
            else {
                int target = get_random_person(id, numberOfPeople);
            

                if(shared_data->people[target].state == WAITING) {
                shared_data->people[id].state = TALKING;
                shared_data->people[target].state = TALKING;
                shared_data->people[id].talkingTo = target;
                shared_data->people[target].talkingTo = id;
                shared_data->people[id].numberOfCalls++;

                printf("Person %d has successfully connected to a person %d :)\n", id, target);
            } else {
                shared_data->people[id].numberOfCalls++;
                printf("Person %d: person %d is busy (state: %d):-)\n", id, target, shared_data->people[target].state);
            }
        }
    
        sem_post(sync_sem);

        if(shared_data->people[id].state == TALKING) {
            sleep(2 + (rand() % 3));

            sem_wait(sync_sem);
            int partner = shared_data->people[id].talkingTo;

            if(partner != -1) {
                shared_data->people[partner].state = WAITING;
                shared_data->people[partner].talkingTo = -1;
            }

            shared_data->people[id].state = WAITING;
            shared_data->people[id].talkingTo = -1;

            printf("Person %d: he has finished conversations with person %d, hurray!! :)\n", id, partner);
            sem_post(sync_sem);
        }
    }
    printf("Person %d: Exiting :)\n", id);
}
    void print_status(int numberOfPeople) {
        printf("Current statuses: \n");
        for (int i = 0; i < numberOfPeople; i++) {
            const char *state_str;
            switch (shared_data->people[i].state) {
                case WAITING: state_str = "WAITING"; break;
                case CALLING: state_str = "CALLING"; break;
                case TALKING: state_str = "TALKING"; break;
                default: state_str = "UNKNOWN"; break;
            }

            printf("Person %-2d: %-7s", i, state_str);
            if (shared_data->people[i].state == TALKING) {
                printf(" (with person %-2d) ", shared_data->people[i].talkingTo);
            } else {
                printf(" ");
            }
            printf("-> Calls %-3d\n", shared_data->people[i].numberOfCalls);
        }
        printf("--------------------------\n");
        printf("\n");
    }

    int main(int argc, char *argv[]) {
        if (argc != 2) {
            fprintf(stderr, "Usage: %s <numberOfPeople>\n", argv[0]);
            fprintf(stderr, "Example: %s 5]n", argv[0]);
            exit(1);
        }

        int num_people = atoi(argv[1]);
        if(num_people < 2 || num_people > NUMBER_OF_CHATTIES) {
            fprintf(stderr, "Number of people must be between 2 and %d\n", NUMBER_OF_CHATTIES);
            exit(1);
        }
        printf("Starting chatterboxes simulation with %d people\n", num_people);
        signal(SIGINT, signal_handler);
        signal(SIGTERM, signal_handler);

        init_shared_memory(num_people);
        init_semaphores();

        pid_t pids[NUMBER_OF_CHATTIES];

        //create filia processes
        printf("Creating %d chatty people processes! ^-^\n", num_people);
        for(int i = 0; i < num_people; i++) {
            pid_t pid = fork();

            if (pid == 0) {
                chatty_person_process(i, num_people);
                exit(0);
            } else if (pid > 0) {
                pids[i] = pid;
            } else {
                perror("fork failed");
                exit(1);
            }
        }

        printf("Simulation started with %d chatty people! :))\n", num_people);
        printf("Remember, you can always press Ctrl+C to stop the simulation prog!\n");

        int status_interval = 0;
        while(shared_data->running) {
            sleep(5);
            status_interval++;

            if (status_interval >=3) {
                sem_wait(sync_sem);
                print_status(num_people);
                sem_post(sync_sem);
                status_interval = 0;
            }

            for(int i = 0; i < num_people; i++) {
                if (waitpid(pids[i], NULL, WNOHANG) > 0) {
                    printf("Person's %d process died, ok, let it be\n", i);
                    shared_data->running = 0;
                }
            }
        }
        printf("\n!!Stopping simulation!!\n");
        for (int i = 0; i < num_people; i++) {
            kill(pids[i], SIGTERM);
        }
        for(int i = 0; i < num_people; i++) {
            waitpid(pids[i], NULL, 0);
        }
        cleanup();
        printf("Simulation stopped successfully!! ^-^\n");
        return 0;
    }




