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

template<class Arg, class Res>
struct Functor1 : public Common::UnaryFunction<Arg, Res> {
	virtual operator bool() const = 0;
	virtual Res operator()(Arg) const = 0;
};

template<class Arg, class Res, class T>
class Functor1Mem : public Functor1<Arg, Res> {
public:
	typedef Res (T::*FuncType)(Arg);
	
	Functor1Mem(T *t, const FuncType &func) : _t(t), _func(func) {}
	
	operator bool() const { return _func != 0; }
	Res operator()(Arg v1) const {
		return (_t->*_func)(v1);
	}
private:
	mutable T *_t;
	Res (T::*_func)(Arg);
};

struct ScriptState;

typedef Functor1<ScriptState*, int> Opcode;

} // end of namespace Kyra

#endif
