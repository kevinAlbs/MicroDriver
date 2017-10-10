// compile with gcc -o poc poc.c
#include <arpa/inet.h>  // inet_addr
#include <ctype.h>      // isascii
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

  if (ret == -1) {
    printf("Could not connect, errno=%d\n", errno);
    return 1;
  }

  printf ("Connected to mongod at 127.0.0.1 port 27017\n");

  char op_msg[] = {
    0x5D, 0x00, 0x00, 0x00, // total message size, including this
    0x00, 0x00, 0x00, 0x00, // requestID
    0x00, 0x00, 0x00, 0x00, // responseTo (unused for sending)
    0xDD, 0x07, 0x00, 0x00, // opCode = 2013 = 0x7DD for OP_MSG
    0x00, 0x00, 0x00, 0x00, // message flags (not needed)
    0x00,                   // only data section, type 0
    // begin bson command document
    // {insert: "test_coll", $db: "db", documents: [{_id:1}]}
    0x48, 0x00, 0x00, 0x00, // total bson obj length

    // insert: "test_coll" key/value
    0x02, 'i','n','s','e','r','t','\0',
    0x0A, 0x00, 0x00, 0x00, // "test_coll" length
    't','e','s','t','_','c','o','l','l','\0',

    // $db: "db"
    0x02, '$','d','b','\0',
    0x03, 0x00, 0x00, 0x00,
    'd','b','\0',

    // documents: [{_id:1}]
    0x04, 'd','o','c','u','m','e','n','t','s','\0',
    0x16, 0x00, 0x00, 0x00, // start of {0: {_id: 1}} 
    0x03, '0', '\0', // key "0"
    0x0E, 0x00, 0x00, 0x00, // start of {_id: 1}
    0x10, '_','i','d','\0', 0x01, 0x00, 0x00, 0x00,
    0x00,                   // end of {_id: 1}
    0x00,                   // end of {0: {_id: 1}}
    0x00                    // end of command document
  };

  ret = send (socket_fd, op_msg, sizeof(op_msg), 0);
  if (ret == -1) {
    printf("Could not send, errno=%d\n", errno);
    return 1;
  }

  printf("Message sent\n");

  char reply_buf[512];
  int num_recv = recv(socket_fd, reply_buf, 512, 0);
  if (num_recv == -1) {
    printf("Could not send, errno=%d\n", errno);
    return 1;
  }

  printf("Message received\n");

  int i;
  for (i = 0; i < num_recv; i++) {
    if (isascii(reply_buf[i])) printf("%c", reply_buf[i]);
    else printf(".");
  }
  printf("\n");

  close (socket_fd); 
}
