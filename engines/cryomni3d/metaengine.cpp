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

#include "base/plugins.h"

#include "engines/advancedDetector.h"
#include "common/file.h"
#include "common/md5.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/translation.h"

#include "cryomni3d/cryomni3d.h"

#ifdef ENABLE_VERSAILLES
#include "cryomni3d/versailles/engine.h"
#endif

#ifdef ENABLE_ATLANTIS
#include "cryomni3d/atlantis/engine.h"
#endif

#include "cryomni3d/detection.h"

namespace CryOmni3D {

const char *CryOmni3DEngine::getGameId() const {
	return _gameDescription->desc.gameId;
}

uint32 CryOmni3DEngine::getFeatures() const {
	return _gameDescription->features;
}

Common::Platform CryOmni3DEngine::getPlatform() const {
	return _gameDescription->desc.platform;
}

uint8 CryOmni3DEngine::getGameType() const {
	return _gameDescription->gameType;
}

Common::Language CryOmni3DEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

bool CryOmni3DEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher)
		|| (f == kSupportsSubtitleOptions);
}


class CryOmni3DMetaEngine : public AdvancedMetaEngine<CryOmni3DGameDescription> {
public:
	const char *getName() const override {
		return "cryomni3d";
	}

	bool hasFeature(MetaEngineFeature f) const override;
	Common::Error createInstance(OSystem *syst, Engine **engine, const CryOmni3DGameDescription *desc) const override;

	SaveStateList listSaves(const char *target) const override;
	int getMaximumSaveSlot() const override { return 999; }
	bool removeSaveState(const char *target, int slot) const override;
	Common::String getSavegameFile(int saveGameIdx, const char *target) const override {
		if (!target)
			target = getName();
		if (saveGameIdx == kSavegameFilePattern)
			return Common::String::format("%s.####", target);
		else
			return Common::String::format("%s.%04d", target, saveGameIdx + 1);
	}
};

bool CryOmni3DMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves)
		|| (f == kSupportsLoadingDuringStartup)
		|| (f == kSupportsDeleteSave);
}

SaveStateList CryOmni3DMetaEngine::listSaves(const char *target) const {
	// Replicate constant here to shorten lines
	static const uint kSaveDescriptionLen = CryOmni3DEngine::kSaveDescriptionLen;
	SaveStateList saveList;

	Common::SaveFileManager *saveMan = g_system->getSavefileManager();

	// Atlantis partitions save slots into per-player blocks of 100; load the
	// player-profile names so each save can be labelled with its owner.  For
	// other games this file is absent and no prefix is applied.
	Common::String playerNames[5];
	{
		Common::InSaveFile *pin = saveMan->openForLoading("atlantis-players.dat");
		if (pin) {
			if (pin->readUint32BE() == 0x41504c52u /* 'APLR' */
			        && pin->readUint32BE() >= 1) {
				uint32 count = pin->readUint32BE();
				for (uint32 i = 0; i < count && i < 5 && !pin->eos(); i++) {
					uint16 len = pin->readUint16BE();
					Common::String name;
					for (uint16 j = 0; j < len && !pin->eos(); j++)
						name += (char)pin->readByte();
					playerNames[i] = name;
				}
			}
			delete pin;
		}
	}

	char saveName[kSaveDescriptionLen + 1];
	saveName[kSaveDescriptionLen] = '\0';
	Common::StringArray filenames = saveMan->listSavefiles(getSavegameFilePattern(target));
	sort(filenames.begin(), filenames.end());   // Sort (hopefully ensuring we are sorted numerically..)

	int slotNum;

	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end();
			++file) {
		// Obtain the last 4 digits of the filename, since they correspond to the save slot
		slotNum = atoi(file->c_str() + file->size() - 4);

		if (slotNum >= 1 && slotNum <= 999) {
			Common::InSaveFile *in = saveMan->openForLoading(*file);
			if (in) {
				if (in->read(saveName, kSaveDescriptionLen) == kSaveDescriptionLen) {
					Common::String desc(saveName);
					int player = (slotNum - 1) / 100;
					if (player >= 0 && player < 5 && !playerNames[player].empty())
						desc = playerNames[player] + ": " + desc;
					saveList.push_back(SaveStateDescriptor(this, slotNum - 1, desc));
				}
				delete in;
			}
		}
	}

	return saveList;
}

bool CryOmni3DMetaEngine::removeSaveState(const char *target, int slot) const {
	return g_system->getSavefileManager()->removeSavefile(getSavegameFile(slot, target));
}

Common::Error CryOmni3DMetaEngine::createInstance(OSystem *syst, Engine **engine,
		const CryOmni3DGameDescription *gd) const {
	switch (gd->gameType) {
	case GType_VERSAILLES:
#ifdef ENABLE_VERSAILLES
		*engine = new Versailles::CryOmni3DEngine_Versailles(syst, gd);
		return Common::kNoError;
#else
		return Common::Error(Common::kUnsupportedGameidError, _s("Versailles 1685 support is not compiled in"));
#endif
	case GType_HNM_PLAYER:
		*engine = new CryOmni3DEngine_HNMPlayer(syst, gd);
		return Common::kNoError;
	case GType_ATLANTIS:
#ifdef ENABLE_ATLANTIS
		*engine = new Atlantis::CryOmni3DEngine_Atlantis(syst, gd);
		return Common::kNoError;
#else
		return Common::Error(Common::kUnsupportedGameidError, _s("Atlantis: The Lost Tale support is not compiled in"));
#endif
	default:
		return Common::kUnsupportedGameidError;
	}
}

} // End of namespace CryOmni3D

#if PLUGIN_ENABLED_DYNAMIC(CRYOMNI3D)
	REGISTER_PLUGIN_DYNAMIC(CRYOMNI3D, PLUGIN_TYPE_ENGINE, CryOmni3D::CryOmni3DMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(CRYOMNI3D, PLUGIN_TYPE_ENGINE, CryOmni3D::CryOmni3DMetaEngine);
#endif
