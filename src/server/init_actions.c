/*
 * Различные функции работы с потоками и дескрипторами.
 *
 * Сейчас содержит функции запуска, завершения потока и закрытия дескриптора.
 *
 * Созданно: 03.09.20.
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
 * Запуск потока
 *##############################################################################
 */

int launch_thread(pthread_t *tid, void *(*thread)(void *), const char *thread_name)
{
  const char *section = "LAUNCH THREAD";

  int count;

  count = 0;
  while (pthread_create(tid, &threadAttr, thread, NULL) != 0)
   {
     if (count < MAX_ATTEMPTS)
     {
       /*
       printf("[%s] - Failed to create (%s) thread."\
               "Retrying again in %d seconds...\n",
               section, thread_name, SLEEP_TIME);
        */
       sleep(SLEEP_TIME);
       count++;
     }
     else
     {
       perror("THREAD CREATE");
       printf("[%s] - Thread (%s) failed to start after %d attempts.\n",
               section, thread_name, count+1);
       return -1;
     }
   }
   /*
    * Все потоки сами сообщают о своём запуске, но на всякий случай, функция
    * тоже отчитывается.
    */
   printf("[%s] - Thread (%s) launched\n", section, thread_name);
   return 0;
}

/*##############################################################################
 * Завершение потока
 *##############################################################################
 */

int close_thread(pthread_t tid, const char *thread_name)
{
  const char *section = "CLOSE THREAD";

  int count;
  int ret;

  ret = 0;

  /* Отмена потока */
  count = 0;
  while (pthread_cancel(tid) != 0)
  {
     perror("CANCEL");
     if (count < MAX_ATTEMPTS)
     {
       /*
       printf("[%s] - Failed to cancel (%s) thread. "\
               "Retrying again in %d seconds...\n",
               section, thread_name, SLEEP_TIME);
      */
       sleep(SLEEP_TIME);
       count++;
     }
     else
     {
       perror("THREAD CANCEL");
       printf("[%s] - Failed to cancel (%s) thread after %d attempts. "\
              "Proceeding anyway...\n",
               section, thread_name, count+1);
        ret = -1;
        break;
     }
  }

   /* Присоединение */
   count = 0;
   while (pthread_join(tid, NULL) != 0)
   {
     perror("THREAD JOIN");
      if (count < MAX_ATTEMPTS)
      {
        /*
        printf("[%s] - Failed to join (%s) thread. "\
                "Retrying again in %d seconds...\n",
                section, thread_name, SLEEP_TIME);
        */
        sleep(SLEEP_TIME);
        count++;
      }
      else
      {
        printf("[%s] - Failed to join (%s) thread after %d attempts. "\
               "Proceeding anyway...\n",
                section, thread_name, count+1);
        ret = -1;
        break;
      }
    }

    if (ret >= 0)
    {
      printf("[%s] - Thread (%s) closed\n", section, thread_name);
    }
    return ret;
}

/*##############################################################################
 * Закрытие сетевых дескрипторов
 *##############################################################################
 */

int close_sock(int sock, const char *sock_name)
{
  const char *section = "CLOSE SOCK";

  int count;
  int ret;

   /* Закрытие сокета */
   count = 0;
   while (((ret = close(sock)) < 0) && (count < MAX_ATTEMPTS))
   {
     count++;
     sleep(SLEEP_TIME);
   }
   if (ret >= 0)
   {
     printf("[%s] - (%s) socket closed\n", section, sock_name);
   }
   else
   {
     perror("SOCKET CLOSE");
     printf("[%s] - Unable to close (%s) socket. Proceeding anyway...\n",
             section, sock_name);
   }
   return 0;
}

/*##############################################################################
 * Очистка очереди сообщений (изьятие всех сообщений из неё)
 *##############################################################################
 */

int clear_mq(mqd_t mq_desc, const char *mq_name)
{
  const char *section = "CLEAR MQ";

  struct mq_attr attr;

  int count;
  int buf[NET_DATA_SIZE];

  count = 0;
  memset(buf, 0, sizeof(buf));

  /* Очистка */
  while (1)
  {
   if (mq_getattr(mq_desc, &attr) < 0)
   {
     perror("MQ GETATTR");
     return -1;
   }
   /* Пока в очереди есть сообщения - считывать их */
   if (attr.mq_curmsgs > 0)
   {
     if(mq_receive(mq_desc, (char *)&buf, mq_msg_size, NULL) < 0)
     {
       perror("MQ RECV");
       return -1;
     }
     count++;
     memset(buf, 0, sizeof(buf));
   }
   else
   {
     break;
   }
  }

  if(count != 0)
  {
    printf("[%s] - Flushed (%d) messages from (%s)\n", section, count, mq_name);
  }
  return 0;
}
