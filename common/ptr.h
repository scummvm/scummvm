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

class BasePtrDeletionInternal {
public:
	virtual ~BasePtrDeletionInternal() {}
};

template<class T>
class BasePtrDeletionImpl : public BasePtrDeletionInternal {
public:
	BasePtrDeletionImpl(T *ptr) : _ptr(ptr) {}
	~BasePtrDeletionImpl() {
		STATIC_ASSERT(sizeof(T) > 0, SharedPtr_cannot_delete_incomplete_type);
		delete _ptr;
	}
private:
	T *_ptr;
};

template<class T, class DL>
class BasePtrDeletionDeleterImpl : public BasePtrDeletionInternal {
public:
	BasePtrDeletionDeleterImpl(T *ptr, DL d) : _ptr(ptr), _deleter(d) {}
	~BasePtrDeletionDeleterImpl() { _deleter(_ptr); }
private:
	T *_ptr;
	DL _deleter;
};

/**
 * A base class for both SharedPtr and WeakPtr.
 *
 * This base class encapsulates the logic for the reference counter
 * used by both.
 */
template<class T>
class BasePtr : public SafeBool<BasePtr<T> > {
#if !defined(__GNUC__) || GCC_ATLEAST(3, 0)
	template<class T2> friend class BasePtr;
#endif
public:
	typedef int RefValue;
	typedef T ValueType;
	typedef T *PointerType;
	typedef T &ReferenceType;

	BasePtr() : _refCount(nullptr), _deletion(nullptr), _pointer(nullptr) {
	}

	explicit BasePtr(std::nullptr_t) : _refCount(nullptr), _deletion(nullptr), _pointer(nullptr) {
	}

	template<class T2>
	explicit BasePtr(T2 *p) : _refCount(new RefValue(1)), _deletion(new BasePtrDeletionImpl<T2>(p)), _pointer(p) {
	}

	template<class T2, class DL>
	BasePtr(T2 *p, DL d) : _refCount(new RefValue(1)), _deletion(new BasePtrDeletionDeleterImpl<T2, DL>(p, d)), _pointer(p) {
	}

	BasePtr(const BasePtr &r) : _refCount(r._refCount), _deletion(r._deletion), _pointer(r._pointer) {
		if (_refCount)
			++(*_refCount);
	}
	template<class T2>
	BasePtr(const BasePtr<T2> &r) : _refCount(r._refCount), _deletion(r._deletion), _pointer(r._pointer) {
		if (_refCount) ++(*_refCount);
	}

	~BasePtr() {
		decRef();
	}

	/**
	 * Implicit conversion operator to bool for convenience, to make
	 * checks like "if (sharedPtr) ..." possible.
	 */
	bool operator_bool() const {
		return _pointer != nullptr;
	}

	/**
	 * Returns the number of references to the assigned pointer.
	 * This should just be used for debugging purposes.
	 */
	RefValue refCount() const {
		return _refCount ? *_refCount : 0;
	}

	/**
	 * Returns whether the referenced object isn't valid
	 */
	bool expired() const {
		return !_refCount;
	}

	/**
	 * Checks if the object is the only object refering
	 * to the assigned pointer. This should just be used for
	 * debugging purposes.
	 */
	bool unique() const {
		return refCount() == 1;
	}

	BasePtr &operator=(const BasePtr &r) {
		reset(r);
		return *this;
	}

	template<class T2>
	BasePtr &operator=(const BasePtr<T2> &r) {
		reset(r);
		return *this;
	}

	/**
	 * Resets the object to a NULL pointer.
	 */
	void reset() {
		decRef();
		_deletion = nullptr;
		_refCount = nullptr;
		_pointer = nullptr;
	}

	/**
	 * Resets the object to the specified pointer
	 */
	void reset(const BasePtr &r) {
		if (r._refCount)
			++(*r._refCount);
		decRef();

		_refCount = r._refCount;
		_deletion = r._deletion;
		_pointer = r._pointer;
	}

	/**
	 * Resets the object to the specified pointer
	 */
	template<class T2>
	void reset(const BasePtr<T2> &r) {
		if (r._refCount)
			++(*r._refCount);
		decRef();

		_refCount = r._refCount;
		_deletion = r._deletion;
		_pointer = r._pointer;
	}

	/**
	 * Resets the object to the specified pointer
	 */
	void reset(T *ptr) {
		reset(BasePtr<T>(ptr));
	}

protected:
	RefValue *_refCount;
	BasePtrDeletionInternal *_deletion;
	PointerType _pointer;
protected:
	/**
	 * Decrements the reference count to the stored pointer, and deletes it if
	 * there are no longer any references to it
	 */
	void decRef() {
		if (_refCount) {
			--(*_refCount);
			if (!*_refCount) {
				delete _refCount;
				delete _deletion;
				_deletion = nullptr;
				_refCount = nullptr;
				_pointer = nullptr;
			}
		}
	}

	/**
	 * Increments the reference count to the stored pointer
	 */
	void incRef() {
		if (_refCount)
			++*_refCount;
	}
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
class SharedPtr : public BasePtr<T> {
public:
	typedef T *PointerType;
	typedef T &ReferenceType;

	SharedPtr() : BasePtr<T>() {
	}

	SharedPtr(std::nullptr_t) : BasePtr<T>() {
	}

	template<class T2>
	explicit SharedPtr(T2 *p) : BasePtr<T>(p) {
	}

	template<class T2, class DL>
	SharedPtr(T2 *p, DL d) : BasePtr<T>(p, d) {
	}

	SharedPtr(const SharedPtr<T> &r) : BasePtr<T>(r) {
	}

	SharedPtr(const WeakPtr<T> &r) : BasePtr<T>(r) {
	}

	template<class T2>
	SharedPtr(const SharedPtr<T2> &r) : BasePtr<T>(r) {
	}

	SharedPtr &operator=(const SharedPtr &r) {
		BasePtr<T>::operator=(r);
		return *this;
	}

	template<class T2>
	SharedPtr &operator=(const SharedPtr<T2> &r) {
		BasePtr<T>::operator=(r);
		return *this;
	}

	T &operator*() const { assert(this->_pointer); return *this->_pointer; }
	T *operator->() const { assert(this->_pointer); return this->_pointer; }

	/**
	 * Returns the plain pointer value. Be sure you know what you
	 * do if you are continuing to use that pointer.
	 *
	 * @return the pointer the SharedPtr object manages
	 */
	PointerType get() const { return this->_pointer; }

	template<class T2>
	bool operator==(const SharedPtr<T2> &r) const {
		return this->_pointer == r.get();
	}

	template<class T2>
	bool operator!=(const SharedPtr<T2> &r) const {
		return this->_pointer != r.get();
	}
};

/**
 * Implements a smart pointer that holds a non-owning ("weak") refrence to
 * a pointer. It needs to be converted to a SharedPtr to access it.
 */
template<class T>
class WeakPtr : public BasePtr<T> {
public:
	WeakPtr() : BasePtr<T>() {
	}

	WeakPtr(std::nullptr_t) : BasePtr<T>() {
	}

	template<class T2>
	explicit WeakPtr(T2 *p) : BasePtr<T>(p) {
	}

	WeakPtr(const BasePtr<T> &r) : BasePtr<T>(r) {
	}

	template<class T2>
	WeakPtr(const BasePtr<T2> &r) : BasePtr<T>(r) {
	}

	/**
	 * Creates a SharedPtr that manages the referenced object
	 */
	SharedPtr<T> lock() const {
		return SharedPtr<T>(*this);
	}
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
