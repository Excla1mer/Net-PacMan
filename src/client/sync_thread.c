/*##############################################################################
 * Синхронизирующий поток. МОДУЛЬ НАХОДИТСЯ В РАЗРАБОТКЕ, ПОКА НЕ ИСПОЛЬЗУЕТСЯ!
 *##############################################################################
 */

  /*
  * Синхронизирующий поток, работает по TCP. Получает запрос от сервера на
  * синхронизацию, в ответ отправляет структуру player_stat с координатами
  * пакмена и его набранными очками.
  */
void *sinc_thread(void  *param)
{
  struct player *player1 = (struct player*)param;
  struct player_stat player_sinc;
  struct sockaddr_in server;
  int n;
  int net_data[7];

  while(1)
  {
    printf("[sinc_thread] - waiting signal\n");
    if(recv(pt_fd, net_data, sizeof(net_data), 0) == -1)
    {
      perror("sinc_thread Recv");
      exit(1);
    }
    if(net_data[0] != SYN_REQ)
    {
      printf("[sinc_thread] - Got wrong msg: \n")
    }
     = player1->x;
    player_sinc.y = player1->y;
    player_sinc.score = player1->score;
    printf("[sinc_thread] - Got signal, sending struct: x=%f y=%f score=%d\n",
          player_sinc.x, player_sinc.y, player_sinc.score);

    if(send(pt_fd, &player_sinc, sizeof(struct player_stat), 0) == -1)
    {
      perror("sinc_thread Send");
      exit(1);
    }
  }
}
