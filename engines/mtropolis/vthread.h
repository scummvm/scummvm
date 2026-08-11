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

#include "mtropolis/coroutine_protos.h"
#include "mtropolis/coroutine_return_value.h"
#include "mtropolis/debug.h"

namespace MTropolis {

struct ICoroutineManager;
class VThread;

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

	virtual VThreadState execute(VThread *thread) = 0;

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

struct VThreadStackFrame;

class VThreadStackChunk {
public:
	explicit VThreadStackChunk(size_t capacity);
	VThreadStackChunk(VThreadStackChunk &&other);
	~VThreadStackChunk();

	VThreadStackFrame *_topFrame;
	byte *_memory;
	size_t _size;

private:
	VThreadStackChunk() = delete;
	VThreadStackChunk(const VThreadStackChunk &) = delete;
};

struct VThreadStackFrame {
	VThreadTaskData *data;
	VThreadStackFrame *prevFrame;
	bool isLastInChunk;
};

template<typename TClass, typename TData>
class VThreadMethodData : public VThreadTaskData {
public:
	VThreadMethodData(const VThreadFaultIdentifier *faultID, TClass *target, VThreadState (TClass::*method)(const TData &data));
	VThreadMethodData(const VThreadMethodData &other);
	VThreadMethodData(VThreadMethodData &&other);

	VThreadState execute(VThread *thread) override;

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

	VThreadState execute(VThread *thread) override;

	TData &getData();

private:
	const VThreadFaultIdentifier *_faultID;
	VThreadState (*_func)(const TData &data);
	TData _data;
};

class VThread {
public:
	explicit VThread(ICoroutineManager *coroManager);
	~VThread();

	template<typename TClass, typename TData>
	TData *pushTask(const char *name, TClass *obj, VThreadState (TClass::*method)(const TData &data));

	template<typename TData>
	TData *pushTask(const char *name, VThreadState (*func)(const TData &data));

	VThreadState step();

	bool hasTasks() const;

	bool popFrame();

	VThreadTaskData *pushCoroutineFrame(const CompiledCoroutine *compiledCoro, const CoroutineParamsBase &params, const CoroutineReturnValueRefBase &returnValueRef);

	template<typename TCoroutine, typename TReturnValue, typename ...TParams>
	void pushCoroutineWithReturn(TReturnValue *returnValuePtr, TParams &&...args);

	template<typename TCoroutine, typename TReturnValue>
	void pushCoroutineWithReturn(TReturnValue *returnValuePtr);

	template<typename TCoroutine, typename... TParams>
	void pushCoroutine(TParams &&...args);

	template<typename TCoroutine>
	void pushCoroutine();


private:
	void reserveFrame(size_t frameAlignment, size_t frameSize, VThreadStackFrame *&outFramePtr, size_t dataAlignment, size_t dataSize, void *&outDataPtr, bool &outIsNewChunk);
	static bool reserveFrameInChunk(VThreadStackChunk *chunk, size_t frameAlignment, size_t frameSize, VThreadStackFrame *&outFramePtr, size_t dataAlignment, size_t dataSize, void *&outDataPtr);

	void pushCoroutineInternal(CompiledCoroutine **compiledCoroPtr, CoroutineCompileFunction_t compileFunc, bool isVoidReturn, const CoroutineParamsBase &params, const CoroutineReturnValueRefBase &returnValueRef);

	template<typename TClass, typename TData>
	TData *pushTaskWithFaultHandler(const VThreadFaultIdentifier *faultID, const char *name, TClass *obj, VThreadState (TClass::*method)(const TData &data));

	template<typename TData>
	TData *pushTaskWithFaultHandler(const VThreadFaultIdentifier *faultID, const char *name, VThreadState (*func)(const TData &data));

