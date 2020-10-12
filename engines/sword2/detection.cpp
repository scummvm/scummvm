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

#include "common/translation.h"

#include "engines/metaengine.h"

#include "sword2/detection.h"
#include "sword2/detection_internal.h"

static const ExtraGuiOption sword2ExtraGuiOption = {
	_s("Show object labels"),
	_s("Show labels for objects on mouse hover"),
	"object_labels",
	false
};

class Sword2MetaEngineDetection : public MetaEngineDetection {
public:
	const char *getEngineId() const override {
		return "sword2";
	}

	const char *getName() const override {
		return "Broken Sword II: The Smoking Mirror";
	}
	const char *getOriginalCopyright() const override {
		return "Broken Sword II: The Smoking Mirror (C) Revolution";
	}

	PlainGameList getSupportedGames() const override;
	const ExtraGuiOptions getExtraGuiOptions(const Common::String &target) const override;
	PlainGameDescriptor findGame(const char *gameid) const override;
	DetectedGames detectGames(const Common::FSList &fslist) const override;
};

PlainGameList Sword2MetaEngineDetection::getSupportedGames() const {
	const Sword2::GameSettings *g = Sword2::sword2_settings;
	PlainGameList games;
	while (g->gameid) {
		games.push_back(PlainGameDescriptor::of(g->gameid, g->description));
		g++;
	}
	return games;
}

const ExtraGuiOptions Sword2MetaEngineDetection::getExtraGuiOptions(const Common::String &target) const {
	ExtraGuiOptions options;
	options.push_back(sword2ExtraGuiOption);
	return options;
}

PlainGameDescriptor Sword2MetaEngineDetection::findGame(const char *gameid) const {
	const Sword2::GameSettings *g = Sword2::sword2_settings;
	while (g->gameid) {
		if (0 == scumm_stricmp(gameid, g->gameid))
			break;
		g++;
	}
	return PlainGameDescriptor::of(g->gameid, g->description);
}

DetectedGames Sword2MetaEngineDetection::detectGames(const Common::FSList &fslist) const {
	// The required game data files can be located in the game directory, or in
	// a subdirectory called "clusters". In the latter case, we don't want to
	// detect the game in that subdirectory, as this will detect the game twice
	// when mass add is searching inside a directory. In this case, the first
	// result (the game directory) will be correct, but the second result (the
	// clusters subdirectory) will be wrong, as the optional speech, music and
	// video data files will be ignored. Note that this fix will skip the game
	// data files if the user has placed them inside a "clusters" subdirectory,
	// or if he/she points ScummVM directly to the "clusters" directory of the
	// game CD. Fixes bug #3049336.
	if (!fslist.empty()) {
		Common::String directory = fslist[0].getParent().getName();
		if (directory.hasPrefixIgnoreCase("clusters") && directory.size() <= 9)
			return DetectedGames();
	}

	return detectGamesImpl(fslist);
}

REGISTER_PLUGIN_STATIC(SWORD2_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, Sword2MetaEngineDetection);
