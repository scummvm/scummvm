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
 */

#ifndef COMMON_COROUTINES_H
#define COMMON_COROUTINES_H

#include "common/scummsys.h"
#include "common/util.h"	// for SCUMMVM_CURRENT_FUNCTION
#include "common/list.h"
#include "common/singleton.h"

namespace Common {

/**
 * @defgroup Coroutine support for simulating multi-threading.
 *
 * The following is loosely based on an article by Simon Tatham:
 *   <http://www.chiark.greenend.org.uk/~sgtatham/coroutines.html>.
 * However, many improvements and tweaks have been made, in particular
 * by taking advantage of C++ features not available in C.
 */
//@{

#define CoroScheduler (Common::CoroutineScheduler::instance())


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


/** This is a special constant that can be temporarily used as a parameter to call coroutine-ised
 * from methods from methods that haven't yet been converted to being a coroutine, so code at least
 * compiles correctly. Be aware, though, that if you use this, you will get runtime errors.
 */
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

/** Methods that have been converted to being a coroutine should have this as the first parameter */
#define CORO_PARAM    Common::CoroContext &coroParam


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
	struct CoroContextTag : Common::CoroBaseContext { \
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
		if (&coroParam == &Common::nullContext) assert(!Common::nullContext);\
		if (!x) {coroParam = x = new CoroContextTag();}\
		Common::CoroContextHolder tmpHolder(coroParam);\
		switch (coroParam->_line) { case 0:;

/**
 * End the code section of a coroutine.
 * @see CORO_END_CODE
 */
#define CORO_END_CODE \
			if (&coroParam == &Common::nullContext) { \
				delete Common::nullContext; \
				Common::nullContext = NULL; \
			} \
		}

/**
 * Sleep for the specified number of scheduler cycles.
 */
#define CORO_SLEEP(delay) do {\
			coroParam->_line = __LINE__;\
			coroParam->_sleep = delay;\
			assert(&coroParam != &Common::nullContext);\
			return; case __LINE__:;\
		} while (0)

#define CORO_GIVE_WAY do { CoroScheduler.giveWay(); CORO_SLEEP(1); } while (0)
#define CORO_RESCHEDULE do { CoroScheduler.reschedule(); CORO_SLEEP(1); } while (0)

/**
 * Stop the currently running coroutine and all calling coroutines.
 *
 * This sets _sleep to -1 rather than 0 so that the context doesn't get
 * deleted by CoroContextHolder, since we want CORO_INVOKE_ARGS to
 * propogate the _sleep value and return immediately (the scheduler will
 * then delete the entire coroutine's state, including all subcontexts).
 */
#define CORO_KILL_SELF() \
		do { if (&coroParam != &Common::nullContext) { coroParam->_sleep = -1; } return; } while (0)


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
				assert(&coroParam != &Common::nullContext);\
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
				assert(&coroParam != &Common::nullContext);\
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

/**
 * Convenience wrapper for CORO_INVOKE_ARGS for invoking a coroutine
 * with four parameters.
 */
#define CORO_INVOKE_4(subCoroutine, a0,a1,a2,a3) \
			CORO_INVOKE_ARGS(subCoroutine,(CORO_SUBCTX,a0,a1,a2,a3))



// the size of process specific info
#define	CORO_PARAM_SIZE	32

// the maximum number of processes
#define	CORO_NUM_PROCESS	100
#define CORO_MAX_PROCESSES	100

#define CORO_INFINITE 0xffffffff
#define CORO_INVALID_PID_VALUE 0

typedef void (*CORO_ADDR)(CoroContext &, const void *);

/** process structure */
struct PROCESS {
	PROCESS *pNext;		///< pointer to next process in active or free list
	PROCESS *pPrevious;	///< pointer to previous process in active or free list

	CoroContext state;		///< the state of the coroutine
	CORO_ADDR  coroAddr;	///< the entry point of the coroutine

	int sleepTime;		///< number of scheduler cycles to sleep
	uint32 pid;			///< process ID
	bool waiting;		///< process is currently in a waiting state
	char param[CORO_PARAM_SIZE];	///< process specific info
};
typedef PROCESS *PPROCESS;


/** Event structure */
struct EVENT {
	uint32 pid;
	bool manualReset;
	bool signalled;
};


/**
 * Creates and manages "processes" (really coroutines).
 */
class CoroutineScheduler: public Singleton<CoroutineScheduler> {
public:
	/** Pointer to a function of the form "void function(PPROCESS)" */
	typedef void (*VFPTRPP)(PROCESS *);

private:

	/** list of all processes */
	PROCESS *processList;

	/** active process list - also saves scheduler state */
	PROCESS *active;

	/** pointer to free process list */
	PROCESS *pFreeProcesses;

	/** the currently active process */
	PROCESS *pCurrent;

	/** Auto-incrementing process Id */
	int pidCounter;

	/** Event list */
	Common::List<EVENT *> _events;

#ifdef DEBUG
	// diagnostic process counters
	int numProcs;
	int maxProcs;

	void CheckStack();
#endif

	/**
	 * Called from killProcess() to enable other resources
	 * a process may be allocated to be released.
	 */
	VFPTRPP pRCfunction;

	PROCESS *getProcess(uint32 pid);
	EVENT *getEvent(uint32 pid);
public:

	CoroutineScheduler();
	~CoroutineScheduler();

	void reset();

	#ifdef	DEBUG
	void printStats();
	#endif

	void schedule();
	void rescheduleAll();
	void reschedule(PPROCESS pReSchedProc = NULL);
	void giveWay(PPROCESS pReSchedProc = NULL);
	void waitForSingleObject(CORO_PARAM, int pid, uint32 duration, bool *expired = NULL);
	void waitForMultipleObjects(CORO_PARAM, int nCount, uint32 *pidList, bool bWaitAll, 
			uint32 duration, bool *expired = NULL);
	void sleep(CORO_PARAM, uint32 duration);

	PROCESS *createProcess(uint32 pid, CORO_ADDR coroAddr, const void *pParam, int sizeParam);
	uint32 createProcess(CORO_ADDR coroAddr, const void *pParam, int sizeParam);
	uint32 createProcess(CORO_ADDR coroAddr, const void *pParam);
	void killProcess(PROCESS *pKillProc);

	PROCESS *getCurrentProcess();
	int getCurrentPID() const;
	int killMatchingProcess(uint32 pidKill, int pidMask = -1);

	void setResourceCallback(VFPTRPP pFunc);

	/* Event methods */
	uint32 createEvent(bool bManualReset, bool bInitialState);
	void closeEvent(uint32 pidEvent);
	void setEvent(uint32 pidEvent);
	void resetEvent(uint32 pidEvent);
	void pulseEvent(uint32 pidEvent);
};

//@}

} // end of namespace Common

#endif		// COMMON_COROUTINES_H
