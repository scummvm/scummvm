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
	static const int kMaxTextResourceEntries = 1 + 25; // Support in-game subs (1) and all possible VQAs (26) with spoken dialogue!
	static const Common::String SUBTITLES_FILENAME_PREFIXES[kMaxTextResourceEntries];
	static const Common::String SUBTITLES_FONT_FILENAME;


	BladeRunnerEngine *_vm;

	TextResource    *_vqaSubsTextResourceEntries[kMaxTextResourceEntries];
	Font            *_subsFont;
#if !BLADERUNNER_SUBTITLES_EXTERNAL_FONT
	Font            *_subsBgFont; // needed for internal font to be used as a shadow effect and make subtitles more legible in certain situations
#endif // BLADERUNNER_SUBTITLES_EXTERNAL_FONT

	bool                 _isVisible;
	Common::String       _currentSubtitleTextFull;
	Common::String _subtitleLineQuote[kMaxNumOfSubtitlesLines];
	int _subtitleLineScreenY[kMaxNumOfSubtitlesLines];
	int _subtitleLineScreenX[kMaxNumOfSubtitlesLines];
	int _subtitleLineSplitAtCharIndex[kMaxNumOfSubtitlesLines];
	int _currentSubtitleLines;
	bool _subtitlesQuoteChanged;

	bool _gameSubsResourceEntriesFound[kMaxTextResourceEntries];          // false if a TRE file did not open successfully
	bool _subsFontsLoaded;                                          // false if external fonts did not load, or internal fonts (fore and background) did not load
	bool _subtitlesSystemInactive;                                  // true if the whole subtitles subsystem should be disabled (due to missing required resources)

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
