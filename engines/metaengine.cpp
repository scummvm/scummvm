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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "engines/metaengine.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymap.h"
#include "backends/keymapper/standard-actions.h"

#include "common/savefile.h"
#include "common/system.h"
#include "common/translation.h"

#include "engines/dialogs.h"

#include "graphics/scaler.h"
#include "graphics/managed_surface.h"
#include "graphics/thumbnail.h"

Common::String MetaEngine::getSavegameFile(int saveGameIdx, const char *target) const {
	if (!target)
		target = getName();
	if (saveGameIdx == kSavegameFilePattern) {
		// Pattern requested
		const char *pattern = hasFeature(kSimpleSavesNames) ? "%s.###" : "%s.s##";
		return Common::String::format(pattern, target);
	} else {
		// Specific filename requested
		const char *pattern = hasFeature(kSimpleSavesNames) ? "%s.%03d" : "%s.s%02d";
		return Common::String::format(pattern, target, saveGameIdx);
	}
}

Common::KeymapArray MetaEngine::initKeymaps(const char *target) const {
	using namespace Common;

	Keymap *engineKeyMap = new Keymap(Keymap::kKeymapTypeGame, "engine-default", _("Default game keymap"));

	Action *act;

	act = new Action(kStandardActionLeftClick, _("Left Click"));
	act->setLeftClickEvent();
	act->addDefaultInputMapping("MOUSE_LEFT");
	act->addDefaultInputMapping("JOY_A");
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionMiddleClick, _("Middle Click"));
	act->addDefaultInputMapping("MOUSE_MIDDLE");
	act->setMiddleClickEvent();
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionRightClick, _("Right Click"));
	act->setRightClickEvent();
	act->addDefaultInputMapping("MOUSE_RIGHT");
	act->addDefaultInputMapping("JOY_B");
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionPause, _("Pause"));
	act->setKeyEvent(KeyState(KEYCODE_SPACE, ' '));
	act->addDefaultInputMapping("SPACE");
	act->allowKbdRepeats();
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionOpenMainMenu, _("Game menu"));
	act->setKeyEvent(KeyState(KEYCODE_F5, ASCII_F5));
	act->addDefaultInputMapping("F5");
	act->addDefaultInputMapping("JOY_LEFT_SHOULDER");
	act->allowKbdRepeats();
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionSkip, _("Skip"));
	act->setKeyEvent(KeyState(KEYCODE_ESCAPE, ASCII_ESCAPE));
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("JOY_Y");
	act->allowKbdRepeats();
	engineKeyMap->addAction(act);

	act = new Action("SKLI", _("Skip line"));
	act->setKeyEvent(KeyState(KEYCODE_PERIOD, '.'));
	act->addDefaultInputMapping("PERIOD");
	act->addDefaultInputMapping("JOY_X");
	act->allowKbdRepeats();
	engineKeyMap->addAction(act);

	act = new Action("PIND", _("Predictive input dialog"));
	act->setEvent(EVENT_PREDICTIVE_DIALOG);
	act->allowKbdRepeats();
	engineKeyMap->addAction(act);

	act = new Action("RETURN", _("Confirm"));
	act->setKeyEvent(KeyState(KEYCODE_RETURN, ASCII_RETURN));
	act->addDefaultInputMapping("RETURN");
	act->allowKbdRepeats();
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionMoveUp, _("Up"));
	act->setKeyEvent(KEYCODE_KP8);
	act->addDefaultInputMapping("JOY_UP");
	act->allowKbdRepeats();
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionMoveDown, _("Down"));
	act->setKeyEvent(KEYCODE_KP2);
	act->addDefaultInputMapping("JOY_DOWN");
	act->allowKbdRepeats();
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionMoveLeft, _("Left"));
	act->setKeyEvent(KEYCODE_KP4);
	act->addDefaultInputMapping("JOY_LEFT");
	act->allowKbdRepeats();
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionMoveRight, _("Right"));
	act->setKeyEvent(KEYCODE_KP6);
	act->addDefaultInputMapping("JOY_RIGHT");
	act->allowKbdRepeats();
	engineKeyMap->addAction(act);

	return Keymap::arrayOf(engineKeyMap);
}

