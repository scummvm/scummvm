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
#include "engines/obsolete.h"

#include "gob/gob.h"

#include "gob/detection/tables.h"

class GobMetaEngine : public AdvancedMetaEngine {
public:
	GobMetaEngine();

	virtual GameDescriptor findGame(const char *gameid) const;

	virtual const ADGameDescription *fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist) const;

	virtual const char *getName() const;
	virtual const char *getOriginalCopyright() const;

	virtual bool hasFeature(MetaEngineFeature f) const;

	virtual Common::Error createInstance(OSystem *syst, Engine **engine) const;
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
};

GobMetaEngine::GobMetaEngine() :
	AdvancedMetaEngine(Gob::gameDescriptions, sizeof(Gob::GOBGameDescription), gobGames) {

	_singleid   = "gob";
	_guioptions = GUIO1(GUIO_NOLAUNCHLOAD);
}

GameDescriptor GobMetaEngine::findGame(const char *gameid) const {
	return Engines::findGameID(gameid, _gameids, obsoleteGameIDsTable);
}

const ADGameDescription *GobMetaEngine::fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist) const {
	ADFilePropertiesMap filesProps;

	const ADGameDescription *game = detectGameFilebased(allFiles, fslist, Gob::fileBased, &filesProps);
	if (!game)
		return 0;

	reportUnknown(fslist.begin()->getParent(), filesProps);
	return game;
}

const char *GobMetaEngine::getName() const {
	return "Gob";
}

const char *GobMetaEngine::getOriginalCopyright() const {
	return "Goblins Games (C) Coktel Vision";
}

bool GobMetaEngine::hasFeature(MetaEngineFeature f) const {
	return false;
}

bool Gob::GobEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsRTL);
}

Common::Error GobMetaEngine::createInstance(OSystem *syst, Engine **engine) const {
	Engines::upgradeTargetIfNecessary(obsoleteGameIDsTable);
	return AdvancedMetaEngine::createInstance(syst, engine);
}

bool GobMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const Gob::GOBGameDescription *gd = (const Gob::GOBGameDescription *)desc;
	if (gd) {
		*engine = new Gob::GobEngine(syst);
		((Gob::GobEngine *)*engine)->initGame(gd);
	}
	return gd != 0;
}


#if PLUGIN_ENABLED_DYNAMIC(GOB)
	REGISTER_PLUGIN_DYNAMIC(GOB, PLUGIN_TYPE_ENGINE, GobMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(GOB, PLUGIN_TYPE_ENGINE, GobMetaEngine);
#endif

namespace Gob {

void GobEngine::initGame(const GOBGameDescription *gd) {
	if (gd->startTotBase == 0)
		_startTot = "intro.tot";
	else
		_startTot = gd->startTotBase;

	if (gd->startStkBase == 0)
		_startStk = "intro.stk";
	else
		_startStk = gd->startStkBase;

	_demoIndex = gd->demoIndex;

	_gameType = gd->gameType;
	_features = gd->features;
	_language = gd->desc.language;
	_platform = gd->desc.platform;
}

} // End of namespace Gob
