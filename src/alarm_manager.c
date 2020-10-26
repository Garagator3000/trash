#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include "alarm.h"

int main(int argc, char *argv[])
{
    enum Message_signal sig = DEFAULT;
    struct Connection connection = {NULL, NULL};
    struct Note *note_array = NULL;     /*buffer array for reading and writing*/
    struct stat st  = {0};              /*to check for the existence of a directory*/
    struct Note note = {0};             /*buffer for recv and writing to file*/
    time_t timer = 0;                   /*buffer for get local time*/
    int return_value = 0;
    int check = 0;
    int max_quantity_messages = 0;

    if (2 != argc)
    {
        printf("Enter: ./alarmManager (max_quantity_messages <= 10 000)\n");
        return_value = -2;
        goto finally;
    }

    if (MAX_QUANTITY_MESSAGES < (max_quantity_messages = atoi(argv[1])))
    {
        printf("Enter: ./alarmManager (max_quantity_messages <= 10 000)\n");
        return_value = -2;
        goto finally;
    }

    printf("Starting ...\n");

    note_array = calloc((size_t)max_quantity_messages, sizeof(*note_array));
    if (NULL == note_array)
    {
        return_value = -4;
        printf("Error: allocation memory error\n");
        goto finally;
    }

    /*Creating a directory for storing a sockets*/
    if ((-1 == stat (PATH_TO_SOCKET, &st)) && (-1 == mkdir(PATH_TO_SOCKET, 0777)))
    {
        printf("Error: socket directory not created\n");
        return_value = -3;
        goto finally;
    }

    check = alarm__create_connection(&connection, SUBSCRIBER, 0);
    if (-1 == check)
    {
        printf("Error: create_connection()\n");
        return_value = check;
        goto finally;
    }

    check = alarm__open_file_first_time(note_array, max_quantity_messages);
    if (-1 == return_value)
    {
        printf("Error: main(): failed the first opening the file\n");
        return_value = check;
        goto finally;
    }

    printf("app in the progress\n");

    /*Infinite loop for listening to a socket*/
    for (int i = 1;; i++)
    {
        printf("#%d -- wait signal...\n", i);
        check = alarm__recv_signal(connection, &sig);
        if (-1 == check)
        {
            printf("Error: recv_signal(): %s\n", zmq_strerror(errno));
            return_value = check;
            goto finally;
        }

        switch (sig)
        {
        case SEND_MESSAGE:
            printf("message is come in\n");
            check = alarm__recv_message(connection, &note.message);
            if (-1 == check)
            {
                printf("Error: recv_message(): %s\n", zmq_strerror(errno));
                return_value = check;
                goto finally;
            }

            time(&timer);
            note.registration_time = *localtime(&timer);

            check = alarm__read_from_file(note_array, max_quantity_messages);
            if (-1 == check)
            {
                printf("Error: read_from_file()\n");
                return_value = check;
                goto finally;
            }
            check = alarm__delete_my_messages();
            if (-1 == check)
            {
                printf("Error: delete_all_my_messages_from_file()");
                goto finally;
            }

            printf("Writing message to file...\n");
            check = alarm__write_to_file(note);
            if (-1 == check)
            {
                printf("Error: error writing to file\n");
                goto finally;
            }
            for (int k = 0; k < max_quantity_messages; k++)
            {
                check = alarm__write_to_file(note_array[k]);
                if (-2 == check)
                {
                    break;
                }
                if (-1 == check)
                {
                    return_value = check;
                    goto finally;
                }
            }

            break;

        case DELETE_ALL:
            printf("delete all...\n");
            check = alarm__delete_my_messages();
            if (-1 == check)
            {
                printf("Error: delete_all_my_message_from_file()\n");
                return_value = check;
                goto finally;
            }
            break;

        case GET_ALL:
            printf("sending all...\n");
            check = alarm__send_all_message(max_quantity_messages);
            if (-1 == check)
            {
                printf("Error: send_all_message()\n");
                return_value = check;
                goto finally;
            }
            break;

        case GET_FILTER:
            printf("sending by filter...\n");
            check = alarm__recv_signal(connection, &sig);
            if (-1 == check)
            {
                printf("Error: recv_signal()\n");
                return_value = check;
                goto finally;
            }
            check = alarm__recv_message(connection, &note.message);
            if (-1 == check)
            {
                printf("Error: recv_message()\n");
                return_value = check;
                goto finally;
            }
            check = alarm__send_by_filter(note.message, max_quantity_messages);
            if (-1 == check)
            {
                printf("Error: send_by_filter()\n");
                return_value = check;
                goto finally;
            }
            break;

        case GET_MAX_NUMB:
            printf("sending quantity\n");
            check = alarm__send_quantity(max_quantity_messages);
            if (-1 == check)
            {
                printf("Error: sending quantity()\n");
            }
            break;

        default:
            break;
        }
    }
finally:

    if (NULL != connection.context)
    {
        check = alarm__destroy_connection(&connection);
        if(-1 == check)
        {
            printf("Error: destroy_connection\n");
        }
    }
    free(note_array);

    return return_value;
}