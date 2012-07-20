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
#include "engines/wintermute/base/BPersistMgr.h"

#include "common/config-manager.h"
#include "common/error.h"
#include "common/fs.h"

#include "engines/metaengine.h"

#include "engines/wintermute/detection_tables.h"

class WinterMuteMetaEngine : public AdvancedMetaEngine {
public:
	WinterMuteMetaEngine() : AdvancedMetaEngine(WinterMute::gameDescriptions, sizeof(ADGameDescription), WinterMute::wintermuteGames) {
		_singleid = "wintermute";
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

		*engine = new WinterMute::WinterMuteEngine(syst, desc);
		return true;
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
		WinterMute::CBPersistMgr pm(NULL, target);
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
		return 100;
	}
	
	void removeSaveState(const char *target, int slot) const {
		WinterMute::CBPersistMgr pm(NULL, target);
		pm.deleteSaveSlot(slot);
	}
	
	virtual SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const {
		WinterMute::CBPersistMgr pm(NULL, target);
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
