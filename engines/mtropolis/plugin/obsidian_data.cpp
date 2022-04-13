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

#include "mtropolis/plugin/obsidian_data.h"

namespace MTropolis {

namespace Data {

namespace Obsidian {

DataReadErrorCode MovementModifier::load(const PlugInModifier& prefix, DataReader& reader) {
	if (prefix.plugInRevision != 0)
		return kDataReadErrorUnsupportedRevision;

	if (!unknown1Event.load(reader)
		|| !unknown2Event.load(reader)
		|| !unknown3Point.load(reader)
		|| !unknown4Bool.load(reader)
		|| !unknown5Point.load(reader)
		|| !unknown6Int.load(reader)
		|| !unknown7Float.load(reader)
		|| !unknown8Int.load(reader)
		|| !unknown9Event.load(reader)
		|| !unknown10Label.load(reader)
		|| !unknown11Null.load(reader)
		|| !unknown12Int.load(reader))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

DataReadErrorCode RectShiftModifier::load(const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision != 1)
		return kDataReadErrorUnsupportedRevision;

	if (!unknown1Event.load(reader) || !unknown2Event.load(reader) || !unknown3Int.load(reader))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

} // End of namespace Obsidian

} // End of namespace Data

} // End of namespace MTropolis
