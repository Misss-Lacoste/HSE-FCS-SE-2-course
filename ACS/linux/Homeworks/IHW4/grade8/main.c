#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>     
#include <string.h>      
#include <signal.h>     
#include <stdbool.h> 
#include <time.h>  
#include <stdarg.h>  
#include <errno.h>  

//It's a program "Let's talk with Andrey Malakhov!""

/*#define NUMBER_OF_CHATTIES 20
#define SHARED_MEMORY_NAME "/chatterboxes_shm"*/

#define MAX_CHATTERS 10          
#define MIN_CHATTERS 3           
#define MAX_RUN_TIME 100         
#define MIN_RUN_TIME 10          
#define MAX_WAIT_TIME 5           
#define MIN_WAIT_TIME 1          
#define MAX_TALK_TIME 6          
#define MIN_TALK_TIME 1          
#define CONFIG_FILE_MAX_LINE 256  

/*typedef struct { //struct for data between processes
    int numberOfPeople;
    int running; //flazhok of a working programm, where 1 = works, 0 = not
    PersonStatus people[NUMBER_OF_CHATTIES]; //an array of chatty people
} Sh;*/

typedef struct //each person has its own thread and condition
{
    int id;                 
    int state;  // 0 -free, 1 - waiting for a call, 2 - chattering, 3 - calling           
    int partner_id;         
    pthread_t thread;   //thread descriptor POSIX
    pthread_mutex_t mutex; //mutex for sync access to chatties 
    pthread_cond_t cond;    
    bool terminate;  //flazhok for ending the thread
} Chatter;

Chatter chatters[MAX_CHATTERS];   
int N; //the num of chatties
int program_runtime;             
time_t start_time;                
bool program_running = true;      
FILE *output_file = NULL;         
bool use_file_output = false;     
pthread_mutex_t output_mutex = PTHREAD_MUTEX_INITIALIZER;  //for syncing
sem_t available_chatters;  //semaphore to check free chatters

//func for sync oytputting messages
 
void print_message(const char* format, ...) //... - many arguments may have
{
    va_list args; 
//init list of args
    va_start(args, format);

    pthread_mutex_lock(&output_mutex); //to avoid mixing messages from dif threads
    vprintf(format, args); //console
    if (use_file_output && output_file != NULL)  //output to file
    {
        vfprintf(output_file, format, args);
        fflush(output_file);  //aka clean buffer
    }
    pthread_mutex_unlock(&output_mutex);
    va_end(args); //stop working with args
}

int what_time_is_it() 
{ //func to know what time is it
    return (int)(time(NULL) - start_time);
}
//void chatty_person_process(int id, int numberOfPeople) { bla-bla

