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

#include "mutationofjb/mutationofjb.h"

#include "common/config-manager.h"

#include "engines/metaengine.h"

static const PlainGameDescriptor mutationofjb_setting[] = {
	{"mutationofjb", "Mutation of J.B."},
	{0, 0}
};

class MutationOfJBMetaEngine : public MetaEngine {
public:
	virtual const char *getName() const {
		return "Mutation of J.B.";
	}

	virtual const char *getOriginalCopyright() const {
		return "Mutation of J.B. (C) 1996 RIKI Computer Games";
	}

	virtual PlainGameList getSupportedGames() const {
		PlainGameList games;
		const PlainGameDescriptor *g = mutationofjb_setting;
		while (g->gameId) {
			games.push_back(*g);
			g++;
		}

		return games;
	}

	virtual PlainGameDescriptor findGame(const char *gameid) const {
		const PlainGameDescriptor *g = mutationofjb_setting;
		while (g->gameId) {
			if (0 == scumm_stricmp(gameid, g->gameId))
				return *g;
			g++;
		}
		return PlainGameDescriptor::empty();
	}

	virtual DetectedGames detectGames(const Common::FSList &fslist) const {
		DetectedGames detectedGames;

		for (Common::FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
			if (!file->isDirectory()) {
				const char *gameName = file->getName().c_str();

				if (0 == scumm_stricmp("startup.dat", gameName)) {
					detectedGames.push_back(DetectedGame(mutationofjb_setting[0]));
					break;
				}
			}
		}
		return detectedGames;
	}

	virtual Common::Error createInstance(OSystem *syst, Engine **engine) const {
		assert(syst);
		assert(engine);

		Common::FSList fslist;
		Common::FSNode dir(ConfMan.get("path"));
		if (!dir.getChildren(fslist, Common::FSNode::kListAll)) {
			return Common::kNoGameDataFoundError;
		}

		// Invoke the detector
		Common::String gameid = ConfMan.get("gameid");
		DetectedGames detectedGames = detectGames(fslist);

		for (uint i = 0; i < detectedGames.size(); i++) {
			if (detectedGames[i].gameId == gameid) {
				// At this point you may want to perform additional sanity checks.
				*engine = new MutationOfJB::MutationOfJBEngine(syst);
				return Common::kNoError;
			}
		}

		// Failed to find any game data
		return Common::kNoGameDataFoundError;
	}
};

#if PLUGIN_ENABLED_DYNAMIC(MUTATIONOFJB)
	REGISTER_PLUGIN_DYNAMIC(MUTATIONOFJB, PLUGIN_TYPE_ENGINE, MutationOfJBMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(MUTATIONOFJB, PLUGIN_TYPE_ENGINE, MutationOfJBMetaEngine);
#endif
