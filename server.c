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
#include <sys/time.h>
#include <getopt.h>
#include <time.h>
#include "dictionary.h"
#include "network.h"

Client* find_highest_priority_client(ActiveConnections *buffer) {
    if (buffer->numItems == 0) return NULL;
    
    size_t highest_prio_idx = buffer->extract_idx;
    int highest_prio = buffer->internal_activefd[highest_prio_idx].assigned_priority;
    
    for (size_t i = 0; i < buffer->numItems; i++) {
        size_t curr_idx = (buffer->extract_idx + i) % buffer->capacity;
        if (buffer->internal_activefd[curr_idx].assigned_priority < highest_prio) {
            highest_prio = buffer->internal_activefd[curr_idx].assigned_priority;
            highest_prio_idx = curr_idx;
        }
    }
    
    return &buffer->internal_activefd[highest_prio_idx];
}


void sort_highest_prio(LogBuffer *logbuffer) {
    int i, j;
    Client key;
    for (i = 1; i < logbuffer->numItems; i++) {
        key = logbuffer->internal_buffer[i];
        j = i - 1;
        while (j >= 0 && logbuffer->internal_buffer[j].assigned_priority > key.assigned_priority) {
            logbuffer->internal_buffer[j + 1] = logbuffer->internal_buffer[j];
            j = j - 1;
        }
        logbuffer->internal_buffer[j+1] = key;
    }
}

void remove_client_from_buffer(ActiveConnections *buffer, size_t idx) {
    // Shift remaining clients
    for (size_t i = idx; i < buffer->numItems - 1; i++) {
        size_t curr = i % buffer->capacity;
        size_t next = (i + 1) % buffer->capacity;
        buffer->internal_activefd[curr] = buffer->internal_activefd[next];
    }
    buffer->numItems--;
}

void write_to_logfile(LogThread *logger_thread, Client client){
    FILE* fp = fopen(logger_thread->logFile,"a");
    if( fp == NULL ){perror("error opening log file");}
    fprintf(fp,"|FD:%d|Word:%s|Status:%s|Priority:%d|Arrival:%s|Completion:%s\n\n",
        client.my_fd, 
        client.word, 
        client.socket_response_value, 
        client.assigned_priority,
        client.time_acceptance,
        client.time_spellcheck_completed
    );
    fclose(fp);
}

void logbuff_insert(WorkerThreads* curr, Client client){
    pthread_mutex_lock(&curr->logbuff_ref->logbuff_lock);
    while(curr->logbuff_ref->numItems == curr->logbuff_ref->capacity){
        pthread_cond_wait(&curr->logbuff_ref->is_empty,&curr->logbuff_ref->logbuff_lock);
    }
    curr->logbuff_ref->internal_buffer[curr->logbuff_ref->insert_idx] = client;
    curr->logbuff_ref->numItems++;
    curr->logbuff_ref->insert_idx = (curr->logbuff_ref->insert_idx + 1) % curr->logbuff_ref->capacity;
    pthread_mutex_unlock(&curr->logbuff_ref->logbuff_lock);
    pthread_cond_signal(&curr->logbuff_ref->is_full);
}



void* extract_logfile(void *args){
    LogThread *logger = (LogThread*)args;
    while(1){
        pthread_mutex_lock(&logger->logbuff_ref->logbuff_lock);
        while(logger->logbuff_ref->numItems==0){
            pthread_cond_wait(&logger->logbuff_ref->is_full,&logger->logbuff_ref->logbuff_lock);
        }
        if(logger->config_ref->sched_type == 2){sort_highest_prio(logger->logbuff_ref);};
        for(int i=0;i<logger->logbuff_ref->numItems;i++){
            printf("CLIENT PRIOS = %d\n",logger->logbuff_ref->internal_buffer[logger->logbuff_ref->extract_idx].assigned_priority);
        }
        Client extracted = logger->logbuff_ref->internal_buffer[logger->logbuff_ref->extract_idx];
        write_to_logfile(logger,extracted);
        logger->logbuff_ref->numItems--;
        logger->logbuff_ref->extract_idx = (logger->logbuff_ref->extract_idx + 1 ) % logger->logbuff_ref->capacity;
        pthread_mutex_unlock(&logger->logbuff_ref->logbuff_lock);
        pthread_cond_signal(&logger->logbuff_ref->is_empty);
    } 
    return NULL;
}


