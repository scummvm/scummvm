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

#include "m4/core/globals.h"

namespace M4 {

Globals::Globals() {
	Common::fill(&_numData[0], &_numData[GLB_SCRATCH_VARS], 0);
	Common::fill(&_ptrData[0], &_ptrData[GLB_SCRATCH_VARS], (void *)nullptr);
	Common::fill(&_isPtr[0], &_isPtr[GLB_SHARED_VARS], false);
}

} // namespace M4
