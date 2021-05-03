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

#include "kyra/engine/kyra_lok.h"
#include "kyra/engine/lol.h"
#include "kyra/engine/kyra_hof.h"
#include "kyra/engine/kyra_mr.h"
#include "kyra/engine/eob.h"
#include "kyra/engine/darkmoon.h"

#include "common/config-manager.h"
#include "common/system.h"
#include "common/savefile.h"
#include "common/translation.h"

#include "engines/advancedDetector.h"

#include "base/plugins.h"

#include "kyra/detection.h"

class KyraMetaEngine : public AdvancedMetaEngine {
public:
	const char *getName() const override {
		return "kyra";
	}

	bool hasFeature(MetaEngineFeature f) const override;
	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;

	SaveStateList listSaves(const char *target) const override;
	int getMaximumSaveSlot() const override;
	void removeSaveState(const char *target, int slot) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;
	virtual int getAutosaveSlot() const override { return 999; }

	Common::KeymapArray initKeymaps(const char *target) const override;
};

bool KyraMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
	    (f == kSupportsListSaves) ||
	    (f == kSupportsLoadingDuringStartup) ||
	    (f == kSupportsDeleteSave) ||
	    (f == kSavesSupportMetaInfo) ||
	    (f == kSavesSupportThumbnail) ||
		(f == kSimpleSavesNames);
}

bool Kyra::KyraEngine_v1::hasFeature(EngineFeature f) const {
	return
	    (f == kSupportsReturnToLauncher) ||
	    (f == kSupportsLoadingDuringRuntime) ||
	    (f == kSupportsSavingDuringRuntime) ||
	    (f == kSupportsSubtitleOptions);
}

Common::Error KyraMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const KYRAGameDescription *gd = (const KYRAGameDescription *)desc;

	Kyra::GameFlags flags = gd->flags;

	flags.lang = gd->desc.language;
	flags.platform = gd->desc.platform;

	Common::Platform platform = Common::parsePlatform(ConfMan.get("platform"));
	if (platform != Common::kPlatformUnknown)
		flags.platform = platform;

	if (flags.lang == Common::UNK_LANG) {
		Common::Language lang = Common::parseLanguage(ConfMan.get("language"));
		if (lang != Common::UNK_LANG)
			flags.lang = lang;
		else
			flags.lang = Common::EN_ANY;
	}

#ifndef USE_RGB_COLOR
	flags.useHiColorMode = false;
#endif

	switch (flags.gameID) {
	case Kyra::GI_KYRA1:
		*engine = new Kyra::KyraEngine_LoK(syst, flags);
		break;
	case Kyra::GI_KYRA2:
		*engine = new Kyra::KyraEngine_HoF(syst, flags);
		break;
	case Kyra::GI_KYRA3:
		*engine = new Kyra::KyraEngine_MR(syst, flags);
		break;
#ifdef ENABLE_LOL
	case Kyra::GI_LOL:
		*engine = new Kyra::LoLEngine(syst, flags);
		break;
#else
	case Kyra::GI_LOL:
		return Common::Error(Common::kUnsupportedGameidError, _s("Lands of Lore support is not compiled in"));
#endif // ENABLE_LOL
#ifdef ENABLE_EOB
	case Kyra::GI_EOB1:
		*engine = new Kyra::EoBEngine(syst, flags);
		break;
	case Kyra::GI_EOB2:
		 if (Common::parseRenderMode(ConfMan.get("render_mode")) == Common::kRenderEGA)
			 flags.useHiRes = true;
		 if (platform == Common::kPlatformFMTowns && !flags.useHiColorMode)
			 return Common::Error(Common::kUnsupportedColorMode, _s("EOB II FM-TOWNS requires support of 16bit color modes which has not been activated in your ScummVM build"));
		*engine = new Kyra::DarkMoonEngine(syst, flags);
		break;
#else
	case Kyra::GI_EOB1:
	case Kyra::GI_EOB2:
		return Common::Error(Common::kUnsupportedGameidError, _s("Eye of Beholder support is not compiled in"));
#endif // ENABLE_EOB
	default:
		return Common::kUnsupportedGameidError;
	}

	return Common::kNoError;
}

