#include <unistd.h>
#include "message.h"

int main (int argc, char *argv[])
{
    struct Connection connection = create_connection(ZMQ_PUB, 1, 0);

    struct Message message;
    strcpy (message.modul, argv[0]);
    message.type = NORMALIZE;
    message.priority = USALLY;
    strcpy (message.message_text, "any text");
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



finally:
    connection = destroy_connection(connection);
    return 0;
}
