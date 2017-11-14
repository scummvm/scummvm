/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"
#include "common/algorithm.h"
#include "common/memstream.h"
#include "xeen/saves.h"
#include "xeen/files.h"
#include "xeen/xeen.h"

namespace Xeen {

OutFile::OutFile(XeenEngine *vm, const Common::String filename) :
		_vm(vm),
		_filename(filename),
		_backingStream(DisposeAfterUse::YES) {
}

uint32 OutFile::write(const void *dataPtr, uint32 dataSize) {
	return _backingStream.write(dataPtr, dataSize);
}

int32 OutFile::pos() const {
	return _backingStream.pos();
}

void OutFile::finalize() {
	uint16 id = BaseCCArchive::convertNameToId(_filename);

	if (!_vm->_saves->_newData.contains(id))
		_vm->_saves->_newData[id] = new Common::MemoryWriteStreamDynamic(DisposeAfterUse::YES);

	Common::MemoryWriteStreamDynamic *out = _vm->_saves->_newData[id];
	out->write(_backingStream.getData(), _backingStream.size());
}

/*------------------------------------------------------------------------*/

SavesManager::SavesManager(XeenEngine *vm, Party &party) :
		BaseCCArchive(), _vm(vm), _party(party) {
	SearchMan.add("saves", this, 0, false);
	_data = nullptr;
	_wonWorld = false;
	_wonDarkSide = false;
}

SavesManager::~SavesManager() {
	for (Common::HashMap<uint16, Common::MemoryWriteStreamDynamic *>::iterator it = _newData.begin(); it != _newData.end(); it++) {
		delete (*it)._value;
	}
	delete[] _data;
}

void SavesManager::syncBitFlags(Common::Serializer &s, bool *startP, bool *endP) {
	byte data = 0;

	int bitCounter = 0;
	for (bool *p = startP; p <= endP; ++p, bitCounter = (bitCounter + 1) % 8) {
		if (p == endP || bitCounter == 0) {
			if (p != endP || s.isSaving())
				s.syncAsByte(data);
			if (p == endP)
				break;

			if (s.isSaving())
				data = 0;
		}

		if (s.isLoading())
			*p = (data >> bitCounter) != 0;
		else if (*p)
			data |= 1 << bitCounter;
	}
}

Common::SeekableReadStream *SavesManager::createReadStreamForMember(const Common::String &name) const {
	CCEntry ccEntry;

	// If the given resource has already been perviously "written" to the
	// save manager, then return that new resource
	uint16 id = BaseCCArchive::convertNameToId(name);
	if (_newData.contains(id)) {
		Common::MemoryWriteStreamDynamic *stream = _newData[id];
		return new Common::MemoryReadStream(stream->getData(), stream->size());
	}

	// Retrieve the resource from the loaded savefile
	if (getHeaderEntry(name, ccEntry)) {
		// Open the correct CC entry
		return new Common::MemoryReadStream(_data + ccEntry._offset, ccEntry._size);
	}

	return nullptr;
}

void SavesManager::load(Common::SeekableReadStream *stream) {
	loadIndex(stream);

	delete[] _data;
	_data = new byte[stream->size()];
	stream->seek(0);
	stream->read(_data, stream->size());

	// Load in the character stats and active party
	Common::SeekableReadStream *chr = createReadStreamForMember("maze.chr");
	Common::Serializer sChr(chr, nullptr);
	_party._roster.synchronize(sChr);
	delete chr;

	Common::SeekableReadStream *pty = createReadStreamForMember("maze.pty");
	Common::Serializer sPty(pty, nullptr);
	_party.synchronize(sPty);
	delete pty;
}

void SavesManager::reset() {
	Common::String prefix = _vm->getGameID() != GType_DarkSide ? "xeen|" : "dark|";
	Common::MemoryWriteStreamDynamic saveFile(DisposeAfterUse::YES);
	Common::File fIn;

	const int RESOURCES[6] = { 0x2A0C, 0x2A1C, 0x2A2C, 0x2A3C, 0x284C, 0x2A5C };
	for (int i = 0; i < 6; ++i) {
		Common::String filename = prefix + Common::String::format("%.4x", RESOURCES[i]);
		if (fIn.exists(filename)) {
			// Read in the next resource
			fIn.open(filename);
			byte *data = new byte[fIn.size()];
			fIn.read(data, fIn.size());

			// Copy it to the combined savefile resource
			saveFile.write(data, fIn.size());
			delete[] data;
			fIn.close();
		}
	}

	Common::MemoryReadStream f(saveFile.getData(), saveFile.size());
	load(&f);

	// Set up the party and characters from dark.cur
	CCArchive gameCur("xeen.cur", false);
	File fParty("maze.pty", gameCur);
	Common::Serializer sParty(&fParty, nullptr);
	_party.synchronize(sParty);
	fParty.close();

	File fChar("maze.chr", gameCur);
	Common::Serializer sChar(&fChar, nullptr);
	_party._roster.synchronize(sChar);
	fChar.close();

	// Set any final initial values
	_party.resetBlacksmithWares();
	_party._year = _vm->getGameID() == GType_WorldOfXeen ? 610 : 850;
	_party._totalTime = 0;
}

void SavesManager::readCharFile() {
	warning("TODO: readCharFile");
}

void SavesManager::writeCharFile() {
	warning("TODO: writeCharFile");
}

void SavesManager::saveChars() {
	warning("TODO: saveChars");
}

} // End of namespace Xeen