SaveStateList KyraMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Kyra::KyraEngine_v1::SaveHeader header;
	Common::String pattern = target;
	pattern += ".###";

	Common::StringArray filenames;
	filenames = saveFileMan->listSavefiles(pattern);

	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		int slotNum = atoi(file->c_str() + file->size() - 3);

		if (slotNum >= 0 && slotNum <= 999) {
			Common::InSaveFile *in = saveFileMan->openForLoading(*file);
			if (in) {
				if (Kyra::KyraEngine_v1::readSaveHeader(in, header) == Kyra::KyraEngine_v1::kRSHENoError) {
					// WORKAROUND: Old savegames are using 'German' as description for kyra3 restart game save (slot 0),
					// since that looks odd we replace it by "New Game".
					if (slotNum == 0 && header.gameID == Kyra::GI_KYRA3)
						header.description = "New Game";

					saveList.push_back(SaveStateDescriptor(slotNum, header.description));
				}
				delete in;
			}
		}
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

int KyraMetaEngine::getMaximumSaveSlot() const {
	return 999;
}

void KyraMetaEngine::removeSaveState(const char *target, int slot) const {
	// In Kyra games slot 0 can't be deleted, it's for restarting the game(s).
	// An exception makes Lands of Lore here, it does not have any way to restart the
	// game except via its main menu.
	if (slot == 0 && !ConfMan.getDomain(target)->getVal("gameid").equalsIgnoreCase("lol") && !ConfMan.getDomain(target)->getVal("gameid").equalsIgnoreCase("eob") && !ConfMan.getDomain(target)->getVal("gameid").equalsIgnoreCase("eob2"))
		return;

	Common::String filename = Kyra::KyraEngine_v1::getSavegameFilename(target, slot);
	g_system->getSavefileManager()->removeSavefile(filename);
}

SaveStateDescriptor KyraMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String filename = Kyra::KyraEngine_v1::getSavegameFilename(target, slot);
	Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(filename);
	const bool nonKyraGame = ConfMan.getDomain(target)->getVal("gameid").equalsIgnoreCase("lol") || ConfMan.getDomain(target)->getVal("gameid").equalsIgnoreCase("eob") || ConfMan.getDomain(target)->getVal("gameid").equalsIgnoreCase("eob2");

	if (in) {
		Kyra::KyraEngine_v1::SaveHeader header;
		Kyra::KyraEngine_v1::ReadSaveHeaderError error;

		error = Kyra::KyraEngine_v1::readSaveHeader(in, header, false);
		delete in;

		if (error == Kyra::KyraEngine_v1::kRSHENoError) {
			SaveStateDescriptor desc(slot, header.description);

			// Slot 0 is used for the 'restart game' save in all three Kyrandia games, thus
			// we prevent it from being deleted.
			desc.setDeletableFlag(slot != 0 || nonKyraGame);

			// We don't allow quick saves (slot 990 till 998) to be overwritten.
			// The same goes for the 'Autosave', which is slot 999. Slot 0 will also
			// be protected in Kyra 1-3, since it's the 'restart game' save.
			desc.setWriteProtectedFlag((slot == 0 && !nonKyraGame) || slot >= 990);
			desc.setThumbnail(header.thumbnail);

			return desc;
		}
	}

	SaveStateDescriptor desc(slot, Common::String());

	// We don't allow quick saves (slot 990 till 998) to be overwritten.
	// The same goes for the 'Autosave', which is slot 999. Slot 0 will also
	// be protected in Kyra 1-3, since it's the 'restart game' save.
	desc.setWriteProtectedFlag((slot == 0 && !nonKyraGame) || slot >= 990);

	return desc;
}

Common::KeymapArray KyraMetaEngine::initKeymaps(const char *target) const {
	Common::String gameId = ConfMan.get("gameid", target);

#ifdef ENABLE_LOL
	if (gameId.contains("lol")) {
		return Kyra::LoLEngine::initKeymaps();
	}
#endif

#ifdef ENABLE_EOB
	if (gameId.contains("eob")) {
		return Kyra::EoBCoreEngine::initKeymaps(gameId);
	}
#endif

	return AdvancedMetaEngine::initKeymaps(target);
}

#if PLUGIN_ENABLED_DYNAMIC(KYRA)
	REGISTER_PLUGIN_DYNAMIC(KYRA, PLUGIN_TYPE_ENGINE, KyraMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(KYRA, PLUGIN_TYPE_ENGINE, KyraMetaEngine);
#endif
