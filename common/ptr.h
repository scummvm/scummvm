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

#ifndef COMMON_PTR_H
#define COMMON_PTR_H

#include "common/scummsys.h"
#include "common/noncopyable.h"
#include "common/safe-bool.h"
#include "common/types.h"

/* For nullptr_t */
#include <cstddef>

namespace Common {

/**
 * @defgroup common_ptr Pointers
 * @ingroup common
 *
 * @brief API and templates for pointers.
 * @{
 */

class BasePtrTrackerInternal {
public:
	typedef int RefValue;

	BasePtrTrackerInternal() : _weakRefCount(1), _strongRefCount(1) {}
	virtual ~BasePtrTrackerInternal() {}

	void incWeak() {
		_weakRefCount++;
	}

	void decWeak() {
		if (--_weakRefCount == 0)
			delete this;
	}

	void incStrong() {
		_strongRefCount++;
	}

	void decStrong() {
		if (--_strongRefCount == 0) {
			destructObject();
			decWeak();
		}
	}

	bool isAlive() const {
		return _strongRefCount > 0;
	}

	RefValue getStrongCount() const {
		return _strongRefCount;
	}

protected:
	virtual void destructObject() = 0;

private:
	RefValue _weakRefCount; // Weak ref count + 1 if object ref count > 0
	RefValue _strongRefCount;
};

template<class T>
class BasePtrTrackerImpl : public BasePtrTrackerInternal {
public:
	BasePtrTrackerImpl(T *ptr) : _ptr(ptr) {}

protected:
	void destructObject() override {
		STATIC_ASSERT(sizeof(T) > 0, SharedPtr_cannot_delete_incomplete_type);
		delete _ptr;
	}

	T *_ptr;
};

template<class T, class DL>
class BasePtrTrackerDeletionImpl : public BasePtrTrackerInternal {
public:
	BasePtrTrackerDeletionImpl(T *ptr, DL d) : _ptr(ptr), _deleter(d) {}

private:
	void destructObject() override {
		_deleter(_ptr);
	}

	T *_ptr;
	DL _deleter;
};

template<class T>
class WeakPtr;

/**
 * A simple shared pointer implementation modelled after boost.
 *
 * This object keeps track of the assigned pointer and automatically
 * frees it when no more SharedPtr references to it exist.
 *
 * To achieve that the object implements an internal reference counting.
 * Thus you should try to avoid using the plain pointer after assigning
 * it to a SharedPtr object for the first time. If you still use the
 * plain pointer be sure you do not delete it on your own. You may also
 * not use the plain pointer to create a new SharedPtr object, since that
 * would result in a double deletion of the pointer sooner or later.
 *
 * Example creation:
 * Common::SharedPtr<int> pointer(new int(1));
 * would create a pointer to int. Later on usage via *pointer is the same
 * as for a normal pointer. If you need to access the plain pointer value
 * itself later on use the get method. The class also supplies a operator
 * ->, which does the same as the -> operator on a normal pointer.
 *
 * Be sure you are using new to initialize the pointer you want to manage.
 * If you do not use new for allocating, you have to supply a deleter as
 * second parameter when creating a SharedPtr object. The deleter has to
 * implement operator() which takes the pointer it should free as argument.
 *
 * Note that you have to specify the type itself not the pointer type as
 * template parameter.
 *
 * When creating a SharedPtr object from a normal pointer you need a real
 * definition of the type you want SharedPtr to manage, a simple forward
 * definition is not enough.
 *
 * The class has implicit upcast support, so if you got a class B derived
 * from class A, you can assign a pointer to B without any problems to a
 * SharedPtr object with template parameter A. The very same applies to
 * assignment of a SharedPtr<B> object to a SharedPtr<A> object.
 *
 * There are also operators != and == to compare two SharedPtr objects
 * with compatible pointers. Comparison between a SharedPtr object and
 * a plain pointer is only possible via SharedPtr::get.
 */
template<class T>
class SharedPtr : public SafeBool<SharedPtr<T> > {
	template<class T2>
	friend class WeakPtr;
	template<class T2>
	friend class SharedPtr;
public:
	// Invariant: If _tracker is non-null, then the object is alive
	typedef T *PointerType;
	typedef T &ReferenceType;
	typedef BasePtrTrackerInternal::RefValue RefValue;

	SharedPtr() : _pointer(nullptr), _tracker(nullptr) {
	}

	SharedPtr(std::nullptr_t) : _pointer(nullptr), _tracker(nullptr) {
	}

	~SharedPtr() {
		if (_tracker)
			_tracker->decStrong();
	}

	template<class T2>
	explicit SharedPtr(T2 *p) : _pointer(p), _tracker(p ? (new BasePtrTrackerImpl<T2>(p)) : nullptr) {
	}

