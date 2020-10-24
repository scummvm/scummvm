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
#include "dgds/font.h"
#include "dgds/image.h"
#include "dgds/includes.h"
#include "dgds/movies.h"
#include "dgds/resource.h"
#include "dgds/parser.h"

namespace Dgds {

// TODO: Move
extern Graphics::ManagedSurface resData;
extern Graphics::Surface bottomBuffer;
extern Graphics::Surface topBuffer;
extern Common::StringArray _bubbles;

const Common::Rect rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

int bk = -1;
int id = 0, sid = 0;
Common::String text;
extern PFont *_fntP;
Common::Rect drawWin(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
Common::String _bmpNames[16];

TTMInterpreter::TTMInterpreter(DgdsEngine *vm) : _vm(vm), _scriptData(0), _filename(0) {}

bool TTMInterpreter::load(const char *filename, TTMData *scriptData) {
	Common::SeekableReadStream *stream = _vm->_resource->getResource(filename);

	if (!stream) {
		error("Couldn't open script file '%s'", filename);
		return false;
	}

	memset(scriptData, 0, sizeof(*scriptData));
	_scriptData = scriptData;
	_filename = filename;

	TTMParser dgds(*stream, _filename);
	dgds.parse(scriptData, _vm->_decompressor);

	delete stream;

	Common::strlcpy(_scriptData->filename, filename, sizeof(_scriptData->filename));
	_scriptData = 0;
	_filename = 0;
	return true;
}

void TTMInterpreter::unload(TTMData *data) {
	if (!data)
		return;
	delete data->scr;

	data->scr = 0;
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
				sval += ch[0];
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
			_vm->_image->drawScreen(sval, bottomBuffer);
			continue;
		case 0xf020:
			// LOAD BMP:	filename:str
			_bmpNames[id] = Common::String(sval);
			continue;
		case 0xf050:
			// LOAD PAL:	filename:str
			_vm->_image->loadPalette(sval);
			continue;
		case 0xf060:
			// LOAD SONG:	filename:str
			if (_vm->_platform == Common::kPlatformAmiga) {
				byte volume = 255;
				byte channel = 0;
				_vm->stopSfx(channel);
				_vm->playSfx("DYNAMIX.INS", channel, volume);
			} else {
				_vm->playMusic(sval.c_str());
			}
			continue;

		case 0x1030:
			// SET BMP:	id:int [-1:n]
			bk = ivals[0];

			if (bk != -1) {
				_vm->_image->loadBitmap(_bmpNames[id], bk);
			}
			continue;
		case 0x1050:
			// SELECT BMP:	    id:int [0:n]
			id = ivals[0];
			continue;
		case 0x1060:
			// SELECT SCR|PAL:  id:int [0]
			sid = ivals[0];
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
			bottomBuffer.fillRect(rect, 0);
			continue;

		// these 3 ops do interaction between the topBuffer (imgData) and the bottomBuffer (scrData) but... it might turn out this uses z values!
		case 0xa050: { //GFX?	    i,j,k,l:int	[i<k,j<l] // HAPPENS IN INTRO.TTM:INTRO9
			// it works like a bitblit, but it doesn't write if there's something already at the destination?
			resData.blitFrom(bottomBuffer);
			resData.transBlitFrom(topBuffer);
			topBuffer.copyFrom(resData);
			continue;
		}
		case 0x0020: //SAVE BG?:    void // OR PERHAPS SWAPBUFFERS ; it makes bmpData persist in the next frames.
			bottomBuffer.copyFrom(topBuffer);
			continue;

		case 0x4200: {
			// STORE AREA:	x,y,w,h:int [0..n]		; it makes this area of bmpData persist in the next frames.
			const Common::Rect destRect(ivals[0], ivals[1], ivals[0] + ivals[2], ivals[1] + ivals[3]);
			resData.blitFrom(bottomBuffer);
			resData.transBlitFrom(topBuffer);
			bottomBuffer.copyRectToSurface(resData, destRect.left, destRect.top, destRect);
		}
			continue;

		case 0x0ff0: {
			// REFRESH:	void
			resData.blitFrom(bottomBuffer);
			Graphics::Surface bmpSub = topBuffer.getSubArea(bmpWin);
			resData.transBlitFrom(bmpSub, Common::Point(bmpWin.left, bmpWin.top));
			topBuffer.fillRect(bmpWin, 0);

			if (!text.empty()) {
				Common::StringArray lines;
				const int h = _fntP->getFontHeight();

				_fntP->wordWrapText(text, SCREEN_HEIGHT, lines);
				Common::Rect r(0, 7, SCREEN_WIDTH, h * lines.size() + 13);
				resData.fillRect(r, 15);
				for (uint i = 0; i < lines.size(); i++) {
					const int w = _fntP->getStringWidth(lines[i]);
					_fntP->drawString(&resData, lines[i], 10, 10 + 1 + i * h, w, 0);
				}
			}
		} break;

		case 0xa520:
			//DRAW BMP: x,y:int ; happens once in INTRO.TTM
		case 0xa500:
			debug("DRAW \"%s\"", _bmpNames[id].c_str());

			// DRAW BMP: x,y,tile-id,bmp-id:int [-n,+n] (CHINA)
			// This is kind of file system intensive, will likely have to change to store all the BMPs.
			if (count == 4) {
				bk = ivals[2];
				id = ivals[3];
				if (bk != -1) {
					_vm->_image->loadBitmap(_bmpNames[id], bk);
				}
			}

			// DRAW BMP: x,y:int [-n,+n] (RISE)
			_vm->_image->drawBitmap(ivals[0], ivals[1], drawWin, topBuffer);
			continue;

		case 0x1110: { //SET SCENE?:  i:int   [1..n]
			// DESCRIPTION IN TTM TAGS.
			debug("SET SCENE: %u", ivals[0]);
			script->scene = ivals[0];

			if (!_bubbles.empty()) {
				// TODO: Are these hardcoded?
				if (!scumm_stricmp(script->dataPtr->filename, "INTRO.TTM")) {
					switch (ivals[0]) {
					case 15:
						text = _bubbles[3];
						break;
					case 16:
						text = _bubbles[4];
						break;
					case 17:
						text = _bubbles[5];
						break;
					case 19:
						text = _bubbles[6];
						break;
					case 20:
						text = _bubbles[7];
						break;
					case 22:
						text = _bubbles[8];
						break;
					case 23:
						text = _bubbles[9];
						break;
					case 25:
						text = _bubbles[10];
						break;
					case 26:
						text = _bubbles[11];
						break;
					default:
						text.clear();
						break;
					}
				} else if (!scumm_stricmp(script->dataPtr->filename, "BIGTV.TTM")) {
					switch (ivals[0]) {
					case 1:
						text = _bubbles[0];
						break;
					case 2:
						text = _bubbles[1];
						break;
					case 3:
						text = _bubbles[2];
						break;
					}
				}
				if (!text.empty())
					script->delay += 1500;
			} else {
				text.clear();
			}
		}
			continue;

		case 0x4000:
			//SET WINDOW? x,y,w,h:int	[0..320,0..200]
			drawWin = Common::Rect(ivals[0], ivals[1], ivals[2], ivals[3]);
			continue;

		case 0xa100:
			//SET WINDOW? x,y,w,h:int	[0..320,0..200]
			bmpWin = Common::Rect(ivals[0], ivals[1], ivals[0] + ivals[2], ivals[1] + ivals[3]);
			continue;

		case 0x1020: //DELAY?:	    i:int   [0..n]
			script->delay += ivals[0] * 10;
			continue;

		case 0x10a0:
			// SET SCR|PAL:	    id:int [0]
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
			warning("Unimplemented TTM opcode: 0x%04X", op);
			continue;
		}
		break;
	} while (scr->pos() < scr->size());

