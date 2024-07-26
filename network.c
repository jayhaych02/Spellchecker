
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <time.h>
#include "dictionary.h"
#include "network.h"

int valid_configflags(ServerConfiguration *config){
    if(config->buffer_capacity <= 0){puts("Invalid connection buffer size");return 0;}
    if(config->num_workers <= 0){puts("Invalid amount of worker threads chosen");return 0;}
    if(config->sched_type < 1 || config->sched_type > 2){puts("Invalid Scheduling Number Chosen");return 0;}
    if(config->port <= 0 || config->port == 80){puts("Invalid Port Number");return 0;}
    return 1;
}

int valid_server(ServerConfiguration *config){
    int all_valid = valid_configflags(config);
    if(!all_valid){puts("Server must be run with all fields specified, except Dictionary if you want to use the default dictionary!");return 0;}
    return 1;
}

