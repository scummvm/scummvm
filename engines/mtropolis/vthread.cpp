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

#include "mtropolis/coroutines.h"
#include "mtropolis/coroutine_manager.h"
#include "mtropolis/vthread.h"

namespace MTropolis {

VThreadTaskData::VThreadTaskData() {
}

VThreadTaskData::~VThreadTaskData() {
}

VThreadStackChunk::VThreadStackChunk(size_t capacity)
	: _memory(nullptr), _size(capacity), _topFrame(nullptr) {

	_memory = static_cast<byte *>(malloc(capacity));
	if (!_memory)
		error("Out of memory");
}

VThreadStackChunk::VThreadStackChunk(VThreadStackChunk &&other)
	: _memory(other._memory), _size(other._size), _topFrame(other._topFrame) {
	other._memory = nullptr;
	other._size = 0;
	other._topFrame = nullptr;
}

VThreadStackChunk::~VThreadStackChunk() {
	if (_memory)
		free(_memory);
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void VThreadTaskData::debugInit(const char *name) {
	_debugName = name;
}

void VThreadTaskData::debugInspect(IDebugInspectionReport *report) const {
}
#endif

VThread::VThread(ICoroutineManager *coroManager)
	: _numActiveStackChunks(0), _coroManager(coroManager) {
}

VThread::~VThread() {
	while (popFrame()) {
	}
}

VThreadState VThread::step() {
	while (hasTasks()) {
		VThreadStackFrame *frame = _stackChunks[_numActiveStackChunks - 1]._topFrame;

		VThreadState state = frame->data->execute(this);
		if (state != kVThreadReturn)
			return state;
	}

	return kVThreadReturn;
}

bool VThread::hasTasks() const {
	return _numActiveStackChunks > 0;
}

bool VThread::reserveFrameInChunk(VThreadStackChunk *chunk, size_t frameAlignment, size_t frameSize, VThreadStackFrame *&outFramePtr, size_t dataAlignment, size_t dataSize, void *&outDataPtr) {
	VThreadStackFrame *framePtr = nullptr;
	void *dataPtr = nullptr;

	uintptr address = 0;
	size_t bytesAvailable = 0;

	if (chunk->_topFrame) {
		address = reinterpret_cast<uintptr>(chunk->_topFrame);
		bytesAvailable = static_cast<size_t>(reinterpret_cast<const byte *>(chunk->_topFrame) - chunk->_memory);
	} else {
		address = reinterpret_cast<uintptr>(chunk->_memory + chunk->_size);
		bytesAvailable = chunk->_size;
	}

	if (bytesAvailable < dataSize)
		return false;

	bytesAvailable -= dataSize;
	address -= dataSize;

	size_t dataAlignPadding = static_cast<size_t>(dataAlignment % dataAlignment);

	if (bytesAvailable < dataAlignPadding)
		return false;

	bytesAvailable -= dataAlignPadding;
	address -= dataAlignPadding;

	dataPtr = reinterpret_cast<void *>(address);

	if (bytesAvailable < frameSize)
		return false;

	bytesAvailable -= frameSize;
	address -= frameSize;

	size_t frameAlignPadding = static_cast<size_t>(address % frameAlignment);

	if (bytesAvailable < frameAlignPadding)
		return false;

	bytesAvailable -= frameAlignPadding;
	address -= frameAlignPadding;

	framePtr = reinterpret_cast<VThreadStackFrame *>(address);

	chunk->_topFrame = framePtr;

	outDataPtr = dataPtr;
	outFramePtr = framePtr;

	return true;
}

void VThread::reserveFrame(size_t frameAlignment, size_t frameSize, VThreadStackFrame *&outFramePtr, size_t dataAlignment, size_t dataSize, void *&outDataPtr, bool &outIsNewChunk) {
	// See if this fits in the last active chunk
	if (_numActiveStackChunks > 0) {
		VThreadStackChunk &lastChunk = _stackChunks[_numActiveStackChunks - 1];

		if (reserveFrameInChunk(&lastChunk, frameAlignment, frameSize, outFramePtr, dataAlignment, dataSize, outDataPtr)) {
			outIsNewChunk = false;
			return;
		}
	}

	// Didn't fit, this is the first one in the chunk
	size_t requiredSize = (frameAlignment - 1) + (dataAlignment - 1) + frameSize + dataSize;

	if (_numActiveStackChunks >= _stackChunks.size() || _stackChunks[_numActiveStackChunks]._size < requiredSize) {
		// Doesn't fit in the next chunk, deallocate the chunk and all subsequent chunks and reallocate

		const size_t kChunkMinSize = 1024 * 1024;	// 1MB chunks
		size_t chunkSize = requiredSize;
		if (chunkSize < kChunkMinSize)
			chunkSize = kChunkMinSize;

		while (_stackChunks.size() > _numActiveStackChunks)
			_stackChunks.pop_back();

		_stackChunks.push_back(VThreadStackChunk(chunkSize));
	}

	VThreadStackChunk &lastChunk = _stackChunks[_numActiveStackChunks++];

	bool reservedOK = reserveFrameInChunk(&lastChunk, frameAlignment, frameSize, outFramePtr, dataAlignment, dataSize, outDataPtr);
	assert(reservedOK);
	(void)reservedOK;

	outIsNewChunk = true;
}

void VThread::pushCoroutineInternal(CompiledCoroutine **compiledCoroPtr, CoroutineCompileFunction_t compileFunction, bool isVoidReturn, const CoroutineParamsBase &params, const CoroutineReturnValueRefBase &returnValueRef) {
	const CompiledCoroutine *compiledCoro = *compiledCoroPtr;
	if (compiledCoro == nullptr) {
		_coroManager->compileCoroutine(compiledCoroPtr, compileFunction, isVoidReturn);
		compiledCoro = *compiledCoroPtr;
		assert(compiledCoro);
	}

	pushCoroutineFrame(compiledCoro, params, returnValueRef);
}

VThreadTaskData *VThread::pushCoroutineFrame(const CompiledCoroutine *compiledCoro, const CoroutineParamsBase &params, const CoroutineReturnValueRefBase &returnValueRef) {
	const size_t frameAlignment = alignof(VThreadStackFrame);
	size_t dataAlignment = 0;
	size_t dataSize = 0;

	compiledCoro->_getFrameParameters(dataSize, dataAlignment);

	VThreadStackFrame *prevFrame = nullptr;
	if (_numActiveStackChunks > 0)
		prevFrame = _stackChunks[_numActiveStackChunks - 1]._topFrame;

	VThreadStackFrame *framePtr = nullptr;
	void *dataPtr = nullptr;
	bool isNewChunk = false;
	reserveFrame(frameAlignment, sizeof(VThreadStackFrame), framePtr, dataAlignment, dataSize, dataPtr, isNewChunk);

	VThreadStackFrame *frame = new (framePtr) VThreadStackFrame();
	VThreadTaskData *frameData = compiledCoro->_frameConstructor(dataPtr, compiledCoro, params, returnValueRef);

	frame->data = frameData;
	frame->prevFrame = prevFrame;
	frame->isLastInChunk = isNewChunk;

	return frameData;
}

bool VThread::popFrame() {
	if (_numActiveStackChunks == 0)
		return false;

	VThreadStackChunk &lastChunk = _stackChunks[_numActiveStackChunks - 1];
	VThreadStackFrame *topFrame = lastChunk._topFrame;

	VThreadStackFrame *secondFrame = topFrame->prevFrame;
	bool isLastFrameInChunk = topFrame->isLastInChunk;

	if (isLastFrameInChunk) {
		lastChunk._topFrame = nullptr;
		_numActiveStackChunks--;
	} else {
		assert(reinterpret_cast<byte *>(secondFrame) >= lastChunk._memory);
		assert(reinterpret_cast<byte *>(secondFrame) < lastChunk._memory + lastChunk._size);

		lastChunk._topFrame = secondFrame;
	}

	topFrame->data->~VThreadTaskData();
	topFrame->~VThreadStackFrame();

	return true;
}

} // End of namespace MTropolis
