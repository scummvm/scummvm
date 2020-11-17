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
#include "common/savefile.h"
#include "common/serializer.h"
#include "common/str-array.h"
#include "engines/advancedDetector.h"

#include "composer/composer.h"
#include "composer/detection.h"

namespace Composer {

int ComposerEngine::getGameType() const {
	return _gameDescription->gameType;
}

const char *ComposerEngine::getGameId() const {
	return _gameDescription->desc.gameId;
}

uint32 ComposerEngine::getFeatures() const {
	return _gameDescription->desc.flags;
}

Common::Language ComposerEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

Common::Platform ComposerEngine::getPlatform() const {
	return _gameDescription->desc.platform;
}

bool ComposerEngine::loadDetectedConfigFile(Common::INIFile &configFile) const {
	const ADGameFileDescription *res = _gameDescription->desc.filesDescriptions;
	while (res->fileName != NULL) {
		if (res->fileType == GAME_CONFIGFILE) {
			return configFile.loadFromFile(res->fileName);
		}
		res++;
	}
	// default config file name
	return configFile.loadFromFile("book.ini") || configFile.loadFromFile("book.mac");
}

} // End of namespace Composer

class ComposerMetaEngine : public AdvancedMetaEngine {
public:
    const char *getName() const override {
		return "composer";
	}

	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;
	bool hasFeature(MetaEngineFeature f) const override;

	int getMaximumSaveSlot() const override;
	SaveStateList listSaves(const char* target) const override;
};

Common::Error ComposerMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new Composer::ComposerEngine(syst, (const Composer::ComposerGameDescription *)desc);
	return Common::kNoError;
}

bool ComposerMetaEngine::hasFeature(MetaEngineFeature f) const {
	return ((f == kSupportsListSaves) || (f == kSupportsLoadingDuringStartup));
}

Common::String getSaveName(Common::InSaveFile *in) {
	Common::Serializer ser(in, NULL);
	Common::String name;
	uint32 tmp;
	ser.syncAsUint32LE(tmp);
	ser.syncAsUint32LE(tmp);
	ser.syncString(name);
	return name;
}
int ComposerMetaEngine::getMaximumSaveSlot() const {
	return 99;
}
SaveStateList ComposerMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String saveDesc;
	Common::String pattern = Common::String::format("%s.##", target);

	filenames = saveFileMan->listSavefiles(pattern);

	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		int slotNum = atoi(file->c_str() + file->size() - 2);

		if (slotNum >= 0 && slotNum <= 99) {
			Common::InSaveFile *in = saveFileMan->openForLoading(*file);
			if (in) {
				saveDesc = getSaveName(in);
				saveList.push_back(SaveStateDescriptor(slotNum, saveDesc));
				delete in;
			}
		}
	}

	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

bool Composer::ComposerEngine::hasFeature(EngineFeature f) const {
	return (f == kSupportsReturnToLauncher
		|| f == kSupportsSavingDuringRuntime
		|| f == kSupportsLoadingDuringRuntime);
}

#if PLUGIN_ENABLED_DYNAMIC(COMPOSER)
	REGISTER_PLUGIN_DYNAMIC(COMPOSER, PLUGIN_TYPE_ENGINE, ComposerMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(COMPOSER, PLUGIN_TYPE_ENGINE, ComposerMetaEngine);
#endif
