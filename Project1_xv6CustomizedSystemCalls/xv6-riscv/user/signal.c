#include "user.h"
#include "kernel/types.h"

void sigint_handler(int signum);

void sigint_handler(int signum) {
    printf("Custom SIGINT handler triggered\n");
    // Custom handling logic here
}

int main(void) {
    // Register the SIGINT signal handler
    signal(SIGINT, &sigint_handler);


    // Printing address of handler using a direct variable
    printf("Address of the signal handler : %p\n", &sigint_handler);
    // Simulate running some process that waits for signals
    int i = 0;
    while (1) {
        // printf("Iteration number: %d\n", i);
        i++;
    }

    return 0;
}
