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

/*
 * This code is based on original Sfinx source code
 * Copyright (c) 1994-1997 Janus B. Wisniewski and L.K. Avalon
 */

#include "sound.h"
#include "cge2/cge2_main.h"
#include "cge2/cge2.h"
#include "cge2/vga13h.h"
#include "cge2/text.h"
#include "cge2/snail.h"
#include "cge2/hero.h"
#include "cge2/spare.h"

namespace CGE2 {

int CGE2Engine::number(char *s) {
	int r = atoi(s);
	char *pp = strchr(s, ':');
	if (pp)
		r = (r << 8) + atoi(pp + 1);
	return r;
}

char *CGE2Engine::token(char *s) {
	return strtok(s, " =\t,;/()");
}

int CGE2Engine::takeEnum(const char **tab, const char *text) {
	if (text) {
		for (const char **e = tab; *e; e++) {
			if (scumm_stricmp(text, *e) == 0) {
				return e - tab;
			}
		}
	}
	return -1;
}

ID CGE2Engine::ident(const char *s) {
	return ID(takeEnum(EncryptedStream::kIdTab, s));
}

bool CGE2Engine::testBool(char *s) {
	return number(s) != 0;
}

void CGE2Engine::badLab(const char *fn) {
	error("Misplaced label in %s!", fn);
}

void CGE2Engine::loadSprite(const char *fname, int ref, int scene, V3D &pos) {
	int shpcnt = 0;
	int seqcnt = 0;
	int cnt[kActions];
	for (int i = 0; i < kActions; i++)
		cnt[i] = 0;
	ID section = kIdPhase;
	bool frnt = true;
	bool east = false;
	bool port = false;
	bool tran = false;
	Hero *h;
	ID id;

	char tmpStr[kLineMax + 1];
	mergeExt(tmpStr, fname, kSprExt);

	if (_resman->exist(tmpStr)) { // sprite description file exist
		EncryptedStream sprf(this, tmpStr);
		if (sprf.err())
			error("Bad SPR [%s]", tmpStr);

		int label = kNoByte;
		Common::String line;

		for (line = sprf.readLine(); !sprf.eos(); line = sprf.readLine()){
			int len = line.size();
			if (len == 0 || *tmpStr == ';')
				continue;
			
			Common::strlcpy(tmpStr, line.c_str(), sizeof(tmpStr));
			
			char *p;
			p = token(tmpStr);
			if (*p == '@') {
				if (label != kNoByte)
					badLab(fname);
				label = atoi(p + 1);
				continue;
			}

			id = ident(p);
			switch (id) {
			case kIdName: // will be taken in Expand routine
				if (label != kNoByte)
					badLab(fname);
				break;
			case kIdType:
				if (label != kNoByte)
					badLab(fname);
				break;
			case kIdNear:
			case kIdMTake:
			case kIdFTake:
			case kIdPhase:
			case kIdSeq:
				if (label != kNoByte)
					badLab(fname);
				section = id;
				break;
			case kIdFront:
				if (label != kNoByte)
					badLab(fname);
				p = token(nullptr);
				frnt = testBool(p);
				break;
			case kIdEast:
				if (label != kNoByte)
					badLab(fname);
				p = token(nullptr);
				east = testBool(p);
				break;
			case kIdPortable:
				if (label != kNoByte)
					badLab(fname);
				p = token(nullptr);
				port = testBool(p);
				break;
			case kIdTransparent:
				if (label != kNoByte)
					badLab(fname);
				p = token(nullptr);
				tran = testBool(p);
				break;
			default:
				if (id >= kIdNear)
					break;
				switch (section) {
				case kIdNear:
				case kIdMTake:
				case kIdFTake:
					if (_commandHandler->com(p) >= 0)
						++cnt[section];
					else
						error("Bad line %d [%s]", sprf.getLineCount(), tmpStr);
					break;
				case kIdPhase:
					if (label != kNoByte)
						badLab(fname);
					++shpcnt;
					break;
				case kIdSeq:
					if (label != kNoByte)
						badLab(fname);
					++seqcnt;
					break;
				}
				break;
			}
			label = kNoByte;
		}

		if (!shpcnt) {
			error("No shapes - %s", fname);
		}
	} else // No sprite description: mono-shaped sprite with only .BMP file.
		++shpcnt;

	// Make sprite of choosen type:
	char c = *fname | 0x20;
	if (c >= 'a' && c <= 'z' && fname[1] == '0' && fname[2] == '\0') {
		h = new Hero(this);
		if (h) {
			h->gotoxyz(pos);
			_sprite = h;
		}
	} else {
		if (_sprite)
			delete _sprite;
		_sprite = new Sprite(this);
		if (_sprite)
			_sprite->gotoxyz(pos);
	}

	if (_sprite) {
		_sprite->_ref = ref;

		_sprite->_flags._frnt = frnt;
		_sprite->_flags._east = east;
		_sprite->_flags._port = port;
		_sprite->_flags._tran = tran;
		_sprite->_flags._kill = true;

		// Extract the filename, without the extension
		Common::strlcpy(_sprite->_file, fname, sizeof(_sprite->_file));
		char *p = strchr(_sprite->_file, '.');
		if (p)
			*p = '\0';

		_sprite->_shpCnt = shpcnt;
		_sprite->_seqPtr = seqcnt;

		for (int i = 0; i < kActions; i++)
			_sprite->_actionCtrl[i]._cnt = cnt[i];
	}
}

void CGE2Engine::loadScript(const char *fname) {
	EncryptedStream scrf(this, fname);

	if (scrf.err())
		return;

	bool ok = true;
	int lcnt = 0;

	char tmpStr[kLineMax + 1];
	Common::String line;

	for (line = scrf.readLine(); !scrf.eos(); line = scrf.readLine()) {
		char *p;

		lcnt++;
		Common::strlcpy(tmpStr, line.c_str(), sizeof(tmpStr));
		if ((line.size() == 0) || (*tmpStr == ';')) // Comments start with ';' - don't bother with them.
			continue;

		ok = false; // not OK if break

		V3D P;

		// sprite ident number
		if ((p = token(tmpStr)) == NULL)
			break;
		int SpI = number(p);

		// sprite file name
		char *SpN;
		if ((SpN = token(nullptr)) == NULL)
			break;

		// sprite scene
		if ((p = token(nullptr)) == NULL)
			break;
		int SpA = number(p);

		// sprite column
		if ((p = token(nullptr)) == NULL)
			break;
		P._x = number(p);

		// sprite row
		if ((p = token(nullptr)) == NULL)
			break;
		P._y = number(p);

		// sprite Z pos
		if ((p = token(nullptr)) == NULL)
			break;
		P._z = number(p);

		// sprite life
		if ((p = token(nullptr)) == NULL)
			break;
		bool BkG = number(p) == 0;

		ok = true; // no break: OK

		_sprite = NULL;
		loadSprite(SpN, SpI, SpA, P);
		if (_sprite) {
			if (BkG)
				_sprite->_flags._back = true;

			int n = _spare->count();
			if (_spare->locate(_sprite->_ref) == nullptr)
				_spare->store(_sprite);
			_sprite = nullptr;
			if (_spare->count() == n)
				error("Durplicated reference! %s", SpN);
		}
	}

	if (!ok)
		error("Bad INI line %d [%s]", scrf.getLineCount(), fname);
}

void CGE2Engine::movie(const char *ext) {
	assert(ext);

	if (_quitFlag)
		return;

	char fn[12];
	sprintf(fn, "CGE.%s", (*ext == '.') ? ext + 1 : ext);

	if (_resman->exist(fn)) {
		int now = _now;
		_now = atoi(ext + 2);
		loadScript(fn);
		caveUp(_now);

		warning("STUB: CGE2Engine::movie()");

		_now = now;
	}
}

void CGE2Engine::caveUp(int cav) {
	_now = cav;
	int bakRef = _now << 8;
	if (_music)
		_midiPlayer->loadMidi(bakRef);
	showBak(bakRef);
	_eye = _eyeTab[_now];
	_mouseTop = V2D(this, V3D(0, 1, kScrDepth)).y;
	loadTab();
	_spare->takeCave(bakRef);
	openPocket();
	

	_vga->update();

	warning("STUB: CGE2Engine::caveUp()");
}

void CGE2Engine::showBak(int ref) {
	Sprite *spr = _spare->locate(ref);
	if (spr != nullptr) {
		_bitmapPalette = _vga->_sysPal;
		spr->expand();
		_bitmapPalette = NULL;
		spr->show(2);
		_vga->copyPage(1, 2);
		_spare->dispose(spr);
	}
}

void CGE2Engine::loadMap(int cav) {
	warning("STUB:  CGE2Engine::loadMap()");
}

void CGE2Engine::openPocket() {
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < kPocketMax + 1; j++) {
			int ref = (_heroTab[i])->_pocket[j]->_ref;
			_heroTab[i]->_pocket[j] = (ref == -1) ? nullptr : _vga->_showQ->locate(ref);
		}
	}
}

