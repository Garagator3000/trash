#include "alarm.h"

/*Write to "messages.txt" a note.
str_time[] - string to form a normal representation of time:
"date: mm/dd/yy time: hh:mi:ss".*/
int alarm__write_to_file(
    struct Note note)
{
    int check = 0;
    char str_time[DATE_TIME_MAX_SIZE] = {0};
    FILE *file = NULL;

    if (0 == note.registration_time.tm_year)
    {
        check = -2;
        goto finally;
    }

    file = fopen(PATH_TO_FILE, "a");
    if (NULL == file)
    {
        check = -1;
        printf("Error: file not open\n");
        goto finally;
    }

    strftime(str_time, DATE_TIME_MAX_SIZE, "date: %x time: %X", &note.registration_time);
    fprintf(file, "Registration time: %s\n", str_time);

    fprintf(file, "Module: %s\n", note.message.module);

    switch (note.message.type)
    {
        case 1:
            fprintf(file, "Type: ALARM\n");
            break;
        case 2:
            fprintf(file, "Type: normalize\n");
            break;
        default:
            check = -1;
            printf("Error: invalid value of message type\n");
            goto finally;
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
            fprintf(file, "Priority: usually\n");
            break;
        case 4:
            fprintf(file, "Priority: not important\n");
            break;
        default:
            check = -1;
            printf("Error: invalid value of message priority\n");
            goto finally;
            break;
    }

    fprintf(file, "%s\n", note.message.message_text);

    fprintf(file, "\n\n");

 finally:

    if (NULL != file)
    {
        fclose(file);
    }

    return check;
}

int alarm__read_from_file(
    struct Note *note_array,
    int max_quantity_notes)
{
    char buffer[MESSAGE_TEXT_SIZE] = {0};    //buffer for read line
    char buf[READ_BUFF_SIZE] = {0};          //buffer for value of type and priority
    int return_value = 0;
    FILE *file = NULL;

    if (NULL == note_array)
    {
        return_value = -1;
        printf("Error: read_from_file: arg[1] cannot be equal NULL\n");
        goto finally;
    }

    file = fopen(PATH_TO_FILE, "r");
    if (NULL == file)
    {
        return_value = -1;
        printf("Error: read_from_file: file not open\n");
        goto finally;
    }

    for (int i = 0; i < max_quantity_notes; i++)
    {
        if (NULL == fgets(buffer, MESSAGE_TEXT_SIZE, file))
        {
            goto finally;
        }
        if ('R' != buffer[0])
        {
            break;
        }

        sscanf(buffer, "Registration time: date: %d/%d/%d time: %d:%d:%d",
                    &note_array[i].registration_time.tm_mon,
                    &note_array[i].registration_time.tm_mday,
                    &note_array[i].registration_time.tm_year,
                    &note_array[i].registration_time.tm_hour,
                    &note_array[i].registration_time.tm_min,
                    &note_array[i].registration_time.tm_sec);
            note_array[i].registration_time.tm_mon -= 1;

        if (NULL == fgets(buffer, MESSAGE_TEXT_SIZE, file))
        {
            return_value = -3;
            goto finally;
        }
        sscanf(buffer, "Module: %s", note_array[i].message.module);

        if (NULL == fgets(buffer, MESSAGE_TEXT_SIZE, file))
        {
            return_value = -3;
            goto finally;
        }
        sscanf(buffer, "Type: %s", buf);
        if ('A' == buf[0])
        {
            note_array[i].message.type = ALARM;
        }
        else
        {
            note_array[i].message.type = NORMALIZE;
        }

        if (NULL == fgets(buffer, MESSAGE_TEXT_SIZE, file))
        {
            return_value = -3;
            goto finally;
        }
        sscanf(buffer, "Priority: %s", buf);
        if ('v' == buf[0])
        {
            note_array[i].message.priority = VERY_IMPORTANT;
        }
        else if ('i' == buf[0])
        {
            note_array[i].message.priority = IMPORTANT;
        }
        else if ('u' == buf[0])
        {
            note_array[i].message.priority = USUALLY;
        }
        else
        {
            note_array[i].message.priority = NOT_IMPORTANT;
        }

        if (NULL == fgets(buffer, MESSAGE_TEXT_SIZE, file))
        {
            return_value = -3;
            goto finally;
        }
        buffer[strlen(buffer) - 1] = 0;
        strncpy(note_array[i].message.message_text, buffer, MESSAGE_TEXT_SIZE);

        if (NULL == fgets(buffer, 2, file))
        {
            return_value = -3;
            goto finally;
        }
        if (NULL == fgets(buffer, 2, file))
        {
            return_value = -3;
            goto finally;
        }

        return_value++;
    }

 finally:
    if (NULL != file)
    {
        fclose(file);
    }
    if (-3 == return_value)
    {
        printf("Error: fgets() in read_from_file()\n");
    }
    return return_value;
}

/*delete only your messages*/
int alarm__delete_my_messages(void)
{
    int check = 0;
    FILE *file = NULL;

    file = fopen(PATH_TO_FILE, "w");
    if (NULL == file)
    {
        printf("Error: delete_all_my_message_from_file(): file not open\n");
        check = -1;
        goto finally;
    }

    if (0 != fclose(file))
    {
        printf("Error: delete_all_my_message_from_file(): file not close\n");
        check = -1;
    }

 finally:

    return check;
}

/*Used to read all messages from the file at startup,
returns the number of messages read.
If the file does not exist,
the file will be created and the function will return int 0*/
int alarm__open_file_first_time(
    struct Note *note_array,
    int max_quantity_messages)
{
    int return_value = 0;
    int check = 0;
    FILE *file_check = NULL;

    return_value = alarm__read_from_file(note_array, max_quantity_messages);
    if (-1 == return_value)
    {
        printf("Creating file... \"%s\"\n", PATH_TO_FILE);
        file_check = fopen(PATH_TO_FILE, "w");
        if (NULL == file_check)
        {
            printf("Failed to create file\n");
            return_value = -1;
            goto finally;
        }

        check = fclose(file_check);
        if (EOF == check)
        {
            printf("Failed to close file\n");
            return_value = -1;
            goto finally;
        }

        printf("File is created\n");
        return_value = check;
    }
    else
    {
        check = alarm__delete_my_messages();
        if (-1 == check)
        {
            printf("Error: alarm__open_file_first_time():\
                    delete_all_my_message_from_file()\n");
            return_value = -1;
            goto finally;
        }
        for(int i = 0; i < max_quantity_messages; i++)
        {
            check = alarm__write_to_file(note_array[i]);
            if (-1 == check)
            {
                printf("Error: alarm__open_file_first_time:\
                        write_to_file()\n");
                return_value = check;
                goto finally;
            }
            if (-2 == check)
            {
                break;
            }
        }
    }

 finally:
    return return_value;
}
