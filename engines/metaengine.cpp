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

#include "engines/metaengine.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymap.h"

#include "common/savefile.h"
#include "common/system.h"
#include "common/translation.h"

#include "graphics/thumbnail.h"

const char *MetaEngine::getSavegameFile(int saveGameIdx, const char *target) const {
	static char buffer[100];

	snprintf(buffer, 200, "%s.s%02d", target == nullptr ? getEngineId() : target, saveGameIdx);

	return buffer;
}

const char *MetaEngine::getSavegamePattern(const char *target) const {
	static char buffer[100];

	snprintf(buffer, 200, "%s.s##", target == nullptr ? getEngineId() : target);

	return buffer;
}

Common::Keymap *MetaEngine::initKeymap(const char *target) const {
	Common::Keymap *const engineKeyMap = new Common::Keymap(Common::Keymap::kKeymapTypeGame, "engine-default");

	// Since the game has multiple built-in keys for each of these anyway,
	// this just attempts to remap one of them.
	const Common::KeyActionEntry keyActionEntries[] = {
		{ "PAUS", Common::KeyState(Common::KEYCODE_SPACE, ' ', 0),                   "SPACE",  _("Pause")     },
		{ "SKCT", Common::KeyState(Common::KEYCODE_ESCAPE, Common::ASCII_ESCAPE, 0), "ESCAPE", _("Skip")      },
		{ "SKLI", Common::KeyState(Common::KEYCODE_PERIOD, '.', 0),                  "PERIOD", _("Skip line") }
	};

	for (uint i = 0; i < ARRAYSIZE(keyActionEntries); i++) {
		Common::Action *const act = new Common::Action(keyActionEntries[i].id, keyActionEntries[i].description);
		act->setKeyEvent(keyActionEntries[i].ks);
		act->addDefaultInputMapping(keyActionEntries[i].defaultHwId);
		engineKeyMap->addAction(act);
	}

	return engineKeyMap;
}

void MetaEngine::appendExtendedSave(Common::OutSaveFile *saveFile, uint32 playtime, Common::String desc) {
	ExtendedSavegameHeader header;

	uint headerPos = saveFile->pos();

	strcpy(header.id, "SVMCR");
	header.version = EXTENDED_SAVE_VERSION;

	TimeDate curTime;
	g_system->getTimeAndDate(curTime);

	header.date = ((curTime.tm_mday & 0xFF) << 24) | (((curTime.tm_mon + 1) & 0xFF) << 16) | ((curTime.tm_year + 1900) & 0xFFFF);
	header.time = ((curTime.tm_hour & 0xFF) << 8) | ((curTime.tm_min) & 0xFF);

	saveFile->write(header.id, 6);
	saveFile->writeByte(header.version);
	saveFile->writeUint32LE(header.date);
	saveFile->writeUint16LE(header.time);
	saveFile->writeUint32LE(playtime);

	saveFile->writeByte(desc.size());
	saveFile->writeString(desc);

	Graphics::saveThumbnail(*saveFile); // FIXME. Render proper screen

	saveFile->writeUint32LE(headerPos);	// Store where the header starts

	saveFile->finalize();
}

void MetaEngine::parseSavegameHeader(ExtendedSavegameHeader *header, SaveStateDescriptor *desc) {
	int day = (header->date >> 24) & 0xFF;
	int month = (header->date >> 16) & 0xFF;
	int year = header->date & 0xFFFF;
	desc->setSaveDate(year, month, day);
	int hour = (header->time >> 8) & 0xFF;
	int minutes = header->time & 0xFF;
	desc->setSaveTime(hour, minutes);
	desc->setPlayTime(header->playtime * 1000);

	desc->setDescription(header->description);
}

void MetaEngine::fillDummyHeader(ExtendedSavegameHeader *header) {
	// This is wrong header, perhaps it is original savegame. Thus fill out dummy values
	header->date = (20 << 24) | (9 << 16) | 2016;
	header->time = (9 << 8) | 56;
	header->playtime = 0;
}

WARN_UNUSED_RESULT bool MetaEngine::readSavegameHeader(Common::InSaveFile *in, ExtendedSavegameHeader *header, bool skipThumbnail) {
	uint oldPos = in->pos();

	in->seek(-4, SEEK_END);

	int headerOffset = in->readUint32LE();

	// Sanity check
	if (headerOffset >= in->pos() || headerOffset == 0) {
		in->seek(oldPos, SEEK_SET); // Rewind the file
		fillDummyHeader(header);
		return false;
	}

	in->seek(headerOffset, SEEK_SET);

	in->read(header->id, 6);

	// Validate the header Id
	if (strcmp(header->id, "SVMCR")) {
		in->seek(oldPos, SEEK_SET); // Rewind the file
		fillDummyHeader(header);
		return false;
	}

	header->version = in->readByte();
	header->date = in->readUint32LE();
	header->time = in->readUint16LE();
	header->playtime = in->readUint32LE();

	if (header->version > 1)
		header->description = in->readPascalString();

	// Generate savename
	SaveStateDescriptor desc;

	parseSavegameHeader(header, &desc);

	header->saveName = Common::String::format("%s %s", desc.getSaveDate().c_str(), desc.getSaveTime().c_str());

	if (header->description.empty())
		header->description = header->saveName;

	// Get the thumbnail
	if (!Graphics::loadThumbnail(*in, header->thumbnail, skipThumbnail)) {
		in->seek(oldPos, SEEK_SET); // Rewind the file
		return false;
	}

	in->seek(oldPos, SEEK_SET); // Rewind the file

	return true;
}


///////////////////////////////////////
// MetaEngine default implementations
///////////////////////////////////////

SaveStateList MetaEngine::listSaves(const char *target) const {
	if (!hasFeature(kSavesUseExtendedFormat))
		return SaveStateList();

	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String pattern(getSavegamePattern());

	filenames = saveFileMan->listSavefiles(pattern);

	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		// Obtain the last 2 digits of the filename, since they correspond to the save slot
		int slotNum = atoi(file->c_str() + file->size() - 2);

		if (slotNum >= 0 && slotNum <= getMaximumSaveSlot()) {
			Common::ScopedPtr<Common::InSaveFile> in(saveFileMan->openForLoading(*file));
			if (in) {
				ExtendedSavegameHeader header;
				if (!readSavegameHeader(in.get(), &header)) {
					continue;
				}

				SaveStateDescriptor desc;

				parseSavegameHeader(&header, &desc);

				desc.setSaveSlot(slotNum);

				saveList.push_back(desc);
			}
		}
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

void MetaEngine::removeSaveState(const char *target, int slot) const {
	if (!hasFeature(kSavesUseExtendedFormat))
		return;

	g_system->getSavefileManager()->removeSavefile(getSavegameFile(slot, target));
}

SaveStateDescriptor MetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	if (!hasFeature(kSavesUseExtendedFormat))
		return SaveStateDescriptor();

	Common::ScopedPtr<Common::InSaveFile> f(g_system->getSavefileManager()->openForLoading(
		getSavegameFile(slot)));

	if (f) {
		ExtendedSavegameHeader header;
		if (!readSavegameHeader(f.get(), &header, false)) {
			return SaveStateDescriptor();
		}

		// Create the return descriptor
		SaveStateDescriptor desc;

		parseSavegameHeader(&header, &desc);

		desc.setSaveSlot(slot);
		desc.setThumbnail(header.thumbnail);

		return desc;
	}

	return SaveStateDescriptor();
}