void CGE2Engine::runGame() {
	warning("STUB: CGE2Engine::runGame()");
}

void CGE2Engine::loadTab() {
	setEye(_text->getText(240));
	for (int i = 0; i < kCaveMax; i++)
		_eyeTab[i] == _eye;

	if (_resman->exist(kTabName)) {
		EncryptedStream f(this, kTabName);
		Common::File output;
		for (int i = 0; i < kCaveMax; i++) {
			for (int j = 0; j < 3; j++) {
				signed b = f.readSigned();
				unsigned a = f.readUnsigned();
				uint16 round = uint16((long(a) << 16) / 100);
				
				if (round > 0x7FFF)
					b++;
			
				switch (j) {
				case 0:
					_eyeTab[i]->_x = b;
					break;
				case 1:
					_eyeTab[i]->_y = b;
					break;
				case 2:
					_eyeTab[i]->_z = b;
					break;
				}
				
			}
		}
	}

	warning("STUB: CGE2Engine::loadTab() - Recheck this");
}

void CGE2Engine::cge2_main() {
	warning("STUB: CGE2Engine::cge2_main()");

	loadTab();

	_mode++;

	if (showTitle("WELCOME")) {
		if (_mode == 1)
			movie(kIntroExt);
		if (_text->getText(255) != NULL) {
			runGame();
			_startupMode = 2;
		} else
			_vga->sunset();
	} else
		_vga->sunset();
}

