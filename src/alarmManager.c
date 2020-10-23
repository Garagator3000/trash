#include <sys/stat.h>
#include "message.h"
#include <errno.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    int return_value = 0;

    struct Connection connection = {NULL, NULL};
    return_value = create_connection(&connection, SUBSCRIBER, 0, 0);
    if (-1 == return_value)
    {
        printf("Error: create_connection()\n");
        goto finally;
    }

    if (2 != argc)
    {
        printf("Enter: ./alarmManager (max_quantity_message)\n");
        return_value = -2;
        goto finally;
    }

    int max_quantity_message = atoi(argv[1]);
    struct stat st  = {0};      /*to check for the existence of a directory*/
    struct Note note;           /*buffer for recv and writing to file*/
    time_t timer = 0;           /*buffer for get local time*/
    struct Note *note_array;    /*buffer array for reading and writing*/
    note_array = (Note*)calloc(max_quantity_message, sizeof(Note));
    enum Message_signal sig = DEFAULT;

    /*Creating a directory for storing a sockets*/
    if (-1 == stat (PATH_TO_SOCKET, &st))
    {
        if (-1 == mkdir(PATH_TO_SOCKET, 0777))
        {
            printf("Error: socket directory not created\n");
            return_value = -3;
            goto finally;
        }
    }

    return_value = read_from_file(note_array, max_quantity_message);
    delete_all_messages();
    for(int i = 0; i < max_quantity_message; i++)
    {
        write_to_file(note_array[i]);
    }

    /*Infinite loop for listening to a socket*/
    for (;;)
    {
        sig = DEFAULT;
        recv_signal(connection, &sig);
        switch (sig)
        {
        case SEND_MESSAGE:
            recv_meassage(connection, &note.message);
            time(&timer);
            note.registration_time = *localtime(&timer);
            read_from_file(note_array, max_quantity_message);
            delete_all_messages();
            if (-1 == write_to_file(note))
            {
                printf("Error: error writing to file\n");
                break;
            }
            for (int i = 0; i < max_quantity_message - 1; i++)
            {
                if (-2 == write_to_file(note_array[i]))
                {
                    break;
                }
            }
            break;

        case DELETE_ALL:
            delete_all_messages();
            break;

        case GET_ALL:
            send_all_message(max_quantity_message);
            break;

        case GET_FILTER:
            recv_signal(connection, &sig);
            recv_meassage(connection, &note.message);
            send_by_filter(note.message, max_quantity_message);
            break;

        case GET_MAX_NUMB:
            send_quantity(max_quantity_message);
            break;

        default:
            break;
        }
    }
finally:
    destroy_connection(&connection);
    return return_value;
}
