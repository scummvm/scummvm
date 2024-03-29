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
#include "common/random.h"
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

static const char *ttmOpName(uint16 op) {
	switch (op) {
	case 0x0000: return "FINISH";
	case 0x0020: return "SAVE(free?) BACKGROUND";
	case 0x0070: return "FREE PALETTE";
	case 0x0080: return "FREE SHAPE / DRAW BACKGROUND??";
	case 0x0090: return "FREE FONT";
	case 0x00B0: return "NULLOP";
	case 0x0110: return "PURGE";
	case 0x0ff0: return "FINISH FRAME / DRAW";
	case 0x1020: return "SET DELAY";
	case 0x1030: return "SET BRUSH";
	case 0x1050: return "SELECT BMP";
	case 0x1060: return "SELECT PAL";
	case 0x1090: return "SELECT SONG";
	case 0x10a0: return "SET SCENE";
	case 0x1100: // fall through
	case 0x1110: return "SET SCENE";
	case 0x1120: return "SET GETPUT NUM";
	case 0x1200: return "GOTO";
	case 0x1300: return "PLAY SFX";
	case 0x2000: return "SET DRAW COLORS";
	case 0x4000: return "SET CLIP WINDOW";
	case 0x4110: return "FADE OUT";
	case 0x4120: return "FADE IN";
	case 0x4200: return "STORE AREA";
	case 0x4210: return "SAVE GETPUT REGION";
	case 0xa000: return "DRAW PIXEL";
	case 0xa050: return "SAVE REGION";
	case 0xa0a0: return "DRAW LINE";
	case 0xa100: return "DRAW FILLED RECT";
	case 0xa110: return "DRAW EMPTY RECT";
	case 0xa500: return "DRAW BMP";
	case 0xa520: return "DRAW SPRITE FLIP";
	case 0xa530: return "DRAW BMP4";
	case 0xa600: return "DRAW GETPUT";
	case 0xf010: return "LOAD SCR";
	case 0xf020: return "LOAD BMP";
	case 0xf040: return "LOAD FONT";
	case 0xf050: return "LOAD PAL";
	case 0xf060: return "LOAD SONG";
	case 0x0220: return "STOP CURRENT MUSIC";

	case 0x00C0: return "FREE BACKGROUND";
	case 0x0230: return "reset current music?";
	case 0x1070: return "SELECT FONT";
	case 0x1310: return "STOP SFX";
	case 0x2010: return "SET FRAME";
	case 0x2020: return "SET TIMER";
	case 0xa300: return "DRAW some string";
	case 0xa400: return "DRAW FILLED CIRCLE";
	case 0xa420: return "DRAW EMPTY CIRCLE";
	case 0xa510: return "DRAW SPRITE1";
	case 0xb000: return "? (0 args)";
	case 0xb010: return "? (3 args: 30, 2, 19)";
	case 0xb600: return "DRAW SCREEN";
	case 0xc020: return "LOAD_SAMPLE";
	case 0xc030: return "SELECT_SAMPLE";
	case 0xc040: return "DESELECT_SAMPLE";
	case 0xc050: return "PLAY_SAMPLE";
	case 0xc060: return "STOP_SAMPLE";

	default: return "UNKNOWN!!";
	}
}

