/***************************************************************************
 kevent.c Copyright (C) 1999 Christoph Reichenbach


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

    Christoph Reichenbach (CJR) [jameson@linuxgames.com]

***************************************************************************/

#include "sci/include/engine.h"

int stop_on_event;

#define SCI_VARIABLE_GAME_SPEED 3

reg_t
kGetEvent(state_t *s, int funct_nr, int argc, reg_t *argv)
{
	int mask = UKPV(0);
	reg_t obj = argv[1];
	sci_event_t e;
	int oldx, oldy;
	int modifier_mask = SCI_VERSION_MAJOR(s->version)==0 ? SCI_EVM_ALL
	                                                     : SCI_EVM_NO_FOOLOCK;

	if (s->kernel_opt_flags & KERNEL_OPT_FLAG_GOT_2NDEVENT) {
		/* Penalty time- too many requests to this function without
		** waiting!  */
		int delay = s->script_000->locals_block->locals[SCI_VARIABLE_GAME_SPEED].offset;

		gfxop_usleep(s->gfx_state, (1000000 * delay) / 60);
	}
  
	/*If there's a simkey pending, and the game wants a keyboard event, use the
	 *simkey instead of a normal event*/
	if (_kdebug_cheap_event_hack && (mask & SCI_EVT_KEYBOARD)) {
		PUT_SEL32V(obj, type, SCI_EVT_KEYBOARD); /*Keyboard event*/
		PUT_SEL32V(obj, message, _kdebug_cheap_event_hack);
		PUT_SEL32V(obj, modifiers, SCI_EVM_NUMLOCK); /*Numlock on*/
		PUT_SEL32V(obj, x, s->gfx_state->pointer_pos.x);
		PUT_SEL32V(obj, y, s->gfx_state->pointer_pos.y);
		_kdebug_cheap_event_hack = 0;
		return make_reg(0, 1);
	}
  
	oldx = s->gfx_state->pointer_pos.x;
	oldy = s->gfx_state->pointer_pos.y;
	e = gfxop_get_event(s->gfx_state, mask);

	s->parser_event = NULL_REG; /* Invalidate parser event */

	PUT_SEL32V(obj, x, s->gfx_state->pointer_pos.x);
	PUT_SEL32V(obj, y, s->gfx_state->pointer_pos.y);

	/*  gfxop_set_pointer_position(s->gfx_state, gfx_point(s->gfx_state->pointer_pos.x, s->gfx_state->pointer_pos.y)); */


	if (e.type)
		s->kernel_opt_flags &= ~(KERNEL_OPT_FLAG_GOT_EVENT
					 | KERNEL_OPT_FLAG_GOT_2NDEVENT);
	else {
		if (s->kernel_opt_flags & KERNEL_OPT_FLAG_GOT_EVENT)
			s->kernel_opt_flags |= KERNEL_OPT_FLAG_GOT_2NDEVENT;
		else
			s->kernel_opt_flags |= KERNEL_OPT_FLAG_GOT_EVENT;
	}

	switch(e.type)
		{
		case SCI_EVT_QUIT:
			quit_vm();
			break;

		case SCI_EVT_KEYBOARD: {

			if ((e.buckybits & SCI_EVM_LSHIFT) && (e.buckybits & SCI_EVM_RSHIFT)
			    && (e.data == '-')) {

				sciprintf("Debug mode activated\n");

				script_debug_flag = 1; /* Enter debug mode */
				_debug_seeking = _debug_step_running = 0;
				s->onscreen_console = 0;

			} else if ((e.buckybits & SCI_EVM_CTRL) && (e.data == '`')) {

				script_debug_flag = 1; /* Enter debug mode */
				_debug_seeking = _debug_step_running = 0;
				s->onscreen_console = 1;

			} else if ((e.buckybits & SCI_EVM_CTRL) && (e.data == '1')) {

				if (s->visual)
					s->visual->print(GFXW(s->visual), 0);

			} else {
				PUT_SEL32V(obj, type, SCI_EVT_KEYBOARD); /*Keyboard event*/
				s->r_acc=make_reg(0, 1);
				PUT_SEL32V(obj, message, e.character);
				/* We only care about the translated
				** character  */
				PUT_SEL32V(obj, modifiers, e.buckybits&modifier_mask);

			}
		} break;

		case SCI_EVT_MOUSE_RELEASE:
		case SCI_EVT_MOUSE_PRESS: {
			int extra_bits=0;

			if(mask & e.type) {
				switch(e.data) {
				case 2: extra_bits=SCI_EVM_LSHIFT|SCI_EVM_RSHIFT; break;
				case 3: extra_bits=SCI_EVM_CTRL;
				default:break;
				}

				PUT_SEL32V(obj, type, e.type);
				PUT_SEL32V(obj, message, 1);
				PUT_SEL32V(obj, modifiers, (e.buckybits|extra_bits)&modifier_mask);
				s->r_acc = make_reg(0, 1);
			}
		} break;

		default: {
			s->r_acc = NULL_REG; /* Unknown or no event */
		}
		}

	if ((s->r_acc.offset) && (stop_on_event)) {
		stop_on_event = 0;
		script_debug_flag = 1;
	}  

	return s->r_acc;
}

reg_t
kMapKeyToDir(state_t *s, int funct_nr, int argc, reg_t *argv)
{
	reg_t obj = argv[0];

	if (GET_SEL32V(obj, type) == SCI_EVT_KEYBOARD) { /* Keyboard */
		int mover = -1;
		switch (GET_SEL32V(obj, message)) {
		case SCI_K_HOME: mover = 8; break;
		case SCI_K_UP: mover = 1; break;
		case SCI_K_PGUP: mover = 2; break;
		case SCI_K_LEFT: mover = 7; break;
		case SCI_K_CENTER:
		case 76: mover = 0; break;
		case SCI_K_RIGHT: mover = 3; break;
		case SCI_K_END: mover = 6; break;
		case SCI_K_DOWN: mover = 5; break;
		case SCI_K_PGDOWN: mover = 4; break;
		default: break;
		}

		if (mover >= 0) {
			PUT_SEL32V(obj, type, SCI_EVT_JOYSTICK);
			PUT_SEL32V(obj, message, mover);
			return make_reg(0, 1);
		} else return NULL_REG;
	}

	return s->r_acc;
}


reg_t
kGlobalToLocal(state_t *s, int funct_nr, int argc, reg_t *argv)
{
	reg_t obj = argc ? argv[0] : NULL_REG; /* Can this really happen? Lars */

	if (obj.segment) {
		int x = GET_SEL32V(obj, x);
		int y = GET_SEL32V(obj, y);

		PUT_SEL32V(obj, x, x - s->port->zone.x);
		PUT_SEL32V(obj, y, y - s->port->zone.y);
	}

	return s->r_acc;

}


reg_t
kLocalToGlobal(state_t *s, int funct_nr, int argc, reg_t *argv)
{
	reg_t obj = argc ? argv[0] : NULL_REG; /* Can this really happen? Lars */

	if (obj.segment) {
		int x = GET_SEL32V(obj, x);
		int y = GET_SEL32V(obj, y);

		PUT_SEL32V(obj, x, x + s->port->zone.x);
		PUT_SEL32V(obj, y, y + s->port->zone.y);
	}

	return s->r_acc;
}

reg_t /* Not implemented */
kJoystick(state_t *s, int funct_nr, int argc, reg_t *argv)
{
	SCIkdebug(SCIkSTUB, "Unimplemented syscall 'Joystick()'\n", funct_nr);
	return NULL_REG;
}


