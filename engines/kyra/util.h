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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef KYRA_UTIL_H
#define KYRA_UTIL_H

#include "common/func.h"

namespace Kyra {

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
	Res (T::*_func)();
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
	Res (T::*_func)(Arg);
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
	Res (T::*_func)(Arg1, Arg2);
};

struct ScriptState;
typedef Functor1<ScriptState*, int> Opcode;

struct TIM;
typedef Functor2<const TIM*, const uint16*, int> TIMOpcode;

} // end of namespace Kyra

#endif

