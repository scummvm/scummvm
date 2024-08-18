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

#include "mtropolis/plugin/mline_data.h"

namespace MTropolis {

namespace Data {

namespace MLine {

DataReadErrorCode MLineLauncherModifier::load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision != 0)
		return kDataReadErrorUnsupportedRevision;

	if (!unknown1String.load(reader) || !unknown2String.load(reader) || !unknown3String.load(reader) || !unknown4Int.load(reader) || !unknown5Int.load(reader))
		return kDataReadErrorReadFailed;

	if (!unknown6Bool.load(reader) || !unknown7Event.load(reader) || !unknown8Bool.load(reader) || !unknown9Bool.load(reader) || !unknown10Bool.load(reader))
		return kDataReadErrorReadFailed;

	if (!unknown11Bool.load(reader) || !unknown12Bool.load(reader))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

} // End of namespace MLine

} // End of namespace Data

} // End of namespace MTropolis
