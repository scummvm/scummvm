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

#include "common/textconsole.h"
#include "tetraedge/game/billboard.h"

namespace Tetraedge {

Billboard::Billboard() {
}

bool Billboard::load(const Common::String &path) {
	error("TODO: implement Billboard::load");
	return false;
}

void Billboard::calcVertex() {
	error("TODO: implement Billboard::calcVertex");
}

void Billboard::position(const TeVector3f32 &pos) {
	_pos = pos;
	calcVertex();
}

void Billboard::position2(const TeVector3f32 &pos) {
	_pos2 = pos;
	_hasPos2 = true;
	calcVertex();
}

void Billboard::size(const TeVector2f32 &size) {
	_size = size;
	calcVertex();
}

} // end namespace Tetraedge
