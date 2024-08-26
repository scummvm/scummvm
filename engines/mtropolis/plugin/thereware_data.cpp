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

#include "mtropolis/plugin/thereware_data.h"

namespace MTropolis {

namespace Data {

namespace Thereware {

DataReadErrorCode RotatorModifier::load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision != 0)
		return kDataReadErrorUnsupportedRevision;

	if (!unknown1Event.load(reader) || !unknown2Event.load(reader) || !unknown3Bool.load(reader) || !unknown4Int.load(reader) || !unknown5Float.load(reader))
		return kDataReadErrorReadFailed;

	if (!unknown6Bool.load(reader) || !unknown7Point.load(reader) || !unknown8Int.load(reader) || !unknown9Bool.load(reader) || !unknown10Int.load(reader))
		return kDataReadErrorReadFailed;

	if (!unknown11Event.load(reader) || !unknown12Label.load(reader) || !unknown13Null.load(reader) || !unknown14Int.load(reader) || !unknown15Point.load(reader))
		return kDataReadErrorReadFailed;

	if (!unknown16Point.load(reader) || !unknown17Point.load(reader) || !unknown18Bool.load(reader) || !unknown19Point.load(reader) || !unknown20Int.load(reader))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

DataReadErrorCode TrackerModifier::load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision != 0)
		return kDataReadErrorUnsupportedRevision;

	if (!unknown1Event.load(reader) || !unknown2Event.load(reader) || !unknown3Label.load(reader) || !unknown4Int.load(reader) || !unknown5Int.load(reader))
		return kDataReadErrorReadFailed;

	if (!unknown6Label.load(reader) || !unknown7Bool.load(reader))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

DataReadErrorCode DoubleClickModifier::load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision != 0)
		return kDataReadErrorUnsupportedRevision;

	if (!unknown1UniversalTime.load(reader) || !unknown2Event.load(reader) || !unknown3Event.load(reader) || !unknown4Null.load(reader) || !unknown5Label.load(reader))
		return kDataReadErrorReadFailed;

	if (!unknown6Int.load(reader) || !unknown7Int.load(reader))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

DataReadErrorCode MouseTrapModifier::load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision != 0)
		return kDataReadErrorUnsupportedRevision;

	if (!unknown1UniversalTime.load(reader) || !unknown2Event.load(reader) || !unknown3Event.load(reader) || !unknown4Event.load(reader) || !unknown5Null.load(reader))
		return kDataReadErrorReadFailed;

	if (!unknown6Label.load(reader) || !unknown7Int.load(reader))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

DataReadErrorCode WrapAroundModifier::load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision != 1)
		return kDataReadErrorUnsupportedRevision;

	if (!unknown1Event.load(reader) || !unknown2Event.load(reader) || !unknown3Point.load(reader))
		return kDataReadErrorReadFailed;

	if (!unknown4Int.load(reader) || !unknown5Bool.load(reader) || !unknown6Int.load(reader))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

DataReadErrorCode EasyScrollerModifier::load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision != 1)
		return kDataReadErrorUnsupportedRevision;

	if (!unknown1Event.load(reader) || !unknown2Event.load(reader) || !unknown3Int.load(reader) || !unknown4Int.load(reader) || !unknown5Label.load(reader))
		return kDataReadErrorReadFailed;

	if (!unknown6Int.load(reader) || !unknown7Int.load(reader) || !unknown8Int.load(reader) || !unknown9Int.load(reader))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

DataReadErrorCode TimeLoopModifier::load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision != 0)
		return kDataReadErrorUnsupportedRevision;

	if (!unknown1Event.load(reader) || !unknown2Event.load(reader) || !unknown3Event.load(reader) || !unknown4Null.load(reader) || !unknown5Label.load(reader))
		return kDataReadErrorReadFailed;

	if (!unknown6Int.load(reader) || !unknown7Int.load(reader) || !unknown8UniversalTime.load(reader) || !unknown9Bool.load(reader))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

DataReadErrorCode GoThereModifier::load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision != 1)
		return kDataReadErrorUnsupportedRevision;

	if (!unknown1Event.load(reader) || !unknown2Event.load(reader) || !unknown3Point.load(reader))
		return kDataReadErrorReadFailed;

	if (!unknown4UniversalTime.load(reader) || !unknown5Event.load(reader) || !unknown6Label.load(reader))
		return kDataReadErrorReadFailed;

	if (!unknown7Null.load(reader) || !unknown8Int.load(reader) || !unknown9Int.load(reader))
		return kDataReadErrorReadFailed;

	if (!unknown10Bool.load(reader) || !unknown11Bool.load(reader))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

DataReadErrorCode RandomizerModifier::load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision != 0)
		return kDataReadErrorUnsupportedRevision;

	if (!unknown1Int.load(reader) || !unknown2Int.load(reader) || !unknown3Int.load(reader))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

DataReadErrorCode ConductorModifier::load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision != 1)
		return kDataReadErrorUnsupportedRevision;

	if (!unknown1Null.load(reader) || !unknown2Null.load(reader) || !unknown3Int.load(reader) || !unknown4Null.load(reader) || !unknown5Null.load(reader))
		return kDataReadErrorReadFailed;

	if (!unknown6Null.load(reader) || !unknown7Event.load(reader) || !unknown8Null.load(reader) || !unknown9Int.load(reader))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

DataReadErrorCode AlphaMaticModifier::load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision != 1)
		return kDataReadErrorUnsupportedRevision;

	error("Data structure loading for the AlphaMatic modifier is not implemented.");

	return kDataReadErrorNone;
}


} // End of namespace Thereware

} // End of namespace Data

} // End of namespace MTropolis
