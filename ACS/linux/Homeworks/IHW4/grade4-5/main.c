#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>
#include <time.h>

//It's a program "Let's talk with Andrey Malakhov!""

/*#define NUMBER_OF_CHATTIES 20
#define SHARED_MEMORY_NAME "/chatterboxes_shm"*/

#define MAX_CHATTERS 10
#define MIN_CHATTERS 3
#define MAX_WAIT_TIME 5
#define MIN_WAIT_TIME 1
#define MAX_TALK_TIME 6
#define MIN_TALK_TIME 1

/*typedef struct { //struct for data between processes
    int numberOfPeople;
    int running; //flazhok of a working programm, where 1 = works, 0 = not
    PersonStatus people[NUMBER_OF_CHATTIES]; //an array of chatty people
} Sh;*/

typedef struct  //each person has its own thread and condition
{
    int id;
    int state;  // 0 -free, 1 - waiting for a call, 2 - chattering, 3 - calling
    int partner_id;
    pthread_t thread; //thread descriptor POSIX
    pthread_mutex_t mutex; //mutex for sync access to chatties
    pthread_cond_t cond;
    bool terminate; //flazhok for ending the thread
} Chatter;

Chatter chatters[MAX_CHATTERS];
int N; //the num of chatties
time_t start_time;
bool program_running = true;
pthread_mutex_t output_mutex = PTHREAD_MUTEX_INITIALIZER; //for syncing
sem_t available_chatters;

int what_time_is_it()  //func to know what time is it
{ 
    return (int)(time(NULL) - start_time);
}

//void chatty_person_process(int id, int numberOfPeople) { bla-bla
int find_random_free_chatter(int caller_id) 
{
    int attempts = 0;
    int max_attempts = N *2; //max search attempts
    while (attempts < max_attempts)  //search for a free charry
    {
        int target = rand() % N; //and take the random one chtty
        if (target == caller_id)  //dont let to call yourself
        {
            attempts++;
            continue;
        }
        
        pthread_mutex_lock(&chatters[target].mutex); //block the mutex of the chatty to check his condition
        bool is_free = (chatters[target].state == 1); //is he free?
        pthread_mutex_unlock(&chatters[target].mutex);
        if (is_free) 
        {
            return target; //if found the free one- we are happy
        }
        attempts++;
        usleep(10000); //zaderzhka between attempts
    }
    
    return -1; //no free chatties :()
}

void* chatter_thread(void* arg) 
{ //each chatter in an endless loop
    //if nobody called - he will call and talk for a random period of time
    Chatter* self = (Chatter*)arg;
    while (!self->terminate && program_running)  //waiting for a call
    {
        pthread_mutex_lock(&self->mutex);
        self->state = 1;
        pthread_mutex_unlock(&self->mutex); //waiting for a call - set the condition
        int wait_time = MIN_WAIT_TIME + rand() % (MAX_WAIT_TIME - MIN_WAIT_TIME + 1); //random wait time
        printf("For entire %d seconds the chatty %d is waiting for a call -- %d seconds!\n", 
               what_time_is_it(), self->id, wait_time);
        //let's wait for a call
        for (int i = 0; i < wait_time*2; i++) {
            if (self->terminate || !program_running) break;
            //check the condition - maybe smbd has called?
            pthread_mutex_lock(&self->mutex);
            if (self->state == 2) //got the call
            {
                pthread_mutex_unlock(&self->mutex);
                break; //stop waiting
            }
            pthread_mutex_unlock(&self->mutex);
            usleep(500000); //wait again
        }
        
        //Now let's call! if nobody called
        pthread_mutex_lock(&self->mutex);
        if (self->state == 1 && !self->terminate && program_running) 
        {
            pthread_mutex_unlock(&self->mutex);
            printf("For entire %d seconds the chatty %d is trying to call somebody!\n", what_time_is_it(), self->id);
            int target_id = find_random_free_chatter(self->id);
            if (target_id != -1) //if have found the free chatter
            {
                pthread_mutex_lock(&chatters[target_id].mutex); //block the mutexes of both chatties
                pthread_mutex_lock(&self->mutex);
                if (chatters[target_id].state == 1 && self->state == 1) 
                {
                    self->state = 2; //start speaking
                    self->partner_id = target_id;
                    chatters[target_id].state = 2;
                    chatters[target_id].partner_id = self->id;
                    
                    pthread_mutex_unlock(&self->mutex);
                    pthread_mutex_unlock(&chatters[target_id].mutex);
                    
                    int talk_time = MIN_TALK_TIME + rand() % (MAX_TALK_TIME - MIN_TALK_TIME + 1); //random talking time
                    printf("For entire %d seconds the chatty %d and the chatty %d are talking (%d seconds)!\n", 
                           what_time_is_it(), self->id, target_id, talk_time);
                    sleep(talk_time); //imitate the talk
                    pthread_mutex_lock(&chatters[target_id].mutex); //block mutexes to end the chat
                    pthread_mutex_lock(&self->mutex);
                    printf("%d seconds: the chatty %d and the chatty %d stopped talking!\n", 
                           what_time_is_it(), self->id, target_id);
                    self->state = 1; //the chatties are waiting again
                    chatters[target_id].state = 1;
                    self->partner_id = -1;
                    chatters[target_id].partner_id = -1;
                    pthread_mutex_unlock(&self->mutex);
                    pthread_mutex_unlock(&chatters[target_id].mutex);
                    sem_post(&available_chatters); //the chatties are free
                    sem_post(&available_chatters);
                } else 
                {
                    pthread_mutex_unlock(&self->mutex);
                    pthread_mutex_unlock(&chatters[target_id].mutex);
                    printf("%d seconds - the chatty %d can't call for a chatty %d :( \n", 
                           what_time_is_it(), self->id, target_id);
                }
            } else 
            { //maybe smbd is talking again
                printf("%d seconds -  the chatty %d did not find another chatties :( \n", 
                       what_time_is_it(), self->id);
            }
        } else 
        {
            pthread_mutex_unlock(&self->mutex);
        }
        usleep(100000);
    }
    printf("%d seconds - the chatty %d ends the talk! \n", what_time_is_it(), self->id);
    return NULL;
}

