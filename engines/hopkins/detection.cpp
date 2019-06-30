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

#include "hopkins/hopkins.h"

#include "base/plugins.h"
#include "common/savefile.h"
#include "common/str-array.h"
#include "common/memstream.h"
#include "engines/advancedDetector.h"
#include "common/system.h"
#include "common/translation.h"
#include "graphics/colormasks.h"
#include "graphics/surface.h"

#define MAX_SAVES 99

namespace Hopkins {

struct HopkinsGameDescription {
	ADGameDescription desc;
};

uint32 HopkinsEngine::getFeatures() const {
	return _gameDescription->desc.flags;
}

Common::Language HopkinsEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

Common::Platform HopkinsEngine::getPlatform() const {
	return _gameDescription->desc.platform;
}

bool HopkinsEngine::getIsDemo() const {
	return _gameDescription->desc.flags & ADGF_DEMO;
}

const Common::String &HopkinsEngine::getTargetName() const {
	return _targetName;
}

} // End of namespace Hopkins

static const PlainGameDescriptor hopkinsGames[] = {
	{"hopkins", "Hopkins FBI"},
	{0, 0}
};

#include "hopkins/detection_tables.h"

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_GORE_DEFAULT_OFF,
		{
			_s("Gore Mode"),
			_s("Enable Gore Mode when available"),
			"enable_gore",
			false
		}
	},

	{
		GAMEOPTION_GORE_DEFAULT_ON,
		{
			_s("Gore Mode"),
			_s("Enable Gore Mode when available"),
			"enable_gore",
			true
		}
	},

	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

const static char *directoryGlobs[] = {
	"voice",
	"link",
	0
};

class HopkinsMetaEngine : public AdvancedMetaEngine {
public:
	HopkinsMetaEngine() : AdvancedMetaEngine(Hopkins::gameDescriptions, sizeof(Hopkins::HopkinsGameDescription), hopkinsGames, optionsList) {
		_maxScanDepth = 3;
		_directoryGlobs = directoryGlobs;
	}

	virtual const char *getName() const {
		return "Hopkins FBI";
	}

	virtual const char *getOriginalCopyright() const {
		return "Hopkins FBI (C) 1997-2003 MP Entertainment";
	}

	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
	virtual SaveStateList listSaves(const char *target) const;
	virtual int getMaximumSaveSlot() const;
	virtual void removeSaveState(const char *target, int slot) const;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const;
};

bool HopkinsMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
	    (f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSimpleSavesNames);
}

bool Hopkins::HopkinsEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsRTL) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

bool HopkinsMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const Hopkins::HopkinsGameDescription *gd = (const Hopkins::HopkinsGameDescription *)desc;
	if (gd) {
		*engine = new Hopkins::HopkinsEngine(syst, gd);
	}
	return gd != 0;
}

SaveStateList HopkinsMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String saveDesc;
	Common::String pattern = Common::String::format("%s.0##", target);

	filenames = saveFileMan->listSavefiles(pattern);

	Hopkins::hopkinsSavegameHeader header;

	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		const char *ext = strrchr(file->c_str(), '.');
		int slot = ext ? atoi(ext + 1) : -1;

		if (slot >= 0 && slot < MAX_SAVES) {
			Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(*file);

			if (in) {
				if (Hopkins::SaveLoadManager::readSavegameHeader(in, header)) {
					saveList.push_back(SaveStateDescriptor(slot, header._saveName));
				}

				delete in;
			}
		}
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

int HopkinsMetaEngine::getMaximumSaveSlot() const {
	return MAX_SAVES;
}

void HopkinsMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String filename = Common::String::format("%s.%03d", target, slot);
	g_system->getSavefileManager()->removeSavefile(filename);
}

SaveStateDescriptor HopkinsMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String filename = Common::String::format("%s.%03d", target, slot);
	Common::InSaveFile *f = g_system->getSavefileManager()->openForLoading(filename);

	if (f) {
		Hopkins::hopkinsSavegameHeader header;
		if (!Hopkins::SaveLoadManager::readSavegameHeader(f, header, false)) {
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


#if PLUGIN_ENABLED_DYNAMIC(HOPKINS)
	REGISTER_PLUGIN_DYNAMIC(HOPKINS, PLUGIN_TYPE_ENGINE, HopkinsMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(HOPKINS, PLUGIN_TYPE_ENGINE, HopkinsMetaEngine);
#endif
