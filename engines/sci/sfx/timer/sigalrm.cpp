/***************************************************************************
 sigalrm.c Copyright (C) 2002 Christoph Reichenbach


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

    Christoph Reichenbach (CR) <jameson@linuxgames.com>

***************************************************************************/

#include <sfx_timer.h>

#ifdef HAVE_SETITIMER

#include <sys/time.h>
#include <signal.h>
#include <stdio.h>

#ifdef HAVE_PTHREAD
#include <pthread.h>
#define sigprocmask pthread_sigmask
#endif

static void (*sig_callback)(void *) = NULL;
static void *sig_callback_data = NULL;
static sigset_t current_sigset;

static void
timer_handler(int i) {
	if (sig_callback)
		sig_callback(sig_callback_data);
}

static int
sigalrm_set_option(char *name, char *value) {
	return SFX_ERROR;
}


static int
sigalrm_start(void) {
	struct itimerval itimer;

	itimer.it_value.tv_sec = 0;
	itimer.it_value.tv_usec = 1000000 / 60;
	itimer.it_interval = itimer.it_value;

	signal(SIGALRM, timer_handler); /* Re-instate timer handler, to make sure */
	setitimer(ITIMER_REAL, &itimer, NULL);

	return SFX_OK;
}


static int
sigalrm_init(void (*callback)(void *), void *data) {
	if (sig_callback) {
		fprintf(stderr, "Error: Attempt to initialize sigalrm timer more than once\n");
		return SFX_ERROR;
	}

	if (!callback) {
		fprintf(stderr, "Error: Attempt to initialize sigalrm timer w/o callback\n");
		return SFX_ERROR;
	}

	sig_callback = callback;
	sig_callback_data = data;

	sigalrm_start();

	sigemptyset(&current_sigset);
	sigaddset(&current_sigset, SIGALRM);

	return SFX_OK;
}


static int
sigalrm_stop(void) {
	struct itimerval itimer;

	if (!sig_callback) {
		fprintf(stderr, "Error: Attempt to stop sigalrm timer when not running\n");
		return SFX_ERROR;
	}

	itimer.it_value.tv_sec = 0;
	itimer.it_value.tv_usec = 0;
	itimer.it_interval = itimer.it_value;

	setitimer(ITIMER_REAL, &itimer, NULL); /* Stop timer */
	signal(SIGALRM, SIG_DFL);

	return SFX_OK;
}


static int
sigalrm_block(void) {
	if (sigprocmask(SIG_BLOCK, &current_sigset, NULL) != 0) {
		fprintf(stderr, "Error: Failed to block sigalrm\n");
		return SFX_ERROR;
	}

	return SFX_OK;
}


static int
sigalrm_unblock(void) {
	if (sigprocmask(SIG_UNBLOCK, &current_sigset, NULL) != 0) {
		fprintf(stderr, "Error: Failed to unblock sigalrm\n");
		return SFX_ERROR;
	}

	return SFX_OK;
}


sfx_timer_t sfx_timer_sigalrm = {
	"sigalrm",
	"0.1",
	17, /* 1000 / 60 */
	0,
	&sigalrm_set_option,
	&sigalrm_init,
	&sigalrm_stop,
	&sigalrm_block,
	&sigalrm_unblock
};

#endif /* HAVE_SETITIMER */
