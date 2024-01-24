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

TTMInterpreter::TTMInterpreter(DgdsEngine *vm) : _vm(vm), _text(nullptr) {}

bool TTMInterpreter::load(const Common::String &filename) {
	TTMParser dgds(_vm->getResourceManager(), _vm->getDecompressor());
	bool parseResult = dgds.parse(&_scriptData, filename);

	_state.delay = 0;
	_state.scene = 0;
	_scriptData.scr->seek(0);

	return parseResult;
}

void TTMInterpreter::unload() {
	delete _scriptData.scr;
	_scriptData.scr = nullptr;
}

void TTMInterpreter::setActiveDialogue(uint16 num) {
	Common::Array<Dialogue> &dialogues = _vm->getScene()->getLines();
	_text = nullptr;
	for (Dialogue &dialogue: dialogues) {
		if (dialogue._num == num)
			_text = &dialogue;
		// HACK to get Dragon intro working
		if (!_text && dialogue._num == num + 14)
			_text = &dialogue;
	}
	if (_text && !_text->_str.empty())
		_state.delay += _text->_time * 9;  // More correctly, 9 - text-speed-setting
}

void TTMInterpreter::handleOperation(uint16 op, byte count, int16 *ivals, Common::String &sval) {
	Common::Rect bmpArea(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	switch (op) {
	case 0x0000:
		// FINISH:	void
		break;
	case 0x0020: // SAVE BACKGROUND
		_vm->getBottomBuffer().copyFrom(_vm->getTopBuffer());
		break;
	case 0x0080: // DRAW BACKGROUND
		_vm->getTopBuffer().copyFrom(_vm->getBottomBuffer());
		break;
	case 0x0110: // PURGE void
		// .. shouldn't actually clear the bmps, what should it do?
		_state._currentBmpId = 0;
		break;
	case 0x0ff0: {
		// REFRESH:	void
		_vm->_resData.blitFrom(_vm->getBottomBuffer());
		Graphics::Surface bmpSub = _vm->getTopBuffer().getSubArea(bmpArea);
		_vm->_resData.transBlitFrom(bmpSub, Common::Point(bmpArea.left, bmpArea.top));
		_vm->getTopBuffer().fillRect(bmpArea, 0);

		if (_text) {
			_text->draw(_vm->_resData.surfacePtr(), 1);
			_text->draw(_vm->_resData.surfacePtr(), 4);
		}
	} break;
	case 0x1020: // SET DELAY:	    i:int   [0..n]
		_state.delay += ivals[0] * 10;
		break;
	case 0x1030: {
		// SET BMP:	id:int [-1:n]
		int bk = ivals[0];
		if (bk != -1) {
			_vm->_image->loadBitmap(_state.bmpNames[_state._currentBmpId], bk);
		}
		break;
	}
	case 0x1050:
		// SELECT BMP:	    id:int [0:n]
		_state._currentBmpId = ivals[0];
		break;
	case 0x1060:
		// SELECT PAL:  id:int [0]
		warning("TTM: Switching palette to %d for opcode 0x1060, but we don't use it yet", ivals[0]);
		_state._currentPalId = ivals[0];
		break;
	case 0x1090:
		// SELECT SONG:	    id:int [0]
		break;
	case 0x10a0: // SET SCENE?:  i:int   [0..n], often 0, called on scene change?
		debug("SET SCENE: %u", ivals[0]);
		break;
	case 0x1110: { // SHOW SCENE TEXT?:  i:int   [1..n]
		// DESCRIPTION IN TTM TAGS.
		debug("SHOW SCENE TEXT: %u", ivals[0]);
		_state.scene = ivals[0];
		setActiveDialogue(_state.scene);
		break;
	}
	case 0x1200: // GOTO
		debug("GOTO SCENE: %u", ivals[0]);
		_state.scene = ivals[0];
		break;
	case 0x2000: // SET (DRAW) COLORS: fgcol,bgcol:int [0..255]
		_state._drawColFG = static_cast<byte>(ivals[0]);
		_state._drawColBG = static_cast<byte>(ivals[1]);
		break;
	case 0x4000:
		// SET DRAW WINDOW? x,y,w,h:int	[0..320,0..200]
		_state._drawWin = Common::Rect(ivals[0], ivals[1], ivals[2], ivals[3]);
		break;
	case 0x4110:
		// FADE OUT:	colorno,ncolors,coloffset,speed:byte
		g_system->delayMillis(_state.delay);
		_vm->_image->clearPalette();
		_vm->getBottomBuffer().fillRect(Common::Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT), 0);
		break;
	case 0x4120:
		// FADE IN:	colorno,ncolors,coloffset,speed:byte
		warning("TTM: FADE IN, implement me");
		_vm->_image->setPalette();
		break;
	case 0x4200: {
		// STORE AREA:	x,y,w,h:int [0..n]		; it makes this area of bmpData persist in the next frames.
		const Common::Rect destRect(ivals[0], ivals[1], ivals[0] + ivals[2], ivals[1] + ivals[3]);
		_vm->_resData.blitFrom(_vm->getBottomBuffer());
		_vm->_resData.transBlitFrom(_vm->getTopBuffer());
		_vm->getBottomBuffer().copyRectToSurface(_vm->_resData, destRect.left, destRect.top, destRect);
		break;
	}
	case 0xa000: // DRAW PIXEL x,y:int
		_vm->getTopBuffer().setPixel(ivals[0], ivals[1], _state._drawColFG);
		break;
	case 0xa050: // SAVE REGION    i,j,k,l:int	[i<k,j<l]
		// it works like a bitblit, but it doesn't write if there's something already at the destination?
		_vm->_resData.blitFrom(_vm->getBottomBuffer());
		_vm->_resData.transBlitFrom(_vm->getTopBuffer());
		_vm->getTopBuffer().copyFrom(_vm->_resData);
		break;
	case 0xa0a0: // DRAW LINE  x1,y1,x2,y2:int
		_vm->getTopBuffer().drawLine(ivals[0], ivals[1], ivals[2], ivals[3], _state._drawColFG);
		break;
	case 0xa100:
		// DRAW FILLED RECT x,y,w,h:int	[0..320,0..200]
		bmpArea = Common::Rect(ivals[0], ivals[1], ivals[0] + ivals[2], ivals[1] + ivals[3]);
		_vm->getTopBuffer().fillRect(bmpArea, _state._drawColFG);
		break;
	case 0xa520:
		// DRAW SPRITE FLIP: x,y:int ; happens once in INTRO.TTM
		// FALL THROUGH
	case 0xa530:
		// CHINA
		// DRAW BMP4:	x,y,tile-id,bmp-id:int	[-n,+n] (CHINA)
		// arguments similar to DRAW BMP but it draws the same BMP multiple times with radial simmetry? you can see this in the Dynamix logo star.
		// FALL THROUGH
	case 0xa500:
		debug("DRAW \"%s\"", _state.bmpNames[_state._currentBmpId].c_str());

		// DRAW BMP: x,y,tile-id,bmp-id:int [-n,+n] (CHINA)
		// This is kind of file system intensive, will likely have to change to store all the BMPs.
		if (count == 4) {
			int tileId = ivals[2];
			_state._currentBmpId = ivals[3];
			if (tileId != -1) {
				_vm->_image->loadBitmap(_state.bmpNames[_state._currentBmpId], tileId);
			}
		} else if (!_vm->_image->isLoaded()) {
			// load on demand?
			warning("trying to load bmp %d (%s) on demand", _state._currentBmpId, _state.bmpNames[_state._currentBmpId].c_str());
			_vm->_image->loadBitmap(_state.bmpNames[_state._currentBmpId], 0);
		}

		// DRAW BMP: x,y:int [-n,+n] (RISE)
		if (_vm->_image->isLoaded())
			_vm->_image->drawBitmap(ivals[0], ivals[1], _state._drawWin, _vm->getTopBuffer());
		else
			warning("request to draw null img at %d %d", ivals[0], ivals[1]);
		break;
	case 0xf010:
		// LOAD SCR:	filename:str
		_vm->_image->drawScreen(sval, _vm->getBottomBuffer());
		break;
	case 0xf020:
		// LOAD BMP:	filename:str
		_state.bmpNames[_state._currentBmpId] = sval;
		break;
	case 0xf050:
		// LOAD PAL:	filename:str
		_vm->_image->loadPalette(sval);
		break;
	case 0xf060:
		// LOAD SONG:	filename:str
		if (_vm->_platform == Common::kPlatformAmiga) {
			_vm->_soundPlayer->playAmigaSfx("DYNAMIX.INS", 0, 255);
		} else if (_vm->_platform == Common::kPlatformMacintosh) {
			_vm->_soundPlayer->playMacMusic(sval.c_str());
		} else {
			_vm->_soundPlayer->playMusic(sval.c_str());
		}
		break;

	// Unimplemented / unknown
	case 0x0070: // ? (0 args)
	case 0x0230: // ? (0 args) - found in HoC intro
	case 0x1100: // ?	    i:int   [9]
	case 0x1120: // SET_BACKGROUND
	case 0x1300: // ? (1 args) - found in Dragon + HoC intro
	case 0x1310: // ?	    i:int   [107]
	case 0x2010: // SET FRAME
	case 0x2020: // SET TIMER
	case 0x4210: // SAVE IMAGE REGION
	// case 0xa100: // DRAW FILLED RECT  x1,y1,x2,y2:int
	case 0xa110: // DRAW EMPTY RECT  x1,y1,x2,y2:int
	case 0xa300: // DRAW some string? x,y,w,h:int
	case 0xa400: // DRAW FILLED CIRCLE
	case 0xa424: // DRAW EMPTY CIRCLE
	case 0xa510: // DRAW SPRITE1
	case 0xa600: // CLEAR SCREEN
	case 0xb000: // ? (0 args) - found in HoC intro
	case 0xb010: // ? (3 args: 30, 2, 19) - found in HoC intro
	case 0xb600: // DRAW SCREEN
	case 0xc020: // LOAD_SAMPLE
	case 0xc030: // SELECT_SAMPLE
	case 0xc040: // DESELECT_SAMPLE
	case 0xc050: // PLAY_SAMPLE
	case 0xc060: // STOP_SAMPLE

	default:
		warning("Unimplemented TTM opcode: 0x%04X (%d args) (ivals: %d %d %d %d)", op, count, ivals[0], ivals[1], ivals[2], ivals[3]);
		break;
	}
}

