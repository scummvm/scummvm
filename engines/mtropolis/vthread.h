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

#ifndef MTROPOLIS_TASKSTACK_H
#define MTROPOLIS_TASKSTACK_H

#include <new>

namespace MTropolis {

#define MTROPOLIS_DEBUG_VTHREAD_STACKS

// Virtual thread, really a task stack
enum VThreadState {
	kVThreadReturn,
	kVThreadSuspend,
	kVThreadError,
};

template<typename TClass>
struct VThreadAlignmentHelper {
	char prefix;
	TClass item;
};


class VThreadTaskData {

public:
	virtual ~VThreadTaskData();

	virtual VThreadState destructAndRunTask() = 0;
	virtual void relocateTo(void *newPosition) = 0;
};

struct VThreadStackFrame {
	size_t taskDataOffset;	// Offset to VThreadTaskData
	size_t prevFrameOffset;

#ifdef MTROPOLIS_DEBUG_VTHREAD_STACKS
	VThreadTaskData *data;
	VThreadStackFrame *prevFrame;
#endif
};

template<typename TClass, typename TData>
class VThreadMethodData : public VThreadTaskData {

public:
	VThreadMethodData(TClass *target, VThreadState (TClass::*method)(const TData &data));
	VThreadMethodData(const VThreadMethodData &other);

#if __cplusplus >= 201103L
	VThreadMethodData(VThreadMethodData &&other);
#endif

	VThreadState destructAndRunTask() override;
	void relocateTo(void *newPosition) override;

	TData &getData();

private:
	TClass *_target;
	VThreadState (TClass::*_method)(const TData &data);
	TData _data;
};

template<typename TData>
class VThreadFunctionData : public VThreadTaskData {

public:
	explicit VThreadFunctionData(VThreadState (*func)(const TData &data));
	VThreadFunctionData(const VThreadFunctionData &other);

#if __cplusplus >= 201103L
	VThreadFunctionData(VThreadMethodData &&other);
#endif

	VThreadState destructAndRunTask() override;
	void relocateTo(void *newPosition) override;

	TData &getData();

private:
	VThreadState (*_func)(const TData &data);
	TData _data;
};

class VThread {

public:
	VThread();
	~VThread();

	template<typename TClass, typename TData>
	TData *pushTask(TClass *obj, VThreadState (TClass::*method)(const TData &data));

	template<typename TData>
	TData *pushTask(VThreadState (*func)(const TData &data));

	VThreadState step();

private:

	void reserveFrame(size_t size, size_t alignment, void *&outFramePtr, void *&outUnadjustedDataPtr, size_t &outPrevFrameOffset);
	bool popFrame(void *&dataPtr, void *&outFramePtr);

