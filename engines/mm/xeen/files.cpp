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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/scummsys.h"
#include "common/archive.h"
#include "common/memstream.h"
#include "common/substream.h"
#include "common/textconsole.h"
#include "mm/xeen/xeen.h"
#include "mm/xeen/files.h"
#include "mm/xeen/saves.h"
#include "mm/shared/utils/engine_data.h"

namespace MM {
namespace Xeen {

FileManager::FileManager(XeenEngine *vm) {
	_ccNum = vm->getGameID() == GType_DarkSide;
	_xeenCc = _darkCc = _introCc = nullptr;
	_xeenSave = _darkSave = nullptr;
	_currentSave = nullptr;
	_currentArchive = nullptr;
}

FileManager::~FileManager() {
	SearchMan.remove("intro");
	SearchMan.remove("data");
	delete _xeenCc;
	delete _darkCc;
}

bool FileManager::setup() {
	if (g_vm->getGameID() == GType_Swords) {
		_xeenCc = nullptr;
		_darkCc = new CCArchive("swrd.cc", "xeen", true);
	} else {
		_xeenCc = (g_vm->getGameID() == GType_DarkSide) ? nullptr :
			new CCArchive("xeen.cc", "xeen", true);
		_darkCc = (g_vm->getGameID() == GType_Clouds) ? nullptr :
			new CCArchive("dark.cc", "dark", true);
	}

	if (Common::File::exists("intro.cc")) {
		_introCc = new CCArchive("intro.cc", "intro", true);
		SearchMan.add("intro", _introCc);
	}

	_currentArchive = g_vm->getGameID() == GType_DarkSide || g_vm->getGameID() == GType_Swords ?
		_darkCc : _xeenCc;
	assert(_currentArchive);

	// Set up the engine data file
	Common::U32String errMsg;
	if (!MM::load_engine_data("xeen", 1, 1, errMsg)) {
		GUIErrorMessage(errMsg);
		return false;
	}

	return true;
}

void FileManager::setGameCc(int ccMode) {
	if (ccMode != 2) {
		if (g_vm->getGameID() != GType_WorldOfXeen)
			ccMode = g_vm->getGameID() == GType_Clouds ? 0 : 1;
	}

	File::setCurrentArchive(ccMode);
	_ccNum = ccMode != 0;
}

void FileManager::load(Common::SeekableReadStream &stream) {
	setGameCc(stream.readByte());
}

void FileManager::save(Common::WriteStream &s) {
	s.writeByte(_ccNum ? 1 : 0);
}

/*------------------------------------------------------------------------*/

SaveArchive::SaveArchive(Party *party) : BaseCCArchive(), _party(party), _data(nullptr), _dataSize(0) {
}

SaveArchive::~SaveArchive() {
	for (Common::HashMap<uint16, Common::MemoryWriteStreamDynamic *>::iterator it = _newData.begin(); it != _newData.end(); ++it) {
		delete (*it)._value;
	}
	delete[] _data;
}

Common::SeekableReadStream *SaveArchive::createReadStreamForMember(const Common::Path &path) const {
	Common::String name = path.toString();

	// If the given resource has already been perviously "written" to the
	// save manager, then return that new resource
	uint16 id = BaseCCArchive::convertNameToId(name);
	return createReadStreamForMember(id);
}

Common::SeekableReadStream *SaveArchive::createReadStreamForMember(uint16 id) const {
	if (_newData.contains(id)) {
		Common::MemoryWriteStreamDynamic *stream = _newData[id];
		return new Common::MemoryReadStream(stream->getData(), stream->size());
	}

	// Retrieve the resource from the loaded savefile
	CCEntry ccEntry;
	if (getHeaderEntry(id, ccEntry)) {
		// Open the correct CC entry
		return new Common::MemoryReadStream(_data + ccEntry._offset, ccEntry._size);
	}

	return nullptr;
}

void SaveArchive::load(Common::SeekableReadStream &stream) {
	_newData.clear();
	loadIndex(stream);

	delete[] _data;
	_dataSize = stream.size();
	_data = new byte[_dataSize];

	if (!stream.seek(0))
		error("Failed to seek to 0 in the save archive");

	if (!stream.read(_data, _dataSize))
		error("Failed to read %u bytes from save archive", _dataSize);
}

void SaveArchive::loadParty() {
	// Load in the character roster and active party
	Common::SeekableReadStream *chr = createReadStreamForMember("maze.chr");
	Common::Serializer sChr(chr, nullptr);
	_party->_roster.synchronize(sChr);
	delete chr;

	Common::SeekableReadStream *pty = createReadStreamForMember("maze.pty");
	Common::Serializer sPty(pty, nullptr);
	_party->synchronize(sPty);
	delete pty;
}

void SaveArchive::reset(CCArchive *src) {
	Common::MemoryWriteStreamDynamic saveFile(DisposeAfterUse::YES);
	File fIn;
	_newData.clear();

	g_vm->_files->setGameCc(g_vm->getGameID() == GType_DarkSide ? 1 : 0);
	const int RESOURCES[6] = { 0x2A0C, 0x2A1C, 0x2A2C, 0x2A3C, 0x284C, 0x2A5C };
	for (int i = 0; i < 6; ++i) {
		Common::String filename = Common::String::format("%.4x", RESOURCES[i]);
		if (src->hasFile(filename)) {
			// Read in the next resource
			fIn.open(filename, *src);

			size_t size = fIn.size();
			byte *data = new byte[size];

			if (fIn.read(data, size) != size) {
				delete[] data;
				error("Failed to read %zu bytes from resource '%s' in save archive", size, filename.c_str());
			}

			// Copy it to the combined savefile resource
			saveFile.write(data, size);
			delete[] data;
			fIn.close();
		}
	}

	assert(saveFile.size() > 0);
	Common::MemoryReadStream f(saveFile.getData(), saveFile.size());
	load(f);
}

void SaveArchive::save(Common::WriteStream &s) {
	// Save the character stats and active party
	OutFile chr("maze.chr", this);
	XeenSerializer sChr(nullptr, &chr);
	_party->_roster.synchronize(sChr);
	chr.finalize();

	OutFile pty("maze.pty", this);
	Common::Serializer sPty(nullptr, &pty);
	_party->synchronize(sPty);
	pty.finalize();

	// First caclculate new offsets and total filesize
	_dataSize = _index.size() * 8 + 2;
	for (uint idx = 0; idx < _index.size(); ++idx) {
		_index[idx]._writeOffset = (idx == 0) ? _dataSize :
			_index[idx - 1]._writeOffset + _index[idx - 1]._size;
		_dataSize += _index[idx]._size;
	}

	s.writeUint32LE(_dataSize);

	// Save out the index
	SubWriteStream dataStream(&s);
	saveIndex(dataStream);

	// Save out each resource in turn
	for (uint idx = 0; idx < _index.size(); ++idx) {
		// Get the entry
		Common::SeekableReadStream *entry = createReadStreamForMember(_index[idx]._id);

		size_t size = entry->size();
		byte *data = new byte[size];

		if (entry->read(data, size) != size) {
			delete[] data;
			delete entry;
			error("Failed to read %zu bytes from entry %hu", size, _index[idx]._id);
		}

		// Write it out to the savegame
		assert(dataStream.pos() == _index[idx]._writeOffset);
		dataStream.write(data, size);
		delete[] data;
		delete entry;
	}
}

void SaveArchive::replaceEntry(uint16 id, const byte *data, size_t size) {
	// Delete any prior set entry
	if (_newData.contains(id))
		delete _newData[id];

	// Create a new entry and write out the data to it
	Common::MemoryWriteStreamDynamic *out = new Common::MemoryWriteStreamDynamic(DisposeAfterUse::YES);
	out->write(data, size);
	_newData[id] = out;

	// Update the index with the entry's size for later convenience when creating savegames
	for (uint idx = 0; idx < _index.size(); ++idx) {
		if (_index[idx]._id == id) {
			_index[idx]._size = size;
			break;
		}
	}
}

/*------------------------------------------------------------------------*/

OutFile::OutFile(const Common::String &filename) :
		_filename(filename), _backingStream(DisposeAfterUse::YES) {
	FileManager &files = *g_vm->_files;
	_archive = files._currentSave;
}

OutFile::OutFile(const Common::String &filename, SaveArchive *archive) :
	_filename(filename), _archive(archive), _backingStream(DisposeAfterUse::YES) {
}

OutFile::OutFile(const Common::String &filename, int ccMode) :
		_filename(filename), _backingStream(DisposeAfterUse::YES) {
	FileManager &files = *g_vm->_files;
	g_vm->_files->setGameCc(ccMode);
	_archive = files._currentSave;
}

uint32 OutFile::write(const void *dataPtr, uint32 dataSize) {
	return _backingStream.write(dataPtr, dataSize);
}

int64 OutFile::pos() const {
	return _backingStream.pos();
}

void OutFile::finalize() {
	uint16 id = BaseCCArchive::convertNameToId(_filename);

	_archive->replaceEntry(id, _backingStream.getData(), _backingStream.size());
}

} // End of namespace Xeen
} // End of namespace MM
