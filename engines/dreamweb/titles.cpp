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

#include "dreamweb/sound.h"
#include "dreamweb/dreamweb.h"
#include "engines/util.h"
#include "common/text-to-speech.h"
#include "common/config-manager.h"

namespace DreamWeb {

namespace {
void initTitlesGfx() {
	Graphics::ModeWithFormatList modes = {
#ifdef USE_HIGHRES
		// First try for a 640x480 mode
		Graphics::ModeWithFormat(640, 480, Graphics::PixelFormat::createFormatCLUT8()),
#endif
		// System doesn't support it, so fall back on 320x240 mode
		Graphics::ModeWithFormat(320, 240, Graphics::PixelFormat::createFormatCLUT8()),
	};

	initGraphicsAny(modes);
}
}

void DreamWebEngine::endGame() {
	loadTempText("T83");
	monkSpeaking();
	if (_quitRequested)
		return;
	gettingShot();
	getRidOfTempText();
	_sound->volumeChange(7, 1);
	hangOn(200);
}

void DreamWebEngine::monkSpeaking() {
	_roomsSample = 35;
	_sound->loadRoomsSample(_roomsSample);
	GraphicsFile graphics;
	loadGraphicsFile(graphics, "G15");
	clearWork();
	showFrame(graphics, 160, 72, 0, 128);	// show monk
	workToScreen();
	_sound->volumeSet(7);
	_sound->volumeChange(hasSpeech() ? 5 : 0, -1);
	_sound->playChannel0(12, 255);
	fadeScreenUps();
	hangOn(300);

	// TODO: Subtitles+speech mode
	if (hasSpeech()) {
		for (int i = 40; i < 48; i++) {
			_speechLoaded = _sound->loadSpeech('T', 83, 'T', i);

			_sound->playChannel1(62);

			do {
				waitForVSync();
				if (_quitRequested)
					return;
			} while (_sound->isChannel1Playing());
		}
	} else {
		for (int i = 40; i <= 44; i++) {
			uint8 printResult = 0;
			const uint8 *string = getTextInFile1(i);

			do {
				uint16 y = 140;
				printResult = printDirect(&string, 36, &y, 239, 239 & 1);
				workToScreen();
				clearWork();
				showFrame(graphics, 160, 72, 0, 128);	// show monk
				hangOnP(240);
				if (_quitRequested)
					return;
			} while (printResult != 0);
		}
	}

	_sound->volumeChange(7, 1);
	fadeScreenDowns();
	hangOn(300);
	graphics.clear();
}

void DreamWebEngine::gettingShot() {
	_newLocation = 55;
	clearPalette();
	loadIntroRoom();
	fadeScreenUps();
	_sound->volumeChange(0, -1);
	runEndSeq();
	clearBeforeLoad();
}

void DreamWebEngine::bibleQuote() {
	const char *enStory = "And I heard a great voice out of the temple saying to the seven angels, "
						"Go your ways and pour out the vails of the wrath of god upon the earth. "
						"Book of revelation Chapter 16 verse 1.";
	const char *frStory = "Puis j'entendis une voix forte qui venait du temple et disait aux sept anges: "
						"Allez verser sur la terre les sept coupes de la col\xC3\xA8re de Dieu. "
						"L'Apocalypse, chapitre 16, verset 1";
	const char *esStory = "O\xC3\xAD una gran voz que dec\xC3\xAD""a"" desde el templo a los siete \xC3\xA1ngeles: "
						"Id y derramad sobre la tierra las siete copas de la ira de Dios. "
						"Apocalipsis, cap\xC3\xADtulo 16, vers\xC3\xAD""culo"" primero.";
	const char *deStory = "Dann h\xC3\xB6rte ich, wie eine laute Stimme aus dem Tempel den sieben Engeln zurief: "
						"Geht und gie\xC3\x9Ft die sieben Schalen mit dem Zorn Gottes \xC3\xBC""ber"" die Erde. "
						"Offenbarung des Johannes. Kapitel 16 Vers 1";
	const char *itStory = "Udii poi una gran voce dal tempio che diceva ai sette angeli: "
						"Andate e versate sulla terra le sette coppe dell'ira di Dio. "
						"Dal libro dell'Apocalisse, capitolo uno, primo versetto";
	const char *ruStory = "\xD1\x83\xD1\x81\xD0\xBB\xD1\x8B\xD1\x88\xD0\xB0\xD0\xBB \xD1\x8F \xD0\xB8\xD0\xB7 \xD1\x85\xD1\x80\xD0\xB0\xD0\xBC\xD0\xB0 \xD1\x80\xD1\x8F\xD1\x89\xD0\xB8\xD0\xB9 \xD1\x81\xD0\xB5\xD0\xBC\xD0\xB8 \xD0\x90\xD0\xBD\xD0\xB3\xD0\xB5\xD0\xBB\xD0\xB0\xD0\xBC\x3A: "
						"\xD0\xB8\xD0\xB4\xD0\xB8\xD1\x82\xD0\xB5 \xD0\xB8 \xD0\xB2\xD1\x8B\xD0\xBB\xD0\xB5\xD0\xB9\xD1\x82\xD0\xB5 \xD1\x81\xD0\xB5\xD0\xBC\xD1\x8C \xD1\x87\xD0\xB0\xD1\x88 \xD0\xB3\xD0\xBD\xD0\xB5\xD0\xB2\xD0\xB0 \xD0\x91\xD0\xBE\xD0\xB6\xD0\xB8\xD1\x8F \xD0\xBD\xD0\xB0 \xD0\xB7\xD0\xB5\xD0\xBC\xD0\xBB\xD1\x8E\x2E. "
						"\xD0\x9E\xD0\xA2\xD0\x9A\xD0\xA0\xD0\x9E\xD0\x92\xD0\x95\xD0\x9D\xD0\x98\xD0\x95 \xD0\x98\xD0\x9E\xD0\x90\xD0\x9D\xD0\x9D\xD0\x90 \xD0\x91\xD0\x9E\xD0\x93\xD0\x9E\xD0\xA1\xD0\x9B\xD0\x9E\xD0\x92\xD0\x90 16:1";
	const char *csStory = "Tu jsem usly\xC5\xA1""el"" mocn\xC3\xBD hlas ze svatyn\xC4\x9B, kter\xC3\xBD t\xC4\x9Bm sedmi and\xC4\x9Bl\xC5\xAFm \xC5\x99\xC3\xADkal: "
						"Jd\xC4\x9Bte a vylejte t\xC4\x9B""ch"" sedm misek Bo\xC5\xBE\xC3\xADho hn\xC4\x9Bvu na zem. "
						"Zjeven\xC3\xAD 16 ver\xC5\xA1 1";


	const char *theStory;
	switch(getLanguage()) {
	case Common::ES_ESP:
		theStory = esStory;
		break;
	case Common::FR_FRA:
		theStory = frStory;
		break;
	case Common::IT_ITA:
		theStory = itStory;
		break;
	case Common::DE_DEU:
		theStory = deStory;
		break;
	case Common::RU_RUS:
		theStory = ruStory;
		break;
	case Common::CS_CZE:
		theStory = csStory;
		break;
	default:
		theStory = enStory;
		break;
	}

	initTitlesGfx();

	showPCX("I00");
	fadeScreenUps();

	hangOne(80);
	if (_lastHardKey == Common::KEYCODE_ESCAPE) {
		_lastHardKey = Common::KEYCODE_INVALID;
		return; // "biblequotearly"
	}

	if (_ttsMan != nullptr && ConfMan.getBool("tts_enabled_objects")) {
		_ttsMan->say(theStory);
		while (_ttsMan->isSpeaking() && _lastHardKey != Common::KEYCODE_ESCAPE)
			hangOne(1);
		_ttsMan->stop();

	} else
		hangOne(560);

	if (_lastHardKey == Common::KEYCODE_ESCAPE) {
		_lastHardKey = Common::KEYCODE_INVALID;
		return; // "biblequotearly"
	}

	fadeScreenDowns();

	hangOne(200);
	if (_lastHardKey == Common::KEYCODE_ESCAPE) {
		_lastHardKey = Common::KEYCODE_INVALID;
		return; // "biblequotearly"
	}

	_sound->cancelCh0();

	_lastHardKey = Common::KEYCODE_INVALID;
}

void DreamWebEngine::hangOne(uint16 delay) {
	do {
		waitForVSync();
		if (_lastHardKey == Common::KEYCODE_ESCAPE)
			return; // "hangonearly"
	} while	(--delay);
}

void DreamWebEngine::intro() {
	loadTempText("T82");
	loadPalFromIFF();
	setMode();
	_newLocation = 50;
	clearPalette();
	loadIntroRoom();
	_sound->volumeSet(7);
	_sound->volumeChange(hasSpeech() ? 4 : 0, -1);
	_sound->playChannel0(12, 255);
	fadeScreenUps();
	runIntroSeq();

	if (_lastHardKey == Common::KEYCODE_ESCAPE) {
		_lastHardKey = Common::KEYCODE_INVALID;
		return; // "introearly"
	}

	clearBeforeLoad();
	_newLocation = 52;
	loadIntroRoom();
	runIntroSeq();

	if (_lastHardKey == Common::KEYCODE_ESCAPE) {
		_lastHardKey = Common::KEYCODE_INVALID;
		return; // "introearly"
	}

	clearBeforeLoad();
	_newLocation = 53;
	loadIntroRoom();
	runIntroSeq();

	if (_lastHardKey == Common::KEYCODE_ESCAPE) {
		_lastHardKey = Common::KEYCODE_INVALID;
		return; // "introearly"
	}

	clearBeforeLoad();
	allPalette();
	_newLocation = 54;
	loadIntroRoom();
	runIntroSeq();

	if (_lastHardKey == Common::KEYCODE_ESCAPE) {
		_lastHardKey = Common::KEYCODE_INVALID;
		return; // "introearly"
	}

	getRidOfTempText();
	clearBeforeLoad();

	_lastHardKey = Common::KEYCODE_INVALID;
}

void DreamWebEngine::runIntroSeq() {
	_getBack = 0;

	do {
		waitForVSync();

		if (_lastHardKey == Common::KEYCODE_ESCAPE)
			break;

		spriteUpdate();
		waitForVSync();

		if (_lastHardKey == Common::KEYCODE_ESCAPE)
			break;

		delEverything();
		printSprites();
		reelsOnScreen();
		afterIntroRoom();
		useTimedText();
		waitForVSync();

		if (_lastHardKey == Common::KEYCODE_ESCAPE)
			break;

		dumpMap();
		dumpTimedText();
		waitForVSync();

		if (_lastHardKey == Common::KEYCODE_ESCAPE)
			break;

	} while (_getBack != 1);


	if (_lastHardKey == Common::KEYCODE_ESCAPE) {
		getRidOfTempText();
		clearBeforeLoad();
	}

	// These were not called in this program arc
	// in the original code.. Bug?
	//getRidOfTempText();
	//clearBeforeLoad();
}

void DreamWebEngine::runEndSeq() {
	atmospheres();
	_getBack = 0;

	do {
		waitForVSync();
		spriteUpdate();
		waitForVSync();
		delEverything();
		printSprites();
		reelsOnScreen();
		afterIntroRoom();
		useTimedText();
		waitForVSync();
		dumpMap();
		dumpTimedText();
		waitForVSync();
	} while (_getBack != 1 && !_quitRequested);
}

void DreamWebEngine::loadIntroRoom() {
	_introCount = 0;
	_vars._location = 255;
	loadRoom();
	_mapOffsetX = 72;
	_mapOffsetY = 16;
	clearSprites();
	_vars._throughDoor = 0;
	_currentKey = 0;
	_mainMode = 0;
	clearWork();
	_vars._newObs = 1;
	drawFloor();
	reelsOnScreen();
	spriteUpdate();
	printSprites();
	workToScreen();
}

void DreamWebEngine::set16ColPalette() {
}

void DreamWebEngine::realCredits() {
	_roomsSample = 33;
	_sound->loadRoomsSample(_roomsSample);
	_sound->volumeSet(0);

	initTitlesGfx();
	hangOn(35);

	showPCX("I01");
	_sound->playChannel0(12, 0);

	hangOne(2);

	if (_lastHardKey == Common::KEYCODE_ESCAPE) {
		_lastHardKey = Common::KEYCODE_INVALID;
		return; // "realcreditsearly"
	}

	allPalette();
	hangOne(80);

	if (_lastHardKey == Common::KEYCODE_ESCAPE) {
		_lastHardKey = Common::KEYCODE_INVALID;
		return; // "realcreditsearly"
	}

	fadeScreenDowns();
	hangOne(256);

	if (_lastHardKey == Common::KEYCODE_ESCAPE) {
		_lastHardKey = Common::KEYCODE_INVALID;
		return; // "realcreditsearly"
	}

	showPCX("I02");
	_sound->playChannel0(12, 0);
	hangOne(2);

	if (_lastHardKey == Common::KEYCODE_ESCAPE) {
		_lastHardKey = Common::KEYCODE_INVALID;
		return; // "realcreditsearly"
	}

	allPalette();
	hangOne(80);

	if (_lastHardKey == Common::KEYCODE_ESCAPE) {
		_lastHardKey = Common::KEYCODE_INVALID;
		return; // "realcreditsearly"
	}

	fadeScreenDowns();
	hangOne(256);

	if (_lastHardKey == Common::KEYCODE_ESCAPE) {
		_lastHardKey = Common::KEYCODE_INVALID;
		return; // "realcreditsearly"
	}

	showPCX("I03");
	_sound->playChannel0(12, 0);
	hangOne(2);

	if (_lastHardKey == Common::KEYCODE_ESCAPE) {
		_lastHardKey = Common::KEYCODE_INVALID;
		return; // "realcreditsearly"
	}

	allPalette();
	hangOne(80);

	if (_lastHardKey == Common::KEYCODE_ESCAPE) {
		_lastHardKey = Common::KEYCODE_INVALID;
		return; // "realcreditsearly"
	}

	fadeScreenDowns();
	hangOne(256);

	if (_lastHardKey == Common::KEYCODE_ESCAPE) {
		_lastHardKey = Common::KEYCODE_INVALID;
		return; // "realcreditsearly"
	}

	showPCX("I04");
	_sound->playChannel0(12, 0);
	hangOne(2);

	if (_lastHardKey == Common::KEYCODE_ESCAPE) {
		_lastHardKey = Common::KEYCODE_INVALID;
		return; // "realcreditsearly"
	}

	allPalette();
	hangOne(80);

	if (_lastHardKey == Common::KEYCODE_ESCAPE) {
		_lastHardKey = Common::KEYCODE_INVALID;
		return; // "realcreditsearly"
	}

	fadeScreenDowns();
	hangOne(256);

	if (_lastHardKey == Common::KEYCODE_ESCAPE) {
		_lastHardKey = Common::KEYCODE_INVALID;
		return; // "realcreditsearly"
	}

	showPCX("I05");
	_sound->playChannel0(12, 0);
	hangOne(2);

	if (_lastHardKey == Common::KEYCODE_ESCAPE) {
		_lastHardKey = Common::KEYCODE_INVALID;
		return; // "realcreditsearly"
	}

	allPalette();
	hangOne(80);

	if (_lastHardKey == Common::KEYCODE_ESCAPE) {
		_lastHardKey = Common::KEYCODE_INVALID;
		return; // "realcreditsearly"
	}

	fadeScreenDowns();
	hangOne(256);

	if (_lastHardKey == Common::KEYCODE_ESCAPE) {
		_lastHardKey = Common::KEYCODE_INVALID;
		return; // "realcreditsearly"
	}

	showPCX("I06");
	fadeScreenUps();
	hangOne(60);

	if (_lastHardKey == Common::KEYCODE_ESCAPE) {
		_lastHardKey = Common::KEYCODE_INVALID;
		return; // "realcreditsearly"
	}

	_sound->playChannel0(13, 0);
	hangOne(350);

	if (_lastHardKey == Common::KEYCODE_ESCAPE) {
		_lastHardKey = Common::KEYCODE_INVALID;
		return; // "realcreditsearly"
	}

	fadeScreenDowns();
	hangOne(256);

	_lastHardKey = Common::KEYCODE_INVALID;
}

} // End of namespace DreamWeb
