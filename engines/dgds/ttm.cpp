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

#include "common/rect.h"
#include "common/serializer.h"

#include "graphics/managed_surface.h"
#include "graphics/primitives.h"

#include "dgds/ttm.h"
#include "dgds/ads.h"
#include "dgds/dgds.h"
#include "dgds/game_palettes.h"
#include "dgds/includes.h"
#include "dgds/image.h"
#include "dgds/sound.h"
#include "dgds/font.h"


namespace Dgds {

void GetPutRegion::reset() {
	_area = Common::Rect(SCREEN_WIDTH, SCREEN_HEIGHT);
	_surf.reset();
}

Common::Error TTMEnviro::syncState(Common::Serializer &s) {
	DgdsEngine *engine = dynamic_cast<DgdsEngine *>(g_engine);
	for (auto &shape : _scriptShapes) {
		bool hasShape = shape.get() != nullptr;
		s.syncAsByte(hasShape);
		if (hasShape) {
			Common::String name;
			if (s.isLoading()) {
				s.syncString(name);
				shape.reset(new Image(engine->getResourceManager(), engine->getDecompressor()));
				shape->loadBitmap(name);
			} else {
				name = shape->getFilename();
				s.syncString(name);
			}
		}
	}

	uint16 ngetput = _getPuts.size();
	s.syncAsUint16LE(ngetput);
	_getPuts.resize(ngetput);
	for (uint i = 0; i < ngetput; i++) {
		s.syncAsUint16LE(_getPuts[i]._area.left);
		s.syncAsUint16LE(_getPuts[i]._area.top);
		s.syncAsUint16LE(_getPuts[i]._area.right);
		s.syncAsUint16LE(_getPuts[i]._area.bottom);
		if (s.isLoading()) {
			_getPuts[i]._surf.reset(new Graphics::ManagedSurface());
		} else {
			// TODO: Save the getput buffer contents here?
		}
	}
	for (uint i = 0; i < ARRAYSIZE(_scriptPals); i++)
		s.syncAsSint32LE(_scriptPals[i]);
	for (uint i = 0; i < ARRAYSIZE(_strings); i++)
		s.syncString(_strings[i]);

	// TODO: Save the font list.

	return Common::kNoError;
}

Common::Error TTMSeq::syncState(Common::Serializer &s) {
	s.syncAsSint16LE(_gotoFrame);
	s.syncAsSint16LE(_currentFrame);
	s.syncAsSint16LE(_lastFrame);
	s.syncAsByte(_selfLoop);
	s.syncAsByte(_executed);
	s.syncAsUint32LE(_timeNext);
	s.syncAsUint32LE(_timeCut);

	s.syncAsUint16LE(_drawWin.left);
	s.syncAsUint16LE(_drawWin.top);
	s.syncAsUint16LE(_drawWin.right);
	s.syncAsUint16LE(_drawWin.bottom);

	s.syncAsSint16LE(_currentFontId);
	s.syncAsSint16LE(_currentPalId);
	s.syncAsSint16LE(_currentSongId);
	s.syncAsSint16LE(_currentBmpId);
	s.syncAsSint16LE(_currentGetPutId);
	s.syncAsSint16LE(_brushNum);
	s.syncAsByte(_drawColFG);
	s.syncAsByte(_drawColBG);
	s.syncAsSint16LE(_runPlayed);
	s.syncAsSint16LE(_runCount);
	s.syncAsSint16LE(_timeInterval);
	s.syncAsUint32LE(_runFlag);
	s.syncAsSint16LE(_scriptFlag);

	return Common::kNoError;
}

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
	case 0x0400: return "PALETTE RESET ?";
	case 0x0510: return "UNKNOWN 0x0510";
	case 0x0ff0: return "FINISH FRAME / DRAW";
	case 0x1020: return "SET DELAY";
	case 0x1030: return "SET BRUSH";
	case 0x1050: return "SELECT BMP";
	case 0x1060: return "SELECT PAL";
	case 0x1070: return "SELECT FONT";
	case 0x1090: return "SELECT SONG";
	case 0x10a0: return "SET SCENE";
	case 0x1100: // fall through
	case 0x1110: return "SET SCENE";
	case 0x1120: return "SET GETPUT NUM";
	case 0x1200: return "GOTO";
	case 0x1300: return "PLAY SFX";
	case 0x2000: return "SET DRAW COLORS";
	case 0x2010: return "SET FRAME";
	case 0x2020: return "SET RANDOM DELAY";
	case 0x2300: return "PAL SET BLOCK SWAP 0";
	case 0x2310: return "PAL SET BLOCK SWAP 1";
	case 0x2320: return "PAL SET BLOCK SWAP 2";
	case 0x2400: return "PAL DO BLOCK SWAP";
	case 0x4000: return "SET CLIP WINDOW";
	case 0x4110: return "FADE OUT";
	case 0x4120: return "FADE IN";
	case 0x4200: return "STORE AREA";
	case 0x4210: return "SAVE GETPUT REGION";
	case 0xa000: return "DRAW PIXEL";
	case 0xa010: return "WIPE 10?";
	case 0xa020: return "WIPE 20?";
	case 0xa030: return "WIPE OUT-TO-IN";
	case 0xa040: return "WIPE INTERLACED";
	case 0xa050: return "WIPE LEFT-TO-RIGHT";
	case 0xa060: return "WIPE RIGHT-TO-LEFT";
	case 0xa070: return "WIPE TOP-TO-BOTTOM";
	case 0xa080: return "WIPE BOTTOM-TO-TOP";
	case 0xa090: return "WIPE IN-TO-OUT";
	case 0xa0a0: return "DRAW LINE";
	case 0xa100: return "DRAW FILLED RECT";
	case 0xa110: return "DRAW EMPTY RECT";
	case 0xa200: return "DRAW STRING 0";
	case 0xa210: return "DRAW STRING 1";
	case 0xa220: return "DRAW STRING 2";
	case 0xa230: return "DRAW STRING 3";
	case 0xa240: return "DRAW STRING 4";
	case 0xa250: return "DRAW STRING 5";
	case 0xa260: return "DRAW STRING 6";
	case 0xa270: return "DRAW STRING 7";
	case 0xa280: return "DRAW STRING 8";
	case 0xa290: return "DRAW STRING 9";
	case 0xa500: return "DRAW BMP";
	case 0xa520: return "DRAW SPRITE FLIP";
	case 0xa530: return "DRAW BMP4";
	case 0xa600: return "DRAW GETPUT";
	case 0xb000: return "INIT CREDITS SCROLL";
	case 0xb010: return "DRAW CREDITS SCROLL";
	case 0xf010: return "LOAD SCR";
	case 0xf020: return "LOAD BMP";
	case 0xf040: return "LOAD FONT";
	case 0xf050: return "LOAD PAL";
	case 0xf060: return "LOAD SONG";
	case 0xf100: return "SET STRING 0";
	case 0xf110: return "SET STRING 1";
	case 0xf120: return "SET STRING 2";
	case 0xf130: return "SET STRING 3";
	case 0xf140: return "SET STRING 4";
	case 0xf150: return "SET STRING 5";
	case 0xf160: return "SET STRING 6";
	case 0xf170: return "SET STRING 7";
	case 0xf180: return "SET STRING 8";
	case 0xf190: return "SET STRING 9";
	case 0x0220: return "STOP CURRENT MUSIC";