char *CGE2Engine::mergeExt(char *buf, const char *name, const char *ext) {
	strcpy(buf, name);
	char *dot = strrchr(buf, '.');
	if (!dot)
		strcat(buf, ext);

	return buf;
}

void CGE2Engine::setEye(V3D &e) { 
	_eye = &e;
}

void CGE2Engine::setEye(const V2D& e2, int z) {
	_eye->_x = e2.x;
	_eye->_y = e2.y;
	_eye->_z = z;
}

void CGE2Engine::setEye(const char *s) {
	char tempStr[kLineMax];
	strcpy(tempStr, s);
	_eye->_x = atoi(token(tempStr));
	_eye->_y = atoi(token(NULL));
	_eye->_z = atoi(token(NULL));
}

int CGE2Engine::newRandom(int range) {
	if (!range)
		return 0;

	return _randomSource.getRandomNumber(range - 1);
}

bool CGE2Engine::showTitle(const char *name) {
	if (_quitFlag)
		return false;

	_bitmapPalette = _vga->_sysPal;
	BitmapPtr *LB = new BitmapPtr[2];
	LB[0] = new Bitmap(this, name);
	LB[1] = NULL;
	_bitmapPalette = NULL;

	Sprite D(this, LB, 1);
	D._flags._kill = true;
	warning("STUB: Sprite::showTitle() - Flags changed compared to CGE1's Sprite type.");
	D.gotoxyz(kScrWidth >> 1, -(kPanHeight >> 1));
	_vga->sunset();

	D.show(2);

	_vga->copyPage(1, 2);
	_vga->copyPage(0, 1);

	_vga->sunrise(_vga->_sysPal);

	_vga->update();

	warning("STUB: CGE2Engine::showTitle()");

	return true;
}

} // End of namespace CGE2
