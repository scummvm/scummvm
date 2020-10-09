/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef COMMON_FOREACH_H
#define COMMON_FOREACH_H

#include "common/scummsys.h"

#if __cplusplus < 201103L

namespace Common {

class _Foreach_Container_Base_ {
public:
	_Foreach_Container_Base_() : brk(1) { }
	mutable uint brk;
};

template<class T>
class _Foreach_Container_ : public _Foreach_Container_Base_ {
public:
	_Foreach_Container_(const T &co) : i(co.begin()), e(co.end()) { }
	void next() const {
		++i;
		brk = 1;
	}
	bool end() const { return i == e; }

	mutable typename T::const_iterator i;
	typename T::const_iterator e;
};

template<class T>
inline _Foreach_Container_<T> _Create_Foreach_Container_(const T &c) {
	return _Foreach_Container_<T>(c);
}

template<class T>
inline const _Foreach_Container_<T> *_Get_Foreach_Container_(const _Foreach_Container_Base_ *c, const T &) {
	return static_cast<const _Foreach_Container_<T> *>(c);
}

}

#define foreach(var, container) \
for (const Common::_Foreach_Container_Base_ &_FOREACH_CONTAINER_ = Common::_Create_Foreach_Container_(container);\
	!Common::_Get_Foreach_Container_(&_FOREACH_CONTAINER_, container)->end(); \
	Common::_Get_Foreach_Container_(&_FOREACH_CONTAINER_, container)->next()) \
	for (var = *Common::_Get_Foreach_Container_(&_FOREACH_CONTAINER_, container)->i;\
		_FOREACH_CONTAINER_.brk > 0; --_FOREACH_CONTAINER_.brk)

#else

#define foreach(var, container) for (var : container)

#endif

#endif
