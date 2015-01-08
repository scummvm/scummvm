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

SavesManager::SavesManager(XeenEngine *vm, Party &party, Roster &roster) : 
		BaseCCArchive(), _vm(vm), _party(party), _roster(roster) {
	SearchMan.add("saves", this, 0, false);
	_data = nullptr;
}

SavesManager::~SavesManager() {
	delete[] _data;
}

/**
 * Synchronizes a boolean array as a bitfield set
 */
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
	_roster.synchronize(sChr);
	delete chr;

	Common::SeekableReadStream *pty = createReadStreamForMember("maze.pty");
	Common::Serializer sPty(pty, nullptr);
	_party.synchronize(sPty);
	delete pty;
}

/**
 * Sets up the dynamic data for the game for a new game
 */
void SavesManager::reset() {
	Common::String prefix = _vm->getGameID() == GType_Clouds ? "xeen|" : "dark|";
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
	_roster.synchronize(sChar);
	fChar.close();
}

void SavesManager::readCharFile() {
	warning("TODO: readCharFile");
}

void SavesManager::writeCharFile() {
	warning("TODO: writeCharFile");
}

} // End of namespace Xeen
