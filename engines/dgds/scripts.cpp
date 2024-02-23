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

TTMInterpreter::TTMInterpreter(DgdsEngine *vm) : _vm(vm) {}

bool TTMInterpreter::load(const Common::String &filename, TTMEnviro &scriptData) {
	TTMParser dgds(_vm->getResourceManager(), _vm->getDecompressor());
	bool parseResult = dgds.parse(&scriptData, filename);

	scriptData.scr->seek(0);

	return parseResult;
}

void TTMInterpreter::unload() {
}

void TTMInterpreter::updateScreen(struct TTMSeq &state) {
	g_system->copyRectToScreen(_vm->_resData.getPixels(), SCREEN_WIDTH, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	if (state._runFlag && _vm->getScene()->checkDialogActive()) {
		Graphics::Surface *screen = g_system->lockScreen();
		_vm->getScene()->drawActiveDialog(screen, 1);
		_vm->getScene()->drawActiveDialog(screen, 4);
		g_system->unlockScreen();
	}

	g_system->updateScreen();
}

void TTMInterpreter::handleOperation(TTMEnviro &env, struct TTMSeq &state, uint16 op, byte count, const int16 *ivals, const Common::String &sval) {
	Common::Rect bmpArea(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	switch (op) {
	case 0x0000:
		// FINISH:	void
		break;
	case 0x0020: // SAVE BACKGROUND
		_vm->getBottomBuffer().copyFrom(_vm->getTopBuffer());
		break;
	case 0x0070: // FREE PALETTE
		error("TODO: Implement me: free palette (current pal)");
		break;
	case 0x0080: // DRAW BACKGROUND
		_vm->getTopBuffer().copyFrom(_vm->getBottomBuffer());
		break;
	case 0x0090: // FREE FONT
		error("TODO: Implement me: free font (current one)");
		break;
	case 0x0110: // PURGE void
		_vm->adsInterpreter()->setHitTTMOp0110();
		break;
	case 0x0ff0: {
		// REFRESH:	void
		_vm->_resData.blitFrom(_vm->getBottomBuffer());
		Graphics::Surface bmpSub = _vm->getTopBuffer().getSubArea(bmpArea);
		_vm->_resData.transBlitFrom(bmpSub, Common::Point(bmpArea.left, bmpArea.top));
		_vm->getTopBuffer().fillRect(bmpArea, 0);
	} break;
	case 0x1020: // SET DELAY:	    i:int   [0..n]
		state._timeNext = g_system->getMillis() + ivals[0] * 10;
		break;
	case 0x1030: {
		// SET BMP:	id:int [-1:n]
		int bk = ivals[0];
		if (bk != -1) {
			_vm->_image->loadBitmap(env._scriptShapes[state._currentBmpId], bk);
		}
		break;
	}
	case 0x1050:
		// SELECT BMP:	    id:int [0:n]
		state._currentBmpId = ivals[0];
		break;
	case 0x1060:
		// SELECT PAL:  id:int [0]
		_vm->getGamePals()->selectPalNum(env._scriptPals[ivals[0]]);
		break;
	case 0x1090:
		// SELECT SONG:	    id:int [0]
		state._currentSongId = ivals[0];
		break;
	case 0x10a0: // SET SCENE?:  i:int   [0..n], often 0, called on scene change?
		debug("SCENE SETUP DONE: %u", ivals[0]);
		break;
	case 0x1100:   // SET_SCENE:  i:int   [1..n]
	case 0x1110: { // SET_SCENE:  i:int   [1..n]
		// DESCRIPTION IN TTM TAGS.
		debug("SET SCENE: %u", ivals[0]);
		break;
	}
	case 0x1200: // GOTO? How different to SET SCENE??
		debug("GOTO SCENE: %u", ivals[0]);
		state._currentFrame = ivals[0];
		break;
	case 0x1300: // PLAY SFX?    i:int - eg [72], found in Dragon + HoC intro
		debug("TODO: PLAY SFX?: %u", ivals[0]);
		break;
	case 0x2000: // SET (DRAW) COLORS: fgcol,bgcol:int [0..255]
		state._drawColFG = static_cast<byte>(ivals[0]);
		state._drawColBG = static_cast<byte>(ivals[1]);
		break;
	case 0x4000:
		// SET DRAW WINDOW? x,y,w,h:int	[0..320,0..200]
		state._drawWin = Common::Rect(ivals[0], ivals[1], ivals[2], ivals[3]);
		break;
	case 0x4110:
		// FADE OUT:	colorno,ncolors,targetcol,speed:byte
		if (ivals[3] == 0) {
			_vm->getGamePals()->clearPalette();
		} else {
			// The original tight-loops here with 640 steps and i/10 as the fade level..
			// bring that down a bit to use less cpu.
			for (int i = 0; i < 320; i += ivals[3]) {
				int fade = MIN(i / 5, 63);
				_vm->getGamePals()->setFade(ivals[0], ivals[1], ivals[2], fade * 4);
				updateScreen(state);
				g_system->delayMillis(5);
			}
		}
		_vm->getBottomBuffer().fillRect(Common::Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT), 0);
		break;
	case 0x4120: {
		// blt first?
		_vm->_resData.blitFrom(_vm->getBottomBuffer());
		Graphics::Surface bmpSub = _vm->getTopBuffer().getSubArea(bmpArea);
		_vm->_resData.transBlitFrom(bmpSub, Common::Point(bmpArea.left, bmpArea.top));
		_vm->getTopBuffer().fillRect(bmpArea, 0);


		// FADE IN:	colorno,ncolors,targetcol,speed:byte
		if (ivals[3] == 0) {
			_vm->getGamePals()->setPalette();
		} else {
			for (int i = 320; i > 0; i -= ivals[3]) {
				int fade = MAX(0, MIN(i / 5, 63));
				_vm->getGamePals()->setFade(ivals[0], ivals[1], ivals[2], fade * 4);
				updateScreen(state);
				g_system->delayMillis(5);
			}
		}
		break;
	}
	case 0x4200: {
		// STORE AREA:	x,y,w,h:int [0..n]		; it makes this area of bmpData persist in the next frames.
		const Common::Rect destRect(ivals[0], ivals[1], ivals[0] + ivals[2], ivals[1] + ivals[3]);
		_vm->_resData.blitFrom(_vm->getBottomBuffer());
		_vm->_resData.transBlitFrom(_vm->getTopBuffer());
		_vm->getBottomBuffer().copyRectToSurface(_vm->_resData, destRect.left, destRect.top, destRect);
		break;
	}
	case 0xa000: // DRAW PIXEL x,y:int
		_vm->getTopBuffer().setPixel(ivals[0], ivals[1], state._drawColFG);
		break;
	case 0xa050: // SAVE REGION    i,j,k,l:int	[i<k,j<l]
		// it works like a bitblit, but it doesn't write if there's something already at the destination?
		_vm->_resData.blitFrom(_vm->getBottomBuffer());
		_vm->_resData.transBlitFrom(_vm->getTopBuffer());
		_vm->getTopBuffer().copyFrom(_vm->_resData);
		break;
	case 0xa0a0: // DRAW LINE  x1,y1,x2,y2:int
		_vm->getTopBuffer().drawLine(ivals[0], ivals[1], ivals[2], ivals[3], state._drawColFG);
		break;
	case 0xa100:
		// DRAW FILLED RECT x,y,w,h:int	[0..320,0..200]
		bmpArea = Common::Rect(ivals[0], ivals[1], ivals[0] + ivals[2], ivals[1] + ivals[3]);
		_vm->getTopBuffer().fillRect(bmpArea, state._drawColFG);
		break;
	case 0xa110: // DRAW EMPTY RECT  x1,y1,x2,y2:int
		bmpArea = Common::Rect(ivals[0], ivals[1], ivals[0] + ivals[2], ivals[1] + ivals[3]);
		_vm->getTopBuffer().drawLine(bmpArea.left, bmpArea.top, bmpArea.right, bmpArea.top, state._drawColFG);
		_vm->getTopBuffer().drawLine(bmpArea.left, bmpArea.bottom, bmpArea.right, bmpArea.bottom, state._drawColFG);
		_vm->getTopBuffer().drawLine(bmpArea.left, bmpArea.top, bmpArea.left, bmpArea.bottom, state._drawColFG);
		_vm->getTopBuffer().drawLine(bmpArea.right, bmpArea.top, bmpArea.right, bmpArea.bottom, state._drawColFG);
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
		debug("DRAW \"%s\"", env._scriptShapes[state._currentBmpId].c_str());

		// DRAW BMP: x,y,tile-id,bmp-id:int [-n,+n] (CHINA)
		// This is kind of file system intensive, will likely have to change to store all the BMPs.
		if (count == 4) {
			int tileId = ivals[2];
			state._currentBmpId = ivals[3];
			if (tileId != -1) {
				_vm->_image->loadBitmap(env._scriptShapes[state._currentBmpId], tileId);
			}
		} else if (!_vm->_image->isLoaded()) {
			// load on demand?
			warning("trying to load bmp %d (%s) on demand", state._currentBmpId, env._scriptShapes[state._currentBmpId].c_str());
			_vm->_image->loadBitmap(env._scriptShapes[state._currentBmpId], 0);
		}

		// DRAW BMP: x,y:int [-n,+n] (RISE)
		if (_vm->_image->isLoaded())
			_vm->_image->drawBitmap(ivals[0], ivals[1], state._drawWin, _vm->getTopBuffer());
		else
			warning("request to draw null img at %d %d", ivals[0], ivals[1]);
		break;
	case 0xf010:
		// LOAD SCR:	filename:str
		_vm->_image->drawScreen(sval, _vm->getBottomBuffer());
		break;
	case 0xf020:
		// LOAD BMP:	filename:str
		env._scriptShapes[state._currentBmpId] = sval;
		break;
	case 0xf040: {
		// LOAD FONT:	filename:str
		error("TODO: Implement opcode 0xf040 load font");
		break;
	}
	case 0xf050: {
		// LOAD PAL:	filename:str
		int newPalNum = _vm->getGamePals()->loadPalette(sval);
		env._scriptPals[state._currentPalId] = newPalNum;
		break;
	}
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
	case 0x00C0: // does something with slot 5?
	case 0x0220: // STOP CURRENT MUSIC
	case 0x0230: // reset current music? (0 args) - found in HoC intro.  Sets params about current music
	case 0x1070: // SELECT FONT  i:int
	case 0x1120: // SET_BACKGROUND
	case 0x1310: // STOP SFX    i:int   eg [107]
	case 0x2010: // SET FRAME
	case 0x2020: // SET TIMER
	case 0x4210: // SAVE IMAGE REGION
	case 0xa300: // DRAW some string? x,y,w,h:int
	case 0xa400: // DRAW FILLED CIRCLE
	case 0xa424: // DRAW EMPTY CIRCLE
	case 0xa510: // DRAW SPRITE1
	case 0xa600: // CLEAR SCREEN
	// From here on are not implemented in DRAGON
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

bool TTMInterpreter::run(TTMEnviro &env, struct TTMSeq &seq) {
	Common::SeekableReadStream *scr = env.scr;
	if (!scr)
		return false;
	if (scr->pos() >= scr->size())
		return false;

	if (seq._timeNext > g_system->getMillis()) {
		return true;
	}
	seq._timeNext = 0;

	uint16 code = 0;
	while (code != 0xff0 && scr->pos() < scr->size()) {
		code = scr->readUint16LE();
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

		handleOperation(env, seq, op, count, ivals, sval);
	}

	updateScreen(seq);
	return true;
}

void TTMInterpreter::findAndAddSequences(TTMEnviro &env, Common::Array<TTMSeq> &seqArray) {
	int16 envno = env._enviro;
	env.scr->seek(0);
	uint16 op = 0;
	for (uint frame = 0; frame < env._totalFrames; frame++) {
		env._frameOffsets[frame] = env.scr->pos();
		//debug("findAndAddSequences: frame %d at offset %d", frame, (int)env.scr->pos());
		op = env.scr->readUint16LE();
		while (op != 0xff0 && env.scr->pos() < env.scr->size()) {
			//debug("findAndAddSequences: check ttm op %04x", op);
			if (op == 0xaf1f || op == 0xaf2f)
				warning("TODO: Fix findAndAddSequences for opcode %x which has variable length arg", op);
			switch (op & 0xf) {
			case 0:
				break;
			case 1:
				if (op == 0x1111) {
					TTMSeq newseq;
					newseq._enviro = envno;
					newseq._seqNum = env.scr->readUint16LE();
					newseq._startFrame = frame;
					newseq._currentFrame = frame;
					newseq._lastFrame = -1;
					//debug("findAndAddSequences: found env %d seq %d at %d", newseq._enviro, newseq._seqNum, (int)env.scr->pos());
					seqArray.push_back(newseq);
				} else {
					env.scr->skip(2);
				}
				break;
			case 0xf: {
				byte ch[2];
				do {
					ch[0] = env.scr->readByte();
					ch[1] = env.scr->readByte();
				} while (ch[0] != 0 && ch[1] != 0);
				break;
			}
			default:
				env.scr->skip((op & 0xf) * 2);
				break;
			}
			op = env.scr->readUint16LE();
		}
	}
	env.scr->seek(0);
}

void TTMSeq::reset() {
	ARRAYCLEAR(_slot);
    _currentFrame = _startFrame;
    _gotoFrame = -1;
    _drawColBG = 0xf;
    _drawColFG = 0xf;
    //_brush_num = 0;
    _currentBmpId = 0;
    _timeCut = 0;
    _timeNext = 0;
    _runCount = 0;
    _runPlayed = 0;
    _executed = false;
    _runFlag = kRunTypeStopped;
    _scriptFlag = 0;
    _selfLoop = false;
    _drawWin = Common::Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
}

ADSInterpreter::ADSInterpreter(DgdsEngine *vm) : _vm(vm), _currentTTMSeq(nullptr) {
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
	dgds.parse(&_adsData, detailfile);

	for (const auto &file : _adsData._scriptNames) {
		_adsData._scriptEnvs.resize(_adsData._scriptEnvs.size() + 1);
		TTMEnviro &data = _adsData._scriptEnvs.back();
		data._enviro = _adsData._scriptEnvs.size();
		_ttmInterpreter->load(file, data);
		_ttmInterpreter->findAndAddSequences(data, _adsData._ttmSeqs);
	}

	_adsData.scr->seek(0);

	uint16 opcode = 0;
	int segcount = 0;
	findUsedSequencesForSegment(0);
	_adsData._segments[0] = 0;
	while (_adsData.scr->pos() < _adsData.scr->size()) {
		opcode = _adsData.scr->readUint16LE();
		if (opcode == 0xffff) {
			segcount++;
			_adsData._segments[segcount] = _adsData.scr->pos();
			findUsedSequencesForSegment(segcount);
		} else {
			_adsData.scr->skip(numArgs(opcode) * 2);
		}
	}

	for (uint i = segcount + 1; i < ARRAYSIZE(_adsData._segments); i++)
		_adsData._segments[i] = 0;

	_adsData._maxSegments = segcount + 1;
	_adsData.filename = filename;

	for (uint i = 0; i < ARRAYSIZE(_adsData._state); i++)
		_adsData._state[i] = 8;
	for (auto &seq : _adsData._ttmSeqs)
		seq.reset();

	return true;
}

static const uint16 ADS_SEQ_OPCODES[] = {
	0x2000, 0x2005, 0x2010, 0x2015, 0x4000, 0x4010, 0x1330,
	0x1340, 0x1360, 0x1370, 0x1320, 0x1310, 0x1350
};

bool ADSInterpreter::updateSeqTimeAndFrame(TTMSeq &seq) {
	if (seq._timeInterval != 0) {
		uint32 now = g_system->getMillis();
		if (now < seq._timeNext)
			return false;
		seq._timeNext = now + seq._timeInterval;
	}

	seq._executed = false;
	if (seq._gotoFrame == -1) {
		seq._currentFrame++;
	} else {
		seq._currentFrame = seq._gotoFrame;
		seq._gotoFrame = -1;
	}

	return true;
}

void ADSInterpreter::findUsedSequencesForSegment(int segno) {
	_adsData._usedSeqs[segno].clear();
	int64 startoff = _adsData.scr->pos();
	uint16 opcode = 0;
	while (opcode != 0xffff && _adsData.scr->pos() < _adsData.scr->size()) {
		opcode = _adsData.scr->readUint16LE();
		for (uint16 o : ADS_SEQ_OPCODES) {
			if (opcode == o) {
				int16 envno = _adsData.scr->readSint16LE();
				int16 seqno = _adsData.scr->readSint16LE();
				TTMSeq *seq = findTTMSeq(envno, seqno);
				if (!seq) {
					warning("ADS opcode %04x at offset %d references unknown seq %d %d",
							opcode, (int)_adsData.scr->pos(), envno, seqno);
				} else {
					bool already_added = false;
					for (TTMSeq *s : _adsData._usedSeqs[segno]) {
						if (s == seq) {
							already_added = true;
							break;
						}
					}
					if (!already_added)
						_adsData._usedSeqs[segno].push_back(seq);
				}
				// Rewind as we will go forward again outside this loop.
				_adsData.scr->seek(-4, SEEK_CUR);
				break;
			}
		}
		_adsData.scr->skip(numArgs(opcode) * 2);
	}
	_adsData.scr->seek(startoff);
}


void ADSInterpreter::unload() {
	_adsData._scriptNames.clear();
	_adsData._scriptEnvs.clear();
	delete _adsData.scr;
	_adsData.scr = nullptr;
	_currentTTMSeq = nullptr;
	_adsData._ttmSeqs.clear();
}

bool ADSInterpreter::playScene() {
	if (!_currentTTMSeq)
		return false;

	TTMEnviro *env = findTTMEnviro(_currentTTMSeq->_enviro);
	if (!env)
		error("Couldn't find environment num %d", _currentTTMSeq->_enviro);

	if (!_ttmInterpreter->run(*env, *_currentTTMSeq)) {
		_currentTTMSeq->_runCount++;
		// TODO: Continue or load next???
		_currentTTMSeq = findTTMSeq(_currentTTMSeq->_enviro, _currentTTMSeq->_seqNum + 1);
	}
	return true;
}

void ADSInterpreter::skipToEndIf(Common::SeekableReadStream *scr) {
	while (scr->pos() < scr->size()) {
		uint16 op = scr->readUint16LE();
		if (op == 0x1520)
			scr->seek(-2, SEEK_CUR);
		if (op == 0 || op == 0x1520)
			return;
		scr->skip(numArgs(op) * 2);
	}
}

TTMEnviro *ADSInterpreter::findTTMEnviro(int16 enviro) {
	for (auto & env : _adsData._scriptEnvs) {
		if (env._enviro == enviro)
			return &env;
	}
	return nullptr;
}

TTMSeq *ADSInterpreter::findTTMSeq(int16 enviro, int16 seq) {
	for (auto & ttm : _adsData._ttmSeqs) {
		if (ttm._enviro == enviro && ttm._seqNum == seq)
			return &ttm;
	}
	return nullptr;
}


void ADSInterpreter::segmentOrState(int16 seg, uint16 val) {
	seg -= 1;
	if (seg >= 0) {
		_adsData._charWhile[seg] = 0;
		if (_adsData._state[seg] != 8)
			_adsData._state[seg] = (_adsData._state[seg] & 8) | val;
	}
}


void ADSInterpreter::segmentSetState(int16 seg, uint16 val) {
	seg -= 1;
	if (seg >= 0) {
		_adsData._charWhile[seg] = 0;
		if (_adsData._state[seg] != 8)
			_adsData._state[seg] = val;
	}
}

void ADSInterpreter::findEndOrInitOp() {
	Common::SeekableReadStream *scr = _adsData.scr;
	int32 startoff = scr->pos();
	while (scr->pos() < scr->size()) {
		uint16 opcode = scr->readUint16LE();
		// on FFFF return the original offset
		if (opcode == 0xffff) {
			scr->seek(startoff);
			return;
		}
		// on 5 (init) return the next offset (don't rewind)
		if (opcode == 0x0005)
			return;
		// everything else just go forward.
		scr->skip(numArgs(opcode) * 2);
	}
}

bool ADSInterpreter::handleOperation(uint16 code, Common::SeekableReadStream *scr) {
	uint16 enviro, seqnum;
	
	//debug("ADSOP: 0x%04x", code);

	switch (code) {
	case 0x0001:
	case 0x0005:
		// "init".  0x0005 can be used for searching for next thing.
		break;
	case 0x1330: { // IF_NOT_PLAYED, 2 params
		enviro = scr->readUint16LE();
		seqnum = scr->readUint16LE();
		TTMSeq *state = findTTMSeq(enviro, seqnum);
		if (state && state->_runPlayed)
			skipToEndIf(scr);
		break;
	}
	case 0x1350: { // IF_PLAYED, 2 params
		enviro = scr->readUint16LE();
		seqnum = scr->readUint16LE();
		TTMSeq *state = findTTMSeq(enviro, seqnum);
		if (state && !state->_runPlayed)
			skipToEndIf(scr);
		break;
	}
	case 0x1360: { // IF_NOT_RUNNING, 2 params
		enviro = scr->readUint16LE();
		seqnum = scr->readUint16LE();
		TTMSeq *seq = findTTMSeq(enviro, seqnum);
		if (seq && seq->_runFlag == kRunTypeStopped)
			skipToEndIf(scr);
		break;
	}
	case 0x1370: { // IF_RUNNING, 2 params
		enviro = scr->readUint16LE();
		seqnum = scr->readUint16LE();
		TTMSeq *seq = findTTMSeq(enviro, seqnum);
		if (seq && (seq->_runFlag == kRunType1 || seq->_runFlag == kRunTypeMulti || seq->_runFlag == kRunTypeTimeLimited))
			skipToEndIf(scr);
		break;
	}
	case 0x1500: // ? IF ?, 0 params
		debug("Unimplemented ADS branch logic opcode 0x1500");
		//sceneLogicOps();
		_adsData._hitBranchOp = true;
		return true;
	case 0x1510: // PLAY_SCENE? 0 params
		return false;
	case 0x1520: // PLAY_SCENE_ENDIF?, 0 params
		_adsData._hitBranchOp = true;
		return false;

	case 0x2000:
	case 0x2005: { // ADD sequence
		enviro = scr->readUint16LE();
		seqnum = scr->readUint16LE();
		int16 runCount = scr->readSint16LE();

		TTMSeq *state = findTTMSeq(enviro, seqnum);
		if (!state)
			error("ADS invalid seq requested %d %d", enviro, seqnum);

		if (code == 0x2000)
			state->_currentFrame = state->_startFrame;

		_currentTTMSeq = state;
		if (runCount == 0) {
			state->_runFlag = kRunType1;
		} else if (runCount < 0) {
			// Negative run count sets the cut time
			state->_timeCut = g_system->getMillis() + runCount;
			// Should this be *10 like delays?
			warning("TODO: checkhandling of negative runcount %d", runCount);
			state->_runFlag = kRunTypeTimeLimited;
		} else {
			state->_runFlag = kRunTypeMulti;
			state->_runCount = runCount - 1;
		}
		state->_runPlayed++;
		uint16 unk = scr->readUint16LE();
		debug("ADSInterpreter add scene - enviro: %d, seqNum: %d, runCount: %d, unk: %d", enviro, seqnum, runCount, unk);
		break;
	}
	case 0x2020: { // RESET SEQ, 2 params (env, seq)
		enviro = scr->readUint16LE();
		seqnum = scr->readUint16LE();
		uint16 unk = scr->readUint16LE();
		debug("ADSInterpreter reset scene - enviro: %d, seqNum: %d, unk: %d", enviro, seqnum, unk);
		TTMSeq *seq = findTTMSeq(enviro, seqnum);
		if (seq)
			seq->reset();
		break;
	}

	case 0xffff:	// END
		return false;

	case 0xF010: {// FADE_OUT, 1 param
		uint16 segment = scr->readUint16LE();
		debug("ADS FADE OUT?? segment param %x", segment);
		break;
	}

	//// unknown / to-be-implemented
	case 0x1010: // unknown, 2 params
	case 0x1020: // unknown, 2 params
	case 0x1030: // unknown, 2 params
	case 0x1040: // unknown, 2 params
	case 0x1050: // unknown, 2 params
	case 0x1060: // unknown, 2 params
	case 0x1070: // unknown, 2 params
	case 0x1080: // unknown if-related, 1 param
	case 0x1340:
	case 0x1420: // AND, 0 params
	case 0x1430: // OR, 0 params
	case 0x2010: // STOP_SCENE, 3 params
	case 0x3010: // RANDOM_START, 0 params
	case 0x3020: // RANDOM_??, 1 param
	case 0x30FF: // RANDOM_END, 0 params
	case 0x4000: // MOVE TO FRONT?, 3 params
	case 0x4010: // MOVE TO BACK??, 3 params
	case 0xF200: // RUN_SCRIPT, 1 param
	case 0xFF10:
	case 0xFFF0: // END_IF, 0 params
	default: {
		int nops = numArgs(code);
		warning("Unimplemented ADS opcode: 0x%04X (skip %d args)", code, nops);
		for (int i = 0; i < nops; i++)
			scr->readUint16LE();
		break;
	}
	}

	return true;
}

int16 ADSInterpreter::getStateForSceneOp(uint16 segnum) {
	if (segnum > _adsData._maxSegments)
		return 0;
	if (!(_adsData._state[segnum] & 4)) {
		for (auto *seq : _adsData._usedSeqs[segnum]) {
			if (!seq)
				return 0;
			if (seq->_runFlag != kRunTypeStopped && !seq->_selfLoop)
				return 1;
		}
	}
	return 0;
}


bool ADSInterpreter::run() {
	if (_adsData._ttmSeqs.empty())
		return false;

	for (int i = 0; i < _adsData._maxSegments; i++) {
		int16 flag = _adsData._state[i] & 0xfff7;
		for (auto seq : _adsData._usedSeqs[i]) {
			if (flag == 3) {
				seq->reset();
			} else {
				seq->_scriptFlag = flag;
			}
		}
	}

	for (int i = 0; i < _adsData._maxSegments; i++) {
		int16 state = _adsData._state[i];
		int32 offset = _adsData._segments[i];
		_adsData.scr->seek(offset);
		if (state & 8) {
			_adsData._state[i] &= 0xfff7;
		} else {
			findEndOrInitOp();
			offset = _adsData.scr->pos();
		}

		if (_adsData._charWhile[i])
			offset = _adsData._charWhile[i];

		if (state == 3 || state == 4) {
			_adsData._state[i] = 1;
			state = 1;
		}

		if (_adsData.scr && state == 1) {
			_adsData.scr->seek(offset);
			runUntilBranchOpOrEnd();
		}
	}

	bool result = false;
	for (auto &seq : _adsData._ttmSeqs) {
		_currentTTMSeq = &seq;
		seq._lastFrame = -1;
		int sflag = seq._scriptFlag;
		TTMRunType rflag = seq._runFlag;
		if (sflag == 6 || (rflag != kRunType1 && rflag != kRunTypeTimeLimited && rflag != kRunTypeMulti && rflag != kRunType5)) {
			if (sflag != 6 && sflag != 5 && rflag == kRunType4) {
			  seq._runFlag = kRunTypeStopped;
			}
		} else {
			int16 curframe = seq._currentFrame;
			TTMEnviro *env = findTTMEnviro(seq._enviro);
			_adsData._hitTTMOp0110 = false;
			bool scriptresult = false;
			if (curframe < env->_totalFrames && curframe > -1 && env->_frameOffsets[curframe] > -1) {
				env->scr->seek(env->_frameOffsets[curframe]);
				_currentTTMSeq = &seq;
				scriptresult = playScene();
			}

			if (scriptresult) {
				seq._executed = true;
				seq._lastFrame = seq._currentFrame;
				result = true;
				if (_adsData._scriptDelay != -1 && seq._timeInterval != _adsData._scriptDelay) {
					uint32 now = g_system->getMillis();
					seq._timeNext = now + _adsData._scriptDelay;
					seq._timeInterval = _adsData._scriptDelay;
				}

				// TODO: What is global 4804?
				if (!_adsData._hitTTMOp0110) {
					warning("TODO: ADSInterpreter::run Finish script result true section");
					// if (global4806 != -1) {
					// 	seq._gotoFrame = global4806;
					// 	if (seq._currentFrame == global4806)
					// 		seq._selfLoop = 1;
					// }
					if (seq._runFlag != kRunType5)
						updateSeqTimeAndFrame(seq);
				} else {
					seq._gotoFrame = seq._startFrame;
					if (seq._runFlag == kRunTypeMulti && seq._runCount != 0) {
						bool updated = updateSeqTimeAndFrame(seq);
						if (updated) {
							seq._runCount--;
						}
					} else if (seq._runFlag == kRunTypeTimeLimited && seq._timeCut != 0) {
						updateSeqTimeAndFrame(seq);
					} else {
						bool updated = updateSeqTimeAndFrame(seq);
						if (updated) {
							seq._runFlag = kRunType4;
							seq._timeInterval = 0;
						}
					}
				}
				// TODO: set script delay here
				// TODO: Finish lines 118 to 149 of disasm.
			} else if (sflag != 5) {
				seq._gotoFrame = seq._startFrame;
				seq._runFlag = kRunType4;
			}
		}

		if (rflag == kRunTypeTimeLimited && seq._timeCut > g_system->getMillis()) {
			seq._runFlag = kRunTypeStopped;
		}
	}
	return result;
}

bool ADSInterpreter::runUntilBranchOpOrEnd() {
	Common::SeekableReadStream *scr = _adsData.scr;
	if (!scr || scr->pos() >= scr->size())
		return false;

	bool more = true;
	do {
		uint16 code = scr->readUint16LE();
		more = handleOperation(code, scr);
	} while (!_adsData._hitBranchOp && more && scr->pos() < scr->size());

	_adsData._hitBranchOp = false;

	return true;
}

void ADSInterpreter::setHitTTMOp0110() {
	_adsData._hitTTMOp0110 = true;
}

int ADSInterpreter::numArgs(uint16 opcode) const {
	// TODO: This list is from DRAGON, there may be more entries in newer games.
	switch (opcode) {
	case 0x1080:
	case 0x3020:
	case 0xF010:
	case 0xF200:
	case 0xF210:
		return 1;

	case 0x1010:
	case 0x1020:
	case 0x1030:
	case 0x1040:
	case 0x1050:
	case 0x1060:
	case 0x1070:
	case 0x1310:
	case 0x1320:
	case 0x1330:
	case 0x1340:
	case 0x1350:
	case 0x1360:
	case 0x1370:
		return 2;

	case 0x2010:
	case 0x2015:
	case 0x2020:
	case 0x4000:
	case 0x4010:
		return 3;

	case 0x2000:
	case 0x2005:
		return 4;

	default:
		return 0;
	}
}


} // End of namespace Dgds