	template<class T2, class DL>
	SharedPtr(T2 *p, DL d) : _pointer(p), _tracker(p ? (new BasePtrTrackerDeletionImpl<T2, DL>(p, d)) : nullptr) {
	}

	SharedPtr(const SharedPtr<T> &r) : _pointer(r._pointer), _tracker(r._tracker) {
		if (_tracker)
			_tracker->incStrong();
	}

	template<class T2>
	SharedPtr(const SharedPtr<T2> &r) : _pointer(r._pointer), _tracker(r._tracker) {
		if (_tracker)
			_tracker->incStrong();
	}

	template<class T2>
	explicit SharedPtr(const WeakPtr<T2> &r) : _pointer(nullptr), _tracker(nullptr) {
		if (r._tracker && r._tracker->isAlive()) {
			_pointer = r._pointer;
			_tracker = r._tracker;
			_tracker->incStrong();
		}
	}

	SharedPtr &operator=(const SharedPtr &r) {
		reset(r);
		return *this;
	}

	template<class T2>
	SharedPtr &operator=(const SharedPtr<T2> &r) {
		reset(r);
		return *this;
	}

	T &operator*() const { assert(_pointer); return *_pointer; }
	T *operator->() const { assert(_pointer); return _pointer; }

	/**
	 * Returns the plain pointer value. Be sure you know what you
	 * do if you are continuing to use that pointer.
	 *
	 * @return the pointer the SharedPtr object manages
	 */
	PointerType get() const { return _pointer; }

	template<class T2>
	bool operator==(const SharedPtr<T2> &r) const {
		return _pointer == r.get();
	}

	template<class T2>
	bool operator!=(const SharedPtr<T2> &r) const {
		return _pointer != r.get();
	}

	bool operator==(std::nullptr_t) const {
		return _pointer == nullptr;
	}

	bool operator!=(std::nullptr_t) const {
		return _pointer != nullptr;
	}

	/**
	 * Implicit conversion operator to bool for convenience, to make
	 * checks like "if (sharedPtr) ..." possible.
	 */
	bool operator_bool() const {
		return _pointer != nullptr;
	}

	/**
	 * Returns the number of strong references to the object.
	 */
	int refCount() const {
		if (_tracker == nullptr)
			return 0;
		return _tracker->getStrongCount();
	}

	/**
	 * Checks if the object is the only object refering
	 * to the assigned pointer. This should just be used for
	 * debugging purposes.
	 */
	bool unique() const {
		return refCount() == 1;
	}

	/**
	 * Resets the object to a NULL pointer.
	 */
	void reset() {
		if (_tracker)
			_tracker->decStrong();
		_tracker = nullptr;
		_pointer = nullptr;
	}

	/**
	 * Resets the object to the specified shared pointer
	 */
	template<class T2>
	void reset(const SharedPtr<T2> &r) {
		BasePtrTrackerInternal *oldTracker = _tracker;

		_pointer = r._pointer;
		_tracker = r._tracker;

		if (_tracker)
			_tracker->incStrong();
		if (oldTracker)
			oldTracker->decStrong();
	}

	/**
	 * Resets the object to the specified weak pointer
	 */
	template<class T2>
	void reset(const WeakPtr<T2> &r) {
		BasePtrTrackerInternal *oldTracker = _tracker;

		if (r._tracker && r._tracker->isAlive()) {
			_tracker = r._tracker;
			_pointer = r._pointer;
			_tracker->incStrong();
		} else {
			_tracker = nullptr;
			_pointer = nullptr;
		}

		if (oldTracker)
			oldTracker->decStrong();
	}

	/**
	 * Resets the object to the specified pointer
	 */
	void reset(T *ptr) {
		if (_tracker)
			_tracker->decStrong();

		_pointer = ptr;
		_tracker = new BasePtrTrackerImpl<T>(ptr);
	}

	/**
	 * Performs the equivalent of static_cast to a new pointer type
	 */
	template<class T2>
	SharedPtr<T2> staticCast() const {
		return SharedPtr<T2>(static_cast<T2 *>(_pointer), _tracker);
	}

	/**
	 * Performs the equivalent of dynamic_cast to a new pointer type
	 */
	template<class T2>
	SharedPtr<T2> dynamicCast() const {
		return SharedPtr<T2>(dynamic_cast<T2 *>(_pointer), _tracker);
	}

	/**
	 * Performs the equivalent of const_cast to a new pointer type
	 */
	template<class T2>
	SharedPtr<T2> constCast() const {
		return SharedPtr<T2>(const_cast<T2 *>(_pointer), _tracker);
	}

