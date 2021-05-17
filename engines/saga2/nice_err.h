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
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_NICE_ERR_H
#define SAGA2_NICE_ERR_H

/* ===================================================================== *
   Includes
 * ===================================================================== */

#include "saga2/errclass.h"
#include "saga2/errors.h"
#include "saga2/messager.h"
#include "saga2/errlist.h"
#include "saga2/localize.h"

/* ===================================================================== *
   Types
 * ===================================================================== */

// ------------------------------------------------------------------
// error codes used by SystemError

namespace Saga2 {

enum configProblem {
	cpNoError = 0,
	cpBadCLArgs,           // Bad command line args

	// Hardware minimums
	cpMinCPU,           // Minimum CPU type
	cpMinCPUSpeed,      // Minimum CPU speed
	cpMinDOSVer,        // Minimum DOS version
	cpInDOSBox,         // Running in a DOS box
	cpInsufPhysRAM,     // Minimum Physical RAM
	cpInsufPhysMemFree, // Minimum Free Physical RAM
	cpInsufVirtRAM,     // Minimum memory
	cpInsufVirtMemFree, // Minimum free memory
	cpNoDPMISupport,    // Minimum memory

	// DOS problems
	cpNoAudioDrivers,   // audio drivers missing
	cpNoDIGAudio,       // no AIL dig driver
	cpNoMDIAudio,       // no AIL midi driver
	cpLowSVGAThruput,   // low frame rate
	cpLowCDROMThruput,  // low CD throughput
	cpNoMouseDriver,    // mouse driver not loaded

	// Win problems
	cpGPHandlerFail,    // couldn't add GP handler
	cpDDrawInitFail,    // direct draw initfailure
	cpDSoundInitFail,   // direct sound init failure
	cpDTimerInitFail,   // direct sound init failure

	// Runtime failures
	cpProcessorExcept,  // GPF et al.

	// Resource file failures
	cpResFileMissing,   // unable to open resource file
	cpResDiskMissing,   // unable to open resource file
	cpVidDiskMissing,   // unable to open video file
	cpNoDIGAudioCheck,       // no AIL dig driver
	cpNoMDIAudioCheck,       // no AIL midi driver

	cpResFileLocked,   // unable to open resource file for read
	cpResFileSeek,     // unable to seek in resource file
	cpResFileRead,     // unable to read resource file
	cpResFileWrite,    // unable to write resource file
	cpResFileClose,     // unable to close resource file

	cpSavFileWrite,    // unable to write save file
	cpSavFileRead,     // unable to read save file

	// Multitasking errors
	cpDDrawReInitFail,  // unable to reinitialize screen

	// Internal
	cpInternal,         // internal game error

	// User cancel
	cpUserAbort,        // user hit break
	cpVideoCompat,      // video card warning
	cpHeapOverflow,     // heap overflow debugging

	// Quiet
	cpExitQuietly,

	maxConfigProblem,   // bounding value
};

// ------------------------------------------------------------------
// SystemError class
//   A gError variant aimed at release versions

class SystemError : public gError {
	// ------------------------------------------------------------------
	// internal types
	enum RetryClasses {
		rcFatal,
		rcClose,
		rcRetry,
		rcYesNo,
		rcIgnore,
	};

	// ------------------------------------------------------------------
	// member data
	configProblem   cp;
	char            msg[256];
	BOOL            notifyFlag;

	static pMessager fatalMessager;

	// ------------------------------------------------------------------
	// internal member functions
	static const char *sysMessage(configProblem prob);
	static RetryClasses getRetry(configProblem prob);

	static bool SystemErrorRetry(char *msg, RetryClasses rc);
	void buildMsg(const char *fmt, va_list vl);


public:
	// ------------------------------------------------------------------
	// external member functions
	static void useHandler(pMessager m) {
		fatalMessager = m;
	}

	static void SystemErrorFatal(SystemError &se) {
		error("%s", se.msg);
	}
	static bool SystemErrorRetry(configProblem errID, const char *notes, ...);
	static void SystemErrorNotify(const char *nmsg, ...);

	SystemError(const char *msg, ...);
	SystemError(configProblem errID, const char *msg, ...);

	void notify(void) {
		if (!notifyFlag) SystemErrorNotify(msg);
		notifyFlag = TRUE;
	}

};

// ------------------------------------------------------------------
// Messagers for errors

// ------------------------------------------------------------------
// For use with unititialized screens

class SimpleErrorMessager : public Messager {
protected:
	int dumpit(char *s, size_t size);

public:
	SimpleErrorMessager();
	~SimpleErrorMessager();

	void *operator new (size_t s) {
		return calloc(1, s);
	}
	void operator delete (void *m) {
		free(m);
	}
};

// ------------------------------------------------------------------
// During init/cleanup

class BufferedErrorMessager : public BufferedTextMessager {
protected:
	int dumpit(char *, size_t);

public:
	BufferedErrorMessager(size_t s);
	~BufferedErrorMessager();

	void *operator new (size_t s) {
		return calloc(1, s);
	}
	void operator delete (void *m) {
		free(m);
	}
};

// ------------------------------------------------------------------
// full screen mode error handling

class GUIErrorMessager : public Messager {
protected:
	int dumpit(char *s, size_t size);

public:
	GUIErrorMessager();
	~GUIErrorMessager();

	void *operator new (size_t s) {
		return calloc(1, s);
	}
	void operator delete (void *m) {
		free(m);
	}
};





extern SequentialErrorList systemErrors;
extern SequentialErrorList programErrors;
#ifdef _WIN32
extern SparseErrorList win32ExceptionList;
extern SparseErrorList directDrawErrorList;
#endif


/* ===================================================================== *
   Prototypes
 * ===================================================================== */

// ------------------------------------------------------------------
// Error descriptions for processor faults
const char *ExceptDescript(unsigned long ExCode);   //, LPEXCEPTION_POINTERS  );

// ------------------------------------------------------------------
// Old system error call
void niceFatal(char *msg, ...);

// ------------------------------------------------------------------
// dialog using no resources / little memory
int16 FTAMessageBox(char *msg, char *btnMsg1, char *btnMsg2);


/* ===================================================================== *
   Inlines & aliases
 * ===================================================================== */

// ------------------------------------------------------------------
// alias to build & throw error
#define systemConfigError throw SystemError

// ------------------------------------------------------------------
// alias to do a retry box
#define retryConfigError SystemError::SystemErrorRetry

// ------------------------------------------------------------------
// alias to do a warning box
#define systemWarning SystemError::SystemErrorNotify

// ------------------------------------------------------------------
// ASSERT variation
#define REQUIRE(requirement,whenfails) if (!requirement) throw SystemError(whenfails,"")

inline void assertAlloc(void *ptr) {
	if (NULL == ptr)
		error(ALLOCATION_ERROR);
}

enum FatalHandlingModes {
	fhmNone     = 0,
	fhmSimple,
	fhmHold,
	fhmGUI,
};

bool SetFatalMode(FatalHandlingModes fhm);


bool FatalErrorFlag(void);

} // end of namespace Saga2

#endif
