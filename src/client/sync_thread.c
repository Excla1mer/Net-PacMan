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
  int sign;
  int pt_fd;
  if((pt_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror("Socket:");
    exit(1);
  }
        
  memset(&server, 0, sizeof(server));
  server.sin_family    = AF_INET;  
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons(PORT);
  if(connect(pt_fd, (struct sockaddr *)&server, sizeof(server)) == -1)
  {
    perror("connect");
    exit(1);
  }
  printf("[sinc_thread] - connecting to server\n");

  while(1)
  {
    printf("[sinc_thread] - waiting signal\n");
    if((n = recv(pt_fd, &sign, sizeof(int), 0)) == -1)
    {
      perror("sinc_thread Recv");
      exit(1);
    }
    player_sinc.x = player1->x;
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
