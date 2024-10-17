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

#include "gnap/gnap.h"

#include "common/config-manager.h"
#include "engines/advancedDetector.h"
#include "common/savefile.h"
#include "common/system.h"
#include "base/plugins.h"
#include "graphics/thumbnail.h"

class GnapMetaEngine : public AdvancedMetaEngine<ADGameDescription> {
public:
	const char *getName() const override {
		return "gnap";
	}

	bool hasFeature(MetaEngineFeature f) const override;
	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;

	int getMaximumSaveSlot() const override;
	SaveStateList listSaves(const char *target) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;
	bool removeSaveState(const char *target, int slot) const override;
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
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

bool GnapMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String fileName = Common::String::format("%s.%03d", target, slot);
	return g_system->getSavefileManager()->removeSavefile(fileName);
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
					saveList.push_back(SaveStateDescriptor(this, slot, header._saveName));
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

		SaveStateDescriptor desc(this, slot, saveName);

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

Common::Error GnapMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new Gnap::GnapEngine(syst, desc);
	return Common::kNoError;
}

#if PLUGIN_ENABLED_DYNAMIC(GNAP)
	REGISTER_PLUGIN_DYNAMIC(GNAP, PLUGIN_TYPE_ENGINE, GnapMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(GNAP, PLUGIN_TYPE_ENGINE, GnapMetaEngine);
#endif
