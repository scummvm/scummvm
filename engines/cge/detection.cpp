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

#include "common/config-manager.h"
#include "engines/advancedDetector.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/translation.h"
#include "base/plugins.h"
#include "graphics/thumbnail.h"
#include "cge/cge.h"

namespace CGE {

struct CgeGameDescription {
	ADGameDescription desc;
	GameType gameType;
};

#define GAMEOPTION_COLOR_BLIND_DEFAULT_OFF  GUIO_GAMEOPTIONS1

} // End of namespace CGE

static const PlainGameDescriptor CGEGames[] = {
	{ "soltys", "Soltys" },
	{ "sfinx", "Sfinx" },
	{ 0, 0 }
};

namespace CGE {

static const CgeGameDescription gameDescriptions[] = {

	{
		{
			"soltys", "",
			{
				{"vol.cat", 0, "0c33e2c304821a2444d297fc5e2d67c6", 50176},
				{"vol.dat", 0, "f9ae2e7f8f7cac91378cdafca43faf1e", 8437572},
				AD_LISTEND
			},
			Common::PL_POL, Common::kPlatformDOS, ADGF_NO_FLAGS, GUIO0()
		},
		kGameTypeSoltys
	},
	{
		{
			"soltys", "Soltys Freeware",
			{
				{"vol.cat", 0, "0c33e2c304821a2444d297fc5e2d67c6", 50176},
				{"vol.dat", 0, "f9ae2e7f8f7cac91378cdafca43faf1e", 8437676},
				AD_LISTEND
			},
			Common::PL_POL, Common::kPlatformDOS, ADGF_NO_FLAGS, GUIO1(GAMEOPTION_COLOR_BLIND_DEFAULT_OFF)
		},
		kGameTypeSoltys
	},
	{
		{
			"soltys", "Soltys Demo (not supported)",
			{
				{"vol.cat", 0, "1e077c8ff58109a187f07ac54b0c873a", 18788},
				{"vol.dat", 0, "75d385a6074c58b69f7730481f256051", 1796710},
				AD_LISTEND
			},
			Common::EN_ANY, Common::kPlatformDOS, ADGF_DEMO , GUIO1(GAMEOPTION_COLOR_BLIND_DEFAULT_OFF)
		},
		kGameTypeSoltys
	},
	{
		{
			"soltys", "Soltys Demo (not supported)",
			{
				{"vol.cat", 0, "f17987487fab1ebddd781d8d02fedecc", 7168},
				{"vol.dat", 0, "c5d9b15863cab61dc125551576dece04", 1075272},
				AD_LISTEND
			},
			Common::PL_POL, Common::kPlatformDOS, ADGF_DEMO , GUIO1(GAMEOPTION_COLOR_BLIND_DEFAULT_OFF)
		},
		kGameTypeSoltys
	},
	{
		{
			"soltys", "Soltys Freeware v1.0",
			{
				{"vol.cat", 0, "f1675684c68ab90272f5776f8f2c3974", 50176},
				{"vol.dat", 0, "4ffeff4abc99ac5999b55ccfc56ab1df", 8430868},
				AD_LISTEND
			},
			Common::EN_ANY, Common::kPlatformDOS, ADGF_NO_FLAGS , GUIO1(GAMEOPTION_COLOR_BLIND_DEFAULT_OFF)
		},
		kGameTypeSoltys
	},
	{
		{
			"soltys", "Soltys Freeware v1.0",
			{
				{"vol.cat", 0, "20fdce799adb618100ef9ee2362be875", 50176},
				{"vol.dat", 0, "0e43331c846094d77f5dd201827e0a3b", 8439339},
				AD_LISTEND
			},
			Common::PL_POL, Common::kPlatformDOS, ADGF_NO_FLAGS, GUIO1(GAMEOPTION_COLOR_BLIND_DEFAULT_OFF)
		},
		kGameTypeSoltys
	},
	{
		{
			"soltys", "Soltys Freeware v1.0",
			{
				{"vol.cat", 0, "fcae86b20eaa5cedec17b24fa5e85eb4", 50176},
				{"vol.dat", 0, "ff10d54acc2c95696c57e05819b6906f", 8450151},
				AD_LISTEND
			},
			Common::ES_ESP, Common::kPlatformDOS, ADGF_NO_FLAGS , GUIO1(GAMEOPTION_COLOR_BLIND_DEFAULT_OFF)
		},
		kGameTypeSoltys
	},
	{
		{
			// Polish version, provided by Strangerke
			"sfinx", "Sfinx Freeware",
			{
				{"vol.cat", 0, "21197b287d397c53261b6616bf0dd880", 129024},
				{"vol.dat", 0, "de14291869a8eb7c2732ab783c7542ef", 34180844},
				AD_LISTEND
			},
			Common::PL_POL, Common::kPlatformDOS, ADGF_NO_FLAGS, GUIO1(GAMEOPTION_COLOR_BLIND_DEFAULT_OFF)
		},
		kGameTypeSfinx
	},

	{AD_TABLE_END_MARKER, kGameTypeNone}
};

static const ADFileBasedFallback fileBasedFallback[] = {
	{ &gameDescriptions[0].desc, { "vol.cat", "vol.dat", 0 } },
	{ 0, { 0 } }
};
} // End of namespace CGE

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_COLOR_BLIND_DEFAULT_OFF,
		{
			_s("Color Blind Mode"),
			_s("Enable Color Blind Mode by default"),
			"enable_color_blind",
			false
		}
	},

	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

