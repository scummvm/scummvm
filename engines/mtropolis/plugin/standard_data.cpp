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

namespace MTropolis {

namespace Data {

namespace Standard {

DataReadErrorCode CursorModifier::load(const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision != 1)
		return kDataReadErrorUnsupportedRevision;

	if (!reader.readU16(unknown1) || !applyWhen.load(reader) || !reader.readU16(unknown2)
		|| !removeWhen.load(reader) || !reader.readU16(unknown3) || !reader.readU32(cursorID) || !reader.readBytes(unknown4))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

DataReadErrorCode STransCtModifier::load(const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision != 0)
		return kDataReadErrorUnsupportedRevision;

	if (!reader.readU16(unknown1) || !unknown2.load(reader) || !reader.readU16(unknown3) || !unknown4.load(reader)
		|| !reader.readU16(unknown5) || !reader.readU32(unknown6) || !reader.readU16(unknown7) || !reader.readU32(unknown8)
		|| !reader.readU16(unknown9) || !reader.readU32(unknown10) || !reader.readU16(unknown11) || !reader.readU32(unknown12)
		|| !reader.readU16(unknown13) || !reader.readU32(unknown14) || !reader.readU16(unknown15) || !reader.readBytes(unknown16))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

DataReadErrorCode MediaCueMessengerModifier::load(const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision != 1)
		return kDataReadErrorUnsupportedRevision;

	if (!enableWhen.load(reader) || !disableWhen.load(reader) || !sendEvent.load(reader)
		|| !nonStandardMessageFlags.load(reader) || !reader.readU16(unknown1) || !reader.readU32(destination)
		|| !reader.readU32(unknown2) || !with.load(reader) || !executeAt.load(reader)
		|| !triggerTiming.load(reader))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

DataReadErrorCode ObjectReferenceVariableModifier::load(const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision != 2)
		return kDataReadErrorUnsupportedRevision;

	if (!setToSourceParentWhen.load(reader) || !unknown1.load(reader) || !objectPath.load(reader))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

DataReadErrorCode MidiModifier::load(const PlugInModifier &prefix, DataReader &reader) {
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

} // End of namespace Standard

} // End of namespace Data

} // End of namespace MTropolis