	/**
	 * Performs the equivalent of const_cast to a new pointer type
	 */
	template<class T2>
	SharedPtr<T2> reinterpretCast() const {
		return SharedPtr<T2>(reinterpret_cast<T2 *>(_pointer), _tracker);
	}

private:
	SharedPtr(T *pointer, BasePtrTrackerInternal *tracker) : _pointer(pointer), _tracker(tracker) {
		if (tracker)
			tracker->incStrong();
	}

	T *_pointer;
	BasePtrTrackerInternal *_tracker;
};



/**
 * Implements a smart pointer that holds a non-owning ("weak") refrence to
 * a pointer. It needs to be converted to a SharedPtr to access it.
 */
template<class T>
class WeakPtr {
	template<class T2>
	friend class WeakPtr;
	template<class T2>
	friend class SharedPtr;
public:
	WeakPtr() : _pointer(nullptr), _tracker(nullptr) {
	}

	WeakPtr(std::nullptr_t) : _pointer(nullptr), _tracker(nullptr) {
	}

	WeakPtr(const WeakPtr<T> &r) : _pointer(r._pointer), _tracker(r._tracker) {
		if (_tracker)
			_tracker->incWeak();
	}

	~WeakPtr() {
		if (_tracker)
			_tracker->decWeak();
	}

	template<class T2>
	WeakPtr(const WeakPtr<T2> &r) : _pointer(r._pointer), _tracker(r._tracker) {
		if (_tracker)
			_tracker->incWeak();
	}

	template<class T2>
	WeakPtr(const SharedPtr<T2> &r) : _pointer(r._pointer), _tracker(r._tracker) {
		if (_tracker)
			_tracker->incWeak();
	}

	/**
	 * Performs the equivalent of static_cast to a new pointer type
	 */
	template<class T2>
	WeakPtr<T2> staticCast() const {
		return WeakPtr<T2>(expired() ? nullptr : static_cast<T2 *>(_pointer), _tracker);
	}

	/**
	 * Performs the equivalent of dynamic_cast to a new pointer type
	 */
	template<class T2>
	WeakPtr<T2> dynamicCast() const {
		return WeakPtr<T2>(expired() ? nullptr : dynamic_cast<T2 *>(_pointer), _tracker);
	}

	/**
	 * Performs the equivalent of const_cast to a new pointer type
	 */
	template<class T2>
	WeakPtr<T2> constCast() const {
		return WeakPtr<T2>(expired() ? nullptr : const_cast<T2 *>(_pointer), _tracker);
	}

	/**
	 * Performs the equivalent of const_cast to a new pointer type
	 */
	template<class T2>
	WeakPtr<T2> reinterpretCast() const {
		return WeakPtr<T2>(expired() ? nullptr : reinterpret_cast<T2 *>(_pointer), _tracker);
	}

	/**
	 * Creates a SharedPtr that manages the referenced object
	 */
	SharedPtr<T> lock() const {
		return SharedPtr<T>(*this);
	}

	/**
	 * Returns the number of strong references to the object.
	 */
	int refCount() const {
		if (_tracker == nullptr)
			return 0;
		return _tracker->getStrongCount();
	}

	/**
	 * Returns whether the referenced object isn't valid
	 */
	bool expired() const {
		return _tracker == nullptr || _tracker->getStrongCount() == 0;
	}

	/**
	 * Returns whether this precedes another weak pointer in owner-based order
	 */
	template<class T2>
	bool owner_before(const WeakPtr<T2>& other) const {
		return _tracker < other._tracker;
	}

	/**
	 * Returns whether this precedes a shared pointer in owner-based order
	 */
	template<class T2>
	bool owner_before(const SharedPtr<T2> &other) const {
		return _tracker < other._tracker;
	}

	WeakPtr<T> &operator=(const WeakPtr<T> &r) {
		reset(r);
		return *this;
	}

	template<class T2>
	WeakPtr<T> &operator=(const WeakPtr<T2> &r) {
		reset(r);
		return *this;
	}

	template<class T2>
	WeakPtr<T> &operator=(const SharedPtr<T2> &r) {
		reset(r);
		return *this;
	}

	/**
	 * Resets the object to a NULL pointer.
	 */
	void reset() {
		if (_tracker)
			_tracker->decWeak();
		_tracker = nullptr;
		_pointer = nullptr;
	}

	/**
	 * Resets the object to the specified shared pointer
	 */
	template<class T2>
	void reset(const SharedPtr<T2> &r) {
		BasePtrTrackerInternal *oldTracker = _tracker;

		_pointer = r._pointer;
		_tracker = r._tracker;

		if (_tracker)
			_tracker->incWeak();
		if (oldTracker)
			oldTracker->decWeak();
	}

