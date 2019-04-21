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

#include "base/plugins.h"
#include "common/file.h"
#include "common/translation.h"

#include "engines/advancedDetector.h"
#include "engines/savestate.h"

#include "graphics/thumbnail.h"

#include "drascula/drascula.h"

namespace Drascula {

struct DrasculaGameDescription {
	ADGameDescription desc;
};

uint32 DrasculaEngine::getFeatures() const {
	return _gameDescription->desc.flags;
}

Common::Language DrasculaEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

void DrasculaEngine::loadArchives() {
	const ADGameFileDescription *ag;

	if (getFeatures() & GF_PACKED) {
		for (ag = _gameDescription->desc.filesDescriptions; ag->fileName; ag++) {
			if (!_archives.hasArchive(ag->fileName))
				_archives.registerArchive(ag->fileName, ag->fileType);
		}
	}

	_archives.enableFallback(true);
}

}

static const PlainGameDescriptor drasculaGames[] = {
	{"drascula", "Drascula: The Vampire Strikes Back"},
	{0, 0}
};

namespace Drascula {

static const DrasculaGameDescription gameDescriptions[] = {

	//// Packed versions //////////////////////////////////////////////////////

	{
		// Drascula English version (original packed files)
		{
			"drascula",
			0,
			{
				{"packet.001", 0, "c6a8697396e213a18472542d5f547cb4", 32847563},
				// HACK: List packet.001 twice to ensure this detector entry
				// is ranked just as high as the others (which each have two
				// detection files).
				{"packet.001", 0, "c6a8697396e213a18472542d5f547cb4", 32847563},
				{NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformDOS,
			GF_PACKED,
			GUIO1(GUIO_LINKSPEECHTOSFX)
		},
	},

	{
		// Drascula French version (original packed files)
		{
			"drascula",
			0,
			{
				{"packet.001", 0, "c6a8697396e213a18472542d5f547cb4", 32847563},
				{"packet.002", 1, "4401123400f22f212b89f15fb4b43013", 721122},
				{NULL, 0, NULL, 0}
			},
			Common::FR_FRA,
			Common::kPlatformDOS,
			GF_PACKED,
			GUIO1(GUIO_LINKSPEECHTOSFX)
		},
	},

	{
		// Drascula French version (ScummVM repacked files)
		{
			"drascula",
			0,
			{
				{"packet.001", 0, "c6a8697396e213a18472542d5f547cb4", 32847563},
				{"packet.002", 1, "7b83cedb9bb326ed5143e5c459508d43", 722383},
				{NULL, 0, NULL, 0}
			},
			Common::FR_FRA,
			Common::kPlatformDOS,
			GF_PACKED,
			GUIO1(GUIO_LINKSPEECHTOSFX)
		},
	},

	{
		// Drascula German version (original packed files)
		{
			"drascula",
			0,
			{
				{"packet.001", 0, "c6a8697396e213a18472542d5f547cb4", 32847563},
				{"packet.003", 1, "e8f4dc6091037329bab4ddb1cba35807", 719728},
				{NULL, 0, NULL, 0}
			},
			Common::DE_DEU,
			Common::kPlatformDOS,
			GF_PACKED,
			GUIO1(GUIO_LINKSPEECHTOSFX)
		},
	},

	{
		// Drascula Italian version (original packed version)
		{
			"drascula",
			0,
			AD_ENTRY1s("packet.001", "0253e924af223f5fe52537023385159b", 32564209),
			Common::IT_ITA,
			Common::kPlatformDOS,
			GF_PACKED,
			GUIO1(GUIO_LINKSPEECHTOSFX)
		},
	},

	{
		// Drascula Italian version (ScummVM repacked files)
		{
			"drascula",
			0,
			{
				{"packet.001", 0, "c6a8697396e213a18472542d5f547cb4", 32847563},
				{"packet.005", 1, "58caac54b891f5d7f335e710e45e5d29", 16209623},
				{NULL, 0, NULL, 0}
			},
			Common::IT_ITA,
			Common::kPlatformDOS,
			GF_PACKED,
			GUIO1(GUIO_LINKSPEECHTOSFX)
		},
	},

	{
		// Drascula Spanish version (original packed version)
		{
			"drascula",
			0,
			AD_ENTRY1s("packet.001", "3c971aba65a037d29d0b479cad6f5943", 31702652),
			Common::ES_ESP,
			Common::kPlatformDOS,
			GF_PACKED,
			GUIO1(GUIO_LINKSPEECHTOSFX)
		},
	},

	{
		// Drascula Spanish version (ScummVM repacked files)
		{
			"drascula",
			0,
			{
				{"packet.001", 0, "c6a8697396e213a18472542d5f547cb4", 32847563},
				{"packet.004", 1, "a289d3cf80d50f25ec569b653248437e", 17205838},
				{NULL, 0, NULL, 0}
			},
			Common::ES_ESP,
			Common::kPlatformDOS,
			GF_PACKED,
			GUIO1(GUIO_LINKSPEECHTOSFX)
		},
	},

	{
		// Drascula Russian version (ScummVM repacked files)
		{
			"drascula",
			0,
			{
				{"packet.001", 0, "c6a8697396e213a18472542d5f547cb4", 32847563},
				{"packet.006", 1, "e464b99de7f226391337510d5c328258", 697173},
				{NULL, 0, NULL, 0}
			},
			Common::RU_RUS,
			Common::kPlatformDOS,
			GF_PACKED,
			GUIO1(GUIO_LINKSPEECHTOSFX)
		},
	},

	//// Unpacked versions ////////////////////////////////////////////////////

	{
		// Drascula English version
		{
			"drascula",
			0,
			AD_ENTRY1s("14.ald", "09b2735953edcd43af115c65ae00b10e", 1595),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_LINKSPEECHTOSFX)
		},
	},

	{
		// Drascula French version
		{
			"drascula",
			0,
			AD_ENTRY1s("14.ald", "eeeee96b82169003630e08992248296c", 608),
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_LINKSPEECHTOSFX)
		},
	},

	{
		// Drascula French version (updated - bug #3612236)
		{
			"drascula",
			0,
			AD_ENTRY1s("14.ald", "1f9fbded768bee061cc22bc5bdeab540", 611),
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_LINKSPEECHTOSFX)
		},
	},

