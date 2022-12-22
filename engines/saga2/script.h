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
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_SCRIPT_H
#define SAGA2_SCRIPT_H

#include "saga2/objects.h"
#include "saga2/calendar.h"

namespace Saga2 {

typedef int16   ThreadID;

//  Various result codes returned from runScript

enum scriptResult {

	//  Code returned when attempt to run a non-existent script
	kScriptResultNoScript = 0,

	//  Code returned when script was aborted before completion
	kScriptResultAborted,

	//  Code returned when script finished
	kScriptResultFinished,

	//  Script spun off as async thread; no answer available.
	kScriptResultAsync
};

//  Variables specific to a thread

struct scriptCallFrame {

	//  ID of object who's method is being called (which can be the same
	//  as one of the other objects below).
	ObjectID        invokedObject;
	ActiveItemID    invokedTAI;

	//  ID of the objects in the interaction.
	ObjectID        enactor,                // actor who caused interaction
	                directObject,           // the object being acted on
	                indirectObject;         // the object being used
	// with the other one
	ActiveItemID    directTAI,              // the tile activity instance
	                // being used
	                indirectTAI;            // the tile activity instance
	// upon which the object is being
	// used

	int16           responseType;           // used with knowledge package

	int16           methodNum;              // which method being invoked

	//  Misc fields used in passing parameters to scripts.
	int16           idNum;                  // a misc. id number
	int16           value;                  // a misc. parameter value
	TilePoint       coords;                 // a misc. tilepoint

	int16           returnVal;              // return value of script
};

//  Standard return codes from scripts in the "returnVal" field

enum {
	//  Code returned by script when script decides requested
	//  action is not possible, and the calling C-code should
	//  take action to inform user
	kActionResultFailure = 0,

	//  Code returned by script when script completes the action
	//  successfully and C-code should not complete the action
	kActionResultSuccess,

	//  Code returned by script when requested action should complete
	//  the action
	kActionResultNotDone
};

//  Method used to refer to a SAGA object

struct SegmentRef {
	uint16              segment;            // segment / resource number
	uint16              offset;             // offset within segment
};

//  Segment numbers of "builtin" SAGA data structures,
//  such as actors and TAGS

enum builtinTypes {
	kBuiltinTypeObject = -1,
	kBuiltinTypeTAG = -2,
	kBuiltinAbstract = -3,
	kBuiltinTypeMission = -4
};

/* ===================================================================== *
   SAGA management functions
 * ===================================================================== */

//  Load the SAGA data segment from the resource file
void initSAGADataSeg();

void saveSAGADataSeg(Common::OutSaveFile *outS);
void loadSAGADataSeg(Common::InSaveFile *in);

//  Dispose of the SAGA data segment -- do nothing
inline void cleanupSAGADataSeg() {}

/* ===================================================================== *
   Thread management functions
 * ===================================================================== */

class Thread;

//  Initialize the SAGA thread list
void initSAGAThreads();

void saveSAGAThreads(Common::OutSaveFile *outS);
void loadSAGAThreads(Common::InSaveFile *in, int32 chunkSize);

//  Dispose of the active SAGA threads
void cleanupSAGAThreads();

//  Dispose of an active SAGA thread
void deleteThread(Thread *p);

void newThread(Thread *p, ThreadID id);

void newThread(Thread *p);

//  Return the ID of the specified SAGA thread
ThreadID getThreadID(Thread *thread);

//  Return a pointer to a SAGA thread, given a thread ID
Thread *getThreadAddress(ThreadID id);

/* ===================================================================== *
   Class Thread: An execution context of a script
 * ===================================================================== */

//  A script task is called a thread

scriptResult runMethod(
	uint16          scriptClassID,      // which script class
	int16           bType,              // builtin type
	uint16          index,              // object index
	uint16          methodNum,          // method number to call
	scriptCallFrame &args);

class Thread {

	friend char *STRING(int strNum);

	friend scriptResult runScript(uint16 exportEntryNum, scriptCallFrame &args);

	friend void wakeUpThread(ThreadID, int16);

public:
	SegmentRef      _programCounter;         // current PC location

	uint8           *_stackPtr;              // current stack location
	byte            *_codeSeg;                // base of current data segment
//					*_stringBase;         // base of string resource

	uint8           *_stackBase;             // base of module stack

	enum threadFlags {
		kTFWaiting     = (1 << 0),             // thread waiting for event
		kTFFinished    = (1 << 1),             // thread finished normally
		kTFAborted     = (1 << 2),             // thread is aborted
		kTFExtended    = (1 << 3),             // this is an extended sequence
		kTFExpectResult = (1 << 4),            // script is expecting result on stack
		kTFSynchronous = (1 << 5),             // when this bit is set this thread will
		// run until it is finished or this bit
		// is cleared

