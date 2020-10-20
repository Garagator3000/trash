#include <unistd.h>
#include "message.h"

int main (int argc, char *argv[])
{
    struct Connection connection = create_connection(ZMQ_PUB, 1, 0);
    struct Message *messages = (Message*)calloc(atoi(argv[1]), sizeof(Message));
    sleep(1);

    struct Message message;
    strcpy (message.modul, argv[0]);
    message.type = ALARM;
    message.priority = IMPORTANT;
    strcpy (message.message_text, "all cool");
    int check;
    sleep(1);
    for(int i = 0; i < atoi(argv[1]); i++)
    {
        //sleep(1);
        check = send_message(connection, message);
        if(-1 == check)
        {
            printf("Error: send_message()\n");
            goto finally;
        }
    }

    /*recv_all_message(connection, messages, atoi(argv[1]));

    for(int i = 0; i < atoi(argv[1]); i++)
    {
        printf("%s\n%d\n%d\n%s\n", messages[i].modul, messages[i].type, messages[i].priority, messages[i].message_text);
    }*/

    struct Message filter;
    strcpy(filter.modul, "deleter");
    filter.type = ALARM;
    filter.priority = 0;
    strcpy(filter.message_text, "");

    recv_by_filter(connection, filter, messages);

    for(int i = 0; i < atoi(argv[1]); i++)
    {
        printf("%s\n%d\n%d\n%s\n", messages[i].modul, messages[i].type, messages[i].priority, messages[i].message_text);
    }

finally:
    connection = destroy_connection(connection);
    return 0;
}
