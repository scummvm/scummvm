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

#include "colony/colony.h"
#include "colony/gfx.h"
#include "common/system.h"
#include "common/events.h"
#include "common/debug.h"
#include "graphics/font.h"
#include "graphics/fonts/dosfont.h"
#include "graphics/fonts/macfont.h"
#include "graphics/cursorman.h"
#include "image/pict.h"
#include <math.h>

namespace Colony {

void ColonyEngine::playIntro() {
	if (getPlatform() == Common::kPlatformMacintosh) {
		// Load the Mac "Commando" font (FOND 190, 12pt) from Colony resources.
		// Original intro.c: TextFont(190); TextSize(12);
		// FONT resource ID = FOND_ID * 128 + size = 190 * 128 + 12 = 24332
		// Some builds store it as NFNT instead of FONT.
		Graphics::MacFONTFont *macFont = nullptr;
		if (_resMan) {
			const uint16 fontResID = 24332;
			Common::SeekableReadStream *fontStream = _resMan->getResource(MKTAG('N', 'F', 'N', 'T'), fontResID);
			if (!fontStream)
				fontStream = _resMan->getResource(MKTAG('F', 'O', 'N', 'T'), fontResID);
			if (fontStream) {
				macFont = new Graphics::MacFONTFont();
				if (!macFont->loadFont(*fontStream)) {
					warning("playIntro: failed to load Commando 12pt font");
					delete macFont;
					macFont = nullptr;
				}
				delete fontStream;
			} else {
				// List available font resources for debugging
				Common::MacResIDArray nfntIDs = _resMan->getResIDArray(MKTAG('N', 'F', 'N', 'T'));
				Common::MacResIDArray fontIDs = _resMan->getResIDArray(MKTAG('F', 'O', 'N', 'T'));
				debugC(1, kColonyDebugUI, "playIntro: FONT/NFNT %d not found. Available NFNT IDs: %d, FONT IDs: %d",
				      fontResID, nfntIDs.size(), fontIDs.size());
				for (uint i = 0; i < nfntIDs.size(); i++)
					debugC(1, kColonyDebugUI, "  NFNT %d", nfntIDs[i]);
				for (uint i = 0; i < fontIDs.size(); i++)
					debugC(1, kColonyDebugUI, "  FONT %d", fontIDs[i]);
			}
		}

		// Original: intro() in intro.c, lines 40-119
		// qt flag propagates through sections  only modifier+click sets it
		bool qt = false;

		// 1. ScrollInfo() - scrolling story text with BeamMe sound
		qt = scrollInfo(macFont);

		// 2. Wait for BeamMe sound to finish
		// Original: if(!qt) while(!SoundDone());
		while (!qt && !shouldQuit() && _sound->isPlaying())
			_system->delayMillis(10);

		// Original: if(Button()) qt=OptionKey();  check for skip
		if (!qt)
			qt = checkSkipRequested();

		if (!qt) {
			// 3. Logo 1 + PlayMars + makestars
			// Original: FillRect black; DoPicture; PlayMars(); makestars()
			_gfx->clear(_gfx->black());
			if (!drawPict(-32565))  // Color Colony
				drawPict(-32748);   // B&W Colony
			_sound->play(Sound::kMars);
			qt = makeStars(_screenR, 0);

			if (!qt) {
				// 4. Logo 2 + makestars (inside the same !qt block as original)
				// Original: FillRect black; DoPicture(-32564); makestars()
				_gfx->clear(_gfx->black());
				if (!drawPict(-32564))  // Color Colony
					drawPict(-32750);   // B&W Colony
				qt = makeStars(_screenR, 0);
			}

			if (!qt) {
				// 5. Empty starfield
				// Original: FillRect black; makestars()
				_gfx->clear(_gfx->black());
				_gfx->copyToScreen();
				qt = makeStars(_screenR, 0);
			}

			if (!qt) {
				// 6. TimeSquare("...BLACK HOLE COLLISION...")
				qt = timeSquare("...BLACK HOLE COLLISION...", macFont);
			}

			if (!qt) {
				// 7. Makeblackhole()
				_gfx->clear(_gfx->black());
				_gfx->copyToScreen();
				qt = makeBlackHole();
			}

			if (!qt) {
				// 8. TimeSquare("...FUEL HAS BEEN DEPLETED...")
				qt = timeSquare("...FUEL HAS BEEN DEPLETED...", macFont);
			}

			// Original: SetPort(&metaPort); before next TimeSquare (no !qt guard)
			if (!qt) {
				// 9. TimeSquare("...PREPARE FOR CRASH LANDING...")
				qt = timeSquare("...PREPARE FOR CRASH LANDING...", macFont);
			}

			if (!qt) {
				// 10. makeplanet() + EndCSound()
				// Simplified: starfield + delay (makeplanet draws a rotating planet)
				_gfx->clear(_gfx->black());
				_gfx->copyToScreen();
				qt = makeStars(_screenR, 0);
				_sound->stop(); // EndCSound()
			}
		}

		// 11. Final crash  always runs (even if qt)
		// Original: FillRect black; if(!qt) while(!SoundDone());
		_gfx->clear(_gfx->black());
		_gfx->copyToScreen();
		while (!qt && !shouldQuit() && _sound->isPlaying())
			_system->delayMillis(10);

		// Original: DoExplodeSound(); while(!SoundDone()) InvertRect(&rScreen); StopSound();
		_sound->play(Sound::kExplode);
		while (!shouldQuit() && _sound->isPlaying()) {
			_gfx->clear(_gfx->white());
			_gfx->copyToScreen();
			_system->delayMillis(50);
			_gfx->clear(_gfx->black());
			_gfx->copyToScreen();
			_system->delayMillis(50);
		}
		_sound->stop();
		_gfx->clear(_gfx->black());
		_gfx->copyToScreen();
		delete macFont;
		macFont = nullptr;

		// Restore palette entries modified during intro (128, 160-176, 200-213)
		// back to grayscale so normal gameplay rendering isn't affected
		byte restorePal[256 * 3];
		for (int i = 0; i < 256; i++) {
			restorePal[i * 3 + 0] = i;
			restorePal[i * 3 + 1] = i;
			restorePal[i * 3 + 2] = i;
		}
		_gfx->setPalette(restorePal + 128 * 3, 128, 128);
	} else {
		scrollInfo();
	}
}

bool ColonyEngine::scrollInfo(const Graphics::Font *macFont) {
	// Original: ScrollInfo() in intro.c, lines 138-221
	// Renders story text in blue gradient to offscreen half-width buffer,
	// scrolls it up from below screen with DoBeammeSound(),
	// waits for click, then scrolls it off the top.
	// Mac original: TextFont(190 = Commando); TextSize(12);
	// Text blue starts at 0xFFFF and fades by -4096 per visible line.
	const char *story[] = {
		"",
		"Mankind has left the",
		"cradle of earth and",
		"is beginning to eye",
		"the galaxy. He has",
		"begun to colonize",
		"distant planets but has",
		"yet to meet any alien",
		"life forms.",
		"",      // null separator in original
		"Until now...",
		"",      // null separator in original
		"Click to begin",
		"the Adventure..."
	};
	const int storyLength = ARRAYSIZE(story);

	if (getPlatform() == Common::kPlatformMacintosh)
		_sound->play(Sound::kBeamMe);

	_gfx->clear(_gfx->black());
	_gfx->copyToScreen();

	Graphics::DosFont dosFont;
	const Graphics::Font *font = macFont ? macFont : (const Graphics::Font *)&dosFont;

	// Original uses 19px line height, centers vertically within height
	int lineHeight = 19;
	int totalHeight = lineHeight * storyLength;
	int ht = (_height - totalHeight) / 2;

	// Set up gradient palette entries (200-213) for story text
	// Mac original: tColor.blue starts at 0xFFFF and decreases by 4096 per visible line
	// B&W Mac: white gradient instead of blue
	const bool bwMac = (macFont && !_hasMacColors);
	byte pal[14 * 3]; // storyLength entries
	memset(pal, 0, sizeof(pal));
	for (int i = 0; i < storyLength; i++) {
		int val = 255 - i * 16;
		if (val < 0)
			val = 0;
		pal[i * 3 + 0] = bwMac ? val : 0;  // R
		pal[i * 3 + 1] = bwMac ? val : 0;  // G
		pal[i * 3 + 2] = val;               // B
	}
	_gfx->setPalette(pal, 200, storyLength);

	// Phase 1: Scroll text up from below screen
	// Original: scrollRect starts at bottom (stayRect.bottom..stayRect.bottom*2),
	// moves up by inc=4 each frame until text is visible at its correct position.
	// We simulate by drawing text with a y-offset that starts at _height and decreases to 0.
	int inc = 4;
	bool qt = false;

	for (int scrollOff = _height; scrollOff > 0 && !qt; scrollOff -= inc) {
		if (checkSkipRequested()) {
			qt = true;
			_sound->stop();
			break;
		}

		_gfx->clear(_gfx->black());
		for (int i = 0; i < storyLength; i++) {
			int drawY = ht + lineHeight * i + scrollOff;
			if (strlen(story[i]) > 0 && drawY >= 0 && drawY < _height)
				_gfx->drawString(font, story[i], _width / 2, drawY, 200 + i, Graphics::kTextAlignCenter);
		}
		_gfx->copyToScreen();
		_system->delayMillis(16);
	}

	// Draw final position (scrollOff = 0)
	if (!qt) {
		_gfx->clear(_gfx->black());
		for (int i = 0; i < storyLength; i++) {
			if (strlen(story[i]) > 0)
				_gfx->drawString(font, story[i], _width / 2, ht + lineHeight * i, 200 + i, Graphics::kTextAlignCenter);
		}
		_gfx->copyToScreen();
	}

	// Wait for click (original: while(!Button()); while(Button()&&!qt);)
	if (!qt)
		qt = waitForInput();

	// Phase 2: Scroll text off the top of the screen
	// Original: scrollRect continues moving up, text slides upward
	if (!qt) {
		for (int scrollOff = 0; scrollOff > -_height && !qt; scrollOff -= inc) {
			if (checkSkipRequested()) {
			qt = true;
			_sound->stop();
			break;
		}

			_gfx->clear(_gfx->black());
			for (int i = 0; i < storyLength; i++) {
				int drawY = ht + lineHeight * i + scrollOff;
				if (strlen(story[i]) > 0 && drawY >= -lineHeight && drawY < _height)
					_gfx->drawString(font, story[i], _width / 2, drawY, 200 + i, Graphics::kTextAlignCenter);
			}
			_gfx->copyToScreen();
			_system->delayMillis(16);
		}
	}

	// Original does NOT stop the sound here  BeamMe continues playing
	// and intro() waits for it with while(!SoundDone()) after ScrollInfo returns.
	// Only stop if skipping (qt already stops in the modifier+click handlers above).
	_gfx->clear(_gfx->black());
	_gfx->copyToScreen();
	return qt;
}

bool ColonyEngine::makeStars(const Common::Rect &r, int btn) {
	// Original: makestars() in stars.c
	// Uses 75 moving stars that streak outward from center using XOR lines.
	const int MAXSTAR = 0x1FF;
	const int NSTARS = 75;
	const int deltapd = 0x008;

	int centerX = r.width() / 2;
	int centerY = r.height() / 2;

	// Build perspective lookup table: rtable[i] = (128*128)/i
	int rtable[MAXSTAR + 1];
	rtable[0] = 32000;
	for (int i = 1; i <= MAXSTAR; i++)
		rtable[i] = (128 * 128) / i;

	// First draw static background stars (150 random dots)
	for (int i = 0; i < 150; i++) {
		int s = (int16)(_randomSource.getRandomNumber(0xFFFF)) >> 7;
		int c = (int16)(_randomSource.getRandomNumber(0xFFFF)) >> 7;
		int d = _randomSource.getRandomNumber(MAXSTAR);
		if (d < 1) d = 1;
		int rr = rtable[d];
		int xx = centerX + (int)(((long long)s * rr) >> 7);
		int yy = centerY + (int)(((long long)c * rr) >> 7);
		if (xx >= 0 && xx < _width && yy >= 0 && yy < _height)
			_gfx->setPixel(xx, yy, 15);
	}

	// Initialize moving stars  original uses PenMode(patXor) so stars
	// don't damage the logo underneath (XOR drawing the same line twice
	// restores the original pixels).
	int xang[NSTARS], yang[NSTARS], dist[NSTARS];
	int xsave1[NSTARS], ysave1[NSTARS], xsave2[NSTARS], ysave2[NSTARS];

	_gfx->setXorMode(true);

	for (int i = 0; i < NSTARS; i++) {
		int d = dist[i] = _randomSource.getRandomNumber(MAXSTAR);
		if (d <= 0x030) d = dist[i] = MAXSTAR;
		int s = xang[i] = (int16)(_randomSource.getRandomNumber(0xFFFF)) >> 7;
		int c = yang[i] = (int16)(_randomSource.getRandomNumber(0xFFFF)) >> 7;

		int rr = rtable[d];
		xsave1[i] = centerX + (int)(((long long)s * rr) >> 7);
		ysave1[i] = centerY + (int)(((long long)c * rr) >> 7);

		int d2 = d - deltapd;
		if (d2 < 1)
			d2 = 1;
		rr = rtable[d2];
		xsave2[i] = centerX + (int)(((long long)s * rr) >> 7);
		ysave2[i] = centerY + (int)(((long long)c * rr) >> 7);

		_gfx->drawLine(xsave1[i], ysave1[i], xsave2[i], ysave2[i], 15);
	}
	_gfx->copyToScreen();

	// Animate: original loops ~200 frames or until Mars sound repeats 2x
	for (int k = 0; k < 120; k++) {
		if (checkSkipRequested()) {
			_gfx->setXorMode(false);
			return true;
		}

		for (int i = 0; i < NSTARS; i++) {
			// Erase previous  XOR the same line again to restore underlying pixels
			_gfx->drawLine(xsave1[i], ysave1[i], xsave2[i], ysave2[i], 15);

			int s = xang[i];
			int c = yang[i];

			if (dist[i] <= 0x030) {
				dist[i] = MAXSTAR;
				int rr = rtable[MAXSTAR];
				xsave1[i] = centerX + (int)(((long long)s * rr) >> 7);
				ysave1[i] = centerY + (int)(((long long)c * rr) >> 7);
			} else {
				xsave1[i] = xsave2[i];
				ysave1[i] = ysave2[i];
			}

			int d = (dist[i] -= deltapd);
			if (d < 1) d = 1;
			int rr = rtable[d];
			xsave2[i] = centerX + (int)(((long long)s * rr) >> 7);
			ysave2[i] = centerY + (int)(((long long)c * rr) >> 7);

			// Draw new star position
			_gfx->drawLine(xsave1[i], ysave1[i], xsave2[i], ysave2[i], 15);
		}
		_gfx->copyToScreen();
		_system->delayMillis(16);
	}

	// Fade-out phase: stars fly off without resetting (trails accumulate via XOR)
	int nstars = 2 * ((MAXSTAR - 0x030) / deltapd);
	if (nstars > 200) nstars = 200;
	for (int k = 0; k < nstars; k++) {
		if (checkSkipRequested()) {
			_gfx->setXorMode(false);
			return true;
		}

		for (int i = 0; i < NSTARS; i++) {
			int d = dist[i];
			int s = xang[i];
			int c = yang[i];
			dist[i] -= deltapd;
			if (dist[i] <= 0x030) dist[i] = MAXSTAR;

			if (d >= 1 && d <= MAXSTAR) {
				int rr1 = rtable[d];
				int d2 = d - deltapd;
				if (d2 < 1)
			d2 = 1;
				int rr2 = rtable[d2];
				int x1 = centerX + (int)(((long long)s * rr1) >> 7);
				int y1 = centerY + (int)(((long long)c * rr1) >> 7);
				int x2 = centerX + (int)(((long long)s * rr2) >> 7);
				int y2 = centerY + (int)(((long long)c * rr2) >> 7);
				_gfx->drawLine(x1, y1, x2, y2, 15);
			}
		}
		_gfx->copyToScreen();
		_system->delayMillis(8);
	}

	_gfx->setXorMode(false);
	return false;
}

bool ColonyEngine::makeBlackHole() {
	// Original: Makeblackhole() in intro.c
	// Mac original draws spiral lines with fading colors:
	// bcolor starts at (0,0,0) and subtracts (rd=2048, gd=1024, bd=4096) per step,
	// which wraps around creating shifting color gradients.
	// We use palette entries 128-191 for the gradient colors.
	int centerX = _width / 2;
	int centerY = _height / 2;
	int dec = 16;
	int starcnt = 0;

	// Build a lookup table matching the original rtable: rtable[i] = (128*128)/i
	int rtable[1024];
	rtable[0] = 32000;
	for (int i = 1; i < 1024; i++)
		rtable[i] = (128 * 128) / i;

	for (int k = 0; k < 17; k += 4) {
		// Reset color per k-iteration (matches Mac: bcolor = {0,0,0} at start of each k)
		int colorR = 0, colorG = 0, colorB = 0;
		int rd = 2048, gd = 1024, bd = 4096;

		for (int i = 1000; i > 32; i -= dec) {
			// Mac original subtracts from color channels (wrapping as uint16);
			// We simulate this as a gradient from dark to bright
			// Since Mac uses unsigned wrap: 0 - 4096 = 0xF000 = bright.
			// After one full cycle (16 steps), the colors cycle.
			// Map to palette entry based on step
			colorB = (colorB - bd) & 0xFFFF;
			colorR = (colorR - rd) & 0xFFFF;
			colorG = (colorG - gd) & 0xFFFF;

			// Map Mac 16-bit color to 8-bit
			uint8 palR = (colorR >> 8) & 0xFF;
			uint8 palG = (colorG >> 8) & 0xFF;
			uint8 palB = (colorB >> 8) & 0xFF;

			// Use palette entry 128 for current step color
			byte pal[3] = { palR, palG, palB };
			_gfx->setPalette(pal, 128, 1);

			starcnt++;
			if (starcnt == 8) starcnt = 0;

			for (int j = 0; j < 256; j += 8) {
				int idx = (j + starcnt) & 0xFF;
				int rt1 = rtable[MIN(i + k, 1023)];
				int x1 = centerX + (int)(((long long)rt1 * _sint[idx]) >> 7);
				int y1 = centerY + (int)(((long long)rt1 * _cost[idx]) >> 7);

				int rt2 = rtable[MIN(i + k + 8, 1023)];
				int x2 = centerX + (int)(((long long)rt2 * _sint[idx]) >> 7);
				int y2 = centerY + (int)(((long long)rt2 * _cost[idx]) >> 7);

				_gfx->drawLine(x1, y1, x2, y2, 128);
			}

			// Update screen every step and add a small delay
			// to simulate the original 68k rendering speed
			_gfx->copyToScreen();
			_system->delayMillis(16);

			if (checkSkipRequested()) return true;
		}
	}
	_gfx->copyToScreen();
	return false;
}

bool ColonyEngine::timeSquare(const Common::String &str, const Graphics::Font *macFont) {
	// Original: TimeSquare() in intro.c
	// 1. Draw horizontal blue gradient lines above/below center
	// 2. Scroll red text from right to center
	// 3. Flash klaxon 6 times with inverted rect
	// 4. Play Mars again, scroll text off to the left
	//
	// Mac original: fcolor starts at (0,0,0xFFFF) and subtracts 4096 per pair of lines.
	// Text is drawn in red (0xFFFF,0,0) on black background.

	_gfx->clear(_gfx->black());

	Graphics::DosFont dosFont;
	const Graphics::Font *font = macFont ? macFont : (const Graphics::Font *)&dosFont;
	int swidth = font->getStringWidth(str);

	int centery = _height / 2 - 10;

	// Set up gradient palette entries (160-175) for the gradient lines
	// Mac original: blue starts at 0xFFFF and decreases by 4096 per line pair
	// B&W Mac: white gradient instead of blue
	const bool bwMac = (macFont && !_hasMacColors);
	for (int i = 0; i < 16; i++) {
		int val = 255 - i * 16; // 255, 239, 223, ... 15
		if (val < 0)
			val = 0;
		byte pal[3] = { (byte)(bwMac ? val : 0), (byte)(bwMac ? val : 0), (byte)val };
		_gfx->setPalette(pal, 160 + i, 1);
	}
	// Set palette entry 176 for text (red in color, white in B&W)
	{
		byte pal[3] = { 255, (byte)(bwMac ? 255 : 0), (byte)(bwMac ? 255 : 0) };
		_gfx->setPalette(pal, 176, 1);
	}

	// Draw blue gradient lines above/below center band
	for (int i = 0; i < 16; i++) {
		_gfx->drawLine(0, centery - 2 - i * 2, _width, centery - 2 - i * 2, 160 + i);
		_gfx->drawLine(0, centery - 2 - (i * 2 + 1), _width, centery - 2 - (i * 2 + 1), 160 + i);
		_gfx->drawLine(0, centery + 16 + i * 2, _width, centery + 16 + i * 2, 160 + i);
		_gfx->drawLine(0, centery + 16 + i * 2 + 1, _width, centery + 16 + i * 2 + 1, 160 + i);
	}
	_gfx->copyToScreen();

	// Phase 1: Scroll text in from the right to center
	// Original: if(Button()) if(qt=OptionKey()) break;
	int targetX = (_width - swidth) / 2;
	for (int x = _width; x > targetX; x -= 2) {
		_gfx->fillRect(Common::Rect(0, centery + 1, _width, centery + 16), 0);
		_gfx->drawString(font, str, x, centery + 2, 176, Graphics::kTextAlignLeft);
		_gfx->copyToScreen();

		if (checkSkipRequested()) return true;
		_system->delayMillis(8);
	}

	// Phase 2: Klaxon flash  original: EndCSound(); then 6 iterations of:
	//   if(Button()) if(qt=OptionKey()) break;
	//   while(!SoundDone()); StopSound(); PlayKlaxon(); InvertRect(&invrt);
	_sound->stop(); // EndCSound()
	for (int i = 0; i < 6; i++) {
		if (checkSkipRequested()) return true;

		// Wait for previous klaxon to finish
		while (_sound->isPlaying() && !shouldQuit())
			_system->delayMillis(10);
		_sound->stop();

		_sound->play(Sound::kKlaxon);

		// InvertRect(&invrt)  toggle the text band
		_gfx->fillRect(Common::Rect(0, centery + 1, _width, centery + 16), i % 2 ? 0 : 15);
		_gfx->drawString(font, str, targetX, centery + 2, i % 2 ? 15 : 0, Graphics::kTextAlignLeft);
		_gfx->copyToScreen();
	}
	// Wait for last klaxon
	while (_sound->isPlaying() && !shouldQuit())
		_system->delayMillis(10);
	_sound->stop();

	// Phase 3: PlayMars(), scroll text off to the left
	_sound->play(Sound::kMars);
	for (int x = targetX; x > -swidth; x -= 2) {
		_gfx->fillRect(Common::Rect(0, centery + 1, _width, centery + 16), 0);
		_gfx->drawString(font, str, x, centery + 2, 176, Graphics::kTextAlignLeft);
		_gfx->copyToScreen();

		if (checkSkipRequested()) return true;
		_system->delayMillis(8);
	}

	return false;
}

bool ColonyEngine::drawPict(int resID) {
	// Original: DoPicture() in intro.c, lines 861-886
	// Loads a PICT resource, centers it in the screen rect, draws with srcCopy.
	// Original applies clip rect inset by 1 pixel on all sides.
	Common::SeekableReadStream *pictStream = nullptr;

	// Try Color Colony resource fork first
	if (_colorResMan && _colorResMan->hasResFork())
		pictStream = _colorResMan->getResource(MKTAG('P', 'I', 'C', 'T'), (int16)resID);

	// Fall back to B&W Colony resource fork
	if (!pictStream && _resMan && (_resMan->isMacFile() || _resMan->hasResFork()))
		pictStream = _resMan->getResource(MKTAG('P', 'I', 'C', 'T'), (int16)resID);

	if (!pictStream) {
		debugC(1, kColonyDebugUI, "drawPict: PICT %d not found", resID);
		return false;
	}

	::Image::PICTDecoder decoder;
	if (decoder.loadStream(*pictStream)) {
		const Graphics::Surface *surface = decoder.getSurface();
		if (surface) {
			// Center PICT on screen (original: locate = centered within rScreen)
			int x = (_width - surface->w) / 2;
			int y = (_height - surface->h) / 2;
			bool isCLUT8 = (surface->format == Graphics::PixelFormat::createFormatCLUT8());
			const Graphics::Palette &pictPal = decoder.getPalette();

			// Original DoPicture clips 1 pixel inset from locate rect
			// clip.top = locate.top+1, clip.left = locate.left+1, etc.
			int clipX1 = x + 1;
			int clipY1 = y + 1;
			int clipX2 = x + surface->w - 1;
			int clipY2 = y + surface->h - 1;

			debugC(1, kColonyDebugUI, "drawPict(%d): %dx%d at (%d,%d), format=%dbpp, palette=%d entries",
			      resID, surface->w, surface->h, x, y,
			      surface->format.bytesPerPixel * 8, pictPal.size());

			// Draw PICT pixels using direct RGB (packRGB) for full color support.
			for (int iy = 0; iy < surface->h; iy++) {
				int sy = y + iy;
				if (sy < clipY1 || sy >= clipY2)
					continue;
				for (int ix = 0; ix < surface->w; ix++) {
					int sx = x + ix;
					if (sx < clipX1 || sx >= clipX2)
						continue;
					byte r, g, b;
					if (isCLUT8) {
						byte idx = *((const byte *)surface->getBasePtr(ix, iy));
						if (pictPal.size() > 0 && idx < (int)pictPal.size()) {
							pictPal.get(idx, r, g, b);
						} else {
							// B&W PICT: 0=white, 1=black
							r = g = b = (idx == 0) ? 255 : 0;
						}
					} else {
						uint32 pixel = surface->getPixel(ix, iy);
						surface->format.colorToRGB(pixel, r, g, b);
					}
					_gfx->setPixel(sx, sy, 0xFF000000 | ((uint32)r << 16) | ((uint32)g << 8) | b);
				}
			}
			_gfx->copyToScreen();
			delete pictStream;
			return true;
		}
	} else {
		warning("drawPict: failed to decode PICT %d", resID);
	}
	delete pictStream;
	return false;
}

void ColonyEngine::terminateGame(bool blowup) {
	Common::Rect savedScreenR = _screenR;
	Common::Rect savedClip = _clip;
	int savedCenterX = _centerX;
	int savedCenterY = _centerY;

	_screenR = Common::Rect(0, 0, _width, _height);
	_clip = _screenR;
	_centerX = _width / 2;
	_centerY = _height / 2;

	_mouseLocked = false;
	_system->lockMouse(false);
	_system->showMouse(true);
	CursorMan.setDefaultArrowCursor(true);
	CursorMan.showMouse(true);

	debugC(1, kColonyDebugUI, "terminateGame(blowup=%d)", blowup);

	_sound->stop();
	_gfx->clear(_gfx->black());
	_gfx->copyToScreen();

	_sound->play(blowup ? Sound::kExplode : Sound::kOuch);

	if (_sound->isPlaying()) {
		bool inverted = false;
		while (_sound->isPlaying() && !shouldQuit()) {
			_gfx->clear(inverted ? _gfx->white() : _gfx->black());
			_gfx->copyToScreen();
			inverted = !inverted;
			_system->delayMillis(50);
		}
	} else {
		for (int i = 0; i < (blowup ? 8 : 4); i++) {
			_gfx->clear((i & 1) ? _gfx->white() : _gfx->black());
			_gfx->copyToScreen();
			_system->delayMillis(50);
		}
	}

	_sound->stop();
	_gfx->clear(_gfx->black());
	_gfx->copyToScreen();

	const char *msg[] = {
		"YOU HAVE BEEN TERMINATED",
		nullptr
	};
	printMessage(msg, true);

	_screenR = savedScreenR;
	_clip = savedClip;
	_centerX = savedCenterX;
	_centerY = savedCenterY;

	_system->quit();
}

int ColonyEngine::countSavedCryos() const {
	int saved = 0;

	for (uint i = 0; i < _patches.size(); i++) {
		if (_patches[i].type == kObjCryo && _patches[i].to.level == 1)
			saved++;
	}

	return saved;
}

void ColonyEngine::takeOff() {
	Common::Rect savedScreenR = _screenR;
	Common::Rect savedClip = _clip;
	int savedCenterX = _centerX;
	int savedCenterY = _centerY;

	_screenR = Common::Rect(0, 0, _width, _height);
	_clip = _screenR;
	_centerX = _width / 2;
	_centerY = _height / 2;

	debugC(1, kColonyDebugUI, "takeOff()");

	_sound->stop();
	_gfx->clear(_gfx->black());
	_gfx->copyToScreen();

	if (getPlatform() == Common::kPlatformMacintosh)
		_sound->play(Sound::kMars);
	else
		_sound->play(Sound::kStars1);

	makeStars(_screenR, 0);
	_sound->stop();

	_gfx->clear(_gfx->black());
	_gfx->copyToScreen();

	_screenR = savedScreenR;
	_clip = savedClip;
	_centerX = savedCenterX;
	_centerY = savedCenterY;
}

void ColonyEngine::gameOver(bool kill) {
	Common::Rect savedScreenR = _screenR;
	Common::Rect savedClip = _clip;
	int savedCenterX = _centerX;
	int savedCenterY = _centerY;

	_screenR = Common::Rect(0, 0, _width, _height);
	_clip = _screenR;
	_centerX = _width / 2;
	_centerY = _height / 2;

	_mouseLocked = false;
	_system->lockMouse(false);
	_system->showMouse(true);
	CursorMan.setDefaultArrowCursor(true);
	CursorMan.showMouse(true);

	const int savedCryos = countSavedCryos();
	int textEntry;

	if (kill)
		textEntry = (savedCryos == 6) ? 256 : (savedCryos > 0 ? 257 : 258);
	else
		textEntry = (savedCryos == 6) ? 259 : (savedCryos > 0 ? 260 : 261);

	debugC(1, kColonyDebugUI, "gameOver(kill=%d, savedCryos=%d, text=%d)", kill, savedCryos, textEntry);

	_sound->stop();
	_gfx->clear(_gfx->black());
	_gfx->copyToScreen();

	if (kill) {
		_sound->play(Sound::kPShot);
		makeStars(_screenR, 0);
		_sound->stop();
		_sound->play(Sound::kExplode);
		for (int i = 0; i < 4; i++) {
			_gfx->clear((i & 1) ? _gfx->black() : _gfx->white());
			_gfx->copyToScreen();
			_system->delayMillis(50);
		}
	} else {
		_sound->play(Sound::kStars4);
		makeStars(_screenR, 0);
		_sound->stop();
	}

	_gfx->clear(_gfx->black());
	_gfx->copyToScreen();
	doText(textEntry, 2);

	_gfx->clear(_gfx->black());
	_gfx->copyToScreen();
	_sound->play(Sound::kStars4);
	makeStars(_screenR, 0);
	_sound->stop();

	_gfx->clear(_gfx->black());
	_gfx->copyToScreen();
	timeSquare("...THE END...", nullptr);

	_gfx->clear(_gfx->black());
	_gfx->copyToScreen();
	_sound->play(Sound::kExplode);
	if (_sound->isPlaying()) {
		while (_sound->isPlaying() && !shouldQuit()) {
			_gfx->clear(_gfx->white());
			_gfx->copyToScreen();
			_system->delayMillis(50);
			_gfx->clear(_gfx->black());
			_gfx->copyToScreen();
			_system->delayMillis(50);
		}
	} else {
		for (int i = 0; i < 4; i++) {
			_gfx->clear((i & 1) ? _gfx->black() : _gfx->white());
			_gfx->copyToScreen();
			_system->delayMillis(50);
		}
	}
	_sound->stop();

	_screenR = savedScreenR;
	_clip = savedClip;
	_centerX = savedCenterX;
	_centerY = savedCenterY;

	_system->quit();
}

} // End of namespace Colony
