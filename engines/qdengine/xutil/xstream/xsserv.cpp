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

#include "qdengine/qd_precomp.h"


namespace QDEngine {

static const char *seekMSG    = "BAD SEEK";
static const char *flushMSG   = "FLUSH FILE BUFFERS ERROR";
static const char *sizeMSG    = "FILE SIZE CALCULATION ERROR";

long XStream::seek(long offset, int dir) {
	long ret;
	if (extSize != -1) {
		switch (dir) {
		case XS_BEG:
			ret = SetFilePointer(handler, extPos + offset, 0, dir) - extPos;
			break;
		case XS_END:
			ret = SetFilePointer(handler, extPos + extSize - offset - 1, 0, XS_BEG) - extPos;
			break;
		case XS_CUR:
			ret = SetFilePointer(handler, extPos + pos + offset, 0, XS_BEG) - extPos;
			break;
		}
	} else
		ret = SetFilePointer(handler, offset, 0, dir);
	if (ret == -1L) {
		if (handleErrors_) error("%s", seekMSG);
		else return -1L;
	}
	if (ret >= size() - 1) eofFlag = 1;
	else eofFlag = 0;
	return pos = ret;
}

void XStream::flush() {
	if (!FlushFileBuffers(handler) && handleErrors_)
		error("%s", flushMSG);
}

long XStream::size() const {
	long tmp = extSize;
	if (tmp == -1) {
		tmp = GetFileSize(handler, 0);
		if (tmp == -1L) {
			if (handleErrors_) error("%s", sizeMSG);
			else return -1;
		}
	}
	return tmp;
}
} // namespace QDEngine
