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

#ifndef AGS_METAENGINE_H
#define AGS_METAENGINE_H

#include "engines/achievements.h"
#include "engines/advancedDetector.h"

#include "engines/ags/detection.h"

class AGSMetaEngine : public AdvancedMetaEngine<AGS::AGSGameDescription> {
public:
	const char *getName() const override;

	Common::Error createInstance(OSystem *syst, Engine **engine, const AGS::AGSGameDescription *desc) const override;

	SaveStateList listSaves(const char *target) const override;

	int getAutosaveSlot() const override;

	int getMaximumSaveSlot() const override {
		return 998;
	}

	/**
	 * Return the name of the save file for the given slot and optional target,
	 * or a pattern for matching filenames against.
	 *
	 * @param saveGameIdx  Index of the save, or kSavegameFilePattern
	 *                     for returning a filename pattern.
	 * @param target       Game target. If omitted, then the engine ID is used.
	 */
	Common::String getSavegameFile(int saveGameIdx, const char *target = nullptr) const override;

	GUI::OptionsContainerWidget *buildEngineOptionsWidget(GUI::GuiObject *boss, const Common::String &name, const Common::String &target) const override;

	/**
	 * Determine whether the engine supports the specified MetaEngine feature.
	 *
	 * Used by e.g. the launcher to determine whether to enable the Load button.
	 */
	bool hasFeature(MetaEngineFeature f) const override;

	/**
	 * Return meta information from the specified save state.
	 *
	 * Depending on the MetaEngineFeatures set, this can include
	 * thumbnails, save date and time, play time.
	 *
	 * @param target  Name of a config manager target.
	 * @param slot    Slot number of the save state.
	 */
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;

	/**
	 * Remove the specified save state.
	 *
	 * @param target  Name of a config manager target.
	 * @param slot    Slot number of the save state to be removed.
	 */
	void removeSaveState(const char *target, int slot) const override;

	const Common::AchievementDescriptionList *getAchievementDescriptionList() const override;

	static Common::StringArray getGameTranslations(const Common::String &domain);
};

#endif
