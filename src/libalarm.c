#include "message.h"

/*role: PUBLISHER/SUBSCRIBER, mode: 0/1(bind/connect), spec: always 0*/
int create_connection(
    struct Connection *connection,
    enum Role role,
    int mode,
    int spec)
{
    char *addr_of_socket;
    int check = 0;

    if (role != SUBSCRIBER && role != PUBLISHER)
    {
        printf("Error: invalid value \"role\"\n");
        check = -1;
        goto finally;
    }

    if (0 == spec)
    {
        addr_of_socket = "ipc:///tmp/alarm_manager/sock0";
    }
    else
    {
        addr_of_socket = "ipc:///tmp/alarm_manager/sock1";
    }

    void *context = zmq_ctx_new ();
    if (NULL == context)
    {
        printf("Error: zmq_ctx_new(): %s", zmq_strerror(errno));
        check = -1;
        goto finally;
    }
    void *socket = zmq_socket (context, role);
    if (NULL == socket)
    {
        printf ("Error: zmq_soocket(): %s\n", zmq_strerror(errno));
        check = -1;
        goto finally;
    }

    if (0 == mode)
    {
        check = zmq_bind(socket, addr_of_socket);
        if (-1 == check)
        {
            printf("Error: zmq_bind(): %s\n", zmq_strerror(errno));
            goto finally;
        }

        check = zmq_setsockopt(socket, ZMQ_SUBSCRIBE, 0, 0);
        if (-1 == check)
        {
            printf("Error: zmq_setsockopt() %s\n", zmq_strerror(errno));
            goto finally;
        }
        connection->context = context;
        connection->socket = socket;
    }
    else if (1 == mode)
    {
        check = zmq_connect(socket, addr_of_socket);
        if (-1 == check)
        {
            printf("Error: zmq_connect(): %s\n", zmq_strerror(errno));
            goto finally;
        }
        connection->context = context;
        connection->socket = socket;
    }
    else
    {
        printf("Error: invalid arguments\n");
        check = -1;
        goto finally;
    }

 finally:
    return check;
}

/*not sure about how this function works*/
int destroy_connection(
    struct Connection *connection)
{
    zmq_close(connection->socket);
    zmq_ctx_destroy(connection->context);
    connection->socket = NULL;
    connection->context = NULL;
    return 0;
}

/*Write to "messages.txt" a note.
/*str_time[32] - string to form a normal representation of time:
"date: mm/dd/yy time: hh:mi:ss".*/
int write_to_file(
    struct Note note)
{
    int check = 0;
    char str_time[32];

    FILE *file = fopen(PATH_TO_FILE, "a");
    if (NULL == file)
    {
        check = -1;
        goto finally;
    }

    if (0 == note.registration_time.tm_year)
    {
        check = -2;
        goto finally;
    }

    strftime(str_time, 32, "date: %x time: %X", &note.registration_time);
    fprintf(file, "Registration time: %s\n", str_time);

    fprintf(file, "Modul: %s\n", note.message.modul);

    switch (note.message.type)
    {
        case 1:
            fprintf(file, "Type: ALARM\n");
            break;
        case 2:
            fprintf(file, "Type: normalize\n");
            break;
        default:
            return 2;
            break;
    }

    switch (note.message.priority)
    {
        case 1:
            fprintf(file, "Priority: very important\n");
            break;
        case 2:
            fprintf(file, "Priority: important\n");
            break;
        case 3:
            fprintf(file, "Priority: usally\n");
            break;
        case 4:
            fprintf(file, "Priority: not important\n");
            break;
        default:
            return 3; //not valid value priority
            break;
    }

    fprintf(file, "%s", note.message.message_text);
    fprintf(file, "\n\n");

 finally:
    if (NULL != file)
    {
        fclose(file);
    }
    return check;
}