	/**
	 * Resets the object to the specified weak pointer
	 */
	template<class T2>
	void reset(const WeakPtr<T2> &r) {
		BasePtrTrackerInternal *oldTracker = _tracker;

		_pointer = r._pointer;
		_tracker = r._tracker;

		if (_tracker)
			_tracker->incWeak();
		if (oldTracker)
			oldTracker->decWeak();
	}

private:
	WeakPtr(T *pointer, BasePtrTrackerInternal *tracker) : _pointer(pointer), _tracker(tracker) {
		if (tracker)
			tracker->incWeak();
	}

	T *_pointer;
	BasePtrTrackerInternal *_tracker;
};

template <typename T>
struct DefaultDeleter {
	inline void operator()(T *object) {
		STATIC_ASSERT(sizeof(T) > 0, cannot_delete_incomplete_type);
		delete object;
	}
};

template<typename T, class DL = DefaultDeleter<T> >
class ScopedPtr : private NonCopyable, public SafeBool<ScopedPtr<T, DL> > {
public:
	typedef T ValueType;
	typedef T *PointerType;
	typedef T &ReferenceType;

	explicit ScopedPtr(PointerType o = nullptr) : _pointer(o) {}
	ScopedPtr(std::nullptr_t) : _pointer(nullptr) {}

	/**
	 * Move constructor
	 */
	template<class T2>
	ScopedPtr(ScopedPtr<T2> &&o) : _pointer(o._pointer) {
		o._pointer = nullptr;
        }

	ReferenceType operator*() const { return *_pointer; }
	PointerType operator->() const { return _pointer; }

	/**
	 * Implicit conversion operator to bool for convenience, to make
	 * checks like "if (scopedPtr) ..." possible.
	 */
	bool operator_bool() const { return _pointer != nullptr; }

	~ScopedPtr() {
		DL()(_pointer);
	}

	/**
	 * Resets the pointer with the new value. Old object will be destroyed
	 */
	void reset(PointerType o = nullptr) {
		DL()(_pointer);
		_pointer = o;
	}

	/**
	 * Affectation with nullptr
	 */
	ScopedPtr &operator=(std::nullptr_t) {
		reset(nullptr);
	}

	/**
	 * Replaces the ScopedPtr with another scoped ScopedPtr.
	 */
	template<class T2>
	ScopedPtr &operator=(ScopedPtr<T2> &&other) {
		PointerType oldPointer = _pointer;
		_pointer = other._pointer;
		other._pointer = nullptr;
		DL()(oldPointer);
		return *this;
	}

	/**
	 * Returns the plain pointer value.
	 *
	 * @return the pointer the ScopedPtr manages
	 */
	PointerType get() const { return _pointer; }

	/**
	 * Returns the plain pointer value and releases ScopedPtr.
	 * After release() call you need to delete object yourself
	 *
	 * @return the pointer the ScopedPtr manages
	 */
	PointerType release() {
		PointerType r = _pointer;
		_pointer = nullptr;
		return r;
	}

private:
	PointerType _pointer;
};

template<typename T, class DL = DefaultDeleter<T> >
class DisposablePtr : private NonCopyable, public SafeBool<DisposablePtr<T, DL> > {
public:
	typedef T  ValueType;
	typedef T *PointerType;
	typedef T &ReferenceType;

	explicit DisposablePtr(PointerType o, DisposeAfterUse::Flag dispose) : _pointer(o), _dispose(dispose) {}

	~DisposablePtr() {
		if (_dispose) DL()(_pointer);
	}

	ReferenceType operator*() const { return *_pointer; }
	PointerType operator->() const { return _pointer; }

	/**
	 * Implicit conversion operator to bool for convenience, to make
	 * checks like "if (scopedPtr) ..." possible.
	 */
	bool operator_bool() const { return _pointer != nullptr; }

	/**
	 * Resets the pointer with the new value. Old object will be destroyed
	 */
	void reset(PointerType o, DisposeAfterUse::Flag dispose) {
		if (_dispose) DL()(_pointer);
		_pointer = o;
		_dispose = dispose;
	}

	/**
	 * Clears the pointer. Old object will be destroyed
	 */
	void reset() {
		reset(nullptr, DisposeAfterUse::NO);
	}

	/**
	 * Clears the pointer without destroying the old object.
	 */
	void disownPtr() {
		_pointer = nullptr;
		_dispose = DisposeAfterUse::NO;
	}

	/**
	 * Returns the plain pointer value.
	 *
	 * @return the pointer the DisposablePtr manages
	 */
	PointerType get() const { return _pointer; }

	/**
	 * Returns the pointer's dispose flag.
	 */
	DisposeAfterUse::Flag getDispose() const { return _dispose; }

private:
	PointerType           _pointer;
	DisposeAfterUse::Flag _dispose;
};

/** @} */

} // End of namespace Common

#endif
