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
 */

#include "qdengine/xlibs/xutil/xglobal.h"


namespace QDEngine {

static char* readMSG    = "WRONG READING";
static char* writeMSG   = "WRONG WRITING";

unsigned xsReadBytes = 0;
unsigned xsReadBytesDelta = 0;
unsigned xsWriteBytes = 0;
unsigned xsWriteBytesDelta = 0;

unsigned xsRdWrResolution = 500000U;

void (*xsReadHandler)(unsigned) = NULL;
void (*xsWriteHandler)(unsigned) = NULL;

void xsSetReadHandler(void (*fp)(unsigned), unsigned res) {
	xsReadHandler = fp;
	xsRdWrResolution = res;
	xsReadBytes = 0;
}

void xsSetWriteHandler(void (*fp)(unsigned), unsigned res) {
	xsWriteHandler = fp;
	xsRdWrResolution = res;
	xsWriteBytes = 0;
}

unsigned long XStream::read(void* buf, unsigned long len) {
	unsigned long ret;
	if (!ReadFile(handler, buf, len, &ret, 0))
		if (handleErrors_)
			ErrH.Abort(readMSG, XERR_USER, GetLastError(), fname);
		else {
			ioError_ = true;
			return 0U;
		}
	if (ret < len)
		eofFlag = 1;
	pos += ret;
	if (extSize != -1 && pos >= extSize)
		eofFlag = 1;

	if (xsReadHandler) {
		xsReadBytesDelta += ret;
		if (xsReadBytesDelta >= xsRdWrResolution) {
			xsReadBytes += xsReadBytesDelta;
			xsReadBytesDelta = 0;
			(*xsReadHandler)(xsReadBytes);
		}
	}
	return ret;
}

unsigned long XStream::write(const void* buf, unsigned long len) {
	unsigned long ret;
	if (!WriteFile(handler, buf, len, &ret, 0))
		if (handleErrors_)
			ErrH.Abort(writeMSG, XERR_USER, GetLastError(), fname);
		else {
			ioError_ = true;
			return 0U;
		}
	pos += ret;
	if (xsWriteHandler) {
		xsWriteBytesDelta += ret;
		if (xsWriteBytesDelta >= xsRdWrResolution) {
			xsWriteBytes += xsWriteBytesDelta;
			xsWriteBytesDelta = 0;
			(*xsWriteHandler)(xsWriteBytes);
		}
	}
	return ret;
}
} // namespace QDEngine
