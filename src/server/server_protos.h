/*
 * Прототипы функций и функций-потоков.
 *
 * Созданно: 01.09.20.
 * Автор: Денис Пащенко.
 */

/* Потоки */
void *input_handling();
void *network_control();
void *network_accept();
void *network_cl_handling();
void *network_dist();
void *network_sync();

/* Функции */
int init_shut();
int launch_thread(pthread_t*, void *(*)(void *), const char*);
int close_thread(pthread_t, const char*);
