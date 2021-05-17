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

#ifndef SAGA2_AUDIOERR_H
#define SAGA2_AUDIOERR_H 1

#include "saga2/errors.h"
#include "saga2/errclass.h"

namespace Saga2 {

const int32 audioErrorOffset = 0xE0;

enum soundErrors {
	serrNoError = audioErrorOffset,
	serrOpenFailed,
	serrReadFailed,
	serrNoFreeBuffers,
	serrBufferSizeTooSmall,
	serrPlayError,
	serrCompUnexpectedEOF,
	serrCompNoMagicNum,
	serrCompBadAlign,
	serrCompBadFloat,
	serrCompBadType,
	serrCompUnkType,
	serrCompUnsupType,
	serrCompOddType,
	serrCompNoType,
	serrCompInternal,
	serrCompWriteFailed,
	serrCompPutFailed,
	serrCompReadFailed,
	serrCompEOFInStream,
	serrVersionMismatch,
	serrFATAL,
	serrMaxErr,
};

/* ===================================================================== *
    Audio error classes
 * ===================================================================== */

class AudioError : public gError {
public:
	AudioError(soundErrors errID);
	AudioError(char *msg);
};

class AudioFatal : public AudioError {
public:
	AudioFatal();
	AudioFatal(char *msg);
};


/* ===================================================================== *
    Error macros
 * ===================================================================== */

inline void SegFatal(soundErrors errID) {
	error("Sound error %d", errID);
}

inline void audioFatal(char *msg) {
	error("Sound error %s", msg);
}

char *IDName(long s);

/* ===================================================================== *
    Logging macro
 * ===================================================================== */

#if DEBUG

#include <io.h>
#define AUDIO_LOG_FILE "AUDIOERR.LOG"
#define AUDIO_MAX_LOG_SIZE 16384

inline void audioLog(char *s) {
	FILE *fp = fopen(AUDIO_LOG_FILE, "at");
	if (fp) {
		if (filelength(fileno(fp)) > AUDIO_MAX_LOG_SIZE) {
			fclose(fp);
			remove(AUDIO_LOG_FILE);
			fp = fopen(AUDIO_LOG_FILE, "at");
			fprintf(fp, "LOG CLEARED AT TIME %d\n==========================", gameTime);
		}
		fprintf(fp, "%s\n", s);
		fclose(fp);
	}
}
#endif

} // end of namespace Saga2

#endif
