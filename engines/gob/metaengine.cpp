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
 *
 * This file is dual-licensed.
 * In addition to the GPLv3 license mentioned above, this code is also
 * licensed under LGPL 2.1. See LICENSES/COPYING.LGPL file for the
 * full text of the license.
 *
 */

#include "engines/advancedDetector.h"

#include "common/translation.h"

#include "gob/gameidtotype.h"
#include "gob/gob.h"

// For struct GOBGameDescription.
#include "gob/detection/detection.h"

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_COPY_PROTECTION,
                {
                        _s("Enable copy protection"),
                        _s("Enable any copy protection that would otherwise be bypassed by default."),
                        "copy_protection",
                        false,
                        0,
                        0
                },
        },
	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

class GobMetaEngine : public AdvancedMetaEngine<Gob::GOBGameDescription> {
public:
	const char *getName() const override {
		return "gob";
	}

	bool hasFeature(MetaEngineFeature f) const override;

	Common::Error createInstance(OSystem *syst, Engine **engine, const Gob::GOBGameDescription *desc) const override;

	const ADExtraGuiOptionsMap *getAdvancedExtraGuiOptions() const override {
		return optionsList;
	}
};

bool GobMetaEngine::hasFeature(MetaEngineFeature f) const {
	return false;
}

bool Gob::GobEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher);
}

Common::Error GobMetaEngine::createInstance(OSystem *syst, Engine **engine, const Gob::GOBGameDescription *gd) const {
	*engine = new Gob::GobEngine(syst);
	((Gob::GobEngine *)*engine)->initGame(gd);
	return Common::kNoError;
}


#if PLUGIN_ENABLED_DYNAMIC(GOB)
	REGISTER_PLUGIN_DYNAMIC(GOB, PLUGIN_TYPE_ENGINE, GobMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(GOB, PLUGIN_TYPE_ENGINE, GobMetaEngine);
#endif

namespace Gob {

GameType GobEngine::getGameType(const char *gameId) const {
	const GameIdToType *gameInfo = gameIdToType;

	while (gameInfo->gameId != nullptr) {
		if (!strcmp(gameId, gameInfo->gameId))
			return gameInfo->gameType;
		gameInfo++;
	}

	error("Unknown game ID: %s", gameId);
}

void GobEngine::initGame(const GOBGameDescription *gd) {
	if (gd->startTotBase == nullptr)
		_startTot = "intro.tot";
	else
		_startTot = gd->startTotBase;

	if (gd->startStkBase == nullptr)
		_startStk = "intro.stk";
	else
		_startStk = gd->startStkBase;

	_demoIndex = gd->demoIndex;

	_gameType = getGameType(gd->desc.gameId);
	_features = gd->features;
	_language = gd->desc.language;
	_platform = gd->desc.platform;
	_extra = gd->desc.extra;

	_enableAdibou2FreeBananasWorkaround = gd->desc.flags & GF_ENABLE_ADIBOU2_FREE_BANANAS_WORKAROUND;
	_enableAdibou2FlowersInfiniteLoopWorkaround = gd->desc.flags & GF_ENABLE_ADIBOU2_FLOWERS_INFINITE_LOOP_WORKAROUND;
}

} // End of namespace Gob

