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

#include "common/debug.h"
#include "common/rect.h"
#include "common/textconsole.h"
#include "common/str.h"
#include "common/stream.h"
#include "common/system.h"
#include "common/platform.h"
#include "graphics/managed_surface.h"
#include "graphics/palette.h"
#include "graphics/surface.h"
#include "dgds/dgds.h"
#include "dgds/scene.h"
#include "dgds/font.h"
#include "dgds/image.h"
#include "dgds/includes.h"
#include "dgds/scripts.h"
#include "dgds/sound.h"
#include "dgds/resource.h"
#include "dgds/parser.h"

namespace Dgds {

// FIXME: Move these into some state
static Common::String _bmpNames[16];

TTMInterpreter::TTMInterpreter(DgdsEngine *vm) : _vm(vm), _drawWin(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT), _currentBmpId(0) {}

bool TTMInterpreter::load(const Common::String &filename, TTMData *scriptData) {
	TTMParser dgds(_vm->getResourceManager());
	return dgds.parse(scriptData, filename);
}

void TTMInterpreter::unload(TTMData *data) {
	if (!data)
		return;
	delete data->scr;

	data->scr = nullptr;
}

void TTMInterpreter::init(TTMState *state, const TTMData *data) {
	state->dataPtr = data;
	state->delay = 0;
	state->scene = 0;
	data->scr->seek(0);
}

bool TTMInterpreter::run(TTMState *script) {
	if (!script)
		return false;

	Common::SeekableReadStream *scr = script->dataPtr->scr;
	if (!scr)
		return false;
	if (scr->pos() >= scr->size())
		return false;

	script->delay = 0;
	do {
		uint16 code;
		byte count;
		uint op;
		int16 ivals[8];

		Common::String sval;

		code = scr->readUint16LE();
		count = code & 0x000F;
		op = code & 0xFFF0;

		debugN("\tOP: 0x%4.4x %2u ", op, count);
		if (count == 0x0F) {
			byte ch[2];

			do {
				ch[0] = scr->readByte();
				ch[1] = scr->readByte();
				if (ch[0])
					sval += ch[0];
				if (ch[1])
					sval += ch[1];
			} while (ch[0] != 0 && ch[1] != 0);

			debugN("\"%s\"", sval.c_str());
		} else {
			for (byte i = 0; i < count; i++) {
				ivals[i] = scr->readSint16LE();
				if (i > 0)
					debugN(", ");
				debugN("%d", ivals[i]);
			}
		}
		debug(" ");

		Common::Rect bmpWin(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

		switch (op) {
		case 0x0000:
			// FINISH:	void
			break;
		case 0xf010:
			// LOAD SCR:	filename:str
			_vm->_image->drawScreen(sval, _vm->getBottomBuffer());
			continue;
		case 0xf020:
			// LOAD BMP:	filename:str
			_bmpNames[_currentBmpId] = sval;
			continue;
		case 0xf050:
			// LOAD PAL:	filename:str
			_vm->_image->loadPalette(sval);
			continue;
		case 0xf060:
			// LOAD SONG:	filename:str
			if (_vm->_platform == Common::kPlatformAmiga) {
				_vm->_soundPlayer->playAmigaSfx("DYNAMIX.INS", 0, 255);
			} else if (_vm->_platform == Common::kPlatformMacintosh) {
				_vm->_soundPlayer->playMacMusic(sval.c_str());
			} else {
				_vm->_soundPlayer->playMusic(sval.c_str());
			}
			continue;

		case 0x1030: {
			// SET BMP:	id:int [-1:n]
			int bk = ivals[0];
			if (bk != -1) {
				_vm->_image->loadBitmap(_bmpNames[_currentBmpId], bk);
			}
			continue;
		}
		case 0x1050:
			// SELECT BMP:	    id:int [0:n]
			_currentBmpId = ivals[0];
			continue;
		case 0x1060:
			// SELECT SCR|PAL:  id:int [0]
			warning("Switching scene %d -> %d for opcode 0x1060 .. is that right?", script->scene, ivals[0]);
			script->scene = ivals[0];
			continue;
		case 0x1090:
			// SELECT SONG:	    id:int [0]
			continue;

		case 0x4120:
			// FADE IN:	?,?,?,?:byte
			_vm->_image->setPalette();
			continue;

		case 0x4110:
			// FADE OUT:	?,?,?,?:byte
			g_system->delayMillis(script->delay);
			_vm->_image->clearPalette();
			_vm->getBottomBuffer().fillRect(Common::Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT), 0);
			continue;

		// these 3 ops do interaction between the topBuffer (imgData) and the bottomBuffer (scrData) but... it might turn out this uses z values!
		case 0xa050: { //GFX?	    i,j,k,l:int	[i<k,j<l] // HAPPENS IN INTRO.TTM:INTRO9
			// it works like a bitblit, but it doesn't write if there's something already at the destination?
			_vm->_resData.blitFrom(_vm->getBottomBuffer());
			_vm->_resData.transBlitFrom(_vm->getTopBuffer());
			_vm->getTopBuffer().copyFrom(_vm->_resData);
			continue;
		}
		case 0x0020: //SAVE BG?:    void // OR PERHAPS SWAPBUFFERS ; it makes bmpData persist in the next frames.
			_vm->getBottomBuffer().copyFrom(_vm->getTopBuffer());
			continue;

		case 0x4200: {
			// STORE AREA:	x,y,w,h:int [0..n]		; it makes this area of bmpData persist in the next frames.
			const Common::Rect destRect(ivals[0], ivals[1], ivals[0] + ivals[2], ivals[1] + ivals[3]);
			_vm->_resData.blitFrom(_vm->getBottomBuffer());
			_vm->_resData.transBlitFrom(_vm->getTopBuffer());
			_vm->getBottomBuffer().copyRectToSurface(_vm->_resData, destRect.left, destRect.top, destRect);
		}
			continue;

		case 0x0ff0: {
			// REFRESH:	void
			_vm->_resData.blitFrom(_vm->getBottomBuffer());
			Graphics::Surface bmpSub = _vm->getTopBuffer().getSubArea(bmpWin);
			_vm->_resData.transBlitFrom(bmpSub, Common::Point(bmpWin.left, bmpWin.top));
			_vm->getTopBuffer().fillRect(bmpWin, 0);

			if (!_text.str.empty()) {
				Common::StringArray lines;
				const PFont *fntP = _vm->getFntP();
				const int h = fntP->getFontHeight();

				fntP->wordWrapText(_text.str, SCREEN_HEIGHT, lines);
				Common::Rect r(Common::Point(_text.rect.x, _text.rect.y), _text.rect.width, _text.rect.height);
				_vm->_resData.fillRect(r, 15);
				for (uint i = 0; i < lines.size(); i++) {
					const int w = fntP->getStringWidth(lines[i]);
					fntP->drawString(&_vm->_resData, lines[i], _text.rect.x, _text.rect.y + 1 + i * h, w, 0);
				}
			}
		} break;

		case 0xa520:
			//DRAW BMP: x,y:int ; happens once in INTRO.TTM
		case 0xa500:
			debug("DRAW \"%s\"", _bmpNames[_currentBmpId].c_str());

			// DRAW BMP: x,y,tile-id,bmp-id:int [-n,+n] (CHINA)
			// This is kind of file system intensive, will likely have to change to store all the BMPs.
			if (count == 4) {
				int bk = ivals[2];
				_currentBmpId = ivals[3];
				if (bk != -1) {
					_vm->_image->loadBitmap(_bmpNames[_currentBmpId], bk);
				}
			} else if (!_vm->_image->isLoaded()) {
				// load on demand?
				warning("trying to load bmp %d (%s) on demand", _currentBmpId, _bmpNames[_currentBmpId].c_str());
				_vm->_image->loadBitmap(_bmpNames[_currentBmpId], 0);
			}

			// DRAW BMP: x,y:int [-n,+n] (RISE)
			if (_vm->_image->isLoaded())
				_vm->_image->drawBitmap(ivals[0], ivals[1], _drawWin, _vm->getTopBuffer());
			else
				warning("request to draw null img at %d %d", ivals[0], ivals[1]);
			continue;

		case 0x1110: { //SET SCENE?:  i:int   [1..n]
			// DESCRIPTION IN TTM TAGS.
			debug("SET SCENE: %u", ivals[0]);
			script->scene = ivals[0];

			const Common::Array<Dialogue> dialogues = _vm->getScene()->getLines();
			_text.str.clear();
			for (const Dialogue &dialogue: dialogues) {
				if (dialogue.num == ivals[0])
					_text = dialogue;
			}
			if (!_text.str.empty())
				script->delay += 1500;
			continue;
		}

		case 0x4000:
			//SET WINDOW? x,y,w,h:int	[0..320,0..200]
			_drawWin = Common::Rect(ivals[0], ivals[1], ivals[2], ivals[3]);
			continue;

		case 0xa100:
			//SET WINDOW? x,y,w,h:int	[0..320,0..200]
			bmpWin = Common::Rect(ivals[0], ivals[1], ivals[0] + ivals[2], ivals[1] + ivals[3]);
			continue;

		case 0x1020: //DELAY?:	    i:int   [0..n]
			script->delay += ivals[0] * 10;
			continue;

		case 0x10a0: //SET SCENE?:  i:int   [0..n], often 0, called on scene change?

		case 0x2000: //SET FRAME1?: i,j:int [0..255]

		case 0xa530: // CHINA
		             // DRAW BMP4:	x,y,tile-id,bmp-id:int	[-n,+n] (CHINA)
		             // arguments similar to DRAW BMP but it draws the same BMP multiple times with radial simmetry? you can see this in the Dynamix logo star.
		case 0x0110: //PURGE IMGS?  void
		case 0x0080: //DRAW BG:	    void
		case 0x1100: //?	    i:int   [9]
		case 0x1300: //?	    i:int   [72,98,99,100,107]

		case 0x1310: //?	    i:int   [107]

		default:
			warning("Unimplemented TTM opcode: 0x%04X (%d args) (ivals: %d %d %d %d)", op, count, ivals[1], ivals[2], ivals[3], ivals[4]);
			continue;
		}
		break;
	} while (scr->pos() < scr->size());

	Graphics::Surface *dst = g_system->lockScreen();
	dst->copyRectToSurface(_vm->_resData, 0, 0, Common::Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));
	g_system->unlockScreen();
	g_system->updateScreen();
	g_system->delayMillis(script->delay);
	return true;
}

