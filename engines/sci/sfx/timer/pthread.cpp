/***************************************************************************
 pthread.c Copyright (C) 2005 Walter van Niftrik


 This program may be modified and copied freely according to the terms of
 the GNU general public license (GPL), as long as the above copyright
 notice and the licensing information contained herein are preserved.

 Please refer to www.gnu.org for licensing details.

 This work is provided AS IS, without warranty of any kind, expressed or
 implied, including but not limited to the warranties of merchantibility,
 noninfringement, and fitness for a specific purpose. The author will not
 be held liable for any damage caused by this work or derivatives of it.

 By using this source code, you agree to the licensing terms as stated
 above.


 Please contact the maintainer for bug reports or inquiries.

 Current Maintainer:

    Walter van Niftrik <w.f.b.w.v.niftrik@stud.tue.nl>

***************************************************************************/

#include <sfx_timer.h>
#include <pthread.h>
#include <unistd.h>

/* Timer frequency in hertz */
#define FREQ 60

/* Delay in ms */
#define DELAY (1000 / FREQ)

static void (*callback)(void *) = NULL;
static void *callback_data = NULL;
pthread_t thread;
volatile static int thread_run;

static void *
timer_thread(void *arg)
{
	while (thread_run) {
		if (callback)
			callback(callback_data);

		usleep(DELAY * 1000);
	}

	return NULL;
}

static int
set_option(char *name, char *value)
{
	return SFX_ERROR;
}

static int
init(void (*func)(void *), void *data)
{
	if (callback) {
		fprintf(stderr, "Error: Attempt to initialize pthread timer more than once\n");
		return SFX_ERROR;
	}

	if (!func) {
		fprintf(stderr, "Error: Attempt to initialize pthread timer w/o callback\n");
		return SFX_ERROR;
	}

	callback = func;
	callback_data = data;

	thread_run = 1;
	if (pthread_create(&thread, NULL, timer_thread, NULL)) {
		fprintf(stderr, "Error: Could not create thread.\n");
		return SFX_ERROR;
	}

	return SFX_OK;
}

static int
stop(void)
{
	thread_run = 0;
	pthread_join(thread, NULL);

	return SFX_OK;
}

sfx_timer_t sfx_timer_pthread = {
	"pthread",
	"0.1",
	DELAY,
	0,
	&set_option,
	&init,
	&stop
};