		kTFAsleep      = (kTFWaiting | kTFFinished | kTFAborted)
	};

	int16           _stackSize,              // allocated size of stack
	                _flags,                  // execution flags
	                _framePtr,               // pointer to call frame
	                _returnVal;              // return value from ccalls

	bool            _valid;

	//  Various signals that a script can wait upon
	enum WaitTypes {
		kWaitNone = 0,                       // waiting for nothing
		kWaitDelay,                          // waiting for a timer
		kWaitFrameDelay,                     // waiting for frame count
		kWaitOther,                          // waiting for to be awoken
		kWaitTagSemaphore                    // waiting for a tag semaphore

//		kWaitSpeech,                         // waiting for speech to finish
//		kWaitDialogEnd,                      // waiting for my dialog to finish
//		kWaitDialogBegin,                    // waiting for other dialog to finish
//		kWaitWalk,                           // waiting to finish walking
//		kWaitRequest,                        // a request is up
	};

	WaitTypes  _waitType;               // what we're waiting for
	union {
		Alarm       _waitAlarm;              // for time-delay
		FrameAlarm  _waitFrameAlarm;         // for frame count delay
		ActiveItem *_waitParam;              // for other waiting
	};

	scriptCallFrame _threadArgs;             // arguments from C to thread

	//  For 'cfunc' member functions, the address of the object who's
	//  member function is being invoked.
	void            *_thisObject;
	uint16          _argCount;               // number of args to cfunc

	//  Constructor
	Thread(uint16 segNum, uint16 segOff, scriptCallFrame &args);

	//  Constructor -- reconstruct from archive buffer
	Thread(void **buf);

	Thread(Common::SeekableReadStream *stream, ThreadID id);

	//  Destructor
	~Thread();

	//  Return the number of bytes need to archive this thread in an
	//  arhive buffer
	int32 archiveSize();

	//  Create an archive of this thread in an archive buffer
	void *archive(void *buf);

	void write(Common::MemoryWriteStreamDynamic *out);

	//  Dispatch all asynchronous threads
	static void dispatch();

	//  Intepret a single thread
	scriptResult run();

	//  Tells thread to wait for an event
	void waitForEvent(WaitTypes wt, ActiveItem *param) {
		_flags |= kTFWaiting;
		_waitType = wt;
		_waitParam = param;
	}

	//  Convert to extended script, and back to synchonous script
	void setExtended();
	void clearExtended();

	bool interpret();
private:
	uint8 *strAddress(int strNum);

};

const int               maxTimeSlice = 16,  // max instructions per call
                        kStackSize = 512;    // thread stack size

/* ============================================================================ *
   C-Function dispatch table
 * ============================================================================ */

typedef int16       C_Call(int16 *);

struct CallTable {
	C_Call          **table;
	uint16          numEntries;
	uint16          classID;
};

extern CallTable    globalCFuncs,
       actorCFuncs,
       tagCFuncs,
       missionCFuncs;

//extern C_Call     *ccall_table[];
//extern int16      ccall_count;

/* ===================================================================== *
   Externals
 * ===================================================================== */

extern Thread           *thisThread;        // task queue

//  Thread control
//void killThread( Thread *th );

/*
void wakeUpActorThread(WaitTypes wakeupType, void *obj);
void wakeUpThreads(WaitTypes wakeupType);
void wakeUpThreadsDelayed(WaitTypes wakeupType, int newdelay);
void abortObjectThreads(Thread *keep, uint16 id);
bool abortAllThreads(void);
*/

//  Run a script function
scriptResult runScript(uint16 exportEntryNum, scriptCallFrame &args);

//  Run a script class method.
scriptResult runObjectMethod(
    ObjectID id, uint16 methodNum, scriptCallFrame &args);

scriptResult runTagMethod(
    uint16 tagNum, uint16 methodNum, scriptCallFrame &args);


struct ResImportTable {
	int16           deadActorProto,
	                reserved[2];
	int16           EXP_spellEffect_CreateFireWisp,
	                EXP_spellEffect_CreateWindWisp,
	                EXP_spellEffect_CreateWraith,
	                EXP_spellEffect_TeleportToShrine,
	                EXP_spellEffect_Rejoin,
	                EXP_spellEffect_Timequake,
	                EXP_spellEffect_CreateFood;
};

extern ResImportTable       *resImports;

} // end of namespace Saga2

#endif
