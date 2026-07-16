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

byte ComfyEngine::scriptReadByte(uint32 pc) {
	uint16 tileIndex = (uint16)(pc / COMFY_TILE_SIZE);
	uint16 tileOffset = (uint16)(pc % COMFY_TILE_SIZE);
	int16 spriteId = (int16)(-(int16)tileIndex - 1);

	SpriteResource *sprite = spriteGetPtr(spriteId);
	uint16 frameId = (uint16)-spriteId;

	if (!sprite || frameId >= _frameCacheEntries.size()) {
		_scriptFault = true;
		return 0;
	}

	SpriteCacheEntry &entry = _frameCacheEntries[frameId];
	uint32 offset = entry.poolOffset + 0x0C + tileOffset;

	if (entry.slotSize == 0xFFFF || offset >= _scenePoolData.size()) {
		_scriptFault = true;
		return 0;
	}

	return _scenePoolData[offset];
}

uint16 ComfyEngine::scriptReadWord(uint32 pc) {
	uint16 tileIndex = (uint16)(pc / COMFY_TILE_SIZE);
	uint16 tileOffset = (uint16)(pc % COMFY_TILE_SIZE);
	int16 spriteId = (int16)(-(int16)tileIndex - 1);

	SpriteResource *sprite = spriteGetPtr(spriteId);
	uint16 frameId = (uint16)-spriteId;

	if (!sprite || frameId >= _frameCacheEntries.size()) {
		_scriptFault = true;
		return 0;
	}

	SpriteCacheEntry &entry = _frameCacheEntries[frameId];
	uint32 offset = entry.poolOffset + 0x0C + tileOffset;

	if (entry.slotSize == 0xFFFF || offset > _scenePoolData.size() || 2 > _scenePoolData.size() - offset) {
		_scriptFault = true;
		return 0;
	}

	return READ_LE_UINT16(&_scenePoolData[offset]);
}

uint32 ComfyEngine::scriptReadDword(uint32 pc) {
	uint16 tileIndex = (uint16)(pc / COMFY_TILE_SIZE);
	uint16 tileOffset = (uint16)(pc % COMFY_TILE_SIZE);
	int16 spriteId = (int16)(-(int16)tileIndex - 1);

	SpriteResource *sprite = spriteGetPtr(spriteId);
	uint16 frameId = (uint16)-spriteId;

	if (!sprite || frameId >= _frameCacheEntries.size()) {
		_scriptFault = true;
		return 0;
	}

	SpriteCacheEntry &entry = _frameCacheEntries[frameId];
	uint32 offset = entry.poolOffset + 0x0C + tileOffset;

	if (entry.slotSize == 0xFFFF || offset > _scenePoolData.size() || 4 > _scenePoolData.size() - offset) {
		_scriptFault = true;
		return 0;
	}

	return READ_LE_UINT32(&_scenePoolData[offset]);
}

uint16 ComfyEngine::scriptReadStringIndex(uint32 pc) {
	uint16 value = scriptReadWord(pc);

	if (value >= 30000) {
		uint32 index = value - 30000;
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

void ComfyEngine::scriptVariableAddTo(uint16 variableIndex, int16 delta) {
	if (variableIndex < _scriptVariables.size())
		_scriptVariables[variableIndex] += delta;
}

void ComfyEngine::scriptVariableCopy(uint16 destination, uint16 source) {
	if (destination < _scriptVariables.size() && source < _scriptVariables.size())
		_scriptVariables[destination] = _scriptVariables[source];
}

void ComfyEngine::scriptVariableSet(uint16 variableIndex, uint16 value) {
	if (variableIndex < _scriptVariables.size())
		_scriptVariables[variableIndex] = value;
}

uint16 ComfyEngine::scriptVariableGet(uint16 variableIndex) {
	return variableIndex < _scriptVariables.size() ? _scriptVariables[variableIndex] : 0;
}

} // End of namespace Comfy
