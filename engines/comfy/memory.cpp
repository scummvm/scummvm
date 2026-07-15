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
 */

#include "comfy/comfy.h"

namespace Comfy {

uint16 ComfyEngine::sysGetExtMemKB() {
	// The original reads CMOS extended-memory registers. The compatibility layer exposes
	// the 4000 KB configuration that selects the engine's normal high-memory paths.
	return 0x0FA0;
}

uint32 ComfyEngine::memCompactAndCheck(uint32 minimumBytes) {
	uint32 availableBytes = (uint32)sysGetExtMemKB() << 10;
	if (availableBytes < minimumBytes)
		return 0;

	return availableBytes >> 10;
}

bool ComfyEngine::memIsReady() {
	return true;
}

uint32 ComfyEngine::memGetFreeKB() {
	uint32 allocatedBytes = _memAllocTotal + _scenePoolSize + 4 + framebufferBytes() + 4;
	if (!_backgroundFramebuf.empty())
		allocatedBytes += _backgroundFramebuf.size() + 4;

	if (!_animFrameBuffer.empty())
		allocatedBytes += _animFrameBuffer.size() + 4;

	if (!_sceneMemoryBlock.empty())
		allocatedBytes += _sceneMemoryBlock.size() + 4;

	if (!_objectCacheEntries.empty())
		allocatedBytes += _objectCacheEntries.size() * (_engineVersion == 3 ? 8 : 6) + 4;

	if (!_frameCacheEntries.empty())
		allocatedBytes += _frameCacheEntries.size() * (_engineVersion == 3 ? 8 : 6) + 4;

	uint32 extendedKB = sysGetExtMemKB();
	uint32 allocatedKB = allocatedBytes >> 10;
	if (extendedKB <= 0x0400 + allocatedKB)
		return 0x0E;

	return (extendedKB - 0x0400 - allocatedKB) / 2 + 0x0E;
}

uint32 ComfyEngine::memGetFreeKBThunk() {
	return memGetFreeKB();
}

uint32 ComfyEngine::memGetXmsLimit() {
	uint32 limit = 0x7D000;
	if (sysGetExtMemKB() < 0x0FA0)
		limit -= 0x25800;

	return limit;
}

uint32 ComfyEngine::memGetXmsLimitFar() {
	return memGetXmsLimit();
}

int32 ComfyEngine::memAllocTrack(uint16 pages) {
	if (_xmsBlockCount + 1 > COMFY_XMS_HANDLE_CAPACITY)
		return -1;

	uint16 handle = 1;
	while (handle < COMFY_XMS_HANDLE_CAPACITY && _xmsBlocks[handle].data)
		handle++;

	if (handle >= COMFY_XMS_HANDLE_CAPACITY)
		return -3;

	uint32 size = (uint32)pages << 10;
	byte *data = (byte *)malloc(size ? size : 1);
	if (!data)
		return -2;

	memset(data, 0, size);
	_xmsBlocks[handle].data = data;
	_xmsBlocks[handle].size = size;
	_xmsBlockCount++;
	_memAllocTotal += size + 4;
	return handle;
}

int32 ComfyEngine::memAllocTrackFar(uint16 pages) {
	return memAllocTrack(pages);
}

int32 ComfyEngine::memFreeTrack(uint16 handle) {
	if (!handle || handle >= COMFY_XMS_HANDLE_CAPACITY || !_xmsBlocks[handle].data)
		return -1;

	_memAllocTotal -= _xmsBlocks[handle].size + 4;
	free(_xmsBlocks[handle].data);
	_xmsBlocks[handle].data = nullptr;
	_xmsBlocks[handle].size = 0;
	_xmsBlockCount--;
	return 0;
}

int32 ComfyEngine::memFreeTrackFar(uint16 handle) {
	return memFreeTrack(handle);
}

int32 ComfyEngine::xmsCopyToConv(XmsMove &move) {
	byte *source = (byte *)move.sourceMemory;
	byte *destination = move.destinationMemory;
	uint32 sourceSize = UINT32_MAX;
	uint32 destinationSize = UINT32_MAX;

	if (move.sourceHandle) {
		if (move.sourceHandle >= COMFY_XMS_HANDLE_CAPACITY || !_xmsBlocks[move.sourceHandle].data)
			return -1;

		source = _xmsBlocks[move.sourceHandle].data;
		sourceSize = _xmsBlocks[move.sourceHandle].size;
	}

	if (move.destinationHandle) {
		if (move.destinationHandle >= COMFY_XMS_HANDLE_CAPACITY || !_xmsBlocks[move.destinationHandle].data)
			return -1;

		destination = _xmsBlocks[move.destinationHandle].data;
		destinationSize = _xmsBlocks[move.destinationHandle].size;
	}

	if (!source || !destination || move.sourceOffset > sourceSize || move.length > sourceSize - move.sourceOffset ||
			move.destinationOffset > destinationSize || move.length > destinationSize - move.destinationOffset)
		return -1;

	memcpy(destination + move.destinationOffset, source + move.sourceOffset, move.length);
	return 0;
}

int32 ComfyEngine::xmsTransfer(XmsMove &move) {
	return xmsCopyToConv(move);
}

void ComfyEngine::xmsReset() {
	for (uint16 handle = 1; handle < COMFY_XMS_HANDLE_CAPACITY; handle++) {
		if (_xmsBlocks[handle].data)
			memFreeTrack(handle);
	}

	_xmsEnvHandle = 0;
	_xmsHeaderHandle = 0;
	_memAllocTotal = 0;
}

} // End of namespace Comfy
