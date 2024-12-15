
#ifndef NETWORK_H
#define NETWORK_H

#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include "dictionary.h"
#include <stdbool.h>

typedef struct {
    int num_workers;
    int buffer_capacity;
    char* dictionary;
    int port;
    int sched_type;  // 1 for FIFO, 2 for Priority
} ServerConfiguration;

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

typedef struct {
    pthread_cond_t is_space;
    pthread_cond_t no_space;
    Client* internal_activefd;
    size_t numItems;
    size_t capacity;
    size_t curr_idx;
    pthread_mutex_t connections_lock;
    pthread_mutex_t priority_lock;
    int current_priority;  // Track current priority being processed
    pthread_t processing_thread;  // Track which thread is processing
    int idnum;
    size_t extract_idx;
} ActiveConnections;

typedef struct {
    size_t numItems;
    size_t capacity;
    size_t insert_idx;
    size_t extract_idx;
    pthread_mutex_t logbuff_lock;
    pthread_cond_t is_full;
    pthread_cond_t is_empty;
    Client* internal_buffer;
} LogBuffer;

typedef struct {
    char* logFile;
    LogBuffer *logbuff_ref;
    ServerConfiguration *config_ref;
} LogThread;

typedef struct {
    ActiveConnections *clientsWaiting;
    LogBuffer *logbuff_ref;
    ServerConfiguration *config_ref;
    pthread_t thread_id;
    size_t curr_idx;
    Client assigned_client;
    size_t capacity;
} WorkerThreads;

int valid_configflags(ServerConfiguration *config);
int valid_server(ServerConfiguration *config);
bool is_highest_priority_ready(ActiveConnections *buffer, pthread_t current_thread);
int find_highest_priority(ActiveConnections *buffer);
void cleanup_priority_status(ActiveConnections *buffer, pthread_t thread_id);


#endif