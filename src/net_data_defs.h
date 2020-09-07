#ifndef NET_DATA_DEFS_H
#define NET_DATA_DEFS_H

/* Формат данных: data[TYPE, DATA, DATA] */

/* TYPE */
#define ID_PORT 0
#define READY 1
#define START 2
#define CL_READY 3
#define CL_CONNECT 4

int max_players;

#endif /* NET_DATA_DEFS_H */
