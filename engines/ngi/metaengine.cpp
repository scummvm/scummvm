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

#include "engines/advancedDetector.h"
#include "common/file.h"

#include "graphics/surface.h"

#include "ngi/ngi.h"
#include "ngi/detection.h"
#include "ngi/gameloader.h"

namespace NGI {

uint32 NGIEngine::getFeatures() const {
	return _gameDescription->desc.flags;
}

bool NGIEngine::isDemo() {
	return _gameDescription->desc.flags & ADGF_DEMO;
}

Common::Language NGIEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

const char *NGIEngine::getGameId() const {
	return _gameDescription->desc.gameId;
}

int NGIEngine::getGameGID() const {
	return _gameDescription->gameId;
}

} // End of namspace Fullpipe

class NGIMetaEngine : public AdvancedMetaEngine {
public:
	const char *getName() const override {
		return "ngi";
	}

	bool hasFeature(MetaEngineFeature f) const override;

	int getMaximumSaveSlot() const override { return 99; }
	SaveStateList listSaves(const char *target) const override;
	void removeSaveState(const char *target, int slot) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;

	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;
};

bool NGIMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSavesSupportCreationDate) ||
		(f == kSavesSupportPlayTime) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSimpleSavesNames);
}

bool NGI::NGIEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

SaveStateList NGIMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String pattern("fullpipe.s##");

	filenames = saveFileMan->listSavefiles(pattern);

	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		// Obtain the last 2 digits of the filename, since they correspond to the save slot
		int slotNum = atoi(file->c_str() + file->size() - 2);

		if (slotNum >= 0 && slotNum <= getMaximumSaveSlot()) {
			Common::ScopedPtr<Common::InSaveFile> in(saveFileMan->openForLoading(*file));
			if (in) {
				NGI::FullpipeSavegameHeader header;
				if (!NGI::readSavegameHeader(in.get(), header)) {
					continue;
				}

				SaveStateDescriptor desc;

				NGI::parseSavegameHeader(header, desc);

				desc.setSaveSlot(slotNum);

				saveList.push_back(desc);
			}
		}
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

void NGIMetaEngine::removeSaveState(const char *target, int slot) const {
	g_system->getSavefileManager()->removeSavefile(NGI::getSavegameFile(slot));
}

SaveStateDescriptor NGIMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::ScopedPtr<Common::InSaveFile> f(g_system->getSavefileManager()->openForLoading(
		NGI::getSavegameFile(slot)));

	if (f) {
		NGI::FullpipeSavegameHeader header;
		if (!NGI::readSavegameHeader(f.get(), header, false)) {
			return SaveStateDescriptor();
		}

		// Create the return descriptor
		SaveStateDescriptor desc;

		NGI::parseSavegameHeader(header, desc);

		desc.setSaveSlot(slot);
		desc.setThumbnail(header.thumbnail);

		return desc;
	}

	return SaveStateDescriptor();
}

Common::Error NGIMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new NGI::NGIEngine(syst, (const NGI::NGIGameDescription *)desc);
	return Common::kNoError;
}

#if PLUGIN_ENABLED_DYNAMIC(NGI)
	REGISTER_PLUGIN_DYNAMIC(NGI, PLUGIN_TYPE_ENGINE, NGIMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(NGI, PLUGIN_TYPE_ENGINE, NGIMetaEngine);
#endif
