// rand_example.cpp
#include <time_mock.h>

int generateRandomNumber()
{
    return T::time(NULL) * 3;
}