const Common::AchievementsInfo MetaEngine::getAchievementsInfo(const Common::String &target) const {
	const Common::AchievementDescriptionList* achievementDescriptionList = getAchievementDescriptionList();
	if (achievementDescriptionList == nullptr) {
		return Common::AchievementsInfo();
	}

	Common::String gameId = ConfMan.get("gameid", target);

	Common::AchievementsPlatform platform = Common::UNK_ACHIEVEMENTS;
	Common::String extra = ConfMan.get("extra", target);
	if (extra.contains("GOG")) {
		platform = Common::GALAXY_ACHIEVEMENTS;
	} else if (extra.contains("Steam")) {
		platform = Common::STEAM_ACHIEVEMENTS;
	}

	// "(gameId, platform) -> result" search
	Common::AchievementsInfo result;
	for (const Common::AchievementDescriptionList *i = achievementDescriptionList; i->gameId; i++) {
		if (i->gameId == gameId && i->platform == platform) {
			result.platform = i->platform;
			result.appId = i->appId;
			break;
		}
	}
	return result;
}

bool MetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSavesSupportCreationDate) ||
		(f == kSavesSupportPlayTime) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSimpleSavesNames) ||
		(f == kSavesUseExtendedFormat);
}

/////////////////////////////////////////
//// Extended Saves
/////////////////////////////////////////

void MetaEngine::appendExtendedSave(Common::OutSaveFile *saveFile, uint32 playtime,
		Common::String desc, bool isAutosave) {
	appendExtendedSaveToStream(saveFile, playtime, desc, isAutosave);

	saveFile->finalize();
}

void MetaEngine::appendExtendedSaveToStream(Common::WriteStream *saveFile, uint32 playtime,
		Common::String desc, bool isAutosave, uint32 posoffset) {
	ExtendedSavegameHeader header;

	uint headerPos = saveFile->pos() + posoffset;

	Common::strcpy_s(header.id, "SVMCR");
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

	if (desc.size() > 0xFF)
		desc = desc.substr(0, 0xFF);

	saveFile->writeByte(desc.size());
	saveFile->writeString(desc);
	saveFile->writeByte(isAutosave);

	// Write out the thumbnail
	Graphics::Surface thumb;
	getSavegameThumbnail(thumb);
	Graphics::saveThumbnail(*saveFile, thumb);
	thumb.free();

	saveFile->writeUint32LE(headerPos);	// Store where the header starts
}

bool MetaEngine::copySaveFileToFreeSlot(const char *target, int slot) {
	const int emptySlot = findEmptySaveSlot(target);
	if (emptySlot == -1)
		return false;
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	return saveFileMan->copySavefile(getSavegameFile(slot, target), getSavegameFile(emptySlot, target));
}

void MetaEngine::getSavegameThumbnail(Graphics::Surface &thumb) {
	::createThumbnailFromScreen(&thumb);
}

void MetaEngine::parseSavegameHeader(ExtendedSavegameHeader *header, SaveStateDescriptor *desc) {
	uint8 day = 0;
	uint8 month = 0;
	uint16 year = 0;
	decodeSavegameDate(header, year, month, day);
	desc->setSaveDate(year, month, day);

	uint8 hour = 0;
	uint8 minutes = 0;
	decodeSavegameTime(header, hour, minutes);

	desc->setSaveTime(hour, minutes);
	desc->setPlayTime(header->playtime);

	desc->setDescription(header->description);
}

void MetaEngine::fillDummyHeader(ExtendedSavegameHeader *header) {
	// This is wrong header, perhaps it is original savegame. Thus fill out dummy values
	header->date = (20 << 24) | (9 << 16) | 2016;
	header->time = (9 << 8) | 56;
	header->playtime = 0;
}

void MetaEngine::decodeSavegameDate(const ExtendedSavegameHeader *header, uint16 &outYear, uint8 &outMonth, uint8 &outDay) {
	outYear = static_cast<uint16>(header->date & 0xffff);
	outMonth = static_cast<uint8>((header->date >> 16) & 0xff);
	outDay = static_cast<uint8>((header->date >> 24) & 0xff);
}

