#include "common/timer.h"
#include "engines/engine.h"
#include "sci/include/sfx_timer.h"


#define FREQ 60
#define DELAY (1000000 / FREQ)

typedef void (*scummvm_timer_callback_t)(void *);
static scummvm_timer_callback_t scummvm_timer_callback = NULL;
static void *scummvm_timer_callback_data = NULL;
extern ::Engine *g_engine;

void scummvm_timer_update_internal(void *ptr) {
	if (scummvm_timer_callback)
		scummvm_timer_callback(scummvm_timer_callback_data);
}

int scummvm_timer_start(void (*func)(void *), void *data) {
	if (scummvm_timer_callback) {
		fprintf(stderr,
		        "Error: Attempt to initialize gametick timer more than once\n");
		return SFX_ERROR;
	}

	if (!func) {
		fprintf(stderr,
		        "Error: Attempt to initialize gametick timer w/o callback\n");
		return SFX_ERROR;
	}

	scummvm_timer_callback = func;
	scummvm_timer_callback_data = data;

	::g_engine->getTimerManager()->installTimerProc(&scummvm_timer_update_internal, DELAY, NULL);
	return SFX_OK;
}

int scummvm_timer_stop() {
	scummvm_timer_callback = NULL;
	return SFX_OK;
}


sfx_timer_t sfx_timer_scummvm = {
	"ScummVM",
	"0.1",
	DELAY / 1000, 0,
	NULL,
	&scummvm_timer_start,
	&scummvm_timer_stop,
	0,
	0
};
