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
 */

#ifndef ULTIMA4_CORE_OBSERVER_H
#define ULTIMA4_CORE_OBSERVER_H

namespace Ultima {
namespace Ultima4 {

template<class O, class A>
class Observable;
class NoArg;

/**
 * This is the interface a class must implement to watch an
 * Observable.
 */
template<class O, class A = NoArg *>
class Observer {
public:
	virtual void update(O observable, A arg) = 0;
	virtual ~Observer() {}
};

/**
 * A specialized observer for watching observables that don't use the
 * "arg" parameter to update.
 */
template<class O>
class Observer<O, NoArg *> {
public:
	virtual void update(O observable, NoArg *arg) {
		update(observable);
	}
	virtual void update(O observable) = 0;
	virtual ~Observer() {}
};

/**
 * Just an empty marker class to identify observers that take no args
 * on update.
 */
class NoArg {
};

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
