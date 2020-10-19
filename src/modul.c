#include <unistd.h>
#include "message.h"

int main (int argc, char *argv[])
{
    struct Connection connection = create_connection(ZMQ_PUB, 1, 0);

    struct Message message;
    strcpy (message.modul, argv[0]);
    message.type = ALARM;
    message.priority = VERY_IMPORTANT;
    strcpy (message.message_text, "any text");

    sleep(1);
    int check;
    for(int i = 0; i < atoi(argv[1]); i++)
    {
        check = send_message(connection, message);
        if(-1 == check)
        {
            printf("Error: send_message()\n");
            goto finally;
        }
    }

finally:
    return 0;
}
