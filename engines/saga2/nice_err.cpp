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
 * aint32 with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#define FORBIDDEN_SYMBOL_ALLOW_ALL // FIXME: Remove

#include "saga2/std.h"
#include "saga2/rmemfta.h"
#include "saga2/cmisc.h"
#include "saga2/fta.h"
#include "saga2/errors.h"
#include "saga2/program.h"
#include "saga2/messager.h"
#include "saga2/display.h"
#include "saga2/localize.h"

namespace Saga2 {

pMessager basicFatal = NULL;
pMessager delayedFatal = NULL;
pMessager activeFatal = NULL;

bool cleaningUp = FALSE;
static bool FatalFlag = FALSE;
static int lasterrno;
void BackToSysPalette(void);

/* ===================================================================== *
   Functions
 * ===================================================================== */

// ------------------------------------------------------------------
// niceFatal is a happy joyful way to tell the user his disk is full

void niceFatal(char *msg, ...) {
	char dump[256];
	va_list argptr;

	va_start(argptr, msg);
	vsprintf(dump, msg, argptr);
	va_end(argptr);

	error("Program ended : %s", dump);
}

int16 FTAMessageBox(char *msg, char *btnMsg1, char *btnMsg2) {
	warning("STUB: FTAMessageBox()");
	return 0;
}

// ------------------------------------------------------------------
// shutdownSave offers a chance to save the game on fatal exits

void shutdownSave(void) {
#if !DEBUG
	cleaningUp = TRUE;
#endif
}

/* ===================================================================== *
   SystemError member functions
 * ===================================================================== */

pMessager SystemError::fatalMessager = NULL;

// ------------------------------------------------------------------
// Constructors

SystemError::SystemError(const char *nmsg, ...) {
	va_list argptr;

	notifyFlag = FALSE;
	cp = cpInternal;
	va_start(argptr, nmsg);
	buildMsg(nmsg, argptr);
	va_end(argptr);
}

SystemError::SystemError(configProblem errID, const char *nmsg, ...) {
	va_list argptr;

	notifyFlag = FALSE;
	cp = errID;
	va_start(argptr, nmsg);
	buildMsg(nmsg, argptr);
	va_end(argptr);
}

#if DEBUG
void SystemError::buildMsg(const char *fmt,  va_list vl)
#else
void SystemError::buildMsg(const char *,  va_list)
#endif
{
#if 0
	char dosErr[256];
	if (errno && (cp >= cpResFileLocked && cp <= cpSavFileRead)) { // unable to read save file
		sprintf(dosErr, "(%s)\n", strerror(errno));
		lasterrno = errno;
		errno = 0;
	} else
		dosErr[0] = '\0';

#if DEBUG
	SureLogMessager slm = SureLogMessager("SYSERRS.LOG", (int16) 0, SureLogMessager::logOpenAppend);
	char tmsg[256];
	vsprintf(tmsg, fmt, vl);
	sprintf(msg, "%s\n%s%s\n", sysMessage(cp), dosErr, tmsg);
	slm("FATAL: %s\n", msg);
#else
	sprintf(msg, "%s\n%s", sysMessage(cp), dosErr);
#endif
	if (getRetry(cp) != rcIgnore)
		if (!notifyFlag) {
			//#if DEBUG
			//  warn(msg);
			//#else
			SystemErrorNotify(msg);
			notifyFlag = TRUE;
			//#endif
		}
#endif
	warning("STUB: SystemError::buildMsg");
}

// ------------------------------------------------------------------
// Code to get the textual version of an error message

#include "saga2/errlist.h"

extern SequentialErrorList programErrors;

const char *SystemError::sysMessage(configProblem prob) {
	ErrText et = programErrors.errMessage(prob);
	return et;
}

// ------------------------------------------------------------------
// Code to get the appropriate action for an error

SystemError::RetryClasses SystemError::getRetry(configProblem prob) {
	switch (prob) {
	case cpMinCPU           :
	case cpMinCPUSpeed      :
	case cpMinDOSVer        :
	case cpInDOSBox         :
	case cpInsufPhysRAM     :
	case cpInsufPhysMemFree :
	case cpInsufVirtRAM     :
	case cpInsufVirtMemFree :
	case cpNoDPMISupport    :
	case cpNoAudioDrivers   :
	case cpNoDIGAudio       :
	case cpNoMDIAudio       :
	case cpNoMouseDriver    :
	case cpLowSVGAThruput   :
	case cpLowCDROMThruput  :
	case cpDDrawInitFail    :
	case cpDSoundInitFail   :
	case cpDTimerInitFail   :
	case cpProcessorExcept  :
	case cpGPHandlerFail    :
	case cpInternal         :
	case cpResFileLocked    :
	case cpResFileSeek      :
	case cpResFileRead      :
	case cpResFileWrite     :
	case cpResFileClose     :
		return rcFatal;

	case cpResFileMissing   :
	case cpResDiskMissing   :
	case cpVidDiskMissing   :
	case cpDDrawReInitFail  :
		return rcRetry;

	case cpUserAbort        :
	case cpVideoCompat      :
	case cpHeapOverflow     :
	case cpNoDIGAudioCheck  :
	case cpNoMDIAudioCheck  :
		return rcYesNo;

	case cpExitQuietly      :
		return rcIgnore;

	default                 :
		return rcFatal;
	}
}

// ------------------------------------------------------------------
// needed for error handling

#ifdef _WIN32
#include "saga2/ftawin.h"
extern HWND hWndMain;
void localCursorOn(void);
void localCursorOff(void);
#else
int textUserDialog(char *, char *msg, char *b1, char *b2, char *b3);
#endif

// ------------------------------------------------------------------
// Simple error handling

SimpleErrorMessager::SimpleErrorMessager() {
}

SimpleErrorMessager::~SimpleErrorMessager() {
}

int SimpleErrorMessager::dumpit(char *s, size_t size) {
#ifdef _WIN32
	MessageBox(hWndMain, s, PROGRAM_FULL_NAME, MB_APPLMODAL | MB_ICONSTOP | MB_OK);
#else
	textUserDialog(PROGRAM_FULL_NAME, s, NULL, NULL, NULL);
#endif
	return size;
}

// ------------------------------------------------------------------
// delayed Simple error handling

int BufferedErrorMessager::dumpit(char *t, size_t s) {
	if (cleaningUp)
		return s;
	return BufferedTextMessager::dumpit(t, s);
}

BufferedErrorMessager::BufferedErrorMessager(size_t s)  :
	BufferedTextMessager(s) {
}

BufferedErrorMessager::~BufferedErrorMessager() {
	if (dumpText) {
		if (bufPos) {
#ifdef _WIN32
			MessageBox(hWndMain, dumpText, PROGRAM_FULL_NAME, MB_APPLMODAL | MB_ICONSTOP | MB_OK);
#else
			textUserDialog(PROGRAM_FULL_NAME, dumpText, NULL, NULL, NULL);
#endif
		}
		bufPos = 0;
	}
}

// ------------------------------------------------------------------
// full screen mode error handling

int GUIErrorMessager::dumpit(char *s, size_t size) {
	FTAMessageBox(s, NULL, "OK");
	return size;
}

GUIErrorMessager::GUIErrorMessager() {
}

GUIErrorMessager::~GUIErrorMessager() {
}

#define errDialog(t,m,b1,b2,b3) FTAMessageBox(m,b1,b2)

#ifdef _WIN32
extern CFTWindow *pWindow;
extern char   TITLE[];
#else
int textUserDialog(char *, char *msg, char *b1, char *b2, char *b3);
int16 userDialog(char *title, char *msg, char *btnMsg1,
                 char *btnMsg2,
                 char *btnMsg3);


// ------------------------------------------------------------------
// in DOS, if Graphics haven't been initialized we still need a Retry
// capability

int textUserDialog(char *, char *msg, char *bt1, char *bt2, char *bt3) {
	int bs = 0;
	char l[5];
	char b1[32];
	char b2[32];
	char b3[32];
	char c = '\0';
	for (int i = 0; i < 5; i++)
		l[i] = '\0';
	if (bt1) strncpy(b1, bt1, 32);
	else b1[0] = '\0';
	if (bt2) strncpy(b2, bt2, 32);
	else b2[0] = '\0';
	if (bt3) strncpy(b3, bt3, 32);
	else b3[0] = '\0';
	if (isalpha(*b1)) l[bs++] = toupper(*b1);
	if (isalpha(*b2)) l[bs++] = toupper(*b2);
	if (isalpha(*b3)) l[bs++] = toupper(*b3);

	fprintf(stderr, "%s\n%s %s %s %s", msg, b1, b2, b3, bs > 1 ? "?" : " ");

	if (strlen(l) < 2 || c == 27)
		return -1;

#if 0
	c = toupper(getch());
#else
	c = 27;
#endif
	warning("STUB: textUserDialog");

	fprintf(stderr, "\n");

#if 0
	while (strlen(l) > 1 && c != 27 && NULL == strchr(l, c))
		c = toupper(getch());
#endif

	return abs(strchr(l, c) - l);
}
#endif

// ------------------------------------------------------------------
// Static member functions to pop up a retry box

bool SystemError::SystemErrorRetry(configProblem errID, const char *nmsg, ...) {
#if 0
	char t[256];
	char t2[256];
	char dosErr[256];
	if (errno && (errID >= cpResFileLocked && errID <= cpSavFileRead)) // unable to read save file
		sprintf(dosErr, "(%s)\n", strerror(errno));
	else
		dosErr[0] = '\0';
	va_list argptr;

	va_start(argptr, nmsg);
	vsprintf(t, nmsg, argptr);
	va_end(argptr);

#if DEBUG
	sprintf(t2, "%s\n%s%s\n", sysMessage(errID), dosErr, t);
#else
	sprintf(t2, "%s\n%s", sysMessage(errID), dosErr);
#endif
	return SystemError::SystemErrorRetry(t2, getRetry(errID));
#endif
	warning("STUB: SystemError::SystemErrorRetry()");
	return false;
}

bool SystemError::SystemErrorRetry(char *nmsg, SystemError::RetryClasses rc) {
#ifdef _WIN32
	int r = 1;
	if (
#if DEBUG
	    0 && (
#endif
	        !displayEnabled(WindowInactive) || !pWindow->m_fullScreen || !displayEnabled(GraphicsInit)
#if DEBUG
	    )
#endif
	) {
		BackToSysPalette();
		switch (rc) {
		case rcRetry:
			r = (IDCANCEL != MessageBox(hWndMain,
			                            nmsg,
			                            PROGRAM_FULL_NAME,
			                            MB_APPLMODAL | MB_ICONSTOP | MB_RETRYCANCEL));
			break;
		case rcYesNo:
			r = (IDYES == MessageBox(hWndMain,
			                         nmsg,
			                         PROGRAM_FULL_NAME,
			                         MB_APPLMODAL | MB_ICONQUESTION | MB_YESNO));
			break;
		default:
			return FALSE;
		}
	} else {
		switch (rc) {
		case rcRetry:
			r = errDialog(PROGRAM_FULL_NAME, nmsg, ERROR_RE_BUTTON, ERROR_CA_BUTTON, NULL);
			break;
		case rcYesNo:
			r = errDialog(PROGRAM_FULL_NAME, nmsg, ERROR_YE_BUTTON, ERROR_NO_BUTTON, NULL);
			break;
		default:
			return FALSE;
		}
	}
	return (r);
#else
	int r = 1;
	if (displayEnabled(GraphicsInit)) {
		switch (rc) {
		case rcRetry:
			r = errDialog(PROGRAM_FULL_NAME, nmsg, ERROR_RE_BUTTON, ERROR_CA_BUTTON, NULL);
			break;
		case rcYesNo:
			r = errDialog(PROGRAM_FULL_NAME, nmsg, ERROR_YE_BUTTON, ERROR_NO_BUTTON, NULL);
			break;
		default:
			return FALSE;
		}
	} else {
		switch (rc) {
		case rcRetry:
			r = textUserDialog(PROGRAM_FULL_NAME, nmsg, ERROR_RE_BUTTON, ERROR_CA_BUTTON, NULL);
			break;
		case rcYesNo:
			r = textUserDialog(PROGRAM_FULL_NAME, nmsg, ERROR_YE_BUTTON, ERROR_NO_BUTTON, NULL);
			break;
		default:
			return FALSE;
		}
	}
	return (r == 0);
#endif
}

void SystemError::SystemErrorNotify(const char *nmsg, ...) {
#ifdef _WIN32
	BackToSysPalette();
#endif
#if DEBUG
	SureLogMessager slm = SureLogMessager("SYSERRS.LOG", (int16) 0, SureLogMessager::logOpenAppend);
#endif
	va_list argptr;
	FatalFlag = TRUE;
#if DEBUG
	va_start(argptr, nmsg);
	slm.va((char *)nmsg, argptr);
	va_end(argptr);
#endif
	va_start(argptr, nmsg);
	if (fatalMessager) {
		fatalMessager->va((char *)nmsg, argptr);
	}
	va_end(argptr);
}

// ------------------------------------------------------------------------
// Fatal error handling

bool FatalErrorFlag(void) {
	return FatalFlag;
}

bool SetFatalMode(FatalHandlingModes fhm) {
	if (basicFatal) delete basicFatal;
	basicFatal = NULL;
	if (delayedFatal) delete delayedFatal;
	delayedFatal = NULL;
	if (activeFatal) delete activeFatal;
	activeFatal = NULL;
	SystemError::useHandler(NULL);
	switch (fhm) {
	case fhmSimple:
		basicFatal = new SimpleErrorMessager;
		if (basicFatal == NULL)
			return FALSE;
		SystemError::useHandler(basicFatal);
		return TRUE;
	case fhmHold:
		delayedFatal = new BufferedErrorMessager(4096);
		if (delayedFatal == NULL)
			return FALSE;
		SystemError::useHandler(delayedFatal);
		return TRUE;
	case fhmGUI:
		activeFatal = new GUIErrorMessager;
		if (activeFatal == NULL)
			return FALSE;
		SystemError::useHandler(activeFatal);
		return TRUE;
	}
	return TRUE;
}

} // end of namespace Saga2
