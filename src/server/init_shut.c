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
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <mqueue.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "server_defs.h"

int init_shut()
{
  const char *section = "INIT SHUT";

  int count;
  int result;

  printf("[%s] - Called\n", section);

/*##############################################################################
 * Остановка потоков
 *##############################################################################
 */

 /*
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
   if(input_handling_tid > 0)
   {
     if (pthread_cancel(input_handling_tid) == 0)
     {
       printf("[%s] - Input handling thread canceled\n", section);
     }
     else
     {
       printf("[%s] - Unable to cancel input handling thread."\
       "Proceeding anyway...\n", section);
     }
   }
   pthread_mutex_unlock(&input_handling_lock);
 }

 /* Поток сетевого контроля */
 if (network_control_tid > 0)
 {
   if (pthread_cancel(network_control_tid) == 0)
   {
     printf("[%s] - Network control thread canceled\n", section);
   }
   else
   {
     printf("[%s] - Unable to cancel network control thread."\
     "Proceeding anyway...\n", section);
   }
 }

 /* Клиентские потоки */
 for (count = 0; count < PLAYERS; count++)
 {
   if (client_thread_tid[count] > 0)
   {
     if (pthread_cancel(client_thread_tid[count]) == 0)
     {
       printf("[%s] - Client #%d thread canceled\n", section, count);
     }
     else
     {
       printf("[%s] - Unable to cancel client #%d thread. "\
       "Proceeding anyway...\n", section, count);
     }
   }
 }

/*##############################################################################
 * Закрытие дескрипторов
 *##############################################################################
 */

  /*
   * Работа с данными идёт только, если они были изменены (например, не 0),
   * и не возвращали при этом ошибку (например, больше нуля).
   *
   * По сути, со всеми данными работает один и тот же код. Можно его
   * унифицировать, но в некоторых случаях, может потребоваться немного иное
   * поведение, так что, пока что, это всё куча очень похожих друг на друга
   * if-ов.
   */

  /* TCP сокет */
  if (tcp_sock_desc > 0)
  {
    count = 0;
    /*
     * До MAX_ATTEMPTS попыток на действия.
     * Иначе - окончить попытки и продолжить.
     */
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

  /* Очередь сообщений */
  if (mq_desc > 0)
  {
    /* Дескриптор */
    count = 0;
    while (((result = mq_close(mq_desc)) < 0) && (count < MAX_ATTEMPTS))
    {
      count++;
      sleep(SLEEP_TIME);
    }
    if (result >= 0)
    {
      printf("[%s] - Message queue closed\n", section);
    }
    else
    {
      printf("[%s] - Unable to close message queue. Proceeding anyway...\n",
              section);
    }

    /* Файл очереди */
    count = 0;
    while (((result = mq_unlink(MESSAGE_QUEUE)) < 0) && (count < MAX_ATTEMPTS))
    {
      count++;
      sleep(SLEEP_TIME);
    }
    if (result >= 0)
    {
      printf("[%s] - Message queue unlinked\n", section);
    }
    else
    {
      printf("[%s] - Unable to unlink message queue. Proceeding anyway...\n",
              section);
    }
  }

  return 0;
}
