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
#include "common/memstream.h"

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
		const ChunkEntry &ce = _chunks[idx];

		if (ce._filename.equalsIgnoreCase(name)) {
			Common::File f;
			if ((!_filename.empty() && !f.open(_filename)) ||
					(_filename.empty() && !f.open(_fileNode)))
				error("Reading failed");

			f.seek(ce._offset);
			Common::SeekableReadStream *result;
			
			if (ce._id == ID_FORM) {
				// AIFF chunks need to be wrapped in a FORM chunk for ScummVM decoder
				byte *sound = (byte *)malloc(ce._size + 8);
				WRITE_BE_UINT32(sound, MKTAG('F', 'O', 'R', 'M'));
				WRITE_BE_UINT32(sound + 4, 0);
				f.read(sound + 8, ce._size);
				assert(READ_BE_UINT32(sound + 8) == ID_AIFF);

				result = new Common::MemoryReadStream(sound, ce._size + 8, DisposeAfterUse::YES);
			} else {
				result = f.readStream(ce._size);
			}

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
				(_interpType == INTERPRETER_ADRIFT && ce._id == ID_ADRI) ||
				(_interpType == INTERPRETER_GLULX && ce._id == ID_GLUL) ||
				(_interpType == INTERPRETER_HUGO && ce._id == ID_HUGO) ||
				(_interpType == INTERPRETER_SCOTT && ce._id == ID_SAAI) ||
				(_interpType == INTERPRETER_TADS2 && ce._id == ID_TAD2) ||
				(_interpType == INTERPRETER_TADS3 && ce._id == ID_TAD3) ||
				(_interpType == INTERPRETER_ZCODE && ce._id == ID_ZCOD)
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

	// Check through any optional remaining chunks for an adaptive palette list
	while (f.pos() < f.size()) {
		uint chunkId = f.readUint32BE();
		uint chunkSize = f.readUint32BE();

		if (chunkId == ID_APal && chunkSize > 0) {
			// Found one, so create an entry so it can be opened as file named "apal"
			ChunkEntry ce;
			ce._filename = "apal";
			ce._offset = f.pos();
			ce._size = chunkSize;
			ce._type = ID_APal;
			_chunks.push_back(ce);
			break;
		}

		if (chunkSize & 1)
			++chunkSize;
		f.skip(chunkSize);
	}

	return Common::kNoError;
}

bool Blorb::readRIdx(Common::SeekableReadStream &stream, Common::Array<ChunkEntry> &chunks) {
	if (stream.readUint32BE() != ID_RIdx)
		return false;

	uint chunkLen = stream.readUint32BE();
	uint count = stream.readUint32BE();
	assert(count == (chunkLen - 4) / 12);

	// First read in the resource index
	for (uint idx = 0; idx < count; ++idx) {
		ChunkEntry ce;
		ce._type = stream.readUint32BE();
		ce._number = stream.readUint32BE();
		ce._offset = stream.readUint32BE();

		chunks.push_back(ce);
	}

	// Temporarily store the start of the next chunk of the file (if any)
	size_t nextChunkOffset = stream.pos();

	// Further iterate through the resources
	for (uint idx = 0; idx < chunks.size(); ++idx) {
		ChunkEntry &ce = chunks[idx];
		stream.seek(ce._offset);
		ce._offset += 8;

		ce._id = stream.readUint32BE();
		ce._size = stream.readUint32BE();
	}

	// Reset back to the next chunk, and return that the index was successfully read
	stream.seek(nextChunkOffset);
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

bool Blorb::hasBlorbExt(const Common::String &filename) {
	return filename.hasSuffixIgnoreCase(".blorb") || filename.hasSuffixIgnoreCase(".zblorb")
		|| filename.hasSuffixIgnoreCase(".gblorb") || filename.hasSuffixIgnoreCase(".blb")
		|| filename.hasSuffixIgnoreCase(".zlb") || filename.hasSuffixIgnoreCase(".a3r");
}

void Blorb::getBlorbFilenames(const Common::String &srcFilename, Common::StringArray &filenames,
		InterpreterType interpType, const Common::String &gameId) {
	// Strip off the source filename extension
	Common::String filename = srcFilename;
	if (!filename.contains('.')) {
		filename += '.';
	} else {
		while (filename[filename.size() - 1] != '.')
			filename.deleteLastChar();
	}

	// Add in the different possible filenames
	filenames.clear();
	filenames.push_back(filename + "blorb");
	filenames.push_back(filename + "blb");

	switch (interpType) {
	case INTERPRETER_ALAN3:
		filenames.push_back(filename + "a3r");
		break;
	case INTERPRETER_GLULX:
		filenames.push_back(filename + "gblorb");
		break;
	case INTERPRETER_ZCODE:
		filenames.push_back(filename + "zblorb");
		getInfocomBlorbFilenames(filenames, gameId);
		break;
	default:
		break;
	}
}

void Blorb::getInfocomBlorbFilenames(Common::StringArray &filenames, const Common::String &gameId) {
	if (gameId == "beyondzork")
		filenames.push_back("beyondzork.blb");
	else if (gameId == "journey")
		filenames.push_back("journey.blb");
	else if (gameId == "lurkinghorror")
		filenames.push_back("lurking.blb");
	else if (gameId == "questforexcalibur")
		filenames.push_back("arthur.blb");
	else if (gameId == "sherlockriddle")
		filenames.push_back("sherlock.blb");
	else if (gameId == "shogun")
		filenames.push_back("shogun.blb");
	else if (gameId == "zork0")
		filenames.push_back("zorkzero.blb");
}

} // End of namespace Glk