int read_from_file(
    struct Note *note_array,
    int size)
{
    int check = 0;

    FILE *file = fopen(PATH_TO_FILE, "r");
    if (NULL == file)
    {
        check = -1;
        goto finally;
    }

    char buffer[80];    //buffer for read line
    char buf[8];        //buffer for value

    for (int i = 0; i < size; i++)
    {
        fgets(buffer, 80, file);
        sscanf(buffer, "Registration time: date: %d/%d/%d time: %d:%d:%d",
                    &note_array[i].registration_time.tm_mon,
                    &note_array[i].registration_time.tm_mday,
                    &note_array[i].registration_time.tm_year,
                    &note_array[i].registration_time.tm_hour,
                    &note_array[i].registration_time.tm_min,
                    &note_array[i].registration_time.tm_sec);
            note_array[i].registration_time.tm_mon -= 1;

        fgets(buffer, 80, file);
        sscanf(buffer, "Modul: %s", note_array[i].message.modul);
        fgets(buffer, 80, file);
        sscanf(buffer, "Type: %s", buf);
        if(strncmp("ALARM", buf, 3) == 0)
        {
            note_array[i].message.type = 1;
        }
        else
        {
            note_array[i].message.type = 2;
        }
        fgets(buffer, 80, file);
        sscanf(buffer, "Priority: %s", buf);
        if (strncmp("very important", buf, 3) == 0)
        {
            note_array[i].message.priority = 1;
        }
        else if (strncmp("important", buf, 3) == 0)
        {
            note_array[i].message.priority = 2;
        }
        else if (strncmp("usally", buf, 3) == 0)
        {
            note_array[i].message.priority = 3;
        }
        else
        {
            note_array[i].message.priority = 4;
        }
        fgets(buffer, 80, file);
        strncpy(note_array[i].message.message_text, buffer, strlen(buffer) - 1);

        fgets(buffer, 2, file);
        if (strncmp("\n\n", buffer, 2) == 0)
        {
            continue;
        }
        if (feof(file))
        {
            break;
        }
    }

 finally:
    if (NULL != file)
    {
        fclose(file);
    }
    return 0;
}

int send_signal(
    struct Connection connection,
    enum Message_signal sig)
{
    int check = 0;
    check = zmq_send(connection.socket, &sig, sizeof(Message_signal), ZMQ_DONTWAIT);
    return check;
}

int recv_signal(
    struct Connection connection,
    enum Message_signal *sig)
{
    int check = 0;
    check = zmq_recv(connection.socket, sig, sizeof(Message_signal), 0);
    return check;
}

int send_message(
    struct Connection connection,
    struct Message message)
{
    int check = 0;
    check = send_signal(connection, SEND_MESSAGE);
    if (-1 == check)
    {
        printf("Error: send_signal()\n");
        goto finally;
    }
    check = zmq_send(connection.socket, &message, sizeof(Message), ZMQ_DONTWAIT);

 finally:
    return check;
}

int recv_meassage(
    struct Connection connection,
    struct Message *message)
{
    int check = 0;
    check = zmq_recv(connection.socket, message, sizeof(Message), 0);
    return check;
}

