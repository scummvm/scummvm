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

#include "engines/advancedDetector.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/input.h"

#include "common/translation.h"
#include "common/config-manager.h"

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GUIO_GAMEOPTIONS1,
		{
			_s("Player Speech"),
			_s("Enable player speech. Only works if speech is enabled in the Audio settings."),
			"player_speech",
			true,
			0,
			0
		}
	},
	{
		GUIO_GAMEOPTIONS2,
		{
			_s("Character Speech"),
			_s("Enable NPC speech. Only works if speech is enabled in the Audio settings."),
			"character_speech",
			true,
			0,
			0
		}
	},
	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

class NancyMetaEngine : public AdvancedMetaEngine {
public:
	const char *getName() const override {
		return "nancy";
	}

	bool hasFeature(MetaEngineFeature f) const override;
	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *gd) const override;

	int getMaximumSaveSlot() const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;

	Common::KeymapArray initKeymaps(const char *target) const override;

	void getSavegameThumbnail(Graphics::Surface &thumb) override;

	void registerDefaultSettings(const Common::String &target) const override;

	const ADExtraGuiOptionsMap *getAdvancedExtraGuiOptions() const override;
};

Common::KeymapArray NancyMetaEngine::initKeymaps(const char *target) const {
	Common::KeymapArray keymaps;
	Nancy::InputManager::initKeymaps(keymaps);
	return keymaps;
}

bool NancyMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsLoadingDuringStartup) ||
		checkExtendedSaves(f);
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

SaveStateDescriptor NancyMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	SaveStateDescriptor ret = AdvancedMetaEngine::querySaveMetaInfos(target, slot);
	if (slot == getMaximumSaveSlot()) {
		// We do not allow the second chance slot to be overwritten
		ret.setWriteProtectedFlag(true);
	}

	return ret;
}

void NancyMetaEngine::getSavegameThumbnail(Graphics::Surface &thumb) {
	// Second Chance autosaves trigger when a scene changes, but before
	// it is drawn, so we need to refresh the screen before we take a screenshot
	Nancy::g_nancy->_graphicsManager->draw();
	AdvancedMetaEngine::getSavegameThumbnail(thumb);
}

void NancyMetaEngine::registerDefaultSettings(const Common::String &target) const {
	ConfMan.setInt("music_volume", 54 * 255 / 100, target);
	ConfMan.setInt("speech_volume", 54 * 255 / 100, target);
	ConfMan.setInt("sfx_volume", 51 * 255 / 100, target);
	ConfMan.setBool("subtitles", true, target);

	ConfMan.setBool("player_speech", true, target);
	ConfMan.setBool("character_speech", true, target);
}

const ADExtraGuiOptionsMap *NancyMetaEngine::getAdvancedExtraGuiOptions() const {
	return optionsList;
}

#if PLUGIN_ENABLED_DYNAMIC(NANCY)
	REGISTER_PLUGIN_DYNAMIC(NANCY, PLUGIN_TYPE_ENGINE, NancyMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(NANCY, PLUGIN_TYPE_ENGINE, NancyMetaEngine);
#endif
