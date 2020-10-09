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

#ifndef COMMON_COROUTINES_H
#define COMMON_COROUTINES_H

#include "common/scummsys.h"
#include "common/util.h"    // for SCUMMVM_CURRENT_FUNCTION
#include "common/list.h"
#include "common/singleton.h"

namespace Common {

/**
 * @defgroup common_coroutine Coroutine support for simulating multi-threading
 * @ingroup common
 *
 * @brief  The following implementation is loosely based on an article by Simon Tatham:
 *         @linkCoroutine.
 *         However, many improvements and tweaks have been made, in particular
 *         by taking advantage of C++ features not available in C.
 *
 * @{
 */

#define CoroScheduler (Common::CoroutineScheduler::instance())


// Enable this macro to enable some debugging support in the coroutine code.
//#define COROUTINE_DEBUG

/**
 * The core of any coroutine context which captures the 'state' of a coroutine.
 * Private use only.
 */
struct CoroBaseContext {
	int _line;
	int _sleep;
	CoroBaseContext *_subctx;
#ifdef COROUTINE_DEBUG
	const char *_funcName;
#endif
	/**
	 * Create a coroutine context.
	 */
	CoroBaseContext(const char *func);

	/**
	 * Destructor for coroutine context.
	 */
	virtual ~CoroBaseContext();
};

typedef CoroBaseContext *CoroContext;


/** This is a special constant that can be temporarily used as a parameter to call coroutine-ised
 * methods from code that have not yet been converted to being a coroutine, so code at least
 * compiles correctly. Be aware, though, that an error will occur if a coroutine that was passed
 * the nullContext tries to sleep or yield control.
 */
extern CoroContext nullContext;

/**
 * Wrapper class which holds a pointer to a pointer to a CoroBaseContext.
 * Note that the destructor kills the context being held,
 * but ONLY if the _sleep val of that context is zero. This way, a coroutine
 * can just 'return' without freeing the allocated context
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
			_ctx = nullptr;
		}
	}
};

/** Set this as the first parameter for methods that have been converted to being a coroutine. */
#define CORO_PARAM    Common::CoroContext &coroParam


/**
 * Begin the declaration of a coroutine context.
 * This allows declaring variables which are 'persistent' during the
 * lifetime of the coroutine. Example usage:
 * @code
 *  CORO_BEGIN_CONTEXT;
 *    int var;
 *    char *foo;
 *  CORO_END_CONTEXT(_ctx);
 * @endcode
 * It is not possible to initialize variables here, due to the way this
 * macro is implemented. Furthermore, to use the variables declared in
 * the coroutine context, you must access them through the context variable
 * name that was specified as a parameter to @c CORO_END_CONTEXT, e.g.
 *   _ctx->var = 0;
 *
 * @see CORO_END_CONTEXT
 *
 * @note A 'DUMMY' variable is declared to allow the user to specify an 'empty'
 * context, and so that compilers do not complain about ";" following the macro.
 */
#define CORO_BEGIN_CONTEXT  \
	struct CoroContextTag : Common::CoroBaseContext { \
 CoroContextTag() : CoroBaseContext(SCUMMVM_CURRENT_FUNCTION) { DUMMY = 0; } \
		int DUMMY

/**
 * End the declaration of a coroutine context.
 * @param x Name of the coroutine context.
 * @see CORO_BEGIN_CONTEXT
 */
#define CORO_END_CONTEXT(x)    } *x = (CoroContextTag *)coroParam

/**
 * Begin the code section of a coroutine.
 * @param x Name of the coroutine context.
 */
#define CORO_BEGIN_CODE(x) \
	if (&coroParam == &Common::nullContext) assert(!Common::nullContext); \
	if (!x) { coroParam = x = new CoroContextTag(); } \
	x->DUMMY = 0; \
	Common::CoroContextHolder tmpHolder(coroParam); \
	switch (coroParam->_line) { default: break; case 0:;

/**
 * End the code section of a coroutine.
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
#define CORO_SLEEP(delay) \
	do { \
		coroParam->_line = __LINE__; \
		coroParam->_sleep = delay; \
		assert(&coroParam != &Common::nullContext); \
		return; case __LINE__:; \
	} while (0)

#define CORO_GIVE_WAY do { CoroScheduler.giveWay(); CORO_SLEEP(1); } while (0)
#define CORO_RESCHEDULE do { CoroScheduler.reschedule(); CORO_SLEEP(1); } while (0)

/**
 * Stop the currently running coroutine and all calling coroutines.
 *
 * This sets _sleep to -1 rather than 0 so that the context does not get
 * deleted by CoroContextHolder, since we want @ref CORO_INVOKE_ARGS to
 * propagate the _sleep value and return immediately (the scheduler will
 * then delete the entire coroutine's state, including all subcontexts).
 */
#define CORO_KILL_SELF() \
	do { if (&coroParam != &Common::nullContext) { coroParam->_sleep = -1; } return; } while (0)


/**
 * Use this macro in conjunction with @ref CORO_INVOKE_ARGS and
 * similar macros for calling coroutine-enabled subroutines.
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
 * @param subCoro   Name of the coroutine-enabled function to invoke.
 * @param ARGS      List of arguments to pass to subCoro.
 *
 * @note @p ARGS must be surrounded by parentheses, and the first argument
 *       in this list must always be CORO_SUBCTX. For example, the
 *       regular function call
 *          myFunc(a, b);
 *       becomes the following:
 *          CORO_INVOKE_ARGS(myFunc, (CORO_SUBCTX, a, b));
 */
#define CORO_INVOKE_ARGS(subCoro, ARGS) \
	do { \
		coroParam->_line = __LINE__; \
		coroParam->_subctx = 0; \
		do { \
			subCoro ARGS; \
			if (!coroParam->_subctx) break; \
			coroParam->_sleep = coroParam->_subctx->_sleep; \
			assert(&coroParam != &Common::nullContext); \
			return; case __LINE__:; \
		} while (1); \
	} while (0)

/**
 * Invoke another coroutine. Similar to @ref CORO_INVOKE_ARGS,
 * but allows specifying a return value which is returned
 * if the invoked coroutine yields (thus causing the current
 * coroutine to yield, too).
 */
#define CORO_INVOKE_ARGS_V(subCoro, RESULT, ARGS) \
	do { \
		coroParam->_line = __LINE__; \
		coroParam->_subctx = 0; \
		do { \
			subCoro ARGS; \
			if (!coroParam->_subctx) break; \
			coroParam->_sleep = coroParam->_subctx->_sleep; \
			assert(&coroParam != &Common::nullContext); \
			return RESULT; case __LINE__:; \
		} while (1); \
	} while (0)

/**
 * Convenience wrapper for @ref CORO_INVOKE_ARGS for invoking a coroutine
 * with no parameters.
 */
#define CORO_INVOKE_0(subCoroutine) \
	CORO_INVOKE_ARGS(subCoroutine, (CORO_SUBCTX))

/**
 * Convenience wrapper for @ref CORO_INVOKE_ARGS for invoking a coroutine
 * with one parameter.
 */
#define CORO_INVOKE_1(subCoroutine, a0) \
	CORO_INVOKE_ARGS(subCoroutine, (CORO_SUBCTX, a0))

/**
 * Convenience wrapper for CORO_INVOKE_ARGS for invoking a coroutine
 * with two parameters.
 */
#define CORO_INVOKE_2(subCoroutine, a0,a1) \
	CORO_INVOKE_ARGS(subCoroutine, (CORO_SUBCTX, a0, a1))

/**
 * Convenience wrapper for @ref CORO_INVOKE_ARGS for invoking a coroutine
 * with three parameters.
 */
#define CORO_INVOKE_3(subCoroutine, a0,a1,a2) \
	CORO_INVOKE_ARGS(subCoroutine, (CORO_SUBCTX, a0, a1, a2))

/**
 * Convenience wrapper for CORO_INVOKE_ARGS for invoking a coroutine
 * with four parameters.
 */
#define CORO_INVOKE_4(subCoroutine, a0,a1,a2,a3) \
	CORO_INVOKE_ARGS(subCoroutine, (CORO_SUBCTX, a0, a1, a2, a3))



/** Size of process-specific information. */
#define CORO_PARAM_SIZE 32

/** Maximum number of processes. */
#define CORO_NUM_PROCESS    100
#define CORO_MAX_PROCESSES  100
#define CORO_MAX_PID_WAITING 5

#define CORO_INFINITE 0xffffffff
#define CORO_INVALID_PID_VALUE 0

/** Coroutine parameter for methods converted to coroutines. */
typedef void (*CORO_ADDR)(CoroContext &, const void *);

/** process structure */
struct PROCESS {
	PROCESS *pNext;     ///< Pointer to the next process in an active or free list.
	PROCESS *pPrevious; ///< Pointer to the previous process in an active or free list.

	CoroContext state;      ///< State of the coroutine.
	CORO_ADDR  coroAddr;    ///< Entry point of the coroutine.

	int sleepTime;      ///< Number of scheduler cycles to sleep.
	uint32 pid;         ///< Process ID.
	uint32 pidWaiting[CORO_MAX_PID_WAITING];    ///< Process ID(s) that the process is currently waiting on.
	char param[CORO_PARAM_SIZE];    ///< Process-specific information.
};
typedef PROCESS *PPROCESS;


/** Event structure. */
struct EVENT {
	uint32 pid;
	bool manualReset;
	bool signalled;
	bool pulsing;
};


/**
 * Create and manage "processes" (really coroutines).
 */
class CoroutineScheduler : public Singleton<CoroutineScheduler> {
public:
	/** Pointer to a function of the form "void function(PPROCESS)" */
	typedef void (*VFPTRPP)(PROCESS *);

private:
	friend class Singleton<CoroutineScheduler>;

	/**
	 * Constructor.
	 */
	CoroutineScheduler();

	/**
	 * Destructor.
	 */
	~CoroutineScheduler();


	/** List of all processes. */
	PROCESS *processList;

	/** Active process list. Saves scheduler state. */
	PROCESS *active;

	/** Pointer to the free process list. */
	PROCESS *pFreeProcesses;

	/** Currently active process. */
	PROCESS *pCurrent;

	/** Auto-incrementing process ID. */
	int pidCounter;

	/** Event list. */
	Common::List<EVENT *> _events;

#ifdef DEBUG
    /** Diagnostic process counters. */
	int numProcs;
	int maxProcs;

	/**
	 * Check both the active and free process list to ensure that all links are valid,
	 * and that no processes have been lost.
	 */
	void checkStack();
#endif

	/**
	 * Called from killProcess() to enable other resources
	 * a process may be allocated to be released.
	 */
	VFPTRPP pRCfunction;

	PROCESS *getProcess(uint32 pid);
	EVENT *getEvent(uint32 pid);
public:
	/**
	 * Kill all processes and place them on the free list.
	 */
	void reset();

#ifdef DEBUG
	/**
	 * Show the maximum number of processes used at once.
	 */
	void printStats();
#endif

	/**
	 * Give all active processes a chance to run.
	 */
	void schedule();

	/**
	 * Reschedule all processes to run again this tick.
	 */
	void rescheduleAll();

	/**
	 * If the specified process has already run on this tick, make it run
	 * again on the current tick.
	 */
	void reschedule(PPROCESS pReSchedProc = nullptr);

	/**
	 * Move the specified process to the end of the dispatch queue
	 * allowing it to run again within the current game cycle.
	 * @param pReSchedProc     The process to move.
	 */
	void giveWay(PPROCESS pReSchedProc = nullptr);

	/**
	 * Continously make a given process wait for another process to finish or event to signal.
	 *
	 * @param pid           Process/Event identifier.
	 * @param duration      Duration in milliseconds.
	 * @param expired       If specified, set to true if the delay period expired.
	 */
	void waitForSingleObject(CORO_PARAM, int pid, uint32 duration, bool *expired = nullptr);

	/**
	 * Continously make a given process wait for given processes to finish or events to be set.
	 *
	 * @param nCount        Number of IDs being passed.
	 * @param pidList       List of process IDs to wait for.
	 * @param bWaitAll      Whether to wait for all or any of the processes/events.
	 * @param duration      Duration in milliseconds.
	 * @param expired       Set to true if the delay period expired.
	 */
	void waitForMultipleObjects(CORO_PARAM, int nCount, uint32 *pidList, bool bWaitAll,
	                            uint32 duration, bool *expired = nullptr);

	/**
	 * Make the active process sleep for the given duration in milliseconds.
	 *
	 * @param duration      Duration in milliseconds
	 * @remarks     This duration is not precise, since it relies on the frequency the
	 *              scheduler is called.
	 */
	void sleep(CORO_PARAM, uint32 duration);

	/**
	 * Create a new process.
	 *
	 * @param pid           Process identifier.
	 * @param coroAddr      Coroutine start address.
	 * @param pParam        Process-specific information.
	 * @param sizeParam     Size of the process-specific information.
	 */
	PROCESS *createProcess(uint32 pid, CORO_ADDR coroAddr, const void *pParam, int sizeParam);

	/**
	 * Create a new process with an auto-incrementing Process ID.
	 *
	 * @param coroAddr      Coroutine start address.
	 * @param pParam        Process-specific information.
	 * @param sizeParam     Size of process-specific information.
	 */
	uint32 createProcess(CORO_ADDR coroAddr, const void *pParam, int sizeParam);

	/**
	 * Create a new process with an auto-incrementing Process ID and a single pointer parameter.
	 *
	 * @param coroAddr      Coroutine start address.
	 * @param pParam        Process-specific information.
	 */
	uint32 createProcess(CORO_ADDR coroAddr, const void *pParam);

	/**
	 * Kill the specified process.
	 *
	 * @param pKillProc     The process to kill.
	 */
	void killProcess(PROCESS *pKillProc);

	/**
	 * Return a pointer to the currently running process.
	 */
	PROCESS *getCurrentProcess();

	/**
	 * Return the process identifier of the currently running process.
	 */
	int getCurrentPID() const;

	/**
	 * Kill any process matching the specified PID. The current
	 * process cannot be killed.
	 *
	 * @param pidKill       Process identifier of the process to kill.
	 * @param pidMask       Mask to apply to process identifiers before comparison.
	 * @return      The number of processes killed.
	 */
	int killMatchingProcess(uint32 pidKill, int pidMask = -1);

	/**
	 * Set pointer to a function to be called by killProcess().
	 *
	 * May be called by a resource allocator. The function supplied is
	 * called by killProcess() to allow the resource allocator to free
	 * resources allocated to the dying process.
	 *
	 * @param pFunc         Function to be called by killProcess().
	 */
	void setResourceCallback(VFPTRPP pFunc);

	/** @name Event methods
	 * @{
	 */
	/**
	 * Create a new event (semaphore) object.
	 *
	 * @param bManualReset      Events needs to be manually reset. Otherwise,
	 *                          events will be automatically reset after a
	 *                          process waits for the event to finish.
	 * @param bInitialState     Specifies whether the event is signalled or not
	 *                          initially.
	 */
	uint32 createEvent(bool bManualReset, bool bInitialState);

	/**
	 * Destroy the given event.
	 * @param pidEvent      Event Process ID.
	 */
	void closeEvent(uint32 pidEvent);

	/**
	 * Set the event.
	 * @param pidEvent      Event Process ID.
	 */
	void setEvent(uint32 pidEvent);

	/**
	 * Reset the event.
	 * @param pidEvent      Event Process ID.
	 */
	void resetEvent(uint32 pidEvent);

	/**
	 * Temporarily set a given event to true, and then run all waiting
	 * processes, allowing any processes waiting on the event to be fired. It
	 * then immediately resets the event again.
	 *
	 * @param pidEvent      Event Process ID.
	 *
	 * @remarks     Should not be run inside of another process.
	 */
	void pulseEvent(uint32 pidEvent);
};

/** @} */

/** @} */

} // end of namespace Common

#endif // COMMON_COROUTINES_H
