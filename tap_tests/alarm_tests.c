#include <tap.h>
#include "../src/alarm.h"

int main()
{
    struct Connection test_connection = {NULL, NULL};

    plan(4);

    cmp_ok(alarm__create_connection(&test_connection, 844684, 0), "!=", 0, "create connection test with SUB 0");
    cmp_ok(alarm__create_connection(&test_connection, SUBSCRIBER, 1), "==", 0, "create connection test with SUB 1");
    cmp_ok(alarm__create_connection(&test_connection, PUBLISHER, 0), "==", 0, "create connection test with PUB 0");
    cmp_ok(alarm__create_connection(&test_connection, ZMQ_SUB, 1), "==", 0, "create connection test with PUB 1");

    done_testing();
}
