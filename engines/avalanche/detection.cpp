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
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */

#include "avalanche/avalanche.h"

#include "common/system.h"
#include "common/savefile.h"

#include "engines/advancedDetector.h"
#include "graphics/thumbnail.h"

namespace Avalanche {

struct AvalancheGameDescription {
	ADGameDescription desc;
};

uint32 AvalancheEngine::getFeatures() const {
	return _gameDescription->desc.flags;
}

const char *AvalancheEngine::getGameId() const {
	return _gameDescription->desc.gameId;
}

Common::Platform AvalancheEngine::getPlatform() const {
	return _gameDescription->desc.platform;
}

static const PlainGameDescriptor avalancheGames[] = {
	{"avalanche", "Lord Avalot d'Argent"},
	{0, 0}
};

static const ADGameDescription gameDescriptions[] = {
	{
		"avalanche", 0,
		{
			{"avalot.sez", 0, "de10eb353228013da3d3297784f81ff9", 48763},
			{"mainmenu.avd", 0, "89f31211af579a872045b175cc264298", 18880},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	AD_TABLE_END_MARKER
};

class AvalancheMetaEngine : public AdvancedMetaEngine {
public:
	AvalancheMetaEngine() : AdvancedMetaEngine(gameDescriptions, sizeof(AvalancheGameDescription), avalancheGames) {
	}

	const char *getName() const {
		return "Avalanche";
	}

	const char *getOriginalCopyright() const {
		return "Avalanche (C) 1994-1995 Mike, Mark and Thomas Thurman.";
	}

	bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *gd) const;
	bool hasFeature(MetaEngineFeature f) const;

	int getMaximumSaveSlot() const { return 99; }
	SaveStateList listSaves(const char *target) const;
	void removeSaveState(const char *target, int slot) const;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const;
};

bool AvalancheMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *gd) const {
	if (gd)
		*engine = new AvalancheEngine(syst, (const AvalancheGameDescription *)gd);
	return gd != 0;
}

bool AvalancheMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsDeleteSave) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSimpleSavesNames);
}

SaveStateList AvalancheMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String pattern = target;
	pattern.toUppercase();
	pattern += ".###";

	filenames = saveFileMan->listSavefiles(pattern);

	SaveStateList saveList;
	for (Common::StringArray::const_iterator filename = filenames.begin(); filename != filenames.end(); ++filename) {
		const Common::String &fname = *filename;
		int slotNum = atoi(fname.c_str() + fname.size() - 3);
		if (slotNum >= 0 && slotNum <= getMaximumSaveSlot()) {
			Common::InSaveFile *file = saveFileMan->openForLoading(fname);
			if (file) {
				// Check for our signature.
				uint32 signature = file->readUint32LE();
				if (signature != MKTAG('A', 'V', 'A', 'L')) {
					warning("Savegame of incompatible type!");
					delete file;
					continue;
				}

				// Check version.
				byte saveVersion = file->readByte();
				if (saveVersion > kSavegameVersion) {
					warning("Savegame of incompatible version!");
					delete file;
					continue;
				}

				// Read name.
				uint32 nameSize = file->readUint32LE();
				if (nameSize >= 255) {
					delete file;
					continue;
				}
				char *name = new char[nameSize + 1];
				file->read(name, nameSize);
				name[nameSize] = 0;

				saveList.push_back(SaveStateDescriptor(slotNum, name));
				delete[] name;
				delete file;
			}
		}
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

void AvalancheMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String fileName = Common::String::format("%s.%03d", target, slot);
	g_system->getSavefileManager()->removeSavefile(fileName);
}

SaveStateDescriptor AvalancheMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String fileName = Common::String::format("%s.%03d", target, slot);
	Common::InSaveFile *f = g_system->getSavefileManager()->openForLoading(fileName);

	if (f) {
		// Check for our signature.
		uint32 signature = f->readUint32LE();
		if (signature != MKTAG('A', 'V', 'A', 'L')) {
			warning("Savegame of incompatible type!");
			delete f;
			return SaveStateDescriptor();
		}

		// Check version.
		byte saveVersion = f->readByte();
		if (saveVersion > kSavegameVersion) {
			warning("Savegame of a too recent version!");
			delete f;
			return SaveStateDescriptor();
		}

		// Read the description.
		uint32 descSize = f->readUint32LE();
		Common::String description;
		for (uint32 i = 0; i < descSize; i++) {
			char actChar = f->readByte();
			description += actChar;
		}

		SaveStateDescriptor desc(slot, description);

		Graphics::Surface *thumbnail;
		if (!Graphics::loadThumbnail(*f, thumbnail)) {
			warning("Cannot read thumbnail data, possibly broken savegame");
			delete f;
			return SaveStateDescriptor();
		}
		desc.setThumbnail(thumbnail);

		delete f;
		return desc;
	}
	return SaveStateDescriptor();
}

} // End of namespace Avalanche

#if PLUGIN_ENABLED_DYNAMIC(AVALANCHE)
	REGISTER_PLUGIN_DYNAMIC(AVALANCHE, PLUGIN_TYPE_ENGINE, Avalanche::AvalancheMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(AVALANCHE, PLUGIN_TYPE_ENGINE, Avalanche::AvalancheMetaEngine);
#endif
