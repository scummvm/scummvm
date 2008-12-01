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

/*
 * The following is loosely based on an article by Simon Tatham:
 *   <http://www.chiark.greenend.org.uk/~sgtatham/coroutines.html>.
 * However, many improvements and tweaks have been made, in particular
 * by taking advantage of C++ features not available in C.
 * 
 * Why is this code here? Well, the Tinsel engine apparently used
 * setjmp/longjmp based coroutines as a core tool from the start, and
 * so they are deeply ingrained into the whole code base. When we
 * started to get Tinsel ready for ScummVM, we had to deal with that.
 * It soon got clear that we could not simply rewrite the code to work
 * without some form of coroutines. While possible in principle, it
 * would have meant a major restructuring of the entire code base, a
 * rather daunting task. Also, it would have very likely introduced
 * tons of regressons.
 * 
 * So instead of getting rid of the coroutines, we chose to implement
 * them in an alternate way, using Simon Tatham's trick as described
 * above. While the trick is dirty, the result seems to be clear enough,
 * we hope; plus, it allowed us to stay relatively close to the
 * original structure of the code, which made it easier to avoid
 * regressions, and will be helpful in the future when comparing things
 * against the original code base.
 */


/**
 * The core of any coroutine context which captures the 'state' of a coroutine.
 * Private use only
 */
struct CoroBaseContext {
	int _line;
	int _sleep;
	CoroBaseContext *_subctx;
	CoroBaseContext() : _line(0), _sleep(0), _subctx(0) {}
	~CoroBaseContext() { delete _subctx; }
};

typedef CoroBaseContext *CoroContext;

extern CoroContext nullContext;

/**
 * Wrapper class which holds a pointer to a pointer to a CoroBaseContext.
 * The interesting part is the destructor, which kills the context being held,
 * but ONLY if the _sleep val of that context is zero. This way, a coroutine
 * can just 'return' w/o having to worry about freeing the allocated context
 * (in Simon Tatham's original code, one had to use a special macro to
 * return from a coroutine).
 */
class CoroContextHolder {
	CoroContext &_ctx;
public:
	CoroContextHolder(CoroContext &ctx) : _ctx(ctx) {}
	~CoroContextHolder() {
		if (_ctx && _ctx->_sleep == 0) {
			delete _ctx;
			_ctx = 0;
		}
	}
};


#define CORO_PARAM     CoroContext &coroParam

#define CORO_SUBCTX   coroParam->_subctx


#define CORO_BEGIN_CONTEXT  struct CoroContextTag : CoroBaseContext { int DUMMY
#define CORO_END_CONTEXT(x)    } *x = (CoroContextTag *)coroParam

#define CORO_BEGIN_CODE(x) \
		if (&coroParam == &nullContext) assert(!nullContext);\
		if (!x) {coroParam = x = new CoroContextTag();}\
		assert(coroParam);\
		assert(coroParam->_sleep >= 0);\
		coroParam->_sleep = 0;\
		CoroContextHolder tmpHolder(coroParam);\
		switch(coroParam->_line) { case 0:;

#define CORO_END_CODE \
			if (&coroParam == &nullContext) nullContext = NULL; \
		}

#define CORO_SLEEP(delay) do {\
			coroParam->_line = __LINE__;\
			coroParam->_sleep = delay;\
			assert(&coroParam != &nullContext);\
			return; case __LINE__:;\
		} while (0)

#define CORO_GIVE_WAY do { g_scheduler->giveWay(); CORO_SLEEP(1); } while (0)
#define CORO_RESCHEDULE do { g_scheduler->reschedule(); CORO_SLEEP(1); } while (0)

/** Stop the currently running coroutine */
#define CORO_KILL_SELF() \
		do { if (&coroParam != &nullContext) { coroParam->_sleep = -1; } return; } while(0)

/** Invoke another coroutine */
#define CORO_INVOKE_ARGS(subCoro, ARGS)  \
		do {\
			coroParam->_line = __LINE__;\
			coroParam->_subctx = 0;\
			do {\
				subCoro ARGS;\
				if (!coroParam->_subctx) break;\
				coroParam->_sleep = coroParam->_subctx->_sleep;\
				assert(&coroParam != &nullContext);\
				return; case __LINE__:;\
			} while(1);\
		} while (0)
#define CORO_INVOKE_ARGS_V(subCoro, RESULT, ARGS)  \
		do {\
			coroParam->_line = __LINE__;\
			coroParam->_subctx = 0;\
			do {\
				subCoro ARGS;\
				if (!coroParam->_subctx) break;\
				coroParam->_sleep = coroParam->_subctx->_sleep;\
				assert(&coroParam != &nullContext);\
				return RESULT; case __LINE__:;\
			} while(1);\
		} while (0)

#define CORO_INVOKE_0(subCoroutine) \
			CORO_INVOKE_ARGS(subCoroutine,(CORO_SUBCTX))
#define CORO_INVOKE_1(subCoroutine, a0) \
			CORO_INVOKE_ARGS(subCoroutine,(CORO_SUBCTX,a0))
#define CORO_INVOKE_2(subCoroutine, a0,a1) \
			CORO_INVOKE_ARGS(subCoroutine,(CORO_SUBCTX,a0,a1))
#define CORO_INVOKE_3(subCoroutine, a0,a1,a2) \
			CORO_INVOKE_ARGS(subCoroutine,(CORO_SUBCTX,a0,a1,a2))


} // end of namespace Tinsel

#endif		// TINSEL_COROUTINE_H
