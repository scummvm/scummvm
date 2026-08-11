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

#ifndef TETRAEDGE_GAME_LOC_FILE_H
#define TETRAEDGE_GAME_LOC_FILE_H

#include "common/str.h"
#include "common/fs.h"

#include "tetraedge/te/te_i_loc.h"
#include "tetraedge/tetraedge.h"

namespace Tetraedge {

class LocFile : public TeILoc {
public:
	LocFile();

	void load(const TetraedgeFSNode &fsnode);
	const Common::String *value(const Common::String &key) const;

};

} // end namespace Tetraedge

#endif // TETRAEDGE_GAME_LOC_FILE_H
