#include "message.h"

int write_to_file(Note note)
{
    FILE *file = fopen(PATH_TO_FILE, "a");
    if (NULL == file)
    {
        return 1; //file not open
    }
    char str_time[32];
    if (0 == note.registration_time.tm_year)
    {
        return 2; //nothing to write
    }
    strftime(str_time, 32, "date: %x time: %X", &note.registration_time);
    fprintf(file, "Registration time: %s\n", str_time);

    fprintf(file, "Modul: %s\n", note.message.modul);

    switch (note.message.type)
    {
        case 0:
            fprintf(file, "Type: ALARM\n");
            break;
        case 1:
            fprintf(file, "Type: normalize\n");
            break;
        default:
            return 2; //not valid value type
            break;
    }

    switch (note.message.priority)
    {
        case 0:
            fprintf(file, "Priority: very important\n");
            break;
        case 1:
            fprintf(file, "Priority: important\n");
            break;
        case 2:
            fprintf(file, "Priority: usally\n");
            break;
        case 3:
            fprintf(file, "Priority: not important\n");
            break;
        default:
            return 3; //not valid value priority
            break;
    }

    fprintf(file, "%s", note.message.message_text);
    fprintf(file, "\n\n");

    fclose(file);

    return 0;
}

/*static char *get_addr_sock()
{
    static int session = 0;
    session++;
    static int numb = 0;
    int len = 0;
    int tmp = numb;
    for (len;;len++)
    {
        if (numb % 10 != 0)
        {
            tmp = tmp / 10;
        }
        else
        {
            break;
        }
    }
    tmp = numb;
    static char *buffer;
    buffer = strcat(buffer, ADDR_OF_SOCKET);
    char *buf = malloc(len);
    for (int i = len - 1; i > -1; i--)
    {
        buf[i] = (tmp % 10) + '0';
        tmp = tmp / 10;
    }
    buffer = strcat(buffer, buf);
    if (session == 2)
    {
        numb++;
        session = 0;
    }
    return buffer;
}*/

/*role: ZMQ_PUB/SUB, mode: 0/1(bind/connect), spec: always 0*/
Connection create_connection(int role, int mode, int spec)
{
    struct Connection connection = {NULL, NULL};
    char *addr_of_socket;

    if (0 == spec)
    {
        addr_of_socket = "ipc:///tmp/alarm_manager/sock0";
    }
    else
    {
        addr_of_socket = "ipc:///tmp/alarm_manager/sock1";//get_addr_sock();
    }
    int check;

    if (role != ZMQ_SUB && role != ZMQ_PUB)
    {
        printf("Error: invalid value \"role\"\n");
        return connection;
    }

    void *context = zmq_ctx_new ();
    if (NULL == context)
    {
        printf("Error: zmq_ctx_new(): %s", zmq_strerror(errno));
        return connection;
    }
    void *socket = zmq_socket (context, role);
    if (NULL == socket)
    {
        printf ("Error: zmq_soocket(): %s\n", zmq_strerror(errno));
        return connection;
    }

    if (0 == mode)
    {
        check = zmq_bind(socket, addr_of_socket);
        if (-1 == check)
        {
            printf("Error: zmq_bind(): %s\n", zmq_strerror(errno));
            return connection;
        }
        check = zmq_setsockopt(socket, ZMQ_SUBSCRIBE, 0, 0);
        if (-1 == check)
        {
            printf("Error: zmq_setsockopt() %s\n", zmq_strerror(errno));
            return connection;
        }
        connection.context = context;
        connection.socket = socket;
    }
    else if (1 == mode)
    {
        check = zmq_connect(socket, addr_of_socket);
        if (-1 == check)
        {
            printf("Error: zmq_connect(): %s\n", zmq_strerror(errno));
            return connection;
        }
        connection.context = context;
        connection.socket = socket;
    }
    else
    {
        printf("Error: invalid arguments\n");
        return connection;
    }
    return connection;
}
/*------------------------------------------------------------------------------------*/
int read_from_file(Note *note_array, int size)
{
    FILE *file = fopen(PATH_TO_FILE, "r");
    if (NULL == file)
    {
        return 1; //file not open
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
            note_array[i].message.type = 0;
        }
        else
        {
            note_array[i].message.type = 1;
        }
        fgets(buffer, 80, file);
        sscanf(buffer, "Priority: %s", buf);
        if (strncmp("very important", buf, 3) == 0)
        {
            note_array[i].message.priority = 0;
        }
        else if (strncmp("important", buf, 3) == 0)
        {
            note_array[i].message.priority = 1;
        }
        else if (strncmp("usally", buf, 3) == 0)
        {
            note_array[i].message.priority = 2;
        }
        else
        {
            note_array[i].message.priority = 3;
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
    fclose(file);
    return 0;
}

/*delete only your messages*/
int delete_all_messages()
{
    FILE *file = fopen(PATH_TO_FILE, "w");
    if (NULL == file)
    {
        return 1;   //file not open
    }
    fclose(file);
    return 0;
}

int send_signal(Connection connection, Message_signal sig)
{
    int check = 0;
    check = zmq_send(connection.socket, &sig, sizeof(Message_signal), ZMQ_DONTWAIT);
    return check;
}

int recv_signal(Connection connection, Message_signal *sig)
{
    int check = 0;
    check = zmq_recv(connection.socket, sig, sizeof(Message_signal), 0);
    return check;
}

int send_message(Connection connection, Message message)
{
    int check;
    check = send_signal(connection, SEND_MESSAGE);
    if (-1 == check)
    {
        printf("Error: send_signal()\n");
        return check;
    }
    check = zmq_send(connection.socket, &message, sizeof(Message), ZMQ_DONTWAIT);
    return check;
}

int recv_meassage(Connection connection, Message *message)
{
    int check;
    zmq_recv(connection.socket, message, sizeof(Message), 0);
    return check;
}

/*how much is ready to get, as much memory allocate*/
int recv_all_message(Connection connection, Message *message_array, int quantity)
{
    struct Connection spec_connection = create_connection(ZMQ_SUB, 0, 1);
    if (NULL == spec_connection.context || NULL == spec_connection.socket)
    {
        printf("Error: create_spec_connection() returned \"NULL\"\n");
        return 3;
    }
    int check;
    check = send_signal(connection, GET_ALL);
    if (-1 == check)
    {
        printf("Error: send_signal()\n");
        return check;
    }
    sleep(1);
    for (int i = 0; i < quantity; i++)
    {
        recv_meassage(spec_connection, &message_array[i]);
    }
    spec_connection = destroy_connection(spec_connection);
    return 0;
}

int send_all_message(int max_quantity_message)
{
    struct Connection spec_connection = create_connection(ZMQ_PUB, 1, 1);
    if (NULL == spec_connection.context || NULL == spec_connection.socket)
    {
        printf("Error: create_connection() returned \"NULL\"\n");
        return 3;
    }
    sleep(1);
    struct Note *note_array_for_send = (Note*)calloc(max_quantity_message, sizeof(Note));
    read_from_file(note_array_for_send, max_quantity_message);
    for (int i = 0; i <= max_quantity_message; i++)
    {
        zmq_send(spec_connection.socket, &note_array_for_send[i].message, sizeof(Message), 0);
    }
    free(note_array_for_send);
    spec_connection = destroy_connection(spec_connection);
    return 0;
}

Connection destroy_connection(Connection connection)
{
    zmq_close(connection.socket);
    zmq_ctx_destroy(connection.context);
    connection.socket = NULL;
    connection.context = NULL;
    return connection;
}
