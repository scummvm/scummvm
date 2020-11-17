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

#include "common/memstream.h"
#include "engines/advancedDetector.h"
#include "common/system.h"
#include "graphics/surface.h"

#include "tony/tony.h"
#include "tony/game.h"
#include "tony/detection.h"

namespace Tony {

uint32 TonyEngine::getFeatures() const {
	return _gameDescription->desc.flags;
}

Common::Language TonyEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

bool TonyEngine::getIsDemo() const {
	return _gameDescription->desc.flags & ADGF_DEMO;
}

bool TonyEngine::isCompressed() const {
	return _gameDescription->desc.flags & GF_COMPRESSED;
}

} // End of namespace Tony

class TonyMetaEngine : public AdvancedMetaEngine {
public:
	const char *getName() const override {
		return "tony";
	}

    bool hasFeature(MetaEngineFeature f) const override;
	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;

	SaveStateList listSaves(const char *target) const override;
	int getMaximumSaveSlot() const override;
	void removeSaveState(const char *target, int slot) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;
};

bool TonyMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail);
}

bool Tony::TonyEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

Common::Error TonyMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new Tony::TonyEngine(syst, (const Tony::TonyGameDescription *)desc);
	return Common::kNoError;
}

SaveStateList TonyMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String saveDesc;
	Common::String pattern = "tony.0##";

	filenames = saveFileMan->listSavefiles(pattern);

	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		int slotNum = atoi(file->c_str() + file->size() - 3);

		if (slotNum >= 0 && slotNum <= 999) {
			byte thumbnailData[160 * 120 * 2];
			Common::String saveName;
			byte difficulty;

			if (Tony::RMOptionScreen::loadThumbnailFromSaveState(slotNum, thumbnailData, saveName, difficulty)) {
				// Add the save name to the savegame list
				saveList.push_back(SaveStateDescriptor(slotNum, saveName));
			}
		}
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

int TonyMetaEngine::getMaximumSaveSlot() const {
	return 99;
}

void TonyMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String filename = Tony::TonyEngine::getSaveStateFileName(slot);

	g_system->getSavefileManager()->removeSavefile(filename);
}

SaveStateDescriptor TonyMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String saveName;
	byte difficulty;

	Graphics::Surface *to = new Graphics::Surface();
	to->create(160, 120, Graphics::PixelFormat(2, 5, 5, 5, 0, 10, 5, 0, 0));

	if (Tony::RMOptionScreen::loadThumbnailFromSaveState(slot, (byte *)to->getPixels(), saveName, difficulty)) {
#ifdef SCUMM_BIG_ENDIAN
		uint16 *pixels = (uint16 *)to->getPixels();
		for (int i = 0; i < to->w * to->h; ++i)
			pixels[i] = READ_LE_UINT16(pixels + i);
#endif
		// Create the return descriptor
		SaveStateDescriptor desc(slot, saveName);
		desc.setDeletableFlag(true);
		desc.setWriteProtectedFlag(false);
		desc.setThumbnail(to);

		return desc;
	}

	delete to;
	return SaveStateDescriptor();
}

#if PLUGIN_ENABLED_DYNAMIC(TONY)
	REGISTER_PLUGIN_DYNAMIC(TONY, PLUGIN_TYPE_ENGINE, TonyMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(TONY, PLUGIN_TYPE_ENGINE, TonyMetaEngine);
#endif
