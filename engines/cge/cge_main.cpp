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

/*
 * This code is based on original Soltys source code
 * Copyright (c) 1994-1995 Janus B. Wisniewski and L.K. Avalon
 */

#include "common/scummsys.h"
#include "common/memstream.h"
#include "common/savefile.h"
#include "common/serializer.h"
#include "common/str.h"
#include "graphics/palette.h"
#include "graphics/scaler.h"
#include "graphics/thumbnail.h"
#include "cge/general.h"
#include "cge/sound.h"
#include "cge/startup.h"
#include "cge/config.h"
#include "cge/vga13h.h"
#include "cge/snail.h"
#include "cge/text.h"
#include "cge/game.h"
#include "cge/events.h"
#include "cge/cfile.h"
#include "cge/vol.h"
#include "cge/talk.h"
#include "cge/vmenu.h"
#include "cge/gettext.h"
#include "cge/mixer.h"
#include "cge/cge_main.h"
#include "cge/cge.h"
#include "cge/walk.h"

namespace CGE {

#define STACK_SIZ   (K(2))
#define SVGCHKSUM   (1956 + _now + _oldLev + _game + _music + _demoText)

#define SVG0NAME    ("{{INIT}}" SVG_EXT)
#define SVG0FILE    INI_FILE

uint16  _stklen = (STACK_SIZ * 2);

Vga *_vga;
Heart *_heart;
System *_sys;
Sprite *_pocLight;
EventManager *_eventManager;
Keyboard *_keyboard;
Mouse *_mouse;
Sprite *_pocket[POCKET_NX];
Sprite *_sprite;
Sprite *_miniCave;
Sprite *_shadow;
HorizLine *_horzLine;
InfoLine *_infoLine;
Sprite *_cavLight;
InfoLine *_debugLine;

Snail *_snail;
Snail *_snail_;

// 0.75 - 17II95  - full sound support
// 0.76 - 18II95  - small MiniEMS in DEMO,
//		    unhide CavLight in SNLEVEL
//		    keyclick suppress in startup
//		    keyclick on key service in: SYSTEM, GET_TEXT
// 1.01 - 17VII95 - default savegame with sound ON
//		    coditionals EVA for 2-month evaluation version

const char *SAVEGAME_STR = "SCUMMVM_CGE";
#define SAVEGAME_STR_SIZE 11

//--------------------------------------------------------------------------

static  bool      _finis       = false;
int	_offUseCount;
uint16 *_intStackPtr = false;

Hxy _heroXY[CAVE_MAX] = {{0, 0}};
Bar _barriers[1 + CAVE_MAX] = { { 0xFF, 0xFF } };

extern Dac _stdPal[58];

void CGEEngine::syncHeader(Common::Serializer &s) {
	debugC(1, kDebugEngine, "CGEEngine::syncHeader(s)");

	int i;

	s.syncAsUint16LE(_now);
	s.syncAsUint16LE(_oldLev);
	s.syncAsUint16LE(_demoText);
	for (i = 0; i < 5; ++i)
		s.syncAsUint16LE(_game);
	s.syncAsSint16LE(i);		// unused VGA::Mono variable
	s.syncAsUint16LE(_music);
	s.syncBytes(_volume, 2);
	for (i = 0; i < 4; ++i)
		s.syncAsUint16LE(_flag[i]);

	for (i = 0; i < CAVE_MAX; ++i) {
		s.syncAsSint16LE(_heroXY[i]._x);
		s.syncAsUint16LE(_heroXY[i]._y);
	}
	for (i = 0; i < 1 + CAVE_MAX; ++i) {
		s.syncAsByte(_barriers[i]._horz);
		s.syncAsByte(_barriers[i]._vert);
	}
	for (i = 0; i < POCKET_NX; ++i)
		s.syncAsUint16LE(_pocref[i]);

	if (s.isSaving()) {
		// Write checksum
		int checksum = SVGCHKSUM;
		s.syncAsUint16LE(checksum);
	} else {
		// Read checksum and validate it
		uint16 checksum;
		s.syncAsUint16LE(checksum);
		if (checksum != SVGCHKSUM)
			error("%s", _text->getText(BADSVG_TEXT));
	}
}

bool CGEEngine::loadGame(int slotNumber, SavegameHeader *header, bool tiny) {
	debugC(1, kDebugEngine, "CGEEngine::loadgame(file, %s)", tiny ? "true" : "false");

	Common::MemoryReadStream *readStream;
	SavegameHeader saveHeader;

	if (slotNumber == -1) {
		// Loading the data for the initial game state
		SVG0FILE file = SVG0FILE(SVG0NAME);
		int size = file.size();
		byte *dataBuffer = (byte *)malloc(size);
		file.read(dataBuffer, size);
		readStream = new Common::MemoryReadStream(dataBuffer, size, DisposeAfterUse::YES);

	} else {
		// Open up the savgame file
		Common::String slotName = generateSaveName(slotNumber);
		Common::InSaveFile *saveFile = g_system->getSavefileManager()->openForLoading(slotName);

		// Read the data into a data buffer
		int size = saveFile->size();
		byte *dataBuffer = (byte *)malloc(size);
		saveFile->read(dataBuffer, size);
		readStream = new Common::MemoryReadStream(dataBuffer, size, DisposeAfterUse::YES);
	}

	// Check to see if it's a ScummVM savegame or not
	char buffer[SAVEGAME_STR_SIZE + 1];
	readStream->read(buffer, SAVEGAME_STR_SIZE + 1);

	if (strncmp(buffer, SAVEGAME_STR, SAVEGAME_STR_SIZE + 1) != 0) {
		// It's not, so rewind back to the start
		readStream->seek(0);

		if (header)
			// Header wanted where none exists, so return false
			return false;
	} else {
		// Found header
		if (!readSavegameHeader(readStream, saveHeader)) {
			delete readStream;
			return false;
		}

		if (header) {
			*header = saveHeader;
			delete readStream;
			return true;
		}

		// Delete the thumbnail
		delete saveHeader.thumbnail;

		// If we're loading the auto-save slot, load the name
		if (slotNumber == 0)
			strncpy(_usrFnam, saveHeader.saveName.c_str(), 8);
	}
	
	// Get in the savegame
	syncGame(readStream, NULL, tiny);

	delete readStream;
	return true;
}

/**
 * Returns true if a given savegame exists
 */
bool CGEEngine::savegameExists(int slotNumber) {
	Common::String slotName = generateSaveName(slotNumber);

	Common::InSaveFile *saveFile = g_system->getSavefileManager()->openForLoading(slotName);
	bool result = saveFile != NULL;
	delete saveFile;
	return result;
}

/**
 * Support method that generates a savegame name
 * @param slot		Slot number
 */
Common::String CGEEngine::generateSaveName(int slot) {
	return Common::String::format("%s.%03d", _targetName.c_str(), slot);
}

Common::Error CGEEngine::loadGameState(int slot) {
	// Clear current game activity
	caveDown();

	// Load the game
	loadGame(slot, NULL, true);
	caveUp();
	loadGame(slot, NULL);

	return Common::kNoError;
}

Common::Error CGEEngine::saveGameState(int slot, const Common::String &desc) {
	saveGame(slot, desc);
	return Common::kNoError;
}


void CGEEngine::saveSound() {
	warning("STUB: CGEEngine::saveSound");
	/*  Convert to saving any such needed data in ScummVM configuration file

	CFile cfg(usrPath(progName(CFG_EXT)), WRI);
	if (!cfg._error)
		cfg.write(&_sndDrvInfo, sizeof(_sndDrvInfo) - sizeof(_sndDrvInfo.Vol2));
	*/
}

void CGEEngine::saveGame(int slotNumber, const Common::String &desc) {
	// Set up the serializer
	Common::String slotName = generateSaveName(slotNumber);
	Common::OutSaveFile *saveFile = g_system->getSavefileManager()->openForSaving(slotName);

	// Write out the ScummVM savegame header
	SavegameHeader header;
	header.saveName = desc;
	header.version = CGE_SAVEGAME_VERSION;
	writeSavegameHeader(saveFile, header);

	// Write out the data of the savegame
	syncGame(NULL, saveFile);

	// Finish writing out game data
	saveFile->finalize();
	delete saveFile;
}

void CGEEngine::writeSavegameHeader(Common::OutSaveFile *out, SavegameHeader &header) {
	// Write out a savegame header
	out->write(SAVEGAME_STR, SAVEGAME_STR_SIZE + 1);

	out->writeByte(CGE_SAVEGAME_VERSION);

	// Write savegame name
	out->write(header.saveName.c_str(), header.saveName.size() + 1);

	// Get the active palette
	uint8 thumbPalette[256 * 3];
	g_system->getPaletteManager()->grabPalette(thumbPalette, 0, 256);

	// Create a thumbnail and save it
	Graphics::Surface *thumb = new Graphics::Surface();
	Graphics::Surface *s = _vga->_page[0];
	::createThumbnail(thumb, (const byte *)s->pixels, SCR_WID, SCR_HIG, thumbPalette);
	Graphics::saveThumbnail(*out, *thumb);
	delete thumb;

	// Write out the save date/time
	TimeDate td;
	g_system->getTimeAndDate(td);
	out->writeSint16LE(td.tm_year + 1900);
	out->writeSint16LE(td.tm_mon + 1);
	out->writeSint16LE(td.tm_mday);
	out->writeSint16LE(td.tm_hour);
	out->writeSint16LE(td.tm_min);
}

void CGEEngine::syncGame(Common::SeekableReadStream *readStream, Common::WriteStream *writeStream, bool tiny) {
	Sprite *spr;
	int i;

	Common::Serializer s(readStream, writeStream);

	if (s.isSaving()) {
		for (i = 0; i < POCKET_NX; i++) {
			register Sprite *s = _pocket[i];
			_pocref[i] = (s) ? s->_ref : -1;
		}

		_volume[0] = _sndDrvInfo.Vol2._d;
		_volume[1] = _sndDrvInfo.Vol2._m;
	}

	// Synchronise header data
	syncHeader(s);

	if (s.isSaving()) {
		// Loop through saving the sprite data
		for (spr = _vga->_spareQ->first(); spr; spr = spr->_next) {
			if ((spr->_ref >= 1000) && !s.err())
				spr->sync(s);
		}
	} else {
		// Loading game
		if (Startup::_core < CORE_HIG)
			_music = false;

		if (Startup::_soundOk == 1 && Startup::_mode == 0) {
			_sndDrvInfo.Vol2._d = _volume[0];
			_sndDrvInfo.Vol2._m = _volume[1];
			sndSetVolume();
		}

		if (! tiny) { // load sprites & pocket
			while (readStream->pos() < readStream->size()) {
				Sprite S(this, NULL);
				S.sync(s);

				S._prev = S._next = NULL;
				spr = (scumm_stricmp(S._file + 2, "MUCHA") == 0) ? new Fly(this, NULL)
					  : new Sprite(this, NULL);
				if (spr == NULL)
					error("No core");
				*spr = S;
				_vga->_spareQ->append(spr);
			}

			for (i = 0; i < POCKET_NX; i++) {
				register int r = _pocref[i];
				delete _pocket[i];
				_pocket[i] = (r < 0) ? NULL : _vga->_spareQ->locate(r);
			}
		}
	}
	debugC(1, kDebugEngine, "CGEEngine::saveSound()");

}

bool CGEEngine::readSavegameHeader(Common::InSaveFile *in, SavegameHeader &header) {
	header.thumbnail = NULL;

	// Get the savegame version
	header.version = in->readByte();
	if (header.version > CGE_SAVEGAME_VERSION)
		return false;

	// Read in the string
	header.saveName.clear();
	char ch;
	while ((ch = (char)in->readByte()) != '\0') header.saveName += ch;

	// Get the thumbnail
	header.thumbnail = new Graphics::Surface();
	if (!Graphics::loadThumbnail(*in, *header.thumbnail)) {
		delete header.thumbnail;
		header.thumbnail = NULL;
		return false;
	}

	// Read in save date/time
	header.saveYear = in->readSint16LE();
	header.saveMonth = in->readSint16LE();
	header.saveDay = in->readSint16LE();
	header.saveHour = in->readSint16LE();
	header.saveMinutes = in->readSint16LE();

	return true;

}

void CGEEngine::heroCover(int cvr) {
	debugC(1, kDebugEngine, "CGEEngine::heroCover(%d)", cvr);

	SNPOST(SNCOVER, 1, cvr, NULL);
}

void CGEEngine::trouble(int seq, int txt) {
	debugC(1, kDebugEngine, "CGEEngine::trouble(%d, %d)", seq, txt);

	_hero->park();
	SNPOST(SNWAIT, -1, -1, _hero);
	SNPOST(SNSEQ, -1, seq, _hero);
	SNPOST(SNSOUND, -1, 2, _hero);
	SNPOST(SNWAIT, -1, -1, _hero);
	SNPOST(SNSAY,  1, txt, _hero);
}

void CGEEngine::offUse() {
	debugC(1, kDebugEngine, "CGEEngine::offUse()");

	trouble(OFF_USE, OFF_USE_TEXT + new_random(_offUseCount));
}

void CGEEngine::tooFar() {
	debugC(1, kDebugEngine, "CGEEngine::tooFar()");

	trouble(TOO_FAR, TOO_FAR_TEXT);
}

void CGEEngine::loadHeroXY() {
	debugC(1, kDebugEngine, "CGEEngine::loadHeroXY()");

	INI_FILE cf(progName(".HXY"));
	memset(_heroXY, 0, sizeof(_heroXY));
	if (!cf._error)
		cf.CFREAD(&_heroXY);
}

void CGEEngine::loadMapping() {
	debugC(1, kDebugEngine, "CGEEngine::loadMapping()");

	if (_now <= CAVE_MAX) {
		INI_FILE cf(progName(".TAB"));
		if (!cf._error) {
			memset(Cluster::_map, 0, sizeof(Cluster::_map));
			cf.seek((_now - 1) * sizeof(Cluster::_map));
			cf.read((uint8 *) Cluster::_map, sizeof(Cluster::_map));
		}
	}
}

class SQUARE : public Sprite {
public:
	SQUARE(CGEEngine *vm);
	virtual void touch(uint16 mask, int x, int y);
private:
	CGEEngine *_vm;
};


SQUARE::SQUARE(CGEEngine *vm)
	: Sprite(vm, NULL), _vm(vm) {
	_flags._kill = true;
	_flags._bDel = false;

	BMP_PTR *MB = new BMP_PTR[2];
	MB[0] = new Bitmap("BRICK", true);
	MB[1] = NULL;
	setShapeList(MB);
}


void SQUARE::touch(uint16 mask, int x, int y) {
	Sprite::touch(mask, x, y);
	if (mask & L_UP) {
		XZ(_x + x, _y + y).cell() = 0;
		SNPOST_(SNKILL, -1, 0, this);
	}
}


void CGEEngine::setMapBrick(int x, int z) {
	debugC(1, kDebugEngine, "CGEEngine::setMapBrick(%d, %d)", x, z);

	SQUARE *s = new SQUARE(this);
	if (s) {
		static char n[] = "00:00";
		s->gotoxy(x * MAP_XGRID, MAP_TOP + z * MAP_ZGRID);
		wtom(x, n + 0, 10, 2);
		wtom(z, n + 3, 10, 2);
		Cluster::_map[z][x] = 1;
		s->setName(n);
		_vga->_showQ->insert(s, _vga->_showQ->first());
	}
}

void CGEEngine::keyClick() {
	debugC(1, kDebugEngine, "CGEEngine::keyClick()");

	SNPOST_(SNSOUND, -1, 5, NULL);
}


void CGEEngine::resetQSwitch() {
	debugC(1, kDebugEngine, "CGEEngine::resetQSwitch()");

	SNPOST_(SNSEQ, 123,  0, NULL);
	keyClick();
}


void CGEEngine::quit() {
	debugC(1, kDebugEngine, "CGEEngine::quit()");

	static Choice QuitMenu[] = { 
		{ NULL, &CGEEngine::startCountDown },
		{ NULL, &CGEEngine::resetQSwitch   },
		{ NULL, &CGEEngine::dummy          }
	};

	if (_snail->idle() && !_hero->_flags._hide) {
		if (Vmenu::_addr) {
			SNPOST_(SNKILL, -1, 0, Vmenu::_addr);
			resetQSwitch();
		} else {
			QuitMenu[0]._text = _text->getText(QUIT_TEXT);
			QuitMenu[1]._text = _text->getText(NOQUIT_TEXT);
			(new Vmenu(this, QuitMenu, -1, -1))->setName(_text->getText(QUIT_TITLE));
			SNPOST_(SNSEQ, 123, 1, NULL);
			keyClick();
		}
	}
}


void CGEEngine::AltCtrlDel() {
	debugC(1, kDebugEngine, "CGEEngine::setup()");

	SNPOST_(SNSAY,  -1, A_C_D_TEXT, _hero);
}

void CGEEngine::miniStep(int stp) {
	if (stp < 0)
		_miniCave->_flags._hide = true;
	else {
		*_miniShp[0] = *_miniShpList[stp];
		if (_fx._current)
			&*(_fx._current->eAddr());

		_miniCave->_flags._hide = false;
	}
}


static void postMiniStep(int stp) {
	static int recent = -2;
	if (_miniCave && stp != recent)
		SNPOST2_(SNEXEC, -1, recent = stp, kMiniStep);
}

void System::setPal() {
	uint i;
	Dac *p = Vga::_sysPal + 256 - ArrayCount(_stdPal);
	for (i = 0; i < ArrayCount(_stdPal); i++) {
		p[i]._r = _stdPal[i]._r >> 2;
		p[i]._g = _stdPal[i]._g >> 2;
		p[i]._b = _stdPal[i]._b >> 2;
	}
}


void System::funTouch() {
	uint16 n = (PAIN) ? HEROFUN1 : HEROFUN0;
	if (_talk == NULL || n > _funDel)
		_funDel = n;
}


static void ShowBak(int ref) {
	Sprite *spr = _vga->_spareQ->locate(ref);
	if (spr) {
		Bitmap::_pal = Vga::_sysPal;
		spr->expand();
		Bitmap::_pal = NULL;
		spr->show(2);
		_vga->copyPage(1, 2);
		_sys->setPal();
		spr->contract();
	}
}


void CGEEngine::caveUp() {
	int BakRef = 1000 * _now;
	if (_music)
		loadMidi(_now);

	ShowBak(BakRef);
	loadMapping();
	_text->preload(BakRef, BakRef + 1000);
	Sprite *spr = _vga->_spareQ->first();
	while (spr) {
		Sprite *n = spr->_next;
		if (spr->_cave == _now || spr->_cave == 0)
			if (spr->_ref != BakRef) {
				if (spr->_flags._back)
					spr->backShow();
				else
					expandSprite(spr);
			}
		spr = n;
	}
	if (_sndDrvInfo._dDev) {
		_sound.stop();
		_fx.clear();
		_fx.preload(0);
		_fx.preload(BakRef);
	}

	if (_hero) {
		_hero->gotoxy(_heroXY[_now - 1]._x, _heroXY[_now - 1]._y);
		// following 2 lines trims Hero's Z position!
		_hero->tick();
		_hero->_time = 1;
		_hero->_flags._hide = false;
	}

	if (!_dark)
		_vga->sunset();

	_vga->copyPage(0, 1);
	selectPocket(-1);
	if (_hero)
		_vga->_showQ->insert(_vga->_showQ->remove(_hero));

	if (_shadow) {
		_vga->_showQ->remove(_shadow);
		_shadow->makeXlat(glass(Vga::_sysPal, 204, 204, 204));
		_vga->_showQ->insert(_shadow, _hero);
		_shadow->_z = _hero->_z;
	}
	feedSnail(_vga->_showQ->locate(BakRef + 999), kTake);
	_vga->show();
	_vga->copyPage(1, 0);
	_vga->show();
	_vga->sunrise(Vga::_sysPal);
	_dark = false;
	if (!_startupMode)
		_mouse->on();

	_heart->_enable = true;
}


void CGEEngine::caveDown() {
	Sprite *spr;
	if (!_horzLine->_flags._hide)
		switchMapping();

	for (spr = _vga->_showQ->first(); spr;) {
		Sprite *n = spr->_next;
		if (spr->_ref >= 1000 /*&& spr->_cave*/) {
			if (spr->_ref % 1000 == 999)
				feedSnail(spr, kTake);
			_vga->_spareQ->append(_vga->_showQ->remove(spr));
		}
		spr = n;
	}
	_text->clear(1000);
}


void CGEEngine::xCave() {
	caveDown();
	caveUp();
}


void CGEEngine::qGame() {
	caveDown();
	_oldLev = _lev;
	saveSound();

	// Write out the user's progress
	saveGame(0, _usrFnam);

	_vga->sunset();
	_finis = true;
}


void CGEEngine::switchCave(int cav) {
	if (cav != _now) {
		_heart->_enable = false;
		if (cav < 0) {
			SNPOST(SNLABEL, -1, 0, NULL);  // wait for repaint
			SNPOST2(SNEXEC,  -1, 0, kQGame); // switch cave
		} else {
			_now = cav;
			_mouse->off();
			if (_hero) {
				_hero->park();
				_hero->step(0);
				if (!_isDemo)
				///// protection: auto-destruction on! ----------------------
					_vga->_spareQ->_show = Startup::_summa * (cav <= CAVE_MAX);
				/////--------------------------------------------------------
			}
			_cavLight->gotoxy(CAVE_X + ((_now - 1) % CAVE_NX) * CAVE_DX + CAVE_SX,
			              CAVE_Y + ((_now - 1) / CAVE_NX) * CAVE_DY + CAVE_SY);
			killText();
			if (!_startupMode)
				keyClick();
			SNPOST(SNLABEL, -1, 0, NULL);  // wait for repaint
			SNPOST2(SNEXEC,   0, 0, kXCave); // switch cave
		}
	}
}

System::System(CGEEngine *vm) : Sprite(vm, NULL), _vm(vm) {
	_funDel = HEROFUN0;
	setPal();
	tick();
}

void System::touch(uint16 mask, int x, int y) {
	static int pp = 0;

	funTouch();

	if (mask & KEYB) {
		int pp0;
		_vm->keyClick();
		killText();
		if (_vm->_startupMode == 1) {
			SNPOST(SNCLEAR, -1, 0, NULL);
			return;
		}
		pp0 = pp;
		switch (x) {
		case Del:
			if (_keyboard->_key[ALT] && _keyboard->_key[CTRL])
				_vm->AltCtrlDel();
			else 
				_vm->killSprite();
			break;
		case 'F':
			if (_keyboard->_key[ALT]) {
				Sprite *m = _vga->_showQ->locate(17001);
				if (m) {
					m->step(1);
					m->_time = 216; // 3s
				}
			}
			break;
		case PgUp:
			_vm->pushSprite();
			break;
		case PgDn:
			_vm->pullSprite();
			break;
		case '+':
			_vm->nextStep();
			break;
		case '`':
			if (_keyboard->_key[ALT])
				_vm->saveMapping();
			else
				_vm->switchMapping();
			break;
		case F1:
			_vm->switchDebug();
			break;
		case F3:
			_hero->step(TSEQ + 4);
			break;
		case F4:
			_hero->step(TSEQ + 5);
			break;
		case F5:
			_hero->step(TSEQ + 0);
			break;
		case F6:
			_hero->step(TSEQ + 1);
			break;
		case F7:
			_hero->step(TSEQ + 2);
			break;
		case F8:
			_hero->step(TSEQ + 3);
			break;
		case F9:
			_sys->_funDel = 1;
			break;
		case 'X':
			if (_keyboard->_key[ALT])
				_finis = true;
			break;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
			if (_keyboard->_key[ALT]) {
				SNPOST(SNLEVEL, -1, x - '0', NULL);
				break;
			}
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			if (_sprite)
				_sprite->step(x - '0');
			break;
		case F10          :
			if (_snail->idle() && !_hero->_flags._hide)
				_vm->startCountDown();
			break;
		case 'J':
			if (pp == 0)
				pp++;
			break;
		case 'B':
			if (pp == 1)
				pp++;
			break;
		case 'W':
			if (pp == 2)
				_vm->_jbw = !_vm->_jbw;
			break;
		}
		if (pp == pp0)
			pp = 0;
	} else {
		if (_vm->_startupMode)
			return;
		int cav = 0;
		_infoLine->update(NULL);
		if (y >= WORLD_HIG) {
			if (x < BUTTON_X) {                           // select cave?
				if (y >= CAVE_Y && y < CAVE_Y + CAVE_NY * CAVE_DY &&
				        x >= CAVE_X && x < CAVE_X + CAVE_NX * CAVE_DX && !_vm->_game) {
					cav = ((y - CAVE_Y) / CAVE_DY) * CAVE_NX + (x - CAVE_X) / CAVE_DX + 1;
					if (cav > _vm->_maxCave)
						cav = 0;
				} else {
					cav = 0;
				}
			} else if (mask & L_UP) {
				if (y >= POCKET_Y && y < POCKET_Y + POCKET_NY * POCKET_DY &&
				        x >= POCKET_X && x < POCKET_X + POCKET_NX * POCKET_DX) {
					int n = ((y - POCKET_Y) / POCKET_DY) * POCKET_NX + (x - POCKET_X) / POCKET_DX;
					_vm->selectPocket(n);
				}
			}
		}

		postMiniStep(cav - 1);

		if (mask & L_UP) {
			if (cav && _snail->idle() && _hero->_tracePtr < 0)
				_vm->switchCave(cav);

			if (!_horzLine->_flags._hide) {
				if (y >= MAP_TOP && y < MAP_TOP + MAP_HIG) {
					int8 x1, z1;
					XZ(x, y).split(x1, z1);
					Cluster::_map[z1][x1] = 1;
					_vm->setMapBrick(x1, z1);
				}
			} else
			{
				if (!_talk && _snail->idle() && _hero
				        && y >= MAP_TOP && y < MAP_TOP + MAP_HIG && !_vm->_game) {
					_hero->findWay(XZ(x, y));
				}
			}
		}
	}
}


void System::tick() {
	if (!_vm->_startupMode)
		if (--_funDel == 0) {
			killText();
			if (_snail->idle()) {
				if (PAIN)
					_vm->heroCover(9);
				else if (Startup::_core >= CORE_MID) {
					int n = new_random(100);
					if (n > 96)
						_vm->heroCover(6 + (_hero->_x + _hero->_w / 2 < SCR_WID / 2));
					else {
						if (n > 90)
							_vm->heroCover(5);
						else {
							if (n > 60)
								_vm->heroCover(4);
							else
								_vm->heroCover(3);
						}
					}
				}
			}
			funTouch();
		}
	_time = SYSTIMERATE;
}


/*
static void SpkOpen() {
  asm   in  al,0x61
  asm   or  al,0x03
  asm   out 0x61,al
  asm   mov al,0x90
  asm   out 0x43,al
}


static void SpkClose() {
  asm   in  al,0x61
  asm   and al,0xFC
  asm   out 0x61,al
}

*/


void CGEEngine::switchColorMode() {
	SNPOST_(SNSEQ, 121, _vga->_mono = !_vga->_mono, NULL);
	keyClick();
	_vga->setColors(Vga::_sysPal, 64);
}



void CGEEngine::switchMusic() {
	if (_keyboard->_key[ALT]) {
		if (Vmenu::_addr)
			SNPOST_(SNKILL, -1, 0, Vmenu::_addr);
		else {
			SNPOST_(SNSEQ, 122, (_music = false), NULL);
			SNPOST2(SNEXEC, -1, 0, kSelectSound);
		}
	} else {
		if (Startup::_core < CORE_HIG)
			SNPOST(SNINF, -1, NOMUSIC_TEXT, NULL);
		else {
			SNPOST_(SNSEQ, 122, (_music = !_music), NULL);
			keyClick();
		}
	}
	if (_music)
		loadMidi(_now);
	else
		killMidi();
}


void CGEEngine::startCountDown() {
	//SNPOST(SNSEQ, 123, 0, NULL);
	switchCave(-1);
}


void CGEEngine::takeName() {
	if (GetText::_ptr)
		SNPOST_(SNKILL, -1, 0, GetText::_ptr);
	else {
		memset(_usrFnam, 0, 15);
		GetText *tn = new GetText(this, _text->getText(GETNAME_PROMPT), _usrFnam, 8);
		if (tn) {
			tn->setName(_text->getText(GETNAME_TITLE));
			tn->center();
			tn->gotoxy(tn->_x, tn->_y - 10);
			tn->_z = 126;
			_vga->_showQ->insert(tn);
		}
	}
}


void CGEEngine::switchMapping() {
	if (_horzLine->_flags._hide) {
		int i;
		for (i = 0; i < MAP_ZCNT; i++) {
			int j;
			for (j = 0; j < MAP_XCNT; j++) {
				if (Cluster::_map[i][j])
					setMapBrick(j, i);
			}
		}
	} else {
		Sprite *s;
		for (s = _vga->_showQ->first(); s; s = s->_next)
			if (s->_w == MAP_XGRID && s->_h == MAP_ZGRID)
				SNPOST_(SNKILL, -1, 0, s);
	}
	_horzLine->_flags._hide = !_horzLine->_flags._hide;
}

void CGEEngine::killSprite() {
	_sprite->_flags._kill = true;
	_sprite->_flags._bDel = true;
	SNPOST_(SNKILL, -1, 0, _sprite);
	_sprite = NULL;
}

void CGEEngine::pushSprite() {
	Sprite *spr = _sprite->_prev;
	if (spr) {
		_vga->_showQ->insert(_vga->_showQ->remove(_sprite), spr);
		while (_sprite->_z > _sprite->_next->_z)
			_sprite->_z--;
	} else
		SNPOST_(SNSOUND, -1, 2, NULL);
}

void CGEEngine::pullSprite() {
	bool ok = false;
	Sprite *spr = _sprite->_next;
	if (spr) {
		spr = spr->_next;
		if (spr)
			ok = (!spr->_flags._slav);
	}
	if (ok) {
		_vga->_showQ->insert(_vga->_showQ->remove(_sprite), spr);
		if (_sprite->_prev)
			while (_sprite->_z < _sprite->_prev->_z)
				_sprite->_z++;
	} else
		SNPOST_(SNSOUND, -1, 2, NULL);
}

void CGEEngine::nextStep() {
	SNPOST_(SNSTEP, 0, 0, _sprite);
}

void CGEEngine::saveMapping() {
	{
		IoHand cf(progName(".TAB"), UPD);
		if (!cf._error) {
			cf.seek((_now - 1) * sizeof(Cluster::_map));
			cf.write((uint8 *) Cluster::_map, sizeof(Cluster::_map));
		}
	}
	{
		IoHand cf(progName(".HXY"), WRI);
			if (!cf._error) {
				_heroXY[_now - 1]._x = _hero->_x;
				_heroXY[_now - 1]._y = _hero->_y;
				cf.write((uint8 *) _heroXY, sizeof(_heroXY));
		}
	}
}

//              1111111111222222222233333333 334444444444555555555566666666667777777777
//    01234567890123456789012345678901234567 890123456789012345678901234567890123456789
static  char    DebugText[] = " N=00000 F=000000 X=000 Y=000 FPS=0000\0S=00:00 000:000:000 000:000 00  ";

#define NFRE    (DebugText +  3)
#define FFRE    (DebugText + 11)
#define ABSX    (DebugText + 20)
#define ABSY    (DebugText + 26)
#define FRPS    (DebugText + 34)
#define XSPR    (DebugText + 38)
#define SP_N    (DebugText + 41)
#define SP_S    (DebugText + 44)

#define SP_X    (DebugText + 47)
#define SP_Y    (DebugText + 51)
#define SP_Z    (DebugText + 55)
#define SP_W    (DebugText + 59)
#define SP_H    (DebugText + 63)
#define SP_F    (DebugText + 67)
#define SP__    (DebugText + 70)

void CGEEngine::sayDebug() {
	if (!_debugLine->_flags._hide) {
		static long t = -1L;
		long t1 = timer();

		if (t1 - t >= 18) {
			static uint32 old = 0L;
			uint32 now = _vga->_frmCnt;
			dwtom(now - old, FRPS, 10, 4);
			old = now;
			t = t1;
		}

		dwtom(_mouse->_x, ABSX, 10, 3);
		dwtom(_mouse->_y, ABSY, 10, 3);
//		dwtom(coreleft(), NFRE, 10, 5);
//		dwtom(farcoreleft(), FFRE, 10, 6);

		// sprite queue size
		uint16 n = 0;
		Sprite *spr;
		for (spr = _vga->_showQ->first(); spr; spr = spr->_next) {
			++ n;
			if (spr == _sprite) {
				*XSPR = ' ';
				dwtom(n, SP_N, 10, 2);
				dwtom(_sprite->_x, SP_X, 10, 3);
				dwtom(_sprite->_y, SP_Y, 10, 3);
				dwtom(_sprite->_z, SP_Z, 10, 3);
				dwtom(_sprite->_w, SP_W, 10, 3);
				dwtom(_sprite->_h, SP_H, 10, 3);
				dwtom(*(uint16 *)(&_sprite->_flags), SP_F, 16, 2);
			}
		}
		dwtom(n, SP_S, 10, 2);
//		*SP__ = (heapcheck() < 0) ? '!' : ' ';
		_debugLine->update(DebugText);
	}
}


void CGEEngine::switchDebug() {
	_debugLine->_flags._hide = !_debugLine->_flags._hide;
}


void CGEEngine::optionTouch(int opt, uint16 mask) {
	switch (opt) {
	case 1 :
		if (mask & L_UP)
			switchColorMode();
		break;
	case 2 :
		if (mask & L_UP)
			switchMusic();
		else if (mask & R_UP)
			if (!Mixer::_appear) {
				Mixer::_appear = true;
				new Mixer(this, BUTTON_X, BUTTON_Y);
			}
		break;
	case 3 :
		if (mask & L_UP)
			quit();
		break;
	}
}


#pragma argsused
void Sprite::touch(uint16 mask, int x, int y) {
	_sys->funTouch();
	if ((mask & ATTN) == 0) {
		_infoLine->update(name());
		if (mask & (R_DN | L_DN))
			_sprite = this;
		if (_ref / 10 == 12) {
			_vm->optionTouch(_ref % 10, mask);
			return;
		}
		if (_flags._syst)
			return;       // cannot access system sprites
		if (_vm->_game) if (mask & L_UP) {
				mask &= ~L_UP;
				mask |= R_UP;
			}
		if ((mask & R_UP) && _snail->idle()) {
			Sprite *ps = (_pocLight->_seqPtr) ? _pocket[_vm->_pocPtr] : NULL;
			if (ps) {
				if (_flags._kept || _hero->distance(this) < MAX_DISTANCE) {
					if (works(ps)) {
						_vm->feedSnail(ps, kTake);
					} else
						_vm->offUse();
					_vm->selectPocket(-1);
				} else
					_vm->tooFar();
			} else {
				if (_flags._kept)
					mask |= L_UP;
				else {
					if (_hero->distance(this) < MAX_DISTANCE) {
						///
						if (_flags._port) {
							if (_vm->findPocket(NULL) < 0)
								_vm->pocFul();
							else {
								SNPOST(SNREACH, -1, -1, this);
								SNPOST(SNKEEP, -1, -1, this);
								_flags._port = false;
							}
						} else {
							if (_takePtr != NO_PTR) {
								if (snList(kTake)[_takePtr]._com == SNNEXT)
									_vm->offUse();
								else
									_vm->feedSnail(this, kTake);
							} else
								_vm->offUse();
						}
					}///
					else
						_vm->tooFar();
				}
			}
		}
		if ((mask & L_UP) && _snail->idle()) {
			if (_flags._kept) {
				int n;
				for (n = 0; n < POCKET_NX; n++) {
					if (_pocket[n] == this) {
						_vm->selectPocket(n);
						break;
					}
				}
			} else
				SNPOST(SNWALK, -1, -1, this); // Hero->FindWay(this);
		}
	}
}


void CGEEngine::loadSprite(const char *fname, int ref, int cav, int col = 0, int row = 0, int pos = 0) {
	static const char *Comd[] = { "Name", "Type", "Phase", "East",
	                              "Left", "Right", "Top", "Bottom",
	                              "Seq", "Near", "Take",
	                              "Portable", "Transparent",
	                              NULL
	                            };
	static const char *Type[] = { "DEAD", "AUTO", "WALK", "NEWTON", "LISSAJOUS",
	                              "FLY", NULL
	                            };
	char line[LINE_MAX];

	int shpcnt = 0;
	int type = 0; // DEAD
	bool east = false;
	bool port = false;
	bool tran = false;
	int i, lcnt = 0;
	uint16 len;

	mergeExt(line, fname, SPR_EXT);
	if (INI_FILE::exist(line)) {      // sprite description file exist
		INI_FILE sprf(line);
		if (sprf._error)
			error("Bad SPR [%s]", line);

		while ((len = sprf.read((uint8 *)line)) != 0) {
			++ lcnt;
			if (len && line[len - 1] == '\n')
				line[-- len] = '\0';
			if (len == 0 || *line == '.')
				continue;

			if ((i = takeEnum(Comd, strtok(line, " =\t"))) < 0)
				error("Bad line %d [%s]", lcnt, fname);


			switch (i) {
			case  0 : // Name - will be taken in Expand routine
				break;
			case  1 : // Type
				if ((type = takeEnum(Type, strtok(NULL, " \t,;/"))) < 0)
					error("Bad line %d [%s]", lcnt, fname);
				break;
			case  2 : // Phase
				++ shpcnt;
				break;
			case  3 : // East
				east = (atoi(strtok(NULL, " \t,;/")) != 0);
				break;
			case 11 : // Portable
				port = (atoi(strtok(NULL, " \t,;/")) != 0);
				break;
			case 12 : // Transparent
				tran = (atoi(strtok(NULL, " \t,;/")) != 0);
				break;
			}
		}
		if (! shpcnt)
			error("No shapes [%s]", fname);
	} else { // no sprite description: mono-shaped sprite with only .BMP file
		++shpcnt;
	}

	// make sprite of choosen type
	switch (type) {
	case 1 : { // AUTO
		_sprite = new Sprite(this, NULL);
		if (_sprite) {
			_sprite->gotoxy(col, row);
			//Sprite->Time = 1;//-----------$$$$$$$$$$$$$$$$
		}
		break;
	}
	case 2 : { // WALK
		WALK *w = new WALK(this, NULL);
		if (w && ref == 1) {
			w->gotoxy(col, row);
			if (_hero)
				error("2nd HERO [%s]", fname);
			_hero = w;
		}
		_sprite = w;
		break;
	}
	/*
	case 3 : // NEWTON
	NEWTON * n = new NEWTON(NULL);
	if (n)
	{
	   n->Ay = (bottom-n->H);
	   n->By = 90;
	   n->Cy = 3;
	   n->Bx = 99;
	   n->Cx = 3;
	   n->Goto(col, row);
	 }
	     _sprite = n;
	     break;
	     */
	case 4 : { // LISSAJOUS
		error("Bad type [%s]", fname);
		/*
		LISSAJOUS * l = new LISSAJOUS(NULL);
		if (l)
		{
		   l->Ax = SCR_WID/2;
		   l->Ay = SCR_HIG/2;
		   l->Bx = 7;
		   l->By = 13;
		   l->Cx = 300;
		   l->Cy = 500;
		   *(long *) &l->Dx = 0; // movex * cnt
		   l->Goto(col, row);
		 }
		     _sprite = l;
		     */
		break;
	}
	case 5 : { // FLY
		Fly *f = new Fly(this, NULL);
		_sprite = f;
		//////Sprite->Time = 1;//-----------$$$$$$$$$$$$$$
		break;
	}
	default: { // DEAD
		_sprite = new Sprite(this, NULL);
		if (_sprite)
			_sprite->gotoxy(col, row);
		break;
	}
	}
	if (_sprite) {
		_sprite->_ref = ref;
		_sprite->_cave = cav;
		_sprite->_z = pos;
		_sprite->_flags._east = east;
		_sprite->_flags._port = port;
		_sprite->_flags._tran = tran;
		_sprite->_flags._kill = true;
		_sprite->_flags._bDel = true;

		// Extract the filename, without the extension
		strcpy(_sprite->_file, fname);
		char *p = strchr(_sprite->_file, '.');
		if (p)
			*p = '\0';

		_sprite->_shpCnt = shpcnt;
		_vga->_spareQ->append(_sprite);
	}
}


void CGEEngine::loadScript(const char *fname) {
	char line[LINE_MAX];
	char *SpN;
	int SpI, SpA, SpX, SpY, SpZ;
	bool BkG = false;
	INI_FILE scrf(fname);
	int lcnt = 0;
	bool ok = true;

	if (scrf._error)
		return;

	while (scrf.read((uint8 *)line) != 0) {
		char *p;

		lcnt++;
		if (*line == 0 || *line == '\n' || *line == '.')
			continue;

		ok = false;   // not OK if break
		// sprite ident number
		if ((p = strtok(line, " \t\n")) == NULL)
			break;
		SpI = atoi(p);
		// sprite file name
		if ((SpN = strtok(NULL, " ,;/\t\n")) == NULL)
			break;
		// sprite cave
		if ((p = strtok(NULL, " ,;/\t\n")) == NULL)
			break;
		SpA = atoi(p);
		// sprite column
		if ((p = strtok(NULL, " ,;/\t\n")) == NULL)
			break;
		SpX = atoi(p);
		// sprite row
		if ((p = strtok(NULL, " ,;/\t\n")) == NULL)
			break;
		SpY = atoi(p);
		// sprite Z pos
		if ((p = strtok(NULL, " ,;/\t\n")) == NULL)
			break;
		SpZ = atoi(p);
		// sprite life
		if ((p = strtok(NULL, " ,;/\t\n")) == NULL)
			break;
		BkG = atoi(p) == 0;

		ok = true;    // no break: OK

		_sprite = NULL;
		loadSprite(SpN, SpI, SpA, SpX, SpY, SpZ);
		if (_sprite && BkG)
			_sprite->_flags._back = true;
	}
	if (! ok)
		error("Bad INI line %d [%s]", lcnt, fname);
}

#define GAME_FRAME_DELAY (1000 / 50)
#define GAME_TICK_DELAY (1000 / 62)

void CGEEngine::mainLoop() {
	sayDebug();

	if (_isDemo) {
//		static uint32 tc = 0;
		if (/* FIXME: TimerCount - tc >= ((182L * 6L) * 5L) && */ _talk == NULL && _snail->idle()) {
			if (_text->getText(_demoText)) {
				SNPOST(SNSOUND,  -1, 4, NULL); // drumla
				SNPOST(SNINF,  -1, _demoText, NULL);
				SNPOST(SNLABEL, -1, -1, NULL);
				if (_text->getText(++_demoText) == NULL)
					_demoText = DEMO_TEXT + 1;
			}
			//FIXME: tc = TimerCount;
		}
	}
	_vga->show();
	_snail_->runCom();
	_snail->runCom();

	// Handle a delay between game frames
	handleFrame();

	// Handle any pending events
	_eventManager->poll();
}

void CGEEngine::handleFrame() {
	// Game frame delay
	uint32 millis = g_system->getMillis();
	while (!_eventManager->_quitFlag && (millis < (_lastFrame + GAME_FRAME_DELAY))) {
		// Handle any pending events
		_eventManager->poll();

		if (millis >= (_lastTick + GAME_TICK_DELAY)) {
			// Dispatch the tick to any active objects
			tick();
			_lastTick = millis;
		}

		// Slight delay
		g_system->delayMillis(10);
		millis = g_system->getMillis();
	}
	_lastFrame = millis;

	if (millis >= (_lastTick + GAME_TICK_DELAY)) {
		// Dispatch the tick to any active objects
		tick();
		_lastTick = millis;
	}
}

void CGEEngine::tick() {
	for (Sprite *spr = _vga->_showQ->first(); spr; spr = spr->_next) {
		if (spr->_time) {
			if (!spr->_flags._hide) {
				if (--spr->_time == 0)
					spr->tick();
			}
		}
	}
}

void CGEEngine::loadUser() {
	// set scene
	if (Startup::_mode == 0) { 
		// user .SVG file found - load it from slot 0
		loadGame(0, NULL);
	} else {
		if (Startup::_mode == 1) {
			// Load either initial game state savegame or launcher specified savegame
			loadGame(_startGameSlot, NULL);
		} else {
			error("Creating setup savegames not supported");
		}
	}
	loadScript(progName(IN0_EXT));
}


void CGEEngine::runGame() {
	if (_eventManager->_quitFlag)
		return;

	_text->clear();
	_text->preload(100, 1000);
	loadHeroXY();

	_cavLight->_flags._tran = true;
	_vga->_showQ->append(_cavLight);
	_cavLight->_flags._hide = true;

	const Seq pocSeq[] = {
		{ 0, 0, 0, 0, 20 },
		{ 1, 2, 0, 0,  4 },
		{ 2, 3, 0, 0,  4 },
		{ 3, 4, 0, 0, 16 },
		{ 2, 5, 0, 0,  4 },
		{ 1, 6, 0, 0,  4 },
		{ 0, 1, 0, 0, 16 },
	};
	Seq *seq = (Seq *)malloc(7 * sizeof(Seq));
	Common::copy(pocSeq, pocSeq + 7, seq);
	_pocLight->setSeq(seq);

	_pocLight->_flags._tran = true;
	_pocLight->_time = 1;
	_pocLight->_z = 120;
	_vga->_showQ->append(_pocLight);
	selectPocket(-1);

	_vga->_showQ->append(_mouse);

//    ___________
	loadUser();
//    ~~~~~~~~~~~

	if ((_sprite = _vga->_spareQ->locate(121)) != NULL)
		SNPOST_(SNSEQ, -1, _vga->_mono, _sprite);
	if ((_sprite = _vga->_spareQ->locate(122)) != NULL)
		_sprite->step(_music);
	SNPOST_(SNSEQ, -1, _music, _sprite);
	if (!_music)
		killMidi();

	if (_mini && INI_FILE::exist("MINI.SPR")) {
		_miniShp = new BMP_PTR[2];
		_miniShp[0] = _miniShp[1] = NULL;

		uint8 *ptr = (uint8 *) &*_mini;
		if (ptr != NULL) {
			loadSprite("MINI", -1, 0, MINI_X, MINI_Y);
			expandSprite(_miniCave = _sprite);  // NULL is ok
			if (_miniCave) {
				_miniCave->_flags._kill = false;
				_miniCave->_flags._hide = true;
				_miniCave->moveShapes(ptr);
				_miniShp[0] = new Bitmap(*_miniCave->shp());
				_miniShpList = _miniCave->setShapeList(_miniShp);
				postMiniStep(-1);
			}
		}
	}

	if (_hero) {
		expandSprite(_hero);
		_hero->gotoxy(_heroXY[_now - 1]._x, _heroXY[_now - 1]._y);
		if (INI_FILE::exist("00SHADOW.SPR")) {
			loadSprite("00SHADOW", -1, 0, _hero->_x + 14, _hero->_y + 51);
			delete _shadow;
			if ((_shadow = _sprite) != NULL) {
				_shadow->_ref = 2;
				_shadow->_flags._tran = true;
				_shadow->_flags._kill = false;
				_hero->_flags._shad = true;
				_vga->_showQ->insert(_vga->_spareQ->remove(_shadow), _hero);
			}
		}
	}

	_infoLine->gotoxy(INFO_X, INFO_Y);
	_infoLine->_flags._tran = true;
	_infoLine->update(NULL);
	_vga->_showQ->insert(_infoLine);

	_debugLine->_z = 126;
	_vga->_showQ->insert(_debugLine);

	_horzLine->_y = MAP_TOP - (MAP_TOP > 0);
	_horzLine->_z = 126;
	_vga->_showQ->insert(_horzLine);

	_mouse->_busy = _vga->_spareQ->locate(BUSY_REF);
	if (_mouse->_busy)
		expandSprite(_mouse->_busy);

	_startupMode = 0;

	SNPOST(SNLEVEL, -1, _oldLev, &_cavLight);
	_cavLight->gotoxy(CAVE_X + ((_now - 1) % CAVE_NX) * CAVE_DX + CAVE_SX,
	              CAVE_Y + ((_now - 1) / CAVE_NX) * CAVE_DY + CAVE_SY);
	caveUp();

	_keyboard->setClient(_sys);
	// main loop
	while (!_finis && !_eventManager->_quitFlag) {
		if (_finis)
			SNPOST2(SNEXEC,  -1, 0, kQGame);
		mainLoop();
	}

	_keyboard->setClient(NULL);
	_heart->_enable = false;
	SNPOST(SNCLEAR, -1, 0, NULL);
	SNPOST_(SNCLEAR, -1, 0, NULL);
	_mouse->off();
	_vga->_showQ->clear();
	_vga->_spareQ->clear();
	_hero = NULL;
	_shadow = NULL;
}


void CGEEngine::movie(const char *ext) {
	if (_eventManager->_quitFlag)
		return;

	const char *fn = progName(ext);
	if (INI_FILE::exist(fn)) {
		loadScript(fn);
		expandSprite(_vga->_spareQ->locate(999));
		feedSnail(_vga->_showQ->locate(999), kTake);

		_vga->_showQ->append(_mouse);

		_heart->_enable = true;
		_keyboard->setClient(_sys);
		while (!_snail->idle() && !_eventManager->_quitFlag)
			mainLoop();

		_keyboard->setClient(NULL);
		_heart->_enable = false;
		SNPOST(SNCLEAR, -1, 0, NULL);
		SNPOST_(SNCLEAR, -1, 0, NULL);
		_vga->_showQ->clear();
		_vga->_spareQ->clear();
	}
}


bool CGEEngine::showTitle(const char *name) {
	if (_eventManager->_quitFlag)
		return false;

	Bitmap::_pal = Vga::_sysPal;
	BMP_PTR *LB = new BMP_PTR[2];
	LB[0] = new Bitmap(name, true);
	LB[1] = NULL;
	Bitmap::_pal = NULL;
	bool usr_ok = false;

	Sprite D(this, LB);
	D._flags._kill = true;
	D._flags._bDel = true;
	D.center();
	D.show(2);

	if (Startup::_mode == 2) {
		inf(SVG0NAME);
		_talk->show(2);
	}

	_vga->sunset();
	_vga->copyPage(1, 2);
	_vga->copyPage(0, 1);
	selectPocket(-1);
	_vga->sunrise(Vga::_sysPal);

	if (Startup::_mode < 2 && !Startup::_soundOk) {
		_vga->copyPage(1, 2);
		_vga->copyPage(0, 1);
		_vga->_showQ->append(_mouse);
		_heart->_enable = true;
		_mouse->on();
		for (selectSound(); !_snail->idle() || Vmenu::_addr;) {
			mainLoop();
			if (_eventManager->_quitFlag)
				return false;
		}

		_mouse->off();
		_heart->_enable = false;
		_vga->_showQ->clear();
		_vga->copyPage(0, 2);
		Startup::_soundOk = 2;
		if (_music)
			loadMidi(0);
	}

	if (Startup::_mode < 2) {
		if (_isDemo) {
			strcpy(_usrFnam, progName(SVG_EXT));
			usr_ok = true;
		} else {
			//-----------------------------------------
#ifndef EVA
#ifdef CD
			Startup::_summa |= (0xC0 + (DriveCD(0) << 6)) & 0xFF;
#else
			// At this point the game originally read the boot sector to get
			// the serial number for it's copy protection check
#endif
			//-----------------------------------------
			movie("X00"); // paylist
			_vga->copyPage(1, 2);
			_vga->copyPage(0, 1);
			_vga->_showQ->append(_mouse);
			//Mouse.On();

			// For ScummVM, skip prompting for name if a savegame in slot 0 already exists
			if ((_startGameSlot == -1) && savegameExists(0)) {
				strcpy(_usrFnam, "User");
				usr_ok = true;
			} else {
				_heart->_enable = true;
				for (takeName(); GetText::_ptr;) {
					mainLoop();
					if (_eventManager->_quitFlag)
						return false;
				}
				_heart->_enable = false;
				if (_keyboard->last() == Enter && *_usrFnam)
					usr_ok = true;
			}
			//Mouse.Off();
			_vga->_showQ->clear();
			_vga->copyPage(0, 2);
#endif
		}

		if (usr_ok && Startup::_mode == 0) {
			if (savegameExists(0)) {
				// Load the savegame
				loadGame(0, NULL, true); // only system vars
				_vga->setColors(Vga::_sysPal, 64);
				_vga->update();
				if (_flag[3]) { //flag FINIS
					Startup::_mode++;
					_flag[3] = false;
				}
			} else
				Startup::_mode++;
		}
	}

	if (Startup::_mode < 2)
		movie("X01"); // wink

	_vga->copyPage(0, 2);

	if (_isDemo)
		return true;
	else
		return (Startup::_mode == 2 || usr_ok);
}


/*
void StkDump () {
  CFILE f("!STACK.DMP", BFW);
  f.Write((uint8 *) (intStackPtr-STACK_SIZ/2), STACK_SIZ*2);
}
*/


void CGEEngine::cge_main() {
	uint16 intStack[STACK_SIZ / 2];
	_intStackPtr = intStack;

	//Debug( memset((void *) (-K(2)), 0, K(1)); )
	//Debug( memset((void *) (-K(4)), 0, K(1)); )
	memset(_barriers, 0xFF, sizeof(_barriers));

	if (!_mouse->_exist)
		error("%s", _text->getText(NO_MOUSE_TEXT));

	if (!SVG0FILE::exist(SVG0NAME))
		Startup::_mode = 2;

	_debugLine->_flags._hide = true;
	_horzLine->_flags._hide = true;

	if (_music && Startup::_soundOk)
		loadMidi(0);

	if (_startGameSlot != -1) {
		// Starting up a savegame from the launcher
		Startup::_mode++;
		runGame();

		_startupMode = 2;
		if (_flag[3]) // Flag FINIS
			movie("X03");
	} else {
		if (Startup::_mode < 2)
			movie(LGO_EXT);

		if (showTitle("WELCOME")) {
			if ((!_isDemo) && (Startup::_mode == 1))
				movie("X02"); // intro
			runGame();
			_startupMode = 2;
			if (_flag[3]) // Flag FINIS
				movie("X03");
		} else
			_vga->sunset();
	}
}

} // End of namespace CGE
