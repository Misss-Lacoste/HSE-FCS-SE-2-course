#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define NUMBER_OF_CHATTIES 20
#define SHARED_MEMORY_NAME "/chatterboxes_shm"
#define SEM_SYNC_SEM_NAME "/chatty_sync_sem"
#define SEM_RUNNING_NAME "/chatty_running"
#define MSG_Q_KEY 1234
#define MAX_MESSAGE_SIZE 256

typedef enum {
    WAITING,
    CALLING, 
    TALKING
} State;

typedef struct {
    State state;
    int talkingTo;
    int numberOfCalls;
} PersonStatus;

typedef struct {
    int numberOfPeople;
    int running;
    PersonStatus people[NUMBER_OF_CHATTIES];
} SharedData;

typedef struct {
    long mtype;
    char mtext[MAX_MESSAGE_SIZE];
    int person_id;
    int target_id;
    int numberOfCalls;
} Message;
//shared resources
SharedData *shared_data = NULL; //pointer to a piece of a shared memory
sem_t *sync_sem = NULL; //named semaphore for synchronisation
sem_t *running_sem = NULL;
int shm_fd = -1; //file descriptor for shared memory
int msg_queue_id = -1;

//func for cleaning the resources after the program is closed
void cleanup() {
    printf("Big brother is watching how you have cleaned up the resources...\n");
    
    if (shared_data != NULL) { //frees the shared memory
        munmap(shared_data, sizeof(SharedData)); //unmap the shared memory
        shared_data = NULL;
    }
    
    if (shm_fd != -1) { //closes and deletes the shared memory 
        close(shm_fd);
        shm_fd = -1;
    }
    
    if (msg_queue_id != -1) {
        //closes the queueu
        msg_queue_id = -1;
    }
    
    printf("Hmm, you cleaned up well, good job!\n");
}

//signal handler in case of pressing Ctrl+C
void signal_handler(int sig) {
    printf("\nObserver has received signal %d, now it is shutting down...\n", sig);
    
    if (running_sem != NULL && shared_data != NULL) {
        sem_wait(running_sem); //catch the semaphore for safe change of a flag
        shared_data->running = 0;  //here we set the flag of the stop
        sem_post(running_sem); //DOBBY IS FREE!
    }
    
    cleanup();
    exit(0);
}


void init_shared_memory() { //open or create the shared memory
    shm_fd = shm_open(SHARED_MEMORY_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open failed...");
        exit(1);
    }
    
    shared_data = mmap(NULL, sizeof(SharedData), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_data == MAP_FAILED) {
        perror("mmap failed");
        exit(1);
    }
    printf("Observer has connected to shared memory!\n");
}

void init_semaphores() {
    sync_sem = sem_open(SEM_SYNC_SEM_NAME, 0);
    running_sem = sem_open(SEM_RUNNING_NAME, 0);
    
    if (sync_sem == SEM_FAILED || running_sem == SEM_FAILED) {
        perror("sem_open failed...");
        exit(1);
    }
    printf("Observer has seccessfully connected to semaphores!\n");
}


void init_message_queue() {
    msg_queue_id = msgget(MSG_Q_KEY, 0666);
    if (msg_queue_id == -1) {
        perror("msgget failed...");
        exit(1);
    }
    printf("Observer has connected to message queue!!\n");
}


void display_status() {
    printf("Statuses of chatterboxes:\n");
    for (int i = 0; i < shared_data->numberOfPeople; i++) {
        const char *state_str;
        const char *color_code;
        
        switch (shared_data->people[i].state) {
            case WAITING: 
                state_str = "WAITING";
                color_code = "\033[32m"; //green
                break;
            case CALLING: 
                state_str = "CALLING";
                color_code = "\033[33m"; //yellow
                break;
            case TALKING: 
                state_str = "TALKING";
                color_code = "\033[31m"; //red
                break;
            default: 
                state_str = "UNKNOWN";
                color_code = "\033[37m"; //white
                break;
        }
        
        printf("Person %-2d: %s%-7s\033]0m", i, color_code, state_str);
        
        if (shared_data->people[i].state == TALKING) {
            printf(" (with person %-2d) ", shared_data->people[i].talkingTo);
        } else {
            printf("  ");
        }
        
        printf("Calls: %-3d|\n", shared_data->people[i].numberOfCalls);
    }
    
    printf("-----------------\n");
}

//income msgs from chatterboxes
void process_messages() {
    Message msg;
    //cleans the queue
    while (msgrcv(msg_queue_id, &msg, sizeof(Message) - sizeof(long), 1, IPC_NOWAIT) != -1) {
        switch (msg.mtype) {
            case 1: //waiting
                printf("%s (Person %d)\n", msg.mtext, msg.person_id);
                break;
            case 2: //call
                printf("Person %d is calling person %d (attempt: %d)*-*\n", 
                       msg.person_id, msg.target_id, msg.numberOfCalls);
                break;
            case 3: //connection
                printf("Person %d has been connected with person %d !! :)\n", 
                       msg.person_id, msg.target_id);
                break;
            case 4: //stop chattering!
                printf("Person %d has ended conversation with person %d ^-^\n",
                       msg.person_id, msg.target_id);
                break;
            default:
                printf("Unknown message type: %ld from person %d\n", //a vdrug..
                       msg.mtype, msg.person_id);
                break;
        }
        fflush(stdout);
    }
}

void observer_process() {
    printf("Remember, big brother is watching you! *-*\n");
    printf("How much are there chatty people? Oh, %d chatty people!\n", shared_data->numberOfPeople);
    printf("You can always press Ctrl+C to stop the whole program!\n");
    
    int update_count = 0;
    
    while (1) {
        //check flazhok
        sem_wait(running_sem);
        int running = shared_data->running;
        sem_post(running_sem);
        
        if (!running) break;
        process_messages();
        
        if (update_count >= 2) {
            sem_wait(sync_sem);
            display_status();
            sem_post(sync_sem);
            update_count = 0;
        }
        
        sleep(5);
        update_count++;
    }
    
    printf("Observeration is stopping...the reason - I am tired and want to sleep...\n");
}

int main() {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    printf("Initializing observer...\n");
    
    init_shared_memory();
    init_semaphores();
    init_message_queue();

    observer_process();
    
    cleanup();
    printf("Hurray! Observer stopped successfully. And I am going to sleep. Do not disturb, please :)\n");
    
    return 0;
}