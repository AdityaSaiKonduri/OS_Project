#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"
#include "kernel/riscv.h"
#include "kernel/fcntl.h"
#include "kernel/spinlock.h"  // Add this to use the kernel's spinlock definition
/*
struct spinlock printlock;  // Global lock for printing

// Helper function to print immediately using write
void print_safe(const char *str) {
    acquire(&printlock);
    write(1, str, strlen(str));
    release(&printlock);
}

// Convert integer to string
void itoa(int num, char *str) {
    int i = 0;
    int is_negative = 0;
    char temp[20];

    if (num < 0) {
        is_negative = 1;
        num = -num;
    }

    if (num == 0) {
        temp[i++] = '0';
    }

    while (num != 0) {
        temp[i++] = (num % 10) + '0';
        num = num / 10;
    }

    if (is_negative) {
        temp[i++] = '-';
    }

    int j;
    for (j = 0; j < i; j++) {
        str[j] = temp[i - 1 - j];
    }
    str[j] = '\0';
}

void print_int(int num) {
    char buf[20];
    itoa(num, buf);
    print_safe(buf);
}
*/
void thread_function(void *arg) {
    // int *value = (int*)arg;
    
    printf("Thread starting\n");
    /*print_safe("Thread value: ");
    print_int(*value);
    print_safe("\n");
    
    sleep(10);
    
    print_safe("Thread after sleep, value: ");
    print_int(*value);
    print_safe("\n");
    
    thread_exit((void*)0);*/
}

int main(int argc, char *argv[]) {
    //initlock(&printlock, "print");  // Modified to match xv6 initlock signature

    int *arg = malloc(sizeof(int));
    if(arg == 0) {
        printf("Failed to allocate memory\n");
        exit(1);
    }
    
    *arg = 42;
    void *stack = malloc(2048 + 16);
    if(stack == 0) {
        printf("Failed to allocate stack\n");
        free(arg);
        exit(1);
    }
    
    uint64 stack_top = (((uint64)stack + 2048 + 15) & ~15);
    
    printf("Main: Creating thread\n");
    printf("Thread creation success\n");
    int pid = thread_create(thread_function, (void*)stack_top, (uint64)arg);
    if(pid < 0) {
        printf("Failed to allocate stack\n");
        free(arg);
        free(stack);  // Added missing stack deallocation
        exit(1);
    }

    /*
    uint64 retval;
    print_safe("Going to thread_join\n");  // Added missing newline
    if(thread_join(&retval) < 0) {
        print_safe("Thread join failed\n");
        free(arg);
        free(stack);  // Added missing stack deallocation
        exit(1);
    }

    print_safe("Thread completed with value: ");
    print_int((int)retval);
    print_safe("\n");
    
    free(arg);
    free(stack);  // Added missing stack deallocation

    */
    exit(0);
}