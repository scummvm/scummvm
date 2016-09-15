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

/*
 * This code is based on original Sfinx source code
 * Copyright (c) 1994-1997 Janus B. Wisniewski and L.K. Avalon
 */

#include "cge2/cge2.h"
#include "cge2/fileio.h"
#include "engines/advancedDetector.h"
#include "common/translation.h"
#include "graphics/surface.h"

namespace CGE2 {

#define GAMEOPTION_COLOR_BLIND_DEFAULT_OFF  GUIO_GAMEOPTIONS1

static const PlainGameDescriptor CGE2Games[] = {
		{ "sfinx", "Sfinx" },
		{ 0, 0 }
};

static const ADGameDescription gameDescriptions[] = {
		{
			"sfinx", "Freeware",
			{
				{ "vol.cat", 0, "21197b287d397c53261b6616bf0dd880", 129024 },
				{ "vol.dat", 0, "de14291869a8eb7c2732ab783c7542ef", 34180844 },
				AD_LISTEND
			},
			Common::PL_POL, Common::kPlatformDOS, ADGF_NO_FLAGS, GUIO1(GAMEOPTION_COLOR_BLIND_DEFAULT_OFF)
		},

		{
			"sfinx", "Freeware v1.0",
			{
				{"vol.cat", 0, "aa402aed24a72c53a4d1211c456b79dd", 129024},
				{"vol.dat", 0, "5966ac26d91d664714349669f9dd09b5", 34180164},
				AD_LISTEND
			},
			Common::PL_POL, Common::kPlatformDOS, ADGF_NO_FLAGS, GUIO1(GAMEOPTION_COLOR_BLIND_DEFAULT_OFF)
		},

		{
			"sfinx", "Freeware v1.1",
			{
				{"vol.cat", 0, "aa402aed24a72c53a4d1211c456b79dd", 129024},
				{"vol.dat", 0, "5966ac26d91d664714349669f9dd09b5", 34180367},
				AD_LISTEND
			},
			Common::PL_POL, Common::kPlatformDOS, ADGF_NO_FLAGS, GUIO1(GAMEOPTION_COLOR_BLIND_DEFAULT_OFF)
		},

		{
			"sfinx", "Freeware v0.3",
			{
				{"vol.cat", 0, "f158e469dccbebc5a632eb848df89779", 129024},
				{"vol.dat", 0, "d40a6b4ae173d6930be54ba56bee15d5", 34183430},
				AD_LISTEND
			},
			Common::EN_ANY, Common::kPlatformDOS, ADGF_NO_FLAGS, GUIO1(GAMEOPTION_COLOR_BLIND_DEFAULT_OFF)
		},

		{
			"sfinx", "Freeware v1.0",
			{
				{"vol.cat", 0, "f158e469dccbebc5a632eb848df89779", 129024},
				{"vol.dat", 0, "d40a6b4ae173d6930be54ba56bee15d5", 34183443},
				AD_LISTEND
			},
			Common::EN_ANY, Common::kPlatformDOS, ADGF_NO_FLAGS, GUIO1(GAMEOPTION_COLOR_BLIND_DEFAULT_OFF)
		},

		{
			"sfinx", "Freeware v1.1",
			{
				{"vol.cat", 0, "f158e469dccbebc5a632eb848df89779", 129024},
				{"vol.dat", 0, "d40a6b4ae173d6930be54ba56bee15d5", 34182773},
				AD_LISTEND
			},
			Common::EN_ANY, Common::kPlatformDOS, ADGF_NO_FLAGS, GUIO1(GAMEOPTION_COLOR_BLIND_DEFAULT_OFF)
		},

		AD_TABLE_END_MARKER
};

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

class CGE2MetaEngine : public AdvancedMetaEngine {
public:
	CGE2MetaEngine() : AdvancedMetaEngine(gameDescriptions, sizeof(ADGameDescription), CGE2Games, optionsList) {
		_singleId = "sfinx";
	}

	const char *getEngineId() const {
		return "cge2";
	}

	virtual const char *getName() const {
		return "CGE2";
	}

	virtual const char *getOriginalCopyright() const {
		return "Sfinx (C) 1994-1997 Janus B. Wisniewski and L.K. Avalon";
	}

	ADDetectedGame fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist) const override;
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual int getMaximumSaveSlot() const;
	virtual SaveStateList listSaves(const char *target) const;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const;
	virtual void removeSaveState(const char *target, int slot) const;
};

