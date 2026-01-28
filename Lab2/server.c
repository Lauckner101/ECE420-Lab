#include "common.h"
#include "timer.h"
#include <stdlib.h>




int main(int argc, char* argv[]){
  int N, i;
  char * server_ip;
  int server_port;

  if (argc != 4) {
    printf("Invalid arguments: server <array_size> <ip> <port>\n");
    return EXIT_FAILURE;
  }

  N = atoi(argv[1]);
  server_ip = argv[2];
  server_port = atoi(argv[3]);

  return EXIT_SUCCESS;
}