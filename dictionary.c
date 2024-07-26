#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include<arpa/inet.h>

#include "network.h"
#include "dictionary.h"

int word_in_dict(char* client_testword,const char* chosen_dict){
    FILE* fp = fopen(chosen_dict,"r");
    if(!fp){fprintf(stderr,"error opening file");}
    size_t len = 0;
    char* line = NULL;
    ssize_t read;
    while( ( read = getline(&line,&len,fp) ) != -1){
        if(read == -1){break;}
        char* token = strtok(line,"\n");
        while(token != NULL){
            if( strcmp(token,client_testword) == 0){
                free(line);
                fclose(fp); 
                return 1;
            }
            token = strtok(NULL,"\n");
        }
    }
    free(line); 
    fclose(fp); 
    return 0; 
}