	case 0x00C0: return "FREE BACKGROUND";
	case 0x0230: return "reset current music?";
	case 0x1310: return "STOP SFX";
	case 0xa300: return "DRAW some string";
	case 0xa400: return "DRAW FILLED CIRCLE";
	case 0xa420: return "DRAW EMPTY CIRCLE";
	case 0xa510: return "DRAW SPRITE1";
	case 0xb600: return "DRAW SCREEN";
	case 0xc020: return "LOAD_SAMPLE";
	case 0xc030: return "SELECT_SAMPLE";
	case 0xc040: return "DESELECT_SAMPLE";
	case 0xc050: return "PLAY_SAMPLE";
	case 0xc060: return "STOP_SAMPLE";

	default: return "UNKNOWN!!";
	}
}

struct ClipSurface {
	Graphics::Surface *_surf;
	Common::Rect _clipWin;
};

static void plotClippedPoint(int x, int y, int color, void *data) {
	ClipSurface *cs = (ClipSurface *)data;
	if (cs->_clipWin.contains(x, y)) {
		byte *ptr = (byte *)cs->_surf->getBasePtr(x, y);
		*ptr = (byte)color;
	}
}

static void _copyRectToScreen(const Graphics::ManagedSurface &src, const Common::Rect &r) {
	if (r.isEmpty())
		return;
	Graphics::Surface *surf = g_system->lockScreen();
	Common::Rect copyRect = r;
	copyRect.clip(Common::Rect(SCREEN_WIDTH, SCREEN_HEIGHT));
	surf->copyRectToSurface(src.rawSurface(), copyRect.left, copyRect.top, copyRect);
	g_system->unlockScreen();
}


