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

#include "sherlock/sherlock.h"
#include "sherlock/saveload.h"
#include "sherlock/scalpel/scalpel.h"
#include "sherlock/tattoo/tattoo.h"

#include "common/system.h"
#include "common/translation.h"
#include "engines/advancedDetector.h"

#include "sherlock/detection.h"

namespace Sherlock {

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_ORIGINAL_SAVES,
		{
			_s("Use original save/load screens"),
			_s("Use the original save/load screens instead of the ScummVM ones"),
			"originalsaveload",
			false,
			0,
			0
		}
	},

	{
		GAMEOPTION_FADE_STYLE,
		{
			_s("Pixellated scene transitions"),
			_s("When changing scenes, a randomized pixel transition is done"),
			"fade_style",
			true,
			0,
			0
		}
	},

	{
		GAMEOPTION_HELP_STYLE,
		{
			_s("Don't show hotspots when moving mouse"),
			_s("Only show hotspot names after you actually click on a hotspot or action button"),
			"help_style",
			false,
			0,
			0
		}
	},

	{
		GAMEOPTION_PORTRAITS_ON,
		{
			_s("Show character portraits"),
			_s("Show portraits for the characters when conversing"),
			"portraits_on",
			true,
			0,
			0
		}
	},

	{
		GAMEOPTION_WINDOW_STYLE,
		{
			_s("Slide dialogs into view"),
			_s("Slide UI dialogs into view, rather than simply showing them immediately"),
			"window_style",
			true,
			0,
			0
		}
	},

	{
		GAMEOPTION_TRANSPARENT_WINDOWS,
		{
			_s("Transparent windows"),
			_s("Show windows with a partially transparent background"),
			"transparent_windows",
			true,
			0,
			0
		}
	},

#ifdef USE_TTS
	{
		GAMEOPTION_TTS_NARRATOR,
		{
			_s("TTS Narrator"),
			_s("Use TTS to read the descriptions (if TTS is available)"),
			"tts_narrator",
			false,
			0,
			0
		}
	},
#endif
	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

GameType SherlockEngine::getGameID() const {
	return _gameDescription->gameID;
}

Common::Platform SherlockEngine::getPlatform() const {
	return _gameDescription->desc.platform;
}

Common::Language SherlockEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

} // End of namespace Sherlock


class SherlockMetaEngine : public AdvancedMetaEngine<Sherlock::SherlockGameDescription> {
public:
	const char *getName() const override {
		return "sherlock";
	}

	const ADExtraGuiOptionsMap *getAdvancedExtraGuiOptions() const override {
		return Sherlock::optionsList;
	}

	/**
	 * Creates an instance of the game engine
	 */
	Common::Error createInstance(OSystem *syst, Engine **engine, const Sherlock::SherlockGameDescription *desc) const override;

	/**
	 * Returns a list of features the game's MetaEngine support
	 */
	bool hasFeature(MetaEngineFeature f) const override;

	/**
	 * Return a list of savegames
	 */
	SaveStateList listSaves(const char *target) const override;

	/**
	 * Returns the maximum number of allowed save slots
	 */
	int getMaximumSaveSlot() const override;

	/**
	 * Deletes a savegame in the specified slot
	 */
	bool removeSaveState(const char *target, int slot) const override;

	/**
	 * Given a specified savegame slot, returns extended information for the save
	 */
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;
};

Common::Error SherlockMetaEngine::createInstance(OSystem *syst, Engine **engine, const Sherlock::SherlockGameDescription *gd) const {
	switch (gd->gameID) {
	case Sherlock::GType_SerratedScalpel:
		*engine = new Sherlock::Scalpel::ScalpelEngine(syst, gd);
		break;
	case Sherlock::GType_RoseTattoo:
		*engine = new Sherlock::Tattoo::TattooEngine(syst, gd);
		break;
	default:
		return Common::kUnsupportedGameidError;
	}
	return Common::kNoError;
}

bool SherlockMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSavesSupportCreationDate) ||
		(f == kSavesSupportPlayTime) ||
		(f == kSimpleSavesNames);
}

bool Sherlock::SherlockEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

bool Sherlock::SherlockEngine::isDemo() const {
	return _gameDescription->desc.flags & ADGF_DEMO;
}

SaveStateList SherlockMetaEngine::listSaves(const char *target) const {
	return Sherlock::SaveManager::getSavegameList(this, target);
}

int SherlockMetaEngine::getMaximumSaveSlot() const {
	return MAX_SAVEGAME_SLOTS;
}

bool SherlockMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String filename = Sherlock::SaveManager(nullptr, target).generateSaveName(slot);
	return g_system->getSavefileManager()->removeSavefile(filename);
}

SaveStateDescriptor SherlockMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String filename = Sherlock::SaveManager(nullptr, target).generateSaveName(slot);
	Common::InSaveFile *f = g_system->getSavefileManager()->openForLoading(filename);

	if (f) {
		Sherlock::SherlockSavegameHeader header;
		if (!Sherlock::SaveManager::readSavegameHeader(f, header, false)) {
			delete f;
			return SaveStateDescriptor();
		}
		delete f;

		// Create the return descriptor
		SaveStateDescriptor desc(this, slot, header._saveName);
		desc.setThumbnail(header._thumbnail);
		desc.setSaveDate(header._year, header._month, header._day);
		desc.setSaveTime(header._hour, header._minute);
		desc.setPlayTime(header._totalFrames * GAME_FRAME_TIME);

		return desc;
	}

	return SaveStateDescriptor();
}

#if PLUGIN_ENABLED_DYNAMIC(SHERLOCK)
	REGISTER_PLUGIN_DYNAMIC(SHERLOCK, PLUGIN_TYPE_ENGINE, SherlockMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(SHERLOCK, PLUGIN_TYPE_ENGINE, SherlockMetaEngine);
#endif
