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

#ifndef TITANIC_STAR_POINTS2_H
#define TITANIC_STAR_POINTS2_H

#include "common/array.h"

namespace Titanic {

class CStarPoints2 {
	struct DataEntry {
		int _v1;
		int _v2;
		int _v3;
	};

	class RootEntry : public Common::Array<DataEntry> {
	public:
		int _field0;
		RootEntry() : _field0(0) {}
	};
private:
	Common::Array<RootEntry> _data;
public:
	/**
	 * Initializes the data
	 */
	bool initialize();
};

} // End of namespace Titanic

#endif /* TITANIC_STAR_POINTS2_H */
