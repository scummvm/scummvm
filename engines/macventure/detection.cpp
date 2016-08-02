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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "base/plugins.h"

#include "engines/advancedDetector.h"
#include "common/system.h"

#include "macventure/macventure.h"

namespace MacVenture {

const char *MacVentureEngine::getGameFileName() const {
	return _gameDescription->filesDescriptions[0].fileName;
}
}

namespace MacVenture {

#include "macventure/detection_tables.h"

static const PlainGameDescriptor macventureGames[] = {
	{ "shadowgate", "Shadowgate" },
	{ "deja_vu", "Deja Vu"},
	{ "deja_vu2", "Deja Vu II"},
	{ 0, 0 }
};

SaveStateDescriptor loadMetaData(Common::SeekableReadStream *s, int slot);

class MacVentureMetaEngine : public AdvancedMetaEngine {
public:
	MacVentureMetaEngine() : AdvancedMetaEngine(MacVenture::gameDescriptions, sizeof(ADGameDescription), macventureGames) {
		_guiOptions = GUIO0();
		_md5Bytes = 5000000; // TODO: Upper limit, adjust it once all games are added
	}

	virtual const char * getName() const override {
		return "MacVenture";
	}
	virtual const char * getOriginalCopyright() const override {
		return "(C) ICOM Simulations";
	}

	virtual bool createInstance(OSystem * syst, Engine ** engine, const ADGameDescription * desc) const;
	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual SaveStateList listSaves(const char *target) const;
	virtual int getMaximumSaveSlot() const;
};

bool MacVentureMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup);
}

SaveStateList MacVentureMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String pattern = target;
	pattern += ".###";

	filenames = saveFileMan->listSavefiles(pattern);

	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		int slotNum = atoi(file->c_str() + file->size() - 3);
		Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(*file);

		SaveStateDescriptor desc;
		// Do not allow save slot 0 (used for auto-saving) to be deleted or
		// overwritten.
		desc.setDeletableFlag(slotNum != 0);
		desc.setWriteProtectedFlag(slotNum == 0);

		if (slotNum >= 0 && slotNum <= getMaximumSaveSlot()) {
			Common::InSaveFile *in = saveFileMan->openForLoading(*file);
			if (in) {
				SaveStateDescriptor desc = loadMetaData(in, slotNum);
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

int MacVentureMetaEngine::getMaximumSaveSlot() const { return 999; }

bool MacVentureMetaEngine::createInstance(OSystem * syst, Engine ** engine, const ADGameDescription *game) const {
	if (game) {
		*engine = new MacVentureEngine(syst, game);
	}
	return game != 0;
}




} // End of namespace MacVenture

#if PLUGIN_ENABLED_DYNAMIC(MACVENTURE)
 REGISTER_PLUGIN_DYNAMIC(MACVENTURE, PLUGIN_TYPE_ENGINE, MacVenture::MacVentureMetaEngine);
#else
 REGISTER_PLUGIN_STATIC(MACVENTURE, PLUGIN_TYPE_ENGINE, MacVenture::MacVentureMetaEngine);
#endif