	void *_stackUnalignedBase;
	void *_stackAlignedBase;
	size_t _size;
	size_t _alignment;
	size_t _used;
};

template<typename TClass, typename TData>
VThreadMethodData<TClass, TData>::VThreadMethodData(TClass *target, VThreadState (TClass::*method)(const TData &data)) : _target(target), _method(method) {
}

template<typename TClass, typename TData>
VThreadMethodData<TClass, TData>::VThreadMethodData(const VThreadMethodData& other) : _target(other._target), _method(other._method), _data(other._data) {
}

#if __cplusplus >= 201103L

template<typename TClass, typename TData>
VThreadMethodData<TClass, TData>::VThreadMethodData(VThreadMethodData &&other) : _target(other._target), _method(other._method), _data(static_cast<TData &&>(*static_cast<TData *>(other._data))) {
}

#endif

template<typename TClass, typename TData>
VThreadState VThreadMethodData<TClass, TData>::destructAndRunTask() {
#if __cplusplus >= 201103L
	TData data(static_cast<TData &&>(_data));
#else
	TData data(_data);
#endif

	TClass *target = _target;
	VThreadState (TClass::*method)(const TData &) = _method;

	this->~VThreadMethodData<TClass, TData>();

	return target->*method(data);
}

template<typename TClass, typename TData>
void VThreadMethodData<TClass, TData>::relocateTo(void *newPosition) {
	void *adjustedPtr = static_cast<VThreadMethodData<TClass, TData> *>(static_cast<VThreadTaskData *>(newPosition));

#if __cplusplus >= 201103L
	new (adjustedPtr) VThreadMethodData<TClass, TData>(static_cast<VThreadMethodData<TClass, TData> &&>(*this));
#else
	new (adjustedPtr) VThreadMethodData<TClass, TData>(*this);
#endif
}

template<typename TClass, typename TData>
TData &VThreadMethodData<TClass, TData>::getData() {
	return _data;
}

template<typename TData>
VThreadFunctionData<TData>::VThreadFunctionData(VThreadState (*func)(const TData &data)) : _func(func) {
}

template<typename TData>
VThreadFunctionData<TData>::VThreadFunctionData(const VThreadFunctionData &other) : _func(other._func), _data(other._data) {
}

#if __cplusplus >= 201103L

template<typename TData>
VThreadFunctionData<TData>::VThreadFunctionData(VThreadMethodData &&other) : func(other._func), data(static_cast<TData &&>(other._data)) {
}

#endif

template<typename TData>
VThreadState VThreadFunctionData<TData>::destructAndRunTask() {
#if __cplusplus >= 201103L
	TData data(static_cast<TData &&>(_data));
#else
	TData data(_data);
#endif

	VThreadState (*func)(const TData &) = _func;

	this->~VThreadFunctionData<TData>();

	return func(data);
}

template<typename TData>
void VThreadFunctionData<TData>::relocateTo(void *newPosition) {
	void *adjustedPtr = static_cast<VThreadFunctionData<TData> *>(static_cast<VThreadTaskData *>(newPosition));

#if __cplusplus >= 201103L
	new (adjustedPtr) VThreadFunctionData<TData>(static_cast<VThreadFunctionData<TData> &&>(*this));
#else
	new (adjustedPtr) VThreadFunctionData<TData>(*this);
#endif
}

template<typename TData>
TData &VThreadFunctionData<TData>::getData() {
	return _data;
}

template<typename TClass, typename TData>
TData *VThread::pushTask(TClass *obj, VThreadState (TClass::*method)(const TData &data)) {
	typedef VThreadMethodData<TClass, TData> FrameData_t; 

	const size_t frameAlignment = offsetof(VThreadAlignmentHelper<VThreadStackFrame>, item);
	const size_t dataAlignment = offsetof(VThreadAlignmentHelper<FrameData_t>, item);
	const size_t maxAlignment = (frameAlignment < dataAlignment) ? dataAlignment : frameAlignment;

	void *framePtr;
	void *dataPtr;
	size_t dataOffset;
	size_t prevFrameOffset;
	reserveFrame(sizeof(FrameData_t), maxAlignment, framePtr, dataPtr, prevFrameOffset);

	VThreadStackFrame *frame = new (framePtr) VThreadStackFrame();
	frame->prevFrameOffset = prevFrameOffset;

	FrameData_t *frameData = new (dataPtr) FrameData_t(obj, method);
	frame->taskDataOffset = reinterpret_cast<char *>(static_cast<VThreadTaskData *>(frameData)) - static_cast<char *>(_stackAlignedBase);

#ifdef MTROPOLIS_DEBUG_VTHREAD_STACKS
	frame->prevFrame = reinterpret_cast<VThreadStackFrame *>(static_cast<char *>(_stackAlignedBase) + prevFrameOffset);
	frame->data = frameData;
#endif

	return &frameData->data;
}

template<typename TData>
TData *VThread::pushTask(VThreadState (*func)(const TData &data)) {
	typedef VThreadFunctionData<TData> FrameData_t;

	const size_t frameAlignment = offsetof(VThreadAlignmentHelper<VThreadStackFrame>, item);
	const size_t dataAlignment = offsetof(VThreadAlignmentHelper<FrameData_t>, item);
	const size_t maxAlignment = (frameAlignment < dataAlignment) ? dataAlignment : frameAlignment;

	void *framePtr;
	void *dataPtr;
	size_t prevFrameOffset;
	reserveFrame(sizeof(FrameData_t), maxAlignment, framePtr, dataPtr, prevFrameOffset);

	VThreadStackFrame *frame = new (framePtr) VThreadStackFrame();
	frame->prevFrameOffset = prevFrameOffset;

	FrameData_t *frameData = new (dataPtr) FrameData_t(func);
	frame->taskDataOffset = reinterpret_cast<char *>(static_cast<VThreadTaskData *>(frameData)) - static_cast<char *>(_stackAlignedBase);

#ifdef MTROPOLIS_DEBUG_VTHREAD_STACKS
	frame->prevFrame = reinterpret_cast<VThreadStackFrame *>(static_cast<char *>(_stackAlignedBase) + prevFrameOffset);
	frame->data = frameData;
#endif

	return &frameData->getData();
}

} // End of namespace MTropolis

#endif
