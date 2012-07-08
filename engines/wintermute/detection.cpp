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

#include "engines/advancedDetector.h"
#include "engines/wintermute/wintermute.h"
#include "engines/wintermute/Base/BPersistMgr.h"

#include "common/config-manager.h"
#include "common/error.h"
#include "common/fs.h"

#include "engines/metaengine.h"

namespace WinterMute {

static const ADGameDescription gameDescriptions[] = {
	{
		"wintermute",
		"Dirty Split",
		AD_ENTRY1s("data.dcp", "8f3dae199361ece0f59fb20cfff6eed3", 88577621),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE |
		ADGF_USEEXTRAASTITLE,
		GUIO0()
	},
	{
		"wintermute",
		"Dirty Split",
		AD_ENTRY1s("data.dcp", "139d8a25579e969f8b37d20e6e3de5f9", 92668291),
		Common::DE_DEU,
		Common::kPlatformWindows,
		ADGF_UNSTABLE |
		ADGF_USEEXTRAASTITLE,
		GUIO0()
	},
	{
		"wintermute",
		"Five Magical Amulets",
		AD_ENTRY1s("data.dcp", "0134e92bcd5fd2837df3971087e96067", 163316498),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE |
		ADGF_USEEXTRAASTITLE,
		GUIO0()
	},
	{
		"wintermute",
		"Rosemary",
		AD_ENTRY1s("data.dcp", "4f2631138bd4d27587d9043f8aeff3df", 29483643),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE |
		ADGF_USEEXTRAASTITLE,
		GUIO0()
	},
	{
		"wintermute",
		"J.U.L.I.A. Demo",
		AD_ENTRY1s("data.dcp", "f0bbc3394555a9811f6050dae428cab6", 7655237),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE |
		ADGF_USEEXTRAASTITLE |
		ADGF_DEMO,
		GUIO0()
	},
	{
		"wintermute",
		"Five Lethal Demons",
		AD_ENTRY1s("data.dcp", "1037a77cbd001e0644898addc022322c", 15407750),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE |
		ADGF_USEEXTRAASTITLE,
		GUIO0()
	},
	{
		"wintermute",
		"Pigeons in the Park",
		AD_ENTRY1s("data.dcp", "9143a5b6ff8206aefe3c4c643add3ec7", 2611100),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE |
		ADGF_USEEXTRAASTITLE,
		GUIO0()
	},
	{
		"wintermute",
		"the white chamber",
		AD_ENTRY1s("data.dcp", "0011d01142547c61e51ba24dc42b579e", 186451273),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE |
		ADGF_USEEXTRAASTITLE,
		GUIO0()
	},
	AD_TABLE_END_MARKER
};

}

static const PlainGameDescriptor wintermuteGames[] = {
	{ "wintermute", "Wintermute - Unspecified game" },
	{ 0, 0 }
};

class WinterMuteMetaEngine : public AdvancedMetaEngine {
public:
	WinterMuteMetaEngine() : AdvancedMetaEngine(WinterMute::gameDescriptions, sizeof(ADGameDescription), wintermuteGames) {
	}
	virtual const char *getName() const {
		return "Wintermute";
	}

	virtual const char *getOriginalCopyright() const {
		return "Copyright (c) 2011 Jan Nedoma";
	}

/*	virtual GameList getSupportedGames() const {
		GameList games;
		const PlainGameDescriptor *g = wintermuteGames;
		while (g->gameid) {
			games.push_back(*g);
			g++;
		}

		return games;
	}

	virtual GameDescriptor findGame(const char *gameid) const {
		const PlainGameDescriptor *g = wintermuteGames;
		while (g->gameid) {
			if (0 == scumm_stricmp(gameid, g->gameid))
				break;
			g++;
		}
		return GameDescriptor(g->gameid, g->description);
	}*/

/*	virtual GameList detectGames(const Common::FSList &fslist) const {
		GameList detectedGames;

		// Iterate over all files in the given directory
		for (Common::FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
			if (!file->isDirectory()) {
				const char *gameName = file->getName().c_str();

				if (0 == scumm_stricmp("data.dcp", gameName)) {
					// You could check the contents of the file now if you need to.
					detectedGames.push_back(WinterMute_setting[0]);
					break;
				}
			}
		}
		return detectedGames;
	}*/
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
		assert(syst);
		assert(engine);

		// Scan the target directory for files (error out if it does not exist)
		Common::FSList fslist;
		Common::FSNode dir(ConfMan.get("path"));
		if (!dir.getChildren(fslist, Common::FSNode::kListAll)) {
			return Common::kNoGameDataFoundError;
		}

		// Invoke the detector
		Common::String gameid = ConfMan.get("gameid");
		GameList detectedGames = detectGames(fslist);

		for (uint i = 0; i < detectedGames.size(); i++) {
			if (detectedGames[i].gameid() == gameid) {
				// At this point you may want to perform additional sanity checks.
				*engine = new WinterMute::WinterMuteEngine(syst);
				return true;
			}
		}

		// Failed to find any game data
		return false;
	}

	bool hasFeature(MetaEngineFeature f) const {
		switch (f) {
			case MetaEngine::kSupportsListSaves:
				return true;
			case MetaEngine::kSupportsLoadingDuringStartup:
				return true;
			case MetaEngine::kSupportsDeleteSave:
				return true;
			case MetaEngine::kSavesSupportCreationDate:
				return true;
			case MetaEngine::kSavesSupportMetaInfo:			
				return true;
			case MetaEngine::kSavesSupportThumbnail:			
				return true;
			default:
				return false;
		}
	}

	SaveStateList listSaves(const char *target) const {
		SaveStateList saves;
		WinterMute::CBPersistMgr pm;
		for (int i = 0; i < getMaximumSaveSlot(); i++) {
			if (pm.getSaveExists(i)) {
				SaveStateDescriptor desc;
				pm.getSaveStateDesc(i, desc);
				saves.push_back(desc);
			}
		}
		return saves;
	}
	
	int getMaximumSaveSlot() const {
		WinterMute::CBPersistMgr pm;
		return pm.getMaxUsedSlot() + 1; // TODO: Since we use slot 0, this misses a bit.
	}
	
	void removeSaveState(const char *target, int slot) const {
		WinterMute::CBPersistMgr pm;
		pm.deleteSaveSlot(slot);
	}
	
	virtual SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const {
		WinterMute::CBPersistMgr pm;
		SaveStateDescriptor retVal;
		retVal.setDescription("Invalid savegame");
		pm.getSaveStateDesc(slot, retVal);
		return retVal;
	}
};

#if PLUGIN_ENABLED_DYNAMIC(WINTERMUTE)
REGISTER_PLUGIN_DYNAMIC(WINTERMUTE, PLUGIN_TYPE_ENGINE, WinterMuteMetaEngine);
#else
REGISTER_PLUGIN_STATIC(WINTERMUTE, PLUGIN_TYPE_ENGINE, WinterMuteMetaEngine);
#endif
