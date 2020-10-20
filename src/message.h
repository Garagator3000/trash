#include <zmq.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h> //временно
#include <pthread.h>

char const PATH_TO_FILE[] = "messages.txt";
char const PATH_TO_SOCKET[] = "/tmp/alarm_manager/";
//char const ADDR_OF_SOCKET[] = "ipc:///tmp/alarm_manager/sock";
//char const SPEC_ADDR_OF_SOCKET[] = "ipc:///tmp/alarm_manager/sock0";

typedef enum Message_type
{
    ALARM = 0,
    NORMALIZE = 1,
} Message_type;

typedef enum Message_priority
{
    VERY_IMPORTANT = 0,
    IMPORTANT = 1,
    USALLY = 2,
    NOT_IMPORTANT = 3,
} Message_priority;

typedef struct Message
{
    char modul[64];
    Message_type type;
    Message_priority priority;
    char message_text[64];
} Message;

typedef struct Note
{
    struct tm registration_time;
    struct Message message;
} Note;

typedef struct Connection
{
    void *context;
    void *socket;
} Connection;

typedef enum Message_signal
{
    SEND_MESSAGE = 0,
    DELETE_ALL = 1,
    GET_ALL = 2,
    GET_FILTER = 3,
    DEFAULT = 16,
} Message_signal;

int write_to_file(Note note);
Connection create_connection(int role, int mode, int spec);
int read_from_file(Note *note_array, int size);
int delete_all_messages();
int send_signal(Connection connection, Message_signal sig);
int recv_signal(Connection connection, Message_signal *sig);
int send_message(Connection connection, Message message);
int recv_meassage(Connection connection, Message *message);
int recv_all_message(Connection connection, Message *message_array, int quantity);
int send_all_message(int max_quantity_message);
Connection destroy_connection(Connection connection);
