/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Process scheduler.
 */

#include "tinsel/handle.h"
#include "tinsel/pcode.h"
#include "tinsel/pid.h"
#include "tinsel/polygons.h"
#include "tinsel/sched.h"

#include "common/textconsole.h"
#include "common/util.h"

namespace Tinsel {

//----------------- LOCAL GLOBAL DATA --------------------

// These vars are reset upon engine destruction

static uint32 g_numSceneProcess = 0;
static SCNHANDLE g_hSceneProcess = 0;

static uint32 g_numGlobalProcess = 0;
static PROCESS_STRUC *g_pGlobalProcess = nullptr;


/**************************************************************************\
|***********    Stuff to do with scene and global processes    ************|
\**************************************************************************/

void ResetVarsSched() {
	g_numSceneProcess = 0;
	g_hSceneProcess = 0;

	g_numGlobalProcess = 0;
	g_pGlobalProcess = nullptr;
}

/**
 * The code for for restored scene processes.
 */
static void RestoredProcessProcess(CORO_PARAM, const void *param) {
	CORO_BEGIN_CONTEXT;
		INT_CONTEXT *pic;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	// get the stuff copied to process when it was created
	// FIXME: Code without typedef emits -Wcast-qual GCC warning.
	//        However, adding const casts break compilation with -fpermissive.
	//        Reverted to local typedef for now until this can be avoided.
#if 0
	_ctx->pic = *(INT_CONTEXT **)param;
	//_ctx->pic = *(const INT_CONTEXT **)param;
#else
	typedef INT_CONTEXT *PINT_CONTEXT;
	_ctx->pic = *(const PINT_CONTEXT *)param;
#endif

	_ctx->pic = RestoreInterpretContext(_ctx->pic);
	AttachInterpret(_ctx->pic, CoroScheduler.getCurrentProcess());

	CORO_INVOKE_1(Interpret, _ctx->pic);

	CORO_END_CODE;
}

/**
 * Process Tinsel Process
 */
static void ProcessTinselProcess(CORO_PARAM, const void *param) {
	// FIXME: Code without typedef emits -Wcast-qual GCC warning.
	//        However, adding const casts break compilation with -fpermissive.
	//        Reverted to local typedef for now until this can be avoided.
#if 0
	//INT_CONTEXT **pPic = (INT_CONTEXT **)param;
	const INT_CONTEXT **pPic = (const INT_CONTEXT **)param;
#else
	typedef INT_CONTEXT *PINT_CONTEXT;
	const PINT_CONTEXT *pPic = (const PINT_CONTEXT *)param;
#endif

	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	// get the stuff copied to process when it was created
	CORO_INVOKE_1(Interpret, *pPic);

	CORO_KILL_SELF();
	CORO_END_CODE;
}


/**************************************************************************\
|*****************    Stuff to do with scene processes    *****************|
\**************************************************************************/

/**
 * Called to restore a scene process.
 */
void RestoreSceneProcess(INT_CONTEXT *pic) {
	const PROCESS_STRUC *processes = _vm->_handle->GetProcessData(g_hSceneProcess, g_numSceneProcess);

	for (uint32 i = 0; i < g_numSceneProcess; i++) {
		if (processes[i].hProcessCode == pic->hCode) {
			CoroScheduler.createProcess(PID_PROCESS + i, RestoredProcessProcess,
					 &pic, sizeof(pic));
			break;
		}
	}

	delete[] processes;
}

/**
 * Run a scene process with the given event.
 */
void SceneProcessEvent(CORO_PARAM, uint32 procID, TINSEL_EVENT event, bool bWait, int myEscape,
						bool *result) {
	if (result) *result = false;

	CORO_BEGIN_CONTEXT;
		const PROCESS_STRUC *processes;
		Common::PPROCESS pProc;
		INT_CONTEXT *pic;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->processes = _vm->_handle->GetProcessData(g_hSceneProcess, g_numSceneProcess);
	for (uint32 i = 0; i < g_numSceneProcess; i++) {
		if (_ctx->processes[i].processId == procID) {
			assert(_ctx->processes[i].hProcessCode); // Must have some code to run

			_ctx->pic = InitInterpretContext(GS_PROCESS,
				_ctx->processes[i].hProcessCode,
				event,
				NOPOLY,		// No polygon
				0,			// No actor
				nullptr,	// No object
				myEscape
			);

			if (_ctx->pic) {
				_ctx->pProc = CoroScheduler.createProcess(
					PID_PROCESS + i,
					ProcessTinselProcess,
					&_ctx->pic,
					sizeof(_ctx->pic));
				AttachInterpret(_ctx->pic, _ctx->pProc);				
			}

			break;
		}
	}

	if (bWait && _ctx->pProc)
		CORO_INVOKE_2(WaitInterpret, _ctx->pProc, result);

	delete[] _ctx->processes;
	CORO_END_CODE;
}

/**
 * Kill all instances of a scene process.
 */
void KillSceneProcess(uint32 procID) {
	const PROCESS_STRUC *processes = _vm->_handle->GetProcessData(g_hSceneProcess, g_numSceneProcess);

	for (uint32 i = 0; i < g_numSceneProcess; i++) {
		if (processes[i].processId == procID) {
			CoroScheduler.killMatchingProcess(PID_PROCESS + i, -1);
			break;
		}
	}

	delete[] processes;
}

/**
 * Register the scene processes in a scene.
 */
void SceneProcesses(uint32 numProcess, SCNHANDLE hProcess) {
	g_numSceneProcess = numProcess;
	g_hSceneProcess = hProcess;
}


/**************************************************************************\
|*****************    Stuff to do with global processes    ****************|
\**************************************************************************/

/**
 * Called to restore a global process.
 */
void RestoreGlobalProcess(INT_CONTEXT *pic) {
	uint32 i;		// Loop counter

	for (i = 0; i < g_numGlobalProcess; i++) {
		if (g_pGlobalProcess[i].hProcessCode == pic->hCode) {
			CoroScheduler.createProcess(PID_GPROCESS + i, RestoredProcessProcess,
					 &pic, sizeof(pic));
			break;
		}
	}

	assert(i < g_numGlobalProcess);
}

/**
 * Kill them all (restore game).
 */
void KillGlobalProcesses() {

	for (uint32 i = 0; i < g_numGlobalProcess; ++i)	{
		CoroScheduler.killMatchingProcess(PID_GPROCESS + i, -1);
	}
}

/**
 * Run a global process with the given event.
 */
bool GlobalProcessEvent(CORO_PARAM, uint32 procID, TINSEL_EVENT event, bool bWait, int myEscape) {
	CORO_BEGIN_CONTEXT;
		INT_CONTEXT *pic;
		Common::PPROCESS	pProc;
	CORO_END_CONTEXT(_ctx);

	bool result = false;

	CORO_BEGIN_CODE(_ctx);

	uint32	i;		// Loop counter
	_ctx->pProc= nullptr;

	for (i = 0; i < g_numGlobalProcess; ++i)	{
		if (g_pGlobalProcess[i].processId == procID) {
			assert(g_pGlobalProcess[i].hProcessCode);		// Must have some code to run

			_ctx->pic = InitInterpretContext(GS_GPROCESS,
				g_pGlobalProcess[i].hProcessCode,
				event,
				NOPOLY,			// No polygon
				0,			// No actor
				NULL,			// No object
				myEscape);

			if (_ctx->pic != NULL) {

				_ctx->pProc = CoroScheduler.createProcess(PID_GPROCESS + i, ProcessTinselProcess,
					&_ctx->pic, sizeof(_ctx->pic));
				AttachInterpret(_ctx->pic, _ctx->pProc);
			}
			break;
		}
	}

	if ((i == g_numGlobalProcess) || (_ctx->pic == NULL))
		result = false;
	else if (bWait)
		CORO_INVOKE_ARGS_V(WaitInterpret, false, (CORO_SUBCTX, _ctx->pProc, &result));

	CORO_END_CODE;
	return result;
}

/**
 * Kill all instances of a global process.
 */
void xKillGlobalProcess(uint32 procID) {
	uint32 i;		// Loop counter

	for (i = 0; i < g_numGlobalProcess; ++i) {
		if (g_pGlobalProcess[i].processId == procID) {
			CoroScheduler.killMatchingProcess(PID_GPROCESS + i, -1);
			break;
		}
	}
}

/**
 * Register the global processes list
 */
void GlobalProcesses(uint32 numProcess, byte *pProcess) {
	g_pGlobalProcess = new PROCESS_STRUC[numProcess];
	g_numGlobalProcess = numProcess;
	byte *p = pProcess;

	for (uint i = 0; i < numProcess; ++i, p += 8) {
		g_pGlobalProcess[i].processId = READ_32(p);
		g_pGlobalProcess[i].hProcessCode = READ_32(p + 4);
	}
}

/**
 * Frees the global processes list
 */
void FreeGlobalProcesses() {
	delete[] g_pGlobalProcess;
	g_pGlobalProcess = 0;
	g_numGlobalProcess = 0;
}

} // End of namespace Tinsel
