#include "alarm.h"

int alarm__send_signal(
    struct Connection connection,
    enum Message_signal sig)
{
    return zmq_send(connection.socket, &sig, sizeof(sig), ZMQ_DONTWAIT);
}

int alarm__recv_signal(
    struct Connection connection,
    enum Message_signal *sig)
{
    return zmq_recv(connection.socket, sig, sizeof(*sig), 0);
}

int alarm__send_message(
    struct Connection connection,
    struct Message message)
{
    int check = 0;

    check = alarm__send_signal(connection, SEND_MESSAGE);
    if (-1 == check)
    {
        printf("Error: send_signal()\n");
        goto finally;
    }

    check = zmq_send(connection.socket, &message, sizeof(message), ZMQ_DONTWAIT);
    if (-1 == check)
    {
        printf("Error: send_message(): %s\n", zmq_strerror(errno));
    }

 finally:

    return check;
}

int alarm__recv_message(
    struct Connection connection,
    struct Message *message)
{
    return zmq_recv(connection.socket, message, sizeof(*message), 0);
}

int alarm__recv_all_message(
    struct Connection connection,
    struct Message *message_array,
    int quantity)
{
    int check = 0;
    int return_value = 0;
    enum Message_signal sig = DEFAULT;
    struct Connection spec_connection = {NULL, NULL};

    if (NULL == message_array)
    {
        printf("Error: recv_all_message: arg[2] cannot be equal NULL\n");
        check = -1;
        goto finally;
    }

    check = alarm__create_connection(&spec_connection, SUBSCRIBER, 1);
    if (-1 == check)
    {
        printf("Error: create_spec_connection()\n");
        goto finally;
    }

    check = alarm__send_signal(connection, GET_ALL);
    if (-1 == check)
    {
        printf("Error: send_signal()\n");
        goto finally;
    }

    for(int i = 0; i < quantity; i++)
    {
        check = alarm__recv_signal(spec_connection, &sig);
        if (-1 == check)
        {
            printf("Error: recv_signal(): %s\n", zmq_strerror(errno));
            return_value = check;
            goto finally;
        }

        if (SEND_MESSAGE == sig)
        {
            check = alarm__recv_message(spec_connection, &message_array[i]);
            if (-1 == check)
            {
                printf("Error: recv_message(): %s\n", zmq_strerror(errno));
                return_value = check;
                goto finally;
            }

            return_value++;
        }
        else
        {
            goto finally;
        }
    }

 finally:

    return return_value;
}

int alarm__send_all_message(
    int quantity_message)
{
    int check = 0;
    struct Note *note_array_for_send = NULL;
    struct Connection spec_connection = {NULL, NULL};

    check = alarm__create_connection(&spec_connection, PUBLISHER, 1);
    if (-1 == check)
    {
        printf("Error: create_connection()\n");
        goto finally;
    }

    note_array_for_send = calloc((size_t)quantity_message, sizeof(*note_array_for_send));
    if (NULL == note_array_for_send)
    {
        check = -4;
        goto finally;
    }

    quantity_message = alarm__read_from_file(note_array_for_send, quantity_message);
    if (-1 == quantity_message)
    {
        printf("Error: read_from_file()\n");
        goto finally;
    }

    for (int i = 0; i < quantity_message; i++)
    {
        check = alarm__send_message(spec_connection, note_array_for_send[i].message);
        if (-1 == check)
        {
            printf("Error: send_message(): %s\n", zmq_strerror(errno));
            goto finally;
        }
    }
    check = alarm__send_signal(spec_connection, DEFAULT);
    if (-1 == check)
    {
        printf("Error: send_signal(): %s\n", zmq_strerror(errno));
        goto finally;
    }

 finally:

    free(note_array_for_send);
    if (-1 == alarm__destroy_connection(&spec_connection))
    {
        printf("Error: destroy_connection\n");
    }

    return check;
}

