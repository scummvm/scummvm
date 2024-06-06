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


#ifndef __HANDLE_H__
#define __HANDLE_H__

////////////////////////////////////////////////////////////////////
// Автоматически удаляемый указатель
////////////////////////////////////////////////////////////////////


namespace QDEngine {

template<class T>
class PtrHandle {
public:
	PtrHandle(T *p = 0) : ptr(p) {}
	PtrHandle(const PtrHandle &p) : ptr(p.release()) {}
	~PtrHandle() {
		delete ptr;
	}

	void set(T *p) {
		ptr = p;
	}

	PtrHandle &operator=(const PtrHandle &p) {
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
	T *release() const {
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
	mutable T *ptr;
};

////////////////////////////////////////////////////////////////////
//   База для интрузивных умных указателей ShareHandle
////////////////////////////////////////////////////////////////////
class ShareHandleBase {
public:
	ShareHandleBase() {
		handleCount = 0;
	}
	ShareHandleBase(const ShareHandleBase &) {
		handleCount = 0;
	}
	void addRef() const {
		++handleCount;
	}
	int decrRef() const {
		return --handleCount;
	}
	int numRef() const {
		return handleCount;
	}

private:
	mutable int handleCount;
};

////////////////////////////////////////////////////////////////////
//	База с виртуальным деструктором для скрытия описания класса
////////////////////////////////////////////////////////////////////
class PolymorphicBase : public ShareHandleBase {
public:
	virtual ~PolymorphicBase() {}
};

////////////////////////////////////////////////////////////////////
// Автоматически удаляемый указатель
// с отслеживанием владельцев.
// Обекты должны наследовать ShareHandleBase
// Второй шаблонный параметр служебный!!!
// В настоящее время serialize соответствует полиморфному классу.
////////////////////////////////////////////////////////////////////
template<class Type, class BaseType = Type>
class ShareHandle {
public:
	ShareHandle(Type *p = 0) {
		set(p);
	}
	ShareHandle(const ShareHandle &orig) {
		set(orig.get());
	}

	~ShareHandle() {
		if (ptr_ && !ptr_->decrRef())
			delete ptr_;
	}

	void set(Type *p) {
		ptr_ = (BaseType *)p;
		if (ptr_)
			ptr_->addRef();
	}

	ShareHandle &operator=(const ShareHandle &orig) {
		if (ptr_ && !ptr_->decrRef() && ptr_ != orig.ptr_)
			delete ptr_;
		set(orig.get());
		return *this;
	}

	ShareHandle &operator=(Type *p) {
		if (ptr_ && !ptr_->decrRef() && ptr_ != p)
			delete ptr_;
		set(p);
		return *this;
	}

	Type *get() const {
		return (Type *)ptr_;
	}

	Type *operator->() const {
		return get();
	}
	Type &operator*() const {
		return *get();
	}
	Type *operator()() const {
		return get();
	}

	operator Type *() const {
		return get();
	}

	template<class U>
	operator ShareHandle<U> () {
		return ShareHandle<U> (get());
	}

	bool serialize(class Archive &ar, const char *name, const char *nameAlt);

private:
	BaseType *ptr_;
};

////////////////////////////////////////////////////////////////////
// Умный указатель, не требующий описания класса.
// Класс должен наследовать PolymorphicBase
////////////////////////////////////////////////////////////////////
template<class Type>
class PolymorphicHandle : public ShareHandle<Type, PolymorphicBase> {
	typedef ShareHandle<Type, PolymorphicBase>  BaseClass;

public:
	PolymorphicHandle(Type *p = 0) : BaseClass(p) {}
};


////////////////////////////////////////////////////////////////////
//	Простейший синглетон
//  Два способа использования:
//
//	typedef Singleton<MyClass> MyClassSingle; // header
//	MyClassSingle::instance().xxx
//
//	extern Singleton<MyClass> myClass; // header
//	Singleton<MyClass> myClass; // cpp
//	myClass().xxx
////////////////////////////////////////////////////////////////////
template<class T>
class Singleton {
public:
	static T &instance() {
		static T *t;
		if (!t) {
			static T tt;
			t = &tt;
		}
		return *t;
	}

	T &operator()() const {
		return instance();
	}
};


} // namespace QDEngine

#endif // __HANDLE_H__
