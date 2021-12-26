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

#include "ags/engine/ac/statobj/ags_static_object.h"
#include "ags/engine/ac/game.h"
#include "ags/engine/ac/game_state.h"
#include "ags/globals.h"

namespace AGS3 {

const char *AGSStaticObject::GetFieldPtr(const char *address, intptr_t offset) {
	return address + offset;
}

void AGSStaticObject::Read(const char *address, intptr_t offset, void *dest, int size) {
	memcpy(dest, address + offset, size);
}

uint8_t AGSStaticObject::ReadInt8(const char *address, intptr_t offset) {
	return *(const uint8_t *)(address + offset);
}

int16_t AGSStaticObject::ReadInt16(const char *address, intptr_t offset) {
	return *(const int16_t *)(address + offset);
}

int32_t AGSStaticObject::ReadInt32(const char *address, intptr_t offset) {
	return *(const int32_t *)(address + offset);
}

float AGSStaticObject::ReadFloat(const char *address, intptr_t offset) {
	return *(const float *)(address + offset);
}

void AGSStaticObject::Write(const char *address, intptr_t offset, void *src, int size) {
	memcpy((void *)(const_cast<char *>(address) + offset), src, size);
}

void AGSStaticObject::WriteInt8(const char *address, intptr_t offset, uint8_t val) {
	*(uint8_t *)(const_cast<char *>(address) + offset) = val;
}

void AGSStaticObject::WriteInt16(const char *address, intptr_t offset, int16_t val) {
	*(int16_t *)(const_cast<char *>(address) + offset) = val;
}

void AGSStaticObject::WriteInt32(const char *address, intptr_t offset, int32_t val) {
	*(int32_t *)(const_cast<char *>(address) + offset) = val;
}

void AGSStaticObject::WriteFloat(const char *address, intptr_t offset, float val) {
	*(float *)(const_cast<char *>(address) + offset) = val;
}


void StaticGame::WriteInt32(const char *address, intptr_t offset, int32_t val) {
	if (offset == 4 * sizeof(int32_t)) {
		set_debug_mode(val != 0);
	} else if (offset == 99 * sizeof(int32_t) || offset == 112 * sizeof(int32_t)) {
		*(int32_t *)(const_cast<char *>(address) + offset) = ReadScriptAlignment(val);
	} else {
		*(int32_t *)(const_cast<char *>(address) + offset) = val;
	}
}


} // namespace AGS3
