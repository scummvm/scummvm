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
			p = EncryptedStream::token(tmpStr);
			if (*p == '@') {
				if (label != kNoByte)
					badLab(fname);
				label = atoi(p + 1);
				continue;
			}

			id = EncryptedStream::ident(p);
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
				p = EncryptedStream::token(nullptr);
				frnt = EncryptedStream::testBool(p);
				break;
			case kIdEast:
				if (label != kNoByte)
					badLab(fname);
				p = EncryptedStream::token(nullptr);
				east = EncryptedStream::testBool(p);
				break;
			case kIdPortable:
				if (label != kNoByte)
					badLab(fname);
				p = EncryptedStream::token(nullptr);
				port = EncryptedStream::testBool(p);
				break;
			case kIdTransparent:
				if (label != kNoByte)
					badLab(fname);
				p = EncryptedStream::token(nullptr);
				tran = EncryptedStream::testBool(p);
				break;
			default:
				if (id >= kIdNear)
					break;
				switch (section) {
				case kIdNear:
				case kIdMTake:
				case kIdFTake:
					if (Snail::com(p) >= 0)
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
		if ((p = EncryptedStream::token(tmpStr)) == NULL)
			break;
		int SpI = EncryptedStream::number(p);

		// sprite file name
		char *SpN;
		if ((SpN = EncryptedStream::token(nullptr)) == NULL)
			break;

		// sprite scene
		if ((p = EncryptedStream::token(nullptr)) == NULL)
			break;
		int SpA = EncryptedStream::number(p);

		// sprite column
		if ((p = EncryptedStream::token(nullptr)) == NULL)
			break;
		P._x = EncryptedStream::number(p);

		// sprite row
		if ((p = EncryptedStream::token(nullptr)) == NULL)
			break;
		P._y = EncryptedStream::number(p);

		// sprite Z pos
		if ((p = EncryptedStream::token(nullptr)) == NULL)
			break;
		P._z = EncryptedStream::number(p);

		// sprite life
		if ((p = EncryptedStream::token(nullptr)) == NULL)
			break;
		bool BkG = EncryptedStream::number(p) == 0;

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

	warning("STUB: CGE2Engine::caveUp()");
}

void CGE2Engine::showBak(int ref) {
	Sprite *spr = _spare->locate(ref);
	if (spr != nullptr) {
		_bitmapPalette = _vga->_sysPal;
		warning("Check again this ^ !");
		spr->expand();
		_bitmapPalette = NULL;
		//spr->show(2);
		_vga->copyPage(1, 2);
		_spare->dispose(spr);

		warning("STUB: CGE2Engine::showBak()");
	}
}

void CGE2Engine::runGame() {
	warning("STUB: CGE2Engine::runGame()");
}

void CGE2Engine::cge2_main() {
	warning("STUB: CGE2Engine::cge2_main()");

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

} // End of namespace CGE2