void TTMInterpreter::handleOperation(TTMEnviro &env, struct TTMSeq &seq, uint16 op, byte count, const int16 *ivals, const Common::String &sval) {
	Common::Rect bmpArea(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	switch (op) {
	case 0x0000: // FINISH:	void
		break;
	case 0x0020: // SAVE (free?) BACKGROUND
		if (seq._executed) // this is a one-shot op
			break;
		_vm->getBottomBuffer().copyFrom(_vm->getTopBuffer());
		break;
	case 0x0070: // FREE PALETTE
		if (seq._executed) // this is a one-shot op
			break;
		error("TODO: Implement me: free palette (current pal)");
		break;
	case 0x0080: // FREE SHAPE // DRAW BACKGROUND??
		_vm->getTopBuffer().copyFrom(_vm->getBottomBuffer());
		break;
	case 0x0090: // FREE FONT
		if (seq._executed) // this is a one-shot op
			break;
		error("TODO: Implement me: free font (current one)");
		break;
	case 0x00B0:
		// Does nothing?
		break;
	case 0x0110: // PURGE void
		_vm->adsInterpreter()->setHitTTMOp0110();
		break;
	case 0x0220: // STOP CURRENT MUSIC
		if (seq._executed) // this is a one-shot op
			break;
		_vm->_soundPlayer->stopMusic();
		break;
	case 0x0ff0: { // REFRESH:	void
		_vm->_resData.blitFrom(_vm->getBottomBuffer());
		Graphics::Surface bmpSub = _vm->getTopBuffer().getSubArea(bmpArea);
		_vm->_resData.transBlitFrom(bmpSub, Common::Point(bmpArea.left, bmpArea.top));
		_vm->getTopBuffer().fillRect(bmpArea, 0);
	} break;
	case 0x1020: // SET DELAY:	    i:int   [0..n]
		// Delay of 240 should be 2 seconds, so this is in quarter-frames.
		// TODO: Probably should do this accounting (as well as timeCut and dialogs)
		// 		 in game frames, not millis.
		_vm->adsInterpreter()->setScriptDelay((int)(ivals[0] * 8.33));
		break;
	case 0x1030: { // SET BRUSH:	id:int [-1:n]
		seq._brushNum = ivals[0];
		_vm->_image->unload();
		//if (seq._brushNum != -1) {
			// TODO: This is probably not the best place to load this - it would be far more
			// efficient to load all frames and pick during the draw.
			//if (!env._scriptShapes[seq._currentBmpId].empty())
			//	_vm->_image->loadBitmap(env._scriptShapes[seq._currentBmpId], seq._brushNum);
		//}
		break;
	}
	case 0x1050: // SELECT BMP:	    id:int [0:n]
		seq._currentBmpId = ivals[0];
		_vm->_image->unload();
		break;
	case 0x1060: // SELECT PAL:  id:int [0]
		seq._currentPalId = ivals[0];
		if (seq._executed) // this is a mostly on-shot op.
			break;
		_vm->getGamePals()->selectPalNum(env._scriptPals[ivals[0]]);
		break;
	case 0x1070: // SELECT FONT  i:int
		seq._currentFontId = ivals[0];
		warning("TODO: Implement TTM 0x1070 select font %d", ivals[0]);
		break;
	case 0x1090: // SELECT SONG:	    id:int [0]
		seq._currentSongId = ivals[0];
		break;
	case 0x10a0: // SET SCENE?:  i:int   [0..n], often 0, called on scene change?
		// In the original this sets a global that seems to be never used?
		break;
	case 0x1100: // SET_SCENE:  i:int   [1..n]
	case 0x1110: // SET_SCENE:  i:int   [1..n]
		// DESCRIPTION IN TTM TAGS. num only used as GOTO target.
		break;
	case 0x1120: // SET GETPUT NUM
		seq._currentGetPutId = ivals[0];
		break;
	case 0x1200: // GOTO
		_vm->adsInterpreter()->setGotoTarget(ivals[0]);
		break;
	case 0x1300: // PLAY SFX    i:int - eg [72], found in Dragon + HoC intro
		if (seq._executed) // this is a one-shot op.
			break;
		_vm->_soundPlayer->playSFX(ivals[0]);
		break;
	case 0x1310: // STOP SFX    i:int   eg [107]
		if (seq._executed) // this is a one-shot op.
			break;
		warning("TODO: Implement TTM 0x1310 stop SFX %d", ivals[0]);
		// Implement this:
		//_vm->_soundPlayer->stopSfxById(ivals[0])
		break;
	case 0x2000: // SET (DRAW) COLORS: fgcol,bgcol:int [0..255]
		seq._drawColFG = static_cast<byte>(ivals[0]);
		seq._drawColBG = static_cast<byte>(ivals[1]);
		break;
	case 0x2020: { // SET RANDOM SLEEP: min,max: int (eg, 60,300)
		if (seq._executed) // this is a one-shot op.
			break;
		uint sleep = _vm->getRandom().getRandomNumberRng(ivals[0], ivals[1]);
		// TODO: do same time fix as for 0x1020
		_vm->adsInterpreter()->setScriptDelay((int)(sleep * 8.33));
		break;
	}
	case 0x4000: // SET CLIP WINDOW x,y,x2,y2:int	[0..320,0..200]
		// NOTE: params are x2/y2, NOT w/h
		seq._drawWin = Common::Rect(ivals[0], ivals[1], ivals[2], ivals[3]);
		break;
	case 0x4110: // FADE OUT:	colorno,ncolors,targetcol,speed:byte
		if (seq._executed) // this is a one-shot op.
			break;
		if (ivals[3] == 0) {
			_vm->getGamePals()->clearPalette();
		} else {
			// The original tight-loops here with 640 steps and i/10 as the fade level..
			// bring that down a bit to use less cpu.
			// Speed 4 should complete fade in 2 seconds (eg, Dynamix logo fade)

			// TODO: this is a pretty bad way to do it - maybe should pump messages at least.
			for (int i = 0; i < 320; i += ivals[3]) {
				int fade = MIN(i / 5, 63);
				_vm->getGamePals()->setFade(ivals[0], ivals[1], ivals[2], fade * 4);
				g_system->copyRectToScreen(_vm->_resData.getPixels(), SCREEN_WIDTH, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
				g_system->updateScreen();
				g_system->delayMillis(5);
			}
		}
		_vm->getBottomBuffer().fillRect(Common::Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT), 0);
		break;
	case 0x4120: { // FADE IN:	colorno,ncolors,targetcol,speed:byte
		if (seq._executed) // this is a one-shot op.
			break;
		// blt first?
		_vm->_resData.blitFrom(_vm->getBottomBuffer());
		Graphics::Surface bmpSub = _vm->getTopBuffer().getSubArea(bmpArea);
		_vm->_resData.transBlitFrom(bmpSub, Common::Point(bmpArea.left, bmpArea.top));
		_vm->getTopBuffer().fillRect(bmpArea, 0);

		if (ivals[3] == 0) {
			_vm->getGamePals()->setPalette();
		} else {
			for (int i = 320; i > 0; i -= ivals[3]) {
				int fade = MAX(0, MIN(i / 5, 63));
				_vm->getGamePals()->setFade(ivals[0], ivals[1], ivals[2], fade * 4);
				g_system->copyRectToScreen(_vm->_resData.getPixels(), SCREEN_WIDTH, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
				g_system->updateScreen();
				g_system->delayMillis(5);
			}
		}
		break;
	}
	case 0x4200: { // STORE AREA: x,y,w,h:int [0..n]		; it makes this area of bmpData persist in the next frames.
		if (seq._executed) // this is a one-shot op
			break;
		const Common::Rect destRect(Common::Point(ivals[0], ivals[1]), ivals[2], ivals[3]);
		_vm->_resData.blitFrom(_vm->getBottomBuffer());
		_vm->_resData.transBlitFrom(_vm->getTopBuffer());
		_vm->getBottomBuffer().copyRectToSurface(_vm->_resData, destRect.left, destRect.top, destRect);
		break;
	}
	case 0x4210: { // SAVE GETPUT REGION (getput area) x,y,w,h:int
		if (seq._executed) // this is a one-shot op.
			break;
		if (seq._currentGetPutId >= (int)env._getPutAreas.size()) {
			env._getPutAreas.resize(seq._currentGetPutId + 1);
			env._getPutSurfaces.resize(seq._currentGetPutId + 1);
		}
		Common::Rect rect = Common::Rect(Common::Point(ivals[0], ivals[1]), ivals[2], ivals[3]);
		env._getPutAreas[seq._currentGetPutId] = rect;
		// TODO: Check which buffer this should get things from
		Graphics::ManagedSurface *surf = new Graphics::ManagedSurface(rect.width(), rect.height(), _vm->getTopBuffer().format);
		surf->blitFrom(_vm->getTopBuffer(), rect, Common::Rect(0, 0, rect.width(), rect.height()));
		env._getPutSurfaces[seq._currentGetPutId].reset(surf);
		break;
	}
	case 0xa000: // DRAW PIXEL x,y:int
		_vm->getTopBuffer().setPixel(ivals[0], ivals[1], seq._drawColFG);
		break;
	case 0xa050: // SAVE REGION    i,j,k,l:int	[i<k,j<l] (not in DRAGON)
		// it works like a bitblit, but it doesn't write if there's something already at the destination?
		_vm->_resData.blitFrom(_vm->getBottomBuffer());
		_vm->_resData.transBlitFrom(_vm->getTopBuffer());
		_vm->getTopBuffer().copyFrom(_vm->_resData);
		break;
	case 0xa0a0: // DRAW LINE  x1,y1,x2,y2:int
		_vm->getTopBuffer().drawLine(ivals[0], ivals[1], ivals[2], ivals[3], seq._drawColFG);
		break;
	case 0xa100: // DRAW FILLED RECT x,y,w,h:int	[0..320,0..200]
		bmpArea = Common::Rect(Common::Point(ivals[0], ivals[1]), ivals[2], ivals[3]);
		_vm->getTopBuffer().fillRect(bmpArea, seq._drawColFG);
		break;
	case 0xa110: // DRAW EMPTY RECT  x1,y1,x2,y2:int
		bmpArea = Common::Rect(Common::Point(ivals[0], ivals[1]), ivals[2], ivals[3]);
		_vm->getTopBuffer().drawLine(bmpArea.left, bmpArea.top, bmpArea.right, bmpArea.top, seq._drawColFG);
		_vm->getTopBuffer().drawLine(bmpArea.left, bmpArea.bottom, bmpArea.right, bmpArea.bottom, seq._drawColFG);
		_vm->getTopBuffer().drawLine(bmpArea.left, bmpArea.top, bmpArea.left, bmpArea.bottom, seq._drawColFG);
		_vm->getTopBuffer().drawLine(bmpArea.right, bmpArea.top, bmpArea.right, bmpArea.bottom, seq._drawColFG);
		break;
	case 0xa510:
		// DRAW SPRITE x,y:int  .. how different from 0xa500??
		// FALL THROUGH
	case 0xa520:
		// DRAW SPRITE FLIP: x,y:int ; happens once in INTRO.TTM
		// FALL THROUGH
	case 0xa530:
		// CHINA
		// DRAW BMP4:	x,y,tile-id,bmp-id:int	[-n,+n] (CHINA)
		// arguments similar to DRAW BMP but it draws the same BMP multiple times with radial simmetry? you can see this in the Dynamix logo star.
		// FALL THROUGH
	case 0xa500:
		debug("DRAW \"%s\" 0x%04x", env._scriptShapes[seq._currentBmpId].c_str(), op);

		// DRAW BMP: x,y,tile-id,bmp-id:int [-n,+n] (CHINA)
		// This is kind of file system intensive, will likely have to change to store all the BMPs.
		if (count == 4) {
			int tileId = ivals[2];
			seq._currentBmpId = ivals[3];
			if (tileId != -1) {
				_vm->_image->loadBitmap(env._scriptShapes[seq._currentBmpId], tileId);
			}
		} else if (!_vm->_image->isLoaded() && !env._scriptShapes[seq._currentBmpId].empty()) {
			// load on demand?
			//warning("trying to draw bmp %d (%s) that was "
			//		" never loaded - do it on demand",
			//		seq._currentBmpId, env._scriptShapes[seq._currentBmpId].c_str());
			_vm->_image->loadBitmap(env._scriptShapes[seq._currentBmpId], seq._brushNum);
		}

		// DRAW BMP: x,y:int [-n,+n] (RISE)
		if (_vm->_image->isLoaded())
			_vm->_image->drawBitmap(ivals[0], ivals[1], seq._drawWin, _vm->getTopBuffer());
		else
			warning("request to draw null img at %d %d", ivals[0], ivals[1]);
		break;
	case 0xa600: { // DRAW GETPUT
		if (seq._executed) // this is a one-shot op.
			break;
		int16 i = ivals[0];
		if (i >= (int16)env._getPutAreas.size() || !env._getPutSurfaces[i]) {
			warning("Trying to put getput region %d we never got", i);
			break;
		}
		const Common::Rect &r = env._getPutAreas[i];
		_vm->getTopBuffer().copyRectToSurface(*(env._getPutSurfaces[i]->surfacePtr()),
						r.left, r.top, Common::Rect(0, 0, r.width(), r.height()));
		break;
	}
	case 0xf010: // LOAD SCR:	filename:str
		if (seq._executed) // this is a one-shot op
			break;
		_vm->_image->drawScreen(sval, _vm->getBottomBuffer());
		break;
	case 0xf020: // LOAD BMP:	filename:str
		if (seq._executed) // this is a one-shot op
			break;
		env._scriptShapes[seq._currentBmpId] = sval;
		_vm->_image->unload();
		break;
	case 0xf040: { // LOAD FONT:	filename:str
		if (seq._executed) // this is a one-shot op
			break;
		//const FontManager *mgr = _vm->getFontMan();
		warning("TODO: Implement opcode 0xf040 load font %s", sval.c_str());
		break;
	}
	case 0xf050: { // LOAD PAL:	filename:str
		if (seq._executed) // this is a one-shot op
			break;
		int newPalNum = _vm->getGamePals()->loadPalette(sval);
		env._scriptPals[seq._currentPalId] = newPalNum;
		break;
	}
	case 0xf060: // LOAD SONG:	filename:str
		if (seq._executed) // this is a one-shot op
			break;
		if (_vm->_platform == Common::kPlatformAmiga) {
			// TODO: remove hard-coded stuff..
			_vm->_soundPlayer->playAmigaSfx("DYNAMIX.INS", 0, 255);
		} else if (_vm->_platform == Common::kPlatformMacintosh) {
			_vm->_soundPlayer->loadMacMusic(sval.c_str());
			_vm->_soundPlayer->playMusic(seq._currentSongId);
		} else {
			_vm->_soundPlayer->loadMusic(sval.c_str());
			_vm->_soundPlayer->playMusic(seq._currentSongId);
		}
		break;


	// Unimplemented / unknown
	case 0x0010: // (one-shot) ??
	case 0x00C0: // (one-shot) FREE GETPUT (free getput item pointed to by _currentGetPutId)
	case 0x0230: // (one-shot) reset current music? (0 args) - found in HoC intro.  Sets params about current music.
	case 0x0400: // (one-shot) set palette??
	case 0x1040: // Sets some global? i:int
	case 0x10B0: // null op?
	case 0x2010: // SET FRAME?? x,y
	case 0xa300: // DRAW some string? x,y,?,?:int
	case 0xa400: // DRAW FILLED CIRCLE
	case 0xa424: // DRAW EMPTY CIRCLE

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
		if (count < 15)
			warning("Unimplemented TTM opcode: 0x%04X (%d args) (ivals: %d %d %d %d)", op, count, ivals[0], ivals[1], ivals[2], ivals[3]);
		else
			warning("Unimplemented TTM opcode: 0x%04X (sval: %s)", op, sval.c_str());
		break;
	}
}

bool TTMInterpreter::run(TTMEnviro &env, struct TTMSeq &seq) {
	Common::SeekableReadStream *scr = env.scr;
	if (!scr)
		return false;
	if (scr->pos() >= scr->size())
		return false;

	debug("TTM: Run env %d seq %d frame %d (scr offset %d)", seq._enviro, seq._seqNum,
			seq._currentFrame, (int)scr->pos());
	uint16 code = 0;
	while (code != 0x0ff0 && scr->pos() < scr->size()) {
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
		debug(" (%s)", ttmOpName(op));

		handleOperation(env, seq, op, count, ivals, sval);
	}

	return true;
}

int32 TTMInterpreter::findGOTOTarget(TTMEnviro &env, TTMSeq &seq) {
	error("TODO: implement TTMInterpreter::findGOTOTarget");
}

void TTMInterpreter::findAndAddSequences(TTMEnviro &env, Common::Array<TTMSeq> &seqArray) {
	int16 envno = env._enviro;
	env.scr->seek(0);
	uint16 op = 0;
	for (uint frame = 0; frame < env._totalFrames; frame++) {
		env._frameOffsets[frame] = env.scr->pos();
		//debug("findAndAddSequences: frame %d at offset %d", frame, (int)env.scr->pos());
		op = env.scr->readUint16LE();
		while (op != 0x0ff0 && env.scr->pos() < env.scr->size()) {
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
	_currentFontId = 0;
	_currentPalId = 0;
	_currentPalId = 0;
	_currentBmpId = 0;
	_currentGetPutId = 0;
    _currentFrame = _startFrame;
    _gotoFrame = -1;
    _drawColBG = 0xf;
    _drawColFG = 0xf;
    _brushNum = 0;
    _timeInterval = 0;
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

	debug("ADSInterpreter: load %s", detailfile.c_str());

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
	opcode = _adsData.scr->readUint16LE();
	while (_adsData.scr->pos() < _adsData.scr->size()) {
		if (opcode == 0xffff) {
			segcount++;
			_adsData._segments[segcount] = _adsData.scr->pos();
			findUsedSequencesForSegment(segcount);
		} else {
			_adsData.scr->skip(numArgs(opcode) * 2);
		}
		opcode = _adsData.scr->readUint16LE();
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
		uint32 now = g_engine->getTotalPlayTime();
		if (now < seq._timeNext) {
			debug("env %d seq %d not advancing from frame %d (now %d timeNext %d interval %d)", seq._enviro,
					seq._seqNum, seq._currentFrame, now, seq._timeNext, seq._timeInterval);
			return false;
		}
		seq._timeNext = now + seq._timeInterval;
	}

	seq._executed = false;
	if (seq._gotoFrame == -1) {
		debug("env %d seq %d advance to frame %d->%d (start %d last %d)", seq._enviro,
				seq._seqNum, seq._currentFrame, seq._currentFrame + 1, seq._startFrame, seq._lastFrame);
		seq._currentFrame++;
	} else {
		debug("env %d seq %d goto to frame %d->%d (start %d last %d)", seq._enviro,
				seq._seqNum, seq._currentFrame, seq._gotoFrame, seq._startFrame, seq._lastFrame);
		seq._currentFrame = seq._gotoFrame;
		seq._gotoFrame = -1;
	}

	return true;
}

void ADSInterpreter::findUsedSequencesForSegment(int segno) {
	_adsData._usedSeqs[segno].clear();
	int64 startoff = _adsData.scr->pos();
	uint16 opcode = 0;
	// Skip the segment number.
	_adsData.scr->readUint16LE();
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
	delete _adsData.scr;
	_adsData = ADSData();
	_currentTTMSeq = nullptr;
}

bool ADSInterpreter::playScene() {
	if (!_currentTTMSeq)
		return false;

	TTMEnviro *env = findTTMEnviro(_currentTTMSeq->_enviro);
	if (!env)
		error("Couldn't find environment num %d", _currentTTMSeq->_enviro);

	_adsData._gotoTarget = -1;
	return _ttmInterpreter->run(*env, *_currentTTMSeq);
}

bool ADSInterpreter::skipSceneLogicBranch() {
	Common::SeekableReadStream *scr = _adsData.scr;
	bool result = true;
	while (scr->pos() < scr->size()) {
		uint16 op = scr->readUint16LE();
		if (op == 0x1510 || op == 0x1500) {
			scr->seek(-2, SEEK_CUR);
			return true;
		} else if (op == 0 || op == 0xffff) {
			return false;
		} else if ((op & 0xff0f) == 0x1300) {
			// a 0x13x0 logic op
			scr->seek(-2, SEEK_CUR);
			result = handleOperation(op, scr);
		} else {
			scr->skip(numArgs(op) * 2);
		}
	}
	return result && scr->pos() < scr->size();
}

bool ADSInterpreter::skipToEndIf() {
	Common::SeekableReadStream *scr = _adsData.scr;
	bool result = skipSceneLogicBranch();
	if (result) {
		uint16 op = scr->readUint16LE();
		if (op == 0x1500) {
			result = runUntilBranchOpOrEnd();
		}
		// don't rewind - the calls to this should always return ptr+2
		//scr->seek(-2, SEEK_CUR);
	}
	return result;
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
	int idx = getArrIndexOfSegNum(seg);
	if (idx >= 0) {
		_adsData._charWhile[idx] = 0;
		_adsData._state[idx] = (_adsData._state[idx] & 8) | val;
	}
}


void ADSInterpreter::segmentSetState(int16 seg, uint16 val) {
	int idx = getArrIndexOfSegNum(seg);
	if (idx >= 0) {
		_adsData._charWhile[idx] = 0;
		if (_adsData._state[idx] != 8)
			_adsData._state[idx] = val;
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

bool ADSInterpreter::logicOpResult(uint16 code, const TTMSeq *seq) {
	switch (code) {
	case 0x1310: // IF runtype 5, 2 params
		debug("ADS: if runtype 5 env %d seq %d", seq->_enviro, seq->_seqNum);
		return seq->_runFlag == kRunType5;
	case 0x1320: // IF not runtype 5, 2 params
		debug("ADS: if not runtype 5 env %d seq %d", seq->_enviro, seq->_seqNum);
		return seq->_runFlag != kRunType5;
	case 0x1330: // IF_NOT_PLAYED, 2 params
		debug("ADS: if not played env %d seq %d", seq->_enviro, seq->_seqNum);
		return !seq->_runPlayed;
	case 0x1340: // IF_PLAYED, 2 params
		debug("ADS: if played env %d seq %d", seq->_enviro, seq->_seqNum);
		return seq->_runPlayed;
	case 0x1350: // IF_FINISHED, 2 params
		debug("ADS: if finished env %d seq %d", seq->_enviro, seq->_seqNum);
		return seq->_runFlag == kRunTypeFinished;
	case 0x1360: // IF_NOT_RUNNING, 2 params
		debug("ADS: if not running env %d seq %d", seq->_enviro, seq->_seqNum);
		return seq->_runFlag == kRunTypeStopped;
	case 0x1370: // IF_RUNNING, 2 params
		debug("ADS: if running env %d seq %d", seq->_enviro, seq->_seqNum);
		return seq->_runFlag == kRunType1 || seq->_runFlag == kRunTypeMulti || seq->_runFlag == kRunTypeTimeLimited;
	default:
		error("Not an ADS logic op: %04x, how did we get here?", code);
	}
}

bool ADSInterpreter::handleLogicOp(uint16 code,  Common::SeekableReadStream *scr) {
	bool testval = true;
	uint16 andor = 0x1420; // start with "true" AND..
	while (scr->pos() < scr->size()) {
		uint16 enviro = scr->readUint16LE();
		uint16 seqnum = scr->readUint16LE();
		TTMSeq *seq = findTTMSeq(enviro, seqnum);
		if (!seq) {
			warning("ADS if op referenced non-existant env %d seq %d", enviro, seqnum);
			return false;
		}

		if (andor == 0x1420) // AND
			testval &= logicOpResult(code, seq);
		else // OR
			testval |= logicOpResult(code, seq);

		code = scr->readUint16LE();

		if (code == 0x1420 || code == 0x1430) {
			andor = code;
			code = scr->readUint16LE();
			// The next op should be another logic op
		} else {
			// No AND or OR, next op is just what to do.
			scr->seek(-2, SEEK_CUR);
			if (testval)
				return runUntilBranchOpOrEnd();
			else
				return skipToEndIf();
		}
	}
	error("didn't return from ADS logic test");
}

int16 ADSInterpreter::randomOpGetProportion(uint16 code, Common::SeekableReadStream *scr) {
	int argsize = numArgs(code) * 2;
	if (argsize == 0)
		error("Unexpected 0-arg ADS opcode 0x%04x inside random block", code);
	if (argsize > 2)
		scr->seek(argsize - 2, SEEK_CUR);
	int16 result = scr->readSint16LE();
	scr->seek(-argsize, SEEK_CUR);
	return result;
}

void ADSInterpreter::handleRandomOp(uint16 code, Common::SeekableReadStream *scr) {
	int16 max = 0;
	int64 startpos = scr->pos();
	// Collect the random proportions
	code = scr->readUint16LE();
	while (code != 0 && code != 0x30ff && scr->pos() < scr->size()) {
		int16 val = randomOpGetProportion(code, scr);
		// leaves pointer at beginning of next op
		max += val;
		scr->skip(numArgs(code) * 2);
		if (scr->pos() >= scr->size())
			break;
		code = scr->readUint16LE();
	}
	if (!max)
		return;

	int64 endpos = scr->pos();

	int16 randval = _vm->getRandom().getRandomNumber(max - 1);
	scr->seek(startpos, SEEK_SET);

	// Now find the random bit to jump to
	code = scr->readUint16LE();
	do {
		int16 val = randomOpGetProportion(code, scr);
		randval -= val;
		if (randval < 1) {
			// This is the opcode we want to execute
			break;
		}
		scr->skip(numArgs(code) * 2);
		if (scr->pos() >= scr->size())
			break;
		code = scr->readUint16LE();
	} while (code != 0 && scr->pos() < scr->size());
	if (code)
		handleOperation(code, scr);

	scr->seek(endpos, SEEK_SET);
}

bool ADSInterpreter::handleOperation(uint16 code, Common::SeekableReadStream *scr) {
	uint16 enviro, seqnum;

	debug("  ADSOP: 0x%04x", code);

	switch (code) {
	case 0x0001:
	case 0x0005:
		//debug("ADS: init code 0x%04x", code);
		// "init".  0x0005 can be used for searching for next thing.
		break;
	case 0x1310: // IF runtype 5, 2 params
	case 0x1320: // IF not runtype 5, 2 params
	case 0x1330: // IF_NOT_PLAYED, 2 params
	case 0x1340: // IF_PLAYED, 2 params
	case 0x1350: // IF_FINISHED, 2 params
	case 0x1360: // IF_NOT_RUNNING, 2 params
	case 0x1370: // IF_RUNNING, 2 params
		return handleLogicOp(code, scr);
	case 0x1500: // ? IF ?, 0 params
		debug("ADS: Unimplemented ADS branch logic opcode 0x1500");
		//sceneLogicOps();
		_adsData._hitBranchOp = true;
		return true;
	case 0x1510: // PLAY_SCENE? 0 params
		debug("ADS: 0x%04x hit branch op true", code);
		_adsData._hitBranchOp = true;
		return true;
	case 0x1520: // PLAY_SCENE_ENDIF?, 0 params
		debug("ADS: 0x%04x hit branch op", code);
		_adsData._hitBranchOp = true;
		return false;

	case 0x2000:
	case 0x2005: { // ADD sequence
		enviro = scr->readUint16LE();
		seqnum = scr->readUint16LE();
		int16 runCount = scr->readSint16LE();
		uint16 unk = scr->readUint16LE();
		debug("ADS: add scene - env %d seq %d runCount %d unk %d", enviro, seqnum, runCount, unk);

		TTMSeq *seq = findTTMSeq(enviro, seqnum);
		if (!seq)
			error("ADS invalid seq requested %d %d", enviro, seqnum);

		if (code == 0x2000)
			seq->_currentFrame = seq->_startFrame;

		_currentTTMSeq = seq;
		if (runCount == 0) {
			seq->_runFlag = kRunType1;
		} else if (runCount < 0) {
			// Negative run count sets the cut time
			seq->_timeCut = g_engine->getTotalPlayTime() + runCount;
			// Should this be *10 like delays?
			warning("TODO: check handling of negative runcount %d", runCount);
			seq->_runFlag = kRunTypeTimeLimited;
		} else {
			seq->_runFlag = kRunTypeMulti;
			seq->_runCount = runCount - 1;
		}
		seq->_runPlayed++;
		break;
	}
	case 0x2010: { // STOP_SCENE, 3 params (ttmenv, ttmseq, ?)
		enviro = scr->readUint16LE();
		seqnum = scr->readUint16LE();
		uint16 unk = scr->readUint16LE();
		debug("ADS: stop seq env %d seq %d unk %d", enviro, seqnum, unk);
		_currentTTMSeq = findTTMSeq(enviro, seqnum);
		if (_currentTTMSeq)
			_currentTTMSeq->_runFlag = kRunTypeStopped;
		break;
	}
	case 0x2020: { // RESET SEQ, 2 params (env, seq)
		enviro = scr->readUint16LE();
		seqnum = scr->readUint16LE();
		uint16 unk = scr->readUint16LE();
		debug("ADS: reset scene env %d seq %d unk %d", enviro, seqnum, unk);
		_currentTTMSeq = findTTMSeq(enviro, seqnum);
		if (_currentTTMSeq)
			_currentTTMSeq->reset();
		break;
	}

	case 0x4000: { // MOVE SEQ TO FRONT
		enviro = scr->readUint16LE();
		seqnum = scr->readUint16LE();
		/*uint16 unk = */scr->readUint16LE();
		// This is O(N) but the N is small and it's not called often.
		TTMSeq seq;
		bool success = false;
		for (uint i = 0; i < _adsData._ttmSeqs.size(); i++) {
			if (_adsData._ttmSeqs[i]._enviro == enviro && _adsData._ttmSeqs[i]._seqNum == seqnum) {
				seq = _adsData._ttmSeqs[i];
				_adsData._ttmSeqs.remove_at(i);
				success = true;
				break;
			}
		}

		if (success)
			_adsData._ttmSeqs.insert_at(0, seq);
		else
			warning("ADS: 0x4000 Request to move env %d seq %d which doesn't exist", enviro, seqnum);

		break;
	}

	case 0x4010: { // MOVE SEQ TO BACK
		enviro = scr->readUint16LE();
		seqnum = scr->readUint16LE();
		/*uint16 unk = */scr->readUint16LE();
		// This is O(N) but the N is small and it's not called often.
		TTMSeq seq;
		bool success = false;
		for (uint i = 0; i < _adsData._ttmSeqs.size(); i++) {
			if (_adsData._ttmSeqs[i]._enviro == enviro && _adsData._ttmSeqs[i]._seqNum == seqnum) {
				seq = _adsData._ttmSeqs[i];
				_adsData._ttmSeqs.remove_at(i);
				success = true;
				break;
			}
		}

		if (success)
			_adsData._ttmSeqs.push_back(seq);
		else
			warning("ADS: 0x4010 Request to move env %d seq %d which doesn't exist", enviro, seqnum);

		break;
	}

	case 0xF010: {// FADE_OUT, 1 param
		int16 segment = scr->readSint16LE();
		int16 idx = _adsData._runningSegmentIdx;
		if (segment >= 0)
			idx = getArrIndexOfSegNum(segment);
		debug("ADS: set state 2, segment param %d (idx %d)", segment, idx);
		if (idx >= 0)
			_adsData._state[idx] = 2;
		if (idx == _adsData._runningSegmentIdx)
			return false;
		else
			return true;
	}

	case 0xffff:	// END
		return false;

	case 0x3020: // RANDOM_NOOP, 1 param (proportion)
		scr->readUint16LE();
		return true;

	case 0x3010: // RANDOM_START, 0 params
	case 0x30FF: // RANDOM_END, 0 params
		handleRandomOp(code, scr);
		break;

	//// unknown / to-be-implemented
	case 0x1010: // unknown, 2 params
	case 0x1020: // unknown, 2 params
	case 0x1030: // unknown, 2 params
	case 0x1040: // unknown, 2 params
	case 0x1050: // unknown, 2 params
	case 0x1060: // unknown, 2 params
	case 0x1070: // unknown, 2 params
	case 0x1080: // if current seq countdown, 1 param
	case 0x1420: // AND, 0 params
	case 0x1430: // OR, 0 params
	case 0xF200: // RUN_SCRIPT, 1 param
	case 0xFF10:
	case 0xFFF0: // END_IF, 0 params
	default: {
		int nops = numArgs(code);
		warning("ADS: Unimplemented opcode: 0x%04X (skip %d args)", code, nops);
		for (int i = 0; i < nops; i++)
			scr->readUint16LE();
		break;
	}
	}

	return true;
}

int16 ADSInterpreter::getStateForSceneOp(uint16 segnum) {
	int idx = getArrIndexOfSegNum(segnum);
	if (idx < 0)
		return 0;
	if (!(_adsData._state[idx] & 4)) {
		for (auto *seq : _adsData._usedSeqs[idx]) {
			if (!seq)
				return 0;
			if (seq->_runFlag != kRunTypeStopped && !seq->_selfLoop)
				return 1;
		}
		return 0;
	}
	return 1;
}


int ADSInterpreter::getArrIndexOfSegNum(uint16 segnum) {
	int32 startoff = _adsData.scr->pos();
	int result = -1;
	for (int i = 0; i < _adsData._maxSegments; i++) {
		_adsData.scr->seek(_adsData._segments[i]);
		int16 seg = _adsData.scr->readSint16LE();
		if (seg == segnum) {
			result = i;
			break;
		}
	}
	_adsData.scr->seek(startoff);
	return result;
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
		offset += 2;
		/*int16 segnum =*/ _adsData.scr->readSint16LE();
		if (state & 8) {
			state &= 0xfff7;
			_adsData._state[i] = state;
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

		_adsData._runningSegmentIdx = i;
		if (_adsData.scr && state == 1) {
			_adsData.scr->seek(offset);
			//debug("ADS: Run segment %d idx %d/%d", segnum, i, _adsData._maxSegments);
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
			if (sflag != 6 && sflag != 5 && rflag == kRunTypeFinished) {
				seq._runFlag = kRunTypeStopped;
			}
		} else {
			int16 curframe = seq._currentFrame;
			TTMEnviro *env = findTTMEnviro(seq._enviro);
			_adsData._hitTTMOp0110 = false;
			_adsData._scriptDelay = -1;
			bool scriptresult = false;
			// Next few lines of code in a separate function in the original..
			if (curframe < env->_totalFrames && curframe > -1 && env->_frameOffsets[curframe] > -1) {
				env->scr->seek(env->_frameOffsets[curframe]);
				scriptresult = playScene();
			}

			if (scriptresult && sflag != 5) {
				seq._executed = true;
				seq._lastFrame = seq._currentFrame;
				result = true;
				if (_adsData._scriptDelay != -1 && seq._timeInterval != _adsData._scriptDelay) {
					uint32 now = g_engine->getTotalPlayTime();
					seq._timeNext = now + _adsData._scriptDelay;
					seq._timeInterval = _adsData._scriptDelay;
				}

				if (!_adsData._hitTTMOp0110) {
					if (_adsData._gotoTarget != -1) {
					 	seq._gotoFrame = _adsData._gotoTarget;
					 	if (seq._currentFrame == _adsData._gotoTarget)
					 		seq._selfLoop = true;
					}
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
							seq._runFlag = kRunTypeFinished;
							seq._timeInterval = 0;
						}
					}
				}
			} else if (sflag != 5) {
				seq._gotoFrame = seq._startFrame;
				seq._runFlag = kRunTypeFinished;
			}
		}

		if (rflag == kRunTypeTimeLimited && seq._timeCut > g_engine->getTotalPlayTime()) {
			seq._runFlag = kRunTypeFinished;
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
		if (code == 0xffff)
			return false;
		more = handleOperation(code, scr);
		// FIXME: Breaking on hitBranchOp here doesn't work - probably need to fix the IF handling properly.
	} while (!_adsData._hitBranchOp && more && scr->pos() < scr->size());

	_adsData._hitBranchOp = false;

	return more;
}

void ADSInterpreter::setHitTTMOp0110() {
	_adsData._hitTTMOp0110 = true;
}

void ADSInterpreter::setGotoTarget(int32 target) {
	_adsData._gotoTarget = target;
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
