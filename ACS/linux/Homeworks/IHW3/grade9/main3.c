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
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define NUMBER_OF_CHATTIES 20
#define SHARED_MEMORY_NAME "/chatterboxes_shm"
#define SEM_SYNC_SEM_NAME "/chatty_sync_sem"
#define SEM_RUNNING_NAME "/chatty_running"
#define MESSAGE_Q_KEY 1234
#define MAX_MESSAGE_SIZE 256

//here we set the conditions of a chatterbox(a chatty person)
typedef enum {
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

//struct for messages queue
typedef struct {
    long mtype;                    //type (1-status, 2-call, 3-connection, 4-end of a chat)
    char mtext[MAX_MESSAGE_SIZE];   
    int person_id;                 //sender's id
    int target_id;                 //target's id
    int numberOfCalls;             //ochev
} Message;
//shared resources
SharedData *shared_data = NULL; //pointer to a piece of a shared memory
sem_t *sync_sem = NULL; //named semaphore for synchronisation
sem_t *running_sem = NULL;
int shm_fd = -1; //file descriptor for shared memory
int msg_queue_id = -1;


void cleanup() { //func for cleaning the resources after the program is closed
    printf("And now I am a powerful machine going to clean the memory from your digits, bits, programms! *-*\n");
    if (shared_data != NULL) { //frees the shared memory
        munmap(shared_data, sizeof(SharedData)); //unmap the shared memory
        shared_data = NULL;
        //shared_data = NULL;
        printf("Shared memory unmapped! :)\n");
    }
    
    if (shm_fd != -1) { //closes and deletes the shared memory 
        close(shm_fd);
        shm_unlink(SHARED_MEMORY_NAME);
        shm_fd = -1;
        printf("Shared memory has been closed and unlinked! :)\n");
    }
    
    if (sync_sem != NULL) { //close and delete the named semaphores
        sem_close(sync_sem); //close access to semaphore
        sem_unlink(SEM_SYNC_SEM_NAME);
        sync_sem = NULL;
        printf("Sync_semaphore has benn closed and unlinked! :)\n");
    }
    
    if (running_sem != NULL) {
        sem_close(running_sem);
        sem_unlink(SEM_RUNNING_NAME);
        running_sem = NULL;
        printf("Running semaphore has been closed and unlinked! :)\n");
    }
    
    if (msg_queue_id != -1) {
        msgctl(msg_queue_id, IPC_RMID, NULL);
        msg_queue_id = -1;
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

//func to send msg to person
void send_message(int person_id, const char *text, int mtype, int target_id, int calls) {
    //here we will open the message queue
    int local_msgq = msgget(MESSAGE_Q_KEY, 0666);
    if (local_msgq != -1) {
        Message msg;
        msg.mtype = mtype;                    
        msg.person_id = person_id;            
        msg.target_id = target_id;            
        msg.numberOfCalls = calls;            
        strncpy(msg.mtext, text, MAX_MESSAGE_SIZE - 1);
        msg.mtext[MAX_MESSAGE_SIZE - 1] = '\0';  
        
        //vstante v ochered'
        if (msgsnd(local_msgq, &msg, sizeof(Message) - sizeof(long), IPC_NOWAIT) == -1) {
            
        }
    }
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
        perror("ftruncate failed...");
        exit(1);
    }

    //free the shared memory to the address space of a process
    shared_data = mmap(NULL, sizeof(SharedData), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_data == MAP_FAILED) {
        perror("mmap failed");
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
            shared_data->people[i].numberOfCalls = 0; //0 attempts to call smb
            shared_data->people[i].numberOfCalls = 0; //0 attempts to call smbd
        }
        printf("Shared memory initialized for %d people ^-^\n", numberOfPeople);
    }
}

//initialisation of named semaphores
void init_semaphores(int create) {
    if (create) { //open the named semaphores
        sync_sem = sem_open(SEM_SYNC_SEM_NAME, O_CREAT, 0666, 1);
        running_sem = sem_open(SEM_RUNNING_NAME, O_CREAT, 0666, 1);
    } else { //and here chatterboxes open existing semaphores
        sync_sem = sem_open(SEM_SYNC_SEM_NAME, 0);
        running_sem = sem_open(SEM_RUNNING_NAME, 0);
    }
    
    if (sync_sem == SEM_FAILED || running_sem == SEM_FAILED) {
        perror("sem_open failed...");
        exit(1);
    }
    printf("Semaphores have been initialized!! :)\n");
}

//inits messages(for controller only)
void init_message_queue(int create) {
    if (create) {
        //here is a new message queue
        msg_queue_id = msgget(MESSAGE_Q_KEY, IPC_CREAT | 0666);
    } else {
        //opens the existing queue
        msg_queue_id = msgget(MESSAGE_Q_KEY, 0666);
    }
    
    if (msg_queue_id == -1) {
        perror("msgget failed...");
        exit(1);
    }
    printf("Message queue has been successfully initialized!! *-*\n");
}
//get chatterboxe's id (excluding yourself)
int get_random_person(int exclude, int numberOfPeople) {
    int target;
    do {
        target = rand() % numberOfPeople;
    } while (target == exclude);
    return target;
}

void chatty_person_process(int id, int numberOfPeople) {
    srand(time(NULL) + id);
    
    printf("Person %d started (PID: %d)\n", id, getpid());
    
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
            usleep(2000000);
            continue;
        }
        
        //decide action : to call or to wait
        if (rand() % 2 == 0) {
            //Wait for call
            shared_data->people[id].state = WAITING;
            printf("Person %d: Waiting for calls ^-^\n", id);
            send_message(id, "Waiting for calls", 1, -1, shared_data->people[id].numberOfCalls);
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
                
                printf("Person %d: Successfully connected to person %d, hurray! :)\n", id, target);
                send_message(id, "Connected successfully", 3, target, shared_data->people[id].numberOfCalls);
            } else {
                //Target is busy
                shared_data->people[id].numberOfCalls++;
                printf("Person %d: Person %d is busy, unfortunately :(\n", id, target);
                send_message(id, "Calling busy person", 2, target, shared_data->people[id].numberOfCalls);
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
            
            printf("Person %d: Finished conversation with person %d *-*\n", id, partner);
            send_message(id, "Conversation ended", 4, partner, shared_data->people[id].numberOfCalls);
            sem_post(sync_sem);
        }
    }
    
    printf("Person %d: I am exiting, goodbye! :)\n", id);
}

//controller process - regulates the simulation
void controller_process(int numberOfPeople) {
    signal(SIGINT, signal_handler); //registrate the handler of signals for controller
    
    printf("Controller started with %d people\n", numberOfPeople);
    printf("Press Ctrl+C to stop\n");
    
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
        init_message_queue(1);  //controller creates the message queue
        controller_process(numberOfPeople);
        
    } else if (strcmp(mode, "person") == 0) {
        if (argc != 4) {
            fprintf(stderr, "Usage: %s person <person_id> <number_of_people>\n", argv[0]);
            exit(1);
        }
        
        int person_id = atoi(argv[2]);
        int numberOfPeople = atoi(argv[3]);
        
        signal(SIGINT, signal_handler);
        init_shared_memory(numberOfPeople, 0);
        init_semaphores(0);
        init_message_queue(0);  //chaterboxes only create the message queue
        chatty_person_process(person_id, numberOfPeople);
        
    } else {
        fprintf(stderr, "Nepravilno! Use 'controller' or 'person'\n");
        exit(1);
    }
    
    return 0;
}