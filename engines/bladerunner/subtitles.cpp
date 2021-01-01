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

#include "bladerunner/subtitles.h"

#include "bladerunner/font.h"
#include "bladerunner/text_resource.h"
#include "bladerunner/audio_speech.h"

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
	reset();
}

/**
 * Subtitles Destructor
 */
Subtitles::~Subtitles() {
	reset();
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

	int32 id = 10000 * actorId + speech_id;
	if (!_gameSubsResourceEntriesFound[0]) {
		_currentText.clear();
		return;
	}

	// Search in the first TextResource of the _vqaSubsTextResourceEntries table, which is the TextResource for in-game dialogue (i.e. not VQA dialogue)
	const char *text = _vqaSubsTextResourceEntries[0]->getText((uint32)id);
	// Use of Common::kWindows1252 codepage to fix bug whereby accented characters
	// would not show for subtitles.
	// TODO maybe the codepage here should be determined based on some subtitles property per language
	//      especially for non-latin languages that still use a FON font rather than a TTF font (eg. Greek would need Common::kWindows1253)
	_currentText = _useUTF8 ? Common::convertUtf8ToUtf32(text) : Common::U32String(text, Common::kWindows1252);
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
		_currentText.clear();
		return;
	}

	// Search in the requested TextResource at the fileIdx index of the _vqaSubsTextResourceEntries table for a quote that corresponds to the specified video frame
	// debug("Number of resource quotes to search: %d, requested frame: %u", _vqaSubsTextResourceEntries[fileIdx]->getCount(), (uint32)frame );
	const char *text = _vqaSubsTextResourceEntries[fileIdx]->getOuttakeTextByFrame((uint32)frame);
	// Use of Common::kWindows1252 codepage to fix bug whereby accented characters
	// would not show for subtitles.
	// TODO maybe the codepage here should be determined based on some subtitles property per language
	//      especially for non-latin languages that still use a FON font rather than a TTF font (eg. Greek would need Common::kWindows1253)
	_currentText = _useUTF8 ? Common::convertUtf8ToUtf32(text) : Common::U32String(text, Common::kWindows1252);
}

/**
 * Explicitly set the active subtitle text to be displayed
 * Used for debug purposes mainly.
 */
void Subtitles::setGameSubsText(Common::String dbgQuote, bool forceShowWhenNoSpeech) {
	// TODO is Common::kWindows1252 correct here?
	_currentText = _useUTF8 ? Common::convertUtf8ToUtf32(dbgQuote) : Common::U32String(dbgQuote, Common::kWindows1252);
	_forceShowWhenNoSpeech = forceShowWhenNoSpeech; // overrides not showing subtitles when no one is speaking
}

/**
 * Sets the _isVisible member var to true if it's not already set
 * @return true if the member was set now, false if the member was already set
 */
bool Subtitles::show() {
	if (!_isSystemActive) {
		return false;
	}

	if (_isVisible) {
		return false;
	}

	_isVisible = true;
	return true;
}

/**
 * Clears the _isVisible member var if not already clear.
 * @return true if the member was cleared, false if it was already clear.
 */
bool Subtitles::hide() {
	if (!_isSystemActive) {
		return false;
	}

	if (!_isVisible) {
		return false;
	}

	_isVisible = false;
	return true;
}

/**
 * Checks whether the subtitles should be visible or not
 * @return the value of the _isVisible member boolean var
 */
bool Subtitles::isVisible() const {
	return !_isSystemActive || _isVisible;
}

/**
 * Tick method specific for outtakes (VQA videos)
 */
void Subtitles::tickOuttakes(Graphics::Surface &s) {
	if (!_isSystemActive || !_vm->isSubtitlesEnabled()) {
		return;
	}

	if (_currentText.empty()) {
		_vm->_subtitles->hide();
	} else {
		_vm->_subtitles->show();
	}

	if (!_isVisible) { // keep it as a separate if
		return;
	}

	draw(s);
}

/**
 * Tick method for in-game subtitles -- Not for outtake cutscenes (VQA videos)
 */
void Subtitles::tick(Graphics::Surface &s) {
	if (!_isSystemActive || !_vm->isSubtitlesEnabled()) {
		return;
	}

	if (_isVisible && !_forceShowWhenNoSpeech && !_vm->_audioSpeech->isPlaying()) {
		_vm->_subtitles->hide(); // TODO might need a better system. Don't call it always.
	}

	if (!_isVisible) { // keep it as a separate if
		return;
	}

	draw(s);
}

/**
 * Draw method for drawing the subtitles on the display surface
 */
void Subtitles::draw(Graphics::Surface &s) {
	if (!_isSystemActive || !_isVisible || _currentText.empty()) {
		return;
	}

	// This check is done so that lines won't be re-calculated multiple times for the same text
	if (_currentText != _prevText) {
		lines.clear();
		_prevText = _currentText;
		_font->wordWrapText(_currentText, kTextMaxWidth, lines, 0, Graphics::kWordWrapEvenWidthLines | Graphics::kWordWrapOnExplicitNewLines);
	}

	int y = s.h - (kMarginBottom + MAX(kPreferedLine, lines.size()) * _font->getFontHeight());

	for (uint i = 0; i < lines.size(); ++i, y += _font->getFontHeight()) {
		switch (_subtitlesInfo.fontType) {
			case Subtitles::kSubtitlesFontTypeInternal:
				// shadow/outline is part of the font color data
				_font->drawString(&s, lines[i], 0, y, s.w, 0, Graphics::kTextAlignCenter);
				break;
			case Subtitles::kSubtitlesFontTypeTTF:
				_font->drawString(&s, lines[i], -1, y    , s.w, s.format.RGBToColor(  0,   0,   0), Graphics::kTextAlignCenter);
				_font->drawString(&s, lines[i],  0, y - 1, s.w, s.format.RGBToColor(  0,   0,   0), Graphics::kTextAlignCenter);
				_font->drawString(&s, lines[i],  1, y    , s.w, s.format.RGBToColor(  0,   0,   0), Graphics::kTextAlignCenter);
				_font->drawString(&s, lines[i],  0, y + 1, s.w, s.format.RGBToColor(  0,   0,   0), Graphics::kTextAlignCenter);

				_font->drawString(&s, lines[i],  0, y    , s.w, s.format.RGBToColor(255, 255, 255), Graphics::kTextAlignCenter);
				break;
		}
	}
}

/**
 * Initialize a few basic member vars
 */
void Subtitles::clear() {
	_isVisible = false;
	_forceShowWhenNoSpeech = false;
	_currentText.clear();
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
}

} // End of namespace BladeRunner
