#include <zmq.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h> //временно

char const PATH_TO_FILE[] = "messages.txt";
char const PATH_TO_SOCKET[] = "/tmp/alarm_manager/";

typedef enum Role
{
    PUBLISHER = ZMQ_PUB,
    SUBSCRIBER = ZMQ_SUB,
} Role;

typedef enum Message_type
{
    NO_TYPE = 0,
    ALARM = 1,
    NORMALIZE = 2,
} Message_type;

typedef enum Message_priority
{
    NO_PRIORITY = 0,
    VERY_IMPORTANT = 1,
    IMPORTANT = 2,
    USALLY = 3,
    NOT_IMPORTANT = 4,
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

int create_connection(
    struct Connection *connection,
    enum Role role,
    int mode,
    int spec);

int destroy_connection(
    struct Connection *connection);

int write_to_file(
    struct Note note);

int read_from_file(
    struct Note *note_array,
    int size);

int send_signal(
    struct Connection connection,
    enum Message_signal sig);

int recv_signal(
    struct Connection connection,
    enum Message_signal *sig);

int send_message(
    struct Connection connection,
    struct Message message);

int recv_meassage(
    struct Connection connection,
    struct Message *message);

int recv_all_message(
    struct Connection connection,
    struct Message *message_array,
    int quantity);

int send_all_message(
    int max_quantity_message);

int delete_all_messages(void);

int recv_by_filter(
    struct Connection connection,
    struct Message filter,
    struct Message *message_array);

int send_by_filter(
    struct Message filter,
    int quantity);

int message_compare(
    struct Message message,
    struct Message filter);
