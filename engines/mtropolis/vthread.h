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

#ifndef MTROPOLIS_VTHREAD_H
#define MTROPOLIS_VTHREAD_H

#include "mtropolis/debug.h"

namespace MTropolis {

// Virtual thread, really a task stack
enum VThreadState {
	kVThreadReturn,
	kVThreadSuspended,
	kVThreadError,
};

struct VThreadFaultIdentifier {
};

template<typename T>
struct VThreadFaultIdentifierSingleton {
	static VThreadFaultIdentifier _identifier;
};

template<typename T>
VThreadFaultIdentifier VThreadFaultIdentifierSingleton<T>::_identifier;

class VThreadTaskData : public Debuggable {
public:
	VThreadTaskData();
	virtual ~VThreadTaskData();

	virtual VThreadState destructAndRunTask() = 0;
	virtual void relocateTo(void *newPosition) = 0;
	virtual bool handlesFault(const VThreadFaultIdentifier *faultID) const = 0;

#ifdef MTROPOLIS_DEBUG_ENABLE
public:
	void debugInit(const char *name);

protected:
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusDone; }
	const char *debugGetTypeName() const override { return "Task"; }
	const Common::String &debugGetName() const override { return _debugName; }
	void debugInspect(IDebugInspectionReport *report) const override;

	Common::String _debugName;
#endif
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
	VThreadMethodData(const VThreadFaultIdentifier *faultID, TClass *target, VThreadState (TClass::*method)(const TData &data));
	VThreadMethodData(const VThreadMethodData &other);
	VThreadMethodData(VThreadMethodData &&other);

	VThreadState destructAndRunTask() override;
	void relocateTo(void *newPosition) override;
	bool handlesFault(const VThreadFaultIdentifier *faultID) const override;

	TData &getData();

private:
	const VThreadFaultIdentifier *_faultID;
	TClass *_target;
	VThreadState (TClass::*_method)(const TData &data);
	TData _data;
};

template<typename TData>
class VThreadFunctionData : public VThreadTaskData {

public:
	explicit VThreadFunctionData(const VThreadFaultIdentifier *faultID, VThreadState (*func)(const TData &data));
	VThreadFunctionData(const VThreadFunctionData &other);

	VThreadFunctionData(VThreadFunctionData &&other);

	VThreadState destructAndRunTask() override;
	void relocateTo(void *newPosition) override;
	bool handlesFault(const VThreadFaultIdentifier *faultID) const override;

	TData &getData();

private:
	const VThreadFaultIdentifier *_faultID;
	VThreadState (*_func)(const TData &data);
	TData _data;
};

class VThread {

public:
	VThread();
	~VThread();

	template<typename TClass, typename TData>
	TData *pushTask(const char *name, TClass *obj, VThreadState (TClass::*method)(const TData &data));

	template<typename TData>
	TData *pushTask(const char *name, VThreadState (*func)(const TData &data));

	template<typename TFaultType, typename TClass, typename TData>
	TData *pushFaultHandler(const char *name, TClass *obj, VThreadState (TClass::*method)(const TData &data));

	template<typename TFaultType, typename TData>
	TData *pushFaultHandler(const char *name, VThreadState (*func)(const TData &data));

	VThreadState step();

	bool hasTasks() const;

private:
	template<typename TClass, typename TData>
	TData *pushTaskWithFaultHandler(const VThreadFaultIdentifier *faultID, const char *name, TClass *obj, VThreadState (TClass::*method)(const TData &data));

	template<typename TData>
	TData *pushTaskWithFaultHandler(const VThreadFaultIdentifier *faultID, const char *name, VThreadState (*func)(const TData &data));

	void reserveFrame(size_t size, size_t alignment, void *&outFramePtr, void *&outUnadjustedDataPtr, size_t &outPrevFrameOffset);
	bool popFrame(void *&dataPtr, void *&outFramePtr);

	void *_stackUnalignedBase;
	void *_stackAlignedBase;
	//size_t _size;
	size_t _alignment;
	size_t _used;
	VThreadFaultIdentifier *_faultID;
};

template<typename TClass, typename TData>
VThreadMethodData<TClass, TData>::VThreadMethodData(const VThreadFaultIdentifier *faultID, TClass *target, VThreadState (TClass::*method)(const TData &data))
	: _faultID(faultID), _target(target), _method(method) {
}

template<typename TClass, typename TData>
VThreadMethodData<TClass, TData>::VThreadMethodData(const VThreadMethodData& other)
	: _faultID(other._faultID), _target(other._target), _method(other._method), _data(other._data) {
}

template<typename TClass, typename TData>
VThreadMethodData<TClass, TData>::VThreadMethodData(VThreadMethodData &&other)
	: _faultID(other._faultID), _target(other._target), _method(other._method), _data(static_cast<TData &&>(other._data)) {
}

template<typename TClass, typename TData>
VThreadState VThreadMethodData<TClass, TData>::destructAndRunTask() {
	TData data(static_cast<TData &&>(_data));

	TClass *target = _target;
	VThreadState (TClass::*method)(const TData &) = _method;

	this->~VThreadMethodData<TClass, TData>();

	return (target->*method)(data);
}

template<typename TClass, typename TData>
void VThreadMethodData<TClass, TData>::relocateTo(void *newPosition) {
	void *adjustedPtr = static_cast<VThreadMethodData<TClass, TData> *>(static_cast<VThreadTaskData *>(newPosition));

	new (adjustedPtr) VThreadMethodData<TClass, TData>(static_cast<VThreadMethodData<TClass, TData> &&>(*this));
}

template<typename TClass, typename TData>
bool VThreadMethodData<TClass, TData>::handlesFault(const VThreadFaultIdentifier* faultID) const {
	return _faultID == faultID;
}

