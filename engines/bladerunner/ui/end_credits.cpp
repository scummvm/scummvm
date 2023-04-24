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

#include "common/system.h"
#include "common/rect.h"

#include "audio/mixer.h"

#include "bladerunner/bladerunner.h"
#include "bladerunner/game_constants.h"
#include "bladerunner/ambient_sounds.h"
#include "bladerunner/audio_speech.h"
#include "bladerunner/font.h"
#include "bladerunner/game_info.h"
#include "bladerunner/mouse.h"
#include "bladerunner/music.h"
#include "bladerunner/text_resource.h"
#include "bladerunner/time.h"
#include "bladerunner/ui/end_credits.h"

namespace BladeRunner {

EndCredits::EndCredits(BladeRunnerEngine *vm) {
	_vm = vm;
}

EndCredits::~EndCredits() {
}

// Aux method with hardcoded fixes for the credits
// in the official localizations
// ENG (not needed)
// ITA, FRA, ESP, DEU
void EndCredits::creditsCheckAndFix(int &textResourceId, Common::String &textStr) {
	switch (_vm->_language) {
	case Common::IT_ITA:
		switch (textResourceId) {
		case 71: // Grafici Ideatori
			textStr = "Ideatori Grafici";
			break;

		case 211:
			textStr.trim();
			break;

		default:
			break;
		}
		break;

	case Common::DE_DEU:
		switch (textResourceId) {
		case 312:
			textStr.trim();
			break;

		default:
			break;
		}
		break;

	case Common::FR_FRA:
		switch (textResourceId) {
		case 97:
			// fall through
		case 265:
			// fall through
		case 266:
			textStr.trim();
			break;

		default:
			break;
		}
		break;

	default:
		break;
	}

	if (_vm->_language == Common::ES_ESP && _vm->_spanishCreditsCorrection) {
		// Corrections in credited names according to
		// https://www.doblajevideojuegos.es/fichajuego/blade-runner
		// Move 280-283 two lines above to accomodate new addition (Early Q actor)
		switch (textResourceId) {
		case 278:
			textStr = "Voces"; // moved two lines above
			break;

		case 280:
			textStr = "McCoy"; // moved two lines above
			break;

		case 281:
			textStr = "^Carlos Salamanca"; // from "^Luis Casanovas" (also moved two lines above)
			break;

		case 282:
			textStr = "Early Q"; // originally uncredited
			break;

		case 283:
			textStr = Common::U32String("^Tino Mart\xedn", Common::kISO8859_1).encode(Common::kDos850); // originally uncredited
			break;

		case 300:
			textStr = Common::U32String("Piernas Locas Larry", Common::kISO8859_1).encode(Common::kDos850); // from "Crazylegs" (use translated name as elsewhere)
			break;

		case 303:
			textStr = "^Antonio Cobos"; // from "^Antonio Fernández" for Chew
			break;

		case 304:
			textStr = "Crystal"; // from "Cristal"
			break;

		case 311:
			textStr = Common::U32String("^Carmen Gamb\xedn", Common::kISO8859_1).encode(Common::kDos850); // from "^María Palacios" for Lucy
			break;

		case 312:
			textStr = "Bob Bala"; // from "Bollet Bob" (use proper translated name)
			break;

		case 313:
			textStr = Common::U32String("^Enrique Jord\xe1", Common::kISO8859_1).encode(Common::kDos850); // from "^Enrique Jorda" for Bullet Bob (accent change)
			break;

		case 314:
			textStr = "Peruana"; // from "Peru Lady"
			break;

		case 317:
			textStr = Common::U32String("^Beatriz Su\xe1rez Cerrato", Common::kISO8859_1).encode(Common::kDos850); // from "^Beatriz Suarez" for Isabella
			break;

		case 318:
			textStr = "Presentadora"; // from "Newscaster" (use translated name as elsewhere)
			break;

		case 319:
			textStr = "^Montse Herranz"; // from "^Montse Pastor" for Presentadora (Newscaster)
			break;

		case 321:
			textStr = Common::U32String("^Beatriz Su\xe1rez Cerrato", Common::kISO8859_1).encode(Common::kDos850); // from "^Beatriz Cerrato" for Contestador (Answering Machine)
			break;

		default:
			break;
		}
	}
}

void EndCredits::show() {
	_vm->_mouse->disable();
	_vm->_mixer->stopAll();
	_vm->_ambientSounds->removeAllNonLoopingSounds(true);
	_vm->_ambientSounds->removeAllLoopingSounds(4u);
	_vm->_audioSpeech->stopSpeech();

	_vm->_music->play(_vm->_gameInfo->getMusicTrack(kMusicCredits), 100, 0, 2, -1, kMusicLoopPlayOnce, 3);

	Font *fontBig = Font::load(_vm, "TAHOMA24.FON", 1, true);
	Font *fontSmall = Font::load(_vm, "TAHOMA18.FON", 1, true);

	TextResource *textResource = new TextResource(_vm);
	textResource->open("ENDCRED");

	int textCount = textResource->getCount();

	int *textYPositions = new int[textCount]();
	int *textXPositions = new int[textCount]();

	int y = 480 - fontBig->getFontHeight();
	bool small = false;
	int textStrWidth = 0;
	const int bigToSmallTextYPosDiff = ((fontBig->getFontHeight() - fontSmall->getFontHeight()) / 2);
	const int smallestMarginXToCreditName = 6; // px

	for (int i = 0; i < textCount; ++i) {
		Common::String s = textResource->getText(i);
		creditsCheckAndFix(i, s);

		if (s.hasPrefix("^")) {
			if (!small) {
				y += fontBig->getFontHeight();
			}
			small = false;
			textYPositions[i] = y;
			textXPositions[i] = 280;
		} else {
			if (small) {
				y += fontSmall->getFontHeight();
			} else {
				y += fontBig->getFontHeight();
			}
			small = true;
			textYPositions[i] = y + bigToSmallTextYPosDiff;
			if (_vm->_language == Common::ES_ESP
			    && _vm->_spanishCreditsCorrection
			    && i == 277) {
				y +=  2 * fontSmall->getFontHeight();
			}
			textStrWidth = fontSmall->getStringWidth(s);
			textXPositions[i] = 270 - textStrWidth;
			//
			// Check here if horizontal alignment of this credit "title"
			// may cause clipping off the right edge of the screen.
			// Note, that we don't do the same check for a credit "name"
			// clipping off the left edge of the screen, as this does not happen
			// with the text resources for the credits in the official releases.
			// For fan made credits, the new text resources can be designed
			// with custom line wrapping and line spacing
			// so as to avoid any clipping, so this is no issue there.
			if (textXPositions[i] < 0) {
				textXPositions[i] = 0;
				if (textStrWidth > 280 - smallestMarginXToCreditName
				    && (i + 1 < textResource->getCount())) {
					Common::String sNext = textResource->getText(i + 1);
					if (sNext.hasPrefix("^")) {
						// If, for this case, the next string is a credit "name", ie.
						// aligned starting from the center (or near the center anyway),
						// then insert an extra line to avoid overlap with the title
						y +=  fontSmall->getFontHeight();
					}
				}
			}
		}
	}

	_vm->_vqaIsPlaying = true;
	_vm->_vqaStopIsRequested = false;

	double position = 0.0;
	uint32 timeLast = _vm->_time->currentSystem();

	Font *font;
	int height;

	while (!_vm->_vqaStopIsRequested && !_vm->shouldQuit()) {
		if (position >= textYPositions[textCount - 1]) {
			break;
		}

		//soundSystem::tick(SoundSystem);
		_vm->handleEvents();

		if (!_vm->_windowIsActive) {
			timeLast = _vm->_time->currentSystem();
			continue;
		}

		uint32 timeNow = _vm->_time->currentSystem();
		position += (double)(timeNow - timeLast) * 0.05f; // unsigned difference is intentional
		timeLast = timeNow;

		_vm->_surfaceFront.fillRect(Common::Rect(BladeRunnerEngine::kOriginalGameWidth, BladeRunnerEngine::kOriginalGameHeight), 0);

		for (int i = 0; i < textCount; ++i) {
			Common::String s = textResource->getText(i);
			creditsCheckAndFix(i, s);

			if (s.hasPrefix("^")) {
				font = fontBig;
				height = fontBig->getFontHeight();
				s.deleteChar(0);
			} else {
				font = fontSmall;
				height = fontSmall->getFontHeight();
			}

			y = textYPositions[i] - (int)position;

			if (y < 452 && y + height > fontBig->getFontHeight()) {
				font->drawString(&_vm->_surfaceFront, s, textXPositions[i], y, _vm->_surfaceFront.w, 0);
			}
		}

		_vm->_surfaceFront.fillRect(Common::Rect(0, 0, BladeRunnerEngine::kOriginalGameWidth, fontBig->getFontHeight()), 0);
		_vm->_surfaceFront.fillRect(Common::Rect(0, BladeRunnerEngine::kOriginalGameHeight - fontBig->getFontHeight(), BladeRunnerEngine::kOriginalGameWidth, BladeRunnerEngine::kOriginalGameHeight), 0);

		_vm->blitToScreen(_vm->_surfaceFront);
	}

	_vm->_vqaIsPlaying = false;
	_vm->_vqaStopIsRequested = false;

	delete[] textYPositions;
	delete[] textXPositions;
	delete textResource;

	delete fontSmall;
	delete fontBig;

	_vm->_music->stop(0u);
	_vm->_mouse->enable();
}

} // End of namespace BladeRunner
