#include <sys/stat.h>
#include "message.h"
#include <errno.h>
#include <unistd.h>

int main(int argc, char *argv[])
{

    if(2 != argc)
    {
        printf("Enter: ./alarmManager (max_quantity_message)\n");
        goto finally;
    }
    int max_quantity_message = atoi(argv[1]) - 1;


    /*Creating a directory for storing a socket*/
    struct stat st  = {0};
    if(-1 == stat (PATH_TO_SOCKET, &st))
    {
        if(-1 == mkdir(PATH_TO_SOCKET, 0777))
        {
            printf("Error: socket directory not created\n");
            goto finally;
        }
    }
    struct Connection connection = create_connection(ZMQ_SUB, 0, 0);
    if(NULL == connection.context || NULL == connection.socket)
    {
        printf("Error: create_connection() returned \"NULL\"\n");
        printf("Try \"sudo\"\n");
        goto finally;
    }

    struct Note note;
    time_t timer;
    struct Note *note_array;
    note_array = (Note*)calloc(max_quantity_message, sizeof(Note));

    enum Message_signal sig;

    /*Infinite loop for listening to a socket*/
    for(;;)
    {
        sig = DEFAULT;
        zmq_recv(connection.socket, &sig, sizeof(int), 0);
        switch (sig)
        {
        case SEND_MESSAGE:
            recv_meassage(connection, &note.message);
            time(&timer);
            note.registration_time = *localtime(&timer);
            read_from_file(note_array, max_quantity_message);
            delete_all_messages();
            if(1 == write_to_file(note))
            {
                printf("Error: error writing to file\n");
                break;
            }
            for(int i = 0; i < max_quantity_message; i++)
            {
                if(1 == write_to_file(note_array[i]))
                {
                    printf("Error: error writing to file\n");
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
        default:
            break;
        }
    }
finally:
    return 0;
}
