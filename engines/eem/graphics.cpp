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

#include "common/debug.h"
#include "common/events.h"
#include "common/file.h"
#include "common/path.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/textconsole.h"

#include "graphics/cursorman.h"
#include "graphics/managed_surface.h"

#include "eem/detection.h"
#include "eem/eem.h"

// EEM — graphics helpers (KDGRAPH.C + KDHELP.C, the latter merged because
// it has only three functions). Animation playback, balloon-table lookup,
// and the help-screen primitives that share the same balloon machinery.

namespace EEM {

// `_InterfaceHelp(num)` @ 1560:0205 reads `HelpData @ 29be:00c8` (5-byte
// entries: u8 count, then up to 2 u16 picIds). Verified bytes:
//   entry 0 (PDA / gallery HELP button): count=2, picIds = 0x0063, 0x01ae
//   entry 1: count=2, picIds = 0x0192, 0x01b1
// Only entry 0 is reachable from the PDA notebook (rect 1) and the
// gallery (rect 1) — both call `_InterfaceHelp(0)`.
const uint16 kHelpPics[][2] = {
	{ 0x0063, 0x01ae },
	{ 0x0192, 0x01b1 },
};

// 52-entry, 10-bytes-each balloon-metadata table at `29be:0875`.
// Used at 1df2:0aef-0af9 (accuse hint) and `_DisplayClue` to position
// `_WordWrap` text inside the balloon. Only +0/+2/+4 are read by
// `getBalloonInsets`:
//   +0..1 = text X inset, +2..3 = Y inset, +4..5 = max wrap width
// (+6/+8 = balloon h / tail offset, both unused for text layout).
struct BalloonInsets { uint16 x; uint16 y; uint16 w; };
const BalloonInsets kBalloonInsetTable[] = {
	{ 6, 4, 142 }, { 6, 4, 142 }, { 6, 4, 142 }, { 6, 4, 142 },
	{ 6, 4, 142 }, { 6, 4, 142 }, { 6, 4, 142 },
	{ 6, 4, 224 }, { 6, 4, 224 }, { 6, 4, 224 }, { 6, 4, 224 },
	{ 6, 4, 224 }, { 6, 4, 224 }, { 6, 4, 224 },
	{ 6, 4, 291 }, { 6, 4, 291 }, { 6, 4, 291 }, { 6, 4, 291 },
	{ 6, 4, 291 }, { 6, 4, 291 }, { 6, 4, 291 },
	{ 5, 4, 155 }, { 5, 4, 155 }, { 5, 4, 155 }, { 5, 4, 155 },
	{ 5, 4, 155 }, { 5, 4, 155 }, { 5, 4, 155 },
	{ 5, 4, 237 }, { 5, 4, 237 }, { 5, 4, 237 }, { 5, 4, 237 },
	{ 5, 4, 237 }, { 5, 4, 237 }, { 5, 4, 237 },
	{ 3, 4, 155 }, { 3, 4, 155 }, { 3, 4, 155 }, { 3, 4, 155 },
	{ 3, 4, 155 }, { 3, 4, 155 }, { 3, 4, 155 },
	{ 5, 4, 238 }, { 5, 4, 238 }, { 5, 4, 238 }, { 5, 4, 238 },
	{ 5, 4, 238 }, { 5, 4, 238 }, { 5, 4, 238 },
	{ 5, 8, 158 }, { 5, 8, 176 }, { 8, 7, 142 }
};

void EEMEngine::doHelp() {
	// `_KDHelp` reads two hint TextBlock offsets from `_KDTextIndex`:
	//   word @ +0xe : first-time hint
	//   word @ +0x10: second-time hint (cycles back to first if missing)
	// `_SawHelpHint` toggles between them.
	if (!_mystery.isLoaded() || !_font.isLoaded())
		return;

	const byte *kd = _mystery.kdTextIndex();
	if (!kd)
		return;

	const uint16 hintFirst  = READ_LE_UINT16(kd + 0x0e);
	const uint16 hintSecond = READ_LE_UINT16(kd + 0x10);
	uint16 use = _mystery._sawHelpHint && hintSecond != 0xFFFF ? hintSecond : hintFirst;
	if (use == 0xFFFF) {
		debugC(1, kDebugScript, "doHelp: no hint configured");
		return;
	}
	if (!_mystery._sawHelpHint && hintFirst != 0xFFFF)
		_mystery._sawHelpHint = true;

	const Common::String raw = _mystery.textAt(use);
	const Common::String text = parseString(raw, _playerName, _partner);

	Graphics::ManagedSurface scratch(320, 200,
		Graphics::PixelFormat::createFormatCLUT8());
	scratch.clear();
	_font.drawString(&scratch, "HELP", 8, 4, 320, 0xF);
	_font.drawWordWrapped(&scratch, 8, 24, 304, text, 0xF);
	g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
							   0, 0, 320, 200);
	g_system->updateScreen();
	waitForInput(60000);
}

