/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "sci/sci.h"
#include "sci/engine/state.h"
#include "sci/engine/kernel.h"
#include "sci/gfx/gfx_widgets.h"
#include "sci/gfx/gfx_state_internal.h"	// required for GfxPort, GfxVisual
#include "sci/console.h"
#include "sci/debug.h"	// for g_debug_simulated_key

namespace Sci {

#define SCI_VARIABLE_GAME_SPEED 3

reg_t kGetEvent(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	int mask = argv[0].toUint16();
	reg_t obj = argv[1];
	sci_event_t e;
	int oldx, oldy;
	int modifier_mask = s->_version <= SCI_VERSION_01 ? SCI_EVM_ALL : SCI_EVM_NO_FOOLOCK;

	// If there's a simkey pending, and the game wants a keyboard event, use the
	// simkey instead of a normal event
	if (g_debug_simulated_key && (mask & SCI_EVT_KEYBOARD)) {
		PUT_SEL32V(obj, type, SCI_EVT_KEYBOARD); // Keyboard event
		PUT_SEL32V(obj, message, g_debug_simulated_key);
		PUT_SEL32V(obj, modifiers, SCI_EVM_NUMLOCK); // Numlock on
		PUT_SEL32V(obj, x, s->gfx_state->pointer_pos.x);
		PUT_SEL32V(obj, y, s->gfx_state->pointer_pos.y);
		g_debug_simulated_key = 0;
		return make_reg(0, 1);
	}

	oldx = s->gfx_state->pointer_pos.x;
	oldy = s->gfx_state->pointer_pos.y;
	e = gfxop_get_event(s->gfx_state, mask);

	s->parser_event = NULL_REG; // Invalidate parser event

	PUT_SEL32V(obj, x, s->gfx_state->pointer_pos.x);
	PUT_SEL32V(obj, y, s->gfx_state->pointer_pos.y);

	//gfxop_set_pointer_position(s->gfx_state, Common::Point(s->gfx_state->pointer_pos.x, s->gfx_state->pointer_pos.y));

	switch (e.type) {
	case SCI_EVT_QUIT:
		quit_vm();
		break;

	case SCI_EVT_KEYBOARD:
		if ((e.buckybits & SCI_EVM_LSHIFT) && (e.buckybits & SCI_EVM_RSHIFT) && (e.data == '-')) {
			printf("Debug mode activated\n");
			scriptState.seeking = kDebugSeekNothing;
			scriptState.runningStep = 0;
		} else if ((e.buckybits & SCI_EVM_CTRL) && (e.data == '`')) {
			printf("Debug mode activated\n");
			scriptState.seeking = kDebugSeekNothing;
			scriptState.runningStep = 0;
		} else {
			PUT_SEL32V(obj, type, SCI_EVT_KEYBOARD); // Keyboard event
			s->r_acc = make_reg(0, 1);
			PUT_SEL32V(obj, message, e.character);
			// We only care about the translated
			// character
			PUT_SEL32V(obj, modifiers, e.buckybits&modifier_mask);
		}
		break;

	case SCI_EVT_MOUSE_RELEASE:
	case SCI_EVT_MOUSE_PRESS: {
		int extra_bits = 0;

		// track left buttton clicks, if requested
		if (e.type == SCI_EVT_MOUSE_PRESS && e.data == 1 && g_debug_track_mouse_clicks) {
			((SciEngine *)g_engine)->getSciDebugger()->DebugPrintf("Mouse clicked at %d, %d\n", 
						s->gfx_state->pointer_pos.x, s->gfx_state->pointer_pos.y);
		}

		if (mask & e.type) {
			switch (e.data) {
			case 2:
				extra_bits = SCI_EVM_LSHIFT | SCI_EVM_RSHIFT;
				break;
			case 3:
				extra_bits = SCI_EVM_CTRL;
			default:
				break;
			}

			PUT_SEL32V(obj, type, e.type);
			PUT_SEL32V(obj, message, 1);
			PUT_SEL32V(obj, modifiers, (e.buckybits | extra_bits)&modifier_mask);
			s->r_acc = make_reg(0, 1);
		}
		break;
	}

	default:
		s->r_acc = NULL_REG; // Unknown or no event
	}

	if ((s->r_acc.offset) && (scriptState.stopOnEvent)) {
		scriptState.stopOnEvent = false;

		// A SCI event occured, and we have been asked to stop, so open the debug console
		Console *con = ((Sci::SciEngine*)g_engine)->getSciDebugger();
		con->DebugPrintf("SCI event occured: ");
		switch (e.type) {
		case SCI_EVT_QUIT:
			con->DebugPrintf("quit event\n");
			break;
		case SCI_EVT_KEYBOARD:
			con->DebugPrintf("keyboard event\n");
			break;
		case SCI_EVT_MOUSE_RELEASE:
		case SCI_EVT_MOUSE_PRESS:
			con->DebugPrintf("mouse click event\n");
			break;
		default:
			con->DebugPrintf("unknown or no event (event type %d)\n", e.type);
		}

		con->attach();
		con->onFrame();
	}

	return s->r_acc;
}

reg_t kMapKeyToDir(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	reg_t obj = argv[0];

	if (GET_SEL32V(obj, type) == SCI_EVT_KEYBOARD) { // Keyboard
		int mover = -1;
		switch (GET_SEL32V(obj, message)) {
		case SCI_K_HOME:
			mover = 8;
			break;
		case SCI_K_UP:
			mover = 1;
			break;
		case SCI_K_PGUP:
			mover = 2;
			break;
		case SCI_K_LEFT:
			mover = 7;
			break;
		case SCI_K_CENTER:
		case 76:
			mover = 0;
			break;
		case SCI_K_RIGHT:
			mover = 3;
			break;
		case SCI_K_END:
			mover = 6;
			break;
		case SCI_K_DOWN:
			mover = 5;
			break;
		case SCI_K_PGDOWN:
			mover = 4;
			break;
		default:
			break;
		}

		if (mover >= 0) {
			PUT_SEL32V(obj, type, SCI_EVT_JOYSTICK);
			PUT_SEL32V(obj, message, mover);
			return make_reg(0, 1);
		} else
			return NULL_REG;
	}

	return s->r_acc;
}

reg_t kGlobalToLocal(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	reg_t obj = argc ? argv[0] : NULL_REG; // Can this really happen? Lars

	if (obj.segment) {
		int x = GET_SEL32V(obj, x);
		int y = GET_SEL32V(obj, y);

		PUT_SEL32V(obj, x, x - s->port->zone.x);
		PUT_SEL32V(obj, y, y - s->port->zone.y);
	}

	return s->r_acc;

}

reg_t kLocalToGlobal(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	reg_t obj = argc ? argv[0] : NULL_REG; // Can this really happen? Lars

	if (obj.segment) {
		int x = GET_SEL32V(obj, x);
		int y = GET_SEL32V(obj, y);

		PUT_SEL32V(obj, x, x + s->port->zone.x);
		PUT_SEL32V(obj, y, y + s->port->zone.y);
	}

	return s->r_acc;
}

reg_t kJoystick(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	// Subfunction 12 sets/gets joystick repeat rate
	debug(5, "Unimplemented syscall 'Joystick()'");
	return NULL_REG;
}

} // End of namespace Sci
