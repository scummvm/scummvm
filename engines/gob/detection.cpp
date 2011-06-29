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

namespace Gob {

struct GOBGameDescription {
	ADGameDescription desc;

	GameType gameType;
	int32 features;
	const char *startStkBase;
	const char *startTotBase;
	uint32 demoIndex;
};

}

using namespace Common;

static const PlainGameDescriptor gobGames[] = {
	{"gob", "Gob engine game"},
	{"gob1", "Gobliiins"},
	{"gob1cd", "Gobliiins CD"},
	{"gob2", "Gobliins 2"},
	{"gob2cd", "Gobliins 2 CD"},
	{"ween", "Ween: The Prophecy"},
	{"bargon", "Bargon Attack"},
	{"littlered", "Little Red Riding Hood"},
	{"ajworld", "A.J's World of Discovery"},
	{"gob3", "Goblins Quest 3"},
	{"gob3cd", "Goblins Quest 3 CD"},
	{"lit1", "Lost in Time Part 1"},
	{"lit2", "Lost in Time Part 2"},
	{"lit", "Lost in Time"},
	{"inca2", "Inca II: Wiracocha"},
	{"woodruff", "The Bizarre Adventures of Woodruff and the Schnibble"},
	{"dynasty", "The Last Dynasty"},
	{"urban", "Urban Runner"},
	{"playtoons1", "Playtoons 1 - Uncle Archibald"},
	{"playtoons2", "Playtoons 2 - The Case of the Counterfeit Collaborator"},
	{"playtoons3", "Playtoons 3 - The Secret of the Castle"},
	{"playtoons4", "Playtoons 4 - The Mandarine Prince"},
	{"playtoons5", "Playtoons 5 - The Stone of Wakan"},
	{"playtnck1", "Playtoons Construction Kit 1 - Monsters"},
	{"playtnck2", "Playtoons Construction Kit 2 - Knights"},
	{"playtnck3", "Playtoons Construction Kit 3 - Far West"},
	{"bambou", "Playtoons Limited Edition - Bambou le sauveur de la jungle"},
	{"fascination", "Fascination"},
	{"geisha", "Geisha"},
	{"adi2", "ADI 2"},
	{"adi4", "ADI 4"},
	{"adibou2", "Adibou 2"},
	{"adibou1", "Adibou 1"},
	{0, 0}
};

static const Engines::ObsoleteGameID obsoleteGameIDsTable[] = {
	{"gob1", "gob", kPlatformUnknown},
	{"gob2", "gob", kPlatformUnknown},
	{0, 0, kPlatformUnknown}
};

#include "gob/detection_tables.h"

class GobMetaEngine : public AdvancedMetaEngine {
public:
	GobMetaEngine() : AdvancedMetaEngine(Gob::gameDescriptions, sizeof(Gob::GOBGameDescription), gobGames) {
		_singleid = "gob";
		_guioptions = Common::GUIO_NOLAUNCHLOAD;
	}

	virtual GameDescriptor findGame(const char *gameid) const {
		return Engines::findGameID(gameid, _gameids, obsoleteGameIDsTable);
	}

	virtual const ADGameDescription *fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist) const {
		return detectGameFilebased(allFiles, Gob::fileBased);
	}

	virtual const char *getName() const {
		return "Gob";
	}

	virtual const char *getOriginalCopyright() const {
		return "Goblins Games (C) Coktel Vision";
	}

	virtual bool hasFeature(MetaEngineFeature f) const;

	virtual Common::Error createInstance(OSystem *syst, Engine **engine) const {
		Engines::upgradeTargetIfNecessary(obsoleteGameIDsTable);
		return AdvancedMetaEngine::createInstance(syst, engine);
	}
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
};

bool GobMetaEngine::hasFeature(MetaEngineFeature f) const {
	return false;
}

bool Gob::GobEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsRTL);
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
