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
#include "saga2/messager.h"

namespace Saga2 {

int16 userDialog(char *title, char *msg, char *btnMsg1,
                 char *btnMsg2,
                 char *btnMsg3);


#ifndef TALLOC
#define TALLOC(s,m) malloc(s)
#endif

size_t Messager::va(char *format, va_list argptr) {
	if (enabled) {
		char tempBuf[ 256 ];
		size_t size;

		size = vsprintf((char *) tempBuf, format, argptr);

		if (size) {
			if (tempBuf[size - 1] != '\n') {
				tempBuf[size++] = '\n';
				tempBuf[size] = '\0';
			}
			return dumpit(tempBuf, size);
		}
	}
	return 0;
}

size_t Messager::operator()(char *format, ...) {
	if (enabled) {
		size_t size;
		va_list argptr;

		va_start(argptr, format);
		size = va(format, argptr);
		va_end(argptr);
		return size;
	}
	return 0;
}

int FastLogMessager::dumpit(char *s, size_t size) {
#if 0
	char timeBuf[32];
	if (logFile) {
		if (maxSize > 0 &&
		        (size_t) filelength(fileno(logFile)) > maxSize) {
			fclose(logFile);
			remove(logFileName);
			logFile = fopen(logFileName, "at+");
			fprintf(logFile, "LOG CLEARED \n==========================");
		}
		if (flags & logTimeStamp) {
			_strtime(timeBuf);
			fprintf(logFile, "[%s] ", timeBuf);
		}
		return fwrite(s, 1, size, logFile);
	}
#endif
	warning("STUB: dumpit");
	return 0;
}

FastLogMessager::FastLogMessager(char *entry, char *filename, size_t sizeLimit, logOpenFlags flg)
	: Messager(entry) {
	flags = flg;
	maxSize = sizeLimit;
	if (flg == logOpenAppend)
		logFile = fopen(filename, "at+");
	else
		logFile = fopen(filename, "wt+");
	enabled = (logFile != NULL);
}

FastLogMessager::FastLogMessager(char *filename, size_t sizeLimit, logOpenFlags flg) {
	flags = flg;
	maxSize = sizeLimit;
	if (flg == logOpenAppend)
		logFile = fopen(filename, "at+");
	else
		logFile = fopen(filename, "wt+");
	enabled = (logFile != NULL);
}

FastLogMessager::~FastLogMessager() {
	if (logFile) {
		fclose(logFile);
	}
}

int SureLogMessager::dumpit(char *s, size_t) {
	warning("STUB: dumpit");
#if 0

	char timeBuf[32];
	FILE *logFile = fopen(logFileName, "at+");
	if (logFile) {
		if (maxSize > 0 &&
		        (size_t) filelength(fileno(logFile)) > maxSize) {
			fclose(logFile);
			remove(logFileName);
			logFile = fopen(logFileName, "at+");
			fprintf(logFile, "LOG CLEARED \n==========================");
		}
		if (flags & logTimeStamp) {
			_strtime(timeBuf);
			fprintf(logFile, "[%s] ", timeBuf);
		}
		size_t l = fprintf(logFile, s);
		fclose(logFile);
		return l;
	}
#endif
	return 0;
}

SureLogMessager::SureLogMessager(char *entry, char *filename, size_t sizeLimit, logOpenFlags flg)
	: Messager(entry) {
	flags = flg;
	strncpy(logFileName, filename, MAX_LOG_NAME_LENGTH);
	maxSize = sizeLimit;
	if (flg & logOpenAppend == 0) {
		FILE *logFile = fopen(filename, "wt+");
		if (logFile) {
			fprintf(logFile, "Log %s opened\n", filename);
			fclose(logFile);
		}
	}
}

SureLogMessager::SureLogMessager(char *filename, size_t sizeLimit, logOpenFlags flg) {
	flags = flg;
	strncpy(logFileName, filename, MAX_LOG_NAME_LENGTH);
	maxSize = sizeLimit;
	if (flg & logOpenAppend == 0) {
		FILE *logFile = fopen(filename, "wt+");
		if (logFile) {
			fprintf(logFile, "Log %s opened\n", filename);
			fclose(logFile);
		}
	}
}

SureLogMessager::~SureLogMessager() {
}



int BufferedTextMessager::dumpit(char *s, size_t l) {
	if (bufPos < bufSiz - 1) {
		size_t copy = MIN(bufSiz - bufPos, l);
		strncpy(dumpText + bufPos, s, copy);
		bufPos += copy;
		dumpText[bufPos] = '\0';
		return copy;
	}
	return 0;
}

BufferedTextMessager::BufferedTextMessager(char *entry, size_t s)
	: Messager(entry) {
	dumpText = (char *) TALLOC(s, memMessagers);
	bufSiz = s;
	bufPos = 0;
}

BufferedTextMessager::BufferedTextMessager(size_t s) {
	dumpText = (char *) TALLOC(s, memMessagers);
	bufSiz = s;
	bufPos = 0;
}

BufferedTextMessager::~BufferedTextMessager() {
	if (dumpText) {
		if (bufPos)
			fprintf(stderr, dumpText);
		if (dumpText) delete dumpText;
		dumpText = NULL;
	}
}


uint16 defaultStatusFX = 468;
uint16 defaultStatusFY = 354;
uint16 blackStatusF = 24;
uint16 heightStatusF = 11;

int StatusLineMessager::dumpit(char *s, size_t size) {
	if (textPort->displayPage != NULL) {
		Rect16          r;

		r.x = atX;
		r.y = atY;
		r.width = atW;
		r.height = heightStatusF;

		textPort->setColor(blackStatusF);
		textPort->fillRect(r);
		textPort->setColor(atColor);
		textPort->setStyle(0);
		textPort->drawTextInBox(s, size, r, textPosLeft, Point16(2, 1));
	}
	return 0;
}

StatusLineMessager::StatusLineMessager(char *entry, int lineno, gDisplayPort *mp, int32 x, int32 y, int32 w, int16 color)
	: Messager(entry) {
	line = lineno;
	textPort = mp;
	atX = (x >= 0 ? x : defaultStatusFX);
	atY = (y >= 0 ? y : defaultStatusFY + line * heightStatusF);
	atW = (w >= 0 ? w : 640 - (defaultStatusFX - 16) - 20);
	atColor = (color >= 0 ? color : line * 16 + 12);
	operator()("Status Line %d", line);
}

StatusLineMessager::StatusLineMessager(int lineno, gDisplayPort *mp, int32 x, int32 y, int32 w, int16 color) {
	line = lineno;
	textPort = mp;
	atX = (x >= 0 ? x : defaultStatusFX);
	atY = (y >= 0 ? y : defaultStatusFY + line * heightStatusF);
	atW = (w >= 0 ? w : 640 - (defaultStatusFX - 16) - 20);
	atColor = (color >= 0 ? color : line * 16 + 12);
	operator()("Status Line %d", line);
}

StatusLineMessager::~StatusLineMessager() {
	operator()("");
}

int DialogMessager::dummy_va_arg_dump(char *s, ...) {
	va_list argptr;
	va_start(argptr, s);
#ifdef GBASE_H
	GRequest(header, s, "_OK", argptr, 0, NULL, 0);
#endif
	va_end(argptr);
	return 0;
}

int DialogMessager::dumpit(char *s, size_t size) {
#ifdef _WIN32
	MessageBox(NULL, s, header, MB_OK);
#else
#if GBASE_H
	dummy_va_arg_dump(s);
#else
	userDialog(header, s, "OK", NULL, NULL);
#endif
#endif
	return size;
}

DialogMessager::DialogMessager(char *entry, char *head)
	: Messager(entry) {
	strncpy(header, head, 31);
	header[31] = '\0';
}

DialogMessager::DialogMessager(char *head) {
	strncpy(header, head, 31);
	header[31] = '\0';
}

DialogMessager::~DialogMessager() {
}


int MonoMessager::dumpit(char *s, size_t t) {
	char t2[82];
	strncpy(t2, s, MIN<size_t>(t, 79));
	t2[MIN<size_t>(t, 79)] = '\0';
	monoprint(t2);
	return t;
}

MonoMessager::MonoMessager(char *entry)
	: Messager(entry) {
}

MonoMessager::MonoMessager(void) {
}

MonoMessager::~MonoMessager() {
}




int TeeMessager::dumpit(char *s, size_t t) {
	for (int i = 0; i < numSplit; i++) {
		if (mSub[i])
			(*mSub[i])(s);
	}
	return t;
}

TeeMessager::TeeMessager(pMessager m1, pMessager m2) {
	for (int i = 0; i < MAX_MESSAGER_TEES; i++)
		mSub[i] = NULL;
	numSplit = 2;
	mSub[0] = m1;
	mSub[1] = m2;
}

TeeMessager::TeeMessager(pMessager m1, pMessager m2, pMessager m3) {
	for (int i = 0; i < MAX_MESSAGER_TEES; i++)
		mSub[i] = NULL;
	numSplit = 3;
	mSub[0] = m1;
	mSub[1] = m2;
	mSub[2] = m3;
}

TeeMessager::TeeMessager(pMessager m1, pMessager m2, pMessager m3, pMessager m4) {
	for (int i = 0; i < MAX_MESSAGER_TEES; i++)
		mSub[i] = NULL;
	numSplit = 4;
	mSub[0] = m1;
	mSub[1] = m2;
	mSub[2] = m3;
	mSub[3] = m4;
}

} // end of namespace Saga2
