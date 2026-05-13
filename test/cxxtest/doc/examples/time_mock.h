// time_mock.h
#include <time.h>
#include <cxxtest/Mock.h>

CXXTEST_MOCK_GLOBAL(time_t,         /* Return type          */
                    time,          /* Name of the function */
                    (time_t *t),   /* Prototype            */
                    (t)          /* Argument list        */);
