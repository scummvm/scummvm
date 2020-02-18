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

#ifndef WORLD_ACTORS_TREASUREINFO_H
#define WORLD_ACTORS_TREASUREINFO_H

#include "ultima/shared/std/containers.h"
#include "ultima/shared/std/string.h"

namespace Ultima {
namespace Ultima8 {

struct TreasureInfo {
	Std::string _special;
	double _chance;
	int _map;
	Std::vector<uint32> _shapes;
	Std::vector<uint32> _frames;
	unsigned int _minCount, _maxCount;

	TreasureInfo() : _chance(1), _map(0), _minCount(1), _maxCount(1) {}
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