static ADGameDescription s_fallbackDesc = {
	"sfinx",
	"Unknown version",
	AD_ENTRY1(0, 0), // This should always be AD_ENTRY1(0, 0) in the fallback descriptor
	Common::UNK_LANG,
	Common::kPlatformDOS,
	ADGF_NO_FLAGS,
	GUIO1(GAMEOPTION_COLOR_BLIND_DEFAULT_OFF)
};

static const ADFileBasedFallback fileBasedFallback[] = {
	{ &s_fallbackDesc, { "vol.cat", "vol.dat", 0 } },
	{ 0, { 0 } }
};

// This fallback detection looks identical to the one used for CGE. In fact, the difference resides
// in the ResourceManager which handles a different archive format. The rest of the detection is identical.
ADDetectedGame CGE2MetaEngine::fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist) const {
	ADDetectedGame game = detectGameFilebased(allFiles, fslist, CGE2::fileBasedFallback);

	if (!game.desc)
		return ADDetectedGame();

	SearchMan.addDirectory("CGE2MetaEngine::fallbackDetect", fslist.begin()->getParent());
	ResourceManager *resman;
	resman = new ResourceManager();
	bool sayFileFound = resman->exist("CGE.SAY");
	delete resman;

	SearchMan.remove("CGE2MetaEngine::fallbackDetect");

	if (!sayFileFound)
		return ADDetectedGame();

	return game;
}

bool CGE2MetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	if (desc)
		*engine = new CGE2::CGE2Engine(syst, desc);

	return desc != 0;
}

bool CGE2MetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSavesSupportCreationDate) ||
		(f == kSavesSupportPlayTime) ||
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSimpleSavesNames);
}

int CGE2MetaEngine::getMaximumSaveSlot() const {
	return 99;
}

SaveStateList CGE2MetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String pattern = target;
	pattern += ".###";

	filenames = saveFileMan->listSavefiles(pattern);

	SaveStateList saveList;
	for (Common::StringArray::const_iterator filename = filenames.begin(); filename != filenames.end(); ++filename) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		int slotNum = atoi(filename->c_str() + filename->size() - 3);

		if (slotNum >= 0 && slotNum <= 99) {

			Common::InSaveFile *file = saveFileMan->openForLoading(*filename);
			if (file) {
				CGE2::SavegameHeader header;

				// Check to see if it's a ScummVM savegame or not
				char buffer[kSavegameStrSize + 1];
				file->read(buffer, kSavegameStrSize + 1);

				if (!strncmp(buffer, kSavegameStr, kSavegameStrSize + 1)) {
					// Valid savegame
					if (CGE2::CGE2Engine::readSavegameHeader(file, header)) {
						saveList.push_back(SaveStateDescriptor(slotNum, header.saveName));
					}
				} else {
					// Must be an original format savegame
					saveList.push_back(SaveStateDescriptor(slotNum, "Unknown"));
				}

				delete file;
			}
		}
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

SaveStateDescriptor CGE2MetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String fileName = Common::String::format("%s.%03d", target, slot);
	Common::InSaveFile *f = g_system->getSavefileManager()->openForLoading(fileName);

	if (f) {
		CGE2::SavegameHeader header;

		// Check to see if it's a ScummVM savegame or not
		char buffer[kSavegameStrSize + 1];
		f->read(buffer, kSavegameStrSize + 1);

		bool hasHeader = !strncmp(buffer, kSavegameStr, kSavegameStrSize + 1) &&
			CGE2::CGE2Engine::readSavegameHeader(f, header, false);
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

			if (header.playTime) {
				desc.setPlayTime(header.playTime * 1000);
			}

			// Slot 0 is used for the 'automatic save on exit' save in Soltys, thus
			// we prevent it from being deleted or overwritten by accident.
			desc.setDeletableFlag(slot != 0);
			desc.setWriteProtectedFlag(slot == 0);

			return desc;
		}
	}

	return SaveStateDescriptor();
}

void CGE2MetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String fileName = Common::String::format("%s.%03d", target, slot);
	g_system->getSavefileManager()->removeSavefile(fileName);
}

} // End of namespace CGE2

#if PLUGIN_ENABLED_DYNAMIC(CGE2)
	REGISTER_PLUGIN_DYNAMIC(CGE2, PLUGIN_TYPE_ENGINE, CGE2::CGE2MetaEngine);
#else
	REGISTER_PLUGIN_STATIC(CGE2, PLUGIN_TYPE_ENGINE, CGE2::CGE2MetaEngine);
#endif
