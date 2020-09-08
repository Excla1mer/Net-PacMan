/*
 * Основная функция сервера.
 *
 * Подготавливает данные; создаёт и настраивает сокеты; очередь сообщений;
 * запускает потоки.
 * Ждёт момента, чтобы завершить работу программы.
 *
 * Созданно: 01.09.20.
 * Автор: Денис Пащенко.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <mqueue.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "server_defs.h"
#include "server_protos.h"

/*##############################################################################
 * Глобальные переменные
 *##############################################################################
 */

int main()
{
/*##############################################################################
 * Объявление и определение/подготовка данных
 *##############################################################################
 */

  /*
   * section - Строка, описывающая текущую секцию. Используется в терминальных
   * выводах для однозначного указания на отчитывающуюся функцию.
   * Например: "[MAIN] - Something changed!\n".
   * Такое написание упрощает переносы строк вывода информации из одной части
   * кода, в другую.
   */
  const char *section = "MAIN";

  int count;
  int ret;

  /* Структура атрибутов очередей */
  struct mq_attr queueAttr;

  /* Настройка атрибутов потоков */
  pthread_attr_init(&threadAttr);
  pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_JOINABLE);

  /* Установка параметров структуры адреса */
  server_addr_struct.sin_family = AF_INET;
  server_addr_struct.sin_addr.s_addr = htonl(INADDR_ANY);
  /* SERVER_PORT определён в defs.h */
  server_addr_struct.sin_port = htons(SERVER_PORT);

  /*Настройка атрибутов очереди*/
  mq_msg_size = sizeof(int)*NET_DATA_SIZE;
  queueAttr.mq_flags = 0;
  queueAttr.mq_maxmsg = 10;
  queueAttr.mq_msgsize = mq_msg_size;
  queueAttr.mq_curmsgs = 0;

  /*
   * Зануление различных дескрипторов на старте. Если всё проходит без ошибок,
   * они всё равно будут переопределенны. Иначе, на значения будет смотреть
   * функция "init_shut".
   */
  tcp_sock_desc = 0;
  for (count = 0; count < MAX_PLAYERS; count++)
  {
    udp_cl_sock_desc[count] = 0;
  }
  local_mq_desc = 0;
  net_mq_desc = 0;

  input_handling_tid = 0;
  network_control_tid = 0;
  network_accept_tid = 0;
  network_sync_tid = 0;
  network_dist_tid = 0;
  memset(network_cl_handling_tid, 0, sizeof(network_cl_handling_tid));

  client_max_id = -1;

  ready_count = 0;

  printf("[%s] - Started\n", section);

/*##############################################################################
 * Запуск потока обработки терминальных команд (input_handling)
 *##############################################################################
 */

  /*
   * Однотипных запусков стало так много, что они были вынесены в отдельную
   * функцию "launch_thread"
   */
  if (launch_thread(&input_handling_tid, input_handling, "INPUT HANDLING") != 0)
  {
    /* Ничего подчищать не нужно, так как ничего и не было создано. Выход. */
    printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n"\
            "[%s] - Failed on crucial part here. Exiting program...\n",
            section);
    exit(0);
  }
  /* При успешном запуске потока, инициализируется мьютекс. */
  input_handling_lock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;

/*##############################################################################
 * Инициализация сокета
 *##############################################################################
 */

 /*
  * Вся работа с сокетами в одном бесконечном цикле. Но этот цикл всё равно
  * будет окончен в любом случае.
  * Цикл выступает логическими скобками, которые держат весь блок работы с
  * сокетами вместе. Если один из этапов провалится - дальше можно будет не
  * продолжать. Значит, цикл оборвётся на этом этапе.
  * На выходе из цикла есть условие, что следит за значением ret, которое и
  * укажет на то, как прошла работа цикла.
  */
  ret = -1;
  while(1)
  {
    /* TCP */
    /* Создание сокета. */
    if ((tcp_sock_desc = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
      /* Это ошибка. Сообщить о ней, и разорвать цикл. */
      perror("TCP SOCKET");
      break;
    }
    printf("[%s] - (TCP) Socket created\n", section);

    /* Привязка сокета */
    if ((bind(tcp_sock_desc, (struct sockaddr *)&server_addr_struct,
              sizeof(server_addr_struct))) == -1)
    {
      perror("TCP BIND");
      break;
    }
    printf("[%s] - (TCP) Socket binded on [%s:%d]\n", section,
            inet_ntoa(server_addr_struct.sin_addr),
            ntohs(server_addr_struct.sin_port));

    /* Перевод сокета в режим прослушки */
    if ((listen(tcp_sock_desc, 0)) == -1)
    {
      perror("TCP LISTEN");
      break;
    }
    printf("[%s] - (TCP) Socket set to listening mode\n", section);

    /*
     * Цикл не был разорван ранее, а значит, всё прошло нормально и все задачи
     * с сокетами выполнены успешно.
     * Меняется значение ret и разрывается цикл.
     */
    ret = 0;
    break;
  }

  /* ret по умолчанию равен -1. Если цикл разорвала ошибка, то это значение не
  * поменялось, а значит продолжать работу нет смысла. Сервер закроется.
  * Если же значение ret было измененно (=/= -1) то все задачи были выполнены
  * и можно продолжать работу сервера. */
  if (ret == -1)
  {
    /* Здесь перед выходом уже потребуется очистка */
    printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n"\
            "[%s] - Failed on crucial part here. Exiting program...\n",
            section);
    init_shut();
    exit(0);
  }

/*##############################################################################
 * Создание очередей сообщений
 *##############################################################################
 */

  /* Применяется та же логика цикла, что и выше */
  ret = -1;
  while (1)
  {
    if ((local_mq_desc = mq_open(LOCAL_MQ, O_RDWR | O_CREAT | O_NONBLOCK, 0655,
      &queueAttr)) == -1)
      {
        perror("LOCAL MQ");
        break;
      }
    printf("[%s] - Local message queue created\n", section);

    if ((net_mq_desc = mq_open(NET_MQ, O_RDWR | O_CREAT | O_NONBLOCK, 0655,
      &queueAttr)) == -1)
      {
        perror("NET MQ");
        break;
      }
    printf("[%s] - Network message queue created\n", section);
    ret = 0;
    break;
  }

  if (ret == -1)
  {
    printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n"\
            "[%s] - Failed on crucial part here. Exiting program...\n",
            section);
    init_shut();
    exit(0);
  }

/*##############################################################################
 * Запуск потока сетевого контроля (network_control)
 *##############################################################################
 */

  /* Как и запуск потока обработки ввода, провал здесь завершит программу */
  if (launch_thread(&network_control_tid, network_control,
                    "NET CONTROL") != 0)
  {
    printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n"\
            "[%s] - Failed on crucial part here. Exiting program...\n",
            section);
   init_shut();
   exit(0);
  }
  ready_count_lock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
  new_port_lock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
  restart_flag = 0;

/*##############################################################################
 * Свободное ожидание
 *##############################################################################
 */

  /*
   * Сейчас, окончив работу выше, "main" просто ждёт конца работы потока
   * "input_handling". Дождавшись его, "main" полагает, что нужно окончить
   * работу сервера. Данные уже должен был почистить сам "input_handling", так
   * как завершился он только после команды /shut - конец работы программы.
   */
  pthread_join(input_handling_tid, NULL);
  printf("[%s] - Input handling exited\n", section);

  printf("[%s] - Server shuted down\n", section);
  exit(0);
}
