#include "user.h"
#include "kernel/types.h"

void sigint_handler(int signum) {
    printf("Custom SIGINT handler triggered\n");
    // Custom handling logic here
}

int main(void) {
    // Register the SIGINT signal handler
    void (*volatile handler_ptr)(int) = sigint_handler;
    printf("Direct handler address: %p\n", handler_ptr);
    // signal(SIGINT, &sigint_handler);

    // Printing address of handler using a direct variable
    
    // Simulate running some process that waits for signals
    int i = 0;
    while (1) {
        printf("Iteration number: %d\n", i);
        i++;
    }

    return 0;
}
