/*
 * Все глобальные определения и переменные программы.
 * Некоторые данные могут использоваться лишь в одной функции программы, но всё
 * равно вынесены сюда, для быстрого доступа к ним (например, порт сокета).
 *
 * Созданно: 01.09.20.
 * Автор: Денис Пащенко.
 */

/* Порты сокетов */
#define SERVER_PORT 1234

/* Максимальное число игроков, поддерживаемое сервером. Не должно быть более 4*/
#define MAX_PLAYERS 4

/* Значения, используемые в различных попытках чего-либо достичь */
#define MAX_ATTEMPTS 3
#define SLEEP_TIME 1

/* Имя файла-очереди сообщений */
/* ПРИМЕЧАНИЕ: вероятно, две очереди нужны не будут, но пока пусть будут. */
#define LOCAL_MQ "/PM-server_localMQ"
#define NET_MQ "/PM-server_netMQ"

/*##############################################################################
 * Глобальные переменные
 *##############################################################################
 */

/* Атрибуты потоков. Устанавливаются в main.*/
pthread_attr_t threadAttr;

/* Дескрипторы очередей сообщений */
mqd_t local_mq_desc;
mqd_t net_mq_desc;

/* Дескрипторы сокетов */
int tcp_sock_desc;
int udp_cl_sock_desc[MAX_PLAYERS];

/* Максимальный ID клиента. Также, общее число клиентов-1 */
int client_max_id;

/* Счётчик готовых клиентов */
int ready_count;

/* Сетевые данные сервера */
struct sockaddr_in server_addr_struct;

/* Сетевые данные клиентов */
struct sockaddr_in net_client_addr[MAX_PLAYERS];
int net_client_desc[MAX_PLAYERS];
socklen_t net_client_addr_size[MAX_PLAYERS];

/* tid-ы*/
pthread_t input_handling_tid;
pthread_t network_control_tid;
pthread_t network_accept_tid;
pthread_t network_sync_tid;
pthread_t network_dist_tid;
pthread_t network_cl_handling_tid[MAX_PLAYERS];

/* Мьютекс */
pthread_mutex_t input_handling_lock;
pthread_mutex_t ready_count_lock;
pthread_mutex_t new_port_lock;

/* Семафор */
sem_t endgame_lock;
