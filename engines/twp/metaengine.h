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

#ifndef TWP_METAENGINE_H
#define TWP_METAENGINE_H

#include "engines/advancedDetector.h"

class TwpMetaEngine : public AdvancedMetaEngine<Twp::TwpGameDescription> {
public:
	const char *getName() const override;

	Common::Error createInstance(OSystem *syst, Engine **engine, const Twp::TwpGameDescription *desc) const override;

	/**
	 * Determine whether the engine supports the specified MetaEngine feature.
	 *
	 * Used by e.g. the launcher to determine whether to enable the Load button.
	 */
	bool hasFeature(MetaEngineFeature f) const override;

	int getMaximumSaveSlot() const override;

	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;
	void registerDefaultSettings(const Common::String &) const override;

	Common::AchievementsPlatform getAchievementsPlatform(const Common::String &target) const override;
	const Common::AchievementDescriptionList *getAchievementDescriptionList() const override;

	GUI::OptionsContainerWidget *buildEngineOptionsWidget(GUI::GuiObject *boss, const Common::String &name, const Common::String &target) const override;

	Common::Array<Common::Keymap *> initKeymaps(const char *target) const override;

	void getSavegameThumbnail(Graphics::Surface &thumb) override;
};

#endif // TWP_METAENGINE_H
