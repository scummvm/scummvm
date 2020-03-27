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
#include "mohawk/riven_card.h"
#include "mohawk/riven_saveload.h"
#include "mohawk/riven_stack.h"

#include "common/system.h"
#include "graphics/thumbnail.h"

namespace Mohawk {

RivenSaveMetadata::RivenSaveMetadata() {
	saveDay = 0;
	saveMonth = 0;
	saveYear = 0;
	saveHour = 0;
	saveMinute = 0;
	totalPlayTime = 0;
	autoSave = false;
}

bool RivenSaveMetadata::sync(Common::Serializer &s) {
	static const Common::Serializer::Version kCurrentVersion = 2;

	if (!s.syncVersion(kCurrentVersion)) {
		return false;
	}

	s.syncAsByte(saveDay);
	s.syncAsByte(saveMonth);
	s.syncAsUint16BE(saveYear);
	s.syncAsByte(saveHour);
	s.syncAsByte(saveMinute);
	s.syncString(saveDescription);
	s.syncAsUint32BE(totalPlayTime);
	s.syncAsByte(autoSave, 2);

	return true;
}

RivenSaveLoad::RivenSaveLoad(MohawkEngine_Riven *vm, Common::SaveFileManager *saveFileMan) : _vm(vm), _saveFileMan(saveFileMan) {
}

RivenSaveLoad::~RivenSaveLoad() {
}

Common::String RivenSaveLoad::buildSaveFilename(const int slot) {
	return Common::String::format("riven-%03d.rvn", slot);
}

Common::String RivenSaveLoad::querySaveDescription(const int slot) {
	Common::String filename = buildSaveFilename(slot);
	Common::InSaveFile *loadFile = g_system->getSavefileManager()->openForLoading(filename);
	if (!loadFile) {
		return "";
	}

	MohawkArchive mhk;
	if (!mhk.openStream(loadFile)) {
		return "";
	}

	if (!mhk.hasResource(ID_META, 1)) {
		return "";
	}

	Common::SeekableReadStream *metaStream = mhk.getResource(ID_META, 1);
	if (!metaStream) {
		return "";
	}

	Common::Serializer serializer = Common::Serializer(metaStream, nullptr);

	RivenSaveMetadata metadata;
	if (!metadata.sync(serializer)) {
		delete metaStream;
		return "";
	}

	delete metaStream;

	return metadata.saveDescription;
}

SaveStateDescriptor RivenSaveLoad::querySaveMetaInfos(const int slot) {
	Common::String filename = buildSaveFilename(slot);
	Common::InSaveFile *loadFile = g_system->getSavefileManager()->openForLoading(filename);
	SaveStateDescriptor descriptor;

	if (!loadFile) {
		return descriptor;
	}

	MohawkArchive mhk;
	if (!mhk.openStream(loadFile)) {
		return descriptor;
	}

	if (!mhk.hasResource(ID_META, 1)) {
		return descriptor;
	}

	Common::SeekableReadStream *metaStream = mhk.getResource(ID_META, 1);
	if (!metaStream) {
		return descriptor;
	}

	Common::Serializer serializer = Common::Serializer(metaStream, nullptr);

	RivenSaveMetadata metadata;
	if (!metadata.sync(serializer)) {
		delete metaStream;
		return descriptor;
	}

	descriptor.setSaveSlot(slot);
	descriptor.setDescription(metadata.saveDescription);
	descriptor.setPlayTime(metadata.totalPlayTime);
	descriptor.setSaveDate(metadata.saveYear, metadata.saveMonth, metadata.saveDay);
	descriptor.setSaveTime(metadata.saveHour, metadata.saveMinute);
	descriptor.setAutosave(metadata.autoSave);

	delete metaStream;

	if (!mhk.hasResource(ID_THMB, 1)) {
		return descriptor;
	}

	Common::SeekableReadStream *thmbStream = mhk.getResource(ID_THMB, 1);
	if (!thmbStream) {
		return descriptor;
	}

	Graphics::Surface *thumbnail;
	if (!Graphics::loadThumbnail(*thmbStream, thumbnail)) {
		return descriptor;
	}
	descriptor.setThumbnail(thumbnail);

	delete thmbStream;

	return descriptor;
}

Common::Error RivenSaveLoad::loadGame(const int slot) {
	if (_vm->isGameVariant(GF_DEMO)) // Don't load games in the demo
		return Common::kNoError;

	Common::String filename = buildSaveFilename(slot);
	Common::InSaveFile *loadFile = _saveFileMan->openForLoading(filename);
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
	if ((saveGameVersion == kCDSaveGameVersion && _vm->isGameVariant(GF_DVD))
		|| (saveGameVersion == kDVDSaveGameVersion && !_vm->isGameVariant(GF_DVD))) {
		warning("Unable to load: Saved game created using an incompatible game version - CD vs DVD");
		delete mhk;
		return Common::Error(Common::kUnknownError, "Saved game created using an incompatible game version - CD vs DVD");
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

	_vm->applyGameSettings();

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

	// Load the ScummVM specific save metadata
	if (mhk->hasResource(ID_META, 1)) {
		Common::SeekableReadStream *metadataStream = mhk->getResource(ID_META, 1);
		Common::Serializer serializer = Common::Serializer(metadataStream, nullptr);

		RivenSaveMetadata metadata;
		metadata.sync(serializer);

		// Set the saved total play time
		_vm->setTotalPlayTime(metadata.totalPlayTime);

		delete metadataStream;
	}
	delete mhk;

	return Common::kNoError;
}

Common::MemoryWriteStreamDynamic *RivenSaveLoad::genVERSSection() {
	Common::MemoryWriteStreamDynamic *stream = new Common::MemoryWriteStreamDynamic(DisposeAfterUse::YES);
	if (_vm->isGameVariant(GF_DVD))
		stream->writeUint32BE(kDVDSaveGameVersion);
	else
		stream->writeUint32BE(kCDSaveGameVersion);
	return stream;
}

Common::MemoryWriteStreamDynamic *RivenSaveLoad::genVARSSection() {
	Common::MemoryWriteStreamDynamic *stream = new Common::MemoryWriteStreamDynamic(DisposeAfterUse::YES);

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
	Common::MemoryWriteStreamDynamic *stream = new Common::MemoryWriteStreamDynamic(DisposeAfterUse::YES);

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
	Common::MemoryWriteStreamDynamic *stream = new Common::MemoryWriteStreamDynamic(DisposeAfterUse::YES);

	stream->writeUint16BE(_vm->_zipModeData.size());

	for (uint16 i = 0; i < _vm->_zipModeData.size(); i++) {
		stream->writeUint16BE(_vm->_zipModeData[i].name.size());
		stream->write(_vm->_zipModeData[i].name.c_str(), _vm->_zipModeData[i].name.size());
		stream->writeUint16BE(_vm->_zipModeData[i].id);
	}

	return stream;
}

Common::MemoryWriteStreamDynamic *RivenSaveLoad::genTHMBSection(const Graphics::Surface *thumbnail) const {
	Common::MemoryWriteStreamDynamic *stream = new Common::MemoryWriteStreamDynamic(DisposeAfterUse::YES);

	if (thumbnail) {
		Graphics::saveThumbnail(*stream, *thumbnail);
	} else {
		Graphics::saveThumbnail(*stream);
	}

	return stream;
}

Common::MemoryWriteStreamDynamic *RivenSaveLoad::genMETASection(const Common::String &desc, bool autoSave) const {
	Common::MemoryWriteStreamDynamic *stream = new Common::MemoryWriteStreamDynamic(DisposeAfterUse::YES);
	Common::Serializer serializer = Common::Serializer(nullptr, stream);

	TimeDate t;
	_vm->_system->getTimeAndDate(t);

	RivenSaveMetadata metadata;
	metadata.saveDay = t.tm_mday;
	metadata.saveMonth = t.tm_mon + 1;
	metadata.saveYear = t.tm_year + 1900;
	metadata.saveHour = t.tm_hour;
	metadata.saveMinute = t.tm_min;
	metadata.saveDescription = desc;
	metadata.totalPlayTime = _vm->getTotalPlayTime();
	metadata.autoSave = autoSave;
	metadata.sync(serializer);

	return stream;
}

Common::Error RivenSaveLoad::saveGame(const int slot, const Common::String &description,
                                      const Graphics::Surface *thumbnail, bool autoSave) {
	// NOTE: This code is designed to only output a Mohawk archive
	// for a Riven saved game. It's hardcoded to do this because
	// (as of right now) this is the only place in the engine
	// that requires this feature. If the time comes when other
	// games need this, we should think about coming up with some
	// more common way of outputting resources to an archive.

	Common::String filename = buildSaveFilename(slot);

	Common::OutSaveFile *saveFile = _saveFileMan->openForSaving(filename);
	if (!saveFile)
		return Common::kWritingFailed;

	debug (0, "Saving game to \'%s\'", filename.c_str());

	Common::MemoryWriteStreamDynamic *metaSection = genMETASection(description, autoSave);
	Common::MemoryWriteStreamDynamic *nameSection = genNAMESection();
	Common::MemoryWriteStreamDynamic *thmbSection = genTHMBSection(thumbnail);
	Common::MemoryWriteStreamDynamic *varsSection = genVARSSection();
	Common::MemoryWriteStreamDynamic *versSection = genVERSSection();
	Common::MemoryWriteStreamDynamic *zipsSection = genZIPSSection();

	// Let's calculate the file size!
	uint32 fileSize = 194;
	fileSize += metaSection->size();
	fileSize += nameSection->size();
	fileSize += thmbSection->size();
	fileSize += varsSection->size();
	fileSize += versSection->size();
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
	saveFile->writeUint16BE(102); // File Table Offset
	saveFile->writeUint16BE(64); // File Table Size (4 bytes count + 6 entries * 10 bytes per entry)

	// Type Table (4 bytes - total: 32)
	saveFile->writeUint16BE(52); // String table offset After the Type Table Entries
	saveFile->writeUint16BE(6); // 6 Type Table Entries

	// Hardcode Entries (48 bytes - total: 80)
	// The original engine relies on the entries being sorted by tag alphabetical order
	// to optimize its lookup algorithm.
	saveFile->writeUint32BE(ID_META);
	saveFile->writeUint16BE(66); // Resource table offset
	saveFile->writeUint16BE(54); // String table offset

	saveFile->writeUint32BE(ID_NAME);
	saveFile->writeUint16BE(72);
	saveFile->writeUint16BE(56);

	saveFile->writeUint32BE(ID_THMB);
	saveFile->writeUint16BE(78);
	saveFile->writeUint16BE(58);

	saveFile->writeUint32BE(ID_VARS);
	saveFile->writeUint16BE(84);
	saveFile->writeUint16BE(60);

	saveFile->writeUint32BE(ID_VERS);
	saveFile->writeUint16BE(90);
	saveFile->writeUint16BE(62);

	saveFile->writeUint32BE(ID_ZIPS);
	saveFile->writeUint16BE(96);
	saveFile->writeUint16BE(64);

	// Pseudo-String Table (2 bytes - total: 82)
	saveFile->writeUint16BE(0); // We don't need a name list

	// Pseudo-Name Tables (12 bytes - total: 94)
	saveFile->writeUint16BE(0);
	saveFile->writeUint16BE(0);
	saveFile->writeUint16BE(0);
	saveFile->writeUint16BE(0);
	saveFile->writeUint16BE(0);
	saveFile->writeUint16BE(0);

	// META Section (Resource Table) (6 bytes - total: 100)
	saveFile->writeUint16BE(1);
	saveFile->writeUint16BE(1);
	saveFile->writeUint16BE(1);

	// NAME Section (Resource Table) (6 bytes - total: 106)
	saveFile->writeUint16BE(1);
	saveFile->writeUint16BE(1);
	saveFile->writeUint16BE(2);

	// THMB Section (Resource Table) (6 bytes - total: 112)
	saveFile->writeUint16BE(1);
	saveFile->writeUint16BE(1);
	saveFile->writeUint16BE(3);

	// VARS Section (Resource Table) (6 bytes - total: 118)
	saveFile->writeUint16BE(1);
	saveFile->writeUint16BE(1);
	saveFile->writeUint16BE(4);

	// VERS Section (Resource Table) (6 bytes - total: 124)
	saveFile->writeUint16BE(1);
	saveFile->writeUint16BE(1);
	saveFile->writeUint16BE(5);

	// ZIPS Section (Resource Table) (6 bytes - total: 130)
	saveFile->writeUint16BE(1);
	saveFile->writeUint16BE(1);
	saveFile->writeUint16BE(6);

	// File Table (4 bytes - total: 134)
	saveFile->writeUint32BE(6);

	// META Section (File Table) (10 bytes - total: 144)
	saveFile->writeUint32BE(194);
	saveFile->writeUint16BE(metaSection->size() & 0xFFFF);
	saveFile->writeByte((metaSection->size() & 0xFF0000) >> 16);
	saveFile->writeByte(0);
	saveFile->writeUint16BE(0);

	// NAME Section (File Table) (10 bytes - total: 154)
	saveFile->writeUint32BE(194 + metaSection->size());
	saveFile->writeUint16BE(nameSection->size() & 0xFFFF);
	saveFile->writeByte((nameSection->size() & 0xFF0000) >> 16);
	saveFile->writeByte(0);
	saveFile->writeUint16BE(0);

	// THMB Section (File Table) (10 bytes - total: 164)
	saveFile->writeUint32BE(194 + metaSection->size() + nameSection->size());
	saveFile->writeUint16BE(thmbSection->size() & 0xFFFF);
	saveFile->writeByte((thmbSection->size() & 0xFF0000) >> 16);
	saveFile->writeByte(0);
	saveFile->writeUint16BE(0);

	// VARS Section (File Table) (10 bytes - total: 174)
	saveFile->writeUint32BE(194 + metaSection->size() + nameSection->size() + thmbSection->size());
	saveFile->writeUint16BE(varsSection->size() & 0xFFFF);
	saveFile->writeByte((varsSection->size() & 0xFF0000) >> 16);
	saveFile->writeByte(0);
	saveFile->writeUint16BE(0);

	// VERS Section (File Table) (10 bytes - total: 184)
	saveFile->writeUint32BE(194 + metaSection->size() + nameSection->size() + thmbSection->size() + varsSection->size());
	saveFile->writeUint16BE(versSection->size() & 0xFFFF);
	saveFile->writeByte((versSection->size() & 0xFF0000) >> 16);
	saveFile->writeByte(0);
	saveFile->writeUint16BE(0);

	// ZIPS Section (File Table) (10 bytes - total: 194)
	saveFile->writeUint32BE(194 + metaSection->size() + nameSection->size() + thmbSection->size() + varsSection->size() + versSection->size());
	saveFile->writeUint16BE(zipsSection->size() & 0xFFFF);
	saveFile->writeByte((zipsSection->size() & 0xFF0000) >> 16);
	saveFile->writeByte(0);
	saveFile->writeUint16BE(0);

	saveFile->write(metaSection->getData(), metaSection->size());
	saveFile->write(nameSection->getData(), nameSection->size());
	saveFile->write(thmbSection->getData(), thmbSection->size());
	saveFile->write(varsSection->getData(), varsSection->size());
	saveFile->write(versSection->getData(), versSection->size());
	saveFile->write(zipsSection->getData(), zipsSection->size());

	saveFile->finalize();

	delete saveFile;
	delete metaSection;
	delete nameSection;
	delete thmbSection;
	delete varsSection;
	delete versSection;
	delete zipsSection;

	return Common::kNoError;
}

void RivenSaveLoad::deleteSave(const int slot) {
	Common::String filename = buildSaveFilename(slot);

	debug (0, "Deleting save file \'%s\'", filename.c_str());
	g_system->getSavefileManager()->removeSavefile(filename);
}

} // End of namespace Mohawk
