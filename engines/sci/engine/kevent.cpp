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
#include "sci/gfx/operations.h"
#include "sci/console.h"
#include "sci/debug.h"	// for g_debug_simulated_key
#include "sci/gui/gui.h"
#include "sci/gui/gui_cursor.h"

namespace Sci {

#define SCI_VARIABLE_GAME_SPEED 3

reg_t kGetEvent(EngineState *s, int argc, reg_t *argv) {
	int mask = argv[0].toUint16();
	reg_t obj = argv[1];
	sci_event_t e;
	int oldx, oldy;
	int modifier_mask = getSciVersion() <= SCI_VERSION_01 ? SCI_EVM_ALL : SCI_EVM_NO_FOOLOCK;
	SegManager *segMan = s->_segMan;
	const Common::Point mousePos = s->_gui->getCursorPos();

	// If there's a simkey pending, and the game wants a keyboard event, use the
	// simkey instead of a normal event
	if (g_debug_simulated_key && (mask & SCI_EVT_KEYBOARD)) {
		PUT_SEL32V(segMan, obj, type, SCI_EVT_KEYBOARD); // Keyboard event
		PUT_SEL32V(segMan, obj, message, g_debug_simulated_key);
		PUT_SEL32V(segMan, obj, modifiers, SCI_EVM_NUMLOCK); // Numlock on
		PUT_SEL32V(segMan, obj, x, mousePos.x);
		PUT_SEL32V(segMan, obj, y, mousePos.y);
		g_debug_simulated_key = 0;
		return make_reg(0, 1);
	}

	oldx = mousePos.x;
	oldy = mousePos.y;
	e = gfxop_get_event(s->gfx_state, mask);

	s->parser_event = NULL_REG; // Invalidate parser event

	PUT_SEL32V(segMan, obj, x, mousePos.x);
	PUT_SEL32V(segMan, obj, y, mousePos.y);

	//s->_gui->moveCursor(s->gfx_state->pointer_pos.x, s->gfx_state->pointer_pos.y);

	switch (e.type) {
	case SCI_EVT_QUIT:
		quit_vm();
		break;

	case SCI_EVT_KEYBOARD:
		if ((e.buckybits & SCI_EVM_LSHIFT) && (e.buckybits & SCI_EVM_RSHIFT) && (e.data == '-')) {
			printf("Debug mode activated\n");
			g_debugState.seeking = kDebugSeekNothing;
			g_debugState.runningStep = 0;
		} else if ((e.buckybits & SCI_EVM_CTRL) && (e.data == '`')) {
			printf("Debug mode activated\n");
			g_debugState.seeking = kDebugSeekNothing;
			g_debugState.runningStep = 0;
		} else {
			PUT_SEL32V(segMan, obj, type, SCI_EVT_KEYBOARD); // Keyboard event
			s->r_acc = make_reg(0, 1);

			// TODO: Remove this as soon as ScummVM handles Ctrl-Alt-X to us
			if ((e.buckybits == SCI_EVM_CTRL) && (e.character = 'x'))
				e.buckybits |= SCI_EVM_ALT;

			if (e.buckybits & SCI_EVM_ALT) {
				// If Alt is pressed, we need to convert the actual key to a DOS scancode
				switch (e.character) {
				case 'a': e.character = 30 << 8; break;
				case 'b': e.character = 48 << 8; break;
				case 'c': e.character = 46 << 8; break;
				case 'd': e.character = 32 << 8; break;
				case 'e': e.character = 18 << 8; break;
				case 'f': e.character = 33 << 8; break;
				case 'g': e.character = 34 << 8; break;
				case 'h': e.character = 35 << 8; break;
				case 'i': e.character = 33 << 8; break;
				case 'j': e.character = 23 << 8; break;
				case 'k': e.character = 37 << 8; break;
				case 'l': e.character = 38 << 8; break;
				case 'm': e.character = 50 << 8; break;
				case 'n': e.character = 49 << 8; break;
				case 'o': e.character = 24 << 8; break;
				case 'p': e.character = 25 << 8; break;
				case 'q': e.character = 16 << 8; break;
				case 'r': e.character = 19 << 8; break;
				case 's': e.character = 31 << 8; break;
				case 't': e.character = 20 << 8; break;
				case 'u': e.character = 22 << 8; break;
				case 'v': e.character = 47 << 8; break;
				case 'w': e.character = 17 << 8; break;
				case 'x': e.character = 45 << 8; break;
				case 'y': e.character = 21 << 8; break;
				case 'z': e.character = 44 << 8; break;
				}
			} else if (e.buckybits & SCI_EVM_CTRL) {
				// Control is pressed...
				if ((e.character >= 97) && (e.character <= 121)) {
					e.character -= 96; // 'a' -> 0x01, etc.
				}
			}

			PUT_SEL32V(segMan, obj, message, e.character);
			// We only care about the translated character
			PUT_SEL32V(segMan, obj, modifiers, e.buckybits&modifier_mask);
		}
		break;

	case SCI_EVT_MOUSE_RELEASE:
	case SCI_EVT_MOUSE_PRESS:

		// track left buttton clicks, if requested
		if (e.type == SCI_EVT_MOUSE_PRESS && e.data == 1 && g_debug_track_mouse_clicks) {
			((SciEngine *)g_engine)->getSciDebugger()->DebugPrintf("Mouse clicked at %d, %d\n", 
						mousePos.x, mousePos.y);
		}

		if (mask & e.type) {
			int extra_bits = 0;

			switch (e.data) {
			case 2:
				extra_bits = SCI_EVM_LSHIFT | SCI_EVM_RSHIFT;
				break;
			case 3:
				extra_bits = SCI_EVM_CTRL;
			default:
				break;
			}

			PUT_SEL32V(segMan, obj, type, e.type);
			PUT_SEL32V(segMan, obj, message, 0);
			PUT_SEL32V(segMan, obj, modifiers, (e.buckybits | extra_bits)&modifier_mask);
			s->r_acc = make_reg(0, 1);
		}
		break;

	default:
		s->r_acc = NULL_REG; // Unknown or no event
	}

	if ((s->r_acc.offset) && (g_debugState.stopOnEvent)) {
		g_debugState.stopOnEvent = false;

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

reg_t kMapKeyToDir(EngineState *s, int argc, reg_t *argv) {
	reg_t obj = argv[0];
	SegManager *segMan = s->_segMan;

	if (GET_SEL32V(segMan, obj, type) == SCI_EVT_KEYBOARD) { // Keyboard
		int mover = -1;
		switch (GET_SEL32V(segMan, obj, message)) {
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
			PUT_SEL32V(segMan, obj, type, SCI_EVT_JOYSTICK);
			PUT_SEL32V(segMan, obj, message, mover);
			return make_reg(0, 1);
		} else
			return NULL_REG;
	}

	return s->r_acc;
}

reg_t kGlobalToLocal(EngineState *s, int argc, reg_t *argv) {
	reg_t obj = argc ? argv[0] : NULL_REG; // Can this really happen? Lars
	SegManager *segMan = s->_segMan;

	if (obj.segment) {
		int16 x = GET_SEL32V(segMan, obj, x);
		int16 y = GET_SEL32V(segMan, obj, y);

		s->_gui->globalToLocal(&x, &y);

		PUT_SEL32V(segMan, obj, x, x);
		PUT_SEL32V(segMan, obj, y, y);
	}

	return s->r_acc;

}

reg_t kLocalToGlobal(EngineState *s, int argc, reg_t *argv) {
	reg_t obj = argc ? argv[0] : NULL_REG; // Can this really happen? Lars
	SegManager *segMan = s->_segMan;

	if (obj.segment) {
		int16 x = GET_SEL32V(segMan, obj, x);
		int16 y = GET_SEL32V(segMan, obj, y);

		s->_gui->localToGlobal(&x, &y);

		PUT_SEL32V(segMan, obj, x, x);
		PUT_SEL32V(segMan, obj, y, y);
	}

	return s->r_acc;
}

reg_t kJoystick(EngineState *s, int argc, reg_t *argv) {
	// Subfunction 12 sets/gets joystick repeat rate
	debug(5, "Unimplemented syscall 'Joystick()'");
	return NULL_REG;
}

} // End of namespace Sci
