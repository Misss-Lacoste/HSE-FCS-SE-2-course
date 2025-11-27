#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <time.h>
#include <string.h>

#define NUMBER_OF_CHATTIES 20
#define SHARED_MEMORY_NAME "/chatterboxes_shm"
#define SEM_SYNC_SEM_NAME "/chatty_sync_sem_2" //it is named semaphore for mutual exclusion
#define SEM_RUNNING_NAME "/chatty_running_2" //named semaphore for flazhok of work

typedef enum { //here we set the conditions of a chatterbox(a chatty person)
    WAITING,
    CALLING, 
    TALKING
} State;

typedef struct { //the structure for keeping the one person status
    State state; //current status
    int talkingTo; //here struct will store sobesednik's ID
    int numberOfCalls; //ochev what it is
} PersonStatus;

typedef struct { //struct for data between processes
    int numberOfPeople;
    int running; //flazhok of a working programm, where 1 = works, 0 = not
    PersonStatus people[NUMBER_OF_CHATTIES]; //an array of chatty people's statuses
} SharedData;

//shared resources
SharedData *shared_data = NULL;  //pointer to a piece of a shared memory
sem_t *sync_sem = NULL; //named semaphore for synchronisation
sem_t *running_sem = NULL; //file descriptor for shared memory
int shm_fd = -1;

void cleanup() { //func for cleaning the resources after the program is closed
    printf("And now I am a powerful machine going to clean the memory from your digits, bits, programms! *-*\n");
    if (shared_data != NULL) { //frees the shared memory
        munmap(shared_data, sizeof(SharedData)); //unmap the shared memory
        shared_data = NULL;
        printf("Shared memory unmapped! :)\n");
    }
    
    if (shm_fd != -1) { //closes and deletes the shared memory 
        close(shm_fd);
        shm_unlink(SHARED_MEMORY_NAME); //here shared memory closes
        shm_fd = -1;
        printf("Shared memory has been closed and unlinked! :)\n");
    }
    
    if (sync_sem != NULL) { //close and delete the named semaphores
        sem_close(sync_sem); //close access to semaphore
        sem_unlink(SEM_SYNC_SEM_NAME); //delete the semaphore from the system
        sync_sem = NULL;
        printf("Sync_semaphore has benn closed and unlinked! :)\n");
    }
    
    if (running_sem != NULL) {
        sem_close(running_sem);
        sem_unlink(SEM_RUNNING_NAME);
        running_sem = NULL;
        printf("Running semaphore has been closed and unlinked! :)\n");
    }
}

//signal handler in case of pressing Ctrl+C
void signal_handler(int sig) {
    printf("\nReceived signal %d, stopping simulation...\n", sig);
    if (running_sem != NULL) {
        sem_wait(running_sem); //catch the semaphore for safe change of a flag
        if (shared_data != NULL) {
            shared_data->running = 0; //here we set the flag of the stop
        }
        sem_post(running_sem); //DOBBY IS FREE!
    }
    cleanup();
    exit(0);
}

void init_shared_memory(int numberOfPeople, int create) { //open or create the shared memory
    if (create) {
        shm_fd = shm_open(SHARED_MEMORY_NAME, O_CREAT | O_RDWR, 0666); //controller creates the new shared_memory
    } else {
        shm_fd = shm_open(SHARED_MEMORY_NAME, O_RDWR, 0666);
    }
    
    if (shm_fd == -1) {
        perror("shm_open failed...");
        exit(1);
    }
     //set the size of the shared_memory
    if (create && ftruncate(shm_fd, sizeof(SharedData)) == -1) {
        perror("ftruncate failed");
        exit(1);
    }
    
    //free the shared memory to the address space of a process
    shared_data = mmap(NULL, sizeof(SharedData), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_data == MAP_FAILED) {
        perror("mmap failed...");
        exit(1);
    }
    
    //inits the data only when it is created by the controller
    if (create) {
        shared_data->numberOfPeople = numberOfPeople;
        shared_data->running = 1; //prog is working
        

        //inits the statuses of all the chatty people
        for (int i = 0; i < numberOfPeople; i++) {
            shared_data->people[i].state = WAITING; //all starts in a waiting condition
            shared_data->people[i].talkingTo = -1; //nobody talks with anybody
            shared_data->people[i].numberOfCalls = 0; //0 attempts to call smbd
        }
        printf("Shared memory initialized for %d people! ^-^\n", numberOfPeople);
    }
}

//initialisation of named semaphores
void init_semaphores(int create) {
    if (create) { //open the named semaphores
        sync_sem = sem_open(SEM_SYNC_SEM_NAME, O_CREAT, 0666, 1); //controller creates initial value == 1
        running_sem = sem_open(SEM_RUNNING_NAME, O_CREAT, 0666, 1);
    } else { //and here chatterboxes open existing semaphores
        sync_sem = sem_open(SEM_SYNC_SEM_NAME, 0);
        running_sem = sem_open(SEM_RUNNING_NAME, 0);
    }
    
    if (sync_sem == SEM_FAILED || running_sem == SEM_FAILED) {
        perror("sem_open failed...");
        exit(1);
    }
    printf("Semaphores have been initialised!! :)\n");
}

//func to get random chatterboxe's id(excluding yourself)
int get_random_person(int exclude, int numberOfPeople) {
    int target;
    do {
        target = rand() % numberOfPeople;
    } while (target == exclude);
    return target;
}