void EEMEngine::doInterfaceHelp(uint num) {
	// Mirrors `_InterfaceHelp(num)` @ 1560:0205. The original walks
	// `HelpData @ 29be:00c8` (5-byte entries: u8 count, then up to 2
	// u16 picIds), `_GetPicture`s each one, blits it via
	// `_Rect_Move_Mask(0, 0, ...)` (a MASKED blit on top of the
	// existing screen — transparent pixels show the caller's BG), and
	// waits for click / key. ESC at `1560:02b3` skips to end. The
	// function also hides the cursor at the top (`MOV [0x3a00], 0` at
	// 1560:0216 + `_RemoveMouse @ 1000:542f` at 1560:021c) and
	// restores it at the tail (`_DrawMouse @ 1000:5429` at 1560:02e8).
	//
	// `kHelpPics` lives at file scope above; see comment there for the
	// HelpData decoding.
	if (num >= ARRAYSIZE(kHelpPics))
		return;

	debugC(1, kDebugScript, "doInterfaceHelp(%u): showing pics 0x%x, 0x%x",
		   num, kHelpPics[num][0], kHelpPics[num][1]);

	// Snapshot the caller's screen ONCE so each help PIC overlays the
	// same clean BG. Without this, after the first PIC is dismissed the
	// second snapshot would include the first PIC's pixels and the two
	// would composite together — same gotcha as the setup-screen help
	// loop fix in `doSetup`.
	Graphics::ManagedSurface bg(320, 200,
		Graphics::PixelFormat::createFormatCLUT8());
	{
		Graphics::Surface *cur = g_system->lockScreen();
		if (cur) {
			for (int row = 0; row < 200; row++)
				memcpy((byte *)bg.getBasePtr(0, row),
					   (const byte *)cur->getBasePtr(0, row), 320);
			g_system->unlockScreen();
		}
	}

	const bool wasShown = CursorMan.isVisible();
	CursorMan.showMouse(false);

	for (uint i = 0; i < 2; i++) {
		const uint16 picId = kHelpPics[num][i];
		Picture pic;
		if (!_picsArchive.getPicture(picId, pic)) {
			warning("doInterfaceHelp: getPicture(0x%x) failed", picId);
			continue;
		}
		debugC(1, kDebugScript, "doInterfaceHelp: pic 0x%x = %dx%d flags=0x%x",
			   picId, pic.surface.w, pic.surface.h, pic.flags);

		// Compose a 320x200 frame from the clean BG snapshot and overlay
		// the help pic with `transBlitFrom` — `Graphics::ManagedSurface`'s
		// masked blit (transparent colour = the pic's `flags >> 8`,
		// matching `_Rect_Move_Mask`'s param_10 at 1000:03fc). Pass an
		// explicit `destPos` of (0, 0) — the no-destPos overload at
		// managed_surface.cpp:738 scales src to fill `this`'s rect,
		// stretching the help PIC to 320x200 instead of placing it at
		// native size. The original `_Rect_Move_Mask` passes destX=0,
		// destY=0 with copy-width = pic[+4] (= `pic.surface.w`) and
		// copy-height = pic[+2] (= `pic.surface.h`) — i.e. native size,
		// not stretched.
		Graphics::ManagedSurface scratch(320, 200,
			Graphics::PixelFormat::createFormatCLUT8());
		scratch.simpleBlitFrom(bg);
		const byte transp = (byte)(pic.flags >> 8);
		scratch.transBlitFrom(pic.surface, Common::Point(0, 0),
							  (uint32)transp);
		g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
								   0, 0, 320, 200);
		g_system->updateScreen();

		bool escape = false;
		while (!shouldQuit() && !escape) {
			Common::Event ev;
			bool advance = false;
			while (g_system->getEventManager()->pollEvent(ev)) {
				if (ev.type == Common::EVENT_QUIT ||
					ev.type == Common::EVENT_RETURN_TO_LAUNCHER) {
					if (wasShown)
						CursorMan.showMouse(true);
					return;
				}
				if (ev.type == Common::EVENT_LBUTTONDOWN) {
					advance = true;
					break;
				}
				if (ev.type == Common::EVENT_KEYDOWN) {
					if (ev.kbd.keycode == Common::KEYCODE_ESCAPE)
						escape = true;
					else
						advance = true;
					break;
				}
			}
			if (advance || escape)
				break;
			g_system->updateScreen();
			g_system->delayMillis(15);
		}
		if (escape)
			break;
	}

	if (wasShown)
		CursorMan.showMouse(true);
}

void EEMEngine::setPartnerEraseBg(const Graphics::ManagedSurface *bg) {
	if (bg && bg->w == 320 && bg->h == 200) {
		_partnerEraseBg.create(320, 200,
			Graphics::PixelFormat::createFormatCLUT8());
		for (int row = 0; row < 200; row++) {
			memcpy((byte *)_partnerEraseBg.getBasePtr(0, row),
				   (const byte *)bg->getBasePtr(0, row), 320);
		}
	} else {
		_partnerEraseBg.free();
	}
}

bool EEMEngine::getBalloonInsets(uint16 bubNum, uint16 &xInset,
								  uint16 &yInset, uint16 &textW) const {
	// `kBalloonInsetTable` lives at file scope above; see comment there.
	const uint idx = bubNum & 0x7F;
	if (idx >= ARRAYSIZE(kBalloonInsetTable))
		return false;
	xInset = kBalloonInsetTable[idx].x;
	yInset = kBalloonInsetTable[idx].y;
	textW  = kBalloonInsetTable[idx].w;
	return true;
}

} // End of namespace EEM
