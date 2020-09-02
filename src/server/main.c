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
  char message[100];
  struct sockaddr_in server_addr_struct;

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
  queueAttr.mq_flags = 0;
  queueAttr.mq_maxmsg = 10;
  queueAttr.mq_msgsize = 50;
  queueAttr.mq_curmsgs = 0;

  /*
   * Зануление различных дескрипторов на старте. Если всё проходит без ошибок,
   * они всё равно будут переопределенны. Иначе, на значения будет смотреть
   * функция "init_shut".
   */
  tcp_sock_desc = 0;
  udp_sock_desc = 0;
  mq_desc = 0;

  input_handling_tid = 0;
  network_control_tid = 0;
  memset(client_thread_tid, 0, sizeof(client_thread_tid));

  client_max_id = 0;
  memset(message, 0, sizeof(message));

  printf("[%s] - Started\n", section);

/*##############################################################################
 * Запуск потока обработки терминальных команд (input_handling)
 *##############################################################################
 */

 count = 0;
 while (pthread_create(&input_handling_tid, &threadAttr, input_handling,
                        NULL) > 0)
  {
    /* Если создание потока проваливается - повторять до MAX_ATTEMPTS попыток */
    if (count < MAX_ATTEMPTS)
    {
      printf("[%s] - Failed to create input hadling thread."\
      "Retrying again in %d seconds...\n", section, SLEEP_TIME);
      sleep(SLEEP_TIME);
      count++;
    }
    /* Иначе - полный провал старта потока. Вся программа завершается. */
    else
    {
      printf("[%s] - Input hadling thread failed to start after %d attempts."\
              "Exiting program...\n", section, count+1);

      /* Ничего подчищать не нужно, так как ничего и не было сделано. */
      exit(0);
    }
  }
  /* При успешном запуске потока, инициализируется мьютекс. */
  input_handling_lock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;

/*##############################################################################
 * Инициализация сокета
 *##############################################################################
 */

  /* TCP */
  /* Создание сокета. */
  while ((tcp_sock_desc = socket(AF_INET, SOCK_STREAM, 0)) == -1) {}
  printf("[%s] - (TCP) Socket created\n", section);

  /* Привязка сокета */
  while ((bind(tcp_sock_desc, (struct sockaddr *)&server_addr_struct,
          sizeof(server_addr_struct))) == -1) {}
  printf("[%s] - (TCP) Socket binded\n", section);

  /* Перевод сокета в режим прослушки */
  while ((listen(tcp_sock_desc, 0)) == -1) {}
  printf("[%s] - (TCP) Socket set to listening mode\n", section);

  /* UDP */
  /* Создание сокета. */
  while ((udp_sock_desc = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {}
  printf("[%s] - (UDP) Socket created\n", section);

  /* Привязка сокета */
  while ((bind(udp_sock_desc, (struct sockaddr *)&server_addr_struct,
          sizeof(server_addr_struct))) == -1) {}
  printf("[%s] - (UDP) Socket binded\n", section);

/*##############################################################################
 * Создание очереди сообщений
 *##############################################################################
 */

  while ((mq_desc = mq_open(MESSAGE_QUEUE, O_RDWR | O_CREAT, 0655,
                                  &queueAttr)) == -1) {}
  printf("[%s] - Message queue created\n", section);

/*##############################################################################
 * Запуск потока сетевого контроля (network_control)
 *##############################################################################
 */

 /* Как и запуск потока обработки ввода, провал здесь завершит программу */
 count = 0;
 while (pthread_create(&network_control_tid, &threadAttr, network_control,
                        NULL) > 0)
  {
    if (count < MAX_ATTEMPTS)
    {
      printf("[%s] - Failed to create network control thread."\
      "Retrying again in %d seconds...\n", section, SLEEP_TIME);
      sleep(SLEEP_TIME);
      count++;
    }
    else
    {
      printf("[%s] - Network control thread failed to start after %d attempts."\
              "Exiting program...\n", section, count+1);

      /* Теперь придётся почистить данные, которые уже успели обработаться */
      init_shut();
      exit(0);
    }
  }

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
