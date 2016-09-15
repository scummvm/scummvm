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

#include "gnap/gnap.h"

#include "common/config-manager.h"
#include "engines/advancedDetector.h"
#include "common/savefile.h"
#include "common/system.h"
#include "base/plugins.h"
#include "graphics/thumbnail.h"

static const PlainGameDescriptor gnapGames[] = {
	{ "gnap", "Gnap" },
	{ 0, 0 }
};

namespace Gnap {

static const ADGameDescription gameDescriptions[] = {
	{
		"gnap", "",
		{
			{"stock_n.dat", 0, "46819043d019a2f36b727cc2bdd6980f", 12515823},
			AD_LISTEND
		},
		Common::EN_ANY, Common::kPlatformWindows, ADGF_NO_FLAGS, GUIO0()
	},
	{
		"gnap", "",
		{
			{"stock_n.dat", 0, "46819043d019a2f36b727cc2bdd6980f", 13497301},
			AD_LISTEND
		},
		Common::EN_ANY, Common::kPlatformWindows, ADGF_NO_FLAGS, GUIO0()
	},
	{
		"gnap", "",
		{
			{"stock_n.dat", 0, "46819043d019a2f36b727cc2bdd6980f", 12995485},
			AD_LISTEND
		},
		Common::RU_RUS, Common::kPlatformWindows, ADGF_NO_FLAGS, GUIO0()
	},
	{
		"gnap", "Fargus",
		{
			{"stock_n.dat", 0, "46819043d019a2f36b727cc2bdd6980f", 12847726},
			AD_LISTEND
		},
		Common::RU_RUS, Common::kPlatformWindows, ADGF_NO_FLAGS, GUIO0()
	},

	AD_TABLE_END_MARKER
};

} // End of namespace Gnap

class GnapMetaEngine : public AdvancedMetaEngine {
public:
	GnapMetaEngine() : AdvancedMetaEngine(Gnap::gameDescriptions, sizeof(ADGameDescription), gnapGames) {
		_singleId = "gnap";
		_maxScanDepth = 3;
	}

	virtual const char *getEngineId() const {
		return "gnap";
	}

	virtual const char *getName() const {
		return "Gnap";
	}

	virtual const char *getOriginalCopyright() const {
		return "Gnap (C) Artech Digital Entertainment 1997";
	}

	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
	virtual int getMaximumSaveSlot() const;
	virtual SaveStateList listSaves(const char *target) const;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const;
	virtual void removeSaveState(const char *target, int slot) const;
};

bool GnapMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSavesSupportCreationDate) ||
		(f == kSimpleSavesNames);
}

bool Gnap::GnapEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsRTL) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

void GnapMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String fileName = Common::String::format("%s.%03d", target, slot);
	g_system->getSavefileManager()->removeSavefile(fileName);
}

int GnapMetaEngine::getMaximumSaveSlot() const { return 99; }

SaveStateList GnapMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String saveDesc;
	Common::String pattern = Common::String::format("%s.0##", target);
	Gnap::GnapSavegameHeader header;

	filenames = saveFileMan->listSavefiles(pattern);

	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		const char *ext = strrchr(file->c_str(), '.');
		int slot = ext ? atoi(ext + 1) : -1;

		if (slot >= 0 && slot < getMaximumSaveSlot()) {
			Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(*file);

			if (in) {
				if (Gnap::GnapEngine::readSavegameHeader(in, header))
					saveList.push_back(SaveStateDescriptor(slot, header._saveName));
				delete in;
			}
		}
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

SaveStateDescriptor GnapMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String fileName = Common::String::format("%s.%03d", target, slot);
	Common::InSaveFile *file = g_system->getSavefileManager()->openForLoading(fileName);
	if (file) {
		char saveIdentBuffer[5];
		file->read(saveIdentBuffer, 5);

		int32 version = file->readByte();
		if (version > GNAP_SAVEGAME_VERSION) {
			delete file;
			return SaveStateDescriptor();
		}

		Common::String saveName;
		char ch;
		while ((ch = (char)file->readByte()) != '\0')
			saveName += ch;

		SaveStateDescriptor desc(slot, saveName);

		if (version != 1) {
			Graphics::Surface *thumbnail;
			if (!Graphics::loadThumbnail(*file, thumbnail)) {
				delete file;
				return SaveStateDescriptor();
			}
			desc.setThumbnail(thumbnail);
		}

		int year = file->readSint16LE();
		int month = file->readSint16LE();
		int day = file->readSint16LE();
		int hour = file->readSint16LE();
		int minutes = file->readSint16LE();

		desc.setSaveDate(year, month, day);
		desc.setSaveTime(hour, minutes);

		delete file;
		return desc;
	}

	return SaveStateDescriptor();
}

bool GnapMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	if (desc) {
		*engine = new Gnap::GnapEngine(syst, desc);
	}
	return desc != 0;
}

#if PLUGIN_ENABLED_DYNAMIC(GNAP)
	REGISTER_PLUGIN_DYNAMIC(GNAP, PLUGIN_TYPE_ENGINE, GnapMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(GNAP, PLUGIN_TYPE_ENGINE, GnapMetaEngine);
#endif