class CGEMetaEngine : public AdvancedMetaEngine {
public:
	CGEMetaEngine() : AdvancedMetaEngine(CGE::gameDescriptions, sizeof(CGE::CgeGameDescription), CGEGames, optionsList) {
		_singleid = "soltys";
	}

	virtual const ADGameDescription *fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist) const {
		return detectGameFilebased(allFiles, fslist, CGE::fileBasedFallback);
	}

	virtual const char *getName() const {
		return "CGE";
	}

	virtual const char *getOriginalCopyright() const {
		return "Soltys (c) 1994-1996 L.K. Avalon";
	}

	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
	virtual int getMaximumSaveSlot() const;
	virtual SaveStateList listSaves(const char *target) const;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const;
	virtual void removeSaveState(const char *target, int slot) const;
};

bool CGEMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
	    (f == kSupportsListSaves) ||
	    (f == kSupportsLoadingDuringStartup) ||
	    (f == kSupportsDeleteSave) ||
	    (f == kSavesSupportMetaInfo) ||
	    (f == kSavesSupportThumbnail) ||
	    (f == kSavesSupportCreationDate);
}

void CGEMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String fileName = Common::String::format("%s.%03d", target, slot);
	g_system->getSavefileManager()->removeSavefile(fileName);
}

int CGEMetaEngine::getMaximumSaveSlot() const {
	return 99;
}

SaveStateList CGEMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String pattern = target;
	pattern += ".???";

	filenames = saveFileMan->listSavefiles(pattern);
	sort(filenames.begin(), filenames.end());   // Sort (hopefully ensuring we are sorted numerically..)

	SaveStateList saveList;
	for (Common::StringArray::const_iterator filename = filenames.begin(); filename != filenames.end(); ++filename) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		int slotNum = atoi(filename->c_str() + filename->size() - 3);

		if (slotNum >= 0 && slotNum <= 99) {

			Common::InSaveFile *file = saveFileMan->openForLoading(*filename);
			if (file) {
				CGE::SavegameHeader header;

				// Check to see if it's a ScummVM savegame or not
				char buffer[kSavegameStrSize + 1];
				file->read(buffer, kSavegameStrSize + 1);

				if (!strncmp(buffer, CGE::savegameStr, kSavegameStrSize + 1)) {
					// Valid savegame
					if (CGE::CGEEngine::readSavegameHeader(file, header)) {
						saveList.push_back(SaveStateDescriptor(slotNum, header.saveName));
						if (header.thumbnail) {
							header.thumbnail->free();
							delete header.thumbnail;
						}
					}
				} else {
					// Must be an original format savegame
					saveList.push_back(SaveStateDescriptor(slotNum, "Unknown"));
				}

				delete file;
			}
		}
	}

	return saveList;
}

SaveStateDescriptor CGEMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String fileName = Common::String::format("%s.%03d", target, slot);
	Common::InSaveFile *f = g_system->getSavefileManager()->openForLoading(fileName);

	if (f) {
		CGE::SavegameHeader header;

		// Check to see if it's a ScummVM savegame or not
		char buffer[kSavegameStrSize + 1];
		f->read(buffer, kSavegameStrSize + 1);

		bool hasHeader = !strncmp(buffer, CGE::savegameStr, kSavegameStrSize + 1) &&
			CGE::CGEEngine::readSavegameHeader(f, header);
		delete f;

		if (!hasHeader) {
			// Original savegame perhaps?
			SaveStateDescriptor desc(slot, "Unknown");
			return desc;
		} else {
			// Create the return descriptor
			SaveStateDescriptor desc(slot, header.saveName);
			desc.setThumbnail(header.thumbnail);
			desc.setSaveDate(header.saveYear, header.saveMonth, header.saveDay);
			desc.setSaveTime(header.saveHour, header.saveMinutes);

			// Slot 0 is used for the 'automatic save on exit' save in Soltys, thus
			// we prevent it from being deleted or overwritten by accident.
			desc.setDeletableFlag(slot != 0);
			desc.setWriteProtectedFlag(slot == 0);

			return desc;
		}
	}

	return SaveStateDescriptor();
}

bool CGEMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	if (desc) {
		*engine = new CGE::CGEEngine(syst, desc);
	}
	return desc != 0;
}

#if PLUGIN_ENABLED_DYNAMIC(CGE)
	REGISTER_PLUGIN_DYNAMIC(CGE, PLUGIN_TYPE_ENGINE, CGEMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(CGE, PLUGIN_TYPE_ENGINE, CGEMetaEngine);
#endif
