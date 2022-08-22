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

#include "mtropolis/vthread.h"

namespace MTropolis {

VThreadTaskData::VThreadTaskData() {
}

VThreadTaskData::~VThreadTaskData() {
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void VThreadTaskData::debugInit(const char *name) {
	_debugName = name;
}

void VThreadTaskData::debugInspect(IDebugInspectionReport *report) const {
}
#endif

VThread::VThread() : _faultID(nullptr), _stackUnalignedBase(nullptr), _stackAlignedBase(nullptr), /* _size(0), */_alignment(1), _used(0) {
}

VThread::~VThread() {
	void *dataPtr;
	void *framePtr;
	while (popFrame(dataPtr, framePtr)) {
		static_cast<VThreadStackFrame *>(framePtr)->~VThreadStackFrame();
		static_cast<VThreadTaskData *>(dataPtr)->~VThreadTaskData();
	}

	if (_stackUnalignedBase)
		free(_stackUnalignedBase);
}

VThreadState VThread::step() {
	void *dataPtr;
	void *framePtr;
	while (popFrame(dataPtr, framePtr)) {
		VThreadTaskData *data = static_cast<VThreadTaskData *>(dataPtr);

		const bool isHandling = (data->handlesFault(_faultID));
		static_cast<VThreadStackFrame *>(framePtr)->~VThreadStackFrame();
		if (isHandling) {
			_faultID = nullptr;
			VThreadState state = data->destructAndRunTask();
			if (state != kVThreadReturn)
				return state;
		} else {
			static_cast<VThreadTaskData *>(dataPtr)->~VThreadTaskData();
		}
	}

	return kVThreadReturn;
}

bool VThread::hasTasks() const {
	return _used > 0;
}

void VThread::reserveFrame(size_t size, size_t alignment, void *&outFramePtr, void *&outUnadjustedDataPtr, size_t &outPrevFrameOffset) {
	const size_t frameAlignment = alignof(VThreadStackFrame);
	const size_t frameAlignmentMask = frameAlignment - 1;

	size_t dataAlignmentMask = alignment - 1;

	bool needToReallocate = false;
	if (alignment > _alignment || frameAlignment > _alignment) {
		if ((reinterpret_cast<uintptr>(_stackAlignedBase) & dataAlignmentMask) != 0) {
			needToReallocate = true;
		}
	}

	size_t dataAlignmentPaddingNeeded = (alignment - (_used & dataAlignmentMask));
	if (dataAlignmentPaddingNeeded == alignment)
		dataAlignmentPaddingNeeded = 0;

	size_t offsetOfData = dataAlignmentPaddingNeeded + _used;
	size_t offsetOfEndOfData = offsetOfData + size;
	size_t frameAlignmentPaddingNeeded = (frameAlignment - (offsetOfData & frameAlignmentMask));
	if (frameAlignmentPaddingNeeded == frameAlignment)
		frameAlignmentPaddingNeeded = 0;

	size_t offsetOfFrame = offsetOfEndOfData + frameAlignmentPaddingNeeded;
	size_t offsetOfEndOfFrame = offsetOfFrame + sizeof(VThreadStackFrame);

	size_t offsetOfPrevFrame = 0;
	if (_used > 0)
		offsetOfPrevFrame = _used - sizeof(VThreadStackFrame);

	if (offsetOfEndOfFrame > _used)
		needToReallocate = true;

	if (needToReallocate) {
		size_t maxAlignment = alignment;
		if (maxAlignment < frameAlignment)
			maxAlignment = frameAlignment;

		void *unalignedBase = malloc(offsetOfEndOfFrame + maxAlignment - 1);
		size_t alignPadding = maxAlignment - (reinterpret_cast<uintptr>(unalignedBase) % maxAlignment);
		if (alignPadding == maxAlignment)
			alignPadding = 0;

		void *alignedBase = static_cast<char *>(unalignedBase) + alignPadding;

		// Copy the previous frames
		size_t framePos = 0;
		if (_used > 0) {
			framePos = _used - sizeof(VThreadStackFrame);
#ifdef MTROPOLIS_DEBUG_VTHREAD_STACKS
			VThreadStackFrame *nextFrame = nullptr;
#endif

			while (framePos != 0) {
				VThreadStackFrame *oldFrame = reinterpret_cast<VThreadStackFrame *>(static_cast<char *>(_stackAlignedBase) + framePos);
				size_t dataPos = oldFrame->taskDataOffset;
				VThreadTaskData *oldData = reinterpret_cast<VThreadTaskData *>(static_cast<char *>(_stackAlignedBase) + dataPos);
				size_t nextPos = oldFrame->prevFrameOffset;

				VThreadStackFrame *newFrame = reinterpret_cast<VThreadStackFrame *>(static_cast<char *>(alignedBase) + framePos);
				VThreadTaskData *newData = reinterpret_cast<VThreadTaskData *>(static_cast<char *>(alignedBase) + dataPos);

				// Relocate the frame
				new (newFrame) VThreadStackFrame(*oldFrame);
				oldFrame->~VThreadStackFrame();

				// Relocate the data
				oldData->relocateTo(newData);
				oldData->~VThreadTaskData();

#ifdef MTROPOLIS_DEBUG_VTHREAD_STACKS
				newFrame->data = newData;
				newFrame->prevFrame = nullptr;
				if (nextFrame)
					nextFrame->prevFrame = newFrame;

				nextFrame = newFrame;
#endif

				framePos = nextPos;
			}
		}

		if (_stackUnalignedBase)
			free(_stackUnalignedBase);

		_stackUnalignedBase = unalignedBase;
		_stackAlignedBase = alignedBase;
	}

	VThreadStackFrame *newFrame = reinterpret_cast<VThreadStackFrame *>(static_cast<char *>(_stackAlignedBase) + offsetOfFrame);
	void *newData = static_cast<char *>(_stackAlignedBase) + offsetOfData;
	_used = offsetOfEndOfFrame;

	outFramePtr = newFrame;
	outUnadjustedDataPtr = newData;
	outPrevFrameOffset = offsetOfPrevFrame;
}

bool VThread::popFrame(void *&dataPtr, void *&outFramePtr) {
	if (_used == 0)
		return false;

	VThreadStackFrame *frame = reinterpret_cast<VThreadStackFrame *>(static_cast<char *>(_stackAlignedBase) + _used - sizeof(VThreadStackFrame));
	VThreadTaskData *data = reinterpret_cast<VThreadTaskData *>(static_cast<char *>(_stackAlignedBase) + frame->taskDataOffset);

	dataPtr = data;
	outFramePtr = frame;

	if (frame->prevFrameOffset == 0)
		_used = 0;
	else
		_used = frame->prevFrameOffset + sizeof(VThreadStackFrame);

	return true;
}

} // End of namespace MTropolis
