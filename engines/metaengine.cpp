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
#include "backends/keymapper/standard-actions.h"

#include "common/savefile.h"
#include "common/system.h"
#include "common/translation.h"

#include "engines/dialogs.h"

#include "graphics/palette.h"
#include "graphics/scaler.h"
#include "graphics/managed_surface.h"
#include "graphics/thumbnail.h"

Common::String MetaEngine::getSavegameFile(int saveGameIdx, const char *target) const {
	if (saveGameIdx == kSavegameFilePattern) {
		// Pattern requested
		const char *pattern = hasFeature(kSavesUseExtendedFormat) ? "%s.###" : "%s.s##";
		return Common::String::format(pattern, target == nullptr ? getEngineId() : target);
	} else {
		// Specific filename requested
		const char *pattern = hasFeature(kSavesUseExtendedFormat) ? "%s.%03d" : "%s.s%02d";
		return Common::String::format(pattern, target == nullptr ? getEngineId() : target, saveGameIdx);
	}
}

Common::KeymapArray MetaEngine::initKeymaps(const char *target) const {
	using namespace Common;

	Keymap *engineKeyMap = new Keymap(Keymap::kKeymapTypeGame, "engine-default", _("Default game keymap"));

	Action *act;

	act = new Action("LCLK", _("Left Click"));
	act->setLeftClickEvent();
	act->addDefaultInputMapping("MOUSE_LEFT");
	act->addDefaultInputMapping("JOY_A");
	engineKeyMap->addAction(act);

	act = new Action("MCLK", _("Middle Click"));
	act->addDefaultInputMapping("MOUSE_MIDDLE");
	act->setMiddleClickEvent();
	engineKeyMap->addAction(act);

	act = new Action("RCLK", _("Right Click"));
	act->setRightClickEvent();
	act->addDefaultInputMapping("MOUSE_RIGHT");
	act->addDefaultInputMapping("JOY_B");
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionPause, _("Pause"));
	act->setKeyEvent(KeyState(KEYCODE_SPACE, ' '));
	act->addDefaultInputMapping("SPACE");
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionOpenMainMenu, _("Game menu"));
	act->setKeyEvent(KeyState(KEYCODE_F5, ASCII_F5));
	act->addDefaultInputMapping("F5");
	act->addDefaultInputMapping("JOY_LEFT_SHOULDER");
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionSkip, _("Skip"));
	act->setKeyEvent(KeyState(KEYCODE_ESCAPE, ASCII_ESCAPE));
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("JOY_Y");
	engineKeyMap->addAction(act);

	act = new Action("SKLI", _("Skip line"));
	act->setKeyEvent(KeyState(KEYCODE_PERIOD, '.'));
	act->addDefaultInputMapping("PERIOD");
	act->addDefaultInputMapping("JOY_X");
	engineKeyMap->addAction(act);

	act = new Action("PIND", _("Predictive input dialog"));
	act->setEvent(EVENT_PREDICTIVE_DIALOG);
	engineKeyMap->addAction(act);

	act = new Action("RETURN", _("Confirm"));
	act->setKeyEvent(KeyState(KEYCODE_RETURN, ASCII_RETURN));
	act->addDefaultInputMapping("RETURN");
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionMoveUp, _("Up"));
	act->setKeyEvent(KEYCODE_KP8);
	act->addDefaultInputMapping("JOY_UP");
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionMoveDown, _("Down"));
	act->setKeyEvent(KEYCODE_KP2);
	act->addDefaultInputMapping("JOY_DOWN");
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionMoveLeft, _("Left"));
	act->setKeyEvent(KEYCODE_KP4);
	act->addDefaultInputMapping("JOY_LEFT");
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionMoveRight, _("Right"));
	act->setKeyEvent(KEYCODE_KP6);
	act->addDefaultInputMapping("JOY_RIGHT");
	engineKeyMap->addAction(act);

	return Keymap::arrayOf(engineKeyMap);
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
		(f == kSavesUseExtendedFormat);
}

void MetaEngine::appendExtendedSave(Common::OutSaveFile *saveFile, uint32 playtime,
		Common::String desc, bool isAutosave) {
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
	saveFile->writeByte(isAutosave);

	saveScreenThumbnail(saveFile);

	saveFile->writeUint32LE(headerPos);	// Store where the header starts

	saveFile->finalize();
}

void MetaEngine::saveScreenThumbnail(Common::OutSaveFile *saveFile) {
	// Create a thumbnail surface from the screen
	Graphics::Surface thumb;
	::createThumbnailFromScreen(&thumb);

	// Write out the thumbnail
	Graphics::saveThumbnail(*saveFile, thumb);
	thumb.free();
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
// MetaEngineConnect default implementations
//////////////////////////////////////////////

SaveStateList MetaEngine::listSaves(const char *target) const {
	if (!hasFeature(kSavesUseExtendedFormat))
		return SaveStateList();

	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String pattern(getSavegameFilePattern(target));

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
				if (slotNum == getAutosaveSlot())
					desc.setWriteProtectedFlag(true);

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
	int autosaveSlot = ConfMan.getInt("autosave_period") ? getAutosaveSlot() : -1;
	if (!saveMode || autosaveSlot == -1)
		return saveList;

	// Check to see if an autosave is present
	for (SaveStateList::iterator it = saveList.begin(); it != saveList.end(); ++it) {
		int slot = it->getSaveSlot();
		if (slot == autosaveSlot) {
			// It has an autosave
			it->setWriteProtectedFlag(true);
			return saveList;
		}
	}

	// No autosave yet. We want to add a dummy one in so that it can be marked as'
	// write protected, and thus be prevented from being saved in
	SaveStateDescriptor desc;
	desc.setDescription(_("Autosave"));
	desc.setSaveSlot(autosaveSlot);
	desc.setWriteProtectedFlag(true);

	saveList.push_back(desc);
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());

	return saveList;
}

void MetaEngineDetection::registerDefaultSettings(const Common::String &) const {
	// Note that as we don't pass the target to getExtraGuiOptions
	//  we get all the options, even those not relevant for the current
	//  game. This is necessary because some engines unconditionally
	//  access the configuration.
	const ExtraGuiOptions engineOptions = getExtraGuiOptions("");
	for (uint i = 0; i < engineOptions.size(); i++) {
		ConfMan.registerDefault(engineOptions[i].configOption, engineOptions[i].defaultState);
	}
}

GUI::OptionsContainerWidget *MetaEngineDetection::buildEngineOptionsWidgetStatic(GUI::GuiObject *boss, const Common::String &name, const Common::String &target) const {
	const ExtraGuiOptions engineOptions = getExtraGuiOptions(target);
	if (engineOptions.empty()) {
		return nullptr;
	}

	return new GUI::ExtraGuiOptionsWidget(boss, name, target, engineOptions);
}

GUI::OptionsContainerWidget *MetaEngine::buildEngineOptionsWidgetDynamic(GUI::GuiObject *boss, const Common::String &name, const Common::String &target) const {
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
		SaveStateDescriptor desc;

		parseSavegameHeader(&header, &desc);

		desc.setSaveSlot(slot);
		desc.setThumbnail(header.thumbnail);
		desc.setAutosave(header.isAutosave);
		if (slot == getAutosaveSlot())
			desc.setWriteProtectedFlag(true);

		return desc;
	}

	return SaveStateDescriptor();
}
