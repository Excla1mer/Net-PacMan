#ifndef NET_DATA_DEFS_H
#define NET_DATA_DEFS_H

/* Формат данных: data[TYPE, DATA, DATA] */

/* TYPE */
#define ID_PORT 0     /* data[ID_PORT, ID, PORT] */
#define READY 1       /* data[READY, -, -] */
#define START 2       /* data[START, -, -] */
#define CL_READY 3    /* data[CL_READY, ID, (число клиентов)] */
#define CL_CONNECT 4  /* data[CL_CONNECT, ID, (число клиентов)] */
#define CL_DIR 5      /* data[CL_DIR, ID, DIR] */

#define ENDGAME 9      /* data[ENDGAME, ID, -] */

int max_players;

#endif /* NET_DATA_DEFS_H */