void TTMInterpreter::doWipeOp(uint16 code, TTMEnviro &env, struct TTMSeq &seq, const Common::Rect &r) {
	//
	// In the original games, these operations copy certain parts of the buffer on to
	// the screen, and rely on the system's speed to make it happen faster than a regular
	// game frame.
	//
	// This gives a "wipe" effect.  In ScummVM we try to emulate the same final effect with
	// some fudge-factors to update the screen roughly every 10 rows copied.  This gives
	// a reasonably nice result.
	//
	// Example uses:
	// Left-to-right wipe: the "AAAHHHHH" in Dragon intro
	// outside-to-inside or in-to-out: Blood cell video from Karyn in Dragon (scene 94)
	//
	switch(code) {
	case 0xa010:
		// TODO: How is this different to a020?
		_copyRectToScreen(_vm->_compositionBuffer, r);
		g_system->updateScreen();
		break;

	case 0xa020:
		_copyRectToScreen(_vm->_compositionBuffer, r);
		g_system->updateScreen();
		break;

	case 0xa040:	// Interlaced effect
		for (int i = 1; i < 11; i++) {
			int yoffset = 0;
			while (r.top + i + yoffset <= r.bottom) {
				if (r.width()) {
					_copyRectToScreen(_vm->_compositionBuffer, Common::Rect(Common::Point(r.left, r.top + i + yoffset), r.width(), 1));
				}
				yoffset += 10;
			}
			if (i % 3 == 0)
				g_system->updateScreen();
		}
		break;

	case 0xa050:	// Copy from left-to-right
	case 0xa060:	// Copy from right-to-left
		for (int i = 3; i <= r.width() - 3; i++) {
			int x;
			if (code == 0xa060) {
				x = r.right - i;
			} else { // a050
				x = i - 3;
			}
			_copyRectToScreen(_vm->_compositionBuffer, Common::Rect(Common::Point(r.left + x, r.top), 3, r.height()));
			if (i % 10 == 0)
				g_system->updateScreen();
		}
		break;

	case 0xa070:	// Copy from top to bottom
	case 0xa080:	// Copy from bottom to top
		for (int i = 3; i <= r.height() - 3; i++) {
			int y;
			if (code == 0xa080) {
				y = r.bottom - i;
			} else { // a070
				y = i - 3;
			}
			_copyRectToScreen(_vm->_compositionBuffer, Common::Rect(Common::Point(r.left, r.top + y), r.width(), 3));
			if (i % 10 == 0)
				g_system->updateScreen();
		}
		break;

	case 0xa030:	// Copy from outside to middle
	case 0xa090:	// Copy from middle to outside
	{
		int halfwidth = r.width() / 2;
		int halfheight = r.height() / 2;
		uint maxside = MAX(halfheight, halfwidth);

		long widthScale = 1000 * halfwidth / maxside;
		long heightScale = 1000 * halfheight / maxside;
		uint i = (code == 0xa030) ? maxside : 1;

		do {
			if (code == 0xa030) {
				if (i < 1)
					return;
				i--;
			} else {
				if (i > maxside)
					return;
				i++;
			}
			int xoff = widthScale * i / 1000;
			int yoff = heightScale * i / 1000;
			int16 xinside = MAX((r.left + halfwidth) - xoff, 0);
			int16 yinside = MAX((r.top + halfheight) - yoff, 0);
			uint16 width = xoff * 2;
			uint16 height = yoff * 2;
			if (code == 0xa030) {
				_copyRectToScreen(_vm->_compositionBuffer, Common::Rect(Common::Point(r.left, r.top), xinside - r.left, r.bottom - r.top));
				_copyRectToScreen(_vm->_compositionBuffer, Common::Rect(Common::Point(xinside + width, r.top), (r.right - xinside) - xoff * 2, r.bottom - r.top));
				_copyRectToScreen(_vm->_compositionBuffer, Common::Rect(Common::Point(xinside, r.top), width, yinside - r.top));
				_copyRectToScreen(_vm->_compositionBuffer, Common::Rect(Common::Point(xinside, yinside + height), width, r.bottom - yinside - yoff * 2));
			} else {
				_copyRectToScreen(_vm->_compositionBuffer, Common::Rect(Common::Point(xinside, yinside), width, height));
			}
			if (i % 10 == 0)
				g_system->updateScreen();
		} while( true );

		break;
	}
	default:
		error("unsupported opcode for doSaveRegion 0x%04x", code);
	}
}


