/*
 * Поток обработки терминальных команд на стороне сервера.
 *
 * Любые команды непосредственно для сервера стоит вносить здесь.
 * Может вывести информацию по команде "/help"
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
#include "../net_data_defs.h"

void *input_handling()
{
/*##############################################################################
 * Объявление и определение/подготовка данных
 *##############################################################################
 */

  const char *section = "INPUT HANDLING";

  char input[50];
  int count;

  const char *command_list =
    "########################################################\n"\
    " \n"\
    " Server commands:\n"\
    " \n"\
    " /shut         - close program\n"\
    " /list_players - list connected players\n"\
    " /verbose+     - Output MORE info (3 levels)\n"\
    " /verbose-     - Output LESS info (3 levels)\n"\
    " /restart      - end current and start new game session (might block!)\n"\
    " /help         - print this command list\n"\
    " \n"\
    "########################################################\n";

  /*printf("[%s] - Started\n", section);*/
  printf("%s", command_list);
  /*count = 0;*/

/*##############################################################################
 * Обработка команд.
 *##############################################################################
 */

  /* Бесконечный цикл. Не прерывается до самого конца работы программы */
  while (1)
  {
    fgets(input, sizeof(input), stdin);

    /* /shut - завершение работы сервера. Это завершит всю программу. */
    if(strcmp(input, "/shut\n") == 0)
    {
      printf("\033[0;33m"\
              "#######################################################\n"\
              "[%s] - Initializing shutdown"\
              "\033[0m\n",
               section);

      /*
       *Блокируется мьютекс, чтобы поток не завершил сам себя, выполняя
       * функцию "init_shut"
       */
      pthread_mutex_lock(&input_handling_lock);
      /* Запустить "init_shut" для очистки данных */
      init_shut();
      pthread_mutex_unlock(&input_handling_lock);
      /*
       * Уничтожение мьютекса, в случае, если функция "init_shut" была вызвана
       * из этого потока, происходит здесь, а не в "init_shut", так как там
       * уничтожать мьютекс, который заблокирован, небезопасно.
       */
      if (pthread_mutex_destroy(&input_handling_lock) == 0)
      {
        if(verbose_flag != 0)
        {
          printf("[%s] - Input handling mutex destroyed\n", section);
        }
      }
      else
      {
        printf("[%s] - Unable to destroy input handling mutex."\
               "Proceeding anyway...\n", section);
      }
      if(verbose_flag != 0)
      {
        printf("[%s] - Exiting thread...\n", section);
      }
      /* Вернуть 0. Теперь контроль перейдёт в "main"*/
      pthread_exit((void *)0);
    }

    /* /help - вывести список команд. */
    else if(strcmp(input, "/help\n") == 0)
    {
      printf("%s", command_list);
    }

    /* /list_players - вывести список подключенных клиентов. */
    else if(strcmp(input, "/list_players\n") == 0)
    {
      if(client_max_id >= 0)
      {
        printf("########################################################\n"\
                " Currently connected players:\n");
        for(count = 0; count <= client_max_id; count++)
        {
          printf("  Player#%d [%s:%d]\n",
          count,
          inet_ntoa(net_client_addr[count].sin_addr),
          ntohs(net_client_addr[count].sin_port));
        }
        printf("########################################################\n");
      }
      else
      {
        printf("\n No connected players!\n\n");
      }
    }

    /* /restart - окончание текущей и начало новой сессии игры. */
    else if(strcmp(input, "/restart\n") == 0)
    {
      restart_flag = 1;
    }

    /* /verbose+ - печатать БОЛЬШЕ информации в вывод */
    else if(strcmp(input, "/verbose+\n") == 0)
    {
      if(verbose_flag < 2)
      {
        verbose_flag++;
        printf("\n Verbose at level (%d) of (3)\n\n", verbose_flag+1);
      }
      else
      {
        printf("\n Already at maximum!\n\n");
      }
    }
    /* /verbose- - печатать МЕНЬШЕ информации в вывод */
    else if(strcmp(input, "/verbose-\n") == 0)
    {
      if(verbose_flag > 0)
      {
        verbose_flag--;
        printf("\n Verbose at level (%d) of (3)\n\n", verbose_flag+1);
      }
      else
      {
        printf("\n Already at minimum!\n\n");
      }
    }

    /* Ответ по-умолчанию - команда не найдена */
    else
    {
      printf("Unrecognized command. Try typing ""/help"" for command list\n");
    }
  }
}