	{
		// Drascula German version
		{
			"drascula",
			0,
			AD_ENTRY1s("14.ald", "72e46089033d56bad1c179ac36e2a9d2", 610),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_LINKSPEECHTOSFX)
		},
	},

	{
		// Drascula Italian version
		{
			"drascula",
			0,
			AD_ENTRY1s("14.ald", "02b49a18328d0bf2efe6ba658c9c7a1d", 2098),
			Common::IT_ITA,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_LINKSPEECHTOSFX)
		},
	},

	{
		// Drascula Italian version (updated - bug #3612236)
		{
			"drascula",
			0,
			AD_ENTRY1s("14.ald", "ccaee939bb3b344c048f28f9205710d1", 2925),
			Common::IT_ITA,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_LINKSPEECHTOSFX)
		},
	},

	{
		// Drascula Spanish version
		{
			"drascula",
			0,
			AD_ENTRY1s("14.ald", "0746ed1a5cc8d9728f790c29813f4b43", 23059),
			Common::ES_ESP,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_LINKSPEECHTOSFX)
		},
	},
	{
		// Drascula Russian version
		{
			"drascula",
			0,
			AD_ENTRY1s("14.ald", "4dfab170eae935a2e9889196df427a4a", 1426),
			Common::RU_RUS,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_LINKSPEECHTOSFX)
		},
	},

	{ AD_TABLE_END_MARKER }
};

