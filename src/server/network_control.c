/*
 * Основной поток контроля сетевого взаимодействия.
 *
 * Ожидает подключение клиентов. Передаёт их созданным здесь же потокам.
 * В дальнейшем, берёт на себя ещё больше сетевых задач.
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

void *network_control()
{
/*##############################################################################
 * Объявление и определение/подготовка данных
 *##############################################################################
 */
  const char *section = "NET CONTROL";

  char net_data[50];


  printf("[%s] - Started\n", section);

/*##############################################################################
 * Подключение и подготовка игроков
 *##############################################################################
 */

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*
 * Вся секция сугубо тестовая. Свою работу должен сохранить механизм работы с
 * accept, хотя и с изменениями. Остальное стоит дополнить. Особенно, действия
 * по окончанию следующего цикла.
 */

  /*
   * ВРЕМЕННО: ожидание до PLAYERS подключений. Затем программа зависает на
   * заглушке в самом конце кода.
   */
  while (client_max_id < PLAYERS)
  {
    net_client_addr_size[client_max_id] = sizeof(net_client_addr[client_max_id]);
    net_client_desc[client_max_id] = accept(tcp_sock_desc,
                            (struct sockaddr *)&net_client_addr[client_max_id],
                            &net_client_addr_size[client_max_id]);
      if (net_client_desc[client_max_id] > 0)
      {
        printf("[%s] - (TCP) Client [%s] connected\n", section,
                inet_ntoa(net_client_addr[client_max_id].sin_addr));

        sprintf(net_data, "%d", client_max_id);
        while (mq_send(mq_desc, net_data, strlen(net_data),
                      0) != 0) {}
        memset(net_data, 0, sizeof(net_data));
        /* До 5ти попыток на запуск потока.*/
        while ((pthread_create(&client_thread_tid[client_max_id], &threadAttr,
                              client_thread, NULL) != 0))
        {
          /*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
          /*ЗДЕСЬ НУЖНА НОРМАЛЬНАЯ ОБРАБОТКА*/
        }
      }
      client_max_id++;
  }

  /* ЗАГЛУШКА */
  while (1) {sleep(10);}
}
