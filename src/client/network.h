#ifndef NETWORK_H
#define NETWORK_H

#define SERVER_PORT 1234              // порт сервера
#define SERVER_ADDR "185.255.132.26"  // адрес сервера

void* net_check(void* args);
void* client_check(void* args);

#endif // NETWORK_H
