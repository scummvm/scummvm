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

#ifndef BLADERUNNER_SUBTITLES_H
#define BLADERUNNER_SUBTITLES_H

#include "bladerunner/bladerunner.h"

#include "common/str.h"
#include "graphics/surface.h"

#include "common/file.h"
#include "common/substream.h"


namespace BladeRunner {

class BladeRunnerEngine;
//class SaveFileReadStream;
//class SaveFileWriteStream;
class TextResource;
class Font;

class Subtitles {
	//
	// Subtitles could be in 6 possible languages are EN_ANY, DE_DEU, FR_FRA, IT_ITA, ES_ESP
	//                   with corresponding _vm->_languageCode values: "E", "G", "F", "I", "R", "S"
	// TODO Maybe support 1 + 6 * 26 entries to support multiple language subtitles? Would that be useful?
	// TODO Or just support the current _vm->_languageCode ? [current implementation]
	static const int kMaxNumOfSubtitlesLines = 3;
	static const int kMaxWidthPerLineToAutoSplitThresholdPx = 610;
	static const int kMaxTextResourceEntries = 1 + 25; // Support in-game subs (1) and all possible VQAs (25) with spoken dialogue or translatable text!

	// 'static const' declaration for SUBTITLES_FILENAME_PREFIXES (and init in cpp) causes warning:
	// "declaration requires a global destructor [-Wglobal-constructors]"
	// So, we declare it as a plain member should be fine, since we only use one instance of Subtitles anyway.
	/*
	* All supported TRE entries that can be in the SUBTITLES.MIX files need to:
	* 1. Have the language code appended (after an underscore delimiter '_').
	* 2. Have the suffix extension ".TRx"; the last letter in extension "TR*" should also be the language code
	*
	* Important Note: If/When adding new Text Resources here --> Update kMaxTextResourceEntries
	* and also check if method getIdxForSubsTreName() needs updating.
	*/
	const char *SUBTITLES_FILENAME_PREFIXES[kMaxTextResourceEntries]{
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
		"END03"             // 25
	};

	BladeRunnerEngine *_vm;

	TextResource    *_vqaSubsTextResourceEntries[kMaxTextResourceEntries];
	Font            *_subsFont;

	bool				_isVisible;
	Common::String		_currentSubtitleTextFull;
	Common::String		_subtitleLineQuote[kMaxNumOfSubtitlesLines];
	int _subtitleLineScreenY[kMaxNumOfSubtitlesLines];
	int _subtitleLineScreenX[kMaxNumOfSubtitlesLines];
	int _subtitleLineSplitAtCharIndex[kMaxNumOfSubtitlesLines];
	int _currentSubtitleLines;
	bool _subtitlesQuoteChanged;

	bool _gameSubsResourceEntriesFound[kMaxTextResourceEntries];	// false if a TRE file did not open successfully
	bool _subsFontsLoaded;											// false if external fonts did not load
	bool _subtitlesSystemInactive;									// true if the whole subtitles subsystem should be disabled (due to missing required resources)

public:
	Subtitles(BladeRunnerEngine *vm);
	~Subtitles();

	void init();
	void setSubtitlesSystemInactive(bool flag);                                    // disable subtitles system (possibly due to missing important resources like SUBTITLES.MIX file)
	const char *getInGameSubsText(int actorId, int speech_id) ;     // get the text for actorId, quoteId (in-game subs)
	const char *getOuttakeSubsText(const Common::String &outtakesName, int frame);  // get the text for this frame if any

	void setGameSubsText(Common::String dbgQuote);                  // for debugging - explicit set subs text
	bool show();
	bool hide();
	bool isVisible() const;
	void tick(Graphics::Surface &s);
	void tickOuttakes(Graphics::Surface &s);

private:
	void draw(Graphics::Surface &s);
	// bool showAt(int x, int y);               // TODO maybe future use (?)
	void calculatePosition();

	int getIdxForSubsTreName(const Common::String &treName) const;

	void clear();
	void reset();

};

} // End of namespace BladeRunner

#endif // BLADERUNNER_SUBTITLES_H
