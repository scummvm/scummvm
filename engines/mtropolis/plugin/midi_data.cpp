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

#include "mtropolis/plugin/midi.h"
#include "mtropolis/plugin/midi_data.h"

namespace MTropolis {

namespace Data {

namespace Midi {

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

		if (!reader.readU8(modeSpecific.embedded.loop) || !reader.readU8(modeSpecific.embedded.overrideTempo) || !reader.readU8(modeSpecific.embedded.volume) || !embeddedTempo.load(reader) || !embeddedFadeIn.load(reader) || !embeddedFadeOut.load(reader))
			return kDataReadErrorReadFailed;
	} else {
		if (!reader.readU8(modeSpecific.singleNote.channel) || !reader.readU8(modeSpecific.singleNote.note) || !reader.readU8(modeSpecific.singleNote.velocity) || !reader.readU8(modeSpecific.singleNote.program) || !singleNoteDuration.load(reader))
			return kDataReadErrorReadFailed;
	}

	return kDataReadErrorNone;
}

} // End of namespace Midi

} // End of namespace Data

} // End of namespace MTropolis