int16 TTMInterpreter::doOpInitCreditScroll(const Image *img) {
	assert(img);
	int16 maxWidth = 0;
	for (int i = 0; i < img->loadedFrameCount(); i++)
		maxWidth = MAX(maxWidth, img->width(i));
	return maxWidth;
}

 bool TTMInterpreter::doOpCreditsScroll(const Image *img, int16 ygap, int16 ymax, int16 xoff, int16 measuredWidth, const Common::Rect &clipRect) {
	int nframes = img->loadedFrameCount();
	bool scrollFinished = true;
	int y = 200 - ymax;
	for (int i = 0; i < nframes; i++) {
		int width = img->width(i);
		int height = img->height(i);
		if (y > -measuredWidth) {
			Common::Rect drawWin(Common::Point(xoff, y), width, height);
			drawWin.clip(clipRect);
			if (!drawWin.isEmpty()) {
				img->drawBitmap(i, xoff, y, drawWin, _vm->_compositionBuffer);
			}
			scrollFinished = false;
		}
		y += ygap + height;
		if (y > 200)
			break;
    }
    return scrollFinished;
}

void TTMInterpreter::handleOperation(TTMEnviro &env, struct TTMSeq &seq, uint16 op, byte count, const int16 *ivals, const Common::String &sval) {
	switch (op) {
	case 0x0000: // FINISH:	void
		break;
	case 0x0020: // SAVE (free?) BACKGROUND
		if (seq._executed) // this is a one-shot op
			break;
		//
		// This appears in the credits, intro sequence, and the
		// "meanwhile" event with the factory in DRAGON.  It
		// should reload the background image to clear any previous 0020
		// event, and then save the current FG over it.
		// Credits   - (no scr loaded) Store large image on black bg after loading and before txt scroll
		// Intro     - (no scr loaded) After each screen change, draw and save the new comic frame as bg
		//			   on "aaaaah" scene, called after only drawing the AAAH and calling store area
		// Meanwhile - (scr loaded) Save the foreground people onto the background before walk animation
		//
		_vm->getStoredAreaBuffer().fillRect(Common::Rect(SCREEN_WIDTH, SCREEN_HEIGHT), 0);
		_vm->getBackgroundBuffer().blitFrom(_vm->_compositionBuffer);
		break;
	case 0x0070: // FREE PALETTE
		if (seq._executed) // this is a one-shot op
			break;
		warning("TODO: Implement me: op 0x0070 free palette (current pal)");
		seq._currentPalId = 0;
		break;
	case 0x0080: // FREE SHAPE
		env._scriptShapes[seq._currentBmpId].reset();
		break;
	case 0x0090: // FREE FONT
		if (seq._executed) // this is a one-shot op
			break;
		if (seq._currentFontId >= (int16)env._fonts.size()) {
			warning("Request to free font not loaded %d", seq._currentFontId);
			break;
		}
		env._fonts.remove_at(seq._currentFontId);
		seq._currentFontId = 0;
		break;
	case 0x00B0:
		// Does nothing?
		break;
	case 0x00C0: // (one-shot) FREE GETPUT (free getput item pointed to by _currentGetPutId)
		if (seq._executed) // this is a one-shot op
			break;
		env._getPuts[seq._currentGetPutId].reset();
		break;
	case 0x0110: // PURGE void
		_vm->adsInterpreter()->setHitTTMOp0110();
		break;
	case 0x0220: // STOP CURRENT MUSIC
		if (seq._executed) // this is a one-shot op
			break;
		_vm->_soundPlayer->stopMusic();
		break;
	case 0x0400: // RESET PALETTE?
		if (seq._executed) // this is a one-shot op
			break;
		warning("TODO: 0x0400 Reset palette");
		break;
	case 0x0ff0: // REFRESH:	void
		break;
	case 0x1020: // SET DELAY:	    i:int   [0..n]
		// TODO: Probably should do this accounting (as well as timeCut and dialogs)
		// 		 in game frames, not millis.
		_vm->adsInterpreter()->setScriptDelay((int)(ivals[0] * MS_PER_FRAME));
		break;
	case 0x1030: // SET BRUSH:	id:int [-1:n]
		seq._brushNum = ivals[0];
		break;
	case 0x1050: // SELECT BMP:	    id:int [0:n]
		seq._currentBmpId = ivals[0];
		break;
	case 0x1060: // SELECT PAL:  id:int [0]
		seq._currentPalId = ivals[0];
		if (seq._executed) // this is a mostly on-shot op.
			break;
		_vm->getGamePals()->selectPalNum(env._scriptPals[ivals[0]]);
		break;
	case 0x1070: // SELECT FONT  i:int
		seq._currentFontId = ivals[0];
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
		_vm->adsInterpreter()->setGotoTarget(findGOTOTarget(env, seq, ivals[0]));
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
		_vm->adsInterpreter()->setScriptDelay((int)(sleep * MS_PER_FRAME));
		break;
	}
	case 0x2300:
	case 0x2310:
	case 0x2320: {
		uint num = (op & 0xf) > 4;
		uint start = ivals[0];
		uint end = ivals[1];
		warning("TODO: 0x%04x Palette configure block swap %d (%d - %d)", op, num, start, end);
		break;
	}
	case 0x2400:
		if (seq._executed) // this is a one-shot op.
			break;
		warning("TODO: 0x%04x Palette do block swaps 0x%x, 0x%x", op, ivals[0], ivals[1]);
		break;
	case 0x4000: // SET CLIP WINDOW x,y,x2,y2:int	[0..320,0..200]
		// NOTE: params are xmax/ymax, NOT w/h
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

			// TODO: this is a pretty bad way to do it - should pump messages in this loop?
			for (int i = 0; i < 320; i += ivals[3]) {
				int fade = MIN(i / 5, 63);
				_vm->getGamePals()->setFade(ivals[0], ivals[1], ivals[2], fade * 4);
				g_system->updateScreen();
				g_system->delayMillis(5);
			}
		}
		// Clear all the buffers
		_vm->getBackgroundBuffer().fillRect(Common::Rect(SCREEN_WIDTH, SCREEN_HEIGHT), 0);
		_vm->getStoredAreaBuffer().fillRect(Common::Rect(SCREEN_WIDTH, SCREEN_HEIGHT), 0);
		_vm->_compositionBuffer.fillRect(Common::Rect(SCREEN_WIDTH, SCREEN_HEIGHT), 0);
		// reset to previous palette.
		_vm->getGamePals()->setFade(ivals[0], ivals[1], ivals[2], 0);
		break;
	case 0x4120: { // FADE IN:	colorno,ncolors,targetcol,speed:byte
		if (seq._executed) // this is a one-shot op.
			break;

		if (ivals[3] == 0) {
			_vm->getGamePals()->setPalette();
		} else {
			for (int i = 320; i > 0; i -= ivals[3]) {
				int fade = MAX(0, MIN(i / 5, 63));
				_vm->getGamePals()->setFade(ivals[0], ivals[1], ivals[2], fade * 4);
				if (i == 320) {
					// update screen first to make the initial fade-in work
					g_system->copyRectToScreen(_vm->_compositionBuffer.getPixels(), SCREEN_WIDTH, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
				}
				g_system->updateScreen();
				g_system->delayMillis(5);
			}
		}
		break;
	}
	case 0x4200: { // STORE AREA: x,y,w,h:int [0..n]  ; makes this area of foreground persist in the next frames.
		if (seq._executed) // this is a one-shot op
			break;
		const Common::Rect rect(Common::Point(ivals[0], ivals[1]), ivals[2], ivals[3]);
		_vm->getStoredAreaBuffer().blitFrom(_vm->_compositionBuffer, rect, rect);
		break;
	}
	case 0x4210: { // SAVE GETPUT REGION (getput area) x,y,w,h:int
		if (seq._executed) // this is a one-shot op.
			break;
		if (seq._currentGetPutId >= (int)env._getPuts.size()) {
			env._getPuts.resize(seq._currentGetPutId + 1);
		}
		const Common::Rect rect(Common::Point(ivals[0], ivals[1]), ivals[2], ivals[3]);
		env._getPuts[seq._currentGetPutId]._area = rect;

		// Getput reads an area from the front buffer.
		Graphics::ManagedSurface *surf = new Graphics::ManagedSurface(rect.width(), rect.height(), _vm->_compositionBuffer.format);
		surf->blitFrom(_vm->_compositionBuffer, rect, Common::Point(0, 0));
		env._getPuts[seq._currentGetPutId]._surf.reset(surf);
		break;
	}
	case 0xa000: // DRAW PIXEL x,y:int
		_vm->_compositionBuffer.setPixel(ivals[0], ivals[1], seq._drawColFG);
		break;
	case 0xa010:
	case 0xa020:
	case 0xa030:
	case 0xa040:
	case 0xa050:
	case 0xa060:
	case 0xa070:
	case 0xa080:
	case 0xa090:
		doWipeOp(op, env, seq, Common::Rect(Common::Point(ivals[0], ivals[1]), ivals[2], ivals[3]));
		break;
	case 0xa0a0: { // DRAW LINE  x1,y1,x2,y2:int
		ClipSurface clipSurf;
		clipSurf._clipWin = seq._drawWin;
		clipSurf._surf = _vm->_compositionBuffer.surfacePtr();
		Graphics::drawLine(ivals[0], ivals[1], ivals[2], ivals[3], seq._drawColFG, plotClippedPoint, &clipSurf);
		break;
	}
	case 0xa100: { // DRAW FILLED RECT x,y,w,h:int	[0..320,0..200]
		const Common::Rect r(Common::Point(ivals[0], ivals[1]), ivals[2], ivals[3]);
		_vm->_compositionBuffer.fillRect(r, seq._drawColFG);
		break;
	}
	case 0xa110: { // DRAW EMPTY RECT  x1,y1,x2,y2:int
		const Common::Rect r(Common::Point(ivals[0], ivals[1]), ivals[2] - 1, ivals[3] - 1);
		_vm->_compositionBuffer.drawLine(r.left, r.top, r.right, r.top, seq._drawColFG);
		_vm->_compositionBuffer.drawLine(r.left, r.bottom, r.right, r.bottom, seq._drawColFG);
		_vm->_compositionBuffer.drawLine(r.left, r.top, r.left, r.bottom, seq._drawColFG);
		_vm->_compositionBuffer.drawLine(r.right, r.top, r.right, r.bottom, seq._drawColFG);
		break;
	}
	case 0xa200: // 0xa2n0 DRAW STRING n: x,y,w,h:int - draw the nth string from the string table
	case 0xa210:
	case 0xa220:
	case 0xa230:
	case 0xa240:
	case 0xa250:
	case 0xa260:
	case 0xa270:
	case 0xa280:
	case 0xa290: {
		int16 fontno = seq._currentFontId;
		if (fontno >= (int16)env._fonts.size()) {
			warning("Trying to draw font no %d but only loaded %d", seq._currentFontId, env._fonts.size());
			fontno = 0;
		}
		uint strnum = (op & 0x70) >> 4;
		const Common::String &str = env._strings[strnum];
		const FontManager *mgr = _vm->getFontMan();
		const DgdsFont *font = mgr->getFont(env._fonts[fontno]);
		// Note: ignore the y-height argument (ivals[3]) for now. If width is 0, draw as much as we can.
		int width = ivals[2];
		if (width == 0)
			width = SCREEN_WIDTH - ivals[0];
		font->drawString(&(_vm->_compositionBuffer), str, ivals[0], ivals[1], width, seq._drawColFG);
		break;
	}
	case 0xa510:
		// DRAW SPRITE x,y:int  .. how different from 0xa500??
		// FALL THROUGH
	case 0xa520:
		// DRAW SPRITE FLIP: x,y:int
		// FALL THROUGH
	case 0xa530:
		// CHINA
		// DRAW BMP4:	x,y,tile-id,bmp-id:int	[-n,+n] (CHINA)
		// arguments similar to DRAW BMP but it draws the same BMP multiple times with radial symmetry?
		// you can see this in the Dynamix logo star.
		// FALL THROUGH FOR NOW
	case 0xa500: {
		// DRAW BMP: x,y,tile-id,bmp-id:int [-n,+n] (CHINA)
		int frameno;
		int bmpNo;
		if (count == 6) {
			frameno = ivals[2];
			bmpNo = ivals[3];
			warning("TODO: Implement other args %d %d of 6-arg draw function 0x%4x", ivals[4], ivals[5], op);
		} else if (count == 4) {
			frameno = ivals[2];
			bmpNo = ivals[3];
		} else {
			frameno = seq._brushNum;
			bmpNo = seq._currentBmpId;
		}

		// DRAW BMP: x,y:int [-n,+n] (RISE)
		Common::SharedPtr<Image> img = env._scriptShapes[bmpNo];
		if (img)
			img->drawBitmap(frameno, ivals[0], ivals[1], seq._drawWin, _vm->_compositionBuffer, op == 0xa520);
		else
			warning("Trying to draw image %d in env %d which is not loaded", bmpNo, env._enviro);
		break;
	}
	case 0xa600: { // DRAW GETPUT: i:int
		if (seq._executed) // this is a one-shot op.
			break;
		int16 i = ivals[0];
		if (i >= (int16)env._getPuts.size() || !env._getPuts[i]._surf.get()) {
			warning("Trying to put getput region %d we never got", i);
			break;
		}
		const Common::Rect &r = env._getPuts[i]._area;
		// Getput should overwrite the contents
		_vm->_compositionBuffer.blitFrom(*(env._getPuts[i]._surf.get()),
						Common::Point(r.left, r.top));
		break;
	}
	case 0xb000:
		if (seq._executed) // this is a one-shot op
			break;
		env._creditScrollMeasure = doOpInitCreditScroll(env._scriptShapes[seq._currentBmpId].get());
		env._creditScrollYOffset = 0;
		break;
	case 0xb010: {
		const Image *img = env._scriptShapes[seq._currentBmpId].get();
		if (img && img->loadedFrameCount()) {
			bool finished = doOpCreditsScroll(env._scriptShapes[seq._currentBmpId].get(), ivals[0], env._creditScrollYOffset,
							ivals[2], env._creditScrollMeasure, seq._drawWin);
			env._creditScrollYOffset += ivals[1];
			if (finished)
				_vm->adsInterpreter()->setHitTTMOp0110();
		}
		break;
	}
	case 0xc020: {	// LOAD SAMPLE: filename:str
		_vm->_soundPlayer->loadMacMusic(sval.c_str());
		break;
	}
	case 0xc030: {	// SELECT SAMPLE: int: i
		// Do nothing for now?
		break;
	}
	case 0xc050: {	// PLAY SAMPLE: int: i
		_vm->_soundPlayer->playMusic(ivals[0]);
		break;
	}
	case 0xf010: { // LOAD SCR:	filename:str
		if (seq._executed) // this is a one-shot op
			break;
		Image tmp = Image(_vm->getResourceManager(), _vm->getDecompressor());
		tmp.drawScreen(sval, _vm->getBackgroundBuffer());
		_vm->_compositionBuffer.blitFrom(_vm->getBackgroundBuffer());
		_vm->getStoredAreaBuffer().fillRect(Common::Rect(SCREEN_WIDTH, SCREEN_HEIGHT), 0);
		_vm->setBackgroundFile(sval);
		break;
	}
	case 0xf020: // LOAD BMP:	filename:str
		if (seq._executed) // this is a one-shot op
			break;
		env._scriptShapes[seq._currentBmpId].reset(new Image(_vm->getResourceManager(), _vm->getDecompressor()));
		env._scriptShapes[seq._currentBmpId]->loadBitmap(sval);
		break;
	case 0xf040: { // LOAD FONT:	filename:str
		if (seq._executed) // this is a one-shot op
			break;
		const FontManager *mgr = _vm->getFontMan();
		env._fonts.push_back(mgr->fontTypeByName(sval));
		seq._currentFontId = env._fonts.size() - 1;
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
	case 0xf100: // 0xf1n0 - SET STRING n: s:str - set the nth string in the table
	case 0xf110:
	case 0xf120:
	case 0xf130:
	case 0xf140:
	case 0xf150:
	case 0xf160:
	case 0xf170:
	case 0xf180:
	case 0xf190: {
		uint strnum = (op & 0xf0) >> 4;
		env._strings[strnum] = sval;
		break;
	}

	// Unimplemented / unknown
	case 0x0010: // (one-shot) ??
	case 0x0230: // (one-shot) reset current music? (0 args) - found in HoC intro.  Sets params about current music.
	case 0x1040: // Sets some global? i:int
	case 0x10B0: // null op?
	case 0x2010: // SET FRAME?? x,y
	case 0xa300: // DRAW some string? x,y,?,?:int
	case 0xa400: // DRAW FILLED CIRCLE
	case 0xa420: // DRAW EMPTY CIRCLE
	case 0xb600: // DRAW SCREEN?? 6 args		// HoC onward
	case 0xc040: // DESELECT_SAMPLE				// SQ5 demo onward
	case 0xc060: // STOP_SAMPLE					// SQ5 demo onward

	default:
		if (count < 15)
			warning("Unimplemented TTM opcode: 0x%04X (%s, %d args) (ivals: %d %d %d %d)",
					op, ttmOpName(op), count, ivals[0], ivals[1], ivals[2], ivals[3]);
		else
			warning("Unimplemented TTM opcode: 0x%04X (%s, sval: %s)", op,
					ttmOpName(op), sval.c_str());
		break;
	}
}

