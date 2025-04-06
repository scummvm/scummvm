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

#include "ultima/ultima8/misc/common_types.h"
#include "ultima/ultima8/misc/util.h"
#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/usecode/usecode_flex.h"
#include "ultima/ultima8/gfx/main_shape_archive.h"
#include "ultima/ultima8/gfx/fonts/font_shape_archive.h"
#include "ultima/ultima8/gfx/gump_shape_archive.h"
#include "ultima/ultima8/world/map_glob.h"
#include "ultima/ultima8/world/fire_type_table.h"
#include "ultima/ultima8/world/actors/npc_dat.h"
#include "ultima/ultima8/world/actors/combat_dat.h"
#include "ultima/ultima8/gfx/palette_manager.h"
#include "ultima/ultima8/gfx/shape.h"
#include "ultima/ultima8/gfx/wpn_ovlay_dat.h"
#include "ultima/ultima8/gfx/fonts/font_manager.h"
#include "ultima/ultima8/games/game_info.h"
#include "ultima/ultima8/gumps/weasel_dat.h"
#include "ultima/ultima8/conf/config_file_manager.h"
#include "ultima/ultima8/convert/crusader/convert_shape_crusader.h"
#include "ultima/ultima8/audio/music_flex.h"
#include "ultima/ultima8/audio/speech_flex.h"

