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
* Based on the Reverse Engineering work of Christophe Fontanel,
* maintainer of the Dungeon Master Encyclopaedia (http://dmweb.free.fr/)
*/


#include "common/config-manager.h"
#include "common/error.h"
#include "common/fs.h"
#include "common/system.h"

#include "engines/advancedDetector.h"

#include "dm/dm.h"

namespace DM {
static const PlainGameDescriptor DMGames[] = {
	{"dm", "Dungeon Master"},
	{0, 0}
};

static const DMADGameDescription gameDescriptions[] = {
	{
		{"dm", "Amiga v2.0 English",
			{
				{"graphics.dat", 0, "c2205f6225bde728417de29394f97d55", 411960},
				{"Dungeon.dat", 0, "43a213da8eda413541dd12f90ce202f6", 25006},
				AD_LISTEND
			},
			Common::EN_ANY, Common::kPlatformAmiga, ADGF_NO_FLAGS, GUIO1(GUIO_NONE)
		},
	    kDMSaveTargetDM21, kDMSaveFormatAmigaPC98FmTowns, kDMSavePlatformAmiga,
		{ kDMSaveTargetDM21, kDMSaveTargetEndOfList },
		{ kDMSaveFormatAmigaPC98FmTowns, kDMSaveFormatEndOfList},
		{ kDMSavePlatformAcceptAny}
	},
	{
		{"dm", "Atari v??? English",
			{
				{"graphics.dat", 0, "6ffff2a17e2df0effa9a12fb4b1bf6b6", 271911},
				{"Dungeon.dat", 0, "be9468b460515741babec9a70501e2e9", 33286},
				AD_LISTEND
			},
	    	Common::EN_ANY, Common::kPlatformAtariST, ADGF_NO_FLAGS, GUIO1(GUIO_NONE),
	    },
	    kDMSaveTargetDM21, kDMSaveFormatAmigaPC98FmTowns, kDMSavePlatformAtariSt,
	    { kDMSaveTargetDM21, kDMSaveTargetEndOfList},
	    { kDMSaveFormatAmigaPC98FmTowns, kDMSaveFormatEndOfList},
	    { kDMSavePlatformAcceptAny }
	},
	{
		// Added by Strangerke
		{"dm", "Amiga Demo v2.0 English",
			{
				{"graphics.dat", 0, "3932c8359bb36c24291b09e915114d38", 192421},
				{"DemoDun.dat", 0, "78848e1a2d3d5a11e5954deb8c7b772b", 1209},
				AD_LISTEND
			},
			Common::EN_ANY, Common::kPlatformAmiga, ADGF_DEMO, GUIO1(GUIO_NONE),
		},
		kDMSaveTargetDM21, kDMSaveFormatAmigaPC98FmTowns, kDMSavePlatformAtariSt,
		{ kDMSaveTargetDM21, kDMSaveTargetEndOfList},
		{ kDMSaveFormatAmigaPC98FmTowns, kDMSaveFormatEndOfList},
		{ kDMSavePlatformAcceptAny }
	},
	{
		AD_TABLE_END_MARKER, kDMSaveTargetNone, kDMSaveFormatNone, kDMSavePlatformNone,
		{kDMSaveTargetNone}, {kDMSaveFormatNone}, {kDMSavePlatformNone}
	}
};

static const ADExtraGuiOptionsMap optionsList[] = {
	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

class DMMetaEngine : public AdvancedMetaEngine {
public:
	DMMetaEngine() : AdvancedMetaEngine(DM::gameDescriptions, sizeof(DMADGameDescription), DMGames, optionsList) {
		_singleId = "dm";
	}

	virtual const char *getName() const {
		return "Dungeon Master";
	}

	virtual const char *getOriginalCopyright() const {
		return "Dungeon Master (C) 1987 FTL Games";
	}

	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
		if (desc)
			*engine = new DM::DMEngine(syst, (const DMADGameDescription*)desc);
		return desc != nullptr;
	}

	virtual bool hasFeature(MetaEngineFeature f) const {
		return
			(f == kSupportsListSaves) ||
			(f == kSupportsLoadingDuringStartup) ||
			(f == kSavesSupportThumbnail) ||
			(f == kSavesSupportMetaInfo) ||
			(f == kSavesSupportCreationDate);
	}

	virtual int getMaximumSaveSlot() const { return 99; }

	virtual SaveStateList listSaves(const char *target) const {
		Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
		SaveGameHeader header;
		Common::String pattern = target;
		pattern += ".###";

		Common::StringArray filenames;
		filenames = saveFileMan->listSavefiles(pattern.c_str());

		SaveStateList saveList;

		for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
			// Obtain the last 3 digits of the filename, since they correspond to the save slot
			int slotNum = atoi(file->c_str() + file->size() - 3);

			if ((slotNum >= 0) && (slotNum <= 999)) {
				Common::InSaveFile *in = saveFileMan->openForLoading(file->c_str());
				if (in) {
					if (DM::readSaveGameHeader(in, &header))
						saveList.push_back(SaveStateDescriptor(slotNum, header._descr.getDescription()));
					delete in;
				}
			}
		}

		// Sort saves based on slot number.
		Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
		return saveList;
	}

	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const {
		Common::String filename = Common::String::format("%s.%03u", target, slot);
		Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(filename.c_str());

		if (in) {
			DM::SaveGameHeader header;

			bool successfulRead = DM::readSaveGameHeader(in, &header);
			delete in;

			if (successfulRead) {
				SaveStateDescriptor desc(slot, header._descr.getDescription());

				return header._descr;
			}
		}

		return SaveStateDescriptor();
	}

	virtual void removeSaveState(const char *target, int slot) const {}
};

}
#if PLUGIN_ENABLED_DYNAMIC(DM)
REGISTER_PLUGIN_DYNAMIC(DM, PLUGIN_TYPE_ENGINE, DM::DMMetaEngine);
#else
REGISTER_PLUGIN_STATIC(DM, PLUGIN_TYPE_ENGINE, DM::DMMetaEngine);
#endif