static const ExtraGuiOption drasculaExtraGuiOption = {
	_s("Use original save/load screens"),
	_s("Use the original save/load screens instead of the ScummVM ones"),
	"originalsaveload",
	false
};

SaveStateDescriptor loadMetaData(Common::ReadStream *s, int slot, bool setPlayTime);

class DrasculaMetaEngine : public AdvancedMetaEngine {
public:
	DrasculaMetaEngine() : AdvancedMetaEngine(Drascula::gameDescriptions, sizeof(Drascula::DrasculaGameDescription), drasculaGames) {
		_singleId = "drascula";
		_guiOptions = GUIO1(GUIO_NOMIDI);
	}

	virtual const char *getName() const {
		return "Drascula: The Vampire Strikes Back";
	}

	virtual const char *getOriginalCopyright() const {
		return "Drascula: The Vampire Strikes Back (C) 2000 Alcachofa Soft, (C) 1996 Digital Dreams Multimedia, (C) 1994 Emilio de Paz";
	}

	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *gd) const;
	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual const ExtraGuiOptions getExtraGuiOptions(const Common::String &target) const;
	virtual SaveStateList listSaves(const char *target) const;
	virtual int getMaximumSaveSlot() const;
	virtual void removeSaveState(const char *target, int slot) const;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const;
};

bool DrasculaMetaEngine::hasFeature(MetaEngineFeature f) const {
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

const ExtraGuiOptions DrasculaMetaEngine::getExtraGuiOptions(const Common::String &target) const {
	ExtraGuiOptions options;
	options.push_back(drasculaExtraGuiOption);
	return options;
}

SaveStateList DrasculaMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String pattern = target;
	pattern += ".###";

	filenames = saveFileMan->listSavefiles(pattern);

	SaveStateList saveList;
	int slotNum = 0;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		slotNum = atoi(file->c_str() + file->size() - 3);

		if (slotNum >= 0 && slotNum <= getMaximumSaveSlot()) {
			Common::InSaveFile *in = saveFileMan->openForLoading(*file);
			if (in) {
				SaveStateDescriptor desc = loadMetaData(in, slotNum, false);
				if (desc.getSaveSlot() != slotNum) {
					// invalid
					delete in;
					continue;
				}
				saveList.push_back(desc);
				delete in;
			}
		}
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

SaveStateDescriptor DrasculaMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String fileName = Common::String::format("%s.%03d", target, slot);

	Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(fileName);

	SaveStateDescriptor desc;
	// Do not allow save slot 0 (used for auto-saving) to be deleted or
	// overwritten.
	desc.setDeletableFlag(slot != 0);
	desc.setWriteProtectedFlag(slot == 0);

	if (in) {
		desc = Drascula::loadMetaData(in, slot, false);
		if (desc.getSaveSlot() != slot) {
			delete in;
			return SaveStateDescriptor();
		}

		Graphics::Surface *thumbnail;
		if (!Graphics::loadThumbnail(*in, thumbnail)) {
			delete in;
			return SaveStateDescriptor();
		}
		desc.setThumbnail(thumbnail);

		delete in;
	}

	return desc;
}

int DrasculaMetaEngine::getMaximumSaveSlot() const { return 999; }

void DrasculaMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String fileName = Common::String::format("%s.%03d", target, slot);
	g_system->getSavefileManager()->removeSavefile(fileName);
}

bool DrasculaMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const Drascula::DrasculaGameDescription *gd = (const Drascula::DrasculaGameDescription *)desc;
	if (gd) {
		*engine = new Drascula::DrasculaEngine(syst, gd);
	}
	return gd != 0;
}

} // End of namespace Drascula

#if PLUGIN_ENABLED_DYNAMIC(DRASCULA)
	REGISTER_PLUGIN_DYNAMIC(DRASCULA, PLUGIN_TYPE_ENGINE, Drascula::DrasculaMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(DRASCULA, PLUGIN_TYPE_ENGINE, Drascula::DrasculaMetaEngine);
#endif
