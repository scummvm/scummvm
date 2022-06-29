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

#ifndef BLADERUNNER_SUBTITLES_H
#define BLADERUNNER_SUBTITLES_H

#include "bladerunner/bladerunner.h"

#include "common/str.h"
#include "common/ustr.h"

namespace Graphics {
class Font;
}

namespace BladeRunner {

class BladeRunnerEngine;
class TextResource;

class Subtitles {
	friend class Debugger;
	friend class KIASectionSettings;
	//
	// Subtitles could be in 6 possible languages are EN_ANY, DE_DEU, FR_FRA, IT_ITA, RU_RUS, ES_ESP
	// with corresponding _vm->_languageCode values: "E", "G", "F", "I", "E", "S" (Russian version is built on top of English one)
	static const uint kPreferedLine            = 2;      // Prefer drawing from this line (the bottom-most of available subtitle lines index is 0) by default
	static const int  kMarginBottom            = 12;     // In pixels. This is the bottom margin beneath the subtitles space
	static const int  kMarginTop               = 12;     // In pixels. This is the top margin before secondary subtitles
	static const int  kTextMaxWidth            = 610;    // In pixels
	static const int  kMaxTextResourceEntries  = 27;     // Support in-game subs (1) and all possible VQAs (26) with spoken dialogue or translatable text
	static const int  kMaxLanguageSelectionNum = 1024;   // Max allowed number of languages to select from (should be available in the MIX file)

	static const char *SUBTITLES_FILENAME_PREFIXES[kMaxTextResourceEntries];
	static const char *SUBTITLES_FONT_FILENAME_EXTERNAL;
	static const char *SUBTITLES_VERSION_TRENAME;

	static const int  kNumOfSubtitleRoles       = 2;

	BladeRunnerEngine *_vm;

	enum SubtitlesFontType {
		kSubtitlesFontTypeInternal,
		kSubtitlesFontTypeTTF
	};

	struct SubtitlesInfo {
		Common::String    versionStr;
		Common::String    dateOfCompile;
		Common::String    languageMode;
		Common::String    credits;
		SubtitlesFontType fontType;
		Common::String    fontName;

		SubtitlesInfo() : versionStr(""), dateOfCompile(""), languageMode(""), credits(""), fontName("")  { fontType = kSubtitlesFontTypeInternal; };
	};

	struct SubtitlesData {
		bool isVisible;
		bool forceShowWhenNoSpeech;
		// U32String for when we use an external font that supports UTF-32 encoding
		Common::U32String currentText32;
		Common::U32String prevText32;
		Common::Array<Common::U32String> lines32;

		// For now, we're using the original game's FON format for native font
		// and the original MIX for file for text resources.
		// This means that when not explicitly using an external font,
		// the text resources are in extended ASCII format that index the native font FON.
		// FUTURE On a next revision we should support UTF-8 text in the MIX files which
		// would work with external font.
		Common::String currentText;
		Common::String prevText;
		Common::Array<Common::String> lines;
	};

	SubtitlesInfo  _subtitlesInfo;
	TextResource  *_vqaSubsTextResourceEntries[kMaxTextResourceEntries];

	Graphics::Font *_font;
	bool            _useUTF8;

	Common::Array<SubtitlesData> _subtitlesData;

	bool _gameSubsResourceEntriesFound[kMaxTextResourceEntries]; // false if a TRE file did not open successfully
	bool _isSystemActive;                                        // true if the whole subtitles subsystem should be disabled (due to missing required resources)

public:
	Subtitles(BladeRunnerEngine *vm);
	~Subtitles();

	bool isSystemActive() const { return _isSystemActive; }

	void init();
	SubtitlesInfo getSubtitlesInfo() const;
	void loadInGameSubsText(int actorId, int speech_id);                     // get the text for actorId, quoteId (in-game subs)
	void loadOuttakeSubsText(const Common::String &outtakesName, int frame); // get the text for this frame if any

	void setGameSubsText(int subsRole, Common::String dbgQuote, bool force); // for debugging - explicit set subs text

	bool show(int subsRole);
	bool hide(int subsRole);
	void clear();

	bool isVisible(int subsRole) const;
	void tick(Graphics::Surface &s);
	void tickOuttakes(Graphics::Surface &s);

	enum SubtitlesRole {
		kSubtitlesPrimary,
		kSubtitlesSecondary
	};

private:
	void draw(Graphics::Surface &s);

	int getIdxForSubsTreName(const Common::String &treName) const;

	void reset();

	bool isNotEmptyCurrentSubsText(int subsRole);
};

} // End of namespace BladeRunner

#endif // BLADERUNNER_SUBTITLES_H
