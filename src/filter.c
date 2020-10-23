#include "message.h"

int main(int argc, char *argv[])
{
    int return_value = 0;

    struct Connection connection = {NULL, NULL};
    return_value = create_connection(&connection, PUBLISHER, 1, 0);
    if (-1 == return_value)
    {
        goto finally;
    }

    struct Message filter;
    strcpy(filter.modul, "");
    filter.priority = IMPORTANT;
    filter.type = NO_TYPE;
    strcpy(filter.message_text, "");

    for(int i = 0; i < 1000000; i++);
    int quantity = recv_quantity(connection);
    printf("%d\n", quantity);
    struct Message *message_array = (Message*)calloc(quantity, sizeof(Message));

    //return_value = recv_by_filter(connection, filter, message_array, quantity);

    return_value = recv_all_message(connection, message_array, quantity);
    if (-1 == return_value)
    {
        printf("Error: recv_by_filter()\n");
        goto finally;
    }

    for (int i = 0; i < return_value; i++)
    {
        printf("%d\n%s\n%d\n%d\n%s\n\n", i, message_array[i].modul, message_array[i].type,
            message_array[i].priority, message_array[i].message_text);
    }

 finally:
    free(message_array);
    return return_value;
}
