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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "mohawk/riven.h"
#include "mohawk/riven_saveload.h"

#include "common/util.h"

namespace Mohawk {
	
RivenSaveLoad::RivenSaveLoad(MohawkEngine_Riven *vm, Common::SaveFileManager *saveFileMan) : _vm(vm), _saveFileMan(saveFileMan) {
	_loadFile = new MohawkFile();
}

RivenSaveLoad::~RivenSaveLoad() {
	delete _loadFile;
}

Common::StringList RivenSaveLoad::generateSaveGameList() {
	return _saveFileMan->listSavefiles("*.rvn");
}

// Note: The stack numbers we use do not match up to what the original executable,
// so, match them ;)
static uint16 mapOldStackIDToNew(uint16 oldID) {
	switch (oldID) {
		case 1:
			return ospit;
		case 2:
			return pspit;
		case 3:
			return rspit;
		case 4:
			return tspit;
		case 5:
			return bspit;
		case 6:
			return gspit;
		case 7:
			return jspit;
		case 8:
			return aspit;
	}
	error ("Unknown old stack ID %d", oldID);
	return 0;
}

static uint16 mapNewStackIDToOld(uint16 newID) {
	switch (newID) {
		case aspit:
			return 8;
		case bspit:
			return 5;
		case gspit:
			return 6;
		case jspit:
			return 7;
		case ospit:
			return 1;
		case pspit:
			return 2;
		case rspit:
			return 3;
		case tspit:
			return 4;
	}
	error ("Unknown new stack ID %d", newID);
	return 0;
}

bool RivenSaveLoad::loadGame(Common::String filename) {
	if (_vm->getFeatures() & GF_DEMO) // Don't load games in the demo
		return false;

	Common::InSaveFile *loadFile;
	if (!(loadFile = _saveFileMan->openForLoading(filename.c_str())))
		return false;
	debug (0, "Loading game from \'%s\'", filename.c_str());

	_loadFile->open(loadFile);

	// First, let's make sure we're using a saved game file from this version of Riven by checking the VERS resource
	Common::SeekableReadStream *vers = _loadFile->getRawData(ID_VERS, 1);
	uint32 saveGameVersion = vers->readUint32BE();
	delete vers;
	if ((saveGameVersion == kCDSaveGameVersion && (_vm->getFeatures() & GF_DVD))
		|| (saveGameVersion == kDVDSaveGameVersion && !(_vm->getFeatures() & GF_DVD))) {
		warning ("Incompatible saved game versions. No support for this yet.");
		delete _loadFile;
		return false;
	}

	// Now, we'll read in the variable values.
	Common::SeekableReadStream *vars = _loadFile->getRawData(ID_VARS, 1);
	Common::Array<uint32> rawVariables;

	while (!vars->eos()) {
		vars->readUint32BE();	// Unknown (Stack?)
		vars->readUint32BE();	// Unknown (0 or 1)
		rawVariables.push_back(vars->readUint32BE());
	}

	delete vars;

	// Next, we set the variables based on the name found by the index in the VARS resource.
	// TODO: Merge with code in mohawk.cpp for loading names?
	Common::SeekableReadStream *names = _loadFile->getRawData(ID_NAME, 1);

	uint16 namesCount = names->readUint16BE();
	uint16 *stringOffsets = new uint16[namesCount];
	for (uint16 i = 0; i < namesCount; i++)
		stringOffsets[i] = names->readUint16BE();
	for (uint16 i = 0; i < namesCount; i++)
		names->readUint16BE();	// Skip unknown values
	uint32 curNamesPos = names->pos();

	uint16 stackID = 0;
	uint16 cardID = 0;
		
	for (uint32 i = 0; i < rawVariables.size() && i < namesCount && !names->eos(); i++) {
		names->seek(curNamesPos);
		names->seek(stringOffsets[i], SEEK_CUR);
			
		Common::String name = Common::String::emptyString;
		char c = (char)names->readByte();

		while (c) {
			name += c;
			c = (char)names->readByte();
		}
			
		uint32 *var = _vm->matchVarToString(name);
			
		*var = rawVariables[i];
				
		if (!scumm_stricmp(name.c_str(), "CurrentStackID"))
			stackID = mapOldStackIDToNew(rawVariables[i]);
		else if (!scumm_stricmp(name.c_str(), "CurrentCardID"))
			cardID = rawVariables[i];
		else if (!scumm_stricmp(name.c_str(), "ReturnStackID"))
			*var = mapOldStackIDToNew(rawVariables[i]);
	}
		
	_vm->changeToStack(stackID);
	_vm->changeToCard(cardID);

	delete names;
		
	// Reset zip mode data
	_vm->_zipModeData.clear();
		
	// Finally, we load in zip mode data.
	Common::SeekableReadStream *zips = _loadFile->getRawData(ID_ZIPS, 1);
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
		
	delete _loadFile;
	_loadFile = NULL;
	return true;
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
		
	for (uint32 i = 0; i < _vm->getVarCount(); i++) {
		stream->writeUint32BE(0); // Unknown
		stream->writeUint32BE(0); // Unknown
		stream->writeUint32BE(_vm->getGlobalVar(i));
	}

	return stream;
}
	
Common::MemoryWriteStreamDynamic *RivenSaveLoad::genNAMESection() {
	Common::MemoryWriteStreamDynamic *stream = new Common::MemoryWriteStreamDynamic();
		
	stream->writeUint16BE((uint16)_vm->getVarCount());
		
	uint16 curPos = 0;
	for (uint16 i = 0; i < _vm->getVarCount(); i++) {
		stream->writeUint16BE(curPos);
		curPos += _vm->getGlobalVarName(i).size() + 1;
	}
		
	for (uint16 i = 0; i < _vm->getVarCount(); i++)
		stream->writeUint16BE(i);
			
	for (uint16 i = 0; i < _vm->getVarCount(); i++) {
		stream->write(_vm->getGlobalVarName(i).c_str(), _vm->getGlobalVarName(i).size());
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

bool RivenSaveLoad::saveGame(Common::String filename) {
	// Note, this code is still WIP. It works quite well for now.
	
	// Make sure we have the right extension
	if (!filename.hasSuffix(".rvn") && !filename.hasSuffix(".RVN"))
		filename += ".rvn";
		
	// Convert class variables to variable numbers
	*_vm->matchVarToString("currentstackid") = mapNewStackIDToOld(_vm->getCurStack());
	*_vm->matchVarToString("currentcardid") = _vm->getCurCard();
	*_vm->matchVarToString("returnstackid") = mapNewStackIDToOld(*_vm->matchVarToString("returnstackid"));
	
	Common::OutSaveFile *saveFile;
	if (!(saveFile = _saveFileMan->openForSaving(filename.c_str())))
		return false;
	debug (0, "Saving game to \'%s\'", filename.c_str());

	Common::MemoryWriteStreamDynamic *versSection = genVERSSection();
	Common::MemoryWriteStreamDynamic *nameSection = genNAMESection();
	Common::MemoryWriteStreamDynamic *varsSection = genVARSSection();
	Common::MemoryWriteStreamDynamic *zipsSection = genZIPSSection();

	// Let's calculate the file size!
	uint32 fileSize = 0;
	fileSize += versSection->size();
	fileSize += nameSection->size();
	fileSize += varsSection->size();
	fileSize += zipsSection->size();
	fileSize += 16; // RSRC Header
	fileSize += 4; // Type Table Header
	fileSize += 4 * 8; // Type Table Entries
	fileSize += 2; // Pseudo-Name entries
		
	// IFF Header
	saveFile->writeUint32BE(ID_MHWK);
	saveFile->writeUint32BE(fileSize);

	// RSRC Header
	saveFile->writeUint32BE(ID_RSRC);
	saveFile->writeUint32BE(16); // Size of RSRC
	saveFile->writeUint32BE(fileSize + 8); // Add on the 8 from the IFF header
	saveFile->writeUint32BE(28); // IFF + RSRC
	saveFile->writeUint16BE(62); // File Table Offset
	saveFile->writeUint16BE(44); // 4 + 4 * 10
	
	//Type Table
	saveFile->writeUint16BE(36); // After the Type Table Entries
	saveFile->writeUint16BE(4); // 4 Type Table Entries

	// Hardcode Entries
	saveFile->writeUint32BE(ID_VERS);
	saveFile->writeUint16BE(38);
	saveFile->writeUint16BE(36);

	saveFile->writeUint32BE(ID_NAME);
	saveFile->writeUint16BE(44);
	saveFile->writeUint16BE(36);

	saveFile->writeUint32BE(ID_VARS);
	saveFile->writeUint16BE(50);
	saveFile->writeUint16BE(36);

	saveFile->writeUint32BE(ID_ZIPS);
	saveFile->writeUint16BE(56);
	saveFile->writeUint16BE(36);

	// Pseudo-Name Table/Name List
	saveFile->writeUint16BE(0); // We don't need a name list

	// VERS Section (Resource Table)
	saveFile->writeUint16BE(1);
	saveFile->writeUint16BE(1);
	saveFile->writeUint16BE(1);

	// NAME Section (Resource Table)
	saveFile->writeUint16BE(1);
	saveFile->writeUint16BE(1);
	saveFile->writeUint16BE(2);

	// VARS Section (Resource Table)
	saveFile->writeUint16BE(1);
	saveFile->writeUint16BE(1);
	saveFile->writeUint16BE(3);

	// ZIPS Section (Resource Table)
	saveFile->writeUint16BE(1);
	saveFile->writeUint16BE(1);
	saveFile->writeUint16BE(4);

	// File Table
	saveFile->writeUint32BE(4);

	// VERS Section (File Table)
	saveFile->writeUint32BE(134);
	saveFile->writeUint16BE(versSection->size() & 0xFFFF);
	saveFile->writeByte((versSection->size() & 0xFF0000) >> 16);
	saveFile->writeByte(0);
	saveFile->writeUint16BE(0);

	// NAME Section (File Table)
	saveFile->writeUint32BE(134 + versSection->size());
	saveFile->writeUint16BE(nameSection->size() & 0xFFFF);
	saveFile->writeByte((nameSection->size() & 0xFF0000) >> 16);
	saveFile->writeByte(0);
	saveFile->writeUint16BE(0);

	// VARS Section (File Table)
	saveFile->writeUint32BE(134 + versSection->size() + nameSection->size());
	saveFile->writeUint16BE(varsSection->size() & 0xFFFF);
	saveFile->writeByte((varsSection->size() & 0xFF0000) >> 16);
	saveFile->writeByte(0);
	saveFile->writeUint16BE(0);

	// ZIPS Section (File Table)
	saveFile->writeUint32BE(134 + versSection->size() + nameSection->size() + varsSection->size());
	saveFile->writeUint16BE(zipsSection->size() & 0xFFFF);
	saveFile->writeByte((zipsSection->size() & 0xFF0000) >> 16);
	saveFile->writeByte(0);
	saveFile->writeUint16BE(0);

	saveFile->write(versSection->getData(), versSection->size());
	saveFile->write(nameSection->getData(), nameSection->size());
	saveFile->write(varsSection->getData(), varsSection->size());
	saveFile->write(zipsSection->getData(), zipsSection->size());

	saveFile->finalize();

	delete saveFile;
	delete versSection;
	delete nameSection;
	delete varsSection;
	delete zipsSection;

	return true;
}

void RivenSaveLoad::deleteSave(Common::String saveName) {
	debug (0, "Deleting save file \'%s\'", saveName.c_str());
	_saveFileMan->removeSavefile(saveName.c_str());
}

} // End of namespace Mohawk
