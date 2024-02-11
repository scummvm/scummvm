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
#include "engines/nancy/state/scene.h"

#include "common/translation.h"
#include "common/config-manager.h"

#include "graphics/scaler.h"

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_PLAYER_SPEECH,
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
		GAMEOPTION_CHARACTER_SPEECH,
		{
			_s("Character Speech"),
			_s("Enable NPC speech. Only works if speech is enabled in the Audio settings."),
			"character_speech",
			true,
			0,
			0
		}
	},
	{
		GAMEOPTION_AUTO_MOVE,
		{
			_s("Auto Move"),
			_s("Automatically rotate the viewport when the mouse reaches an edge."),
			"auto_move",
			true,
			0,
			0
		}
	},
	{
		GAMEOPTION_FIX_SOFTLOCKS,
		{
			_s("Fix softlocks"),
			_s("Fix instances where missing something earlier in the game blocks you from progressing any further."),
			"softlocks_fix",
			true,
			0,
			0
		}
	},
	{
		GAMEOPTION_FIX_ANNOYANCES,
		{
			_s("Fix annoyances"),
			_s("Fix various minor annoyances."),
			"annoyances_fix",
			true,
			0,
			0
		}
	},
	{
		GAMEOPTION_NANCY2_TIMER,
		{
			_s("Extend endgame timer"),
			_s("Get a little more time before failing the final puzzle. This is an official patch by HeR Interactive."),
			"final_timer",
			false,
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
	Common::KeymapArray keymaps = MetaEngine::initKeymaps(target);
	Nancy::InputManager::initKeymaps(keymaps, target);
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

int NancyMetaEngine::getMaximumSaveSlot() const { int r = ConfMan.getInt("nancy_max_saves"); return r ? r : AdvancedMetaEngine::getMaximumSaveSlot(); }

SaveStateDescriptor NancyMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	SaveStateDescriptor ret = AdvancedMetaEngine::querySaveMetaInfos(target, slot);
	if (slot == getMaximumSaveSlot()) {
		// We do not allow the second chance slot to be overwritten
		ret.setWriteProtectedFlag(true);
	}

	return ret;
}

void NancyMetaEngine::getSavegameThumbnail(Graphics::Surface &thumb) {
	if (Nancy::g_nancy->getState() == Nancy::NancyState::kLoadSave) {
		// Do not screenshot the screen if we're in the engine's original menus,
		// since that would just screenshot the menu itself
		if (Nancy::State::Scene::hasInstance()) {
			Graphics::ManagedSurface &screenshot = Nancy::State::Scene::instance().getLastScreenshot();
			if (!screenshot.empty() && createThumbnail(&thumb, &screenshot)) {
				return;
			}
		}
	}

	// Make sure we always trigger a screen redraw to support second chance saves
	Graphics::ManagedSurface screenshotSurf;
	Nancy::g_nancy->_graphics->screenshotScreen(screenshotSurf);
	if (!screenshotSurf.empty() && createThumbnail(&thumb, &screenshotSurf)) {
		return;
	}
}

void NancyMetaEngine::registerDefaultSettings(const Common::String &target) const {
	ConfMan.registerDefault("music_volume", 54 * 255 / 100);
	ConfMan.registerDefault("speech_volume", 54 * 255 / 100);
	ConfMan.registerDefault("sfx_volume", 51 * 255 / 100);
	ConfMan.registerDefault("subtitles", true);

	ConfMan.registerDefault("player_speech", true);
	ConfMan.registerDefault("character_speech", true);
	ConfMan.registerDefault("nancy_max_saves", 999);
}

const ADExtraGuiOptionsMap *NancyMetaEngine::getAdvancedExtraGuiOptions() const {
	return optionsList;
}

#if PLUGIN_ENABLED_DYNAMIC(NANCY)
	REGISTER_PLUGIN_DYNAMIC(NANCY, PLUGIN_TYPE_ENGINE, NancyMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(NANCY, PLUGIN_TYPE_ENGINE, NancyMetaEngine);
#endif