void chatty_person_process(int id, int numberOfPeople) {
    srand(time(NULL) + id);
    
    printf("Person %d started\n", id);
    
    while (1) {
        //check if we should stop
        sem_wait(running_sem);
        int running = shared_data->running;
        sem_post(running_sem);
        if (!running) break; //free the semaphore
        
        //random delay
        usleep((rand() % 1000000) + 500000);
        
        sem_wait(sync_sem); //catch the sem for changing data
        
        //skip if smbd is already talking
        if (shared_data->people[id].state == TALKING) {
            sem_post(sync_sem);
            usleep(2000000); //2 seconds of a chat
            continue;
        }
        
        //decide action : to call or to wait
        if (rand() % 2 == 0) {
            // Wait for call
            shared_data->people[id].state = WAITING;
            printf("Person %d: Waiting for calls ^-^\n", id);
        } else {
            //trying to call someone
            int target = get_random_person(id, numberOfPeople);
            
            if (shared_data->people[target].state == WAITING) {
                //successfully connected - they start chatting
                shared_data->people[id].state = TALKING;
                shared_data->people[target].state = TALKING;
                shared_data->people[id].talkingTo = target;
                shared_data->people[target].talkingTo = id;
                shared_data->people[id].numberOfCalls++;
                
                printf("Person %d: Successfully connected to person %d, hurray!! :))\n", id, target);
            } else {
                //Target is busy
                shared_data->people[id].numberOfCalls++;
                printf("Person %d: Person %d is busy, unfortunately :(\n", id, target);
            }
        }
        
        sem_post(sync_sem); //free the semaphore after changing the data
        
        //simulates conversation
        if (shared_data->people[id].state == TALKING) {
            sleep(2 + (rand() % 3)); //they chat in general for 3 srconds
            
            sem_wait(sync_sem);
            int partner = shared_data->people[id].talkingTo;
            if (partner != -1) { //stop the conversation, TABU!
                shared_data->people[partner].state = WAITING;
                shared_data->people[partner].talkingTo = -1;
            }
            shared_data->people[id].state = WAITING;
            shared_data->people[id].talkingTo = -1;
            printf("Person %d: has finished conversation with person %d *-*\n", id, partner);
            sem_post(sync_sem);
        }
    }
    
    printf("Person %d: I am exiting, goodbye!\n", id);
}

//controller process - regulates the simulation
void controller_process(int numberOfPeople) {
    signal(SIGINT, signal_handler); //registrate the handler of signals for controller
    
    printf("wow! Controller started with %d people! :)\n", numberOfPeople);
    printf("Remember, you can always stop the process by pressing Ctrl+C! \n");
    
    int status_interval = 0;
    while (1) {
        sleep(5); //between the checks give the 5 second relax
        status_interval++;
        
        sem_wait(running_sem); //check the flag of work with syncronisation
        int running = shared_data->running;
        sem_post(running_sem);
        if (!running) break;
        
        if (status_interval >= 3) {
            sem_wait(sync_sem);
            printf("\nStatus Updatings:\n");
            for (int i = 0; i < numberOfPeople; i++) {
                const char *state_str;
                switch (shared_data->people[i].state) {
                    case WAITING: state_str = "WAITING"; break;
                    case CALLING: state_str = "CALLING"; break;
                    case TALKING: state_str = "TALKING"; break;
                    default: state_str = "UNKNOWN"; break;
                }
                
                printf("Person %d: %s", i, state_str);
                if (shared_data->people[i].state == TALKING) {
                    printf(" (with person %d)", shared_data->people[i].talkingTo);
                }
                printf("Calls: %d\n", shared_data->people[i].numberOfCalls);
            }
            printf("---------------\n\n");
            sem_post(sync_sem);
            status_interval = 0;
        }
    }
    
    printf("Controller is stopping his work...it's already 6pm!\n");
    cleanup();
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage:\n");
        fprintf(stderr, "  Controller: %s controller <number_of_people>\n", argv[0]);
        fprintf(stderr, "  Person: %s person <person_id> <number_of_people>\n", argv[0]);
        exit(1);
    }
    
    char *mode = argv[1]; //the mode of working: controller or chatterbox
    
    if (strcmp(mode, "controller") == 0) {
        int numberOfPeople = atoi(argv[2]);
        if (numberOfPeople < 2 || numberOfPeople > NUMBER_OF_CHATTIES) {
            fprintf(stderr, "Number of people must be between 2 and %d\n", NUMBER_OF_CHATTIES);
            exit(1);
        }
        
        init_shared_memory(numberOfPeople, 1);
        init_semaphores(1);
        controller_process(numberOfPeople);
        
    } else if (strcmp(mode, "person") == 0) { //if the mode of a chatterbox, he is in simulation
        if (argc != 4) {
            fprintf(stderr, "Usage: %s person <person_id> <number_of_people>\n", argv[0]);
            exit(1);
        }
        
        int person_id = atoi(argv[2]);
        int numberOfPeople = atoi(argv[3]);
        
        //registrate the handler for chatterbox
        signal(SIGINT, signal_handler);
        
        init_shared_memory(numberOfPeople, 0);
        init_semaphores(0);
        chatty_person_process(person_id, numberOfPeople);
        
    } else {
        fprintf(stderr, "Nepravilno! Use 'controller' or 'person'\n");
        exit(1);
    }
    
    return 0;
}