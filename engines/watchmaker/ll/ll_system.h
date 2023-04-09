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

#ifndef WATCHMAKER_LL_SYSTEM_H
#define WATCHMAKER_LL_SYSTEM_H

#include "watchmaker/types.h"
#include "common/stream.h"
#include "common/ptr.h"

namespace Watchmaker {

//------------------------- Memory Functions -------------------------------
void   *t3dMalloc(uint32 n);
void   *t3dCalloc(uint32 n);

template<typename T>
T *t3dMalloc(uint32 num) {
	return static_cast<T *>(t3dMalloc(sizeof(T) * num));
}

template<typename T>
T *t3dCalloc(uint32 num) {
	// HACK: There are a few places where we actually access the first element even when zero-sized
	// those need to be fixed, but for now, let's pad.
	if (num == 0) {
		num++;
	}
	return (T *)t3dCalloc(sizeof(T) * num);
}
void *t3dRealloc(void *p, uint32 additionalBytes);
void t3dFree(void *p);

//------------------------- Time Functions ---------------------------------
void t3dStartTime();
void t3dEndTime();
uint32 t3dReadTime();

bool t3dFastFileInit(const char *name);
void t3dForceNOFastFile(char valore);
int t3dAccessFile(char *name);
bool t3dGetFileDate(uint32 *date, uint32 *time, const char *name);
bool checkFileExists(const Common::String &filename);
Common::SeekableReadStream *resolveFile(const char *path);
Common::SharedPtr<Common::SeekableReadStream> openFile(const Common::String &filename, int offset = 0, int size = -1);

} // End of namespace Watchmaker

#endif // WATCHMAKER_LL_SYSTEM_H
