/*
 * Функция завершения работы сервера.
 *
 * При вызове, закрывает различные дескрипторы и в целом подчищает данные.
 * По завершению своей работы, возвращает 0.
 * Предполагается, что функция вызывается только, если нужно закрыть программу,
 * но сама она её не закрывает.
 *
 * Созданно: 02.09.20.
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

int init_shut()
{
  const char *section = "INIT SHUT";

  int count;
  int result;

  /* Используется только для передачи имени клиентских потоков */
  char thread_name[9];

  memset(thread_name, 0, sizeof(thread_name));

  printf("[%s] - Called\n", section);

/*##############################################################################
 * Остановка потоков
 *##############################################################################
 */

 /*
  * Работа с данными идёт только, если они были изменены (например, не 0),
  * и не возвращали при этом ошибку (например, больше нуля).
  *
  * Первым делом закрываются все потоки, чтобы как можно раньше исключить работу
  * с данными.
  */

 /*
  * Поток обработки ввода.
  * Если мьютекс окажется заблокированным, значит функцию и вызвал сам поток
  * контроля ввода. Чтобы он не закрыл сам себя, он и
  * заблокировал мьютекс ранее. Поток сам закроет себя, когда закончит
  * выполнение "init_shut". Иначе, если сюда вошёл любой другой поток - он
  * первым делом закроет поток обработки ввода.
  */
  if (pthread_mutex_trylock(&input_handling_lock) == 0)
  {
    if (input_handling_tid > 0)
    {
      close_thread(input_handling_tid, "INPUT HANDLING");
      pthread_mutex_unlock(&input_handling_lock);
      /*
      * Мьютекс больше нигде не используется. Можно его уничтожить.
      *
      * ВАЖНО: Уничтожить мьютекс здесь сможет только любой поток, который НЕ
      * является потоком обработки команд ввода ("input_handling"). Специально
      * для его случая, мьютекс удаляется в том же потоке, после выполнения
      * функции "init_shut", но до выхода из потока.
      */
      if (pthread_mutex_destroy(&input_handling_lock) == 0)
      {
        printf("[%s] - Input handling mutex destroyed\n", section);
      }
      else
      {
        printf("[%s] - Unable to destroy input handling mutex."\
        "Proceeding anyway...\n", section);
      }
    }
 }

 /* Поток принятия подключений */
 if (network_accept_tid > 0)
 {
   close_thread(network_accept_tid, "NET ACCEPT");
 }

 /* Клиентские потоки */
 for (count = 0; count <= client_max_id; count++)
 {
   if (network_cl_handling_tid[count] > 0)
   {
     sprintf(thread_name, "NET CL#%d", count);
     close_thread(network_cl_handling_tid[count], thread_name);
   }
 }

 /* Поток сетевой рассылки */
 if (network_dist_tid > 0)
 {
   close_thread(network_dist_tid, "NET DIST");
   /* Уничтожение мьютекса */
   pthread_mutex_destroy(&ready_count_lock);
 }

 /* Поток сетевого контроля */
 if (network_control_tid > 0)
 {
   close_thread(network_control_tid, "NET CONTROL");
 }

/*##############################################################################
 * Закрытие дескрипторов
 *##############################################################################
 */

  /* TCP сокет */
  if (tcp_sock_desc > 0)
  {
    count = 0;
    while (((result = close(tcp_sock_desc)) < 0) && (count < MAX_ATTEMPTS))
    {
      count++;
      sleep(SLEEP_TIME);
    }
    if (result >= 0)
    {
      printf("[%s] - TCP socket closed\n", section);
    }
    else
    {
      printf("[%s] - Unable to close TCP socket. Proceeding anyway...\n",
              section);
    }
  }

  /* UDP сокет */
  if (udp_sock_desc > 0)
  {
    count = 0;
    while (((result = close(udp_sock_desc)) < 0) && (count < MAX_ATTEMPTS))
    {
      count++;
      sleep(SLEEP_TIME);
    }
    if (result >= 0)
    {
      printf("[%s] - UDP socket closed\n", section);
    }
    else
    {
      printf("[%s] - Unable to close UDP socket. Proceeding anyway...\n",
              section);
    }
  }

  /* Очереди сообщений */
  /* Локальная */
  if (local_mq_desc > 0)
  {
    /* Дескриптор */
    count = 0;
    while (((result = mq_close(local_mq_desc)) < 0) && (count < MAX_ATTEMPTS))
    {
      count++;
      sleep(SLEEP_TIME);
    }
    if (result >= 0)
    {
      printf("[%s] - Local message queue closed\n", section);
    }
    else
    {
      printf("[%s] - Unable to close local message queue. "\
              "Proceeding anyway...\n", section);
    }
    /* Файл очереди */
    count = 0;
    while (((result = mq_unlink(LOCAL_MQ)) < 0) && (count < MAX_ATTEMPTS))
    {
      count++;
      sleep(SLEEP_TIME);
    }
    if (result >= 0)
    {
      printf("[%s] - Local message queue unlinked\n", section);
    }
    else
    {
      printf("[%s] - Unable to unlink local message queue. "\
              "Proceeding anyway...\n", section);
    }
  }

  /* Сетевая */
  if (net_mq_desc > 0)
  {
    /* Дескриптор */
    count = 0;
    while (((result = mq_close(net_mq_desc)) < 0) && (count < MAX_ATTEMPTS))
    {
      count++;
      sleep(SLEEP_TIME);
    }
    if (result >= 0)
    {
      printf("[%s] - Network message queue closed\n", section);
    }
    else
    {
      printf("[%s] - Unable to close network message queue. "\
              "Proceeding anyway...\n", section);
    }
    /* Файл очереди */
    count = 0;
    while (((result = mq_unlink(NET_MQ)) < 0) && (count < MAX_ATTEMPTS))
    {
      count++;
      sleep(SLEEP_TIME);
    }
    if (result >= 0)
    {
      printf("[%s] - Network message queue unlinked\n", section);
    }
    else
    {
      printf("[%s] - Unable to unlink network message queue. "\
              "Proceeding anyway...\n", section);
    }
  }

  return 0;
}
