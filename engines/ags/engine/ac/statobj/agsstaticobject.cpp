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
 */

//include <string.h>
#include "ags/engine/ac/statobj/agsstaticobject.h"
#include "ags/engine/ac/game.h"
#include "ags/engine/ac/gamestate.h"

namespace AGS3 {

AGSStaticObject GlobalStaticManager;
StaticGame      GameStaticManager;

const char *AGSStaticObject::GetFieldPtr(const char *address, intptr_t offset) {
	return address + offset;
}

void AGSStaticObject::Read(const char *address, intptr_t offset, void *dest, int size) {
	memcpy(dest, address + offset, size);
}

uint8 AGSStaticObject::ReadInt8(const char *address, intptr_t offset) {
	return *(const uint8 *)(address + offset);
}

int16 AGSStaticObject::ReadInt16(const char *address, intptr_t offset) {
	return *(const int16 *)(address + offset);
}

int AGSStaticObject::ReadInt32(const char *address, intptr_t offset) {
	return *(const int *)(address + offset);
}

float AGSStaticObject::ReadFloat(const char *address, intptr_t offset) {
	return *(const float *)(address + offset);
}

void AGSStaticObject::Write(const char *address, intptr_t offset, void *src, int size) {
	memcpy((void *)(const_cast<char *>(address) + offset), src, size);
}

void AGSStaticObject::WriteInt8(const char *address, intptr_t offset, uint8 val) {
	*(uint8 *)(const_cast<char *>(address) + offset) = val;
}

void AGSStaticObject::WriteInt16(const char *address, intptr_t offset, int16 val) {
	*(int16 *)(const_cast<char *>(address) + offset) = val;
}

void AGSStaticObject::WriteInt32(const char *address, intptr_t offset, int val) {
	*(int *)(const_cast<char *>(address) + offset) = val;
}

void AGSStaticObject::WriteFloat(const char *address, intptr_t offset, float val) {
	*(float *)(const_cast<char *>(address) + offset) = val;
}


void StaticGame::WriteInt32(const char *address, intptr_t offset, int val) {
	if (offset == 4 * sizeof(int)) {
		// game.debug_mode
		set_debug_mode(val != 0);
	} else if (offset == 99 * sizeof(int) || offset == 112 * sizeof(int)) {
		// game.text_align, game.speech_text_align
		*(int *)(const_cast<char *>(address) + offset) = ReadScriptAlignment(val);
	} else {
		*(int *)(const_cast<char *>(address) + offset) = val;
	}
}

} // namespace AGS3
