#ifndef MICRO_DRIVER_H
#define MICRO_DRIVER_H

// forward declare a struct representing a client handle
typedef struct mongo_client_t_private mongo_client_t;

// connect to a single mongod server. returns null on error.
mongo_client_t* mongo_connect (char* ip, int port);

// clean up the client handle
void mongo_disconnect (mongo_client_t* client);

// send a command and get a reply, returns 0 on error
int mongo_send_command (mongo_client_t* client,
                        char* command,
                        int size,
                        char* reply,
                        int reply_size,
                        int* num_recv);

#endif