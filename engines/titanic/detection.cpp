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

#include "titanic/core/project_item.h"
#include "titanic/events.h"
#include "titanic/support/simple_file.h"
#include "titanic/titanic.h"
#include "base/plugins.h"
#include "common/savefile.h"
#include "common/str-array.h"
#include "common/memstream.h"
#include "engines/advancedDetector.h"
#include "common/system.h"
#include "graphics/colormasks.h"
#include "graphics/surface.h"

namespace Titanic {

struct TitanicGameDescription {
	ADGameDescription desc;
};

uint32 TitanicEngine::getFeatures() const {
	return _gameDescription->desc.flags;
}

Common::Language TitanicEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

} // End of namespace Titanic

static const PlainGameDescriptor TitanicGames[] = {
	{"titanic", "Starship Titanic"},
	{0, 0}
};

#include "titanic/detection_tables.h"

class TitanicMetaEngine : public AdvancedMetaEngine {
public:
	TitanicMetaEngine() : AdvancedMetaEngine(Titanic::gameDescriptions, sizeof(Titanic::TitanicGameDescription), TitanicGames) {
		_maxScanDepth = 3;
	}

	virtual const char *getName() const {
		return "Starship Titanic";
	}

	virtual const char *getOriginalCopyright() const {
		return "Starship Titanic (C) The Digital Village";
	}

	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
	virtual SaveStateList listSaves(const char *target) const;
	virtual int getMaximumSaveSlot() const;
	virtual void removeSaveState(const char *target, int slot) const;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const;
};

bool TitanicMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportCreationDate) ||
		(f == kSavesSupportPlayTime) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSimpleSavesNames);
}

bool Titanic::TitanicEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsRTL) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

bool TitanicMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const Titanic::TitanicGameDescription *gd = (const Titanic::TitanicGameDescription *)desc;
	*engine = new Titanic::TitanicEngine(syst, gd);

	return gd != 0;
}

SaveStateList TitanicMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String saveDesc;
	Common::String pattern = Common::String::format("%s.0##", target);
	Titanic::TitanicSavegameHeader header;

	filenames = saveFileMan->listSavefiles(pattern);

	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		const char *ext = strrchr(file->c_str(), '.');
		int slot = ext ? atoi(ext + 1) : -1;

		if (slot >= 0 && slot <= MAX_SAVES) {
			Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(*file);

			if (in) {
				Titanic::CompressedFile cf;
				cf.open(in);

				if (Titanic::CProjectItem::readSavegameHeader(&cf, header))
					saveList.push_back(SaveStateDescriptor(slot, header._saveName));

				cf.close();
			}
		}
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

int TitanicMetaEngine::getMaximumSaveSlot() const {
	return MAX_SAVES;
}

void TitanicMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String filename = Common::String::format("%s.%03d", target, slot);
	g_system->getSavefileManager()->removeSavefile(filename);
}

SaveStateDescriptor TitanicMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String filename = Common::String::format("%s.%03d", target, slot);
	Common::InSaveFile *f = g_system->getSavefileManager()->openForLoading(filename);

	if (f) {
		Titanic::CompressedFile file;
		file.open(f);

		Titanic::TitanicSavegameHeader header;
		if (!Titanic::CProjectItem::readSavegameHeader(&file, header, false)) {
			file.close();
			return SaveStateDescriptor();
		}

		file.close();

		// Create the return descriptor
		SaveStateDescriptor desc(slot, header._saveName);

		if (header._version) {
			desc.setThumbnail(header._thumbnail);
			desc.setSaveDate(header._year, header._month, header._day);
			desc.setSaveTime(header._hour, header._minute);
			desc.setPlayTime(header._totalFrames * GAME_FRAME_TIME);
		}

		return desc;
	}

	return SaveStateDescriptor();
}


#if PLUGIN_ENABLED_DYNAMIC(TITANIC)
	REGISTER_PLUGIN_DYNAMIC(TITANIC, PLUGIN_TYPE_ENGINE, TitanicMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(TITANIC, PLUGIN_TYPE_ENGINE, TitanicMetaEngine);
#endif
