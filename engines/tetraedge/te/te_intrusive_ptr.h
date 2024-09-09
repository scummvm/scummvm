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

#ifndef TETRAEDGE_TE_TE_INTRUSIVE_PTR_H
#define TETRAEDGE_TE_TE_INTRUSIVE_PTR_H

namespace Tetraedge {

/**
 Like the boost intrusive pointer.  To be used with an
 object inheriting from TeReferencesCounter
 */
template<class T> class TeIntrusivePtr {
public:
	// NOTE: The original uses a member function for this, which is cleaner, but
	// MSVC compiles member functions for different class types (forward
	// declaration, multi-inheritance, etc) as different sizes which causes all
	// sorts of issues.  Only accept a static function to avoid such problems.
	typedef void(*Tdestructor)(T *obj);

	TeIntrusivePtr() : _p(nullptr), _deleteFn(nullptr) {}

	TeIntrusivePtr(const TeIntrusivePtr<T> &other) : _deleteFn(nullptr) {
		_p = other._p;
		if (_p)
			_p->incrementCounter();
	}

	TeIntrusivePtr(T *obj) : _deleteFn(nullptr) {
		_p = obj;
		if (_p)
			_p->incrementCounter();
	}

	virtual ~TeIntrusivePtr() {
		release();
	}

	TeIntrusivePtr<T> &operator=(T *obj) {
		if (_p != obj) {
			release();
			_p = obj;
			if (_p)
				_p->incrementCounter();
		}
		return *this;
	}

	TeIntrusivePtr<T> &operator=(const TeIntrusivePtr<T> &other) {
		if (this != &other) {
			release();
			_p = other._p;
			_deleteFn = other._deleteFn;
			if (_p)
				_p->incrementCounter();
		}
		return *this;
	}

	void release() {
		if (_p) {
			if (_p->decrementCounter()) {
				if (_deleteFn)
					(_deleteFn)(_p);
				else
					delete _p;
			}
		}
		_p = nullptr;
	}

	bool operator==(const TeIntrusivePtr<T> &other) const {
		return (this == &other || _p == other._p);
	}

	T *get() {
		return _p;
	}

	const T *get() const {
		return _p;
	}

	T &operator*() {
		return *_p;
	}

	const T &operator*() const {
		return *_p;
	}

	operator bool() const {
		return _p != nullptr;
	}

	T *operator->() {
		return _p;
	}

	const T *operator->() const {
		return _p;
	}

	void setDeleteFn(Tdestructor destructor) {
		_deleteFn = destructor;
	}

private:
	T *_p;
	Tdestructor _deleteFn;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_INTRUSIVE_PTR_H
