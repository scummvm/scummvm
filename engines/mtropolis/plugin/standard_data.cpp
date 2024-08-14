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

#include "mtropolis/plugin/standard_data.h"
#include "mtropolis/plugin/standard.h"

namespace MTropolis {

namespace Data {

namespace Standard {

CursorModifier::CursorModifier() : haveRemoveWhen(false) {
}

DataReadErrorCode CursorModifier::load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision != 0 && prefix.plugInRevision != 1 && prefix.plugInRevision != 2)
		return kDataReadErrorUnsupportedRevision;

	if (!applyWhen.load(reader))
		return kDataReadErrorReadFailed;

	if (prefix.plugInRevision >= 1) {
		if (!removeWhen.load(reader))
			return kDataReadErrorReadFailed;
		haveRemoveWhen = true;
	} else {
		removeWhen.type = PlugInTypeTaggedValue::kNull;
		haveRemoveWhen = false;
	}

	if (!cursorIDAsLabel.load(reader))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

DataReadErrorCode STransCtModifier::load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision != 0)
		return kDataReadErrorUnsupportedRevision;

	if (!enableWhen.load(reader) || !disableWhen.load(reader) || !transitionType.load(reader) ||
		!transitionDirection.load(reader) || !unknown1.load(reader) || !steps.load(reader) ||
		!duration.load(reader) || !fullScreen.load(reader))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

MediaCueMessengerModifier::MediaCueMessengerModifier()
	: unknown1(0), destination(0), unknown2(0) {
}

DataReadErrorCode MediaCueMessengerModifier::load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision != 1)
		return kDataReadErrorUnsupportedRevision;

	if (!enableWhen.load(reader) || !disableWhen.load(reader) || !sendEvent.load(reader)
		|| !nonStandardMessageFlags.load(reader) || !reader.readU16(unknown1) || !reader.readU32(destination)
		|| !reader.readU32(unknown2) || !with.load(reader) || !executeAt.load(reader)
		|| !triggerTiming.load(reader))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

DataReadErrorCode ObjectReferenceVariableModifier::load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision != 0 && prefix.plugInRevision != 2)
		return kDataReadErrorUnsupportedRevision;

	if (!setToSourceParentWhen.load(reader))
		return kDataReadErrorReadFailed;

	if (prefix.plugInRevision == 0) {
		unknown1.type = Data::PlugInTypeTaggedValue::kNull;
		if (!objectPath.load(reader))
			return kDataReadErrorReadFailed;
	} else {
		if (!unknown1.load(reader))
			return kDataReadErrorReadFailed;

		bool hasNoPath = (unknown1.type == Data::PlugInTypeTaggedValue::kInteger && unknown1.value.asInt == 0);
		if (hasNoPath)
			objectPath.type = Data::PlugInTypeTaggedValue::kNull;
		else if (!objectPath.load(reader))
			return kDataReadErrorReadFailed;
	}

	return kDataReadErrorNone;
}

ListVariableModifier::ListVariableModifier() : unknown1(0), contentsType(0), unknown2{0, 0, 0, 0},
	havePersistentData(false), numValues(0), values(nullptr), persistentValuesGarbled(false) {
}

ListVariableModifier::~ListVariableModifier() {
	if (values)
		delete[] values;
}

DataReadErrorCode ListVariableModifier::load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision < 1 || prefix.plugInRevision > 3)
		return kDataReadErrorUnsupportedRevision;

	int64 privateDataPos = reader.tell();

	if (!reader.readU16(unknown1) || !reader.readU32(contentsType) || !reader.readBytes(unknown2))
		return kDataReadErrorReadFailed;

	persistentValuesGarbled = false;

	if (prefix.plugInRevision == 1 || prefix.plugInRevision == 3) {
		if (prefix.plugInRevision == 1) {
			havePersistentData = true;
		} else if (prefix.plugInRevision == 3) {
			PlugInTypeTaggedValue persistentFlag;
			if (!persistentFlag.load(reader) || persistentFlag.type != PlugInTypeTaggedValue::kBoolean)
				return kDataReadErrorReadFailed;

			havePersistentData = (persistentFlag.value.asBoolean != 0);
		}

		if (havePersistentData) {
			PlugInTypeTaggedValue numValuesVar;
			if (!numValuesVar.load(reader) || numValuesVar.type != PlugInTypeTaggedValue::kInteger || numValuesVar.value.asInt < 0)
				return kDataReadErrorReadFailed;

			numValues = static_cast<uint32>(numValuesVar.value.asInt);

			values = new PlugInTypeTaggedValue[numValues];
			for (size_t i = 0; i < numValues; i++) {
				if (!values[i].load(reader)) {
					if (static_cast<const MTropolis::Standard::StandardPlugIn &>(plugIn).getHacks().allowGarbledListModData) {
						persistentValuesGarbled = true;
						if (!reader.seek(privateDataPos + prefix.subObjectSize))
							return kDataReadErrorReadFailed;
						break;
					} else {
						return kDataReadErrorReadFailed;
					}
				}
			}
		} else {
			numValues = 0;
			values = nullptr;
		}
	} else {
		havePersistentData = false;
		numValues = 0;
		values = nullptr;
	}

	return kDataReadErrorNone;
}

DataReadErrorCode PanningModifier::load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision != 3)
		return kDataReadErrorUnsupportedRevision;

	if (!unknown1Event.load(reader) || !unknown2Event.load(reader) || !unknown3Int.load(reader) || !unknown4Int.load(reader) || !unknown5Int.load(reader))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

DataReadErrorCode SysInfoModifier::load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision != 0)
		return kDataReadErrorUnsupportedRevision;

	return kDataReadErrorNone;
}

DataReadErrorCode FadeModifier::load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision != 1)
		return kDataReadErrorUnsupportedRevision;

	if (!unknown1Event.load(reader) || !unknown2Event.load(reader) || !unknown3Int.load(reader) || !unknown4Int.load(reader) || !unknown5Int.load(reader))
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

} // End of namespace Standard

} // End of namespace Data

} // End of namespace MTropolis
