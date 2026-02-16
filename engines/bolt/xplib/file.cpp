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

#include "bolt/bolt.h"
#include "bolt/xplib/xplib.h"

namespace Bolt {

void XpLib::fileError(const char *message) {
}

int32 XpLib::createFile(const char *fileName) {
	return int32();
}

void XpLib::deleteFile(const char *fileName) {
}

int32 XpLib::openFile(const char *fileName, short flags) {
	return int32();
}

void XpLib::closeFile(int32 handle) {
}

bool XpLib::readFile(int32 handle, void *buffer, uint32 *size) {
	return false;
}

bool XpLib::setFilePos(int32 handle, uint32 offset, int16 origin) {
	return false;
}

void *XpLib::allocMem(uint32 size) {
	void *result = malloc(size);
	if (!result)
		error("XpLib::allocMem(): Not enough memory");

	return result;
}

void *XpLib::tryAllocMem(uint32 size) {
	void *result = (void *)malloc(size);
	if (!result)
		warning("XpLib::tryAllocMem(): Couldn't allocate memory, returning nullptr");

	return result;
}

void XpLib::freeMem(void *mem) {
	free(mem);
}

} // End of namespace Bolt
