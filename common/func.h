/* Residual - Virtual machine to run LucasArts' 3D adventure games
 * Copyright (C) 2003-2008 The ScummVM-Residual Team (www.scummvm.org)
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

#ifndef COMMON_FUNC_H
#define COMMON_FUNC_H

#include "common/sys.h"

namespace Common {

template<class Arg, class Result>
struct UnaryFunction {
	typedef Arg ArgumenType;
	typedef Result ResultType;
};

template<class Arg1, class Arg2, class Result>
struct BinaryFunction {
	typedef Arg1 FirstArgumentType;
	typedef Arg2 SecondArgumentType;
	typedef Result ResultType;
};

template<class T>
struct EqualTo : public BinaryFunction<T, T, bool> {
	bool operator()(const T &x, const T &y) const { return x == y; }
};

template<class T>
struct Less : public BinaryFunction<T, T, bool> {
	bool operator()(const T &x, const T &y) const { return x < y; }
};

template<class T>
struct Greater : public BinaryFunction<T, T, bool> {
	bool operator()(const T &x, const T &y) const { return x > y; }
};

template<class Op>
class Binder1st : public UnaryFunction<typename Op::SecondArgumentType, typename Op::ResultType> {
private:
	Op _op;
	typename Op::FirstArgumentType _arg1;
public:
	Binder1st(const Op &op, const typename Op::FirstArgumentType &arg1) : _op(op), _arg1(arg1) {}

	typename Op::ResultType operator()(typename Op::SecondArgumentType v) const {
		return _op(_arg1, v);
	}
};

template<class Op, class T>
inline Binder1st<Op> bind1st(const Op &op, const T &t) {
	return Binder1st<Op>(op, t);
}

template<class Op>
class Binder2nd : public UnaryFunction<typename Op::FirstArgumentType, typename Op::ResultType> {
private:
	Op _op;
	typename Op::SecondArgumentType _arg2;
public:
	Binder2nd(const Op &op, const typename Op::SecondArgumentType &arg2) : _op(op), _arg2(arg2) {}

	typename Op::ResultType operator()(typename Op::FirstArgumentType v) const {
		return _op(v, _arg2);
	}
};

template<class Op, class T>
inline Binder2nd<Op> bind2nd(const Op &op, const T &t) {
	return Binder2nd<Op>(op, t);
}

template<class Arg, class Result>
class PointerToUnaryFunc : public UnaryFunction<Arg, Result> {
private:
	Result (*_func)(Arg);
public:
	typedef Result (*FuncType)(Arg);

	PointerToUnaryFunc(const FuncType &func) : _func(func) {}
	Result operator()(Arg v) const {
		return _func(v);
	}
};

template<class Arg1, class Arg2, class Result>
class PointerToBinaryFunc : public BinaryFunction<Arg1, Arg2, Result> {
private:
	Result (*_func)(Arg1, Arg2);
public:
	typedef Result (*FuncType)(Arg1, Arg2);

	PointerToBinaryFunc(const FuncType &func) : _func(func) {}
	Result operator()(Arg1 v1, Arg2 v2) const {
		return _func(v1, v2);
	}
};

template<class Arg, class Result>
inline PointerToUnaryFunc<Arg, Result> ptr_fun(Result (*func)(Arg)) {
	return PointerToUnaryFunc<Arg, Result>(func);
}

template<class Arg1, class Arg2, class Result>
inline PointerToBinaryFunc<Arg1, Arg2, Result> ptr_fun(Result (*func)(Arg1, Arg2)) {
	return PointerToBinaryFunc<Arg1, Arg2, Result>(func);
}

template<class Result, class T>
class MemFunc0 : public UnaryFunction<T*, Result> {
private:
	Result (T::*_func)();
public:
	typedef Result (T::*FuncType)();

	MemFunc0(const FuncType &func) : _func(func) {}
	Result operator()(T *v) const {
		return (v->*_func)();
	}
};

template<class Result, class T>
class ConstMemFunc0 : public UnaryFunction<T*, Result> {
private:
	Result (T::*_func)() const;
public:
	typedef Result (T::*FuncType)() const;

	ConstMemFunc0(const FuncType &func) : _func(func) {}
	Result operator()(T *v) const {
		return (v->*_func)();
	}
};

template<class Result, class Arg, class T>
class MemFunc1 : public BinaryFunction<T*, Arg, Result> {
private:
	Result (T::*_func)(Arg);
public:
	typedef Result (T::*FuncType)(Arg);

	MemFunc1(const FuncType &func) : _func(func) {}
	Result operator()(T *v1, Arg v2) const {
		return (v1->*_func)(v2);
	}
};

template<class Result, class Arg, class T>
class ConstMemFunc1 : public BinaryFunction<T*, Arg, Result> {
private:
	Result (T::*_func)(Arg) const;
public:
	typedef Result (T::*FuncType)(Arg) const;

	ConstMemFunc1(const FuncType &func) : _func(func) {}
	Result operator()(T *v1, Arg v2) const {
		return (v1->*_func)(v2);
	}
};

template<class Result, class T>
inline MemFunc0<Result, T> mem_fun(Result (T::*f)()) {
	return MemFunc0<Result, T>(f);
}

template<class Result, class T>
inline ConstMemFunc0<Result, T> mem_fun(Result (T::*f)() const) {
	return ConstMemFunc0<Result, T>(f);
}

template<class Result, class Arg, class T>
inline MemFunc1<Result, Arg, T> mem_fun(Result (T::*f)(Arg)) {
	return MemFunc1<Result, Arg, T>(f);
}

template<class Result, class Arg, class T>
inline ConstMemFunc1<Result, Arg, T> mem_fun(Result (T::*f)(Arg) const) {
	return ConstMemFunc1<Result, Arg, T>(f);
}

// functor code

template<class Res>
struct Functor0 {
	virtual ~Functor0() {}

	virtual bool isValid() const = 0;
	virtual Res operator()() const = 0;
};

template<class Res, class T>
class Functor0Mem : public Functor0<Res> {
public:
	typedef Res (T::*FuncType)();

	Functor0Mem(T *t, const FuncType &func) : _t(t), _func(func) {}

	bool isValid() const { return _func != 0; }
	Res operator()() const {
		return (_t->*_func)();
	}
private:
	mutable T *_t;
	const FuncType _func;
};

template<class Arg, class Res>
struct Functor1 : public Common::UnaryFunction<Arg, Res> {
	virtual ~Functor1() {}

	virtual bool isValid() const = 0;
	virtual Res operator()(Arg) const = 0;
};

template<class Arg, class Res, class T>
class Functor1Mem : public Functor1<Arg, Res> {
public:
	typedef Res (T::*FuncType)(Arg);

	Functor1Mem(T *t, const FuncType &func) : _t(t), _func(func) {}

	bool isValid() const { return _func != 0; }
	Res operator()(Arg v1) const {
		return (_t->*_func)(v1);
	}
private:
	mutable T *_t;
	const FuncType _func;
};

template<class Arg1, class Arg2, class Res>
struct Functor2 : public Common::BinaryFunction<Arg1, Arg2, Res> {
	virtual ~Functor2() {}

	virtual bool isValid() const = 0;
	virtual Res operator()(Arg1, Arg2) const = 0;
};

template<class Arg1, class Arg2, class Res, class T>
class Functor2Mem : public Functor2<Arg1, Arg2, Res> {
public:
	typedef Res (T::*FuncType)(Arg1, Arg2);

	Functor2Mem(T *t, const FuncType &func) : _t(t), _func(func) {}

	bool isValid() const { return _func != 0; }
	Res operator()(Arg1 v1, Arg2 v2) const {
		return (_t->*_func)(v1, v2);
	}
private:
	mutable T *_t;
	const FuncType _func;
};

/**
 * Base template for hash functor objects, used by HashMap.
 * This needs to be specialized for every type that you need to hash.
 */
template<typename T> struct Hash;


#define GENERATE_TRIVIAL_HASH_FUNCTOR(T) \
	template<> struct Hash<T> : public UnaryFunction<T, uint> { \
		uint operator()(T val) const { return (uint)val; } \
	}

GENERATE_TRIVIAL_HASH_FUNCTOR(bool);
GENERATE_TRIVIAL_HASH_FUNCTOR(char);
GENERATE_TRIVIAL_HASH_FUNCTOR(signed char);
GENERATE_TRIVIAL_HASH_FUNCTOR(unsigned char);
GENERATE_TRIVIAL_HASH_FUNCTOR(short);
GENERATE_TRIVIAL_HASH_FUNCTOR(int);
GENERATE_TRIVIAL_HASH_FUNCTOR(long);
GENERATE_TRIVIAL_HASH_FUNCTOR(unsigned short);
GENERATE_TRIVIAL_HASH_FUNCTOR(unsigned int);
GENERATE_TRIVIAL_HASH_FUNCTOR(unsigned long);

#undef GENERATE_TRIVIAL_HASH_FUNCTOR

}	// End of namespace Common

#endif

