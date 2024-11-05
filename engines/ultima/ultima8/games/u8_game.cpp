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

#include "common/config-manager.h"
#include "common/file.h"
#include "common/translation.h"

#include "gui/error.h"

#include "ultima/ultima8/misc/debugger.h"

#include "ultima/ultima8/games/u8_game.h"

#include "ultima/ultima8/gfx/palette_manager.h"
#include "ultima/ultima8/gfx/fade_to_modal_process.h"
#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/gfx/xform_blend.h"
#include "ultima/ultima8/filesys/u8_save_file.h"
#include "ultima/ultima8/world/world.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/world/item_factory.h"
#include "ultima/ultima8/kernel/object_manager.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/gumps/movie_gump.h"
#include "ultima/ultima8/gumps/credits_gump.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/audio/music_process.h"
#include "ultima/ultima8/games/start_u8_process.h"
#include "ultima/ultima8/world/get_object.h"
#include "common/memstream.h"

namespace Ultima {
namespace Ultima8 {

U8Game::U8Game() : Game() {
	// Set some defaults for gameplay-related settings
	ConfMan.registerDefault("endgame", false);
	ConfMan.registerDefault("quotes", false);
	ConfMan.registerDefault("footsteps", true);
	ConfMan.registerDefault("targetedjump", true);
	ConfMan.registerDefault("subtitles", true);
	ConfMan.registerDefault("speech_mute", false);

	const GameInfo *info = Ultima8Engine::get_instance()->getGameInfo();
	if (info->_language == GameInfo::GAMELANG_JAPANESE) {
		ConfMan.registerDefault("talkspeed", 24);
	} else {
		ConfMan.registerDefault("talkspeed", 60);
	}
}

U8Game::~U8Game() {
}

bool U8Game::loadFiles() {
	// Load palette
	debug(1, "Load Palette");
	Common::File pf;
	if (!pf.open("static/u8pal.pal")) {
		warning("Unable to load static/u8pal.pal.");
		return false;
	}
	pf.seek(4); // seek past header

	Common::MemoryReadStream xfds(U8XFormPal, 1024);
	PaletteManager::get_instance()->load(PaletteManager::Pal_Game, pf, xfds);

	debug(1, "Load GameData");
	GameData::get_instance()->loadU8Data();

	return true;
}

bool U8Game::startGame() {
	// NOTE: assumes the entire engine has been reset!
	debug(1, "Starting new Ultima 8 game.");

	ObjectManager *objman = ObjectManager::get_instance();

	// reserve a number of objids just in case we'll need them sometime
	for (uint16 i = 384; i < 512; ++i)
		objman->reserveObjId(i);

	// Reserved for the Guardian Bark hack
	objman->reserveObjId(kGuardianId);

	auto *savers = new Common::File();
	if (!savers->open("savegame/u8save.000")) {
		Common::U32String errmsg = _(
			"Missing Required File\n\n"
			"Starting a game requires SAVEGAME/U8SAVE.000\n"
			"from an original installation.\n\n"
			"Please check you have copied all the files correctly.");
		::GUI::displayErrorDialog(errmsg);
		delete savers;
		error("Unable to load savegame/u8save.000");
		return false;
	}
	U8SaveFile *u8save = new U8SaveFile(savers);

	Common::SeekableReadStream *nfd = u8save->createReadStreamForMember("NONFIXED.DAT");
	if (!nfd) {
		warning("Unable to load savegame/u8save.000/NONFIXED.DAT.");
		return false;
	}
	World::get_instance()->loadNonFixed(nfd); // deletes nfd

	Common::SeekableReadStream *icd = u8save->createReadStreamForMember("ITEMCACH.DAT");
	if (!icd) {
		warning("Unable to load savegame/u8save.000/ITEMCACH.DAT.");
		return false;
	}
	Common::SeekableReadStream *npcd = u8save->createReadStreamForMember("NPCDATA.DAT");
	if (!npcd) {
		warning("Unable to load savegame/u8save.000/NPCDATA.DAT.");
		delete icd;
		return false;
	}

	World::get_instance()->loadItemCachNPCData(icd, npcd); // deletes icd, npcd
	delete u8save;

	MainActor *av = getMainActor();
	assert(av);

	av->setName("Avatar"); // default name

	// avatar needs a backpack ... CONSTANTs and all that
	Item *backpack = ItemFactory::createItem(529, 0, 0, 0, 0, 0, 0, true);
	backpack->moveToContainer(av);

	World::get_instance()->switchMap(av->getMapNum());

	Ultima8Engine::get_instance()->setAvatarInStasis(true);

	return true;
}

bool U8Game::startInitialUsecode(int saveSlot) {
	Process *proc = new StartU8Process(saveSlot);
	Kernel::get_instance()->addProcess(proc);

	return true;
}


ProcId U8Game::playIntroMovie(bool fade) {
	const GameInfo *gameinfo = Ultima8Engine::get_instance()->getGameInfo();
	char langletter = gameinfo->getLanguageFileLetter();
	if (!langletter) {
		warning("U8Game::playIntro: Unknown language.");
		return 0;
	}

	Common::String filename = "static/";
	filename += langletter;
	filename += "intro.skf";

	auto *skf = new Common::File();
	if (!skf->open(filename.c_str())) {
		debug(1, "U8Game::playIntro: movie not found.");
		delete skf;
		return 0;
	}

	return MovieGump::U8MovieViewer(skf, fade, true, true);
}

ProcId U8Game::playEndgameMovie(bool fade) {
	static const Common::Path filename = "static/endgame.skf";
	auto *skf = new Common::File();
	if (!skf->open(filename)) {
		debug(1, "U8Game::playEndgame: movie not found.");
		delete skf;
		return 0;
	}

	return MovieGump::U8MovieViewer(skf, fade, false, true);
}

void U8Game::playCredits() {
	const GameInfo *gameinfo = Ultima8Engine::get_instance()->getGameInfo();
	char langletter = gameinfo->getLanguageFileLetter();
	if (!langletter) {
		warning("U8Game::playCredits: Unknown language.");
		return;
	}

	Common::String filename = "static/";
	filename += langletter;
	filename += "credits.dat";

	auto *rs = new Common::File();
	if (!rs->open(filename.c_str())) {
		warning("U8Game::playCredits: error opening credits file: %s", filename.c_str());
		delete rs;
		return;
	}
	Std::string text = getCreditText(rs);
	delete rs;

	MusicProcess *musicproc = MusicProcess::get_instance();
	if (musicproc) musicproc->playMusic(51); // CONSTANT!

	CreditsGump *gump = new CreditsGump(text);
	gump->SetFlagWhenFinished("quotes");
	FadeToModalProcess *p = new FadeToModalProcess(gump);
	Kernel::get_instance()->addProcess(p);
}

void U8Game::playQuotes() {
	static const Common::Path filename = "static/quotes.dat";

	auto *rs = new Common::File();
	if (!rs->open(filename)) {
		warning("U8Game::playQuotes: error opening quotes file: %s", filename.toString().c_str());
		delete rs;
		return;
	}
	const Std::string text = getCreditText(rs);
	delete rs;

	MusicProcess *musicproc = MusicProcess::get_instance();
	if (musicproc) musicproc->playMusic(113); // CONSTANT!

	CreditsGump *gump = new CreditsGump(text, 80);
	FadeToModalProcess *p = new FadeToModalProcess(gump);
	Kernel::get_instance()->addProcess(p);
}


void U8Game::writeSaveInfo(Common::WriteStream *ws) {
	MainActor *av = getMainActor();

	const Std::string &avname = av->getName();
	const uint8 namelength = static_cast<uint8>(avname.size());
	ws->writeByte(namelength);
	for (unsigned int i = 0; i < namelength; ++i)
		ws->writeByte(static_cast<uint8>(avname[i]));

	Point3 pt = av->getLocation();
	ws->writeUint16LE(av->getMapNum());
	ws->writeUint32LE(static_cast<uint32>(pt.x));
	ws->writeUint32LE(static_cast<uint32>(pt.y));
	ws->writeUint32LE(static_cast<uint32>(pt.z));

	ws->writeUint16LE(av->getStr());
	ws->writeUint16LE(av->getInt());
	ws->writeUint16LE(av->getDex());
	ws->writeUint16LE(av->getHP());
	ws->writeUint16LE(av->getMaxHP());
	ws->writeUint16LE(av->getMana());
	ws->writeUint16LE(av->getMaxMana());
	ws->writeUint16LE(av->getArmourClass());
	ws->writeUint16LE(av->getTotalWeight());

	for (unsigned int i = 1; i <= 6; i++) {
		uint16 objid = av->getEquip(i);
		Item *item = getItem(objid);
		if (item) {
			ws->writeUint32LE(item->getShape());
			ws->writeUint32LE(item->getFrame());
		} else {
			ws->writeUint32LE(0);
			ws->writeUint32LE(0);
		}
	}
}

Std::string U8Game::getCreditText(Common::SeekableReadStream *rs) {
	Std::string text;
	unsigned int size = rs->size();
	text.resize(size);
	for (unsigned int i = 0; i < size; ++i) {
		uint8 c = rs->readByte();
		int x;
		switch (i) {
		case 0:
		case 1:
			x = 0;
			break;
		case 2:
			x = 0xE1;
			break;
		default:
			x = 0x20 * (i + 1) + (i >> 1);
			x += (i % 0x40) * ((i & 0xC0) >> 6) * 0x40;
			break;
		}
		char d = (c ^ x) & 0xFF;
		if (d == 0) d = '\n';
		text[i] = d;
	}

	return text;
}

} // End of namespace Ultima8
} // End of namespace Ultima
