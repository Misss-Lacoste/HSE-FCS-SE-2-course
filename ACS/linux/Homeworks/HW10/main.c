#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>

#define MY_MEMORY 1000 //max buffer size
#define THREADS 100
#define I_SLEEP_MNOGO 7
#define I_SLEEP_MALO 1
#define I_CALCULATE_MINIMUM 3
#define I_CALCULATE_MAXIMUM 6

/*int randomNum(int min, int max) {
    return min + rand() % (max - min + 1);
}*/

//struct to keep data in buffer
typedef struct {
    int value;
    int id; //of a thread or a summator
    bool thread; //true -thread, false - summator
} Storage;

Storage buffer[MY_MEMORY];
int bufferCount = 0;
//bufferNum = 0
int calculatedNumbers = 0;
int result = 0;
bool haveICalculated = false;

pthread_mutex_t buffer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t bufferIFilled = PTHREAD_COND_INITIALIZER;
pthread_cond_t buffersFriend = PTHREAD_COND_INITIALIZER; //pair for a bufferr
pthread_mutex_t random_mutex = PTHREAD_MUTEX_INITIALIZER;

//generates a pseudo random number
long random_range(long min, long max) {
    pthread_mutex_lock(&random_mutex);
    long val = min + random() % (max - min + 1);
    pthread_mutex_unlock(&random_mutex);
    return val;
}
/*void* sum_thread(void* arg) {
    int sum_id = *(int*)arg;
    free(arg);
    
    int a = 0, b = 0;*/

//func of an source of a thread
void* source(void* arg) {
    int source_id = *(int*)arg;
    free(arg);
    int delay = (int)random_range(I_SLEEP_MALO, I_SLEEP_MNOGO);
    sleep(delay);
    int number = (int)random_range(1, 100);
    pthread_mutex_lock(&buffer_mutex);
    
    if (bufferCount < MY_MEMORY) { //add a number to a buffer
        buffer[bufferCount].value = number;
        buffer[bufferCount].id = source_id;
        buffer[bufferCount].thread = true;
        bufferCount++;
        calculatedNumbers++;
        
        printf("The thread %d has just generated a number: %d with delay in %d seconds.\n", source_id, number, delay);
        printf("Our buffer contains %d elements now!\n", bufferCount);
        //checks the data is in buffer
        if (bufferCount >= 2) {
            pthread_cond_signal(&buffersFriend);
        }
        pthread_cond_signal(&bufferIFilled);
    } else {
        printf("Source %d tells that buffer is overflowed! Number %d has been lost...\n", source_id, number);
    }
    pthread_mutex_unlock(&buffer_mutex);
    return NULL;
}

