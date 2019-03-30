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

/*--------------------------------------------------------------------------*/

Blorb::Blorb(const Common::String &filename, InterpreterType interpType) :
		Common::Archive(), _filename(filename), _interpType(interpType) {
	if (load() != Common::kNoError)
		error("Could not parse blorb file");
}

Blorb::Blorb(const Common::FSNode &fileNode, InterpreterType interpType) :
		Common::Archive(), _fileNode(fileNode), _interpType(interpType) {
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
			if ((!_filename.empty() && !f.open(_filename)) ||
					(_filename.empty() && !f.open(_fileNode)))
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
	if ((!_filename.empty() && !f.open(_filename)) ||
			(_filename.empty() && !f.open(_fileNode)))
		return Common::kReadingFailed;

	if (!isBlorb(f))
		return Common::kReadingFailed;

	if (!readRIdx(f, _chunks))
		return Common::kReadingFailed;

	// Further iterate through the resources
	for (uint idx = 0; idx < _chunks.size(); ++idx) {
		ChunkEntry &ce = _chunks[idx];

		if (ce._type == ID_Pict) {
			ce._filename = Common::String::format("pic%u", ce._number);
			if (ce._id == ID_JPEG)
				ce._filename += ".jpg";
			else if (ce._id == ID_PNG)
				ce._filename += ".png";
			else if (ce._id == ID_Rect)
				ce._filename += ".rect";

		} else if (ce._type == ID_Snd) {
			ce._filename = Common::String::format("sound%u", ce._number);
			if (ce._id == ID_MIDI)
				ce._filename += ".midi";
			else if (ce._id == ID_MP3)
				ce._filename += ".mp3";
			else if (ce._id == ID_WAVE)
				ce._filename += ".wav";
			else if (ce._id == ID_AIFF || ce._id == ID_FORM)
				ce._filename += ".aiff";
			else if (ce._id == ID_OGG)
				ce._filename += ".ogg"; 
			else if (ce._id == ID_MOD)
				ce._filename += ".mod";

		} else if (ce._type == ID_Data) {
			ce._filename = Common::String::format("data%u", ce._number);

		} else if (ce._type == ID_Exec) {
			if (
				(_interpType == INTERPRETER_FROTZ && ce._id == ID_ZCOD) ||
				(_interpType == INTERPRETER_GLULXE && ce._id == ID_GLUL) ||
				(_interpType == INTERPRETER_TADS2 && ce._id == ID_TAD2) ||
				(_interpType == INTERPRETER_TADS3 && ce._id == ID_TAD3) ||
				(_interpType == INTERPRETER_HUGO && ce._id == ID_HUGO) ||
				(_interpType == INTERPRETER_SCOTT && ce._id == ID_SAAI)
			) {
				// Game executable
				ce._filename = "game";
			} else {
				char buffer[5];
				WRITE_BE_UINT32(buffer, ce._id);
				buffer[4] = '\0';
				Common::String type(buffer);
				ce._filename = type;
			}
		}
	}

	return Common::kNoError;
}

bool Blorb::readRIdx(Common::SeekableReadStream &stream, Common::Array<ChunkEntry> &chunks) {
	if (stream.readUint32BE() != ID_RIdx)
		return false;

	stream.readUint32BE();
	uint count = stream.readUint32BE();

	// First read in the resource index
	for (uint idx = 0; idx < count; ++idx) {
		ChunkEntry ce;
		ce._type = stream.readUint32BE();
		ce._number = stream.readUint32BE();
		ce._offset = stream.readUint32BE();

		chunks.push_back(ce);
	}

	// Further iterate through the resources
	for (uint idx = 0; idx < chunks.size(); ++idx) {
		ChunkEntry &ce = chunks[idx];
		stream.seek(ce._offset);
		ce._offset += 8;

		ce._id = stream.readUint32BE();
		ce._size = stream.readUint32BE();
	}

	return true;
}

bool Blorb::isBlorb(Common::SeekableReadStream &stream, uint32 type) {
	if (stream.size() < 12)
		return false;
	if (stream.readUint32BE() != ID_FORM)
		return false;
	stream.readUint32BE();
	if (stream.readUint32BE() != ID_IFRS)
		return false;

	if (type == 0)
		return true;

	Common::Array<ChunkEntry> chunks;
	if (!readRIdx(stream, chunks))
		return false;

	// Further iterate through the resources
	for (uint idx = 0; idx < chunks.size(); ++idx) {
		ChunkEntry &ce = chunks[idx];
		if (ce._type == ID_Exec && ce._id == type)
			return true;
	}

	return false;
}

bool Blorb::isBlorb(const Common::String &filename, uint32 type) {
	Common::File f;
	if (!filename.empty() && !f.open(filename))
		return false;

	return isBlorb(f, type);
}

} // End of namespace Glk
