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

#ifndef TETRAEDGE_TE_TE_RESOURCE_H
#define TETRAEDGE_TE_TE_RESOURCE_H

#include "common/path.h"
#include "tetraedge/te/te_object.h"
#include "tetraedge/te/te_references_counter.h"

namespace Tetraedge {

class TeResource : public TeObject, public TeReferencesCounter {
public:
	TeResource();
	virtual ~TeResource();

	void generateAccessName();

	const Common::Path &getAccessName() const {
		return _accessName;
	}

	void setAccessName(const Common::Path &name) {
		_accessName = name;
	}

private:
	Common::Path _accessName;
	static uint32 _idCounter;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_RESOURCE_H
