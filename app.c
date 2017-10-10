// compile with gcc -o app app.c micro_driver.c
#include <errno.h> // errno
#include <stdio.h> // printf

#include "micro_driver.h"

int main() {
  mongo_client_t* client = mongo_connect("127.0.0.1", 27017);
  if (client == NULL) {
    printf ("Could not connect, error=%d\n", errno);
    return 1;
  }

  char command[] = {
    // {insert: "test_coll", $db: "db", documents: [{x:1}]}
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

  char reply_buf[512];
  int num_recv = 0;
  int ret = mongo_send_command (client,
                                command,
                                sizeof(command),
                                reply_buf,
                                sizeof(reply_buf),
                                &num_recv);
  if (!ret) {
    printf("Could not send, errno=%d\n", errno);
    return 1;
  }

  printf ("Command sent, recieved %d bytes\n", num_recv);
  mongo_disconnect (client);
}
