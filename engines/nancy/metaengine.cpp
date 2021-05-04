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

#include "engines/advancedDetector.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/input.h"
#include "engines/nancy/dialogs.h"

class NancyMetaEngine : public AdvancedMetaEngine {
public:
	virtual const char *getName() const override {
		return "nancy";
	}

	virtual bool hasFeature(MetaEngineFeature f) const override;
	virtual Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *gd) const override;

	virtual int getMaximumSaveSlot() const override;

	virtual Common::KeymapArray initKeymaps(const char *target) const override;

	virtual GUI::OptionsContainerWidget *buildEngineOptionsWidgetDynamic(GUI::GuiObject *boss, const Common::String &name, const Common::String &target) const override;
};

Common::KeymapArray NancyMetaEngine::initKeymaps(const char *target) const {
	Common::KeymapArray keymaps;
	Nancy::InputManager::initKeymaps(keymaps);
	return keymaps;
}

bool NancyMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSavesSupportCreationDate) ||
		(f == kSavesSupportPlayTime) ||
		(f == kSavesUseExtendedFormat);
}

Common::Error NancyMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *gd) const {
	if (gd) {
		*engine = Nancy::NancyEngine::create(((const Nancy::NancyGameDescription *)gd)->gameType, syst, (const Nancy::NancyGameDescription *)gd);
	}

	if (gd) {
		return Common::kNoError;
	} else {
		return Common::Error();
	}
}

int NancyMetaEngine::getMaximumSaveSlot() const { return 8; }

GUI::OptionsContainerWidget *NancyMetaEngine::buildEngineOptionsWidgetDynamic(GUI::GuiObject *boss, const Common::String &name, const Common::String &target) const {
	return new Nancy::NancyOptionsWidget(boss, name, target);
}

#if PLUGIN_ENABLED_DYNAMIC(NANCY)
	REGISTER_PLUGIN_DYNAMIC(NANCY, PLUGIN_TYPE_ENGINE, NancyMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(NANCY, PLUGIN_TYPE_ENGINE, NancyMetaEngine);
#endif
