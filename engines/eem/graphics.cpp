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

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/events.h"
#include "common/file.h"
#include "common/path.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/textconsole.h"

#include "graphics/cursorman.h"
#include "graphics/managed_surface.h"

#include "eem/audio.h"
#include "eem/detection.h"
#include "eem/eem.h"
#include "eem/music.h"
#include "eem/site.h"

namespace EEM {

const uint16 kHelpPics[][2] = {
	{ 0x0063, 0x01ae },
	{ 0x0192, 0x01b1 },
};

// kBalloonInsetTable: 52 entries x 10 bytes @ 29be:0875 (CD) / 2608:05f9
// (floppy), indexed by (bubNum & 0x7F). Used by _DisplayClue to position
// _WordWrap text inside the balloon AND by _DisplayHotspotClue_Floppy
// @ 22dc:05c8 to position the "click to continue" indicator. Layout per entry:
//   +0..1 = text X inset
//   +2..3 = text Y inset
//   +4..5 = wrap width
//   +6..7 = "more / end" indicator X within the balloon
//   +8..9 = "more / end" indicator Y within the balloon
struct BalloonInsets {
	uint16 x;
	uint16 y;
	uint16 w;
	uint16 indDX;
	uint16 indDY;
};
const BalloonInsets kBalloonInsetTable[] = {
	{ 6, 4, 142, 150,  6 }, { 6, 4, 142, 150, 14 }, { 6, 4, 142, 150, 22 }, { 6, 4, 142, 150, 40 },
	{ 6, 4, 142, 150, 50 }, { 6, 4, 142, 150, 70 }, { 6, 4, 142, 150, 86 },
	{ 6, 4, 224, 233,  6 }, { 6, 4, 224, 233, 14 }, { 6, 4, 224, 233, 23 }, { 6, 4, 224, 233, 41 },
	{ 6, 4, 224, 233, 59 }, { 6, 4, 224, 233, 68 }, { 6, 4, 224, 233, 77 },
	{ 6, 4, 291, 300,  6 }, { 6, 4, 291, 300, 14 }, { 6, 4, 291, 300, 23 }, { 6, 4, 291, 300, 32 },
	{ 6, 4, 291, 300, 50 }, { 6, 4, 291, 300, 69 }, { 6, 4, 291, 300, 77 },
	{ 5, 4, 155, 150, 14 }, { 5, 4, 155, 150, 25 }, { 5, 4, 155, 150, 35 }, { 5, 4, 155, 150, 52 },
	{ 5, 4, 155, 150, 61 }, { 5, 4, 155, 150, 78 }, { 5, 4, 155, 150, 81 },
	{ 5, 4, 237, 233, 15 }, { 5, 4, 237, 233, 24 }, { 5, 4, 237, 233, 42 }, { 5, 4, 237, 233, 50 },
	{ 5, 4, 237, 233, 69 }, { 5, 4, 237, 233, 80 }, { 5, 4, 237, 233, 80 },
	{ 3, 4, 155, 150, 15 }, { 3, 4, 155, 150, 24 }, { 3, 4, 155, 150, 34 }, { 3, 4, 155, 150, 40 },
	{ 3, 4, 155, 150, 61 }, { 3, 4, 155, 150, 82 }, { 3, 4, 155, 150, 88 },
	{ 5, 4, 238, 232, 16 }, { 5, 4, 238, 232, 25 }, { 5, 4, 238, 232, 34 }, { 5, 4, 238, 232, 51 },
	{ 5, 4, 238, 232, 66 }, { 5, 4, 238, 232, 71 }, { 5, 4, 238, 232, 95 },
	{ 5, 8, 158, 160, 45 }, { 5, 8, 176, 176, 50 }, { 8, 7, 142, 142, 71 }
};

struct BalloonFamily {
	uint16 first;
	uint16 last;
};

const BalloonFamily kBalloonFamilies[] = {
	{ 0x00, 0x06 },
	{ 0x07, 0x0d },
	{ 0x0e, 0x14 },
	{ 0x15, 0x1b },
	{ 0x1c, 0x22 },
	{ 0x23, 0x29 },
	{ 0x2a, 0x30 },
	{ 0x31, 0x31 },
	{ 0x32, 0x32 },
	{ 0x33, 0x33 },
};

bool findBalloonFamily(uint16 balloonId, uint16 &first, uint16 &last) {
	for (uint i = 0; i < ARRAYSIZE(kBalloonFamilies); i++) {
		if (balloonId >= kBalloonFamilies[i].first &&
			balloonId <= kBalloonFamilies[i].last) {
			first = kBalloonFamilies[i].first;
			last  = kBalloonFamilies[i].last;
			return true;
		}
	}
	first = last = balloonId;
	return false;
}

uint getBalloonLineCapacity(uint16 balloonId, int lineH) {
	const uint idx = balloonId & 0x7F;
	if (idx >= ARRAYSIZE(kBalloonInsetTable) || lineH <= 0)
		return 0;

	const BalloonInsets &insets = kBalloonInsetTable[idx];
	return MAX<uint>(1, ((int)insets.indDY - (int)insets.y) / lineH + 1);
}

bool EEMEngine::floppyHotspotSearched(uint siteIdx, uint hotspotIdx) const {
	const byte *site = _mystery.siteData(siteIdx);
	if (!site)
		return false;
	const uint16 dlgListOff = READ_LE_UINT16(site + 6);
	const byte *bufBase = _mystery.blobAt(0);
	const uint32 dsz = _mystery.dataSize();
	if (!bufBase || dlgListOff == 0 || dlgListOff >= dsz)
		return false;
	uint32 off = dlgListOff;
	for (uint h = 0; h < hotspotIdx; h++) {
		if (off + 10 >= dsz)
			return false;
		const uint32 mainLen = 11u + (uint)bufBase[off + 10];
		off += mainLen;
		if (off >= dsz)
			return false;
		const uint contCount = (uint)(bufBase[off] & 0x7F);
		off += 1;
		for (uint c = 0; c < contCount; c++) {
			if (off + 10 >= dsz)
				return false;
			off += 11u + (uint)bufBase[off + 10];
			if (off >= dsz)
				return false;
		}
	}
	if (off + 10 >= dsz)
		return false;
	const uint32 mainLen = 11u + (uint)bufBase[off + 10];
	const uint32 contFlagsOff = off + mainLen;
	if (contFlagsOff >= dsz)
		return false;
	uint32 searchedRecOff = off;
	if ((bufBase[contFlagsOff] & 0x7F) != 0)
		searchedRecOff = contFlagsOff + 1;
	if (searchedRecOff + 11 >= dsz || bufBase[searchedRecOff + 10] == 0)
		return false;
	const uint8 textIdx = bufBase[searchedRecOff + 11] & 0x7F;
	return textIdx < EEM::Mystery::kCluesFoundCap &&
		   _mystery._cluesFound[textIdx] != 0;
}

// fgets-style line read (until newline / NUL / EOF) from a puzzle file.
static Common::String readPuzzleLine(Common::File &f) {
	Common::String s;
	while (!f.eos()) {
		const byte c = f.readByte();
		if (f.eos() || c == '\n' || c == 0)
			break;
		if (c != '\r')
			s += (char)c;
	}
	return s;
}
// `_DoPuzzle @ 2542:1482`. 
bool EEMEngine::doPuzzle(uint puzzleId) {
	Common::File f;
	const Common::String fname = Common::String::format("P%u.BIN", puzzleId);
	if (!f.open(Common::Path(fname))) {
		// Fail open: never let a missing puzzle file permanently block a clue.
		warning("doPuzzle: %s missing — leaving the clue ungated", fname.c_str());
		return true;
	}

	const uint16 type = f.readUint16LE();

	// the puzzle pics nor any bubble is left on the background.
	Graphics::ManagedSurface cleanBg(kScreenWidth, kScreenHeight,
		Graphics::PixelFormat::createFormatCLUT8());
	cleanBg.clear();
	{
		Graphics::Surface *cur = g_system->lockScreen();
		if (cur) {
			cleanBg.simpleBlitFrom(*cur);
			g_system->unlockScreen();
		}
	}
	Graphics::ManagedSurface scratch(kScreenWidth, kScreenHeight,
		Graphics::PixelFormat::createFormatCLUT8());
	scratch.simpleBlitFrom(cleanBg);

	for (int phase = 0; phase < 2; phase++) {
		uint16 n = 1;
		if (phase == 1)
			n = f.readUint16LE();
		for (uint16 i = 0; i < n; i++) {
			const uint16 id = f.readUint16LE();
			const int16 px = (int16)f.readUint16LE();
			const int16 py = (int16)f.readUint16LE();
			Picture pic;
			if (_picsArchive.getPicture(id, pic) && !pic.surface.empty())
				scratch.transBlitFrom(pic.surface, Common::Point(px, py),
									  (uint32)(byte)(pic.flags >> 8));
		}
	}

	const int16 qx = (int16)f.readUint16LE();
	const int16 qy = (int16)f.readUint16LE();
	const int16 qw = (int16)f.readUint16LE();
	const uint16 voiceAlt  = f.readUint16LE();  // partner != Jake
	const uint16 voiceMain = f.readUint16LE();  // partner == Jake
	const Common::String question =
		parseString(readPuzzleLine(f), _playerName, _partner);
	if (_font.isLoaded() && !question.empty())
		_font.drawWordWrapped(&scratch, qx, qy, MAX<int>(8, (int)qw),
							  question, 0);
	g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
							   0, 0, kScreenWidth, kScreenHeight);
	g_system->updateScreen();

