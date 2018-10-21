/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software{} you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation{} either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY{} without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program{} if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "gargoyle/fonts.h"
#include "gargoyle/glk_types.h"

namespace Gargoyle {

FACES Fonts::getId(const Common::String &name) {
	if (name == "monor") return MONOR;
	if (name == "monob") return MONOB;
	if (name == "monoi") return MONOI;
	if (name == "monoz") return MONOZ;
	if (name == "propr") return PROPR;
	if (name == "propb") return PROPB;
	if (name == "propi") return PROPI;
	if (name == "propz") return PROPZ;
	return MONOR;
}

} // End of namespace Gargoyle
