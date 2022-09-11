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

CursorModifier::CursorModifier()
	: unknown1(0), applyWhen(Event::createDefault()), unknown2(0), removeWhen(Event::createDefault()),
	  unknown3(0), unknown4{0, 0, 0, 0}, cursorID(0) {
}

DataReadErrorCode CursorModifier::load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision != 1)
		return kDataReadErrorUnsupportedRevision;

	if (!reader.readU16(unknown1) || !applyWhen.load(reader) || !reader.readU16(unknown2)
		|| !removeWhen.load(reader) || !reader.readU16(unknown3) || !reader.readU32(cursorID) || !reader.readBytes(unknown4))
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

MidiModifier::MidiModifier() : embeddedFlag(0) {
	memset(&this->modeSpecific, 0, sizeof(this->modeSpecific));
}

DataReadErrorCode MidiModifier::load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision != 1 && prefix.plugInRevision != 2)
		return kDataReadErrorUnsupportedRevision;

	if (!executeWhen.load(reader) || !terminateWhen.load(reader) || !reader.readU8(embeddedFlag))
		return kDataReadErrorReadFailed;

	if (embeddedFlag) {
		if (!reader.readU8(modeSpecific.embedded.hasFile))
			return kDataReadErrorReadFailed;
		if (modeSpecific.embedded.hasFile) {
			embeddedFile = Common::SharedPtr<EmbeddedFile>(new EmbeddedFile());

			uint8 bigEndianLength[4];
			if (!reader.readBytes(bigEndianLength))
				return kDataReadErrorReadFailed;

			uint32 length = (bigEndianLength[0] << 24) + (bigEndianLength[1] << 16) + (bigEndianLength[2] << 8) + bigEndianLength[3];

			embeddedFile->contents.resize(length);
			if (length > 0 && !reader.read(&embeddedFile->contents[0], length))
				return kDataReadErrorReadFailed;
		}

		if (!reader.readU8(modeSpecific.embedded.loop) || !reader.readU8(modeSpecific.embedded.overrideTempo)
			|| !reader.readU8(modeSpecific.embedded.volume) || !embeddedTempo.load(reader)
			|| !embeddedFadeIn.load(reader) || !embeddedFadeOut.load(reader))
			return kDataReadErrorReadFailed;
	} else {
		if (!reader.readU8(modeSpecific.singleNote.channel) || !reader.readU8(modeSpecific.singleNote.note) || !reader.readU8(modeSpecific.singleNote.velocity)
			|| !reader.readU8(modeSpecific.singleNote.program) || !singleNoteDuration.load(reader))
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

DataReadErrorCode SysInfoModifier::load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision != 0)
		return kDataReadErrorUnsupportedRevision;

	return kDataReadErrorNone;
}

} // End of namespace Standard

} // End of namespace Data

} // End of namespace MTropolis
