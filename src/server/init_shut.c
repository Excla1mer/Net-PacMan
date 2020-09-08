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
#include <semaphore.h>
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

  /* Используется для передачи клиентских данных */
  char name[9];

  memset(name, 0, sizeof(name));

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
     sprintf(name, "NET CL#%d", count);
     close_thread(network_cl_handling_tid[count], name);
   }
 }

 /* Поток сетевой синхронизации */
 if (network_sync_tid > 0)
 {
   close_thread(network_sync_tid, "NET SYNC");
 }

 /* Поток сетевой рассылки */
 if (network_dist_tid > 0)
 {
   close_thread(network_dist_tid, "NET DIST");
 }

 /* Поток сетевого контроля */
 if (network_control_tid > 0)
 {
   close_thread(network_control_tid, "NET CONTROL");

   /*
    * Уничтожение мьютексов и семафоров. Они инициализировались вместе с потоком
    * сетевого контроля, и с ним же уничтожаются.
    */
   /* Мьютекс счётчика готовности */
   if (pthread_mutex_destroy(&ready_count_lock) == 0)
   {
     printf("[%s] - Ready count mutex destroyed\n", section);
   }
   else
   {
     printf("[%s] - Unable to destroy ready count mutex."\
            "Proceeding anyway...\n", section);
   }
   /* Мьютекс подбора нового порта */
   if (pthread_mutex_destroy(&new_port_lock) == 0)
   {
     printf("[%s] - New port searching mutex destroyed\n", section);
   }
   else
   {
     printf("[%s] - Unable to destroy new port searching mutex."\
            "Proceeding anyway...\n", section);
   }
 }

/*##############################################################################
 * Закрытие дескрипторов
 *##############################################################################
 */

  /* TCP сокет */
  if (tcp_sock_desc > 0)
  {
    close_sock(tcp_sock_desc, "TCP");
  }

  /* Личные UDP сокеты клиентов */
  for (count = 0; count <= client_max_id; count++)
  {
    if (udp_cl_sock_desc[count] > 0)
    {
      sprintf(name, "UDP CL#%d", count);
      close_sock(udp_cl_sock_desc[count], name);
      memset(name, 0, sizeof(name));
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
