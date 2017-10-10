#include "micro_driver.h"

#include <arpa/inet.h>  // inet_addr
#include <ctype.h>      // isascii
#include <errno.h>      // errno
#include <stdio.h>      // printf
#include <stdlib.h>     // malloc
#include <string.h>     // memcpy
#include <sys/socket.h> // socket
#include <unistd.h>     // close

struct mongo_client_t_private {
	int socket_fd;
};

mongo_client_t* mongo_connect (char* ip, int port) {
  // create a socket to a mongod instance on localhost port 27017
  struct sockaddr_in addr = {0};
  addr.sin_family = AF_INET;
  // now that user is supplying ip, check for error
  int ip_as_int = inet_addr(ip);
  if (ip_as_int == -1) return NULL;
  addr.sin_addr.s_addr = ip_as_int;
  addr.sin_port = htons(port);

  int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd == -1) return NULL;

  // connect to the remote address, associating this socket
  int ret = connect (
    socket_fd, (struct sockaddr*)&addr, sizeof (struct sockaddr));

  if (ret != 0) return NULL;

  // create our client struct and return
  mongo_client_t* client = 
    (mongo_client_t*)malloc(sizeof (mongo_client_t));
  client->socket_fd = socket_fd;

  return client;
}

void mongo_disconnect (mongo_client_t* client) {
  close (client->socket_fd);
  free (client);
}

int mongo_send_command (mongo_client_t* client,
                        char* command,
                        int command_size,
                        char* reply,
                        int reply_size,
                        int* num_recv) {

  static char op_msg_header[] = {
    0x00, 0x00, 0x00, 0x00, // total message size, including this
    0x00, 0x00, 0x00, 0x00, // requestID (can be 0)
    0x00, 0x00, 0x00, 0x00, // responseTo (unused for sending)
    0xDD, 0x07, 0x00, 0x00, // opCode = 2013 = 0x7DD for OP_MSG
    0x00, 0x00, 0x00, 0x00, // message flags (not needed)
    0x00                    // only data section, type 0
  };
  // allocate enough memory for the header and command
  int total_bytes = sizeof(op_msg_header) + command_size;
  char *op_msg = (char*)malloc(total_bytes);
  memcpy (op_msg, op_msg_header, sizeof(op_msg_header));
  memcpy (op_msg + sizeof(op_msg_header), command, command_size);

  // set the length of the total op_msg
  int *as_int = (int*)op_msg;
  *as_int = total_bytes; // hope you're on a little-endian machine :)

  int ret = send (client->socket_fd, op_msg, total_bytes, 0);
  if (ret == -1) return 0;

  *num_recv = recv (client->socket_fd, reply, reply_size, 0);
  if (*num_recv == -1) return 0;

  return 1;
}
