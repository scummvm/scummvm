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

#include "comfy/comfy.h"

#include "common/endian.h"

#include <cstdarg>

namespace Comfy {

bool ComfyEngine::scriptHasRange(uint32 pc, uint32 width) {
	uint32 tileOffset = pc % COMFY_TILE_SIZE;
	uint32 tileBase = pc - tileOffset;
	uint32 tileSize = tileBase < _picDataSize ? MIN<uint32>(COMFY_TILE_SIZE, _picDataSize - tileBase) : 0;
	if (tileBase > _comfyObjData.size() || tileSize > _comfyObjData.size() - tileBase ||
			tileOffset > tileSize || width > tileSize - tileOffset) {
		_scriptFault = true;
		return false;
	}

	return true;
}

byte ComfyEngine::scriptReadByte(uint32 pc) {
	return scriptHasRange(pc, 1) ? _comfyObjData[pc] : 0;
}

uint16 ComfyEngine::scriptReadWord(uint32 pc) {
	return scriptHasRange(pc, 2) ? READ_LE_UINT16(&_comfyObjData[pc]) : 0;
}

uint32 ComfyEngine::scriptReadDword(uint32 pc) {
	return scriptHasRange(pc, 4) ? READ_LE_UINT32(&_comfyObjData[pc]) : 0;
}

uint16 ComfyEngine::scriptReadStringIndex(uint32 pc) {
	uint16 value = scriptReadWord(pc);
	if (value >= 0x7530) {
		uint32 index = value - 0x7530;
		if (index < _stringTable.size())
			value = _stringTable[index];
	}

	return value;
}

uint32 ComfyEngine::scriptReadArgs(uint32 pc, uint16 fallbackActor, const char *format, ...) {
	va_list args;
	va_start(args, format);
	for (char type = *format; type; type = *++format) {
		if (type == 'C') {
			byte *value = va_arg(args, byte *);
			if (value)
				*value = scriptReadByte(pc);

			pc++;
		} else if (type == 'I' || type == 'N') {
			uint16 *value = va_arg(args, uint16 *);
			if (value)
				*value = type == 'N' ? scriptReadStringIndex(pc) : scriptReadWord(pc);

			pc += 2;
		} else if (type == 'L') {
			uint32 *value = va_arg(args, uint32 *);
			if (value)
				*value = scriptReadDword(pc);

			pc += 4;
		} else if (type == 'O') {
			Actor **value = va_arg(args, Actor **);
			if (value)
				*value = actorResolve(scriptReadWord(pc), fallbackActor);

			pc += 2;
		}
	}

	va_end(args);
	return pc;
}


} // End of namespace Comfy
