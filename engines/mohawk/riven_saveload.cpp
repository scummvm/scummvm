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

#include "mohawk/resource.h"
#include "mohawk/riven.h"
#include "mohawk/riven_saveload.h"

#include "common/util.h"

namespace Mohawk {

RivenSaveLoad::RivenSaveLoad(MohawkEngine_Riven *vm, Common::SaveFileManager *saveFileMan) : _vm(vm), _saveFileMan(saveFileMan) {
}

RivenSaveLoad::~RivenSaveLoad() {
}

Common::StringArray RivenSaveLoad::generateSaveGameList() {
	return _saveFileMan->listSavefiles("*.rvn");
}

Common::Error RivenSaveLoad::loadGame(Common::String filename) {
	if (_vm->getFeatures() & GF_DEMO) // Don't load games in the demo
		return Common::kNoError;

	Common::InSaveFile *loadFile =  _saveFileMan->openForLoading(filename);
	if (!loadFile)
		return Common::kReadingFailed;

	debug(0, "Loading game from \'%s\'", filename.c_str());

	MohawkArchive *mhk = new MohawkArchive();

	if (!mhk->openStream(loadFile)) {
		warning("Save file is not a Mohawk archive");
		delete mhk;
		return Common::Error(Common::kUnknownError, "Invalid save file");
	}

	// First, let's make sure we're using a saved game file from this version of Riven by checking the VERS resource
	Common::SeekableReadStream *vers = mhk->getResource(ID_VERS, 1);
	uint32 saveGameVersion = vers->readUint32BE();
	delete vers;
	if ((saveGameVersion == kCDSaveGameVersion && (_vm->getFeatures() & GF_DVD))
		|| (saveGameVersion == kDVDSaveGameVersion && !(_vm->getFeatures() & GF_DVD))) {
		warning("Incompatible saved game versions. No support for this yet");
		delete mhk;
		return Common::Error(Common::kUnknownError, "Incompatible save version");
	}

	// Now, we'll read in the variable values.
	Common::SeekableReadStream *vars = mhk->getResource(ID_VARS, 1);
	Common::Array<uint32> rawVariables;

	while (!vars->eos()) {
		// The original engine stores the variables values in an array. All the slots in
		// the array may not be in use, which is why it needs a reference counter and
		// a flag to tell if the value has been set.
		vars->readUint32BE();	// Reference counter
		vars->readUint32BE();	// Variable initialized flag
		rawVariables.push_back(vars->readUint32BE());
	}

	delete vars;

	// Next, we set the variables based on the name found by the index in the VARS resource.
	// TODO: Merge with code in mohawk.cpp for loading names?
	Common::SeekableReadStream *names = mhk->getResource(ID_NAME, 1);

	uint16 namesCount = names->readUint16BE();
	uint16 *stringOffsets = new uint16[namesCount];
	for (uint16 i = 0; i < namesCount; i++)
		stringOffsets[i] = names->readUint16BE();
	for (uint16 i = 0; i < namesCount; i++)
		names->readUint16BE();	// Skip unknown values
	uint32 curNamesPos = names->pos();

	for (uint32 i = 0; i < namesCount && !names->eos(); i++) {
		names->seek(curNamesPos);
		names->seek(stringOffsets[i], SEEK_CUR);

		Common::String name;
		char c = (char)names->readByte();

		while (c) {
			name += c;
			c = (char)names->readByte();
		}

		// These are timing variables used with the DVD version of Riven for the whark
		// puzzle and are not needed at all. See xjschool280_resetleft() and
		// xjschool280_resetright.
		if (name == "dropLeftStart" || name == "dropRightStart")
			continue;

		uint32 &var = _vm->_vars[name];
		name.toLowercase();

		// WORKAROUND: time variables are reset here for one main reason:
		// The save does not store any start point for the time, so we don't know the real time.
		// Because of this, in many cases, the original would just give a 'free' Ytram upon saving
		// since the time would be used in a new (improper) time frame.
		if (name.contains("time"))
			var = 0;
		else
			var = rawVariables[i];
	}

	_vm->changeToStack(_vm->_vars["CurrentStackID"]);
	_vm->changeToCard(_vm->_vars["CurrentCardID"]);

	delete names;
	delete[] stringOffsets;

	// Reset zip mode data
	_vm->_zipModeData.clear();

	// Finally, we load in zip mode data.
	Common::SeekableReadStream *zips = mhk->getResource(ID_ZIPS, 1);
	uint16 zipsRecordCount = zips->readUint16BE();
	for (uint16 i = 0; i < zipsRecordCount; i++) {
		ZipMode zip;
		uint16 zipsNameLength = zips->readUint16BE();
		for (uint16 j = 0; j < zipsNameLength; j++)
			zip.name += zips->readByte();
		zip.id = zips->readUint16BE();
		_vm->_zipModeData.push_back(zip);
	}

	delete zips;
	delete mhk;

	return Common::kNoError;
}

Common::MemoryWriteStreamDynamic *RivenSaveLoad::genVERSSection() {
	Common::MemoryWriteStreamDynamic *stream = new Common::MemoryWriteStreamDynamic();
	if (_vm->getFeatures() & GF_DVD)
		stream->writeUint32BE(kDVDSaveGameVersion);
	else
		stream->writeUint32BE(kCDSaveGameVersion);
	return stream;
}

Common::MemoryWriteStreamDynamic *RivenSaveLoad::genVARSSection() {
	Common::MemoryWriteStreamDynamic *stream = new Common::MemoryWriteStreamDynamic();

	for (RivenVariableMap::const_iterator it = _vm->_vars.begin(); it != _vm->_vars.end(); it++) {
		stream->writeUint32BE(1); // Reference counter
		stream->writeUint32BE(1); // Variable initialized flag
		stream->writeUint32BE(it->_value);
	}

	return stream;
}

static int stringCompareToIgnoreCase(const Common::String &s1, const Common::String &s2) {
	return s1.compareToIgnoreCase(s2) < 0;
}

Common::MemoryWriteStreamDynamic *RivenSaveLoad::genNAMESection() {
	Common::MemoryWriteStreamDynamic *stream = new Common::MemoryWriteStreamDynamic();

	stream->writeUint16BE(_vm->_vars.size());

	uint16 curPos = 0;
	for (RivenVariableMap::const_iterator it = _vm->_vars.begin(); it != _vm->_vars.end(); it++) {
		stream->writeUint16BE(curPos);
		curPos += it->_key.size() + 1;
	}

	// The original engine does not store the variables in a HashMap, but in a "NameList"
	// for the keys and an array for the values. The NameList data structure maintains an array
	// of indices in the string table sorted by case insensitive key alphabetical order.
	// It is used to perform fast key -> index lookups.
	// ScummVM does not need the sorted array, but has to write it anyway for the saved games
	// to be compatible with original engine.
	Common::Array<Common::String> sortedKeys;
	for (RivenVariableMap::const_iterator it = _vm->_vars.begin(); it != _vm->_vars.end(); it++) {
		sortedKeys.push_back(it->_key);
	}
	Common::sort(sortedKeys.begin(), sortedKeys.end(), stringCompareToIgnoreCase);

	for (uint i = 0; i < sortedKeys.size(); i++) {
		uint16 varIndex = 0;
		for (RivenVariableMap::const_iterator it = _vm->_vars.begin(); it != _vm->_vars.end(); it++) {
			if (it->_key == sortedKeys[i]) {
				stream->writeUint16BE(varIndex);
				break;
			}
			varIndex++;
		}
	}

	for (RivenVariableMap::const_iterator it = _vm->_vars.begin(); it != _vm->_vars.end(); it++) {
		stream->write(it->_key.c_str(), it->_key.size());
		stream->writeByte(0);
	}

	return stream;
}

Common::MemoryWriteStreamDynamic *RivenSaveLoad::genZIPSSection() {
	Common::MemoryWriteStreamDynamic *stream = new Common::MemoryWriteStreamDynamic();

	stream->writeUint16BE(_vm->_zipModeData.size());

	for (uint16 i = 0; i < _vm->_zipModeData.size(); i++) {
		stream->writeUint16BE(_vm->_zipModeData[i].name.size());
		stream->write(_vm->_zipModeData[i].name.c_str(), _vm->_zipModeData[i].name.size());
		stream->writeUint16BE(_vm->_zipModeData[i].id);
	}

	return stream;
}

Common::Error RivenSaveLoad::saveGame(Common::String filename) {
	// NOTE: This code is designed to only output a Mohawk archive
	// for a Riven saved game. It's hardcoded to do this because
	// (as of right now) this is the only place in the engine
	// that requires this feature. If the time comes when other
	// games need this, we should think about coming up with some
	// more common way of outputting resources to an archive.

	// Make sure we have the right extension
	if (!filename.matchString("*.rvn", true))
		filename += ".rvn";

	// Convert class variables to variable numbers
	_vm->_vars["currentstackid"] = _vm->getCurStack();
	_vm->_vars["currentcardid"] = _vm->getCurCard();

	Common::OutSaveFile *saveFile = _saveFileMan->openForSaving(filename);
	if (!saveFile)
		return Common::kWritingFailed;

	debug (0, "Saving game to \'%s\'", filename.c_str());

	Common::MemoryWriteStreamDynamic *versSection = genVERSSection();
	Common::MemoryWriteStreamDynamic *nameSection = genNAMESection();
	Common::MemoryWriteStreamDynamic *varsSection = genVARSSection();
	Common::MemoryWriteStreamDynamic *zipsSection = genZIPSSection();

	// Let's calculate the file size!
	uint32 fileSize = 142;
	fileSize += versSection->size();
	fileSize += nameSection->size();
	fileSize += varsSection->size();
	fileSize += zipsSection->size();

	// MHWK Header (8 bytes - total: 8)
	saveFile->writeUint32BE(ID_MHWK);
	saveFile->writeUint32BE(fileSize - 8);

	// RSRC Header (20 bytes - total: 28)
	saveFile->writeUint32BE(ID_RSRC);
	saveFile->writeUint16BE(0x100); // Resource Version (1.0)
	saveFile->writeUint16BE(1); // Compaction -- original saves have this too
	saveFile->writeUint32BE(fileSize); // Subtract off the MHWK header size
	saveFile->writeUint32BE(28); // Absolute offset: right after both headers
	saveFile->writeUint16BE(70); // File Table Offset
	saveFile->writeUint16BE(44); // File Table Size (4 bytes count + 4 entries * 10 bytes per entry)

	// Type Table (4 bytes - total: 32)
	saveFile->writeUint16BE(36); // String table offset After the Type Table Entries
	saveFile->writeUint16BE(4); // 4 Type Table Entries

	// Hardcode Entries (32 bytes - total: 64)
	// The original engine relies on the entries being sorted by tag alphabetical order
	// to optimize its lookup algorithm.
	saveFile->writeUint32BE(ID_NAME);
	saveFile->writeUint16BE(46); // Resource table offset
	saveFile->writeUint16BE(38); // String table offset

	saveFile->writeUint32BE(ID_VARS);
	saveFile->writeUint16BE(52);
	saveFile->writeUint16BE(40);

	saveFile->writeUint32BE(ID_VERS);
	saveFile->writeUint16BE(58);
	saveFile->writeUint16BE(42);

	saveFile->writeUint32BE(ID_ZIPS);
	saveFile->writeUint16BE(64);
	saveFile->writeUint16BE(44);

	// Pseudo-String Table (2 bytes - total: 66)
	saveFile->writeUint16BE(0); // We don't need a name list

	// Pseudo-Name Tables (8 bytes - total: 74)
	saveFile->writeUint16BE(0);
	saveFile->writeUint16BE(0);
	saveFile->writeUint16BE(0);
	saveFile->writeUint16BE(0);

	// NAME Section (Resource Table) (6 bytes - total: 80)
	saveFile->writeUint16BE(1);
	saveFile->writeUint16BE(1);
	saveFile->writeUint16BE(1);

	// VARS Section (Resource Table) (6 bytes - total: 86)
	saveFile->writeUint16BE(1);
	saveFile->writeUint16BE(1);
	saveFile->writeUint16BE(2);

	// VERS Section (Resource Table) (6 bytes - total: 92)
	saveFile->writeUint16BE(1);
	saveFile->writeUint16BE(1);
	saveFile->writeUint16BE(3);

	// ZIPS Section (Resource Table) (6 bytes - total: 98)
	saveFile->writeUint16BE(1);
	saveFile->writeUint16BE(1);
	saveFile->writeUint16BE(4);

	// File Table (4 bytes - total: 102)
	saveFile->writeUint32BE(4);

	// NAME Section (File Table) (10 bytes - total: 112)
	saveFile->writeUint32BE(142);
	saveFile->writeUint16BE(nameSection->size() & 0xFFFF);
	saveFile->writeByte((nameSection->size() & 0xFF0000) >> 16);
	saveFile->writeByte(0);
	saveFile->writeUint16BE(0);

	// VARS Section (File Table) (10 bytes - total: 122)
	saveFile->writeUint32BE(142 + nameSection->size());
	saveFile->writeUint16BE(varsSection->size() & 0xFFFF);
	saveFile->writeByte((varsSection->size() & 0xFF0000) >> 16);
	saveFile->writeByte(0);
	saveFile->writeUint16BE(0);

	// VERS Section (File Table) (10 bytes - total: 132)
	saveFile->writeUint32BE(142 + nameSection->size() + varsSection->size());
	saveFile->writeUint16BE(versSection->size() & 0xFFFF);
	saveFile->writeByte((versSection->size() & 0xFF0000) >> 16);
	saveFile->writeByte(0);
	saveFile->writeUint16BE(0);

	// ZIPS Section (File Table) (10 bytes - total: 142)
	saveFile->writeUint32BE(142 + nameSection->size() + varsSection->size() + versSection->size());
	saveFile->writeUint16BE(zipsSection->size() & 0xFFFF);
	saveFile->writeByte((zipsSection->size() & 0xFF0000) >> 16);
	saveFile->writeByte(0);
	saveFile->writeUint16BE(0);

	saveFile->write(nameSection->getData(), nameSection->size());
	saveFile->write(varsSection->getData(), varsSection->size());
	saveFile->write(versSection->getData(), versSection->size());
	saveFile->write(zipsSection->getData(), zipsSection->size());

	saveFile->finalize();

	delete saveFile;
	delete versSection;
	delete nameSection;
	delete varsSection;
	delete zipsSection;

	return Common::kNoError;
}

void RivenSaveLoad::deleteSave(Common::String saveName) {
	debug (0, "Deleting save file \'%s\'", saveName.c_str());
	_saveFileMan->removeSavefile(saveName);
}

} // End of namespace Mohawk
