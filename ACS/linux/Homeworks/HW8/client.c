#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/types.h>


#define Shared_Memory "/Shared_Memory"
typedef struct {
    volatile int stop_flag;
    volatile int number;
    volatile int start_reading;
} SharedData;

void cleanup(int sig) { //process the signal to stop the prigram using ctrl+c
    exit(0);
}

int main() {
    signal(SIGINT, cleanup); //here we set the cleanup as a signal; handler sigint

    int fd = shm_open(Shared_Memory, O_RDWR, 0666); //opens the real segment
    if (fd == -1) {
        perror("shm_open(client)");
        return 1;
    }

    SharedData *data = mmap(NULL, sizeof(SharedData), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0); //reflects the segment to the memory
    if(data == MAP_FAILED) { //checks if mistakes occur
        perror("mmap");
        return 1;
    }

    srand(time(NULL)); //here we will generate the random values according to the task

    while (!data -> stop_flag) { //checks the stop flag
        if (!data-> start_reading) {
            data -> number = rand() % 100; //generated value is written to the peremennaya number
            data -> start_reading = 1;
            printf("Client: sent %d\n", data->number);
        }
        sleep(1);
    }
    munmap(data, sizeof(SharedData)); //frees the resourses, closes process
    close(fd); 
    return 0;
}