void* work_threads(void* args) {
    WorkerThreads *curr_worker = (WorkerThreads*)args;
    while(1) {
        pthread_mutex_lock(&curr_worker->clientsWaiting->connections_lock);
        
        while (curr_worker->clientsWaiting->numItems == 0) {
            pthread_cond_wait(&curr_worker->clientsWaiting->no_space, 
                            &curr_worker->clientsWaiting->connections_lock);
        }
        
        Client* highest_prio_client = find_highest_priority_client(curr_worker->clientsWaiting);
        if (!highest_prio_client) {
            pthread_mutex_unlock(&curr_worker->clientsWaiting->connections_lock);
            continue;
        }
        
        // Each thread makes their own copy of their assigned client's data
        Client extracted = *highest_prio_client;
        
        remove_client_from_buffer(curr_worker->clientsWaiting, highest_prio_client - curr_worker->clientsWaiting->internal_activefd);
        
        pthread_mutex_unlock(&curr_worker->clientsWaiting->connections_lock);
        pthread_cond_signal(&curr_worker->clientsWaiting->is_space);
        
        // Client service logic
        char* message;
        char client_word[128];
        int read_size;
        int client_writing_to = extracted.my_fd;
        char* correct_spelling = ":CORRECT";
        char* incorrect_spelling = ":MISSPELLED";
        
        message = "Hello Client, this is the work thread designated to you\n"
                 "Now, type a word and I will spellcheck it: ";
        
        if(send(client_writing_to, message, strlen(message), 0) == -1) {
            perror("send failed!");
            continue;
        }
        
        char response_buffer[500];
        
        // Record arrival time
        struct timeval times;
        struct tm *start_info, *end_info;
        gettimeofday(&times, NULL);
        start_info = localtime(&times.tv_sec);
        char arrival_time[100], time_completed[100], entire_time[300];
        strftime(arrival_time, 80, "%Y-%m-%d %H:%M:%S", start_info);
        snprintf(entire_time, 300, "%s.%ld\n", arrival_time, times.tv_usec / 1000);
        strcpy(extracted.time_acceptance, entire_time);
        
        while((read_size = recv(client_writing_to, client_word, sizeof(client_word), 0)) > 0) {
            client_word[read_size] = '\0';
            
            if(word_in_dict(client_word, curr_worker->config_ref->dictionary)) {
                snprintf(response_buffer, sizeof(response_buffer), "%s%s", 
                        client_word, correct_spelling);
            } else {
                snprintf(response_buffer, sizeof(response_buffer), "%s%s", 
                        client_word, incorrect_spelling);
            }
            
            strcpy(extracted.socket_response_value, response_buffer);
            write(client_writing_to, response_buffer, strlen(response_buffer));
            
            // Record completion time
            gettimeofday(&times, NULL);
            end_info = localtime(&times.tv_sec);
            strftime(time_completed, 80, "%Y-%m-%d %H:%M:%S", end_info);
            snprintf(entire_time, 300, "%s.%ld\n", time_completed, times.tv_usec / 1000);
            strcpy(extracted.time_spellcheck_completed, entire_time);
            strcpy(extracted.word, client_word);
            
            logbuff_insert(curr_worker, extracted);
        }
    }
    return NULL;
}


void insert(ActiveConnections *clientbuffer_ref, Client* client){
    pthread_mutex_lock(&clientbuffer_ref->connections_lock);
    while(clientbuffer_ref->numItems == clientbuffer_ref->capacity){
        pthread_cond_wait(&clientbuffer_ref->is_space, &clientbuffer_ref->connections_lock);
    }
    clientbuffer_ref->internal_activefd[clientbuffer_ref->curr_idx] = *client;
    printf("INSERTED CLIENT FD = %d at INDEX = %ld\n",clientbuffer_ref->internal_activefd[clientbuffer_ref->curr_idx].my_fd,clientbuffer_ref->curr_idx);
    clientbuffer_ref->numItems++;
    clientbuffer_ref->curr_idx = (clientbuffer_ref->curr_idx + 1) % clientbuffer_ref->capacity;
    pthread_mutex_unlock(&clientbuffer_ref->connections_lock);
    pthread_cond_signal(&clientbuffer_ref->no_space);
}

void handle_connections(int listening_fd, ActiveConnections *clientbuffer_ref, ServerConfiguration *config, LogBuffer* logbuff){
    int c;
    struct timeval start;
    struct tm *start_info;
    char time_accepted[200],entire_time[300];
    struct sockaddr_in client;
    c = sizeof(struct sockaddr_in);
    
    while(1){
        Client* curr = malloc(sizeof(Client));
        if(!curr){perror("malloc space for accepted client");}
		int* connfdp = malloc(sizeof(int));
		if(!connfdp){perror("error malloc fd for new client");}
        *connfdp = accept(listening_fd, (struct sockaddr*)&client, (socklen_t*)&c);
        if(*connfdp < 0){printf("client failed to be accepted!\n");exit(EXIT_FAILURE);}
        int randomprio = (rand()%10)+1;
        gettimeofday(&start, NULL);
        start_info = localtime(&start.tv_sec);
        strftime(time_accepted, 80, "%Y-%m-%d %H:%M:%S", start_info);
        snprintf(entire_time,300,"%s.%ld\n",time_accepted, start.tv_usec / 1000);
        curr->addr = client;
        curr->my_fd = *connfdp;
        curr->assigned_priority = randomprio;
        curr->len = c;
        strcpy(curr->time_acceptance,entire_time);
		insert(clientbuffer_ref,curr);
    }
}

