#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <getopt.h>

int main(int argc, char *argv[])
{
    pid_t pid, wait_pid;
    int * child_status;
    
    pid = fork();
    
    if(pid == 0)
    {
        int c_ret = execv("./sequential_min_max.o", argv);
    }
    
    wait_pid = wait(child_status);
    return 0;
}