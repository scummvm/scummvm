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

#include "mtropolis/plugin/mti_data.h"

namespace MTropolis {

namespace Data {

namespace MTI {

DataReadErrorCode ShanghaiModifier::load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision != 0)
		return kDataReadErrorUnsupportedRevision;

	if (!unknown1Event.load(reader) || !unknown2VarRef.load(reader))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}


DataReadErrorCode PrintModifier::load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision != 0)
		return kDataReadErrorUnsupportedRevision;

	if (!executeWhen.load(reader) || !unknown1Bool.load(reader) || !unknown2Bool.load(reader) ||
		!unknown3Bool.load(reader) || !filePath.load(reader) || !unknown4Bool.load(reader))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

} // End of namespace MTI

} // End of namespace Data

} // End of namespace MTropolis