namespace Ultima {
namespace Ultima8 {

GameData *GameData::_gameData = nullptr;

GameData::GameData(GameInfo *gameInfo)
	: _fixed(nullptr), _mainShapes(nullptr), _mainUsecode(nullptr), _globs(),
	  _fonts(nullptr), _gumps(nullptr), _mouse(nullptr), _music(nullptr),
	  _weaponOverlay(nullptr), _soundFlex(nullptr), _gameInfo(gameInfo) {
	debug(1, "Creating GameData...");

	_gameData = this;
	_speech.resize(1024);
}

GameData::~GameData() {
	debug(1, "Destroying GameData...");

	delete _fixed;
	_fixed = nullptr;

	delete _mainShapes;
	_mainShapes = nullptr;

	delete _mainUsecode;
	_mainUsecode = nullptr;

	for (unsigned int i = 0; i < _globs.size(); ++i)
		delete _globs[i];
	_globs.clear();

	delete _fonts;
	_fonts = nullptr;

	delete _gumps;
	_gumps = nullptr;

	delete _mouse;
	_mouse = nullptr;

	delete _music;
	_music = nullptr;

	delete _weaponOverlay;
	_weaponOverlay = nullptr;

	delete _soundFlex;
	_soundFlex = nullptr;

	for (unsigned int i = 0; i < _npcTable.size(); ++i)
		delete _npcTable[i];
	_npcTable.clear();

	_gameData = nullptr;

	for (unsigned int i = 0; i < _speech.size(); ++i) {
		SpeechFlex **s = _speech[i];
		if (s) delete *s;
		delete s;
	}
	_speech.clear();
}

MapGlob *GameData::getGlob(uint32 glob) const {
	if (glob < _globs.size())
		return _globs[glob];
	else
		return nullptr;
}

ShapeArchive *GameData::getShapeFlex(uint16 flexId) const {
	switch (flexId) {
	case MAINSHAPES:
		return _mainShapes;
	case GUMPS:
		return _gumps;
	default:
		break;
	};
	return nullptr;
}

Shape *GameData::getShape(FrameID f) const {
	ShapeArchive *sf = getShapeFlex(f._flexId);
	if (!sf)
		return nullptr;
	Shape *shape = sf->getShape(f._shapeNum);
	return shape;
}

const ShapeFrame *GameData::getFrame(FrameID f) const {
	const Shape *shape = getShape(f);
	if (!shape)
		return nullptr;
	const ShapeFrame *frame = shape->getFrame(f._frameNum);
	return frame;
}

void GameData::loadTranslation() {
	ConfigFileManager *config = ConfigFileManager::get_instance();
	Common::Path translationfile;

	if (_gameInfo->_type == GameInfo::GAME_U8) {
		switch (_gameInfo->_language) {
		case GameInfo::GAMELANG_ENGLISH:
			// using "translation" to enable bug fixes
			translationfile = "u8english.ini";
			break;
		case GameInfo::GAMELANG_FRENCH:
			translationfile = "u8french.ini";
			break;
		case GameInfo::GAMELANG_GERMAN:
			translationfile = "u8german.ini";
			break;
		case GameInfo::GAMELANG_SPANISH:
			translationfile = "u8spanish.ini";
			break;
		case GameInfo::GAMELANG_JAPANESE:
			translationfile = "u8japanese.ini";
			break;
		default:
			warning("Unknown language.");
			break;
		}
	}

	if (!translationfile.empty()) {
		debug(1, "Loading translation: %s", translationfile.toString().c_str());

		config->readConfigFile(translationfile, "language");
	}
}

Std::string GameData::translate(const Std::string &text) {
	// TODO: maybe cache these lookups? config calls may be expensive
	ConfigFileManager *config = ConfigFileManager::get_instance();
	Std::string trans;
	if (config->get("language", "text", text, trans)) {
		return trans;
	}
	return text;
}

FrameID GameData::translate(FrameID f) {
	// TODO: maybe cache these lookups? config calls may be expensive
	// TODO: add any non-gump shapes when applicable
	// TODO: allow translations to be in another shapeflex

	ConfigFileManager *config = ConfigFileManager::get_instance();
	Std::string category = "language";
	Std::string section;

	switch (f._flexId) {
	case GUMPS:
		section = "gumps";
		break;
	default:
		return f;
	}

	char buf[100];
	Common::sprintf_s(buf, "%d,%d", f._shapeNum, f._frameNum);

	Std::string key = buf;
	Std::string trans;
	if (!config->get(category, section, key, trans)) {
		return f;
	}

	FrameID t;
	t._flexId = f._flexId;
	int n = sscanf(trans.c_str(), "%u,%u", &t._shapeNum, &t._frameNum);
	if (n != 2) {
		warning("Invalid shape translation: %s", trans.c_str());
		return f;
	}

	return t;
}

void GameData::loadU8Data() {
	auto *fd = new Common::File();
	if (!fd->open("static/fixed.dat"))
		error("Unable to load static/fixed.dat");

	_fixed = new RawArchive(fd);

	char langletter = _gameInfo->getLanguageUsecodeLetter();
	if (!langletter)
		error("Unknown language. Unable to open usecode");

	Std::string filename = "usecode/";
	filename += langletter;
	filename += "usecode.flx";


	auto *uds = new Common::File();
	if (!uds->open(Common::Path(filename)))
		error("Unable to load %s", filename.c_str());

	_mainUsecode = new UsecodeFlex(uds);

	// Load main shapes
	debug(1, "Load Shapes");
	auto *sf = new Common::File();
	if (!(sf->open("static/u8shapes.flx") || sf->open("static/u8shapes.cmp")))
		error("Unable to load static/u8shapes.flx or static/u8shapes.cmp");

	_mainShapes = new MainShapeArchive(sf, MAINSHAPES,
	                                  PaletteManager::get_instance()->getPalette(PaletteManager::Pal_Game));

	// Load weapon, armour info
	ConfigFileManager *config = ConfigFileManager::get_instance();
	config->readConfigFile("u8weapons.ini", "weapons");
	config->readConfigFile("u8armour.ini", "armour");
	config->readConfigFile("u8monsters.ini", "monsters");
	config->readConfigFile("u8game.ini", "game");

	// Load typeflags
	auto *tfs = new Common::File();
	if (!tfs->open("static/typeflag.dat"))
		error("Unable to load static/typeflag.dat");

	_mainShapes->loadTypeFlags(tfs);
	delete tfs;

	// Load animdat
	auto *af = new Common::File();
	if (!af->open("static/anim.dat"))
		error("Unable to load static/anim.dat");

	_mainShapes->loadAnimDat(af);
	delete af;

	// Load weapon overlay data
	auto *wod = new Common::File();
	if (!wod->open("static/wpnovlay.dat"))
		error("Unable to load static/wpnovlay.dat");

	RawArchive *overlayflex = new RawArchive(wod);
	_weaponOverlay = new WpnOvlayDat();
	_weaponOverlay->load(overlayflex);
	delete overlayflex;

	// Load _globs
	auto *gds = new Common::File();
	if (!gds->open("static/glob.flx"))
		error("Unable to load static/glob.flx");

	RawArchive *globflex = new RawArchive(gds);
	_globs.clear();
	_globs.resize(globflex->getCount());
	for (unsigned int i = 0; i < globflex->getCount(); ++i) {
		MapGlob *glob = 0;
		Common::SeekableReadStream *globrs = globflex->get_datasource(i);

		if (globrs && globrs->size()) {
			glob = new MapGlob();
			glob->read(globrs);
		}
		delete globrs;

		_globs[i] = glob;
	}
	delete globflex;

	// Load fonts
	auto *fds = new Common::File();
	if (!fds->open("static/u8fonts.flx"))
		error("Unable to load static/u8fonts.flx");

	_fonts = new FontShapeArchive(fds, OTHER,
	                             PaletteManager::get_instance()->getPalette(PaletteManager::Pal_Game));
	_fonts->setHVLeads();

	// Load \mouse
	auto *msds = new Common::File();
	if (!msds->open("static/u8mouse.shp"))
		error("Unable to load static/u8mouse.shp");

	_mouse = new Shape(msds, 0);
	_mouse->setPalette(PaletteManager::get_instance()->getPalette(PaletteManager::Pal_Game));
	delete msds;

	auto *gumpds = new Common::File();
	if (!gumpds->open("static/u8gumps.flx"))
		error("Unable to load static/u8gumps.flx");

	_gumps = new GumpShapeArchive(gumpds, GUMPS,
	                             PaletteManager::get_instance()->getPalette(PaletteManager::Pal_Game));

	auto *gumpageds = new Common::File();
	if (!gumpageds->open("static/gumpage.dat"))
		error("Unable to load static/gumpage.dat");

	_gumps->loadGumpage(gumpageds);
	delete gumpageds;


	auto *mf = new Common::File();
	if (!mf->open("sound/music.flx"))
		error("Unable to load sound/music.flx");

	_music = new MusicFlex(mf);

	auto *sndflx = new Common::File();
	if (!sndflx->open("sound/sound.flx"))
		error("Unable to load sound/sound.flx");

	_soundFlex = new SoundFlex(sndflx);

	loadTranslation();
}

void GameData::setupFontOverrides() {
	setupTTFOverrides("game", false);

	if (_gameInfo->_language == GameInfo::GAMELANG_JAPANESE)
		setupJPOverrides();
}

void GameData::setupJPOverrides() {
	ConfigFileManager *config = ConfigFileManager::get_instance();
	FontManager *fontmanager = FontManager::get_instance();
	KeyMap jpkeyvals;

	jpkeyvals = config->listKeyValues("language", "jpfonts");
	for (const auto &i : jpkeyvals) {
		int fontnum = atoi(i._key.c_str());
		const Std::string &fontdesc = i._value;

		Std::vector<Std::string> vals;
		SplitString(fontdesc, ',', vals);
		if (vals.size() != 2) {
			warning("Invalid jpfont override: %s", fontdesc.c_str());
			continue;
		}

		unsigned int jpfontnum = atoi(vals[0].c_str());
		uint32 col32 = strtol(vals[1].c_str(), 0, 0);

		if (!fontmanager->addJPOverride(fontnum, jpfontnum, col32)) {
			warning("failed to setup jpfont override for font %d", fontnum);
		}
	}

	setupTTFOverrides("language", true);
}

void GameData::setupTTFOverrides(const char *category, bool SJIS) {
	ConfigFileManager *config = ConfigFileManager::get_instance();
	FontManager *fontmanager = FontManager::get_instance();
	KeyMap ttfkeyvals;

	bool overridefonts = ConfMan.getBool("font_override");
	if (!overridefonts) return;

	ttfkeyvals = config->listKeyValues(category, "fontoverride");
	for (const auto &i : ttfkeyvals) {
		int fontnum = atoi(i._key.c_str());
		const Std::string &fontdesc = i._value;

		Std::vector<Std::string> vals;
		SplitString(fontdesc, ',', vals);
		if (vals.size() != 4) {
			warning("Invalid ttf override: %s", fontdesc.c_str());
			continue;
		}

		const Common::Path filename(vals[0]);
		int pointsize = atoi(vals[1].c_str());
		uint32 col32 = strtol(vals[2].c_str(), 0, 0);
		int border = atoi(vals[3].c_str());

		if (!fontmanager->addTTFOverride(fontnum, filename, pointsize,
										 col32, border, SJIS)) {
			warning("failed to setup ttf override for font %d", fontnum);
		}
	}
}

SpeechFlex *GameData::getSpeechFlex(uint32 shapeNum) {
	if (shapeNum >= _speech.size())
		return nullptr;

	SpeechFlex **s = _speech[shapeNum];
	if (s)
		return *s;

	char langletter = _gameInfo->getLanguageFileLetter();
	if (!langletter) {
		warning("GameData::getSpeechFlex: Unknown language.");
		return nullptr;
	}

	Common::Path path(Common::String::format("sound/%c%i.flx", langletter, shapeNum));

	auto *sflx = new Common::File();
	if (!sflx->open(path)) {
		delete sflx;
		return nullptr;
	}

	s = new SpeechFlex *;
	*s = new SpeechFlex(sflx);
	_speech[shapeNum] = s;

	return *s;
}

const NPCDat *GameData::getNPCData(uint16 entry) const {
	if (entry < _npcTable.size()) {
		return _npcTable[entry];
	}
	return nullptr;
}

const NPCDat *GameData::getNPCDataForShape(uint16 shapeno) const {
	for (const auto *npcdat : _npcTable) {
		if (npcdat->getShapeNo() == shapeno)
			return npcdat;
	}
	return nullptr;
}

const CombatDat *GameData::getCombatDat(uint16 entry) const {
	if (entry < _combatData.size()) {
		return _combatData[entry];
	}
	return nullptr;
}

const WeaselDat *GameData::getWeaselDat(uint16 entry) const {
	if (entry < _weaselData.size()) {
		return _weaselData[entry];
	}
	return nullptr;
}

const FireType *GameData::getFireType(uint16 type) const {
	return FireTypeTable::get(type);
}

void GameData::loadRemorseData() {
	auto *fd = new Common::File();
	if (!fd->open("static/fixed.dat"))
		error("Unable to load static/fixed.dat");

	_fixed = new RawArchive(fd);

	char langletter = _gameInfo->getLanguageUsecodeLetter();
	if (!langletter)
		error("Unknown language. Unable to open usecode");

	Std::string filename = "usecode/";
	filename += langletter;
	filename += "usecode.flx";

	auto *uds = new Common::File();
	if (!uds->open(filename.c_str()))
		error("Unable to load %s", filename.c_str());

	_mainUsecode = new UsecodeFlex(uds);

	// Load main shapes
	debug(1, "Load Shapes");
	auto *sf = new Common::File();
	if (!sf->open("static/shapes.flx"))
		error("Unable to load static/shapes.flx");

	_mainShapes = new MainShapeArchive(sf, MAINSHAPES,
	                                  PaletteManager::get_instance()->getPalette(PaletteManager::Pal_Game),
	                                  &CrusaderShapeFormat);

	ConfigFileManager *config = ConfigFileManager::get_instance();
	// Load weapon, armour info
	if (_gameInfo->_type == GameInfo::GAME_REMORSE)
		config->readConfigFile("remorseweapons.ini", "weapons");
	else
		config->readConfigFile("regretweapons.ini", "weapons");

	config->readConfigFile("remorsegame.ini", "game");

	// Load typeflags
	auto *tfs = new Common::File();
	if (!tfs->open("static/typeflag.dat"))
		error("Unable to load static/typeflag.dat");

	_mainShapes->loadTypeFlags(tfs);
	delete tfs;

	// Load animdat
	auto *af = new Common::File();
	if (!af->open("static/anim.dat"))
		error("Unable to load static/anim.dat");

	_mainShapes->loadAnimDat(af);
	delete af;

	// Load weapon overlay data
	auto *wod = new Common::File();
	if (!wod->open("static/wpnovlay.dat"))
		error("Unable to load static/wpnovlay.dat");

	RawArchive *overlayflex = new RawArchive(wod);
	_weaponOverlay = new WpnOvlayDat();
	_weaponOverlay->load(overlayflex);
	delete overlayflex;

	// Load globs
	auto *gds = new Common::File();
	if (!gds->open("static/glob.flx"))
		error("Unable to load static/glob.flx");

	RawArchive *globflex = new RawArchive(gds);
	_globs.clear();
	_globs.resize(globflex->getCount());
	for (unsigned int i = 0; i < globflex->getCount(); ++i) {
		MapGlob *glob = 0;
		Common::SeekableReadStream *globrs = globflex->get_datasource(i);

		if (globrs && globrs->size()) {
			glob = new MapGlob();
			glob->read(globrs);
		}
		delete globrs;

		_globs[i] = glob;
	}
	delete globflex;

	// Load fonts
	auto *fds = new Common::File();
	if (!fds->open("static/fonts.flx"))
		error("Unable to load static/fonts.flx");

	_fonts = new FontShapeArchive(fds, OTHER,
	                             PaletteManager::get_instance()->getPalette(PaletteManager::Pal_Game));
	_fonts->setHVLeads();

	// Load mouse
	auto *msds = new Common::File();
	if (!msds->open("static/mouse.shp"))
		error("Unable to load static/mouse.shp");

	_mouse = new Shape(msds, 0);
	_mouse->setPalette(PaletteManager::get_instance()->getPalette(PaletteManager::Pal_Game));
	delete msds;

	auto *gumpds = new Common::File();
	if (!gumpds->open("static/gumps.flx"))
		error("Unable to load static/gumps.flx");

	_gumps = new GumpShapeArchive(gumpds, GUMPS,
		PaletteManager::get_instance()->getPalette(PaletteManager::Pal_Game));

	auto *dtableds = new Common::File();
	if (!dtableds->open("static/dtable.flx"))
		error("Unable to load static/dtable.flx");

	RawArchive *dtableflex = new RawArchive(dtableds);
	_npcTable = NPCDat::load(dtableflex);
	delete dtableflex;

	auto *damageds = new Common::File();
	if (!damageds->open("static/damage.flx"))
		error("Unable to load static/damage.flx");

	RawArchive *damageflex = new RawArchive(damageds);
	if (damageflex->getCount() != 1)
		error("static/damage.flx appears corrupted");

	_mainShapes->loadDamageDat(damageflex->get_datasource(0));

	delete damageflex;

	auto *combatds = new Common::File();
	if (!combatds->open("static/combat.dat"))
		error("Unable to load static/combat.dat");

	RawArchive *combatflex = new RawArchive(combatds);
	_combatData.clear();
	_combatData.resize(combatflex->getCount());
	for (uint32 i = 0; i < combatflex->getCount(); i++) {
		Common::SeekableReadStream *combatflexrs = combatflex->get_datasource(i);

		if (combatflexrs && combatflexrs->size() > 20) {
			_combatData[i] = new CombatDat(*combatflexrs);
		}
		delete combatflexrs;
	}

	delete combatflex;

	auto *stuffds = new Common::File();
	if (!stuffds->open("static/stuff.dat"))
		error("Unable to load static/stuff.dat");

	// Weasel shop data.
	// 14 blocks of 323 bytes, references like W01 and I07
	// (weapon and inventory)
	while (!stuffds->eos()) {
		WeaselDat *data = new WeaselDat(stuffds);
		_weaselData.push_back(data);
	}

	delete stuffds;

	auto *xformpalds = new Common::File();
	if (!xformpalds->open("static/xformpal.dat"))
		error("Unable to load static/xformpal.dat");
	RawArchive *xformpalflex = new RawArchive(xformpalds);

	// TODO: What's in this flex?
	// Object 1: 32 bytes
	// Object 2: 2304 bytes - presumably data for 3 palettes == 768 * 3
	//           almost no low numbers (so not raw palette data, would be missing black..)

	delete xformpalflex;

	// Note: No MusicFlex for Remorse, as the music is all in different AMF files.
	// The remorse_music_process will load them.

	auto *sndflx = new Common::File();
	if (!sndflx->open("sound/sound.flx"))
		error("Unable to load sound/sound.flx");

	_soundFlex = new SoundFlex(sndflx);

	loadTranslation();
}

} // End of namespace Ultima8
} // End of namespace Ultima
