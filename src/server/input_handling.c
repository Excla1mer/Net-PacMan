/*
 * Поток обработки терминальных команд на стороне сервера.
 *
 * Любые команды непосредственно для сервера стоит вносить здесь.
 * На текущий момент, есть только команда завершения работы программы.
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

void *input_handling()
{
/*##############################################################################
 * Объявление и определение/подготовка данных
 *##############################################################################
 */

  const char *section = "INPUT HANDLING";

  char input[50];
  /*int count;*/

  printf("[%s] - Started\n", section);
  printf("-------------Type /shut to exit the program-------------\n");
  /*count = 0;*/

/*##############################################################################
 * Обработка команд.
 *##############################################################################
 */

  /* Бесконечный цикл. Прерывается только командой /shut */
  while (1)
  {
    fgets(input, 50, stdin);

    /* /shut - завершение работы сервера. Это завершит всю программу. */
    if (strcmp(input, "/shut\n") == 0)
    {
      printf("[%s] - Initializing shutdown\n", section);
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
        printf("[%s] - Input handling mutex destroyed\n", section);
      }
      else
      {
        printf("[%s] - Unable to destroy input handling mutex."\
               "Proceeding anyway...\n", section);
      }

      printf("[%s] - Exiting thread...\n", section);
      /* Вернуть 0. Теперь контроль перейдёт в "main"*/
      pthread_exit((void *)0);
    }
  }
}
