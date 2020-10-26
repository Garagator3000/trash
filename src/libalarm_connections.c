#include "alarm.h"

/*role: PUBLISHER/SUBSCRIBER, mode: 0/1(bind/connect), spec: always 0*/
int alarm__create_connection(
    struct Connection *connection,
    enum Role role,
    int spec)
{
    void *context = NULL;
    void *socket = NULL;
    char addr_of_socket[MAX_SIZE_SOCKET_PATH] = "ipc://";
    int check = 0;
    int hwm = HIGH_WATER_MARK;

    if (role != SUBSCRIBER && role != PUBLISHER)
    {
        printf("Error: invalid value \"role\"\n");
        check = -1;
        goto finally;
    }

    if (0 == spec)
    {
        strcat(addr_of_socket, PATH_TO_SOCKET);
        strcat(addr_of_socket, "sock0");
    }
    else
    {
        strcat(addr_of_socket, PATH_TO_SOCKET);
        strcat(addr_of_socket, "sock1");
    }

    context = zmq_ctx_new();
    if (NULL == context)
    {
        printf("Error: create_connection: zmq_ctx_new(): %s",
            zmq_strerror(errno));
        check = -1;
        goto finally;
    }

    socket = zmq_socket(context, role);
    if (NULL == socket)
    {
        printf ("Error: create_connection: zmq_socket(): %s\n",
            zmq_strerror(errno));
        check = -1;
        goto finally;
    }

    if (SUBSCRIBER == role)
    {
        check = zmq_bind(socket, addr_of_socket);
        if (-1 == check)
        {
            printf("Error: create_connection: zmq_bind(): %s\n",
                zmq_strerror(errno));
            goto finally;
        }
        check = zmq_setsockopt(socket, ZMQ_SUBSCRIBE, "", 0);
        if (-1 == check)
        {
            printf("Error: create_connection: zmq_setsockopt(): %s\n",
                zmq_strerror(errno));
            goto finally;
        }
        check = zmq_setsockopt(socket, ZMQ_RCVHWM, &hwm, sizeof(int));
        if (-1 == check)
        {
            printf("Error: create_connection: zmq_setsockopt(): %s\n",
                zmq_strerror(errno));
            goto finally;
        }
    }
    else if (PUBLISHER == role)
    {
        check = zmq_connect(socket, addr_of_socket);
        if (-1 == check)
        {
            printf("Error: create_connection: zmq_connect(): %s\n",
                zmq_strerror(errno));
            goto finally;
        }
        check = zmq_setsockopt(socket, ZMQ_SNDHWM, &hwm, sizeof(int));
        if (-1 == check)
        {
            printf("Error: create_connection: zmq_setsockopt(): %s\n",
                zmq_strerror(errno));
            goto finally;
        }
    }
    else
    {
        printf("Error: create_connection: invalid arguments\n");
        check = -1;
        goto finally;
    }

    connection->context = context;
    connection->socket = socket;

    alarm__delay(DELAY_ITER);

 finally:

    return check;
}

/*not sure about how this function works*/
int alarm__destroy_connection(
    struct Connection *connection)
{
    int return_value = 0;

    return_value = zmq_close(connection->socket);
    if (-1 == return_value)
    {
        printf("Error: destroy_connection(): zmq_close(): %s\n",
            zmq_strerror(errno));
        goto finally;
    }

    return_value = zmq_ctx_destroy(connection->context);
    if (-1 == return_value)
    {
        printf("Error: destroy_connection(): zmq_ctx_destroy(): %s\n",
            zmq_strerror(errno));
        goto finally;
    }

    connection->socket = NULL;
    connection->context = NULL;

 finally:

    return return_value;
}
