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
    int return_value = 0;

    check = alarm__send_signal(connection, SEND_MESSAGE);
    if (-1 == check)
    {
        printf("Error: send_signal()\n");
        return_value = check;
        goto finally;
    }

    check = zmq_send(connection.socket, &message, sizeof(message), ZMQ_DONTWAIT);
    if (-1 == check)
    {
        printf("Error: %s(): %s\n", __func__, zmq_strerror(errno));
        return_value = check;
    }

 finally:

    return return_value;
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
        printf("Error: %s(): arg[2] cannot be equal NULL\n", __func__);
        return_value = -1;
        goto finally;
    }

    check = alarm__create_connection(&spec_connection, SUBSCRIBER, 1);
    if (-1 == check)
    {
        printf("Error: %s(): create_spec_connection()\n", __func__);
        return_value = check;
        goto finally;
    }

    check = alarm__send_signal(connection, GET_ALL);
    if (-1 == check)
    {
        printf("Error: %s(): send_signal()\n", __func__);
        return_value = check;
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
    if (0 > alarm__destroy_connection(&spec_connection))
    {
        printf("Error: %s(): destroy_connection\n", __func__);
    }
    return return_value;
}

int alarm__send_all_message(
    int quantity_message)
{
    int check = 0;
    int return_value = 0;
    struct Note *note_array_for_send = NULL;
    struct Connection spec_connection = {NULL, NULL};

    check = alarm__create_connection(&spec_connection, PUBLISHER, 1);
    if (0 > check)
    {
        printf("Error: create_connection()\n");
        return_value = check;
        goto finally;
    }

    note_array_for_send = calloc((size_t)quantity_message, sizeof(*note_array_for_send));
    if (NULL == note_array_for_send)
    {
        printf("Error: %s(): calloc()\n", __func__);
        return_value = -4;
        goto finally;
    }

    quantity_message = alarm__read_from_file(note_array_for_send, quantity_message);
    if (0 > quantity_message)
    {
        printf("Error: %s(): read_from_file()\n", __func__);
        return_value = quantity_message;
        goto finally;
    }

    for (int i = 0; i < quantity_message; i++)
    {
        check = alarm__send_message(spec_connection, note_array_for_send[i].message);
        if (0 > check)
        {
            printf("Error: %s(): send_message(): %s\n", __func__, zmq_strerror(errno));
            return_value = check;
            goto finally;
        }
    }
    check = alarm__send_signal(spec_connection, DEFAULT);
    if (0 > check)
    {
        printf("Error: %s(): send_signal(): %s\n", __func__, zmq_strerror(errno));
        return_value = check;
        goto finally;
    }

 finally:

    free(note_array_for_send);
    if (0 > alarm__destroy_connection(&spec_connection))
    {
        printf("Error: destroy_connection\n");
    }

    return return_value;
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
        printf("Error: %s(): arg[2] cannot be equal NULL\n", __func__);
        check = -1;
        goto finally;
    }

    check = alarm__create_connection(&spec_connection, SUBSCRIBER, 1);
    if (0 > check)
    {
        printf("Error: %s(): create_connection()\n", __func__);
        return_value = check;
        goto finally;
    }

    check = alarm__send_signal(connection, GET_FILTER);
    if (0 > check)
    {
        printf("Error: %s(): send_signal()\n", __func__);
        return_value = check;
        goto finally;
    }

    check = alarm__send_message(connection, filter);
    if (0 > check)
    {
        printf("Error: %s(): send_message()\n", __func__);
        return_value = check;
        goto finally;
    }

    for (int i = 0; i < quantity; i++)
    {
        check = alarm__recv_signal(spec_connection, &sig);
        if (0 > check)
        {
            printf("Error: %s(): recv_signal()\n", __func__);
            return_value = check;
            goto finally;
        }

        if (SEND_MESSAGE == sig)
        {
            check = alarm__recv_message(spec_connection, &message_array[i]);
            if (0 > check)
            {
                printf("Error: %s(): recv_message()\n", __func__);
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
    if (0 > alarm__destroy_connection(&spec_connection))
    {
        printf("Error: %s(): destroy_connection\n", __func__);
    }

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
    if (0 > check)
    {
        printf("Error: %s(): create_connection()\n", __func__);
        return_value = check;
        goto finally;
    }

    note_array = calloc((size_t)quantity, sizeof(*note_array));
    if (NULL == note_array)
    {
        printf("Error: %s(): allocation memory error\n", __func__);
        return_value = -1;
        goto finally;
    }

    quantity = alarm__read_from_file(note_array, quantity);
    if (quantity <= 0)
    {
        printf("Error: %s(): the file was not read or empty\n", __func__);
        return_value = -1;
        goto finally;
    }

    for (int i = 0; i < quantity; i++)
    {
        if (0 != alarm__message_compare(note_array[i].message, filter))
        {
            check = alarm__send_message(spec_connection, note_array[i].message);
            if (0 > check)
            {
                printf("Error: %s(): zmq_send: %s\n", __func__, zmq_strerror(errno));
                return_value = check;
                goto finally;
            }
        }
    }

 finally:
    check = alarm__send_signal(spec_connection, DEFAULT);
    if(0 > check)
    {
        printf("Error: %s(): the last signal was not send\n", __func__);
        return_value = check;
    }

    free(note_array);
    if (0 > alarm__destroy_connection(&spec_connection))
    {
        printf("Error: %s(): destroy_connection\n", __func__);
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
    if (0 > check)
    {
        return_value = check;
        printf("Error: %s(): connection was not created\n", __func__);
        goto finally;
    }

    check = zmq_send(spec_connection.socket, &quantity, sizeof(quantity), 0);
    if (0 > check)
    {
        return_value = check;
        printf("Error: %s(): quantity was not sent\n", __func__);
        goto finally;
    }

 finally:

    if (0 > alarm__destroy_connection(&spec_connection))
    {
        printf("Error: %s(): destroy_connection()\n", __func__);
    }

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
    if (0 > check)
    {
        return_value = check;
        printf("Error: %s(): destroy_connection()\n", __func__);
        goto finally;
    }

    check = alarm__send_signal(connection, GET_MAX_NUMB);
    if (0 > check)
    {
        return_value = check;
        printf("Error: %s(): send_signal()\n", __func__);
        goto finally;
    }

    check = zmq_recv(spec_connection.socket, &quantity, sizeof(int), 0);
    if (0 > check)
    {
        return_value = check;
        printf("Error: %s(): zmq_recv(): %s()\n", __func__, zmq_strerror(errno));
        goto finally;
    }

    return_value = quantity;

 finally:

    if (0 > alarm__destroy_connection(&spec_connection))
    {
        printf("Error: %s(): destroy_connection()\n", __func__);
    }

    return return_value;
}

int alarm__delete_all_messages(
    struct Connection connection)
{
    return alarm__send_signal(connection, DELETE_ALL);
}
