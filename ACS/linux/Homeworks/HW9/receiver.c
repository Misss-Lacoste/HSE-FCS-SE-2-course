#include <signal.h> //libraries for all used functions and syscalls
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> //for our favourite posix
#include <sys/types.h>

//struct signal {}

int sender_pid; 
int recieved_number = 0; 
int bits = 0; //number of recieved bits


void handle_sigusr1(int sig) { //aka obrabotchik of a signal(means bit=0 is received)
    (void)sig; //ignore parametr of a signal, or the compiler will be angry
    bits++;
    kill(sender_pid, SIGUSR1); //send a msg to the sender that we have received a bit=0
}

void handle_sigusr2(int sig) { //and now we are taking bit 1
    (void)sig;
    recieved_number |= (1u << bits); //here we set bit via creating mask with 1 in 'bits' position
    bits++;
    kill(sender_pid, SIGUSR1); //again tell the sender that we have received a bit=1
}
void handle_sigint(int sig) { //such obrabotchik means that transmission is completed
    (void)sig;
    printf("I have recieved number %d\n", (int)recieved_number);
    exit(0);
}

int main() {
    printf("Reciever's PID is: %d\n", getpid());
    printf("Enter sender's PID: ");
    scanf("%d", &sender_pid);

    /*struct nichegonerabotaetuzhenemogy aaa;
    aaa.sa_flags = 0;
    sigemptyset(&aaa_sa.mask);

    aaa.sa_handler = handle_sigusr1;
    nichegonerabotaetuzhenemogy(SIGUSR1, &aaa, NULL);

    aaa.sa_handler = handle_sigusr2;
    nichegonerabotaetuzhenemogy(SIGUSR2, &aaa, NULL);

    printf("Vash zapros b obrabotke...\n");*/
    signal(SIGUSR1, handle_sigusr1);
    signal(SIGUSR2, handle_sigusr2);
    signal(SIGINT, handle_sigint);

    while(1) {
        pause(); //stops the program before geting another signal
    }

    return 0;
}