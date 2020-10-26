#include <unistd.h>
#include "message.h"

int main (int argc, char *argv[])
{
    int return_value = 0;
    int quantity = atoi(argv[1]);
    struct Connection connection = {NULL, NULL};
    return_value = create_connection(&connection, PUBLISHER, 1, 0);
    if (-1 == return_value)
    {
        goto finally;
    }

    struct Message message;
    strcpy(message.modul, argv[0]);
    message.type = ALARM;
    message.priority = IMPORTANT;
    strcpy(message.message_text, "all fine");

    for(int i = 0; i < 1000000; i++);

    for (int i = 0; i < quantity; i++)
    {
        return_value = send_message(connection, message);
        if (-1 == return_value)
        {
            goto finally;
        }
    }

finally:

    return return_value;
}
