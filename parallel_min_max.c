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

#include "find_min_max.h"
#include "utils.h"

int main(int argc, char **argv) {
  //cloud9
  int seed = -1;
  int array_size = -1;
  int pnum = -1;
  bool with_files = false;

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"seed", required_argument, 0, 0},
                                      {"array_size", required_argument, 0, 0},
                                      {"pnum", required_argument, 0, 0},
                                      {"by_files", no_argument, 0, 'f'},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "f", options, &option_index);
    
    if (c == -1) break;

    switch (c) {
      case 0:
        switch (option_index) {
          case 0:
            seed = atoi(optarg);
            if(seed < 0){
                printf("seed < 0");
                exit(1);
            }
            break;
          case 1:
            array_size = atoi(optarg);
            if(array_size < 0){
                printf("array_size < 0");
                exit(1);
            }
            break;
          case 2:
            pnum = atoi(optarg);
            if(pnum < 0){
                printf("pnum < 0");
                exit(1);
            }
            break;
          case 3:
            with_files = true;
            break;

          defalut:
            printf("Index %d is out of options\n", option_index);
        }
        break;
      case 'f':
        with_files = true;
        break;

      case '?':
        break;

      default:
        printf("getopt returned character code 0%o?\n", c);
    }
  }
  
  if(seed == -1 || array_size == -1 || pnum == -1){
      printf("usage isn't valid!");
      return 1;
  }

  int *array = malloc(sizeof(int) * array_size);
  GenerateArray(array, array_size, seed);
  int active_child_processes = 0;

  struct timeval start_time;
  gettimeofday(&start_time, NULL);

  struct MinMax min_max;
  min_max.min = INT_MAX;
  min_max.max = INT_MIN;

    //min
    int fd1[2];
    if (pipe(fd1) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    
    // max
    int fd2[2];
    if (pipe(fd2) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    int * els = malloc(pnum * sizeof(int) * 2);

    for (int i = 0; i < pnum; i++) {
        pid_t child_pid = fork();

        // if child
        if (child_pid == 0)
        {  
            printf("\nChild process: %i\n", i);
            min_max = GetMinMax(array, (int)(array_size * i / pnum), (int)(array_size * (i+1) / pnum));
              
            char message[20];
            int max = min_max.max;
            sprintf(message,"%i", max);
            
            int min = min_max.min;
            sprintf(message,"%i", min);  
            
            if(with_files){
                printf("with_files");
                FILE *fp;
                fp=fopen("test.txt", "w");
                fprintf(fp, "%d %d", min, max);
                fclose(fp);
            } else {
                write(fd1[1], message, strlen(message));
                write(fd2[1], message, strlen(message));
            }
            
            exit(EXIT_SUCCESS);
        }
        // if parent
        else if (child_pid > 0)
        {
            wait(NULL);                    
            printf("\nParent process: %i\n", i);
            
            char message[20];
            if(with_files){
                FILE *fp;
                fp=fopen("test.txt", "r");
                fscanf(fp, "%d %d", &els[2*i], &els[2*i+1]);
                fclose(fp);
            } else {
                read(fd1[0], message, sizeof(message));
                //max 
                sscanf(message, "%d", &els[2*i]);
            
                read(fd2[0], message, sizeof(message));
                //min
                sscanf(message, "%d", &els[2*i+1]);
            }
            continue;
        }
    }

    struct MinMax min_max_res = min_max_res = GetMinMax(els, 0, pnum * 2);
    printf("Result min: %d\n", min_max_res.min);
    printf("Result max: %d\n", min_max_res.max); 
        
    struct timeval finish_time;
    gettimeofday(&finish_time, NULL);

    double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
    elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;
    
    //printf("\nelapsed_time = %l\n",elapsed_time);

    for (int i = 0; i < array_size; i++)
        printf("%i, ", array[i]);

    close(fd1[0]);
    close(fd1[1]);
    close(fd2[0]);
    close(fd2[1]);
    free(array);
    fflush(NULL);
    exit(0);
    return 0;
}