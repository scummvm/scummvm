/* Header is not present in Windows CE SDK */

struct timeval
{
	int tv_sec;
	int tv_usec;
};

void gettimeofday(struct timeval* tp, void* dummy);
void usleep(long usec);