ADSInterpreter::ADSInterpreter(DgdsEngine *vm) : _vm(vm), _scriptData(nullptr) {}

bool ADSInterpreter::load(const Common::String &filename, ADSData *scriptData) {
	_scriptData = scriptData;
	_filename = filename;

	ADSParser dgds(_vm->getResourceManager());
	dgds.parse(scriptData, filename);

	TTMInterpreter interp(_vm);

	TTMData *scriptDatas;
	scriptDatas = new TTMData[_scriptData->count];
	assert(scriptDatas);
	_scriptData->scriptDatas = scriptDatas;

	for (uint16 i = _scriptData->count; i--;)
		interp.load(_scriptData->names[i], &_scriptData->scriptDatas[i]);

	_scriptData->filename = filename;
	_scriptData = nullptr;
	return true;
}

void ADSInterpreter::unload(ADSData *data) {
	if (!data)
		return;
	data->names.clear();
	delete data->scriptDatas;
	delete data->scr;

	data->count = 0;
	data->scriptDatas = 0;
	data->scr = 0;
}

void ADSInterpreter::init(ADSState *state, const ADSData *data) {
	state->dataPtr = data;
	state->scene = 0;
	state->subIdx = 0;
	state->subMax = 0;
	data->scr->seek(0);

	TTMInterpreter interp(_vm);

	state->scriptStates.resize(data->count);

	for (uint16 i = data->count; i--;)
		interp.init(&state->scriptStates[i], &data->scriptDatas[i]);
}

