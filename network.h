
#ifndef NETWORK_H
#define NETWORK_H

#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include "dictionary.h"

typedef struct{
   int num_workers;
   int buffer_capacity;
   char* dictionary;
   int port;
   int sched_type;
}ServerConfiguration;

typedef struct {
   int my_fd;
   struct sockaddr_in addr;
   int assigned_priority;
   int len;
   char time_acceptance[400];
   char time_spellcheck_completed[400];
   char word[400];
   char socket_response_value[400];
} Client;

typedef struct{
   pthread_cond_t is_space;
   pthread_cond_t no_space;
   Client* internal_activefd;
   size_t numItems;
   size_t capacity;
   size_t curr_idx;
   pthread_mutex_t connections_lock;
   int idnum;
   size_t extract_idx;
}ActiveConnections;

typedef struct{
   size_t numItems;
   size_t capacity;
   size_t insert_idx;
   size_t extract_idx;
   const size_t prio_extract_idx;
   pthread_mutex_t logbuff_lock;
   pthread_cond_t is_full;
   pthread_cond_t is_empty;
   Client* internal_buffer;
}LogBuffer;

typedef struct{
   char* logFile;
   LogBuffer *logbuff_ref;
   ServerConfiguration *config_ref;
}LogThread;

typedef struct{                               
   ActiveConnections *clientsWaiting;
   LogBuffer *logbuff_ref;
   ServerConfiguration *config_ref;
   pthread_t thread_id;                      
   size_t curr_idx;                          
   Client assigned_client;                   
   size_t capacity;
}WorkerThreads;

int valid_configflags(ServerConfiguration *config);
int valid_server(ServerConfiguration *config);

#endif