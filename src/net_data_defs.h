#ifndef NET_DATA_DEFS_H
#define NET_DATA_DEFS_H

/*##############################################################################
 * Общий формат сетевых данных
 *##############################################################################
 */

/* Формат данных: data[TYPE, DATA1, DATA2, ... ] */

/* TYPE */
#define ID_PORT 0     /* data[ID_PORT, ID, PORT] */
#define READY 1       /* data[READY, -, -] */
#define START 2       /* data[START, -, -] */
#define CL_READY 3    /* data[CL_READY, ID, (число клиентов)] */
#define CL_CONNECT 4  /* data[CL_CONNECT, ID, (число клиентов)] */
#define CL_DIR 5      /* data[CL_DIR, ID, DIR] */
#define SYN_REQ 6     /* data[SYN_REQ, -, -] */
#define SYN_REP 7     /* data[SYN_REP, ID, x, dx, y, dy, score]*/
#define ENDGAME 9     /* data[ENDGAME, ID, -] */

/*##############################################################################
 * Общие константы
 *##############################################################################
 */

/*
 * Порт TCP соединения, прослушиваемого сервером.
 * Общение клиента и сервера начинаетя здесь.
 */
#define SERVER_TCP_PORT 1234

/* Размер массива сетевых данных, пересылаемых между клиентом и сервером */
#define NET_DATA_SIZE 7

/*##############################################################################
 * Константы сервера
 *##############################################################################
 */

/* Частота, в секундах, повтора синхронизации */
#define SYNC_FREQ 3
/*
 * Пауза, в секундах, перед тем, как сервер начнёт проводить синхронизацию
 * (промежуток времени, когда поток синхронизации уже запущен, но ещё не начал
 * свою работу)
 */
#define SYNC_DELAY 5

#endif /* NET_DATA_DEFS_H */