bool ADSInterpreter::run(ADSState *script) {

	if (script->subMax != 0) {
		TTMInterpreter interp(_vm);
		TTMState *ttmState = &script->scriptStates[script->subIdx - 1];
		if (!interp.run(ttmState) || ttmState->scene >= script->subMax)
			script->subMax = 0;
		return true;
	}

	if (!script)
		return false;
	Common::SeekableReadStream *scr = script->dataPtr->scr;
	if (!scr)
		return false;
	if (scr->pos() >= scr->size())
		return false;

	do {
		uint16 code = scr->readUint16LE();
		byte count = code & 0x000F;
		//uint op = code & 0xFFF0;

		if ((code & 0xFF00) == 0) {
			continue;
		}

		switch (code) {
		case 0x2005: {
			// play scene.
			script->subIdx = scr->readUint16LE();
			script->subMax = scr->readUint16LE();
			uint16 unk1 = scr->readUint16LE();
			uint16 unk2 = scr->readUint16LE();
			debug("ADSInterpreter play scene - subIdx: %d, subMax: %d, unk1: %d, unk2: %d", script->subIdx, script->subMax, unk1, unk2);
			return true;
		}

		case 0xF010:
		case 0xF200:
		case 0xFDA8:
		case 0xFE98:
		case 0xFF88:
		case 0xFF10:
		case 0xFFFF:
		case 0x0190:
		case 0x1070:
		case 0x1330:
		case 0x1340:
		case 0x1350:
		case 0x1360:
		case 0x1370:
		case 0x1420:
		case 0x1430:
		case 0x1500:
		case 0x1510:
		case 0x1520:
		case 0x2000:
		case 0x2010:
		case 0x2020:
		case 0x3010:
		case 0x3020:
		case 0x30FF:
		case 0x4000:
		case 0x4010:
		default:
			warning("Unimplemented ADS opcode: 0x%04X (count %d)", code, count);
			continue;
		}
		break;
	} while (scr->pos() < scr->size());
	return false;
}

} // End of namespace Dgds
