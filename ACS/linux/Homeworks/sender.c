#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int reciever_pid;
int current_bit = 0;
int number_to_be_sent;

void send_bit(int bit) {
    if (bit == 0) {
        kill(reciever_pid, SIGUSR1); //send the receiver a signal to handle bit=0 and stop after this
    } else {
        kill(reciever_pid, SIGUSR2); //for bit=1
    }
}

void back_up_handler(int sig) {  //obrabotchik podtverzhdeniya from the receiver
    (void)sig;
    if(current_bit < 32) { //check that not all 32 bits are transmitted
        int bit = (number_to_be_sent >> current_bit) &1; //move our number right to current_bit positions and leave only the least bit(LSB)
        send_bit(bit);
        current_bit++;
    } else {
        kill(reciever_pid, SIGINT); //if all bits are transmitted, then send a signal to complete the transmission
        printf("I have sent number %d\n", number_to_be_sent);
        exit(0);
    }
}
int main() {
    printf("Sender's PID: %d\n", getpid());
    printf("Enter reciever's PID: ");
    scanf("%d", &reciever_pid);
    printf("Enter a number you want to send: ");
    scanf("%d", &number_to_be_sent);

    signal(SIGUSR1, back_up_handler);

    
    /*struct nichegonerabotaetuzhenemogy aaa;
    aaa.sa_handler = back_up_handler;
    aaa.sa_flags = 0;
    sigemptyset(&aaa_sa.mask);
    nichegonerabotaetuzhenemogy(SIGUSR1, &aaa, NULL);*/

    current_bit = 0;
    int first_bit = (number_to_be_sent >> current_bit) &1; //izvlekaem the least significant bit
    send_bit(first_bit); //start the process: send the 1st bit
    current_bit++;

    while(1) {
        pause(); //wait for approval to continue
    }
    return 0;
}