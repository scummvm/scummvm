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

#include "ultima8/misc/pent_include.h"

#include "ultima8/misc/util.h"
#include "ultima8/games/game_data.h"
#include "ultima8/filesys/file_system.h"
#include "ultima8/filesys/idata_source.h"
#include "ultima8/usecode/usecode_flex.h"
#include "ultima8/graphics/main_shape_archive.h"
#include "ultima8/graphics/fonts/font_shape_archive.h"
#include "ultima8/graphics/gump_shape_archive.h"
#include "ultima8/filesys/raw_archive.h"
#include "ultima8/world/map_glob.h"
#include "ultima8/graphics/palette_manager.h"
#include "ultima8/graphics/shape.h"
#include "ultima8/graphics/wpn_ovlay_dat.h"
#include "ultima8/kernel/core_app.h"
#include "ultima8/conf/config_file_manager.h"
#include "ultima8/graphics/fonts/font_manager.h"
#include "ultima8/games/game_info.h"
#include "ultima8/conf/setting_manager.h"
#include "crusader/ConvertShapeCrusader.h"
#include "ultima8/audio/music_flex.h"
#include "ultima8/audio/sound_flex.h"
#include "SpeechFlex.h"

namespace Ultima8 {

GameData *GameData::gamedata = 0;


GameData::GameData(GameInfo *gameinfo_)
	: fixed(0), mainshapes(0), mainusecode(0), globs(0), fonts(0), gumps(0),
	  mouse(0), music(0), weaponoverlay(0), soundflex(0), speech(1024), gameinfo(gameinfo_) {
	con.Print(MM_INFO, "Creating GameData...\n");

	assert(gamedata == 0);
	gamedata = this;
}

GameData::~GameData() {
	con.Print(MM_INFO, "Destroying GameData...\n");

	delete fixed;
	fixed = 0;

	delete mainshapes;
	mainshapes = 0;

	delete mainusecode;
	mainusecode = 0;

	for (unsigned int i = 0; i < globs.size(); ++i)
		delete globs[i];
	globs.clear();

	delete fonts;
	fonts = 0;

	delete gumps;
	gumps = 0;

	delete mouse;
	mouse = 0;

	delete music;
	music = 0;

	delete weaponoverlay;
	weaponoverlay = 0;

	delete soundflex;
	soundflex = 0;

	gamedata = 0;

	for (unsigned int i = 0; i < speech.size(); ++i) {
		SpeechFlex **s = speech[i];
		if (s) delete *s;
		delete s;
	}
	speech.clear();
}

MapGlob *GameData::getGlob(uint32 glob) const {
	if (glob < globs.size())
		return globs[glob];
	else
		return 0;
}

ShapeArchive *GameData::getShapeFlex(uint16 flexId) const {
	switch (flexId) {
	case MAINSHAPES:
		return mainshapes;
	case GUMPS:
		return gumps;
	default:
		break;
	};
	return 0;
}

Shape *GameData::getShape(FrameID f) const {
	ShapeArchive *sf = getShapeFlex(f.flexid);
	if (!sf) return 0;
	Shape *shape = sf->getShape(f.shapenum);
	return shape;
}

ShapeFrame *GameData::getFrame(FrameID f) const {
	Shape *shape = getShape(f);
	if (!shape) return 0;
	ShapeFrame *frame = shape->getFrame(f.framenum);
	return frame;
}

void GameData::loadTranslation() {
	ConfigFileManager *config = ConfigFileManager::get_instance();
	std::string translationfile;

	if (gameinfo->type == GameInfo::GAME_U8) {
		switch (gameinfo->language) {
		case GameInfo::GAMELANG_ENGLISH:
			// default. Don't need to do anything
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
			perr << "Unknown language." << std::endl;
			break;
		}
	}

	if (!translationfile.empty()) {
		translationfile = "@data/" + translationfile;

		pout << "Loading translation: " << translationfile << std::endl;

		config->readConfigFile(translationfile, "language", true);
	}
}

std::string GameData::translate(std::string text) {
	// TODO: maybe cache these lookups? config calls may be expensive

	ConfigFileManager *config = ConfigFileManager::get_instance();
	Pentagram::istring key = "language/text/" + text;
	if (!config->exists(key))
		return text;

	std::string trans;
	config->get(key, trans);
	return trans;
}

FrameID GameData::translate(FrameID f) {
	// TODO: maybe cache these lookups? config calls may be expensive
	// TODO: add any non-gump shapes when applicable
	// TODO: allow translations to be in another shapeflex

	ConfigFileManager *config = ConfigFileManager::get_instance();
	Pentagram::istring key = "language/";
	switch (f.flexid) {
	case GUMPS:
		key += "gumps/";
		break;
	default:
		return f;
	}

	char buf[100];
	sprintf(buf, "%d,%d", f.shapenum, f.framenum);

	key += buf;
	if (!config->exists(key))
		return f;

	std::string trans;
	config->get(key, trans);

	FrameID t;
	t.flexid = f.flexid;
	int n = sscanf(trans.c_str(), "%d,%d", &t.shapenum, &t.framenum);
	if (n != 2) {
		perr << "Invalid shape translation: " << trans << std::endl;
		return f;
	}

	return t;
}

void GameData::loadU8Data() {
	FileSystem *filesystem = FileSystem::get_instance();

	IDataSource *fd = filesystem->ReadFile("@game/static/fixed.dat");
	if (!fd) {
		perr << "Unable to load static/fixed.dat. Exiting" << std::endl;
		std::exit(-1);
	}
	fixed = new RawArchive(fd);

	char langletter = gameinfo->getLanguageUsecodeLetter();
	if (!langletter) {
		perr << "Unknown language. Unable to open usecode." << std::endl;
		std::exit(-1);
	}
	std::string filename = "@game/usecode/";
	filename += langletter;
	filename += "usecode.flx";


	IDataSource *uds = filesystem->ReadFile(filename);
	if (!uds) {
		perr << "Unable to load " << filename << ". Exiting" << std::endl;
		std::exit(-1);
	}
	mainusecode = new UsecodeFlex(uds);

	// Load main shapes
	pout << "Load Shapes" << std::endl;
	IDataSource *sf = filesystem->ReadFile("@game/static/u8shapes.flx");
	if (!sf) sf = filesystem->ReadFile("@game/static/u8shapes.cmp");

	if (!sf) {
		perr << "Unable to load static/u8shapes.flx or static/u8shapes.cmp. Exiting" << std::endl;
		std::exit(-1);
	}
	mainshapes = new MainShapeArchive(sf, MAINSHAPES,
	                                  PaletteManager::get_instance()->getPalette(PaletteManager::Pal_Game));

	// Load weapon, armour info
	ConfigFileManager *config = ConfigFileManager::get_instance();
	config->readConfigFile("@data/u8weapons.ini", "weapons", true);
	config->readConfigFile("@data/u8armour.ini", "armour", true);
	config->readConfigFile("@data/u8monsters.ini", "monsters", true);
	config->readConfigFile("@data/u8.ini", "game", true);

	// Load typeflags
	IDataSource *tfs = filesystem->ReadFile("@game/static/typeflag.dat");
	if (!tfs) {
		perr << "Unable to load static/typeflag.dat. Exiting" << std::endl;
		std::exit(-1);
	}
	mainshapes->loadTypeFlags(tfs);
	delete tfs;

	// Load animdat
	IDataSource *af = filesystem->ReadFile("@game/static/anim.dat");
	if (!af) {
		perr << "Unable to load static/anim.dat. Exiting" << std::endl;
		std::exit(-1);
	}
	mainshapes->loadAnimDat(af);
	delete af;

	// Load weapon overlay data
	IDataSource *wod = filesystem->ReadFile("@game/static/wpnovlay.dat");
	if (!wod) {
		perr << "Unable to load static/wpnovlay.dat. Exiting" << std::endl;
		std::exit(-1);
	}
	RawArchive *overlayflex = new RawArchive(wod);
	weaponoverlay = new WpnOvlayDat();
	weaponoverlay->load(overlayflex);
	delete overlayflex;

	// Load globs
	IDataSource *gds = filesystem->ReadFile("@game/static/glob.flx");
	if (!gds) {
		perr << "Unable to load static/glob.flx. Exiting" << std::endl;
		std::exit(-1);
	}
	RawArchive *globflex = new RawArchive(gds);
	globs.clear();
	globs.resize(globflex->getCount());
	for (unsigned int i = 0; i < globflex->getCount(); ++i) {
		MapGlob *glob = 0;
		IDataSource *globds = globflex->get_datasource(i);

		if (globds && globds->getSize()) {
			glob = new MapGlob();
			glob->read(globds);
		}
		delete globds;

		globs[i] = glob;
	}
	delete globflex;

	// Load fonts
	IDataSource *fds = filesystem->ReadFile("@game/static/u8fonts.flx");
	if (!fds) {
		perr << "Unable to load static/u8fonts.flx. Exiting" << std::endl;
		std::exit(-1);
	}
	fonts = new FontShapeArchive(fds, OTHER,
	                             PaletteManager::get_instance()->getPalette(PaletteManager::Pal_Game));
	fonts->setHVLeads();

	// Load mouse
	IDataSource *msds = filesystem->ReadFile("@game/static/u8mouse.shp");
	if (!msds) {
		perr << "Unable to load static/u8mouse.shp. Exiting" << std::endl;
		std::exit(-1);
	}
	mouse = new Shape(msds, 0);
	mouse->setPalette(PaletteManager::get_instance()->getPalette(PaletteManager::Pal_Game));
	delete msds;

	IDataSource *gumpds = filesystem->ReadFile("@game/static/u8gumps.flx");
	if (!gumpds) {
		perr << "Unable to load static/u8gumps.flx. Exiting" << std::endl;
		std::exit(-1);
	}
	gumps = new GumpShapeArchive(gumpds, GUMPS,
	                             PaletteManager::get_instance()->getPalette(PaletteManager::Pal_Game));

	IDataSource *gumpageds = filesystem->ReadFile("@game/static/gumpage.dat");
	if (!gumpageds) {
		perr << "Unable to load static/gumpage.dat. Exiting" << std::endl;
		std::exit(-1);
	}
	gumps->loadGumpage(gumpageds);
	delete gumpageds;


	IDataSource *mf = filesystem->ReadFile("@game/sound/music.flx");
	if (!mf) {
		perr << "Unable to load sound/music.flx. Exiting" << std::endl;
		std::exit(-1);
	}
	music = new MusicFlex(mf);

	IDataSource *sndflx = filesystem->ReadFile("@game/sound/sound.flx");
	if (!sndflx) {
		perr << "Unable to load sound/sound.flx. Exiting" << std::endl;
		std::exit(-1);
	}
	soundflex = new SoundFlex(sndflx);

	loadTranslation();
}

void GameData::setupFontOverrides() {
	setupTTFOverrides("game/fontoverride", false);

	if (gameinfo->language == GameInfo::GAMELANG_JAPANESE)
		setupJPOverrides();
}

void GameData::setupJPOverrides() {
	SettingManager *settingman = SettingManager::get_instance();
	ConfigFileManager *config = ConfigFileManager::get_instance();
	FontManager *fontmanager = FontManager::get_instance();
	std::map<Pentagram::istring, std::string> jpkeyvals;
	std::map<Pentagram::istring, std::string>::iterator iter;

	jpkeyvals = config->listKeyValues("language/jpfonts");
	for (iter = jpkeyvals.begin(); iter != jpkeyvals.end(); ++iter) {
		int fontnum = std::atoi(iter->first.c_str());
		std::string fontdesc = iter->_value;

		std::vector<std::string> vals;
		Pentagram::SplitString(fontdesc, ',', vals);
		if (vals.size() != 2) {
			perr << "Invalid jpfont override: " << fontdesc << std::endl;
			continue;
		}

		unsigned int jpfontnum = std::atoi(vals[0].c_str());
		uint32 col32 = std::strtol(vals[1].c_str(), 0, 0);

		if (!fontmanager->addJPOverride(fontnum, jpfontnum, col32)) {
			perr << "failed to setup jpfont override for font " << fontnum
			     << std::endl;
		}
	}

	bool ttfoverrides = false;
	settingman->get("ttf", ttfoverrides);
	if (ttfoverrides)
		setupTTFOverrides("language/fontoverride", true);
}

void GameData::setupTTFOverrides(const char *configkey, bool SJIS) {
	ConfigFileManager *config = ConfigFileManager::get_instance();
	SettingManager *settingman = SettingManager::get_instance();
	FontManager *fontmanager = FontManager::get_instance();
	std::map<Pentagram::istring, std::string> ttfkeyvals;
	std::map<Pentagram::istring, std::string>::iterator iter;

	bool ttfoverrides = false;
	settingman->get("ttf", ttfoverrides);
	if (!ttfoverrides) return;

	ttfkeyvals = config->listKeyValues(configkey);
	for (iter = ttfkeyvals.begin(); iter != ttfkeyvals.end(); ++iter) {
		int fontnum = std::atoi(iter->first.c_str());
		std::string fontdesc = iter->_value;

		std::vector<std::string> vals;
		Pentagram::SplitString(fontdesc, ',', vals);
		if (vals.size() != 4) {
			perr << "Invalid ttf override: " << fontdesc << std::endl;
			continue;
		}

		std::string filename = vals[0];
		int pointsize = std::atoi(vals[1].c_str());
		uint32 col32 = std::strtol(vals[2].c_str(), 0, 0);
		int border = std::atoi(vals[3].c_str());

		if (!fontmanager->addTTFOverride(fontnum, filename, pointsize,
		                                 col32, border, SJIS)) {
			perr << "failed to setup ttf override for font " << fontnum
			     << std::endl;
		}
	}
}

SpeechFlex *GameData::getSpeechFlex(uint32 shapenum) {
	if (shapenum >= speech.size()) return 0;

	SpeechFlex **s = speech[shapenum];
	if (s) return *s;

	s = new SpeechFlex*;
	*s = 0;

	FileSystem *filesystem = FileSystem::get_instance();

	std::string u8_sound_ = "@game/sound/";
	char num_flx [32];
	snprintf(num_flx , 32, "%i.flx", shapenum);

	char langletter = gameinfo->getLanguageFileLetter();
	if (!langletter) {
		perr << "GameData::getSpeechFlex: Unknown language." << std::endl;
		return 0;
	}

	IDataSource *sflx = filesystem->ReadFile(u8_sound_ + langletter + num_flx);
	if (sflx) {
		*s = new SpeechFlex(sflx);
	}

	speech[shapenum] = s;

	return *s;
}


void GameData::loadRemorseData() {
	FileSystem *filesystem = FileSystem::get_instance();

	IDataSource *fd = filesystem->ReadFile("@game/static/fixed.dat");
	if (!fd) {
		perr << "Unable to load static/fixed.dat. Exiting" << std::endl;
		std::exit(-1);
	}
	fixed = new RawArchive(fd);

	char langletter = gameinfo->getLanguageUsecodeLetter();
	if (!langletter) {
		perr << "Unknown language. Unable to open usecode." << std::endl;
		std::exit(-1);
	}
	std::string filename = "@game/usecode/";
	filename += langletter;
	filename += "usecode.flx";


	IDataSource *uds = filesystem->ReadFile(filename);
	if (!uds) {
		perr << "Unable to load " << filename << ". Exiting" << std::endl;
		std::exit(-1);
	}
	mainusecode = new UsecodeFlex(uds);

	// Load main shapes
	pout << "Load Shapes" << std::endl;
	IDataSource *sf = filesystem->ReadFile("@game/static/shapes.flx");

	if (!sf) {
		perr << "Unable to load static/shapes.flx. Exiting" << std::endl;
		std::exit(-1);
	}
	mainshapes = new MainShapeArchive(sf, MAINSHAPES,
	                                  PaletteManager::get_instance()->getPalette(PaletteManager::Pal_Game),
	                                  &CrusaderShapeFormat);

	ConfigFileManager *config = ConfigFileManager::get_instance();
#if 0
	// Load weapon, armour info
	config->readConfigFile("@data/u8weapons.ini", "weapons", true);
	config->readConfigFile("@data/u8armour.ini", "armour", true);
	config->readConfigFile("@data/u8monsters.ini", "monsters", true);
#endif
	config->readConfigFile("@data/remorse.ini", "game", true);

	// Load typeflags
	IDataSource *tfs = filesystem->ReadFile("@game/static/typeflag.dat");
	if (!tfs) {
		perr << "Unable to load static/typeflag.dat. Exiting" << std::endl;
		std::exit(-1);
	}
	mainshapes->loadTypeFlags(tfs);
	delete tfs;

	// Load animdat
	IDataSource *af = filesystem->ReadFile("@game/static/anim.dat");
	if (!af) {
		perr << "Unable to load static/anim.dat. Exiting" << std::endl;
		std::exit(-1);
	}
	mainshapes->loadAnimDat(af);
	delete af;

	// Load weapon overlay data
	IDataSource *wod = filesystem->ReadFile("@game/static/wpnovlay.dat");
	if (!wod) {
		perr << "Unable to load static/wpnovlay.dat. Exiting" << std::endl;
		std::exit(-1);
	}
	RawArchive *overlayflex = new RawArchive(wod);
	weaponoverlay = new WpnOvlayDat();
	weaponoverlay->load(overlayflex);
	delete overlayflex;

	// Load globs
	IDataSource *gds = filesystem->ReadFile("@game/static/glob.flx");
	if (!gds) {
		perr << "Unable to load static/glob.flx. Exiting" << std::endl;
		std::exit(-1);
	}
	RawArchive *globflex = new RawArchive(gds);
	globs.clear();
	globs.resize(globflex->getCount());
	for (unsigned int i = 0; i < globflex->getCount(); ++i) {
		MapGlob *glob = 0;
		IDataSource *globds = globflex->get_datasource(i);

		if (globds && globds->getSize()) {
			glob = new MapGlob();
			glob->read(globds);
		}
		delete globds;

		globs[i] = glob;
	}
	delete globflex;

	// Load fonts
	IDataSource *fds = filesystem->ReadFile("@game/static/fonts.flx");
	if (!fds) {
		perr << "Unable to load static/fonts.flx. Exiting" << std::endl;
		std::exit(-1);
	}
	fonts = new FontShapeArchive(fds, OTHER,
	                             PaletteManager::get_instance()->getPalette(PaletteManager::Pal_Game));
	fonts->setHVLeads();

	// Load mouse
	IDataSource *msds = filesystem->ReadFile("@game/static/mouse.shp");
	if (!msds) {
		perr << "Unable to load static/mouse.shp. Exiting" << std::endl;
		std::exit(-1);
	}
	mouse = new Shape(msds, 0);
	mouse->setPalette(PaletteManager::get_instance()->getPalette(PaletteManager::Pal_Game));
	delete msds;

	IDataSource *gumpds = filesystem->ReadFile("@game/static/gumps.flx");
	if (!gumpds) {
		perr << "Unable to load static/gumps.flx. Exiting" << std::endl;
		std::exit(-1);
	}
	gumps = new GumpShapeArchive(gumpds, GUMPS,
	                             PaletteManager::get_instance()->getPalette(PaletteManager::Pal_Game));

#if 0
	IDataSource *gumpageds = filesystem->ReadFile("@game/static/gumpage.dat");
	if (!gumpageds) {
		perr << "Unable to load static/gumpage.dat. Exiting" << std::endl;
		std::exit(-1);
	}
	gumps->loadGumpage(gumpageds);
	delete gumpageds;
#endif

	IDataSource *dummyds = filesystem->ReadFile("@data/empty.flx");
	music = 0; //new MusicFlex(dummyds);
	delete dummyds;
#if 0
	IDataSource *mf = filesystem->ReadFile("@game/sound/music.flx");
	if (!mf) {
		perr << "Unable to load sound/music.flx. Exiting" << std::endl;
		std::exit(-1);
	}
	music = new MusicFlex(mf);
#endif

	dummyds = filesystem->ReadFile("@data/empty.flx");
	soundflex = new SoundFlex(dummyds);
	delete dummyds;
#if 0
	IDataSource *sndflx = filesystem->ReadFile("@game/sound/sound.flx");
	if (!sndflx) {
		perr << "Unable to load sound/sound.flx. Exiting" << std::endl;
		std::exit(-1);
	}
	soundflex = new SoundFlex(sndflx);
#endif

	loadTranslation();
}

} // End of namespace Ultima8