int find_random_free_chatter(int caller_id) 
{
    int attempts = 0;
    int max_attempts = N *2; //max search attempts
    while (attempts < max_attempts) //search for a free charry
    {
        int target = rand() % N; //and take the random one chtty
        if (target == caller_id) //dont let to call yourself
        {
            attempts++;
            continue; 
        }
        pthread_mutex_lock(&chatters[target].mutex); //block the mutex of the chatty to check his condition
        bool is_free = (chatters[target].state == 1);  //is he free?
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
{ //each chatter is an endless loop
    //if nobody called - he will call and talk for a random period of time
    Chatter* self = (Chatter*)arg;
    while (!self->terminate && program_running) 
    {
        pthread_mutex_lock(&self->mutex);
        self->state = 1;  //free and waits for a call
        self->partner_id = -1; //no partner chatty
        pthread_mutex_unlock(&self->mutex);
        int wait_time = MIN_WAIT_TIME + rand() % (MAX_WAIT_TIME - MIN_WAIT_TIME + 1); //random wait time
        print_message("For entire %d seconds the chatty %d is waiting for a call -- %d seconds!\n", what_time_is_it(), self->id, wait_time);
        //let's wait for a call
        for (int i = 0; i < wait_time*2; i++) 
        {
            if (self->terminate || !program_running) //check the condition - maybe smbd has called?
            {
                break;
            }
            pthread_mutex_lock(
                &self->mutex);
            if (self->state == 2) //got the call
            {  
                pthread_mutex_unlock(&self->mutex);
                break;  //stop waiting
            }
            pthread_mutex_unlock(&self->mutex);
            usleep(500000);
        }
        //Now let's call! if nobody called
        pthread_mutex_lock(&self->mutex);
        if (self->state == 1 && !self->terminate && program_running) 
        {
            pthread_mutex_unlock(&self->mutex);
            print_message("For entire %d seconds the chatty %d is trying to call somebody!\n", what_time_is_it(), self->id);
            int target_id = find_random_free_chatter(self->id); //if have found the free chatter
            if (target_id != -1) 
            {
                pthread_mutex_lock(&chatters[target_id].mutex); //block the mutexes of both chatties
                pthread_mutex_lock(&self->mutex);
                
                if (chatters[target_id].state == 1 && self->state == 1) 
                {
                    self->state = 2;  //start speaking
                    self->partner_id = target_id;
                    chatters[target_id].state = 2;
                    chatters[target_id].partner_id = self->id;

                    pthread_mutex_unlock(&self->mutex);
                    pthread_mutex_unlock(&chatters[target_id].mutex);
                    
                    int talk_time = MIN_TALK_TIME + rand() % (MAX_TALK_TIME - MIN_TALK_TIME + 1); //random talking time
                    sleep(talk_time); //imitate the talk
                    print_message("For entire %d seconds the chatty %d and the chatty %d are talking - %d seconds!\n", what_time_is_it(), self->id, target_id, talk_time);
                    pthread_mutex_lock(&chatters[target_id].mutex); //block mutexes to end the chat
                    pthread_mutex_lock(&self->mutex);
                    print_message("%d seconds: the chatty %d and the chatty %d stopped talking!\n", what_time_is_it(), self->id, target_id);
                    self->state = 1;
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
                    print_message("%d seconds - the chatty %d can't call for a chatty %d :( \n", what_time_is_it(), self->id, target_id);
                }
            } else 
            { //maybe smbd is talking again
                print_message("%d seconds -  the chatty %d did not find another chatties :(\n", what_time_is_it(), self->id);
            }
        } else { //smbd called and prog stops
            pthread_mutex_unlock(&self->mutex);
        }
        usleep(100000);
    }
    printf("%d seconds - the chatty %d ends the talk! \n", what_time_is_it(), self->id);
    return NULL;
}

void signal_handler(int sig) { //to correctly end the programm
    if (sig == SIGINT) {
        print_message("%d seconds -  the stopping signal got. Now we are stopping the work!\n", what_time_is_it());
        program_running = false;
    }
}

void initialize_chatters() 
{
    srand(time(NULL));
    sem_init(&available_chatters, 0, N);
     //init each chatty
    for (int i = 0; i < N; i++) 
    {
        chatters[i].id = i;                
        chatters[i].state = 1;  //wait for a call           
        chatters[i].partner_id = -1;  //no partner chatter     
        chatters[i].terminate = false;  //not stopped yet 
        pthread_mutex_init(&chatters[i].mutex, NULL);
        pthread_cond_init(&chatters[i].cond, NULL);
    }
}
//closing the lavochka
void cleanup_chatters() 
{
    program_running = false;
    for (int i = 0; i < N; i++) 
    {
        chatters[i].terminate = true;
        if (pthread_join(chatters[i].thread, NULL) != 0) 
        {
            fprintf(stderr, "Errrror! %d\n", i);
        }
        pthread_mutex_destroy(&chatters[i].mutex);
        pthread_cond_destroy(&chatters[i].cond);
    }
    sem_destroy(&available_chatters);
    if (output_file != NULL) 
    {
        fclose(output_file);
        output_file = NULL;
    }
}

bool read_config_from_file(const char* filename, int* num_chatters, int* runtime) 
{
    FILE* file = fopen(filename, "r"); //opens file
    if (!file) 
    {
        fprintf(stderr, "Errror when oppening the file: %s\n", filename);
        return false;
    }
    
    char line[CONFIG_FILE_MAX_LINE];//buff to read the string
    while (fgets(line, sizeof(line), file)) 
    {
        if (line[0] == '#' || line[0] == '\n') { //ignore lines with symbols as here 
            continue;
        }
        char key[50], value[50]; //key-value
        if (sscanf(line, "%49[^=]=%49s", key, value) == 2) //we let to read inly 49 symbols not to overflow buff
        {
            if (strcmp(key, "chatters") == 0) 
            {
                *num_chatters = atoi(value);  //str to int
            }
            else if (strcmp(key, "runtime") == 0) 
            {
                *runtime = atoi(value);  //str to int atoi = str to int
            }
        }
    }
    fclose(file);
    if (*num_chatters < MIN_CHATTERS || *num_chatters > MAX_CHATTERS) 
    {
        fprintf(stderr, "Not right number of chatties in file: %d\n", *num_chatters);
        return false;
    }
    
    if (*runtime < MIN_RUN_TIME || *runtime > MAX_RUN_TIME) {
        fprintf(stderr, "Wrong working time in file: %d\n", *runtime);
        return false;
    }
    return true;
}

void print_help() //you will see the help -- the key difference between programms - as we were asked, it is arguments of the command line

{
    printf("What you can enter:\n");
    printf("./main -n <number_of_chatties> -t <working_time> [-o <output file>]\n");
    printf("Or to use config files: ./main -c <config_file> [-o <output_file>]\n");
    
    printf("Parametres:\n");
    printf("  -n <number_of_chatties> from %d to %d\n", MIN_CHATTERS, MAX_CHATTERS);
    printf("  -t <working_time> from %d to %d\n", MIN_RUN_TIME, MAX_RUN_TIME);
    printf("  -o <file> is a file for outputting results\n");
    printf("  -c <file> is a config file instead of -n and -t\n");
    printf("  -h <see the help window>\n");
    
    printf("Parametres:\n");
    printf("  ./main -n 3 -t 30 -o output.txt\n");
    printf("  ./main -c config.txt -o log.txt\n");
    printf("  ./main -n 3 -t 20\n");
    
    printf("Format of config file:\n");
    printf("Comments start with #\n");
    printf("chatters=3\n");
    printf("runtime=30\n");
}

int main(int argc, char* argv[]) //here they are - our arguments
{
    struct sigaction sa; //setting signal handlers
    sa.sa_handler = signal_handler; 
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0; 
    if (sigaction(SIGINT, &sa, NULL) == -1) 
    {
        perror("ERrrrror!");
        return 1;
    }
    N = 3;
    program_runtime = 30;
    char* output_filename = NULL;   //output's file's name (NULL = no file)
    char* config_filename = NULL;   //config's file's name (NULL = not used)
    for (int i = 1; i < argc; i++) 
    {
        if (strcmp(argv[i], "-h") == 0) 
        {
            print_help();
            return 0;
        }
        else if (strcmp(argv[i], "-n") == 0 && i + 1 < argc) 
        {
            N = atoi(argv[++i]);  // atoi = str to int
        }
        else if (strcmp(argv[i], "-t") == 0 && i + 1 < argc) 
        {
            program_runtime = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) 
        {
            output_filename = argv[++i];
        }
        else if (strcmp(argv[i], "-c") == 0 && i + 1 < argc) 
        {
            config_filename = argv[++i];
        }
    }
    if (config_filename != NULL) 
    {
        if (!read_config_from_file(config_filename, &N, &program_runtime)) 
        {
            fprintf(stderr, "Errrror reading config file\n");
            return 1;
        }
    }
    //check the params
    if (N < MIN_CHATTERS || N > MAX_CHATTERS) 
    {
        fprintf(stderr, "You didn't understand. Let me explain one more time: enter the number from %d to %d: \n", MIN_CHATTERS, MAX_CHATTERS);
        return 1;
    }
    if (program_runtime < MIN_RUN_TIME || program_runtime > MAX_RUN_TIME) {
        fprintf(stderr, "No, let's see: enter the number from %d to %d seconds\n", 
                MIN_RUN_TIME, MAX_RUN_TIME);
        return 1;
    }
    if (output_filename != NULL) 
    {
        output_file = fopen(output_filename, "w");
        if (output_file == NULL) 
        {
            fprintf(stderr, "Errrror oppening output file: %s\n", output_filename);
            return 1;
        }
        use_file_output = true;
    }
    printf("The number of chatties: %d\n", N);
    printf("Working time is: %d seconds\n", program_runtime);
    printf("You can always press Ctrl+C to stop the programm!\n");
    
    if (config_filename) 
    {
        printf("Config file: %s\n", config_filename);
    }
    
    if (output_filename) 
    {
        printf("Output file: %s\n", output_filename);
    }
    if (use_file_output) 
    {
        fprintf(output_file, "The number of chatties: %d\n", N);
        fprintf(output_file, "Working time is: %d seconds\n", program_runtime);
        if (config_filename) 
        {
            fprintf(output_file, "Config file: %s\n", config_filename);
        }
    }
    initialize_chatters();
    start_time = time(NULL);
    for (int i = 0; i < N; i++) 
    {
        if (pthread_create(&chatters[i].thread, NULL, chatter_thread, &chatters[i]) != 0) {
            fprintf(stderr, "Error while creating a chatty %d\n", i);
            cleanup_chatters(); 
            return 1;
        }
    }
    print_message("%d seconds - the prog is launched. All chatties are active! \n", what_time_is_it(), N);
    time_t end_time = start_time + program_runtime;
    while (time(NULL) < end_time && program_running) 
    {
        sleep(1);
    }
    if (program_running) 
    {
        print_message("%d seconds - The time is up!. Ending the work!\n", what_time_is_it());
    }
    cleanup_chatters();
    print_message("%d seconds -  the program has stopped succesfully!\n", what_time_is_it());
    return 0;
}