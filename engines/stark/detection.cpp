/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/advancedDetector.h"
#include "engines/stark/savemetadata.h"
#include "engines/stark/stark.h"
#include "engines/stark/services/stateprovider.h"

#include "common/savefile.h"
#include "common/system.h"

namespace Stark {

static const PlainGameDescriptor starkGames[] = {
	{"stark", "Stark Game"},
	{"tlj", "The Longest Journey"},
	{0, 0}
};

static const ADGameDescription gameDescriptions[] = {
	// The Longest Journey
	// English DVD
	{
		"tlj", "DVD", {
			{"x.xarc", 0, "de8327850d7bba90b690b141eaa23f61", 3032},
			{"chapters.ini", 0, "5b5a1f1dd2297d9ce0d3d12216d5d2c5", 485},
			{NULL, 0, NULL, 0}
		},
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO_NONE
	},

	// The Longest Journey
	// GOG edition
	{
		"tlj", "GOG", {
			{"x.xarc", 0, "a0559457126caadab0cadac02d35f26f", 3032},
			{"chapters.ini", 0, "5b5a1f1dd2297d9ce0d3d12216d5d2c5", 485},
			{NULL, 0, NULL, 0}
		},
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO_NONE
	},

	// The Longest Journey
	// English Old Demo
	{
		"tlj", "Old Demo", {
			{"x.xarc", 0, "97abc1bb9239dee4c208e533f3c97e1c", 98},
			{"chapters.ini", 0, "5b5a1f1dd2297d9ce0d3d12216d5d2c5", 485},
			{NULL, 0, NULL, 0}
		},
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_DEMO | ADGF_UNSTABLE,
		GUIO_NONE
	},

	// The Longest Journey
	// English v1.61 Demo
	{
		"tlj", "v1.61 Demo", {
			{"x.xarc", 0, "61093bcd499b386ed5c0345c52f48909", 98},
			{"chapters.ini", 0, "5b5a1f1dd2297d9ce0d3d12216d5d2c5", 485},
			{NULL, 0, NULL, 0}
		},
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_DEMO | ADGF_UNSTABLE,
		GUIO_NONE
	},

	// The Longest Journey
	// French Demo
	{
		"tlj", "Demo", {
			{"x.xarc", 0, "97abc1bb9239dee4c208e533f3c97e1c", 98},
			{"chapters.ini", 0, "e54f6370dca06496069790840409cf95", 506},
			{NULL, 0, NULL, 0}
		},
		Common::FR_FRA,
		Common::kPlatformWindows,
		ADGF_DEMO | ADGF_UNSTABLE,
		GUIO_NONE
	},

	// The Longest Journey
	// Norwegian Demo
	{
		"tlj", "Demo", {
			{"x.xarc", 0, "97abc1bb9239dee4c208e533f3c97e1c", 98},
			{"chapters.ini", 0, "f358f604abd1aa1476ed05d6d271ac70", 473},
			{NULL, 0, NULL, 0}
		},
		Common::NB_NOR,
		Common::kPlatformWindows,
		ADGF_DEMO | ADGF_UNSTABLE,
		GUIO_NONE
	},

	// The Longest Journey
	// Norwegian 4 CD version - supplied by L0ngcat
	{
		"tlj", "4 CD", {
			{"x.xarc", 0, "a0559457126caadab0cadac02d35f26f", 3032},
			{"chapters.ini", 0, "f358f604abd1aa1476ed05d6d271ac70", 473},
			{NULL, 0, NULL, 0}
		},
		Common::NB_NOR,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO_NONE
	},

	// The Longest Journey
	// Norwegian DLC-edition (DVD?)
	{
		"tlj", "DVD", {
			{"x.xarc", 0, "de8327850d7bba90b690b141eaa23f61", 3032},
			{"chapters.ini", 0, "f358f604abd1aa1476ed05d6d271ac70", 473},
			{NULL, 0, NULL, 0}
		},
		Common::NB_NOR,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO_NONE
	},

	// The Longest Journey
	// German DVD version supplied by Vorph on the forums
	{
		"tlj", "DVD", {
			{"x.xarc", 0, "de8327850d7bba90b690b141eaa23f61", 3032},
			{"chapters.ini", 0, "e4611d143a87b263d8d7a54edc7e7cd7", 515},
			{NULL, 0, NULL, 0}
		},
		Common::DE_DEU,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO_NONE
	},

	// The Longest Journey
	// German 4CD
	{
		"tlj", "4 CD", {
			{"x.xarc", 0, "a0559457126caadab0cadac02d35f26f", 3032},
			{"chapters.ini", 0, "e4611d143a87b263d8d7a54edc7e7cd7", 515},
			{NULL, 0, NULL, 0}
		},
		Common::DE_DEU,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO_NONE
	},

	// The Longest Journey
	// Spanish 4CD
	{
		"tlj", "4 CD", {
			{"x.xarc", 0, "a0559457126caadab0cadac02d35f26f", 3032},
			{"chapters.ini", 0, "3640df6d536b186bff228337284d9631", 525},
			{NULL, 0, NULL, 0}
		},
		Common::ES_ESP,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO_NONE
	},

	// The Longest Journey
	// French 2CD
	{
		"tlj", "2 CD", {
			{"x.xarc", 0, "de8327850d7bba90b690b141eaa23f61", 3032},
			{"chapters.ini", 0, "e54f6370dca06496069790840409cf95", 506},
			{NULL, 0, NULL, 0}
		},
		Common::FR_FRA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO_NONE
	},

	// The Longest Journey
	// French 4CD
	{
		"tlj", "4 CD", {
			{"x.xarc", 0, "a0559457126caadab0cadac02d35f26f", 3032},
			{"chapters.ini", 0, "e54f6370dca06496069790840409cf95", 506},
			{NULL, 0, NULL, 0}
		},
		Common::FR_FRA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO_NONE
	},

	// The Longest Journey
	// Swedish Demo
	{
		"tlj", "Demo", {
			{"x.xarc", 0, "97abc1bb9239dee4c208e533f3c97e1c", 98},
			{"chapters.ini", 0, "f6a2007300209492b7b90b4c0467832d", 462},
			{NULL, 0, NULL, 0}
		},
		Common::SE_SWE,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO_NONE
	},

	// The Longest Journey
	// Swedish 4CD
	{
		"tlj", "4 CD", {
			{"x.xarc", 0, "a0559457126caadab0cadac02d35f26f", 3032},
			{"chapters.ini", 0, "f6a2007300209492b7b90b4c0467832d", 462},
			{NULL, 0, NULL, 0}
		},
		Common::SE_SWE,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO_NONE
	},

	// The Longest Journey
	// Swedish DVD Nordic Special Edition - supplied by L0ngcat
	{
		"tlj", "DVD", {
			{"x.xarc", 0, "de8327850d7bba90b690b141eaa23f61", 3032},
			{"chapters.ini", 0, "f6a2007300209492b7b90b4c0467832d", 462},
			{NULL, 0, NULL, 0}
		},
		Common::SE_SWE,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO_NONE
	},

	// The Longest Journey
	// Polish 4CD
	{
		"tlj", "4 CD", {
			{"x.xarc", 0, "a0559457126caadab0cadac02d35f26f", 3032},
			{"chapters.ini", 0, "6abc5c38e6e31face4b675355b117620", 499},
			{NULL, 0, NULL, 0}
		},
		Common::PL_POL,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO_NONE
	},

	AD_TABLE_END_MARKER
};
/*
// File based fallback game description
static const ADGameDescription fallbackDescription = {
	"tlj", "Unknown", {
		{"x.xarc", 0, NULL, -1},
		{"chapters.ini", 0, NULL, -1},
		{NULL, 0, NULL, 0}
	},
	Common::UNK_LANG,
	Common::kPlatformWindows,
	ADGF_NO_FLAGS,
	GUIO_NONE
};

static const ADFileBasedFallback fileBasedFallback[] = {
	{&fallbackDescription, {"x.xarc", "chapters.ini", "w_world.ini", NULL}},
	{NULL, {NULL}}
};*/

class StarkMetaEngine : public AdvancedMetaEngine {
public:
	StarkMetaEngine() : AdvancedMetaEngine(gameDescriptions, sizeof(ADGameDescription), starkGames) {
		_singleId = "stark";
		_guiOptions = GUIO1(GUIO_NOMIDI);
	}

