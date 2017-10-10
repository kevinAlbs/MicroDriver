// compile with gcc -o connect connect.c
#include <arpa/inet.h>  // inet_addr
#include <errno.h>      // errno
#include <stdio.h>      // printf
#include <sys/socket.h> // socket
#include <unistd.h>     // close

int main () {   
  // create a socket to a mongod instance on localhost port 27017
  struct sockaddr_in addr = {0};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  addr.sin_port = htons(27017);

  int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd == -1) {
    printf("Could not create socket, errno=%d\n", errno);
    return -1;
  }

  // connect to the remote address, associating this socket
  int ret = connect (
    socket_fd, (struct sockaddr*)&addr, sizeof (struct sockaddr));

  if (ret != 0) {
    printf("Could not connect, errno=%d\n", errno);
    return 1;
  }

  printf ("Connected to mongod at 127.0.0.1 port 27017\n");
  close (socket_fd); 
}
