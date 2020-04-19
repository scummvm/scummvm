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

#ifndef ULTIMA4_CORE_OBSERVABLE_H
#define ULTIMA4_CORE_OBSERVABLE_H

#include "ultima/ultima4/core/observer.h"
#include "ultima/shared/std/containers.h"

namespace Ultima {
namespace Ultima4 {

/**
 * Classes can report updates to a list of decoupled Observers by
 * extending this class.
 *
 * The O class parameter should be a pointer to the class of the
 * observable itself, so it can be passed in a typesafe manner to the
 * observers update method.
 *
 * The A class can be any additional information to pass to observers.
 * Observables that don't need to pass an argument when they update
 * observers should use the default "NoArg" class for the second
 * template parameter and pass nullptr to notifyObservers.
 */
template <class O, class A = NoArg *>
class Observable {
public:
	Observable() : _changed(false) {}

	void addObserver(Observer<O, A> *o) {
		typename Std::vector< Observer<O, A> *>::iterator i;
		i = Common::find(_observers.begin(), _observers.end(), o);
		if (i == _observers.end())
			_observers.push_back(o);
	}

	int countObservers() const {
		return _observers.size();
	}

	void deleteObserver(Observer<O, A> *o) {
		typename Std::vector< Observer<O, A> *>::iterator i;
		i = Common::find(_observers.begin(), _observers.end(), o);
		if (i != _observers.end())
			_observers.erase(i);
	}

	void deleteObservers() {
		_observers.clear();
	}

	bool hasChanged() const {
		return _changed;
	}

	void notifyObservers(A arg) {
		if (!_changed)
			return;

		// vector iterators are invalidated if erase is called, so a copy
		// is used to prevent problems if the observer removes itself (or
		// otherwise changes the observer list)
		typename Std::vector< Observer<O, A> *> tmp = _observers;
		typename Std::vector< Observer<O, A> *>::iterator i;

		clearChanged();

		for (i = tmp.begin(); i != tmp.end(); i++) {
			Observer<O, A> *observer = *i;
			observer->update(static_cast<O>(this), arg);
		}
	}

protected:
	void clearChanged() {
		_changed = false;
	}
	void setChanged() {
		_changed = true;
	}

private:
	bool _changed;
	Std::vector< Observer<O, A> *> _observers;
};

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
