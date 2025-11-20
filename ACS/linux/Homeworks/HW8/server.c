#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

#define Shared_Memory "/Shared_Memory"

//here we will set the structure for shared (razdelyaemaya) memory
typedef struct {
    volatile int stop_flag; //volatile - peremennye may differ from process to process, flag to stop the process
    volatile int number; //generated number
    volatile int start_reading; //flag that means that number is ready to be read
} SharedData;

void cleanup(int sig) { //process the signal to stop the prigram using ctrl+c
    shm_unlink(Shared_Memory); //deletes the segment of a shared memory
    _exit(0);
}

int main() {
    signal(SIGINT, cleanup); //here we set the cleanup as a signal; handler sigint

    int fd = shm_open(Shared_Memory, O_CREAT | O_RDWR, 0666); //opens the share memory space and starts to work with it, gives the access rights(to read, write and all in one)
    if(fd == -1) {
        perror("shm_open (server)");
        return 1;
    }
    
    if (ftruncate(fd, sizeof(SharedData)) == -1) { //sets the segment(SharedData) in bytes(its size)
        perror("ftruncate");
        return 1;
    }

    SharedData *data = mmap(NULL, sizeof(SharedData), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0); //mmap sets the segment to the address space of a process and also checks the rights
    if(data == MAP_FAILED) {
        perror ("mmap");
        return 1;
    }
    data -> stop_flag = 0; //here we initialize the inital values in shared memory
    data -> start_reading = 0;

    while (!data -> stop_flag) { //checks if we have to kill the process? if the stop_flag == 1
        if (data -> start_reading) {
            printf("Server: received %d\n", data->number);
            data->start_reading = 0;
        }
        usleep(100000); //in order to give a processor a chance to relax for a while
    }
    munmap(data, sizeof(SharedData)); //separates shared memory from the address space
    close(fd);
    shm_unlink(Shared_Memory); //deletes the segment of a memory
    return 0;
}