int listen_fd(int port) {
    int server_fd;
    struct sockaddr_in address;
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {perror("socket failed");exit(EXIT_FAILURE);}
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( port );
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {perror("bind failed");exit(EXIT_FAILURE);}
    if (listen(server_fd, 40) < 0) {perror("listen");exit(EXIT_FAILURE);}
    return server_fd;
}

int main(int argc, char* argv[]) {
    ServerConfiguration *config = malloc(sizeof(ServerConfiguration));
    if(!config){perror("malloc server config in main");}
    int opt;

        while( (opt = getopt(argc,argv,"w:j:k:e:b") ) != -1){
            switch(opt) {
                case 'w':   //worker threads
                    config->num_workers = atoi(optarg);
                    break;
                case 'j':   //buffer capacity
                    config->buffer_capacity = atoi(optarg);
                    break;
                case 'b':   //dictionary
                    config->dictionary = argv[optind];
                    if(config->dictionary == NULL){ config->dictionary = DEFAULT_DICTIONARY;}
                    break;
                case 'k':   //port number
                    config->port = atoi(optarg);
                    break;
                case 'e':   //scheduling type
                    config->sched_type = atoi(optarg);
                    break;
                case ':':
                    fprintf(stderr, "Option %c\n needs a value", opt);
                    exit(EXIT_FAILURE);

                case '?':
                    fprintf(stderr, "Unrecognized option: %c\n", opt);
                    exit(EXIT_FAILURE);
            }
        }

    int valid_flags = valid_configflags(config);
    if(!valid_flags){exit(EXIT_FAILURE);}
    int runnable_server = valid_server(config);
    if(!runnable_server){exit(EXIT_FAILURE);}
    
    ActiveConnections *clientBuffer = malloc(sizeof(ActiveConnections));
    if(!clientBuffer){perror("mallocing connection buffer");}
    clientBuffer->capacity = config->buffer_capacity;
    clientBuffer->numItems = 0;
    clientBuffer->curr_idx = 0;
    clientBuffer->extract_idx = 0;
    clientBuffer->internal_activefd = malloc(sizeof(Client) * clientBuffer->capacity);
    if(!clientBuffer->internal_activefd){perror("error mallocing internal client buffer");}

    LogBuffer *log_buff = malloc(sizeof(LogBuffer));
    if(!log_buff){perror("malloc log buffer in main");}
    log_buff->insert_idx = 0;
    log_buff->extract_idx = 0;
    log_buff->numItems = 0;
    log_buff->capacity = config->buffer_capacity;
    log_buff->internal_buffer = malloc(sizeof(Client) * log_buff->capacity);
    if(!log_buff->internal_buffer){perror("malloc log buff internal char* buffer in main");}
    pthread_t logging_thread;
    LogThread logth;
    logth.logbuff_ref = log_buff;
    logth.config_ref = config;
    logth.logFile = "logFile.txt";
    
    pthread_mutex_init(&log_buff->logbuff_lock,NULL);
    pthread_cond_init(&log_buff->is_full,NULL);
    pthread_cond_init(&log_buff->is_empty,NULL);
    pthread_mutex_init(&clientBuffer->connections_lock,NULL);
    pthread_cond_init(&clientBuffer->no_space,NULL);
    pthread_cond_init(&clientBuffer->is_space,NULL);

    WorkerThreads *th_arr = malloc(sizeof(WorkerThreads) * config->num_workers); 
    if(!th_arr){perror("error mallocing space for all worker threads");}
    th_arr->curr_idx = 0;
    th_arr->capacity = config->num_workers;
	for(int i=0;i<config->num_workers;i++){
		th_arr[i].clientsWaiting = clientBuffer;
		th_arr[i].config_ref = config;
        th_arr[i].logbuff_ref = log_buff;
		if(pthread_create(&th_arr[i].thread_id,NULL,&work_threads,(void*)&th_arr[i])!=0){perror("error init worker threads");}
	} 

    if(pthread_create(&logging_thread,NULL,&extract_logfile,(void*)&logth) != 0){perror("log thread init");}      
    int listenfd = listen_fd(config->port);
    handle_connections(listenfd, clientBuffer,config,log_buff);   

    free(config);
    free(clientBuffer->internal_activefd);
    free(clientBuffer);
    free(th_arr);
    free(log_buff);
    free(log_buff->internal_buffer);    
    pthread_mutex_destroy(&clientBuffer->connections_lock);
    pthread_cond_destroy(&clientBuffer->no_space);
    pthread_cond_destroy(&clientBuffer->is_space);
    pthread_cond_destroy(&log_buff->is_full);
    pthread_cond_destroy(&log_buff->is_empty);
    pthread_mutex_destroy(&log_buff->logbuff_lock);
    return 0;
}