void* sum_thread(void* arg) {
    int sum_id = *(int*)arg;
    free(arg);

    pthread_mutex_lock(&buffer_mutex);
    while (bufferCount < 2 && !haveICalculated) {
        pthread_cond_wait(&buffersFriend, &buffer_mutex);
    }
    if (haveICalculated) {
        pthread_mutex_unlock(&buffer_mutex);
        return NULL;
    }
    int a = buffer[0].value;
    int b = buffer[1].value;
    int a_id = buffer[0].id;
    int b_id = buffer[1].id;
    bool a_is_source = buffer[0].thread;
    bool b_is_source = buffer[1].thread;

    for (int i = 2; i < bufferCount; i++) {
        buffer[i - 2] = buffer[i];
    }
    bufferCount -= 2; //deletes the elements of a buffer

    char* a_type;
    if (a_is_source) {
        a_type = "source";
    } else {
        a_type = "summator";
    }
    
    char* b_type;
    if (b_is_source) {
        b_type = "source";
    } else {
        b_type = "summator";
    }
    printf("The summator %d took %d from %s %d & %d from %s %d\n", sum_id, a, a_type, a_id, b, b_type, b_id);
    printf("Buffer tells that %d elements are left\n", bufferCount);
    
    pthread_mutex_unlock(&buffer_mutex);
    
    int calc_time = (int)random_range(I_CALCULATE_MINIMUM, I_CALCULATE_MAXIMUM);
    sleep(calc_time);
    int sum = a + b;
    
    printf("Summator %d calculates the sum as %d + %d = %d (within %d seconds)\n", sum_id, a, b, sum, calc_time);
    pthread_mutex_lock(&buffer_mutex);
    if (haveICalculated) {
        pthread_mutex_unlock(&buffer_mutex);
        return NULL;
    }
    if (bufferCount < MY_MEMORY) { //go back to the buffer
        buffer[bufferCount].value = sum;
        buffer[bufferCount].id = sum_id;
        buffer[bufferCount].thread = false;
        bufferCount++;
        
        printf("Summator %d took the result %d to the buffer\n", sum_id, sum);
        printf("Now there are %d elements in a buffer\n", bufferCount);
        
        if (bufferCount == 1 && calculatedNumbers == THREADS) {
            result = buffer[0].value;
            haveICalculated = true;
            printf("The result is %d\n", result);
            pthread_cond_broadcast(&buffersFriend);
            pthread_cond_broadcast(&bufferIFilled);
        } else if (bufferCount >= 2) {
            pthread_cond_signal(&buffersFriend);
        }
    } else {
        printf("Summator %d tells that buffer is owerflowed, the result %d is lost :( \n", sum_id, sum);
    }
    pthread_mutex_unlock(&buffer_mutex);
    return NULL;
}

void* dispatcher_thread(void* arg) { //dispatcher creates the summator
    int sum_id = 0;
    
    while (!haveICalculated) {
        pthread_mutex_lock(&buffer_mutex);
        while (bufferCount < 2 && !haveICalculated) {
            pthread_cond_wait(&buffersFriend, &buffer_mutex);
        }
        if (haveICalculated) {
            pthread_mutex_unlock(&buffer_mutex);
            break;
        }
        pthread_mutex_unlock(&buffer_mutex);

        pthread_t sumThread;
        int* id_ptr = malloc(sizeof(int));
        if (!id_ptr) {
            perror("malloc");
            continue;
        }
        *id_ptr = sum_id++;

        if (pthread_create(&sumThread, NULL, sum_thread, id_ptr) != 0) {
            perror("Failed to create the summator");
            free(id_ptr);
        } else {
            pthread_detach(sumThread);
            printf("Dispatcher tells that it has created the summator %d\n", *(id_ptr));
        }

        usleep(50000); //sleep, little baby
    }
    return NULL;
}

int main() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    srandom(tv.tv_sec ^ tv.tv_usec ^ getpid());
    printf("Starting position:\n");
    printf("Sources number is %d\n", THREADS);
    
    pthread_t dispatcher;
    if (pthread_create(&dispatcher, NULL, dispatcher_thread, NULL) != 0) {
        perror("Can not create dispatcher");
        return 1;
    }

    pthread_t sources[THREADS];
    for (int i = 0; i < THREADS; i++) {
        int* id = malloc(sizeof(int));
        *id = i;
        if (pthread_create(&sources[i], NULL, source, id) != 0) {
            perror("Failed to create the source");
            free(id);
        }
    }

    for (int i = 0; i < THREADS; i++) {
        pthread_join(sources[i], NULL);
    }
    printf("All sources finished successfully! :) \n");

   
    int max_wait = 100;
    while (!haveICalculated && max_wait > 0) {
        sleep(1);
        max_wait--;
    }

    pthread_join(dispatcher, NULL);

    printf("The final result is %d! :)\n", result);
    printf("It has been generated %d numbers!\n", calculatedNumbers);

    //Cleanup like a duck in telegram
    pthread_mutex_destroy(&buffer_mutex);
    pthread_cond_destroy(&bufferIFilled);
    pthread_cond_destroy(&buffersFriend);
    pthread_mutex_destroy(&random_mutex);

    return 0;
}