bool TTMInterpreter::run() {
	Common::SeekableReadStream *scr = _scriptData.scr;
	if (!scr)
		return false;
	if (scr->pos() >= scr->size())
		return false;

	_state.delay = 0;

	uint16 code = scr->readUint16LE();
	uint16 op = code & 0xFFF0;
	byte count = code & 0x000F;
	int16 ivals[8];
	Common::String sval;

	if (count > 8 && count != 0x0f)
		error("Invalid TTM opcode %04x requires %d locals", code, count);

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

	handleOperation(op, count, ivals, sval);

	g_system->copyRectToScreen(_vm->_resData.getPixels(), SCREEN_WIDTH, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	g_system->updateScreen();
	g_system->delayMillis(_state.delay);

	return true;
}

ADSInterpreter::ADSInterpreter(DgdsEngine *vm) : _vm(vm) {
	_ttmInterpreter = new TTMInterpreter(_vm);
}

ADSInterpreter::~ADSInterpreter() {
	delete _ttmInterpreter;
	_ttmInterpreter = nullptr;
}

bool ADSInterpreter::load(const Common::String &filename) {
	unload();

	// For high detail, replace extension ADS with ADH.  Low detail is ADL.
	Common::String detailfile = filename.substr(0, filename.size() - 1);
	if (_vm->getDetailLevel() == kDgdsDetailLow)
		detailfile += "L";
	else
		detailfile += "H";

	if (!_vm->getResourceManager()->hasResource(detailfile))
		detailfile = filename;

	ADSParser dgds(_vm->getResourceManager(), _vm->getDecompressor());
	dgds.parse(&_scriptData, detailfile);

	_state.scene = 0;
	_state.subIdx = 0;
	_state.subMax = 0;
	_scriptData.scr->seek(0);
	_scriptData.filename = filename;

	return true;
}

void ADSInterpreter::unload() {
	_scriptData.names.clear();
	delete _scriptData.scr;
	_scriptData.scr = nullptr;
	_state = ADSState();
}

bool ADSInterpreter::run() {
	// This is the main scene player loop, which will run
	// after the first time the ADS script is loaded below
	// TODO/FIXME: Rewrite this
	if (_state.subMax != 0) {
		if (!_ttmInterpreter->run()) {
			const uint16 id = _state.subIdx - 1;
			if (id + 1 < _scriptData.names.size()) {
				_state.subIdx++;
				_ttmInterpreter->load(_scriptData.names[_state.subIdx - 1]);
			} else {
				return false;
			}
		}

		return true;
	}

	Common::SeekableReadStream *scr = _scriptData.scr;
	if (!scr)
		return false;
	if (scr->pos() >= scr->size())
		return false;

	do {
		uint16 code = scr->readUint16LE();
		//uint op = code & 0xFFF0;

		if ((code & 0xFF00) == 0) {
			continue;
		}

		switch (code) {
		case 0x2005: {
			// ADD SCENE
			_state.subIdx = scr->readUint16LE();
			_state.subMax = scr->readUint16LE();
			uint16 unk1 = scr->readUint16LE();
			uint16 unk2 = scr->readUint16LE();
			_ttmInterpreter->load(_scriptData.names[_state.subIdx - 1]);
			debug("ADSInterpreter add scene - subIdx: %d, subMax: %d, unk1: %d, unk2: %d", _state.subIdx, _state.subMax, unk1, unk2);
			return true;
		}
		case 0xFFFF:
			// END
			return false;

		case 0x1330: { // IF_NOT_PLAYED, 2 params
			uint16 unk1 = scr->readUint16LE();
			uint16 unk2 = scr->readUint16LE();
			warning("Unimplemented ADS opcode: IF_NOT_PLAYED %02x %02x", unk1, unk2);
			continue;
		}

		//// unknown / to-be-implemented
		case 0x0190:
		case 0x1070: // unknown, 2 params
		case 0x1340:
		case 0x1350: // IF_PLAYED, 2 params
		case 0x1360: // IF_NOT_RUNNING, 2 params
		case 0x1370: // IF_RUNNING, 2 params
		case 0x1420: // AND, 0 params
		case 0x1430: // OR, 0 params
		case 0x1500:
		case 0x1510: // PLAY_SCENE, 0 params
		case 0x1520: // PLAY_SCENE_2, 5 params
		case 0x2000:
		case 0x2010: // STOP_SCENE, 3 params
		case 0x2020:
		case 0x3010: // RANDOM_START, 0 params
		case 0x3020: // RANDOM_??, 1 param
		case 0x30FF: // RANDOM_END, 0 params
		case 0x4000: // unknown, 3 params
		case 0x4010:
		case 0xF010: // FADE_OUT, 0 params
		case 0xF200: // RUN_SCRIPT, 1 param
		case 0xFDA8:
		case 0xFE98:
		case 0xFF88:
		case 0xFF10:
		case 0xFFF0: // END_IF, 0 params
		default:
			warning("Unimplemented ADS opcode: 0x%04X", code);
			continue;
		}
		break;
	} while (scr->pos() < scr->size());

	return false;
}

} // End of namespace Dgds
