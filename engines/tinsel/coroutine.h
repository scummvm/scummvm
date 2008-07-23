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

#ifndef TINSEL_COROUTINE_H
#define TINSEL_COROUTINE_H

#include "common/scummsys.h"

namespace Tinsel {

// The following is loosely based on <http://www.chiark.greenend.org.uk/~sgtatham/coroutines.html>,
// Proper credit to Simon Tatham shall be given.

/*

 * `ccr' macros for re-entrant coroutines.

 */

struct CoroBaseContext {
	int _line;
	int _sleep;
	CoroBaseContext *_subctx;
	CoroBaseContext() : _line(0), _sleep(0), _subctx(0) {}
	~CoroBaseContext() { delete _subctx; }
};

typedef CoroBaseContext *CoroContext;


/**
 * Wrapper class which holds a pointer to a pointer to a CoroBaseContext.
 * The interesting part is the destructor, which kills the context being held,
 * but ONLY if the _sleep val of that context is zero.
 */
struct CoroContextHolder {
	CoroContext &_ctx;
	CoroContextHolder(CoroContext &ctx) : _ctx(ctx) {}
	~CoroContextHolder() { if (_ctx && _ctx->_sleep == 0) { delete _ctx; _ctx = 0; } }
};

#define CORO_PARAM     CoroContext &coroParam


#define CORO_BEGIN_CONTEXT  struct CoroContextTag : CoroBaseContext { int DUMMY
#define CORO_END_CONTEXT(x)    } *x = (CoroContextTag *)coroParam

#define CORO_BEGIN_CODE(x) \
		if (!x) {coroParam = x = new CoroContextTag();}\
		assert(coroParam);\
		assert(coroParam->_sleep >= 0);\
		coroParam->_sleep = 0;\
		CoroContextHolder tmpHolder(coroParam);\
		switch(coroParam->_line) { case 0:;

#define CORO_END_CODE \
		}

#define CORO_SLEEP(delay) \
		do {\
			coroParam->_line=__LINE__;\
			coroParam->_sleep=delay;\
			return; case __LINE__:;\
		} while (0)

// Stop the currently running coroutine
#define CORO_KILL_SELF()         do { coroParam->_sleep = -1; return; } while(0)

//#define CORO_ABORT()    do { delete (ctx); ctx = 0; } while (0)

#define CORO_INVOKE_ARGS(subCoro, ARGS)  \
		do {\
			coroParam->_line=__LINE__;\
			coroParam->_subctx = 0;\
			do {\
				subCoro ARGS;\
				if (!coroParam->_subctx) break;\
				coroParam->_sleep = coroParam->_subctx->_sleep;\
				return; case __LINE__:;\
			} while(1);\
		} while (0)

#define CORO_SUBCTX   coroParam->_subctx

#define CORO_INVOKE_0(subCoroutine) \
			CORO_INVOKE_ARGS(subCoroutine,(CORO_SUBCTX))
#define CORO_INVOKE_1(subCoroutine, a0) \
			CORO_INVOKE_ARGS(subCoroutine,(CORO_SUBCTX,a0))
#define CORO_INVOKE_2(subCoroutine, a0,a1) \
			CORO_INVOKE_ARGS(subCoroutine,(CORO_SUBCTX,a0,a1))
#define CORO_INVOKE_3(subCoroutine, a0,a1,a2) \
			CORO_INVOKE_ARGS(subCoroutine,(CORO_SUBCTX,a0,a1,a2))
#define CORO_INVOKE_4(subCoroutine, a0,a1,a2,a3) \
			CORO_INVOKE_ARGS(subCoroutine,(CORO_SUBCTX,a0,a1,a2,a3))
#define CORO_INVOKE_5(subCoroutine, a0,a1,a2,a3,a4) \
			CORO_INVOKE_ARGS(subCoroutine,(CORO_SUBCTX,a0,a1,a2,a3,a4))



} // end of namespace Tinsel

#endif		// TINSEL_COROUTINE_H
