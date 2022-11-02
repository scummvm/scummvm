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

#include "bladerunner/subtitles.h"

#include "bladerunner/font.h"
#include "bladerunner/text_resource.h"
#include "bladerunner/audio_speech.h"
#include "bladerunner/game_constants.h"
#include "bladerunner/time.h"

#include "common/debug.h"

#include "graphics/font.h"
#include "graphics/fonts/ttf.h"

namespace BladeRunner {

/*
 * Optional support for subtitles
 *
 * CHECK what happens in VQA where the audio plays separately (are the finales such VQAs ?)
 * TODO? Use another escape sequence to progressively display text in a line (like in SCUMM games) <-- this could be very useful with very long lines
 *			- might also need an extra manual time or ticks parameter to determine when during the display of the first segment we should switch to the second.
 * TODO? A more advanced subtitles system:
 *          TODO: subtitles could be independent from sound playing (but probably should disappear when switching between UI screens)
 *          TODO?: Support for queuing subtitles when more than one subtitle should play for a spoken dialogue (due to a very long quote)
 *          TODO?: Predefine a minimum time for a subtitle to appear, before it is interrupted by the next one. (might need queuing)
 *          TODO?: If the subtitle is the last one then extend its duration to another predefined delay.
 *
 * DONE Removed support for internal font TAHOMA18 - this particular font is corrupted!
 * DONE Create and Support proper external FON for subtitles.
 * DONE split at new line character (priority over auto-split)
 * DONE auto-split a long line into two
 * DONE support the basic 2 line subtitles
 * DONE support a third line for subtitles (some quotes are too long for 2 lines). Are there quotes that are too long for 3 lines?
 * DONE handle missing subtitle files! Gracefully don't show subtitles for VQAs or in-game dialogue if the required respective files are missing!
 * DONE add subtitle files for the rest of VQAs that have spoken dialogue
 * DONE A system to auto-split a dialogue after some max total width of character glyphs per line.
 * DONE - OK - CHECK What happens with skipped dialogue (enter / skip dialogue key pressed)
 * DONE - OK - CHECK what happens in VQA when no corresponding TRE subs file?
 */

const char *Subtitles::SUBTITLES_FONT_FILENAME_EXTERNAL = "SUBTLS_E.FON";

const char *Subtitles::SUBTITLES_VERSION_TRENAME        = "SBTLVERS"; // addon resource file for Subtitles version info - can only be SBTLVERS.TRE
const char *Subtitles::EXTRA_TRENAME                    = "EXTRA";

const Color256 Subtitles::kTextColors[] = {
	{ 0, 0, 0 },
	{ 16, 8, 8 },
	{ 32, 24, 8 },
	{ 56, 32, 16 },
	{ 72, 48, 16 },
	{ 88, 56, 24 },
	{ 104, 72, 32 },
	{ 128, 80, 40 },
	{ 136, 96, 48 },
	{ 152, 112, 56 },
	{ 168, 128, 72 },
	{ 184, 144, 88 },
	{ 200, 160, 96 },
	{ 216, 184, 112 },
	{ 232, 200, 128 },
	{ 240, 224, 144 }
};

/*
 * All entries need to have the language code appended (after a '_').
 * And all entries should get the suffix extension ".TRx"; the last letter in extension "TR*" should also be the language code
 * If/When adding new Text Resources here --> Update kMaxTextResourceEntries and also update method getIdxForSubsTreName()
 */
const char *Subtitles::SUBTITLES_FILENAME_PREFIXES[kMaxTextResourceEntries] = {
	"INGQUO",           // 0 // (in-game subtitles, not VQA subtitles)
	"WSTLGO",           // 1 // all game (language) versions have the English ('E') version of WSTLGO
	"BRLOGO",           // 2 // all game (language) versions have the English ('E') version of BRLOGO
	"INTRO",            // 3
	"MW_A",             // 4
	"MW_B01",           // 5
	"MW_B02",           // 6
	"MW_B03",           // 7
	"MW_B04",           // 8
	"MW_B05",           // 9
	"INTRGT",           // 10
	"MW_C01",           // 11
	"MW_C02",           // 12
	"MW_C03",           // 13
	"MW_D",             // 14
	"END04A",           // 15
	"END04B",           // 16
	"END04C",           // 17
	"END06",            // 18
	"END01A",           // 19
	"END01B",           // 20
	"END01C",           // 21
	"END01D",           // 22
	"END01E",           // 23
	"END01F",           // 24
	"END03",            // 25
	"TB_FLY"            // 26
};

/**
 * Subtitles Constructor
 */
Subtitles::Subtitles(BladeRunnerEngine *vm) {
	_vm = vm;
	_isSystemActive = false;
	for (int i = 0; i < kMaxTextResourceEntries; ++i) {
		_vqaSubsTextResourceEntries[i] = nullptr;
		_gameSubsResourceEntriesFound[i] = false;
	}
	_font = nullptr;
	_useUTF8 = false;
	_useHDC = false;
	_subtitlesDataActive.resize(kNumOfSubtitleRoles);
	_loadAvgStr = "";
	_excTitlStr = "";
	_goVib = "";
	_xcStringIndex = 0;
	_xcTimeLast    = 0;

	for (int i = 0; i < kxcStringCount; ++i) {
		_xcStrings[i] = "";
	}

	for (int i = 0; i < kxcLineCount; ++i) {
		_xcLineTexts[i]    = "";
		_xcLineTimeouts[i] = 0;
		_xcLineOffsets[i]  = 0;
	}
	reset();
}

/**
 * Subtitles Destructor
 */
Subtitles::~Subtitles() {
	reset();
	_subtitlesDataActive.clear();
	clearQueue();
	_subtitlesEXC.clear();
}

//
// Init is kept separated from constructor to allow not loading up resources if subtitles system is disabled
//
void Subtitles::init(void) {
	// Loading subtitles versioning info if available
	TextResource versionTxtResource(_vm);
	if ( versionTxtResource.open(SUBTITLES_VERSION_TRENAME, false)) {
		_subtitlesInfo.credits       = versionTxtResource.getText((uint32)0);
		_subtitlesInfo.versionStr    = versionTxtResource.getText((uint32)1);
		_subtitlesInfo.dateOfCompile = versionTxtResource.getText((uint32)2);
		_subtitlesInfo.languageMode  = versionTxtResource.getText((uint32)3);
		Common::String fontType      = versionTxtResource.getText((uint32)4);
		_subtitlesInfo.fontName      = versionTxtResource.getText((uint32)5);
		Common::String license       = versionTxtResource.getText((uint32)6);
		Common::String licenseLink   = versionTxtResource.getText((uint32)7);

		if (fontType.equalsIgnoreCase("ttf")) {
			_subtitlesInfo.fontType = Subtitles::kSubtitlesFontTypeTTF;
		} else {
			_subtitlesInfo.fontType = Subtitles::kSubtitlesFontTypeInternal;
		}

		if (_subtitlesInfo.fontName.empty()) {
			_subtitlesInfo.fontName = SUBTITLES_FONT_FILENAME_EXTERNAL;
		}

		debug("Subtitles version info: v%s (%s) %s",
		       _subtitlesInfo.versionStr.c_str(),
		       _subtitlesInfo.dateOfCompile.c_str(),
		       _subtitlesInfo.languageMode.c_str());

	} else {
		debug("Subtitles version info: N/A");
	}

	TextResource extraTxtResource(_vm);
	if ( extraTxtResource.open(EXTRA_TRENAME, false) && extraTxtResource.getCount() > 0) {
		_subtitlesEXC.resize(extraTxtResource.getCount());
		for (uint8 i = 0; i < extraTxtResource.getCount(); ++i) {
			_subtitlesEXC[i] = extraTxtResource.getText((uint32)i);
		}
		_loadAvgStr = "";
		_excTitlStr = "";
		_goVib = "";
		if (extraTxtResource.getCount() == kxcStringCount + 1) {
			_loadAvgStr.insertChar(_subtitlesEXC[3][13], 0);
			_loadAvgStr.insertChar(_subtitlesEXC[3][12], 0);
			_loadAvgStr.insertChar(_subtitlesEXC[0][7],  0);
			_loadAvgStr.insertChar(_subtitlesEXC[14][1], 0);
			_loadAvgStr.insertChar(_subtitlesEXC[10][7], 0);
			_loadAvgStr.insertChar(_subtitlesEXC[1][8],  0);
			_loadAvgStr.insertChar(_subtitlesEXC[0][5],  0);
			_loadAvgStr.insertChar(_subtitlesEXC[2][12], 0);
			_loadAvgStr.insertChar(_subtitlesEXC[1][2],  0);
			_loadAvgStr.insertChar(_subtitlesEXC[7][5],  0);
			_loadAvgStr.insertChar(_subtitlesEXC[2][1],  0);
			_loadAvgStr.insertChar(_subtitlesEXC[2][7],  0);
			_loadAvgStr.toUppercase();
			_excTitlStr.insertChar(_subtitlesEXC[14][0], 0);
			_excTitlStr.insertChar(_subtitlesEXC[0][2],  0);
			_excTitlStr.insertChar(_subtitlesEXC[1][2],  0);
			_excTitlStr.insertChar(_subtitlesEXC[3][8],  0);
			_excTitlStr.insertChar(_subtitlesEXC[2][7],  0);
			_excTitlStr.insertChar(_subtitlesEXC[7][3],  0);
			_excTitlStr.insertChar(_subtitlesEXC[2][4],  0);
			_excTitlStr.insertChar(_subtitlesEXC[0][8],  0);
			_excTitlStr.insertChar(_subtitlesEXC[1][8],  0);
			_excTitlStr.insertChar(_subtitlesEXC[3][11], 0);
			_excTitlStr.insertChar(_subtitlesEXC[2][6],  0);
			_excTitlStr.insertChar(_subtitlesEXC[8][7],  0);
			_excTitlStr.insertChar(_subtitlesEXC[6][8],  0);
			_excTitlStr.insertChar(_subtitlesEXC[4][3],  0);
			_excTitlStr.insertChar(_subtitlesEXC[5][2],  0);
			_excTitlStr.insertChar(_subtitlesEXC[7][6],  0);
			_excTitlStr.insertChar(_subtitlesEXC[0][12], 0);
			_excTitlStr.insertChar(_subtitlesEXC[10][9], 0);
			_excTitlStr.insertChar(_subtitlesEXC[1][5],  0);
			_excTitlStr.insertChar(_subtitlesEXC[8][10], 0);
			_excTitlStr.insertChar(_subtitlesEXC[6][2],  0);
			_excTitlStr.insertChar(_subtitlesEXC[5][3],  0);
			_excTitlStr.insertChar(_subtitlesEXC[11][1], 0);
			_excTitlStr.insertChar(_subtitlesEXC[4][4],  0);
			_goVib = extraTxtResource.getText((uint32)9);
			_goVib.toUppercase();
			_useHDC = true;
		}
	}

	//
	// Initializing/Loading Subtitles Fonts
	if (_subtitlesInfo.fontType == Subtitles::kSubtitlesFontTypeInternal) {
		// Use TAHOMA18.FON (is corrupted in places)
		// 10PT or TAHOMA24 or KIA6PT  have all caps glyphs (and also are too big or too small) so they are not appropriate.
		_font = Font::load(_vm, _subtitlesInfo.fontName, -1, true);
		_useUTF8 = false;
	} else if (_subtitlesInfo.fontType == Subtitles::kSubtitlesFontTypeTTF) {
#if defined(USE_FREETYPE2)
		Common::ScopedPtr<Common::SeekableReadStream> stream(_vm->getResourceStream(_subtitlesInfo.fontName));
		_font = Graphics::loadTTFFont(*stream, 18);
		_useUTF8 = true;
#else
		warning("Subtitles require a TTF font but this ScummVM build doesn't support it.");
		return;
#endif
	}

	if (_font) {
		debug("Subtitles font '%s' was loaded successfully.", _subtitlesInfo.fontName.c_str());
	} else {
		warning("Subtitles font '%s' could not be loaded.", _subtitlesInfo.fontName.c_str());
		return;
	}
	//Done - Initializing/Loading Subtitles Fonts
	//

	//
	// Loading text resources
	for (int i = 0; i < kMaxTextResourceEntries; ++i) {
		_vqaSubsTextResourceEntries[i] = new TextResource(_vm);
		Common::String tmpConstructedFileName = "";
		bool localizedResource = true;
		if (!strcmp(SUBTITLES_FILENAME_PREFIXES[i], "WSTLGO") || !strcmp(SUBTITLES_FILENAME_PREFIXES[i], "BRLOGO")) {
			tmpConstructedFileName = Common::String(SUBTITLES_FILENAME_PREFIXES[i]) + "_E"; // Only English versions of these exist
			localizedResource = false;
		}
		else {
			tmpConstructedFileName = Common::String(SUBTITLES_FILENAME_PREFIXES[i]) + "_" + _vm->_languageCode;
		}

		if (_vqaSubsTextResourceEntries[i]->open(tmpConstructedFileName, localizedResource)) {
			_gameSubsResourceEntriesFound[i] = true;
		}
	}
	// Done - Loading text resources
	//

	_isSystemActive = true;
}

Subtitles::SubtitlesInfo Subtitles::getSubtitlesInfo() const {
	return _subtitlesInfo;
}

void Subtitles::xcReload() {
	_xcStringIndex = 0;
	for (int i = 0; i < kxcStringCount; ++i) {
		_xcStrings[i] = _subtitlesEXC[i];
	}

	for (int i = 0; i < kxcStringCount; ++i) {
		int j = _vm->_rnd.getRandomNumberRng(i, kxcStringCount - 1);
		SWAP<Common::String>(_xcStrings[i], _xcStrings[j]);
	}

	for (int i = 0; i < kxcLineCount; ++i) {
		_xcLineTexts[i] = "";
		_xcLineTimeouts[i] = _vm->_rnd.getRandomNumberRng(0, 63);
		_xcLineOffsets[i] = 0;
	}

	_xcTimeLast = _vm->_time->currentSystem();
}

/**
 * Returns the index of the specified Text Resource filename in the SUBTITLES_FILENAME_PREFIXES table
 */
int Subtitles::getIdxForSubsTreName(const Common::String &treName) const {
	Common::String tmpConstructedFileName = "";
	for (int i = 0; i < kMaxTextResourceEntries; ++i) {
		if (!strcmp(SUBTITLES_FILENAME_PREFIXES[i], "WSTLGO") || !strcmp(SUBTITLES_FILENAME_PREFIXES[i], "BRLOGO")) {
			tmpConstructedFileName = Common::String(SUBTITLES_FILENAME_PREFIXES[i]) + "_E"; // Only English versions of these exist
		} else {
			tmpConstructedFileName = Common::String(SUBTITLES_FILENAME_PREFIXES[i]) + "_" + _vm->_languageCode;
		}
		if (tmpConstructedFileName == treName) {
			return i;
		}
	}
	// error case
	return -1;
}

/**
 * Get the active subtitle text by searching with actor ID and speech ID
 * Use this method for in-game dialogue - Not dialogue during a VQA cutscene
 */
void Subtitles::loadInGameSubsText(int actorId, int speech_id)  {
	if (!_isSystemActive) {
		return;
	}

	if (!_gameSubsResourceEntriesFound[0]) {
		_subtitlesDataActive[kSubtitlesPrimary].currentText.clear();
		_subtitlesDataActive[kSubtitlesPrimary].currentText32.clear();
		_subtitlesDataActive[kSubtitlesPrimary].prevText.clear();
		_subtitlesDataActive[kSubtitlesPrimary].prevText32.clear();
		return;
	}

	bool specialSubtitleCase = false;
	if (_vm->_language == Common::DE_DEU) {
		// Special cases for some localizations
		if (actorId == kActorVoiceOver && speech_id == 1850) {
			mergeSubtitleQuotes(actorId, 1850, 1860);
			specialSubtitleCase = true;
		}
	}

	if (!specialSubtitleCase) {
		// Search in the first TextResource of the _vqaSubsTextResourceEntries table,
		// which is the TextResource for in-game dialogue (i.e. not VQA dialogue)
		int32 id = 10000 * actorId + speech_id;
		const char *text = _vqaSubsTextResourceEntries[0]->getText((uint32)id);
		if (_useUTF8) {
			_subtitlesDataActive[kSubtitlesPrimary].currentText32 = Common::convertUtf8ToUtf32(text);
		} else {
			_subtitlesDataActive[kSubtitlesPrimary].currentText = text;
		}
	}
}

/**
 * Use this method for dialogue during VQA cutscenes
 */
void Subtitles::loadOuttakeSubsText(const Common::String &outtakesName, int frame) {
	if (!_isSystemActive) {
		return;
	}

	int fileIdx = getIdxForSubsTreName(outtakesName);
	if (fileIdx == -1 || !_gameSubsResourceEntriesFound[fileIdx]) {
		_subtitlesDataActive[kSubtitlesPrimary].currentText.clear();
		_subtitlesDataActive[kSubtitlesPrimary].currentText32.clear();
		_subtitlesDataActive[kSubtitlesPrimary].prevText.clear();
		_subtitlesDataActive[kSubtitlesPrimary].prevText32.clear();
		return;
	}

	if (!_vm->_showSubtitlesForTextCrawl
	   && (fileIdx == 1
	      || fileIdx == 2
	      || (fileIdx == 3 && (uint32)frame < 1200))) {
		// "WSTLGO" or "BRLOGO" or "INTRO" affected
		// NOTE fileIdx indexes the SUBTITLES_FILENAME_PREFIXES array
		return;
	}

	// Search in the requested TextResource at the fileIdx index of the _vqaSubsTextResourceEntries table for a quote that corresponds to the specified video frame
	// debug("Number of resource quotes to search: %d, requested frame: %u", _vqaSubsTextResourceEntries[fileIdx]->getCount(), (uint32)frame );
	const char *text = _vqaSubsTextResourceEntries[fileIdx]->getOuttakeTextByFrame((uint32)frame);
	if (_useUTF8) {
		_subtitlesDataActive[kSubtitlesPrimary].currentText32 = Common::convertUtf8ToUtf32(text);
	} else {
		_subtitlesDataActive[kSubtitlesPrimary].currentText = text;
	}
}

/**
 * Explicitly set the active subtitle text to be displayed
 * Used for debug purposes mainly.
 */
void Subtitles::setGameSubsText(int subsRole, Common::String dbgQuote, bool forceShowWhenNoSpeech) {
	if (_useUTF8) {
		_subtitlesDataActive[subsRole].currentText32 = Common::convertUtf8ToUtf32(dbgQuote);
	} else {
		_subtitlesDataActive[subsRole].currentText = dbgQuote;
	}
	_subtitlesDataActive[subsRole].forceShowWhenNoSpeech = forceShowWhenNoSpeech; // overrides not showing subtitles when no one is speaking
}


void Subtitles::addGameSubsTextToQueue(Common::String dbgQuote, uint32 duration) {
	SubtitlesQueueEntry tmpItem;
	if (duration > kMinDuration) {
		tmpItem.duration = duration;
	}
	tmpItem.quote = dbgQuote;
	_subtitlesDataQueue.insert_at(0, tmpItem);
}

void Subtitles::clearQueue() {
	_subtitlesDataQueue.clear();
}

/**
 * Sets the _isVisible member var to true if it's not already set
 * @return true if the member was set now, false if the member was already set
 */
bool Subtitles::show(int subsRole) {
	if (!_isSystemActive) {
		return false;
	}

	if (_subtitlesDataActive[subsRole].isVisible) {
		return false;
	} else {
		_subtitlesDataActive[subsRole].isVisible = true;
		return true;
	}
}

/**
 * Clears the _isVisible member var if not already clear.
 * @return true if the member was cleared, false if it was already clear.
 */
bool Subtitles::hide(int subsRole) {
	if (!_isSystemActive) {
		return false;
	}

	if (!_subtitlesDataActive[subsRole].isVisible) {
		return false;
	} else {
		_subtitlesDataActive[subsRole].isVisible = false;
		return true;
	}
}

/**
 * Checks whether the subtitles should be visible or not
 * @return the value of the _isVisible member boolean var
 */
bool Subtitles::isVisible(int subsRole) const {
	return _isSystemActive
	       && _subtitlesDataActive[subsRole].isVisible;
}

/**
 * Tick method specific for outtakes (VQA videos)
 */
void Subtitles::tickOuttakes(Graphics::Surface &s) {
	if (_isSystemActive && _vm->isSubtitlesEnabled()) {
		for (int i = 0; i < kNumOfSubtitleRoles; ++i) {
			if (isNotEmptyCurrentSubsText(i)) {
				_vm->_subtitles->show(i);
			} else {
				_vm->_subtitles->hide(i);
			}
		}

		// keep this as a separate if clause
		if (isVisible(kSubtitlesPrimary) || isVisible(kSubtitlesSecondary)) {
			draw(s);
		}
	}

}

/**
 * Tick method for in-game subtitles -- Not for outtake cutscenes (VQA videos)
 */
void Subtitles::tick(Graphics::Surface &s) {
	bool proceedToDraw = false;
	if (_isSystemActive && _vm->isSubtitlesEnabled()) {
		// Check and handle queue first
		if (!_subtitlesDataQueue.empty()) {
			if (!_subtitlesDataQueue.back().started) {
				_subtitlesDataQueue.back().started = true;
				_subtitlesDataQueue.back().timeStarted = _vm->_time->currentSystem();
				setGameSubsText(kSubtitlesSecondary, _subtitlesDataQueue.back().quote, true);
				show(kSubtitlesSecondary);
			} else {
				if (_vm->_time->currentSystem() - _subtitlesDataQueue.back().timeStarted >= _subtitlesDataQueue.back().duration) {
					setGameSubsText(kSubtitlesSecondary, "", false);
					hide(kSubtitlesSecondary);
					_subtitlesDataQueue.pop_back();
				}
			}
		}

		if (_subtitlesDataActive[kSubtitlesPrimary].isVisible
		    && !_subtitlesDataActive[kSubtitlesPrimary].forceShowWhenNoSpeech
		    && !_vm->_audioSpeech->isPlaying()) {
			_vm->_subtitles->hide(kSubtitlesPrimary); // TODO might need a better system. Don't call it always.
		}

		// keep this as a separate if clause
		if (isVisible(kSubtitlesPrimary) || isVisible(kSubtitlesSecondary)) {
			proceedToDraw = true;
		}
	}

	if (_vm->getExtraCNotify() == 3) {
		proceedToDraw = true;
	}

	if (proceedToDraw) {
		draw(s);
	}
}

bool Subtitles::isNotEmptyCurrentSubsText(int subsRole) {
	if ((_useUTF8 && !_subtitlesDataActive[subsRole].currentText32.empty())
	    || (!_useUTF8 && !_subtitlesDataActive[subsRole].currentText.empty())) {
		return true;
	} else {
		return false;
	}
}

void Subtitles::mergeSubtitleQuotes(int actorId, int quoteFirst, int quoteSecond) {
	int32 idFirst = 10000 * actorId + quoteFirst;
	int32 idSecond = 10000 * actorId + quoteSecond;
	const char *textFirst = _vqaSubsTextResourceEntries[0]->getText((uint32)idFirst);
	const char *textSecond = _vqaSubsTextResourceEntries[0]->getText((uint32)idSecond);
	if (_useUTF8) {
		_subtitlesDataActive[kSubtitlesPrimary].currentText32 = Common::convertUtf8ToUtf32(textFirst);
		_subtitlesDataActive[kSubtitlesPrimary].currentText32 += " " + Common::convertUtf8ToUtf32(textSecond);
	} else {
		_subtitlesDataActive[kSubtitlesPrimary].currentText = textFirst;
		_subtitlesDataActive[kSubtitlesPrimary].currentText += " " + Common::String(textSecond);
	}
}

/**
 * Draw method for drawing the subtitles on the display surface
 */
void Subtitles::draw(Graphics::Surface &s) {
	if (_isSystemActive && _vm->getExtraCNotify() == 3) {
		// Timing fixed for 60Hz by ScummVM team
		uint32 timeNow = _vm->_time->currentSystem();
		bool updateTimeout = false;
		// unsigned difference is intentional
		if (timeNow - _xcTimeLast > (1000u / 60u)) {
			updateTimeout = true;
			_xcTimeLast = timeNow;
		}

		_vm->_mainFont->drawString(&s, _excTitlStr, 313 - _vm->_mainFont->getStringWidth(_excTitlStr) / 2, 143, s.w, s.format.RGBToColor(240, 232, 192));

		int y = 158;
		int lineTextWidth;
		for (int i = 0; i < kxcLineCount; ++i) {
			if (updateTimeout) {
				if (_xcLineTimeouts[i] > 0) {
					--_xcLineTimeouts[i];
				} else {
					_xcLineTexts[i] = _xcStrings[_xcStringIndex];
					_xcLineTimeouts[i] = 63;
					lineTextWidth = _vm->_mainFont->getStringWidth(_xcLineTexts[i]);
					_xcLineOffsets[i] = _vm->_rnd.getRandomNumberRng(0, (306 -  lineTextWidth) > 0 ? (306 - lineTextWidth) : 0) + 155;

					_xcStringIndex = (_xcStringIndex + 1) % kxcStringCount;
				}
			}

			if (!_xcLineTexts[i].empty()) {
				int colorIndex = _xcLineTimeouts[i];
				if (colorIndex >= 32) {
					colorIndex = 63 - colorIndex;
				}
				colorIndex /= 2;
				_vm->_mainFont->drawString(&s, _xcLineTexts[i], _xcLineOffsets[i], y, s.w, s.format.RGBToColor(kTextColors[colorIndex].r, kTextColors[colorIndex].g, kTextColors[colorIndex].b));
			}
			y += 10;
		}
	}

	if (!_isSystemActive
	    || (!isVisible(kSubtitlesPrimary) && !isVisible(kSubtitlesSecondary))
	    || (!isNotEmptyCurrentSubsText(kSubtitlesPrimary) && !isNotEmptyCurrentSubsText(kSubtitlesSecondary))) {
		return;
	}

	for (int i = 0; i < kNumOfSubtitleRoles; ++i) {
		if (isVisible(i) && isNotEmptyCurrentSubsText(i)) {
			uint linesNum = 0;
			if (_useUTF8) {
				// This check is done so that lines won't be re-calculated multiple times for the same text
				if (_subtitlesDataActive[i].currentText32 != _subtitlesDataActive[i].prevText32) {
					_subtitlesDataActive[i].lines32.clear();
					_subtitlesDataActive[i].prevText32 = _subtitlesDataActive[i].currentText32;
					_font->wordWrapText(_subtitlesDataActive[i].currentText32, kTextMaxWidth, _subtitlesDataActive[i].lines32, 0, Graphics::kWordWrapEvenWidthLines | Graphics::kWordWrapOnExplicitNewLines);
				}
				linesNum = _subtitlesDataActive[i].lines32.size();
			} else {
				// This check is done so that lines won't be re-calculated multiple times for the same text
				if (_subtitlesDataActive[i].currentText != _subtitlesDataActive[i].prevText) {
					_subtitlesDataActive[i].lines.clear();
					_subtitlesDataActive[i].prevText = _subtitlesDataActive[i].currentText;
					_font->wordWrapText(_subtitlesDataActive[i].currentText, kTextMaxWidth, _subtitlesDataActive[i].lines, 0, Graphics::kWordWrapEvenWidthLines | Graphics::kWordWrapOnExplicitNewLines);
				}
				linesNum = _subtitlesDataActive[i].lines.size();
			}

			int y = kMarginTop;
			if (i == kSubtitlesPrimary) {
				// bottom of the screen
				y = s.h - (kMarginBottom + MAX(kPreferedLine, linesNum) * _font->getFontHeight());
			}

			for (uint j = 0; j < linesNum; ++j, y += _font->getFontHeight()) {
				switch (_subtitlesInfo.fontType) {
				case Subtitles::kSubtitlesFontTypeInternal:
					// shadow/outline is part of the font color data
					_font->drawString(&s, _subtitlesDataActive[i].lines[j], 0, y, s.w, 0, Graphics::kTextAlignCenter);
					break;
				case Subtitles::kSubtitlesFontTypeTTF:
					_font->drawString(&s, _subtitlesDataActive[i].lines32[j], -1, y    , s.w, s.format.RGBToColor(  0,   0,   0), Graphics::kTextAlignCenter);
					_font->drawString(&s, _subtitlesDataActive[i].lines32[j],  0, y - 1, s.w, s.format.RGBToColor(  0,   0,   0), Graphics::kTextAlignCenter);
					_font->drawString(&s, _subtitlesDataActive[i].lines32[j],  1, y    , s.w, s.format.RGBToColor(  0,   0,   0), Graphics::kTextAlignCenter);
					_font->drawString(&s, _subtitlesDataActive[i].lines32[j],  0, y + 1, s.w, s.format.RGBToColor(  0,   0,   0), Graphics::kTextAlignCenter);

					_font->drawString(&s, _subtitlesDataActive[i].lines32[j],  0, y    , s.w, s.format.RGBToColor(255, 255, 255), Graphics::kTextAlignCenter);
					break;
				}
			}
		}
	}
}

/**
 * Clear subtitles text and reset isVisible and forceShowWhenNoSpeech flags
 */
void Subtitles::clear() {
	for (uint8 i = 0; i < kNumOfSubtitleRoles; ++i) {
		_subtitlesDataActive[i].isVisible = false;
		_subtitlesDataActive[i].forceShowWhenNoSpeech = false;
		_subtitlesDataActive[i].currentText32.clear();
		_subtitlesDataActive[i].prevText32.clear();
		_subtitlesDataActive[i].lines32.clear();

		_subtitlesDataActive[i].currentText.clear();
		_subtitlesDataActive[i].prevText.clear();
		_subtitlesDataActive[i].lines.clear();
	}
}

bool Subtitles::isHDCPresent() {
	return _useHDC;
}

/**
 * Initialize/reset member vars, close open file descriptors and garbage collect subtitle fonts and text resource
 */
void Subtitles::reset() {
	clear();

	_subtitlesInfo.credits = "N/A";
	_subtitlesInfo.versionStr = "N/A";
	_subtitlesInfo.dateOfCompile = "N/A";
	_subtitlesInfo.languageMode = "N/A";
	_subtitlesInfo.fontType = kSubtitlesFontTypeInternal;
	_subtitlesInfo.fontName = "N/A";

	for (int i = 0; i < kMaxTextResourceEntries; ++i) {
		if (_vqaSubsTextResourceEntries[i] != nullptr) {
			delete _vqaSubsTextResourceEntries[i];
			_vqaSubsTextResourceEntries[i] = nullptr;
		}
		_gameSubsResourceEntriesFound[i] = false;
	}

	if (_font != nullptr) {
		delete _font;
		_font = nullptr;
	}

	_useUTF8 = false;
	_useHDC = false;
}

} // End of namespace BladeRunner
