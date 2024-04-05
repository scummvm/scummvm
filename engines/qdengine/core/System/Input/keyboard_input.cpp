/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "keyboard_input.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

keyboardDispatcher::keyboardDispatcher() : handler_(0)
{
	for(int i = 0; i < 256; i ++) key_states_[i] = false;
}

keyboardDispatcher::~keyboardDispatcher()
{
}

keyboardDispatcher* keyboardDispatcher::instance()
{
	static keyboardDispatcher dsp;
	return &dsp;
}
