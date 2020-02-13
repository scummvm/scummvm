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

#include "glk/alan3/current.h"

namespace Glk {
namespace Alan3 {

/* PUBLIC DATA */
CurVars current;
bool gameStateChanged = FALSE;

void CurVars::synchronize(Common::Serializer &s) {
	s.syncAsSint32LE(syntax);
	s.syncAsSint32LE(verb);
	s.syncAsSint32LE(location);
	s.syncAsSint32LE(actor);
	s.syncAsSint32LE(instance);
	s.syncAsSint32LE(tick);
	s.syncAsSint32LE(score);
	s.syncAsSint32LE(visits);
	s.syncAsSint32LE(sourceLine);
	s.syncAsSint32LE(sourceFile);
	s.syncAsUint32LE(meta);
}

} // End of namespace Alan3
} // End of namespace Glk
