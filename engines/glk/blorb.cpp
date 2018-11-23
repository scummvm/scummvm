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

#include "glk/blorb.h"

namespace Glk {

enum {
	ID_FORM = MKTAG('F', 'O', 'R', 'M'),
	ID_IFRS = MKTAG('I', 'F', 'R', 'S'),
	ID_RIdx = MKTAG('R', 'I', 'd', 'x'),

	ID_Snd = MKTAG('S', 'n', 'd', ' '),
	ID_Exec = MKTAG('E', 'x', 'e', 'c'),
	ID_Pict = MKTAG('P', 'i', 'c', 't'),
	ID_Data = MKTAG('D', 'a', 't', 'a'),

	ID_Copyright = MKTAG('(', 'c', ')', ' '),
	ID_AUTH = MKTAG('A', 'U', 'T', 'H'),
	ID_ANNO = MKTAG('A', 'N', 'N', 'O'),

	ID_JPEG = MKTAG('J', 'P', 'E', 'G'),
	ID_PNG  = MKTAG('P', 'N', 'G', ' '),
	ID_Rect = MKTAG('R', 'e', 'c', 't'),

	ID_MIDI = MKTAG('M', 'I', 'D', 'I'),
	ID_MP3 = MKTAG('M', 'P', '3', ' '),
	ID_WAVE = MKTAG('W', 'A', 'V', 'E'),
	ID_AIFF = MKTAG('A', 'I', 'F', 'F'),
	ID_OGG = MKTAG('O', 'G', 'G', ' '),
	ID_MOD = MKTAG('M', 'O', 'D', ' '),
};

/*--------------------------------------------------------------------------*/

Blorb::Blorb(const Common::String &filename, InterpreterType interpType) :
		Common::Archive(), _filename(filename), _interpType(interpType) {
	if (load() != Common::kNoError)
		error("Could not parse blorb file");
}

bool Blorb::hasFile(const Common::String &name) const {
	for (uint idx = 0; idx < _chunks.size(); ++idx) {
		if (_chunks[idx]._filename.equalsIgnoreCase(name))
			return true;
	}

	return false;
}

int Blorb::listMembers(Common::ArchiveMemberList &list) const {
	for (uint idx = 0; idx < _chunks.size(); ++idx) {
		list.push_back(Common::ArchiveMemberList::value_type(new Common::GenericArchiveMember(_chunks[idx]._filename, this)));
	}

	return (int)_chunks.size();
}

const Common::ArchiveMemberPtr Blorb::getMember(const Common::String &name) const {
	if (!hasFile(name))
		return Common::ArchiveMemberPtr();

	return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(name, this));
}

Common::SeekableReadStream *Blorb::createReadStreamForMember(const Common::String &name) const {
	for (uint idx = 0; idx < _chunks.size(); ++idx) {
		if (_chunks[idx]._filename.equalsIgnoreCase(name)) {
			Common::File f;
			if (!f.open(_filename))
				error("Reading failed");

			f.seek(_chunks[idx]._offset);
			Common::SeekableReadStream *result = f.readStream(_chunks[idx]._size);
			f.close();

			return result;
		}
	}

	return nullptr;
}

Common::ErrorCode Blorb::load() {
	// First, chew through the file and index the chunks
	Common::File f;
	if (!f.open(_filename) || f.size() < 12)
		return Common::kReadingFailed;

	if (f.readUint32BE() != ID_FORM)
		return Common::kReadingFailed;
	f.readUint32BE();
	if (f.readUint32BE() != ID_IFRS)
		return Common::kReadingFailed;
	if (f.readUint32BE() != ID_RIdx)
		return Common::kReadingFailed;

	f.readUint32BE();
	uint count = f.readUint32BE();

	// First read in the resource index
	for (uint idx = 0; idx < count; ++idx) {
		ChunkEntry ce;
		ce._type = f.readUint32BE();
		ce._number = f.readUint32BE();
		ce._offset = f.readUint32BE();

		_chunks.push_back(ce);
	}

	// Further iterate through the resources
	for (uint idx = 0; idx < _chunks.size(); ++idx) {
		ChunkEntry &ce = _chunks[idx];
		f.seek(ce._offset);
		ce._offset += 8;

		ce._id = f.readUint32BE();
		ce._size = f.readUint32BE();

		if (ce._type == ID_Pict) {
			ce._filename = Common::String::format("pic%u", ce._number);
			if (ce._id == ID_JPEG)
				ce._filename += ".jpg";
			else if (ce._id == ID_PNG)
				ce._filename += ".png";
			else if (ce._id == ID_Rect)
				ce._filename += ".rect";

		} else if (ce._type == ID_Snd) {
			ce._filename = Common::String::format("snd%u", ce._number);
			if (ce._id == ID_MIDI)
				ce._filename += ".midi";
			else if (ce._id == ID_MP3)
				ce._filename += ".mp3";
			else if (ce._id == ID_WAVE)
				ce._filename += ".wav";
			else if (ce._id == ID_AIFF)
				ce._filename += ".aiff";
			else if (ce._id == ID_OGG)
				ce._filename += ".ogg";
			else if (ce._id == ID_MOD)
				ce._filename += ".mod";

		} else if (ce._type == ID_Data) {
			ce._filename = Common::String::format("data%u", ce._number);

		} else if (ce._type == ID_Exec) {
			char buffer[5];
			WRITE_BE_UINT32(buffer, ce._id);
			buffer[4] = '\0';
			Common::String type(buffer);

			if (
				(_interpType == INTERPRETER_FROTZ && type == "ZCOD") ||
				(_interpType == INTERPRETER_TADS && (type == "TAD2" || type == "TAD3")) ||
				(_interpType == INTERPRETER_HUGO && type == "HUGO")
			) {
				// Game executable
				ce._filename = "game";
			} else {
				ce._filename = type;
			}
		}
	}

	return Common::kNoError;
}

} // End of namespace Glk