void signal_handler(int sig)  //to correctly end the programm
{
    if (sig == SIGINT) 
    {
        printf("%d seconds -  the stopping signal got. Now we are stopping the work!\n", 
               what_time_is_it());
        program_running = false;
    }
}

void initialize_chatters() 
{
    srand(time(NULL));
    sem_init(&available_chatters, 0, N); //initially - all the chatties are free
    //init each chatty
    for (int i = 0; i < N; i++) {
        chatters[i].id = i;
        chatters[i].state = 1; //wait for a call
        chatters[i].partner_id = -1; //no partner chatter
        chatters[i].terminate = false; //not stopped yet
        pthread_mutex_init(&chatters[i].mutex, NULL);
        pthread_cond_init(&chatters[i].cond, NULL);
    }
}
//closing the lavochka
void cleanup_chatters() {
    program_running = false;
    for (int i = 0; i < N; i++) {
        chatters[i].terminate = true;
        if (pthread_join(chatters[i].thread, NULL) != 0) {
            fprintf(stderr, "Errrror! %d\n", i);
        }
        pthread_mutex_destroy(&chatters[i].mutex);
        pthread_cond_destroy(&chatters[i].cond);
    }
    
    sem_destroy(&available_chatters);
}

int main() 
{
    struct sigaction sa; //setting signal handlers
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask); //clean the signal mask
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    
    printf("Enter the number of chatties: %d-%d: ", MIN_CHATTERS, MAX_CHATTERS);
    while (scanf("%d", &N) != 1 || N < MIN_CHATTERS || N > MAX_CHATTERS) 
    {
        printf("You didn't understand. Let me explain one more time: enter the number from %d to %d: ", MIN_CHATTERS, MAX_CHATTERS);
        while (getchar() != '\n'); //clear the buffer so we have a possibility to enter the new value
    }
    
    int program_runtime; //how much does the prog works
    printf("Enter the program working time in seconds: ");
    while (scanf("%d", &program_runtime) != 1 || program_runtime < 30 || program_runtime > 300) 
    {
        printf("No, let's see: enter the number from 30 до 300: ");
        while (getchar() != '\n'); //again clear buffer
    }
    
    printf("The number of chatties: %d\n", N);
    printf("Working time is: %d seconds\n", program_runtime);
    printf("You can always press Ctrl+C to stop the programm!\n");
    
    //init
    initialize_chatters();
    start_time = time(NULL);
    //threads:
    for (int i = 0; i < N; i++) 
    {
        if (pthread_create(&chatters[i].thread, NULL, chatter_thread, &chatters[i]) != 0) 
        {
            fprintf(stderr, "Error while creating a chatty %d\n", i);
            cleanup_chatters();
            return 1;
        }
    }
    
    printf("%d seconds - the prog is launched. All chatties are active! \n", what_time_is_it());
    
    //wait for the time is over
    time_t end_time = start_time + program_runtime;
    while (time(NULL) < end_time && program_running) {
        sleep(1);
    }
    
    printf("%d seconds - The time is up!. Ending the work!\n", what_time_is_it());
    cleanup_chatters();
    
    printf("%d seconds -  the program has stopped succesfully!\n", what_time_is_it());
    
    return 0;
}