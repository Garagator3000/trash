#include "alarm.h"

int main(int argc, char *argv[])
{
    int return_value = 0;
    int check = 0;
    int quantity = 0;
    int mode = 0;
    struct Connection connection = {NULL, NULL};
    struct Message *message_array = NULL;
    struct Message filter = {0};

    if (2 != argc)
    {
        printf("Enter: \"./filter (mode)\"\n");
        return_value = -1;
        goto finally;
    }
    mode = atoi(argv[1]);

    check = alarm__create_connection(&connection, PUBLISHER, 0);
    if (-1 == check)
    {
        printf("Error: %s\n", zmq_strerror(errno));
        return_value = check;
        goto finally;
    }

    /*----FILTER----*/
    strcpy(filter.module, "");
    filter.type = NO_TYPE;
    filter.priority = IMPORTANT;
    strcpy(filter.message_text, "");
    /*----FILTER----*/

    quantity = alarm__recv_quantity(connection);
    if (-1 == quantity)
    {
        return_value = -2;
        goto finally;
    }
    printf("max %d messages\n\n", quantity);

    message_array = calloc((size_t)quantity, sizeof(*message_array));
    if (NULL == message_array)
    {
        return_value = -4;
        printf("Error: allocation memory error\n");
        goto finally;
    }

    if (0 == mode)
    {
        quantity = alarm__recv_by_filter(connection, filter, message_array, quantity);
    }
    else
    {
        quantity = alarm__recv_all_message(connection, message_array, quantity);
    }

    if (quantity < 0)
    {
        printf("Error: recv*()\n");
        return_value = -3;
        goto finally;
    }

    printf("Received %d messages\n\n", quantity);

    for (int i = 0; i < quantity; i++)
    {
        printf("Message #%d\n%s\n%d\n%d\n%s\n\n", i + 1,
                message_array[i].module,
                message_array[i].type,
                message_array[i].priority,
                message_array[i].message_text);
    }

    printf("final!\n");

 finally:

    free(message_array);
    if (NULL != connection.context)
    {
        check = alarm__destroy_connection(&connection);
        if(-1 == check)
        {
            printf("Error: destroy_connection\n");
        }
    }


    return return_value;
}