void MetaEngine::decodeSavegameTime(const ExtendedSavegameHeader *header, uint8 &outHour, uint8 &outMinute) {
	outMinute = static_cast<uint16>(header->time & 0xff);
	outHour = static_cast<uint8>((header->time >> 8) & 0xff);
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

	// Get the flag for whether it's an autosave
	header->isAutosave = (header->version >= 4) ? in->readByte() : false;

	// Get the thumbnail
	if (!Graphics::loadThumbnail(*in, header->thumbnail, skipThumbnail)) {
		in->seek(oldPos, SEEK_SET); // Rewind the file
		return false;
	}

	in->seek(oldPos, SEEK_SET); // Rewind the file

	return true;
}


//////////////////////////////////////////////
// MetaEngine default implementations
//////////////////////////////////////////////

int MetaEngine::findEmptySaveSlot(const char *target) {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	const int maxSaveSlot = getMaximumSaveSlot();
	const int autosaveSlot = getAutosaveSlot();
	for (int slot = 0; slot <= maxSaveSlot; ++slot) {
		if (slot == autosaveSlot)
			continue;
		const Common::String filename = getSavegameFile(slot, target);
		if (!saveFileMan->exists(filename))
			return slot;
	}
	return -1;
}

SaveStateList MetaEngine::listSaves(const char *target) const {
	if (!hasFeature(kSavesUseExtendedFormat))
		return SaveStateList();

	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String pattern(getSavegameFilePattern(target));

	filenames = saveFileMan->listSavefiles(pattern);

	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		// Obtain the last 2/3 digits of the filename, since they correspond to the save slot
		const char *slotStr = file->c_str() + file->size() - 2;
		const char *prev = slotStr - 1;
		if (*prev >= '0' && *prev <= '9')
			slotStr = prev;
		int slotNum = atoi(slotStr);

		if (slotNum >= 0 && slotNum <= getMaximumSaveSlot()) {
			SaveStateDescriptor desc = querySaveMetaInfos(target, slotNum);
			if (desc.getSaveSlot() != -1) {
				saveList.push_back(desc);
			}
		}
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

SaveStateList MetaEngine::listSaves(const char *target, bool saveMode) const {
	SaveStateList saveList = listSaves(target);
	int autosaveSlot = getAutosaveSlot();
	if (!saveMode || autosaveSlot == -1)
		return saveList;

	// Check to see if an autosave is present
	for (SaveStateList::iterator it = saveList.begin(); it != saveList.end(); ++it) {
		int slot = it->getSaveSlot();
		if (slot == autosaveSlot) {
			// It has an autosave
			return saveList;
		}
	}

	// No autosave yet. We want to add a dummy one in so that it can be marked as
	// write protected, and thus be prevented from being saved in
	const Common::U32String &dummyAutosave = (ConfMan.getInt("autosave_period") ? _("Autosave on") : _("Autosave off"));
	SaveStateDescriptor desc(this, autosaveSlot, dummyAutosave);
	desc.setWriteProtectedFlag(true);
	desc.setDeletableFlag(false);
	
	saveList.push_back(desc);
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());

	return saveList;
}

void MetaEngine::registerDefaultSettings(const Common::String &) const {
	// Note that as we don't pass the target to getExtraGuiOptions
	//  we get all the options, even those not relevant for the current
	//  game. This is necessary because some engines unconditionally
	//  access the configuration.
	const ExtraGuiOptions engineOptions = getExtraGuiOptions("");
	for (uint i = 0; i < engineOptions.size(); i++) {
		ConfMan.registerDefault(engineOptions[i].configOption, engineOptions[i].defaultState);
	}
}

GUI::OptionsContainerWidget *MetaEngine::buildEngineOptionsWidget(GUI::GuiObject *boss, const Common::String &name, const Common::String &target) const {
	const ExtraGuiOptions engineOptions = getExtraGuiOptions(target);
	if (engineOptions.empty()) {
		return nullptr;
	}

	return new GUI::ExtraGuiOptionsWidget(boss, name, target, engineOptions);
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
		getSavegameFile(slot, target)));

	if (f) {
		ExtendedSavegameHeader header;
		if (!readSavegameHeader(f.get(), &header, false)) {
			return SaveStateDescriptor();
		}

		// Create the return descriptor
		SaveStateDescriptor desc(this, slot, Common::U32String());
		parseSavegameHeader(&header, &desc);
		desc.setThumbnail(header.thumbnail);
		desc.setAutosave(header.isAutosave);
		return desc;
	}

	return SaveStateDescriptor();
}
