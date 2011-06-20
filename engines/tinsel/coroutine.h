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
 */

#ifndef TINSEL_COROUTINE_H
#define TINSEL_COROUTINE_H

#include "common/scummsys.h"
#include "common/util.h"	// for SCUMMVM_CURRENT_FUNCTION

namespace Tinsel {

/**
 * @defgroup TinselCoroutines	Coroutine support for Tinsel
 *
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
//@{


// Enable this macro to enable some debugging support in the coroutine code.
//#define COROUTINE_DEBUG	1

/**
 * The core of any coroutine context which captures the 'state' of a coroutine.
 * Private use only.
 */
struct CoroBaseContext {
	int _line;
	int _sleep;
	CoroBaseContext *_subctx;
#if COROUTINE_DEBUG
	const char *_funcName;
#endif
	CoroBaseContext(const char *func);
	~CoroBaseContext();
};

typedef CoroBaseContext *CoroContext;


// FIXME: Document this!
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
	CoroContextHolder(CoroContext &ctx) : _ctx(ctx) {
		assert(ctx);
		assert(ctx->_sleep >= 0);
		ctx->_sleep = 0;
	}
	~CoroContextHolder() {
		if (_ctx && _ctx->_sleep == 0) {
			delete _ctx;
			_ctx = 0;
		}
	}
};


#define CORO_PARAM    CoroContext &coroParam


/**
 * Begin the declaration of a coroutine context.
 * This allows declaring variables which are 'persistent' during the
 * lifetime of the coroutine. An example use would be:
 *
 *  CORO_BEGIN_CONTEXT;
 *    int var;
 *    char *foo;
 *  CORO_END_CONTEXT(_ctx);
 *
 * It is not possible to initialize variables here, due to the way this
 * macro is implemented. Furthermore, to use the variables declared in
 * the coroutine context, you have to access them via the context variable
 * name that was specified as parameter to CORO_END_CONTEXT, e.g.
 *   _ctx->var = 0;
 *
 * @see CORO_END_CONTEXT
 *
 * @note We declare a variable 'DUMMY' to allow the user to specify an 'empty'
 * context, and so compilers won't complain about ";" following the macro.
 */
#define CORO_BEGIN_CONTEXT  \
	struct CoroContextTag : CoroBaseContext { \
		CoroContextTag() : CoroBaseContext(SCUMMVM_CURRENT_FUNCTION) {} \
		int DUMMY

/**
 * End the declaration of a coroutine context.
 * @param x	name of the coroutine context
 * @see CORO_BEGIN_CONTEXT
 */
#define CORO_END_CONTEXT(x)    } *x = (CoroContextTag *)coroParam

/**
 * Begin the code section of a coroutine.
 * @param x	name of the coroutine context
 * @see CORO_BEGIN_CODE
 */
#define CORO_BEGIN_CODE(x) \
		if (&coroParam == &nullContext) assert(!nullContext);\
		if (!x) {coroParam = x = new CoroContextTag();}\
		CoroContextHolder tmpHolder(coroParam);\
		switch (coroParam->_line) { case 0:;

/**
 * End the code section of a coroutine.
 * @see CORO_END_CODE
 */
#define CORO_END_CODE \
			if (&coroParam == &nullContext) { \
				delete nullContext; \
				nullContext = NULL; \
			} \
		}

/**
 * Sleep for the specified number of scheduler cycles.
 */
#define CORO_SLEEP(delay) do {\
			coroParam->_line = __LINE__;\
			coroParam->_sleep = delay;\
			assert(&coroParam != &nullContext);\
			return; case __LINE__:;\
		} while (0)

#define CORO_GIVE_WAY do { g_scheduler->giveWay(); CORO_SLEEP(1); } while (0)
#define CORO_RESCHEDULE do { g_scheduler->reschedule(); CORO_SLEEP(1); } while (0)

/**
 * Stop the currently running coroutine and all calling coroutines.
 *
 * This sets _sleep to -1 rather than 0 so that the context doesn't get
 * deleted by CoroContextHolder, since we want CORO_INVOKE_ARGS to
 * propogate the _sleep value and return immediately (the scheduler will
 * then delete the entire coroutine's state, including all subcontexts).
 */
#define CORO_KILL_SELF() \
		do { if (&coroParam != &nullContext) { coroParam->_sleep = -1; } return; } while (0)


/**
 * This macro is to be used in conjunction with CORO_INVOKE_ARGS and
 * similar macros for calling coroutines-enabled subroutines.
 */
#define CORO_SUBCTX   coroParam->_subctx

/**
 * Invoke another coroutine.
 *
 * If the subcontext still exists after the coroutine is invoked, it has
 * either yielded/slept or killed itself, and so we copy the _sleep value
 * to our own context and return (execution will continue at the case
 * statement below, where we loop and call the coroutine again).
 * If the subcontext is null, the coroutine ended normally, and we can
 * simply break out of the loop and continue execution.
 *
 * @param subCoro	name of the coroutine-enabled function to invoke
 * @param ARGS		list of arguments to pass to subCoro
 *
 * @note ARGS must be surrounded by parentheses, and the first argument
 *       in this list must always be CORO_SUBCTX. For example, the
 *       regular function call
 *          myFunc(a, b);
 *       becomes the following:
 *          CORO_INVOKE_ARGS(myFunc, (CORO_SUBCTX, a, b));
 */
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
			} while (1);\
		} while (0)

/**
 * Invoke another coroutine. Similar to CORO_INVOKE_ARGS,
 * but allows specifying a return value which is returned
 * if invoked coroutine yields (thus causing the current
 * coroutine to yield, too).
 */
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
			} while (1);\
		} while (0)

/**
 * Convenience wrapper for CORO_INVOKE_ARGS for invoking a coroutine
 * with no parameters.
 */
#define CORO_INVOKE_0(subCoroutine) \
			CORO_INVOKE_ARGS(subCoroutine,(CORO_SUBCTX))

/**
 * Convenience wrapper for CORO_INVOKE_ARGS for invoking a coroutine
 * with one parameter.
 */
#define CORO_INVOKE_1(subCoroutine, a0) \
			CORO_INVOKE_ARGS(subCoroutine,(CORO_SUBCTX,a0))

/**
 * Convenience wrapper for CORO_INVOKE_ARGS for invoking a coroutine
 * with two parameters.
 */
#define CORO_INVOKE_2(subCoroutine, a0,a1) \
			CORO_INVOKE_ARGS(subCoroutine,(CORO_SUBCTX,a0,a1))

/**
 * Convenience wrapper for CORO_INVOKE_ARGS for invoking a coroutine
 * with three parameters.
 */
#define CORO_INVOKE_3(subCoroutine, a0,a1,a2) \
			CORO_INVOKE_ARGS(subCoroutine,(CORO_SUBCTX,a0,a1,a2))

//@}

} // End of namespace Tinsel

#endif		// TINSEL_COROUTINE_H