	if (_audio && _voiceOn) {
		const uint16 v = (_partner == kPartnerJake) ? voiceMain : voiceAlt;
		if (v != 0 && v != 0xFFFF)
			_audio->spoolSound((uint)(v - 1));
	}

	bool correct = false;
	CursorMan.showMouse(true);

	if (type == 0) {
		const int16 ax1 = (int16)f.readUint16LE();
		const int16 ay1 = (int16)f.readUint16LE();
		const int16 ax2 = (int16)f.readUint16LE();
		const int16 ay2 = (int16)f.readUint16LE();
		const Common::Rect rect(ax1, ay1, ax2, ay2);
		Common::String answer = readPuzzleLine(f);
		const uint maxLen = answer.size() + 2;

		Common::String input;
		bool done = false, blink = true;
		uint32 blinkMs = g_system->getMillis();
		g_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, true);
		while (!done && !shouldQuit()) {
			Graphics::ManagedSurface fld(kScreenWidth, kScreenHeight,
				Graphics::PixelFormat::createFormatCLUT8());
			fld.simpleBlitFrom(scratch);
			Common::String shown = input;
			if (blink)
				shown += "_";
			if (_font.isLoaded())
				_font.drawString(&fld, shown, rect.left + 2, rect.top + 1,
								 MAX<int>(8, rect.width()), 0x0F);
			g_system->copyRectToScreen(fld.getPixels(), fld.pitch, 0, 0,
									   kScreenWidth, kScreenHeight);
			g_system->updateScreen();

			Common::Event ev;
			while (g_system->getEventManager()->pollEvent(ev)) {
				if (ev.type == Common::EVENT_QUIT ||
					ev.type == Common::EVENT_RETURN_TO_LAUNCHER) {
					input.clear();
					done = true;
					break;
				}
				if (ev.type != Common::EVENT_KEYDOWN)
					continue;
				const Common::KeyCode k = ev.kbd.keycode;
				if (k == Common::KEYCODE_RETURN || k == Common::KEYCODE_KP_ENTER) {
					if (!input.empty())
						done = true;
				} else if (k == Common::KEYCODE_ESCAPE) {
					input.clear();
					done = true;
				} else if (k == Common::KEYCODE_BACKSPACE) {
					if (!input.empty())
						input.deleteLastChar();
				} else if (ev.kbd.ascii >= ' ' && ev.kbd.ascii < 127 &&
						   input.size() < maxLen) {
					input += (char)ev.kbd.ascii;
				}
			}
			const uint32 now = g_system->getMillis();
			if (now - blinkMs >= 400) {
				blink = !blink;
				blinkMs = now;
			}
			g_system->delayMillis(15);
		}
		g_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);
		input.toUppercase();
		correct = input.equals(answer);
	} else {
		// Multiple choice: click a region; correct = the FIRST rect.
		const uint16 count = f.readUint16LE();
		Common::Array<Common::Rect> rects;
		for (uint16 i = 0; i < count; i++) {
			const int16 cx1 = (int16)f.readUint16LE();
			const int16 cy1 = (int16)f.readUint16LE();
			const int16 cx2 = (int16)f.readUint16LE();
			const int16 cy2 = (int16)f.readUint16LE();
			rects.push_back(Common::Rect(cx1, cy1, cx2, cy2));
		}
		applyHotspotGlowPalette();
		const bool showBoxes = !ConfMan.getBool("hide_highlight_boxes");
		int picked = -1;
		uint32 lastPhase = (uint32)-1;
		bool overOption = false;
		while (picked == -1 && !shouldQuit()) {
			if (showBoxes) {
				const uint32 phase = g_system->getMillis() / 80;
				if (phase != lastPhase) {
					lastPhase = phase;
					Graphics::ManagedSurface fr(kScreenWidth, kScreenHeight,
						Graphics::PixelFormat::createFormatCLUT8());
					fr.simpleBlitFrom(scratch);
					for (uint i = 0; i < rects.size(); i++) {
						const byte color =
							(byte)(0xF9 + ((i + phase) & 0x07) % 6);
						fr.frameRect(rects[i], color);
					}
					g_system->copyRectToScreen(fr.getPixels(), fr.pitch, 0, 0,
											   kScreenWidth, kScreenHeight);
				}
			}
			const Common::Point mp = g_system->getEventManager()->getMousePos();
			bool nowOver = false;
			for (uint i = 0; i < rects.size(); i++) {
				if (rects[i].contains(mp.x, mp.y)) {
					nowOver = true;
					break;
				}
			}
			if (nowOver != overOption) {
				overOption = nowOver;
				setInteractiveMouseCursor(nowOver);
			}
			Common::Event ev;
			while (g_system->getEventManager()->pollEvent(ev)) {
				if (ev.type == Common::EVENT_QUIT ||
					ev.type == Common::EVENT_RETURN_TO_LAUNCHER ||
					(ev.type == Common::EVENT_KEYDOWN &&
					 ev.kbd.keycode == Common::KEYCODE_ESCAPE)) {
					picked = -2;  // ESC / quit → wrong
					break;
				}
				if (ev.type == Common::EVENT_LBUTTONDOWN) {
					for (uint i = 0; i < rects.size(); i++) {
						if (rects[i].contains(ev.mouse.x, ev.mouse.y)) {
							picked = (int)i;
							break;
						}
					}
				}
			}
			g_system->updateScreen();
			g_system->delayMillis(15);
		}
		// Restore the plain arrow before leaving the puzzle.
		setInteractiveMouseCursor(false);
		correct = (picked == 0);
	}
	f.close();

	g_system->copyRectToScreen(cleanBg.getPixels(), cleanBg.pitch, 0, 0,
							   kScreenWidth, kScreenHeight);
	g_system->updateScreen();

	if (!correct && !shouldQuit()) {
		const byte *kd = _mystery.kdTextIndex();
		const uint16 hintOff = kd ? READ_LE_UINT16(kd + 0x0c) : 0xFFFF;
		if (hintOff != 0xFFFF) {
			if (_music && _voiceOn)
				_music->playMus(40, /* loop= */ false);
			Common::String hint =
				parseString(_mystery.textAt(hintOff), _playerName, _partner);
			Graphics::ManagedSurface ms(kScreenWidth, kScreenHeight,
				Graphics::PixelFormat::createFormatCLUT8());
			ms.simpleBlitFrom(cleanBg);
			const byte firstChar = hint.empty() ? (byte)0 : (byte)hint[0];
			uint16 bubNum = getKDTextBalloon(firstChar);
			if (firstChar >= '0' && firstChar <= '9')
				hint.deleteChar(0);
			bubNum = fitBalloonToText(bubNum, hint);
			Picture balloon;
			const bool haveBalloon = _balloonArchive.size() > (bubNum & 0x7F) &&
				_balloonArchive.loadEntry(bubNum & 0x7F, balloon);
			const int balloonX = 0x21;
			int balloonY = 1;
			if (haveBalloon && balloon.surface.h < 0x4e)
				balloonY = (0x50 - balloon.surface.h) / 2;
			if (haveBalloon)
				ms.transBlitFrom(balloon.surface,
								 Common::Point(balloonX, balloonY),
								 (uint32)(byte)(balloon.flags >> 8));
			uint16 tx = 5, ty = 4, tw = 155;
			getBalloonInsets(bubNum, tx, ty, tw);
			if (_font.isLoaded())
				_font.drawWordWrapped(&ms, balloonX + tx, balloonY + ty, tw,
									  hint, haveBalloon ? 0 : 0xF);
			g_system->copyRectToScreen(ms.getPixels(), ms.pitch, 0, 0,
									   kScreenWidth, kScreenHeight);
			g_system->updateScreen();
			if (_audio && _voiceOn && kd)
				_audio->sayKDDigital(kd, 6, _partner);
			waitForInput(60000);
			stopMusic();

			g_system->copyRectToScreen(cleanBg.getPixels(), cleanBg.pitch,
									   0, 0, kScreenWidth, kScreenHeight);
			g_system->updateScreen();
		}
	}
	setInteractiveMouseCursor(false);
	return correct;
}

