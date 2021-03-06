#ifndef GLOBALS_H
#define GLOBALS_H

#include <pthread.h>
#include <semaphore.h>

int max_players;
float ttime;
int udp_server_port;
int my_id;
int udp_sockfd;
int tcp_sockfd;
int dots;
int end_game;
sem_t sem;
pthread_mutex_t mutex;
pthread_mutex_t dot_mutex;
pthread_t listen_thread_tid;

#endif // GLOBALS_H
