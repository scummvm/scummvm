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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef GLK_JUMPS
#define GLK_JUMPS

#include "common/str.h"

/* This provides a simplified version of the ScummVM coroutines to allow for automated
 * breakouts to the main game loop from subroutinese rather than using unportable setjmps
 */

namespace Glk {

/**
 * Context used for flagging when a break to the outer game loop
 */
struct Context {
	bool _break;
	Common::String _label;

	Context() : _break(false) {}

	/**
	 * Clear the context
	 */
	void clear() {
		_break = false;
		_label = "";
	}
};

#define CALL0(METHOD) { METHOD(context); if (context._break) return; }
#define CALL1(METHOD, P1) { METHOD(context, P1); if (context._break) return; }
#define CALL2(METHOD, P1, P2) { METHOD(context, P1, P2); if (context._break) return; }
#define CALL3(METHOD, P1, P2, P3) { METHOD(context, P1, P2, P3); if (context._break) return; }
#define CALL4(METHOD, P1, P2, P3, P4) { METHOD(context, P1, P2, P3, P4); if (context._break) return; }
#define FUNC0(METHOD, RET) { RET = METHOD(context); if (context._break) return; }
#define FUNC1(METHOD, RET, P1) { RET = METHOD(context, P1); if (context._break) return; }
#define FUNC2(METHOD, RET, P1, P2) { RET = METHOD(context, P1, P2); if (context._break) return; }
#define FUNC3(METHOD, RET, P1, P2, P3) { RET = METHOD(context, P1, P2, P3); if (context._break) return; }
#define FUNC4(METHOD, RET, P1, P2, P3, P4) { RET = METHOD(context, P1, P2, P3, P4); if (context._break) return; }

#define R0CALL0(METHOD) { METHOD(context); if (context._break) return 0; }
#define R0CALL1(METHOD, P1) { METHOD(context, P1); if (context._break) return 0; }
#define R0CALL2(METHOD, P1, P2) { METHOD(context, P1, P2); if (context._break) return 0; }
#define R0CALL3(METHOD, P1, P2, P3) { METHOD(context, P1, P2, P3); if (context._break) return 0; }
#define R0CALL4(METHOD, P1, P2, P3, P4) { METHOD(context, P1, P2, P3, P4); if (context._break) return 0; }
#define R0FUNC0(METHOD, RET) { RET = METHOD(context); if (context._break) return 0; }
#define R0FUNC1(METHOD, RET, P1) { RET = METHOD(context, P1); if (context._break) return 0; }
#define R0FUNC2(METHOD, RET, P1, P2) { RET = METHOD(context, P1, P2); if (context._break) return 0; }
#define R0FUNC3(METHOD, RET, P1, P2, P3) { RET = METHOD(context, P1, P2, P3); if (context._break) return 0; }
#define R0FUNC4(METHOD, RET, P1, P2, P3, P4) { RET = METHOD(context, P1, P2, P3, P4); if (context._break) return 0; }

#define CONTEXT Context &context
#define LONG_JUMP { context._break = true; return; }
#define LONG_JUMP0 { context._break = true; return 0; }
#define LONG_JUMP_LABEL(LBL) { context._break = true; context._label = LBL; return; }
#define LONG_JUMP_LABEL0(LBL) { context._break = true; context._label = LBL; return 0; }

} // End of namespace Glk

#endif