bool TTMInterpreter::run(TTMEnviro &env, struct TTMSeq &seq) {
	Common::SeekableReadStream *scr = env.scr;
	if (!scr || scr->pos() >= scr->size())
		return false;

	debug(10, "TTM: Run env %d seq %d (%s) frame %d (scr offset %d, %s)", seq._enviro, seq._seqNum,
			env._tags[seq._seqNum].c_str(), seq._currentFrame, (int)scr->pos(),
			seq._executed ? "already executed" : "first execution");
	uint16 code = 0;
	while (code != 0x0ff0 && scr->pos() < scr->size()) {
		code = scr->readUint16LE();
		uint16 op = code & 0xFFF0;
		byte count = code & 0x000F;
		int16 ivals[8];
		Common::String sval;

		if (count > 8 && count != 0x0f)
			error("Invalid TTM opcode %04x requires %d locals", code, count);

		debugN(10, "\tOP: 0x%4.4x %2u ", op, count);
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

			debugN(10, "\"%s\"", sval.c_str());
		} else {
			for (byte i = 0; i < count; i++) {
				ivals[i] = scr->readSint16LE();
				if (i > 0)
					debugN(10, ", ");
				debugN(10, "%d", ivals[i]);
			}
		}
		debug(10, " (%s)", ttmOpName(op));

		handleOperation(env, seq, op, count, ivals, sval);
	}

	return true;
}

int32 TTMInterpreter::findGOTOTarget(TTMEnviro &env, TTMSeq &seq, int16 targetFrame) {
	int64 startpos = env.scr->pos();
	int32 retval = -1;
	for (int32 i = 0; i < (int)env._frameOffsets.size(); i++) {
		if (env._frameOffsets[i] < 0)
			continue;
		env.scr->seek(env._frameOffsets[i]);
		uint16 op = env.scr->readUint16LE();
		if (op == 0x1101 || op == 0x1111) {
			uint16 frameno = env.scr->readUint16LE();
			if (frameno == targetFrame) {
				retval = i;
				break;
			}
		}
	}
	env.scr->seek(startpos);
	return retval;
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
	_drawWin = Common::Rect(SCREEN_WIDTH, SCREEN_HEIGHT);
}


} // end namespace Dgds
