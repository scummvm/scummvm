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
 *
 */

#include "mads/mads.h"

#include "base/plugins.h"
#include "common/savefile.h"
#include "common/str-array.h"
#include "common/memstream.h"
#include "engines/advancedDetector.h"
#include "common/system.h"
#include "common/translation.h"
#include "graphics/colormasks.h"
#include "graphics/surface.h"
#include "mads/events.h"
#include "mads/game.h"

#define MAX_SAVES 99

namespace MADS {

struct MADSGameDescription {
	ADGameDescription desc;

	int gameID;
	uint32 features;
};

uint32 MADSEngine::getGameID() const {
	return _gameDescription->gameID;
}

uint32 MADSEngine::getGameFeatures() const {
	return _gameDescription->features;
}

uint32 MADSEngine::getFeatures() const {
	return _gameDescription->desc.flags;
}

Common::Language MADSEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

Common::Platform MADSEngine::getPlatform() const {
	return _gameDescription->desc.platform;
}

} // End of namespace MADS

static const PlainGameDescriptor MADSGames[] = {
	{"MADS", "MADS"},
	{"dragonsphere", "Dragonsphere"},
	{"nebular", "Rex Nebular and the Cosmic Gender Bender"},
	{"phantom", "Return of the Phantom"},
	{0, 0}
};

#define GAMEOPTION_EASY_MOUSE          GUIO_GAMEOPTIONS1
#define GAMEOPTION_ANIMATED_INVENTORY  GUIO_GAMEOPTIONS2
#define GAMEOPTION_ANIMATED_INTERFACE  GUIO_GAMEOPTIONS3
#define GAMEOPTION_NAUGHTY_MODE        GUIO_GAMEOPTIONS4
//#define GAMEOPTION_GRAPHICS_DITHERING  GUIO_GAMEOPTIONS5

#include "mads/detection_tables.h"

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_EASY_MOUSE,
		{
			_s("Easy mouse interface"),
			_s("Shows object names when hovering the mouse over them"),
			"EasyMouse",
			true
		}
	},

	{
		GAMEOPTION_ANIMATED_INVENTORY,
		{
			_s("Animated inventory items"),
			_s("Animated inventory items"),
			"InvObjectsAnimated",
			true
		}
	},

	{
		GAMEOPTION_ANIMATED_INTERFACE,
		{
			_s("Animated game interface"),
			_s("Animated game interface"),
			"TextWindowAnimated",
			true
		}
	},

	{
		GAMEOPTION_NAUGHTY_MODE,
		{
			_s("Naughty game mode"),
			_s("Naughty game mode"),
			"NaughtyMode",
			true
		}
	},

	/*{
		GAMEOPTION_GRAPHICS_DITHERING,
		{
			_s("Graphics dithering"),
			_s("Graphics dithering"),
			"GraphicsDithering",
			true
		}
	},*/

	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

class MADSMetaEngine : public AdvancedMetaEngine {
public:
	MADSMetaEngine() : AdvancedMetaEngine(MADS::gameDescriptions, sizeof(MADS::MADSGameDescription), MADSGames, optionsList) {
		_maxScanDepth = 3;
	}

	virtual const char *getName() const {
		return "MADS";
	}

	virtual const char *getOriginalCopyright() const {
		return "MADS (C) Microprose";
	}

	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
	virtual SaveStateList listSaves(const char *target) const;
	virtual int getMaximumSaveSlot() const;
	virtual void removeSaveState(const char *target, int slot) const;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const;
};

bool MADSMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
	    (f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSimpleSavesNames);
}

bool MADS::MADSEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsRTL) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

bool MADSMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const MADS::MADSGameDescription *gd = (const MADS::MADSGameDescription *)desc;
	if (gd) {
		*engine = new MADS::MADSEngine(syst, gd);
	}
	return gd != 0;
}

SaveStateList MADSMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String saveDesc;
	Common::String pattern = Common::String::format("%s.0##", target);
	MADS::MADSSavegameHeader header;

	filenames = saveFileMan->listSavefiles(pattern);

	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		const char *ext = strrchr(file->c_str(), '.');
		int slot = ext ? atoi(ext + 1) : -1;

		if (slot >= 0 && slot < MAX_SAVES) {
			Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(*file);

			if (in) {
				if (MADS::Game::readSavegameHeader(in, header))
					saveList.push_back(SaveStateDescriptor(slot, header._saveName));
				delete in;
			}
		}
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

int MADSMetaEngine::getMaximumSaveSlot() const {
	return MAX_SAVES;
}

void MADSMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String filename = Common::String::format("%s.%03d", target, slot);
	g_system->getSavefileManager()->removeSavefile(filename);
}

SaveStateDescriptor MADSMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String filename = Common::String::format("%s.%03d", target, slot);
	Common::InSaveFile *f = g_system->getSavefileManager()->openForLoading(filename);

	if (f) {
		MADS::MADSSavegameHeader header;
		if (!MADS::Game::readSavegameHeader(f, header, false)) {
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


#if PLUGIN_ENABLED_DYNAMIC(MADS)
	REGISTER_PLUGIN_DYNAMIC(MADS, PLUGIN_TYPE_ENGINE, MADSMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(MADS, PLUGIN_TYPE_ENGINE, MADSMetaEngine);
#endif
