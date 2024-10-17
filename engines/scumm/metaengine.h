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

#ifndef SCUMM_METAENGINE_H
#define SCUMM_METAENGINE_H

#include "engines/metaengine.h"

class ScummMetaEngine : public MetaEngine {
	const char *getName() const override;

	bool hasFeature(MetaEngineFeature f) const override;

	Common::Error createInstance(OSystem *syst, Engine **engine,
	                             const DetectedGame &gameDescriptor, const void *metaEngineDescriptor) override;

	SaveStateList listSaves(const char *target) const override;
	int getMaximumSaveSlot() const override;
	bool removeSaveState(const char *target, int slot) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;

	const ExtraGuiOptions getExtraGuiOptions(const Common::String &target) const override;
	void registerDefaultSettings(const Common::String &) const override;

	GUI::OptionsContainerWidget *buildEngineOptionsWidget(GUI::GuiObject *boss, const Common::String &name, const Common::String &target) const override;

	Common::KeymapArray initKeymaps(const char *target) const override;

private:
	GUI::OptionsContainerWidget *buildLoomOptionsWidget(GUI::GuiObject *boss, const Common::String &name, const Common::String &target) const;
	GUI::OptionsContainerWidget *buildMI1OptionsWidget(GUI::GuiObject *boss, const Common::String &name, const Common::String &target) const;
};

#endif // SCUMM_METAENGINE_H