void EEMEngine::doHelp() {
	if (isFloppy() && _mystery.isLoaded()) {
		const Common::String filename = Common::String::format("H%u.BIN",
															   _mystery.number());
		Common::File hf;
		if (!hf.open(Common::Path(filename))) {
			warning("doHelp: cannot open %s", filename.c_str());
			return;
		}
		const uint32 hsz = hf.size();
		Common::Array<byte> hbuf;
		hbuf.resize(hsz);
		if (hf.read(hbuf.data(), hsz) != hsz)
			return;
		const byte *hd = hbuf.data();

		const uint chainCount = hd[0];
		uint off = 1;
		uint chainEnd = off + chainCount * 2;
		if (chainEnd >= hsz)
			return;
		const uint extraCount = hd[chainEnd];
		uint extraStart = chainEnd + 1;
		uint extraEnd = extraStart + extraCount * 2;
		if (extraEnd >= hsz)
			return;
		// Three NUL-terminated strings follow.
		const char *str1 = (const char *)(hd + extraEnd);
		const char *str2 = nullptr;
		const char *str3 = nullptr;
		const char *p = str1;
		while ((uint)((const byte *)p - hd) < hsz && *p != 0)
			p++;
		if ((uint)((const byte *)p - hd) >= hsz)
			return;
		str2 = p + 1;
		p = str2;
		while ((uint)((const byte *)p - hd) < hsz && *p != 0)
			p++;
		if ((uint)((const byte *)p - hd) >= hsz)
			return;
		str3 = p + 1;

		const char *chosen = nullptr;
		bool anyChainUnseen = false;
		for (uint i = 0; i < chainCount; i++) {
			const uint8 siteIdx    = hd[off + i * 2 + 0];
			const uint8 hotspotIdx = hd[off + i * 2 + 1];
			if (!floppyHotspotSearched(siteIdx, hotspotIdx)) {
				anyChainUnseen = true;
				break;
			}
		}
		bool anyExtraUnseen = false;
		if (!anyChainUnseen) {
			for (uint i = 0; i < extraCount; i++) {
				const uint8 siteIdx    = hd[extraStart + i * 2 + 0];
				const uint8 hotspotIdx = hd[extraStart + i * 2 + 1];
				if (!floppyHotspotSearched(siteIdx, hotspotIdx)) {
					anyExtraUnseen = true;
					break;
				}
			}
		}
		if (anyChainUnseen)
			chosen = str1;
		else if (anyExtraUnseen)
			chosen = str2;
		else if (_mystery.selectedPoints() >= 100)
			chosen = str3;
		if (!chosen || *chosen == 0)
			return;

		static const uint8 kFloppyDigitToBalloon[10] = {
			0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1c, 0x1d, 0x1e, 0x0a
		};
		uint balloonIdx = 0x17;
		const char *txt = chosen;
		if (*txt >= '0' && *txt <= '9') {
			balloonIdx = kFloppyDigitToBalloon[(int)(*txt - '0')];
			txt++;
		}

		Common::String text = parseString(Common::String(txt),
										   _playerName, _partner);
		balloonIdx = fitBalloonToText((uint16)balloonIdx, text) & 0x7F;
		Graphics::ManagedSurface ms(kScreenWidth, kScreenHeight,
			Graphics::PixelFormat::createFormatCLUT8());
		ms.clear();
		{
			Graphics::Surface *cur = g_system->lockScreen();
			if (cur) {
				ms.simpleBlitFrom(*cur);
				g_system->unlockScreen();
			}
		}
		Picture balloon;
		const bool haveBalloon = _balloonArchive.size() > balloonIdx &&
			_balloonArchive.loadEntry(balloonIdx, balloon);
		uint16 balloonY = 1;
		if (haveBalloon) {
			const uint h = (uint)balloon.surface.h;
			if (h < 0x4e)
				balloonY = (uint16)((0x50 - h) >> 1);
			ms.transBlitFrom(balloon.surface,
							 Common::Point(0x21, balloonY),
							 (uint32)(byte)(balloon.flags >> 8));
		}
		uint16 bx = 5;
		uint16 by = 4;
		uint16 bw = 142;
		getBalloonInsets(balloonIdx, bx, by, bw);
		_font.drawWordWrapped(&ms, 0x21 + bx, balloonY + by,
							  MAX<int>(8, (int)bw), text, 0);
		g_system->copyRectToScreen(ms.getPixels(), ms.pitch, 0, 0, kScreenWidth, kScreenHeight);
		g_system->updateScreen();

		while (!shouldQuit()) {
			Common::Event ev;
			bool advance = false;
			while (g_system->getEventManager()->pollEvent(ev)) {
				if (ev.type == Common::EVENT_QUIT ||
					ev.type == Common::EVENT_RETURN_TO_LAUNCHER ||
					ev.type == Common::EVENT_LBUTTONDOWN ||
					ev.type == Common::EVENT_KEYDOWN) {
					advance = true;
					break;
				}
			}
			if (advance)
				break;
			g_system->updateScreen();
			g_system->delayMillis(10);
		}
		return;
	}

	// per-puzzle hint: partner points at whichever chain clue the
	// player hasn't found yet, only falling back to the generic line once
	// every chain hint has been triggered.
	if (!_mystery.isLoaded() || !_font.isLoaded())
		return;

	const byte *kd  = _mystery.kdTextIndex();
	const byte *hb  = _mystery.hintBlock();
	if (!kd)
		return;

	uint16 chosenText = 0xFFFF;
	int    soundNum   = 0;
	int    hintVoiceSlot = -1;
	bool   anyHintDefined = false;

	const uint kChains = isLondon() ? 3u : 1u;
	const uint kSlots  = isLondon() ? Mystery::kChainLen : 2u;
	if (hb) {
		for (uint c = 0; c < kChains && chosenText == 0xFFFF; c++) {
			for (uint slot = 0; slot < kSlots; slot++) {
				const uint16 chainClue = _mystery.hintChain(c, slot);
				if (chainClue == 0xFFFF)
					continue;
				const uint16 hintOff = READ_LE_UINT16(
					hb + (c * Mystery::kChainLen + slot) * 2);
				if (hintOff == 0xFFFF)
					continue;
				anyHintDefined = true;
				if (chainClue < Mystery::kCluesFoundCap &&
					_mystery._cluesFound[chainClue] == 0) {
					chosenText = hintOff;
					if (isLondon())
						hintVoiceSlot = (int)slot;
					else
						soundNum = (int)slot + 10;
					break;
				}
			}
		}
	}

	if (chosenText == 0xFFFF) {
		if (anyHintDefined) {
			const uint16 hintFirst  = READ_LE_UINT16(kd + 0x0e);
			const uint16 hintSecond = READ_LE_UINT16(kd + 0x10);
			if (!_mystery._sawHelpHint && hintFirst != 0xFFFF) {
				chosenText = hintFirst;
				soundNum   = 7;
				_mystery._sawHelpHint = true;
			} else if (hintSecond != 0xFFFF) {
				chosenText = hintSecond;
				soundNum   = 8;
			}
		}
		// Else: original would render NoHints string @ 29be:00d3; we bail.
	}

	if (chosenText == 0xFFFF) {
		debugC(1, kDebugScript, "doHelp: no hint available");
		return;
	}

	const Common::String raw  = _mystery.textAt(chosenText);
	Common::String text = parseString(raw, _playerName, _partner);
	Graphics::ManagedSurface ms(kScreenWidth, kScreenHeight,
		Graphics::PixelFormat::createFormatCLUT8());
	ms.clear();
	{
		Graphics::Surface *cur = g_system->lockScreen();
		if (cur) {
			ms.simpleBlitFrom(*cur);
			g_system->unlockScreen();
		}
	}

	const byte firstChar =
		text.empty() ? (byte)0 : (byte)text[0];
	uint16 bubNum = getKDTextBalloon(firstChar);
	if (firstChar >= '0' && firstChar <= '9')
		text.deleteChar(0);
	bubNum = fitBalloonToText(bubNum, text);
	Picture balloon;
	const bool haveBalloon =
		_balloonArchive.size() > (bubNum & 0x7F) &&
		_balloonArchive.loadEntry(bubNum & 0x7F, balloon);

	const int balloonX = 0x21;
	int balloonY = 1;
	if (haveBalloon && balloon.surface.h < 0x4e)
		balloonY = (0x50 - balloon.surface.h) / 2;

	if (haveBalloon) {
		const byte transp = (byte)(balloon.flags >> 8);
		ms.transBlitFrom(balloon.surface,
						 Common::Point(balloonX, balloonY),
						 (uint32)transp);
	}

	uint16 tx = 5, ty = 4, tw = 155;
	getBalloonInsets(bubNum, tx, ty, tw);
	_font.drawWordWrapped(&ms, balloonX + tx, balloonY + ty, tw, text,
						  haveBalloon ? 0 : 0xF);

	g_system->copyRectToScreen(ms.getPixels(), ms.pitch,
							   0, 0, kScreenWidth, kScreenHeight);
	g_system->updateScreen();

	if (_audio && _mystery.kdTextIndex()) {
		if (hintVoiceSlot >= 0)
			_audio->sayKDHintDigital(_mystery.kdTextIndex(),
									 (uint)hintVoiceSlot, _partner);
		else if (soundNum > 0)
			_audio->sayKDDigital(_mystery.kdTextIndex(), (uint)soundNum,
								 _partner);
	}

	waitForInput(60000);
}
// _InterfaceHelp(num) @ 1560:0205
void EEMEngine::doInterfaceHelp(uint num) {
	if (num >= ARRAYSIZE(kHelpPics))
		return;

	debugC(1, kDebugScript, "doInterfaceHelp(%u): showing pics 0x%x, 0x%x",
		   num, kHelpPics[num][0], kHelpPics[num][1]);

	// Snapshot caller's screen once: each PIC overlays the same clean BG.
	Graphics::ManagedSurface bg(kScreenWidth, kScreenHeight,
		Graphics::PixelFormat::createFormatCLUT8());
	{
		Graphics::Surface *cur = g_system->lockScreen();
		if (cur) {
			bg.simpleBlitFrom(*cur);
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

		Graphics::ManagedSurface scratch(kScreenWidth, kScreenHeight,
			Graphics::PixelFormat::createFormatCLUT8());
		scratch.simpleBlitFrom(bg);
		const byte transp = (byte)(pic.flags >> 8);
		scratch.transBlitFrom(pic.surface, Common::Point(0, 0),
							  (uint32)transp);
		g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
								   0, 0, kScreenWidth, kScreenHeight);
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
	if (bg && bg->w == kScreenWidth && bg->h == kScreenHeight) {
		_partnerEraseBg.create(kScreenWidth, kScreenHeight,
			Graphics::PixelFormat::createFormatCLUT8());
		_partnerEraseBg.simpleBlitFrom(*bg);
	} else {
		_partnerEraseBg.free();
	}
}

uint16 EEMEngine::fitBalloonToText(uint16 bubNum,
								   const Common::String &text) {
	// Opt-in via "fit_dialog_balloons", CD only
	if (isFloppy() || !ConfMan.getBool("fit_dialog_balloons"))
		return bubNum;

	const uint16 originalId = bubNum & 0x7F;
	if (bubNum == 0xFFFF || text.empty() || !_font.isLoaded() ||
		originalId >= ARRAYSIZE(kBalloonInsetTable))
		return bubNum;

	const BalloonInsets &originalInsets = kBalloonInsetTable[originalId];
	const int lineH = _font.getFontHeight();
	const uint originalCapacity = getBalloonLineCapacity(originalId, lineH);
	if (originalCapacity == 0)
		return bubNum;

	Common::Array<Common::String> lines;
	_font.wordWrapText(text, MAX<int>(8, (int)originalInsets.w), lines);
	if (lines.empty())
		return bubNum;

	const uint usedLines = lines.size();
	if (usedLines > originalCapacity)
		return bubNum;

	uint16 familyFirst = 0;
	uint16 familyLast = 0;
	if (!findBalloonFamily(originalId, familyFirst, familyLast))
		return bubNum;

	uint16 chosenId = originalId;
	uint chosenCapacity = originalCapacity;
	debug(
		   "fitBalloonToText: original 0x%02x, usedLines=%u, capacity=%u, family=0x%02x-0x%02x",
		   (int)originalId, usedLines, originalCapacity,
		   (int)familyFirst, (int)familyLast);
	while (chosenId > familyFirst) {
		if (chosenCapacity < usedLines)
			break;

		const uint16 candidateId = chosenId - 1;
		if (candidateId >= ARRAYSIZE(kBalloonInsetTable) ||
			kBalloonInsetTable[candidateId].w != originalInsets.w)
			break;
		const uint candidateCapacity =
			getBalloonLineCapacity(candidateId, lineH);
		if (candidateCapacity < usedLines)
			break;
		chosenId = candidateId;
		chosenCapacity = candidateCapacity;
	}

	if (chosenId == originalId)
		return bubNum;

	debug(
		   "fitBalloonToText: 0x%02x -> 0x%02x (%u lines, capacity %u -> %u)",
		   (int)originalId, (int)chosenId, usedLines,
		   originalCapacity, chosenCapacity);
	return (bubNum & 0x80) | chosenId;
}

bool EEMEngine::getBalloonInsets(uint16 bubNum, uint16 &xInset,
								  uint16 &yInset, uint16 &textW) const {
	const uint idx = bubNum & 0x7F;
	if (idx >= ARRAYSIZE(kBalloonInsetTable))
		return false;
	xInset = kBalloonInsetTable[idx].x;
	yInset = kBalloonInsetTable[idx].y;
	textW  = kBalloonInsetTable[idx].w;
	return true;
}

bool EEMEngine::getBalloonIndicatorPos(uint16 bubNum, uint16 &dx,
										uint16 &dy) const {
	const uint idx = bubNum & 0x7F;
	if (idx >= ARRAYSIZE(kBalloonInsetTable))
		return false;
	dx = kBalloonInsetTable[idx].indDX;
	dy = kBalloonInsetTable[idx].indDY;
	return true;
}

void EEMEngine::drawFloppyBubbleIndicator(Graphics::ManagedSurface &dst,
										   uint16 bubNum, int ballX, int ballY,
										   bool endIndicator) {
	uint16 dx = 0;
	uint16 dy = 0;
	if (!getBalloonIndicatorPos(bubNum, dx, dy))
		return;
	const uint picId = endIndicator ? 0xa1 : 0xa0;
	Picture pic;
	if (!_picsArchive.getPicture(picId, pic))
		return;
	const int x = ballX + (int)dx;
	const int y = ballY + (int)dy;
	dst.transBlitFrom(pic.surface, Common::Point(x, y),
					  (uint32)(byte)(pic.flags >> 8));
}

} // End of namespace EEM
