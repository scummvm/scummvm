/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 */

#ifndef COMMON_PTR_H
#define COMMON_PTR_H

#include "common/scummsys.h"

namespace Common {

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
 * Pointers pointing to memory not allocated by new, will cause undefined
 * behavior on deletion. That is for example the case on pointers created
 * with malloc (or similar) and new[]. This prevents the use of SharedPtr
 * for arrays!
 *
 * Note that you have to specify the type itself not the pointer type as
 * template parameter. You also need to have a real definition of the type
 * you want to use, a simple forward definition is not enough.
 *
 * The class has implicit upcast support, so if you got a class B derived
 * from class A, you can assign a pointer to B without any problems to a
 * SharedPtr object with template parameter A. The very same applies to 
 * assigment of a SharedPtr<B> object to a SharedPtr<A> object.
 *
 * There are also operators != and == to compare two SharedPtr objects
 * with compatible pointers.
 */
template<class T>
class SharedPtr {
	template<class T2> friend class SharedPtr;
public:
	typedef int RefValue;
	typedef T ValueType;
	typedef T *Pointer;

	SharedPtr() : _refCount(0), _pointer(0) {}
	template<class T2> explicit SharedPtr(T2 *p) : _refCount(new RefValue(1)), _pointer(p) {}

	SharedPtr(const SharedPtr &r) : _refCount(r._refCount), _pointer(r._pointer) { if (_refCount) ++(*_refCount); }
	template<class T2> SharedPtr(const SharedPtr<T2> &r) : _refCount(r._refCount), _pointer(r._pointer) { if (_refCount) ++(*_refCount); }

	~SharedPtr() { decRef(); }

	SharedPtr &operator =(const SharedPtr &r) {
		if (r._refCount)
			++(*r._refCount);
		decRef();

		_refCount = r._refCount;
		_pointer = r._pointer;

		return *this;
	}

	template<class T2>
	SharedPtr &operator =(const SharedPtr<T2> &r) {
		if (r._refCount)
			++(*r._refCount);
		decRef();

		_refCount = r._refCount;
		_pointer = r._pointer;

		return *this;
	}

	ValueType &operator *() const { assert(_pointer); return *_pointer; }
	Pointer operator ->() const { assert(_pointer); return _pointer; }

	/**
	 * Returns the plain pointer value. Be sure you know what you
	 * do if you are continuing to use that pointer.
	 *
	 * @return the pointer the SharedPtr object manages
	 */
	Pointer get() const { return _pointer; }

	operator bool() const { return _pointer != 0; }

	/**
	 * Checks if the SharedPtr object is the only object refering
	 * to the assigned pointer. This should just be used for
	 * debugging purposes.
	 */
	bool unique() const { return refCount() == 1; }

	/**
	 * Returns the number of references to the assigned pointer.
	 * This should just be used for debugging purposes.
	 */
	RefValue refCount() const { return _refCount ? *_refCount : 0; }
private:
	void decRef() {
		if (_refCount) {
			--(*_refCount);
			if (!*_refCount) {
				delete _refCount;
				delete _pointer;
				_refCount = 0;
				_pointer = 0;
			}
		}
	}

	RefValue *_refCount;
	T *_pointer;
};

} // end of namespace Common

template<class T1, class T2>
bool operator ==(const Common::SharedPtr<T1> &l, const Common::SharedPtr<T2> &r) {
	return l.get() == r.get();
}

template<class T1, class T2>
bool operator !=(const Common::SharedPtr<T1> &l, const Common::SharedPtr<T2> &r) {
	return l.get() != r.get();
}


#endif

