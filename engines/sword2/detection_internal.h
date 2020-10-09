/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1994-1998 Revolution Software Ltd.
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
 */

#ifndef SWORD2_DETECTION_INTERNAL_H
#define SWORD2_DETECTION_INTERNAL_H

#include "engines/metaengine.h"
#include "common/gui_options.h"
#include "sword2/detection.h"

/**
 * The contents of this file are helpful in detecting games, 
 * as well as helping to create an instance of the game.
 */
namespace Sword2 {

struct GameSettings {
	const char *gameid;
	const char *description;
	uint32 features;
	const char *detectname;
};

static const GameSettings sword2_settings[] = {
	/* Broken Sword II */
	{"sword2", "Broken Sword II: The Smoking Mirror", 0, "players.clu" },
	{"sword2alt", "Broken Sword II: The Smoking Mirror (alt)", 0, "r2ctlns.ocx" },
	{"sword2psx", "Broken Sword II: The Smoking Mirror (PlayStation)", 0, "screens.clu"},
	{"sword2psxdemo", "Broken Sword II: The Smoking Mirror (PlayStation/Demo)", Sword2::GF_DEMO, "screens.clu"},
	{"sword2demo", "Broken Sword II: The Smoking Mirror (Demo)", Sword2::GF_DEMO, "players.clu" },
	{"sword2demo-es", "Broken Sword II: The Smoking Mirror (Spanish/Demo)", Sword2::GF_DEMO | Sword2::GF_SPANISHDEMO, "vielogo.tga" },
	{NULL, NULL, 0, NULL}
};

} // End of namespace Sword2

static bool isFullGame(const Common::FSList &fslist) {
	Common::FSList::const_iterator file;

	// We distinguish between the two versions by the presence of paris.clu
	for (file = fslist.begin(); file != fslist.end(); ++file) {
		if (!file->isDirectory()) {
			if (file->getName().equalsIgnoreCase("paris.clu"))
				return true;
		}
	}

	return false;
}

static DetectedGames detectGamesImpl(const Common::FSList &fslist, bool recursion = false) {
	DetectedGames detectedGames;
	const Sword2::GameSettings *g;
	Common::FSList::const_iterator file;
	bool isFullVersion = isFullGame(fslist);

	for (g = Sword2::sword2_settings; g->gameid; ++g) {
		// Iterate over all files in the given directory
		for (file = fslist.begin(); file != fslist.end(); ++file) {
			if (file->isDirectory()) continue;

			if (file->getName().equalsIgnoreCase(g->detectname)) {
				// Make sure that the sword2 demo is not mixed up with the
				// full version, since they use the same filename for detection
				if ((g->features == Sword2::GF_DEMO && isFullVersion) ||
					(g->features == 0 && !isFullVersion))
					continue;

				// Match found, add to list of candidates, then abort inner loop.
				DetectedGame game = DetectedGame("sword2", g->gameid, g->description);
				game.setGUIOptions(GUIO2(GUIO_NOMIDI, GUIO_NOASPECT));

				detectedGames.push_back(game);
				break;
			}
		}
	}


	if (detectedGames.empty() && !recursion) {
		// Nothing found -- try to recurse into the 'clusters' subdirectory,
		// present e.g. if the user copied the data straight from CD.
		for (file = fslist.begin(); file != fslist.end(); ++file) {
			if (file->isDirectory()) {
				if (file->getName().equalsIgnoreCase("clusters")) {
					Common::FSList recList;
					if (file->getChildren(recList, Common::FSNode::kListAll)) {
						DetectedGames recGames = detectGamesImpl(recList, true);
						if (!recGames.empty()) {
							detectedGames.push_back(recGames);
							break;
						}
					}
				}
			}
		}
	}


	return detectedGames;
}

#endif // SWORD2_DETECTION_INTERNAL_H
