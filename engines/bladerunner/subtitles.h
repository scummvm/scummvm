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
	friend class Debugger;
	//
	// Subtitles could be in 6 possible languages are EN_ANY, DE_DEU, FR_FRA, IT_ITA, ES_ESP
	//                   with corresponding _vm->_languageCode values: "E", "G", "F", "I", "R", "S"

	static const int kMaxNumOfSubtitlesLines = 4;					// At least one quote in the game requires 4 lines to be displayed correctly
	static const int kStartFromSubtitleLineFromTop = 2;				// Prefer drawing from this line (the top-most of available subtitle lines index is 0) by default
	static const int kSubtitlesBottomYOffsetPx = 12;				// In pixels. This is the bottom margin beneath the subtitles space
	static const int kMaxWidthPerLineToAutoSplitThresholdPx = 610;	// In pixels
	static const int kMaxTextResourceEntries = 1 + 25; 				// Support in-game subs (1) and all possible VQAs (25) with spoken dialogue or translatable text!
	static const char *SUBTITLES_FILENAME_PREFIXES[kMaxTextResourceEntries];
	static const char *SUBTITLES_FONT_FILENAME_EXTERNAL;


	BladeRunnerEngine *_vm;

	TextResource *_vqaSubsTextResourceEntries[kMaxTextResourceEntries];
	Font         *_subsFont;

	bool           _isVisible;
	bool           _forceShowWhenNoSpeech;
	Common::String _currentSubtitleTextFull;
	Common::String _subtitleLineQuote[kMaxNumOfSubtitlesLines];
	int            _subtitleLineScreenY[kMaxNumOfSubtitlesLines];
	int            _subtitleLineScreenX[kMaxNumOfSubtitlesLines];
	int            _subtitleLineSplitAtCharIndex[kMaxNumOfSubtitlesLines];
	int            _currentSubtitleLines;
	bool           _subtitlesQuoteChanged;

	bool _gameSubsResourceEntriesFound[kMaxTextResourceEntries];	// false if a TRE file did not open successfully
	bool _subsFontsLoaded;											// false if external fonts did not load
	bool _subtitlesSystemActive;									// true if the whole subtitles subsystem should be disabled (due to missing required resources)

public:
	Subtitles(BladeRunnerEngine *vm);
	~Subtitles();

	bool isSystemActive() const { return _subtitlesSystemActive; }

	void init();
	const char *getInGameSubsText(int actorId, int speech_id);						// get the text for actorId, quoteId (in-game subs)
	const char *getOuttakeSubsText(const Common::String &outtakesName, int frame);	// get the text for this frame if any

	void setGameSubsText(Common::String dbgQuote, bool force);	// for debugging - explicit set subs text
	bool show();
	bool hide();
	bool isVisible() const;
	void tick(Graphics::Surface &s);
	void tickOuttakes(Graphics::Surface &s);

private:
	void draw(Graphics::Surface &s);
	void calculatePosition();

	int getIdxForSubsTreName(const Common::String &treName) const;

	void clear();
	void reset();

};

} // End of namespace BladeRunner

#endif // BLADERUNNER_SUBTITLES_H