int alarm__recv_by_filter(
    struct Connection connection,
    struct Message filter,
    struct Message *message_array,
    int quantity)
{
    int check = 0;
    int return_value = 0;
    enum Message_signal sig = DEFAULT;
    struct Connection spec_connection = {NULL, NULL};

    if (NULL == message_array)
    {
        printf("Error: recv_all_message: arg[2] cannot be equal NULL\n");
        check = -1;
        goto finally;
    }

    check = alarm__create_connection(&spec_connection, SUBSCRIBER, 1);
    if (-1 == check)
    {
        printf("Error: create_connection()\n");
        return_value = check;
        goto finally;
    }

    check = alarm__send_signal(connection, GET_FILTER);
    if (-1 == check)
    {
        printf("Error: send_signal()\n");
        return_value = check;
        goto finally;
    }

    check = alarm__send_message(connection, filter);
    if (-1 == check)
    {
        printf("Error: send_message()\n");
        return_value = check;
        goto finally;
    }

    for (int i = 0; i < quantity; i++)
    {
        check = alarm__recv_signal(spec_connection, &sig);
        if (-1 == check)
        {
            printf("Error: recv_signal()\n");
            return_value = check;
            goto finally;
        }

        if (SEND_MESSAGE == sig)
        {
            check = alarm__recv_message(spec_connection, &message_array[i]);
            if (-1 == check)
            {
                printf("Error: recv_message()\n");
                return_value = check;
                goto finally;
            }

            return_value++;
        }
        else
        {
            goto finally;
        }
    }

 finally:

    return return_value;
}

int alarm__send_by_filter(
    struct Message filter,
    int quantity)
{
    int check = 0;
    int return_value = 0;
    struct Connection spec_connection = {NULL, NULL};
    struct Note *note_array = NULL;

    check = alarm__create_connection(&spec_connection, PUBLISHER, 1);
    if (-1 == check)
    {
        printf("Error: create_connection()\n");
        return_value = check;
        goto finally;
    }

    note_array = calloc((size_t)quantity, sizeof(*note_array));
    if (NULL == note_array)
    {
        printf("Error: send_by_filter(): allocation memory error\n");
        return_value = -1;
        goto finally;
    }

    quantity = alarm__read_from_file(note_array, quantity);
    if (quantity <= 0)
    {
        printf("Error: send_by_filter(): the file was not read\n");
        return_value = -1;
        goto finally;
    }

    for (int i = 0; i < quantity; i++)
    {
        if (0 != alarm__message_compare(note_array[i].message, filter))
        {
            check = alarm__send_message(spec_connection, note_array[i].message);
            if (-1 == check)
            {
                printf("Error: zmq_send: %s\n", zmq_strerror(errno));
                return_value = check;
                goto finally;
            }
        }
    }
    check = alarm__send_signal(spec_connection, DEFAULT);
    if(-1 == check)
    {
        printf("Error: send_by_filter(): the last signal was not send\n");
        return_value = check;
        goto finally;
    }

 finally:

    free(note_array);
    if (-1 == alarm__destroy_connection(&spec_connection))
    {
        printf("Error: destroy_connection\n");
    }

    return return_value;
}

int alarm__send_quantity(
    int quantity)
{
    struct Connection spec_connection = {NULL, NULL};
    int check = 0;
    int return_value = 0;

    check = alarm__create_connection(&spec_connection, PUBLISHER, 1);
    if (-1 == check)
    {
        return_value = -1;
        printf("Error: send_quantity(): connection was not created\n");
        goto finally;
    }

    check = zmq_send(spec_connection.socket, &quantity, sizeof(quantity), 0);
    if (-1 == check)
    {
        return_value = -1;
        printf("Error: send_quantity(): quantity was not sent\n");
        goto finally;
    }

 finally:
    alarm__destroy_connection(&spec_connection);

    return return_value;
}

int alarm__recv_quantity(
    struct Connection connection)
{
    struct Connection spec_connection = {NULL, NULL};
    int quantity = 0;
    int return_value = 0;
    int check = 0;

    check = alarm__create_connection(&spec_connection, SUBSCRIBER, 1);
    if (-1 == check)
    {
        return_value = -1;
        goto finally;
    }

    check = alarm__send_signal(connection, GET_MAX_NUMB);
    if (-1 == check)
    {
        return_value = -1;
        goto finally;
    }

    check = zmq_recv(spec_connection.socket, &quantity, sizeof(int), 0);
    if (-1 == check)
    {
        return_value = -1;
        goto finally;
    }

    return_value = quantity;

 finally:

    if (-1 == alarm__destroy_connection(&spec_connection))
    {
        printf("Error: destroy_connection\n");
    }

    return return_value;
}

int alarm__delete_all_messages(
    struct Connection connection)
{
    return alarm__send_signal(connection, DELETE_ALL);
}