int recv_all_message(
    struct Connection connection,
    struct Message *message_array,
    int quantity)
{
    int check = 0;
    int return_value = 0;
    enum Message_signal sig = DEFAULT;

    struct Connection spec_connection = {NULL, NULL};
    check = create_connection(&spec_connection, SUBSCRIBER, 0, 1);
    if (-1 == check)
    {
        printf("Error: create_spec_connection()\n");
        goto finally;
    }

    for(int i = 0; i < 1000000; i++);

    check = send_signal(connection, GET_ALL);
    if (-1 == check)
    {
        printf("Error: send_signal()\n");
        goto finally;
    }

    for(int i = 0; i < quantity; i++)
    {
        recv_signal(spec_connection, &sig);
        if (SEND_MESSAGE == sig)
        {
            check = recv_meassage(spec_connection, &message_array[i]);
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

int send_all_message(
    int max_quantity_message)
{
    int check = 0;
    struct Connection spec_connection = {NULL, NULL};

    check = create_connection(&spec_connection, PUBLISHER, 1, 1);
    if (-1 == check)
    {
        printf("Error: create_connection()\n");
        goto finally;
    }

    for(int i = 0; i < 1000000; i++);

    struct Note *note_array_for_send = (Note*)calloc(max_quantity_message, sizeof(Note));

    check = read_from_file(note_array_for_send, max_quantity_message);
    if(-1 == check)
    {
        printf("Error: read_from_file()\n");
        goto finally;
    }

    for (int i = 0; i < max_quantity_message; i++)
    {
        check = send_message(spec_connection, note_array_for_send[i].message);
        if (-1 == check)
        {
            printf("Error: zmq_send(): %s\n", zmq_strerror(errno));
            goto finally;
        }
    }

 finally:
    free(note_array_for_send);
    destroy_connection(&spec_connection);
    return check;
}

/*delete only your messages*/
int delete_all_messages(void)
{
    int check = 0;
    FILE *file = fopen(PATH_TO_FILE, "w");
    if (NULL == file)
    {
        check = -1;
        goto finally;
    }

    fclose(file);

 finally:
    return check;
}

int recv_by_filter(
    struct Connection connection,
    struct Message filter,
    struct Message *message_array,
    int quantity)
{
    int check = 0;
    int return_value = 0;
    enum Message_signal sig = DEFAULT;
    struct Connection spec_connection = {NULL, NULL};
    check = create_connection(&spec_connection, SUBSCRIBER, 0, 1);
    if (-1 == check)
    {
        printf("Error: create_connection()\n");
        goto finally;
    }

    for(int i = 0; i < 100000000; i++);

    check = send_signal(connection, GET_FILTER);
    if (-1 == check)
    {
        printf("Error: send_signal()\n");
        goto finally;
    }

    check = send_message(connection, filter);
    if (-1 == check)
    {
        printf("Error: send_message()\n");
        goto finally;
    }

    for(int i = 0; i < quantity; i++)
    {
        recv_signal(spec_connection, &sig);
        if (SEND_MESSAGE == sig)
        {
            check = recv_meassage(spec_connection, &message_array[i]);
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

int send_by_filter(
    struct Message filter,
    int quantity)
{
    int check = 0;
    struct Connection spec_connection = {NULL, NULL};
    check = create_connection(&spec_connection, PUBLISHER, 1, 1);
    if (-1 == check)
    {
        printf("Error: create_connection()\n");
        goto finally;
    }

    struct Note *note_array = (Note*)calloc(quantity, sizeof(Note));

    for(int i = 0; i < 1000000; i++);

    read_from_file(note_array, quantity);
    for(int i = 0; i < quantity; i++)
    {
        if(0 != message_compare(note_array[i].message, filter))
        {
            check = send_message(spec_connection, note_array[i].message);
            if (-1 == check)
            {
                printf("Error: zmq_send: %s\n", zmq_strerror(errno));
                goto finally;
            }
        }
    }
    send_signal(spec_connection, DEFAULT);

 finally:
    free(note_array);
    destroy_connection(&spec_connection);
    return 0;
}

/*zero - not equal, higher than zero - equal*/
int message_compare(
    struct Message message,
    struct Message filter)
{
    int return_value = 0;
    int likeness = 0;
    int fields = 0;
    if (0 != strcmp(filter.modul, ""))
    {
        fields = fields + 1;
        if (0 == strcmp(message.modul, filter.modul))
        {
            likeness = likeness + 1;
        }
    }
    if (0 != filter.type)
    {
        fields = fields + 2;
        if(message.type == filter.type)
        {
            likeness = likeness + 2;
        }
    }
    if (0 != filter.priority)
    {
        fields = fields + 3;
        if(message.priority == filter.priority)
        {
            likeness = likeness + 3;
        }
    }
    if (fields == likeness)
    {
        return_value = likeness;
    }
    return return_value;
}

int send_quantity(
    int quantity)
{
    int check = 0;
    struct Connection spec_connection = {NULL, NULL};
    check = create_connection(&spec_connection, PUBLISHER, 1, 1);
    if(-1 == check)
    {
        goto finally;
    }
    for(int i = 0; i < 1000000; i++);
    check = zmq_send(spec_connection.socket, &quantity, sizeof(int), 0);
 finally:
    destroy_connection(&spec_connection);
    return check;
}

int recv_quantity(
    struct Connection connection)
{
    int check = 0;
    struct Connection spec_connection = {NULL, NULL};
    check = create_connection(&spec_connection, SUBSCRIBER, 0, 1);
    if (-1 == check)
    {
        goto finally;
    }
    check = send_signal(connection, GET_MAX_NUMB);
    for(int i = 0; i < 1000000; i++);
    zmq_recv(spec_connection.socket, &check, sizeof(int), 0);

 finally:
    destroy_connection(&spec_connection);
    return check;
}
