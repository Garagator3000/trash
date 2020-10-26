#include <unistd.h>
#include "alarm.h"

int main (int argc, char *argv[])
{
    int return_value = 0;
    int quantity = 0;
    struct Connection connection = {NULL, NULL};
    struct Message message = {
        "trololo",
        NORMALIZE,
        USUALLY,
        "anything"
    };

    return_value = alarm__create_connection(&connection, PUBLISHER, 0);
    if (-1 == return_value)
    {
        goto finally;
    }

    if (2 != argc)
    {
        printf("Enter: ./module (quantity_message <= 9000)\n");
        return_value = -1;
        goto finally;
    }
    if (MAX_QUANTITY_MESSAGES < (quantity = atoi(argv[1])))
    {
        printf("Enter: ./module (quantity_message <= 9000)\n");
        return_value = -1;
        goto finally;
    }

    printf("send %d messages\n", quantity);

    for (int i = 0; i < quantity; i++)
    {
        return_value = alarm__send_message(connection, message);
        if (-1 == return_value)
        {
            printf("Error: send_message()\n");
            goto finally;
        }
        printf("message #%d sent\n", i + 1);
    }

 finally:

    if (NULL != connection.context)
    {
        return_value = alarm__destroy_connection(&connection);
        if(-1 == return_value)
        {
            printf("Error: destroy_connection\n");
        }
    }

    return return_value;
}
