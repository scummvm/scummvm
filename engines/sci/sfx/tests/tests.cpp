#include <stdio.h>
#include <sfx_iterator.h>

#define TESTEQUAL(x, y) if(x != y) printf("test failure: expected %04x, got %04x @ file %s line %d \n", x, y, __FILE__, __LINE__);

static char calledDeathListenerCallback;

static unsigned char song[] = {
// PCM not present
0,
// channel defs
0, 0x20, 0, 0x21, 0, 0, 0, 0, 
0, 0,    0, 0,    0, 0, 0, 0, 
0, 0,    0, 0,    0, 0, 0, 0, 
0, 0,    0, 0,    0, 0, 0, 0,
// note on, wait, note off
0xAA,
0x90, 0xAA, 0xAA,
0xAA, 
0x80, 0xAA, 0xAA,
0xAA,
0x91, 0xAA, 0xAA,
0xAA,
0x81, 0xAA, 0xAA,
0xAA,
// end track
0xFC};

#define SONG_CMD_COUNT 10

#define TEST_SETUP()	\
	unsigned char cmds[4] = {0}; \
	int result = 0; \
	int message; \
	int i; \
\
	song_iterator_t *it = songit_new(song, sizeof(song), 0, 0); \
	it->init(it); \
	SIMSG_SEND(it, SIMSG_SET_PLAYMASK(0x20));\
	calledDeathListenerCallback = 0;

#define TEST_TEARDOWN() \
	songit_free(it);


void testFinishSong()
{
	TEST_SETUP();
	message = songit_next(&it, &cmds, &result, IT_READER_MASK_ALL);
	TESTEQUAL(0xAA, message);

	message = songit_next(&it, &cmds, &result, IT_READER_MASK_ALL);
	TESTEQUAL(0, message);
	TESTEQUAL(3, result);

	for (i=0; i < SONG_CMD_COUNT - 2; i++)
	{
		message = songit_next(&it, &cmds, &result, IT_READER_MASK_ALL);
	}

	TESTEQUAL(SI_FINISHED, message);

	TEST_TEARDOWN();
}


void DeathListenerCallback(void *v1, void *v2)
{
	calledDeathListenerCallback++;
	return;
}

void testDeathListener()
{
	TEST_SETUP();

	song_iterator_add_death_listener(
			it,
			it, 
			(void (*)(void *, void*))DeathListenerCallback);

	for (i=0; i < SONG_CMD_COUNT; i++)
	{
		message = songit_next(&it, &cmds, &result, IT_READER_MASK_ALL);
	}

	TESTEQUAL(SI_FINISHED, message);

	TEST_TEARDOWN();

	TESTEQUAL(1, calledDeathListenerCallback);
}

void testMultipleDeathListeners()
{
	TEST_SETUP();

	song_iterator_add_death_listener(
			it,
			it, 
			(void (*)(void *, void*))DeathListenerCallback);

	song_iterator_add_death_listener(
			it,
			it, 
			(void (*)(void *, void*))DeathListenerCallback);

	for (i=0; i < SONG_CMD_COUNT; i++)
	{
		message = songit_next(&it, &cmds, &result, IT_READER_MASK_ALL);
	}

	TESTEQUAL(SI_FINISHED, message);

	TEST_TEARDOWN();

	TESTEQUAL(2, calledDeathListenerCallback);
}

void testStopSong()
{
	TEST_SETUP();
	SIMSG_SEND(it, SIMSG_STOP);
	
	message = songit_next(&it, &cmds, &result, IT_READER_MASK_ALL);
	TESTEQUAL(SI_FINISHED, message);

	TEST_TEARDOWN();
}

void testStopLoopedSong()
{
	TEST_SETUP();

	SIMSG_SEND(it, SIMSG_SET_LOOPS(3));
	message = songit_next(&it, &cmds, &result, IT_READER_MASK_ALL);
	TESTEQUAL(0xAA, message);

	SIMSG_SEND(it, SIMSG_STOP);
	message = songit_next(&it, &cmds, &result, IT_READER_MASK_ALL);
	TESTEQUAL(SI_FINISHED, message);

	TEST_TEARDOWN();
}

void testChangeSongMask()
{
	TEST_SETUP();

	message = songit_next(&it, &cmds, &result, IT_READER_MASK_ALL);
	TESTEQUAL(0xAA, message);

	SIMSG_SEND(it, SIMSG_SET_PLAYMASK(0x40));
	message = songit_next(&it, &cmds, &result, IT_READER_MASK_ALL);
	TESTEQUAL(0, message);
	TESTEQUAL(0, result);

	TEST_TEARDOWN();
}


int main(int argc, char* argv[])
{
	testFinishSong();
	testDeathListener();
	testMultipleDeathListeners();
	testStopSong();
	testStopLoopedSong();
	testChangeSongMask();
	return 0;
}

