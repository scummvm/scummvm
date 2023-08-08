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

#ifndef CRAB_BACKINSERTER_H
#define CRAB_BACKINSERTER_H

#include "common/util.h"

namespace Crab {

template<class Container>
class backInserterIterator {
protected:
	Container *_container;

public:
	explicit backInserterIterator(Container &x) : _container(&x) {}

	backInserterIterator &operator=(const typename Container::value_type &value) {
		*_container += value;
		return *this;
	}

	backInserterIterator &operator=(const typename Container::value_type &&value) {
		*_container += Common::move(value);
		return *this;
	}

	backInserterIterator &operator*() { return *this; }
	backInserterIterator &operator++() { return *this; }
	backInserterIterator &operator++(int) { return *this; }
};

template<class Container>
backInserterIterator<Container>
backInserter(Container &x) {
	return backInserterIterator<Container>(x);
}

} // End of namespace Crab

#endif
