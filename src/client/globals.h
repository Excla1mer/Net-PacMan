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
pthread_mutex_t mutex;
sem_t sem;

#endif // GLOBALS_H
