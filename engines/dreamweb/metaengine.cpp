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

#include "common/savefile.h"

#include "graphics/thumbnail.h"

#include "engines/advancedDetector.h"

#include "dreamweb/dreamweb.h"
#include "dreamweb/detection.h"

class DreamWebMetaEngine : public AdvancedMetaEngine {
public:
	const char *getName() const override {
		return "dreamweb";
	}

	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;

	bool hasFeature(MetaEngineFeature f) const override;

	SaveStateList listSaves(const char *target) const override;
	int getMaximumSaveSlot() const override;
	void removeSaveState(const char *target, int slot) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;
};

bool DreamWebMetaEngine::hasFeature(MetaEngineFeature f) const {
	switch(f) {
	case kSupportsListSaves:
	case kSupportsLoadingDuringStartup:
	case kSupportsDeleteSave:
	case kSavesSupportMetaInfo:
	case kSavesSupportThumbnail:
	case kSavesSupportCreationDate:
	case kSavesSupportPlayTime:
		return true;
	default:
		return false;
	}
}

bool DreamWeb::DreamWebEngine::hasFeature(EngineFeature f) const {
	switch(f) {
	case kSupportsReturnToLauncher:
		return true;
	case kSupportsSubtitleOptions:
		return _gameDescription->desc.flags & ADGF_CD;
	default:
		return false;
	}
}

Common::Error DreamWebMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new DreamWeb::DreamWebEngine(syst, (const DreamWeb::DreamWebGameDescription *)desc);
	return Common::kNoError;
}

SaveStateList DreamWebMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray files = saveFileMan->listSavefiles("DREAMWEB.D##");

	SaveStateList saveList;
	for (uint i = 0; i < files.size(); ++i) {
		const Common::String &file = files[i];
		Common::InSaveFile *stream = saveFileMan->openForLoading(file);
		if (!stream)
			error("cannot open save file %s", file.c_str());
		char name[17] = {};
		stream->seek(0x61);
		stream->read(name, sizeof(name) - 1);
		delete stream;

		int slotNum = atoi(file.c_str() + file.size() - 2);
		SaveStateDescriptor sd(slotNum, name);
		saveList.push_back(sd);
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

int DreamWebMetaEngine::getMaximumSaveSlot() const { return 99; }

void DreamWebMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String fileName = Common::String::format("DREAMWEB.D%02d", slot);
	g_system->getSavefileManager()->removeSavefile(fileName);
}

SaveStateDescriptor DreamWebMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String filename = Common::String::format("DREAMWEB.D%02d", slot);
	Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(filename.c_str());

	if (in) {
		DreamWeb::FileHeader header;
		in->read((uint8 *)&header, sizeof(DreamWeb::FileHeader));

		Common::String saveName;
		byte descSize = header.len(0);
		byte i;

		for (i = 0; i < descSize; i++)
			saveName += (char)in->readByte();

		SaveStateDescriptor desc(slot, saveName);

		// Check if there is a ScummVM data block
		if (header.len(6) == SCUMMVM_BLOCK_MAGIC_SIZE) {
			// Skip the game data
			for (i = 1; i <= 5; i++)
				in->skip(header.len(i));

			uint32 tag = in->readUint32BE();
			if (tag != SCUMMVM_HEADER) {
				warning("ScummVM data block found, but the block header is incorrect - skipping");
				delete in;
				return desc;
			}

			byte version = in->readByte();
			if (version > SAVEGAME_VERSION) {
				warning("ScummVM data block found, but it has been saved with a newer version of ScummVM - skipping");
				delete in;
				return desc;
			}

			uint32 saveDate = in->readUint32LE();
			uint32 saveTime = in->readUint32LE();
			uint32 playTime = in->readUint32LE();
			Graphics::Surface *thumbnail;
			if (!Graphics::loadThumbnail(*in, thumbnail)) {
				warning("Missing or broken thumbnail - skipping");
				delete in;
				return desc;
			}

			int day = (saveDate >> 24) & 0xFF;
			int month = (saveDate >> 16) & 0xFF;
			int year = saveDate & 0xFFFF;
			int hour = (saveTime >> 16) & 0xFF;
			int minutes = (saveTime >> 8) & 0xFF;

			desc.setSaveDate(year, month, day);
			desc.setSaveTime(hour, minutes);
			desc.setPlayTime(playTime * 1000);
			desc.setThumbnail(thumbnail);
		}

		delete in;
		return desc;
	}

	return SaveStateDescriptor();
}

#if PLUGIN_ENABLED_DYNAMIC(DREAMWEB)
	REGISTER_PLUGIN_DYNAMIC(DREAMWEB, PLUGIN_TYPE_ENGINE, DreamWebMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(DREAMWEB, PLUGIN_TYPE_ENGINE, DreamWebMetaEngine);
#endif

namespace DreamWeb {

Common::Error DreamWebEngine::loadGameState(int slot) {
	return Common::kNoError;
}

Common::Error DreamWebEngine::saveGameState(int slot, const Common::String &desc, bool isAutosave) {
	return Common::kNoError;
}

bool DreamWebEngine::canLoadGameStateCurrently() {
	return false;
}

bool DreamWebEngine::canSaveGameStateCurrently() {
	return false;
}

Common::Language DreamWebEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

bool DreamWebEngine::isCD() {
	return _gameDescription->desc.flags & ADGF_CD;
}

} // End of namespace DreamWeb