	const char *getName() const override {
		return "Stark Engine";
	}

	const char *getOriginalCopyright() const override {
		return "(C) Funcom";
	}

	bool hasFeature(MetaEngineFeature f) const override {
		return
			(f == kSupportsListSaves) ||
			(f == kSupportsLoadingDuringStartup) ||
			(f == kSupportsDeleteSave) ||
			(f == kSavesSupportThumbnail) ||
			(f == kSavesSupportMetaInfo) ||
			(f == kSavesSupportPlayTime) ||
			(f == kSavesSupportCreationDate);
	}

	int getMaximumSaveSlot() const override {
		return 999;
	}

	SaveStateList listSaves(const char *target) const override {
		Common::String pattern = Common::String::format("%s-###.tlj", target);
		Common::StringArray filenames = g_system->getSavefileManager()->listSavefiles(pattern);

		int targetLen = strlen(target);

		SaveStateList saveList;
		for (Common::StringArray::const_iterator filename = filenames.begin(); filename != filenames.end(); ++filename) {
			// Extract the slot number from the filename
			char slot[4];
			slot[0] = (*filename)[targetLen + 1];
			slot[1] = (*filename)[targetLen + 2];
			slot[2] = (*filename)[targetLen + 3];
			slot[3] = '\0';

			// Read the description from the save
			Common::String description;
			Common::InSaveFile *save = g_system->getSavefileManager()->openForLoading(*filename);
			if (save) {
				StateReadStream stream(save);
				description = stream.readString();
			}

			saveList.push_back(SaveStateDescriptor(atoi(slot), description));
		}

		Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
		return saveList;
	}

	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override {
		Common::String filename = StarkEngine::formatSaveName(target, slot);
		Common::InSaveFile *save = g_system->getSavefileManager()->openForLoading(filename);
		if (!save) {
			return SaveStateDescriptor();
		}

		SaveMetadata metadata;
		Common::ErrorCode readError = metadata.read(save, filename);
		if (readError != Common::kNoError) {
			delete save;
			return SaveStateDescriptor();
		}

		SaveStateDescriptor descriptor;
		descriptor.setDescription(metadata.description);

		if (metadata.version >= 9) {
			Graphics::Surface *thumb = metadata.readGameScreenThumbnail(save);
			descriptor.setThumbnail(thumb);
			descriptor.setPlayTime(metadata.totalPlayTime);
			descriptor.setSaveDate(metadata.saveYear, metadata.saveMonth, metadata.saveDay);
			descriptor.setSaveTime(metadata.saveHour, metadata.saveMinute);
		}

		delete save;

		return descriptor;
	}

	void removeSaveState(const char *target, int slot) const override {
		Common::String filename = StarkEngine::formatSaveName(target, slot);
		g_system->getSavefileManager()->removeSavefile(filename);
	}

	bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override {
		if (desc)
			*engine = new StarkEngine(syst, desc);

		return desc != 0;
	}
};

} // End of namespace Stark

#if PLUGIN_ENABLED_DYNAMIC(STARK)
	REGISTER_PLUGIN_DYNAMIC(STARK, PLUGIN_TYPE_ENGINE, Stark::StarkMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(STARK, PLUGIN_TYPE_ENGINE, Stark::StarkMetaEngine);
#endif
