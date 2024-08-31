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


#ifndef QDENGINE_QDCORE_UTIL_HANDLE_H
#define QDENGINE_QDCORE_UTIL_HANDLE_H


namespace QDEngine {

////////////////////////////////////////////////////////////////////
// Автоматически удаляемый указатель
////////////////////////////////////////////////////////////////////
template<class T>
class PtrHandle {
public:
	PtrHandle(T *p = 0) : ptr(p) {}
	PtrHandle(PtrHandle &p) : ptr(p.release()) {}
	~PtrHandle() {
		delete ptr;
	}

	void set(T *p) {
		ptr = p;
	}

	PtrHandle &operator=(PtrHandle &p) {
		if (get() != p.get()) {
			delete ptr;
			ptr = p.release();
		}
		return *this;
	}

	PtrHandle &operator=(T *p) {
		if (get() != p)
			delete ptr;
		set(p);
		return *this;
	}

	T *get() const {
		return ptr;
	}
	T *release() {
		T *tmp = ptr;
		ptr = 0;
		return tmp;
	}

	T *operator->() const {
		return ptr;
	}
	T &operator*() const {
		return *ptr;
	}
	T *operator()() const {
		return ptr;
	}
	operator T *() const {
		return ptr;
	}

private:
	T *ptr;
};

////////////////////////////////////////////////////////////////////
// Автоматически удаляемый указатель
// с отслеживанием владельцев.
// Обекты должны наследовать ShareHandleBase
////////////////////////////////////////////////////////////////////
template<class T>
class ShareHandle {
public:
	ShareHandle(T *p = 0) {
		set(p);
	}
	ShareHandle(const ShareHandle &orig) {
		set(orig.ptr);
	}

	~ShareHandle() {
		if (ptr && !ptr->decrRef())
			delete ptr;
	}

	void set(T *p) {
		ptr = p;
		if (p)
			p->addRef();
	}

	ShareHandle &operator=(const ShareHandle &orig) {
		if (ptr && !ptr->decrRef() && ptr != orig.ptr)
			delete ptr;
		set(orig.ptr);
		return *this;
	}

	ShareHandle &operator=(T *p) {
		if (ptr && !ptr->decrRef() && ptr != p)
			delete ptr;
		set(p);
		return *this;
	}

	T *get() const {
		return ptr;
	}
	T *release() {
		T *tmp = ptr;
		if (ptr) ptr->decrRef();
		ptr = 0;
		return tmp;
	}

	T *operator->() const {
		return ptr;
	}
	T &operator*() const {
		return *ptr;
	}
	T *operator()() const {
		return ptr;
	}
	operator T *() const {
		return ptr;
	}

private:
	T *ptr;
};

class ShareHandleBase {
public:
	ShareHandleBase() {
		handleCount = 0;
	}
	ShareHandleBase(const ShareHandleBase &) {
		handleCount = 0;
	}
	~ShareHandleBase() {}
	void addRef() {
		++handleCount;
	}
	int decrRef() {
		return --handleCount;
	}
	int numRef() const {
		return handleCount;
	}

private:
	mutable int handleCount;
};

} // namespace QDEngine

#endif // QDENGINE_QDCORE_UTIL_HANDLE_H
