/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "engines/advancedDetector.h"
#include "common/system.h"
#include "common/savefile.h"
#include "common/textconsole.h"
#include "graphics/thumbnail.h"
#include "graphics/surface.h"

#include "efh/efh.h"

namespace Efh {

uint32 EfhEngine::getFeatures() const {
	return _gameDescription->flags;
}

const char *EfhEngine::getGameId() const {
	return _gameDescription->gameId;
}

void EfhEngine::initGame(const ADGameDescription *gd) {
	_platform = gd->platform;
}

bool EfhEngine::hasFeature(EngineFeature f) const {
	return (f == kSupportsReturnToLauncher) || (f == kSupportsLoadingDuringRuntime) || (f == kSupportsSavingDuringRuntime);
}

const char *EfhEngine::getCopyrightString() const {
	return "Escape From Hell (C) Electronic Arts, 1990";
}

Common::Platform EfhEngine::getPlatform() const {
	return _platform;
}
} // End of namespace Efh

namespace Efh {

class EfhMetaEngine : public AdvancedMetaEngine<ADGameDescription> {
public:
	const char *getName() const override {
		return "efh";
	}

	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *gd) const override;
	bool hasFeature(MetaEngineFeature f) const override;

	int getMaximumSaveSlot() const override;
	SaveStateList listSaves(const char *target) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;
	bool removeSaveState(const char *target, int slot) const override;
};

Common::Error EfhMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *gd) const {
	*engine = new EfhEngine(syst, gd);
	((EfhEngine *)*engine)->initGame(gd);
	return Common::kNoError;
}

bool EfhMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSavesSupportCreationDate);
}

int EfhMetaEngine::getMaximumSaveSlot() const {
	return 99;
}

SaveStateList EfhMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::String pattern = target;
	pattern += ".###";

	Common::StringArray filenames = saveFileMan->listSavefiles(pattern);

	SaveStateList saveList;
	char slot[3];
	for (Common::StringArray::const_iterator filename = filenames.begin(); filename != filenames.end(); ++filename) {
		slot[0] = filename->c_str()[filename->size() - 2];
		slot[1] = filename->c_str()[filename->size() - 1];
		slot[2] = '\0';
		// Obtain the last 2 digits of the filename (without extension), since they correspond to the save slot
		int slotNum = atoi(slot);
		if (slotNum >= 0 && slotNum <= getMaximumSaveSlot()) {
			Common::InSaveFile *file = saveFileMan->openForLoading(*filename);
			if (file) {
				uint32 sign = file->readUint32LE();
				uint8 saveVersion = file->readByte();

				if (sign != EFH_SAVE_HEADER || saveVersion > kSavegameVersion) {
					warning("Incompatible savegame");
					delete file;
					continue;
				}

				// read name
				uint16 nameSize = file->readUint16LE();
				if (nameSize >= 255) {
					delete file;
					continue;
				}
				char name[256];
				file->read(name, nameSize);
				name[nameSize] = 0;

				saveList.push_back(SaveStateDescriptor(this, slotNum, name));
				delete file;
			}
		}
	}

	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

SaveStateDescriptor EfhMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String fileName = Common::String::format("%s.%03d", target, slot);
	Common::InSaveFile *file = g_system->getSavefileManager()->openForLoading(fileName);

	if (file) {
		uint32 sign = file->readUint32LE();
		uint8 saveVersion = file->readByte();

		if (sign != EFH_SAVE_HEADER || saveVersion > kSavegameVersion) {
			warning("Incompatible savegame");
			delete file;
			return SaveStateDescriptor();
		}

		uint32 saveNameLength = file->readUint16LE();
		Common::String saveName;
		for (uint32 i = 0; i < saveNameLength; ++i) {
			char curChr = file->readByte();
			saveName += curChr;
		}

		SaveStateDescriptor desc(this, slot, saveName);

		Graphics::Surface *thumbnail;
		if (!Graphics::loadThumbnail(*file, thumbnail)) {
			delete file;
			return SaveStateDescriptor();
		}
		desc.setThumbnail(thumbnail);

		// Read in save date/time
		int16 year = file->readSint16LE();
		int16 month = file->readSint16LE();
		int16 day = file->readSint16LE();
		int16 hour = file->readSint16LE();
		int16 minute = file->readSint16LE();
		desc.setSaveDate(year, month, day);
		desc.setSaveTime(hour, minute);

		desc.setDeletableFlag(slot != 0);
		desc.setWriteProtectedFlag(slot == 0);

		delete file;
		return desc;
	}
	return SaveStateDescriptor();
}

bool EfhMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String fileName = Common::String::format("%s.%03d", target, slot);
	return g_system->getSavefileManager()->removeSavefile(fileName);
}

} // End of namespace Efh

#if PLUGIN_ENABLED_DYNAMIC(EFH)
	REGISTER_PLUGIN_DYNAMIC(EFH, PLUGIN_TYPE_ENGINE, Efh::EfhMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(EFH, PLUGIN_TYPE_ENGINE, Efh::EfhMetaEngine);
#endif
