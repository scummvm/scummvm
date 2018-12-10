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

#include "sherlock/sherlock.h"
#include "sherlock/saveload.h"
#include "sherlock/scalpel/scalpel.h"
#include "sherlock/tattoo/tattoo.h"
#include "common/system.h"
#include "common/translation.h"
#include "engines/advancedDetector.h"

namespace Sherlock {

struct SherlockGameDescription {
	ADGameDescription desc;

	GameType gameID;
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

static const PlainGameDescriptor sherlockGames[] = {
	{ "scalpel", "The Case of the Serrated Scalpel" },
	{ "rosetattoo", "The Case of the Rose Tattoo" },
	{0, 0}
};


#define GAMEOPTION_ORIGINAL_SAVES	GUIO_GAMEOPTIONS1
#define GAMEOPTION_FADE_STYLE		GUIO_GAMEOPTIONS2
#define GAMEOPTION_HELP_STYLE		GUIO_GAMEOPTIONS3
#define GAMEOPTION_PORTRAITS_ON		GUIO_GAMEOPTIONS4
#define GAMEOPTION_WINDOW_STYLE		GUIO_GAMEOPTIONS5
#define GAMEOPTION_TRANSPARENT_WINDOWS		GUIO_GAMEOPTIONS6

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_ORIGINAL_SAVES,
		{
			_s("Use original save/load screens"),
			_s("Use the original save/load screens instead of the ScummVM ones"),
			"originalsaveload",
			false
		}
	},

	{
		GAMEOPTION_FADE_STYLE,
		{
			_s("Pixellated scene transitions"),
			_s("When changing scenes, a randomized pixel transition is done"),
			"fade_style",
			true
		}
	},

	{
		GAMEOPTION_HELP_STYLE,
		{
			_s("Don't show hotspots when moving mouse"),
			_s("Only show hotspot names after you actually click on a hotspot or action button"),
			"help_style",
			false
		}
	},

	{
		GAMEOPTION_PORTRAITS_ON,
		{
			_s("Show character portraits"),
			_s("Show portraits for the characters when conversing"),
			"portraits_on",
			true
		}
	},

	{
		GAMEOPTION_WINDOW_STYLE,
		{
			_s("Slide dialogs into view"),
			_s("Slide UI dialogs into view, rather than simply showing them immediately"),
			"window_style",
			true
		}
	},

	{
		GAMEOPTION_TRANSPARENT_WINDOWS,
		{
			_s("Transparent windows"),
			_s("Show windows with a partially transparent background"),
			"transparent_windows",
			true
		}
	},

	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};


#include "sherlock/detection_tables.h"

class SherlockMetaEngine : public AdvancedMetaEngine {
public:
	SherlockMetaEngine() : AdvancedMetaEngine(Sherlock::gameDescriptions, sizeof(Sherlock::SherlockGameDescription),
		sherlockGames, optionsList) {}

	virtual const char *getName() const {
		return "Sherlock";
	}

	virtual const char *getOriginalCopyright() const {
		return "Sherlock (C) 1992-1996 Mythos Software, (C) 1992-1996 Electronic Arts";
	}

	/**
	 * Creates an instance of the game engine
	 */
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;

	/**
	 * Returns a list of features the game's MetaEngine support
	 */
	virtual bool hasFeature(MetaEngineFeature f) const;

	/**
	 * Return a list of savegames
	 */
	virtual SaveStateList listSaves(const char *target) const;

	/**
	 * Returns the maximum number of allowed save slots
	 */
	virtual int getMaximumSaveSlot() const;

	/**
	 * Deletes a savegame in the specified slot
	 */
	virtual void removeSaveState(const char *target, int slot) const;

	/**
	 * Given a specified savegame slot, returns extended information for the save
	 */
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const;
};

bool SherlockMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const Sherlock::SherlockGameDescription *gd = (const Sherlock::SherlockGameDescription *)desc;
	if (gd) {
		switch (gd->gameID) {
		case Sherlock::GType_SerratedScalpel:
			*engine = new Sherlock::Scalpel::ScalpelEngine(syst, gd);
			break;
		case Sherlock::GType_RoseTattoo:
			*engine = new Sherlock::Tattoo::TattooEngine(syst, gd);
			break;
		default:
			error("Unknown game");
			break;
		}
	}
	return gd != 0;
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
		(f == kSupportsRTL) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

bool Sherlock::SherlockEngine::isDemo() const {
	return _gameDescription->desc.flags & ADGF_DEMO;
}

SaveStateList SherlockMetaEngine::listSaves(const char *target) const {
	return Sherlock::SaveManager::getSavegameList(target);
}

int SherlockMetaEngine::getMaximumSaveSlot() const {
	return MAX_SAVEGAME_SLOTS;
}

void SherlockMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String filename = Sherlock::SaveManager(nullptr, target).generateSaveName(slot);
	g_system->getSavefileManager()->removeSavefile(filename);
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
		SaveStateDescriptor desc(slot, header._saveName);
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