	Common::Array<VThreadStackChunk> _stackChunks;
	ICoroutineManager *_coroManager;
	uint _numActiveStackChunks;
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
VThreadState VThreadMethodData<TClass, TData>::execute(VThread *thread) {
	TData data(static_cast<TData &&>(_data));

	TClass *target = _target;
	VThreadState (TClass::*method)(const TData &) = _method;

	thread->popFrame();

	return (target->*method)(data);
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
VThreadState VThreadFunctionData<TData>::execute(VThread *thread) {
	TData data(static_cast<TData &&>(_data));

	VThreadState (*func)(const TData &) = _func;

	thread->popFrame();

	return func(data);
}

template<typename TData>
TData &VThreadFunctionData<TData>::getData() {
	return _data;
}

template<typename TCoroutine, typename TReturnValue, typename... TParams>
void VThread::pushCoroutineWithReturn(TReturnValue *returnValuePtr, TParams &&...args) {
	assert(returnValuePtr != nullptr);
	this->pushCoroutineInternal(&TCoroutine::ms_compiledCoro, TCoroutine::compileCoroutine, CoroutineReturnValueRef<typename TCoroutine::ReturnValue_t>::isVoid(), typename TCoroutine::Params(Common::forward<TParams>(args)...), CoroutineReturnValueRef<typename TCoroutine::ReturnValue_t>(returnValuePtr));
}

template<typename TCoroutine, typename TReturnValue>
void VThread::pushCoroutineWithReturn(TReturnValue *returnValuePtr) {
	assert(returnValuePtr != nullptr);
	this->pushCoroutineInternal(&TCoroutine::ms_compiledCoro, TCoroutine::compileCoroutine, CoroutineReturnValueRef<typename TCoroutine::ReturnValue_t>::isVoid(), typename TCoroutine::Params(), CoroutineReturnValueRef<typename TCoroutine::ReturnValue_t>(returnValuePtr));
}

template<typename TCoroutine, typename... TParams>
void VThread::pushCoroutine(TParams &&...args) {
	this->pushCoroutineInternal(&TCoroutine::ms_compiledCoro, TCoroutine::compileCoroutine, CoroutineReturnValueRef<typename TCoroutine::ReturnValue_t>::isVoid(), typename TCoroutine::Params(Common::forward<TParams>(args)...), CoroutineReturnValueRef<typename TCoroutine::ReturnValue_t>());
}

template<typename TCoroutine>
void VThread::pushCoroutine() {
	this->pushCoroutineInternal(&TCoroutine::ms_compiledCoro, TCoroutine::compileCoroutine, CoroutineReturnValueRef<typename TCoroutine::ReturnValue_t>::isVoid(), typename TCoroutine::Params(), CoroutineReturnValueRef<typename TCoroutine::ReturnValue_t>());
}

template<typename TClass, typename TData>
TData *VThread::pushTask(const char *name, TClass *obj, VThreadState (TClass::*method)(const TData &data)) {
	return this->pushTaskWithFaultHandler(nullptr, name, obj, method);
}

template<typename TData>
TData *VThread::pushTask(const char *name, VThreadState (*func)(const TData &data)) {
	return this->pushTaskWithFaultHandler(nullptr, name, func);
}

template<typename TClass, typename TData>
TData *VThread::pushTaskWithFaultHandler(const VThreadFaultIdentifier *faultID, const char *name, TClass *obj, VThreadState (TClass::*method)(const TData &data)) {
	typedef VThreadMethodData<TClass, TData> FrameData_t;

	const size_t frameAlignment = alignof(VThreadStackFrame);
	const size_t dataAlignment = alignof(FrameData_t);

	VThreadStackFrame *prevFrame = nullptr;
	if (_numActiveStackChunks > 0)
		prevFrame = _stackChunks[_numActiveStackChunks - 1]._topFrame;

	VThreadStackFrame *framePtr = nullptr;
	void *dataPtr = nullptr;
	bool isNewChunk = false;
	reserveFrame(frameAlignment, sizeof(VThreadStackFrame), framePtr, dataAlignment, sizeof(FrameData_t), dataPtr, isNewChunk);

	VThreadStackFrame *frame = new (framePtr) VThreadStackFrame();
	FrameData_t *frameData = new (dataPtr) FrameData_t(faultID, obj, method);

	frame->data = frameData;
	frame->prevFrame = prevFrame;
	frame->isLastInChunk = isNewChunk;

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

	VThreadStackFrame *prevFrame = nullptr;
	if (_numActiveStackChunks > 0)
		prevFrame = _stackChunks[_numActiveStackChunks - 1]._topFrame;

	VThreadStackFrame *framePtr = nullptr;
	void *dataPtr = nullptr;
	bool isNewChunk = false;
	reserveFrame(frameAlignment, sizeof(VThreadStackFrame), framePtr, dataAlignment, sizeof(FrameData_t), dataPtr, isNewChunk);

	VThreadStackFrame *frame = new (framePtr) VThreadStackFrame();
	FrameData_t *frameData = new (dataPtr) FrameData_t(faultID, func);

	frame->data = frameData;
	frame->prevFrame = prevFrame;
	frame->isLastInChunk = isNewChunk;

#ifdef MTROPOLIS_DEBUG_ENABLE
	frameData->debugInit(name);
#endif

	return &frameData->getData();
}

} // End of namespace MTropolis

#endif
