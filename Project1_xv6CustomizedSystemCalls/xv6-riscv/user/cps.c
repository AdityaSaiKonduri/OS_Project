#include "kernel/types.h"
#include "user.h"
// #include "fcntl.h"

int
main(int argc, char *argv[]){
    int k = cps();
    printf("%d\n", k);
    exit(0);
}