template<typename TClass, typename TData>
TData &VThreadMethodData<TClass, TData>::getData() {
	return _data;
}

template<typename TData>
VThreadFunctionData<TData>::VThreadFunctionData(const VThreadFaultIdentifier *faultID, VThreadState (*func)(const TData &data))
	: _faultID(faultID), _func(func) {
}

template<typename TData>
VThreadFunctionData<TData>::VThreadFunctionData(const VThreadFunctionData &other)
	: _faultID(other._faultID), _func(other._func), _data(other._data) {
}

template<typename TData>
VThreadFunctionData<TData>::VThreadFunctionData(VThreadFunctionData &&other)
	: _faultID(other._faultID), _func(other._func), _data(static_cast<TData &&>(other._data)) {
}

template<typename TData>
VThreadState VThreadFunctionData<TData>::destructAndRunTask() {
	TData data(static_cast<TData &&>(_data));

	VThreadState (*func)(const TData &) = _func;

	this->~VThreadFunctionData<TData>();

	return func(data);
}

template<typename TData>
void VThreadFunctionData<TData>::relocateTo(void *newPosition) {
	void *adjustedPtr = static_cast<VThreadFunctionData<TData> *>(static_cast<VThreadTaskData *>(newPosition));

	new (adjustedPtr) VThreadFunctionData<TData>(static_cast<VThreadFunctionData<TData> &&>(*this));
}

template<typename TData>
bool VThreadFunctionData<TData>::handlesFault(const VThreadFaultIdentifier *faultID) const {
	return _faultID == faultID;
}

template<typename TData>
TData &VThreadFunctionData<TData>::getData() {
	return _data;
}

template<typename TClass, typename TData>
TData *VThread::pushTask(const char *name, TClass *obj, VThreadState (TClass::*method)(const TData &data)) {
	return this->pushTaskWithFaultHandler(nullptr, name, obj, method);
}

template<typename TData>
TData *VThread::pushTask(const char *name, VThreadState (*func)(const TData &data)) {
	return this->pushTaskWithFaultHandler(nullptr, name, func);
}

template<typename TFaultType, typename TClass, typename TData>
TData *VThread::pushFaultHandler(const char *name, TClass *obj, VThreadState (TClass::*method)(const TData &data)) {
	return this->pushTaskWithFaultHandler(&VThreadFaultIdentifierSingleton<TFaultType>::_identifier, name, obj, method);
}

template<typename TFaultType, typename TData>
TData *VThread::pushFaultHandler(const char *name, VThreadState (*func)(const TData &data)) {
	return this->pushTaskWithFaultHandler(&VThreadFaultIdentifierSingleton<TFaultType>::_identifier, name, func);
}


template<typename TClass, typename TData>
TData *VThread::pushTaskWithFaultHandler(const VThreadFaultIdentifier *faultID, const char *name, TClass *obj, VThreadState (TClass::*method)(const TData &data)) {
	typedef VThreadMethodData<TClass, TData> FrameData_t;

	const size_t frameAlignment = alignof(VThreadStackFrame);
	const size_t dataAlignment = alignof(FrameData_t);
	const size_t maxAlignment = (frameAlignment < dataAlignment) ? dataAlignment : frameAlignment;

	void *framePtr;
	void *dataPtr;
	size_t prevFrameOffset;
	reserveFrame(sizeof(FrameData_t), maxAlignment, framePtr, dataPtr, prevFrameOffset);

	VThreadStackFrame *frame = new (framePtr) VThreadStackFrame();
	frame->prevFrameOffset = prevFrameOffset;

	FrameData_t *frameData = new (dataPtr) FrameData_t(faultID, obj, method);
	frame->taskDataOffset = reinterpret_cast<char *>(static_cast<VThreadTaskData *>(frameData)) - static_cast<char *>(_stackAlignedBase);

#ifdef MTROPOLIS_DEBUG_VTHREAD_STACKS
	frame->prevFrame = reinterpret_cast<VThreadStackFrame *>(static_cast<char *>(_stackAlignedBase) + prevFrameOffset);
	frame->data = frameData;
#endif
#ifdef MTROPOLIS_DEBUG_ENABLE
	frameData->debugInit(name);
#endif

	return &frameData->getData();
}

template<typename TData>
TData *VThread::pushTaskWithFaultHandler(const VThreadFaultIdentifier *faultID, const char *name, VThreadState (*func)(const TData &data)) {
	typedef VThreadFunctionData<TData> FrameData_t;

	const size_t frameAlignment = alignof(VThreadStackFrame);
	const size_t dataAlignment = alignof(FrameData_t);
	const size_t maxAlignment = (frameAlignment < dataAlignment) ? dataAlignment : frameAlignment;

	void *framePtr;
	void *dataPtr;
	size_t prevFrameOffset;
	reserveFrame(sizeof(FrameData_t), maxAlignment, framePtr, dataPtr, prevFrameOffset);

	VThreadStackFrame *frame = new (framePtr) VThreadStackFrame();
	frame->prevFrameOffset = prevFrameOffset;

	FrameData_t *frameData = new (dataPtr) FrameData_t(faultID, func);
	frame->taskDataOffset = reinterpret_cast<char *>(static_cast<VThreadTaskData *>(frameData)) - static_cast<char *>(_stackAlignedBase);

#ifdef MTROPOLIS_DEBUG_VTHREAD_STACKS
	frame->prevFrame = reinterpret_cast<VThreadStackFrame *>(static_cast<char *>(_stackAlignedBase) + prevFrameOffset);
	frame->data = frameData;
#endif
#ifdef MTROPOLIS_DEBUG_ENABLE
	frameData->debugInit(name);
#endif

	return &frameData->getData();
}

} // End of namespace MTropolis

#endif
