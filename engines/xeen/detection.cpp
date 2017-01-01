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

#include "xeen/xeen.h"
#include "xeen/worldofxeen/worldofxeen.h"

#include "base/plugins.h"
#include "common/savefile.h"
#include "engines/advancedDetector.h"
#include "common/system.h"

#define MAX_SAVES 99

namespace Xeen {

struct XeenGameDescription {
	ADGameDescription desc;

	int gameID;
	uint32 features;
};

uint32 XeenEngine::getGameID() const {
	return _gameDescription->gameID;
}

uint32 XeenEngine::getGameFeatures() const {
	return _gameDescription->features;
}

uint32 XeenEngine::getFeatures() const {
	return _gameDescription->desc.flags;
}

Common::Language XeenEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

Common::Platform XeenEngine::getPlatform() const {
	return _gameDescription->desc.platform;
}

} // End of namespace Xeen

static const PlainGameDescriptor XeenGames[] = {
	{ "xeen", "Xeen" },
	{ "clouds", "Clouds of Xeen" },
	{ "darkside", "Dark Side of Xeen" },
	{ "worldofxeen", "World of Xeen" },
	{0, 0}
};

#include "xeen/detection_tables.h"

class XeenMetaEngine : public AdvancedMetaEngine {
public:
	XeenMetaEngine() : AdvancedMetaEngine(Xeen::gameDescriptions, sizeof(Xeen::XeenGameDescription), XeenGames) {
		_maxScanDepth = 3;
	}

	virtual const char *getName() const {
		return "Xeen Engine";
	}

	virtual const char *getOriginalCopyright() const {
		return "Xeen Engine (c) ???";
	}

	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
	virtual SaveStateList listSaves(const char *target) const;
	virtual int getMaximumSaveSlot() const;
	virtual void removeSaveState(const char *target, int slot) const;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const;
};

bool XeenMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
	    (f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail);
}

bool Xeen::XeenEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsRTL) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

bool XeenMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const Xeen::XeenGameDescription *gd = (const Xeen::XeenGameDescription *)desc;

	switch (gd->gameID) {
	case Xeen::GType_Clouds:
	case Xeen::GType_DarkSide:
	case Xeen::GType_WorldOfXeen:
		*engine = new Xeen::WorldOfXeen::WorldOfXeenEngine(syst, gd);
		break;
	default:
		break;
	}

	return gd != 0;
}

SaveStateList XeenMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String saveDesc;
	Common::String pattern = Common::String::format("%s.0??", target);
	Xeen::XeenSavegameHeader header;

	filenames = saveFileMan->listSavefiles(pattern);
	sort(filenames.begin(), filenames.end());   // Sort to get the files in numerical order

	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		const char *ext = strrchr(file->c_str(), '.');
		int slot = ext ? atoi(ext + 1) : -1;

		if (slot >= 0 && slot < MAX_SAVES) {
			Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(*file);

			if (in) {
				Xeen::XeenEngine::readSavegameHeader(in, header);
				saveList.push_back(SaveStateDescriptor(slot, header._saveName));

				header._thumbnail->free();
				delete header._thumbnail;
				delete in;
			}
		}
	}

	return saveList;
}

int XeenMetaEngine::getMaximumSaveSlot() const {
	return MAX_SAVES;
}

void XeenMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String filename = Common::String::format("%s.%03d", target, slot);
	g_system->getSavefileManager()->removeSavefile(filename);
}

SaveStateDescriptor XeenMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String filename = Common::String::format("%s.%03d", target, slot);
	Common::InSaveFile *f = g_system->getSavefileManager()->openForLoading(filename);

	if (f) {
		Xeen::XeenSavegameHeader header;
		Xeen::XeenEngine::readSavegameHeader(f, header);
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


#if PLUGIN_ENABLED_DYNAMIC(XEEN)
	REGISTER_PLUGIN_DYNAMIC(XEEN, PLUGIN_TYPE_ENGINE, XeenMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(XEEN, PLUGIN_TYPE_ENGINE, XeenMetaEngine);
#endif