	Graphics::Surface *dst;
	dst = g_system->lockScreen();
	dst->copyRectToSurface(resData, 0, 0, rect);
	g_system->unlockScreen();
	g_system->updateScreen();
	g_system->delayMillis(script->delay);
	return true;
}

ADSInterpreter::ADSInterpreter(DgdsEngine *vm) : _vm(vm), _scriptData(0), _filename(0) {}

bool ADSInterpreter::load(const char *filename, ADSData *scriptData) {
	Common::SeekableReadStream *stream = _vm->_resource->getResource(filename);

	if (!stream) {
		error("Couldn't open script file '%s'", filename);
		return false;
	}

	memset(scriptData, 0, sizeof(*scriptData));
	_scriptData = scriptData;
	_filename = filename;

	ADSParser dgds(*stream, _filename);
	dgds.parse(scriptData, _vm->_decompressor);

	delete stream;

	TTMInterpreter interp(_vm);

	TTMData *scriptDatas;
	scriptDatas = new TTMData[_scriptData->count];
	assert(scriptDatas);
	_scriptData->scriptDatas = scriptDatas;

	for (uint16 i = _scriptData->count; i--;)
		interp.load(_scriptData->names[i], &_scriptData->scriptDatas[i]);

	Common::strlcpy(_scriptData->filename, filename, sizeof(_scriptData->filename));
	_scriptData = 0;
	_filename = 0;
	return true;
}

void ADSInterpreter::unload(ADSData *data) {
	if (!data)
		return;
	for (uint16 i = data->count; i--;)
		delete data->names[i];
	delete data->names;
	delete data->scriptDatas;
	delete data->scr;

	data->count = 0;
	data->names = 0;
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

	TTMState *scriptStates;
	scriptStates = new TTMState[data->count];
	assert(scriptStates);
	state->scriptStates = scriptStates;

	for (uint16 i = data->count; i--;)
		interp.init(&state->scriptStates[i], &data->scriptDatas[i]);
}

bool ADSInterpreter::run(ADSState *script) {
	TTMInterpreter interp(_vm);

	if (script->subMax != 0) {
		TTMState *state = &script->scriptStates[script->subIdx - 1];
		if (!interp.run(state) || state->scene >= script->subMax)
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
		uint16 code;
		byte count;
		uint op;
		code = scr->readUint16LE();
		count = code & 0x000F;
		op = code & 0xFFF0;

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
		}
			return true;
		case 0xF010:
		case 0xF200:
		case 0xFDA8:
		case 0xFE98:
		case 0xFF88:
		case 0xFF10:
		case 0xFFFF:
		case 0x0190:
		case 0x1070:
		case 0x1340:
		case 0x1360:
		case 0x1370:
		case 0x1420:
		case 0x1430:
		case 0x1500:
		case 0x1520:
		case 0x2000:
		case 0x2010:
		case 0x2020:
		case 0x3010:
		case 0x3020:
		case 0x30FF:
		case 0x4000:
		case 0x4010:
		case 0x1510:
		case 0x1330:
		case 0x1350:
		default:
			warning("Unimplemented ADS opcode: 0x%04X", code);
			continue;
		}
		break;
	} while (scr->pos() < scr->size());
	return false;
}

} // End of namespace Dgds
