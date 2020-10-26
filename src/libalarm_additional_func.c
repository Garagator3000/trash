#include "alarm.h"

/*zero - not equal, higher than zero - equal*/
int alarm__message_compare(
    struct Message message,
    struct Message filter)
{
    int return_value = 0;
    int likeness = 0;
    int fields = 0;

    if (0 != strcmp(filter.module, ""))
    {
        fields = fields + 1;
        if (0 == strcmp(message.module, filter.module))
        {
            likeness = likeness + 1;
        }
    }
    if (0 != filter.type)
    {
        fields = fields + 2;
        if (message.type == filter.type)
        {
            likeness = likeness + 2;
        }
    }
    if (0 != filter.priority)
    {
        fields = fields + 3;
        if (message.priority == filter.priority)
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

/*X-iteration delay
to let zmq make a context and socket*/
void alarm__delay(int x)
{
    for (int i = 0; i < x; i++);
}
