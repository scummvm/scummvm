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

#ifndef MOHAWK_ZOOMBINI_TEXT_H
#define MOHAWK_ZOOMBINI_TEXT_H

#include "common/hashmap.h"
#include "common/language.h"
#include "common/noncopyable.h"
#include "common/scummsys.h"
#include "common/str-enc.h"
#include "graphics/fontman.h"
#include "graphics/fonts/ttf.h"

#include "mohawk/resource.h"
#include "mohawk/zoombini_resource.h"

namespace Common {

class SeekableReadStream;

} // End of namespace Common

namespace Mohawk {

class TTFLoader;
class MohawkEngine_Zoombini;
class ZmbReleaseInfo;

/** Provide release-specific strings, credits, fonts, text encoding, and Zoombini names. */
class ZoombiniText : private Common::NonCopyable {
public:
	/** Load localized text sources and fonts for the engine's release. */
	explicit ZoombiniText(MohawkEngine_Zoombini *vm);
	/** Release loaded fonts, strings, and name caches. */
	~ZoombiniText();

	/** Tokenize a @ref Common::String by CRLF, CR, or LF. */
	static Common::Array<Common::String> tokenizeLines(const Common::String &text);
	/** Tokenize a @ref Common::U32String by CRLF, CR, or LF. */
	static Common::Array<Common::U32String> tokenizeLines(const Common::U32String &text);
	/** Format paragraph and line indices as a stable text-dump key. */
	static Common::String formatCreditLineKey(uint32 paragraphIndex, uint32 lineIndex);

	enum Key : uint32 {
		kNone = 0,
		// RODMAP: Page Names
		kPicker = 1,
		kBridge = 2,
		kCaves = 3,
		kPizza = 4,
		kBasecamp1 = 5,
		kFerry = 6,
		kLilly = 7,
		kSlides = 8,
		kFleens = 9,
		kHotel = 10,
		kNet = 11,
		kBasecamp2 = 12,
		kTunnels = 13,
		kSmoke = 14,
		kMaze = 16,
		kTown = 17,
		// XFER: Entrance to Zoombiniville
		kXferVillePopulation = 100,
		// RODMAP/XFER: Route Names
		kRoute1 = 200,
		kRoute2,
		kRoute3,
		kRoute4,
		// RODMAP: Practice/Journey Stat
		kNewGame = 300,
		kPracticeMode,
		kContinueJourney,
		kPracticeTitle,
		kPracticeDesc1,
		kPracticeDesc2,
		kPracticeDesc3,
		kPracticeDesc4,
		// RODMAP: (v2.0PL-only) Journey-stat location descriptions.
		kPolishJourneyStatPicker,
		kPolishJourneyStatBasecamp1,
		kPolishJourneyStatBasecamp2,
		kPolishJourneyStatTown,
		// RODMAP: Legend
		kTerrainKey,
		kChooseLevel,
		kLevel1,
		kLevel2,
		kLevel3,
		kLevel4,
		/** (v2.0TLC-only) Label for options button. */
		kRodMapOptionsButton,
		// RODMAP: Journey-stat location labels for the USv1.0 and Europe-v1 families.
		// These are distinct from the full page-name keys. Europe-v1 uses STRL 2002
		// instead of STRL 2000; USv1.0 uses executable strings, including its demos.
		kJourneyStatZoombiniIsle,
		kJourneyStatShelterRock,
		kJourneyStatShadeTree,
		kJourneyStatZoombiniton,
		// RODMAP: Practice return-path descriptions for the USv1.0 and Europe-v1 families.
		// Four progress-dependent four-line blocks. Europe-v1 uses STRL 2002 entries 5-20;
		// USv1.0 uses executable strings, including its demos.
		kPracticeReturnInitial1,
		kPracticeReturnInitial2,
		kPracticeReturnInitial3,
		kPracticeReturnInitial4,
		kPracticeReturnShelterRock1,
		kPracticeReturnShelterRock2,
		kPracticeReturnShelterRock3,
		kPracticeReturnShelterRock4,
		kPracticeReturnShadeTree1,
		kPracticeReturnShadeTree2,
		kPracticeReturnShadeTree3,
		kPracticeReturnShadeTree4,
		kPracticeReturnZoombiniton1,
		kPracticeReturnZoombiniton2,
		kPracticeReturnZoombiniton3,
		kPracticeReturnZoombiniton4,
		// TOWN: Memorial
		kMemorialJanuary = 400,
		kMemorialFebruary,
		kMemorialMarch,
		kMemorialApril,
		kMemorialMay,
		kMemorialJune,
		kMemorialJuly,
		kMemorialAugust,
		kMemorialSeptember,
		kMemorialOctober,
		kMemorialNovember,
		kMemorialDecember,
		kMemorialWhenLevel,
		kMemorialHonorMonument,
		kMemorialHonorWindmill,
		kMemorialHonorObservatory,
		kMemorialHonorBowlingAlley,
		kMemorialHonorGeneralStore,
		kMemorialHonorSwimmingPool,
		kMemorialHonorPlayground,
		kMemorialHonorBandShell,
		kMemorialHonorSchool,
		kMemorialHonorLibrary,
		kMemorialHonorFire,
		kMemorialHonorOpera,
		kMemorialHonorCityHall,
		kMemorialHonorClockTower,
		kMemorialHonorMuseum,
		kMemorialHonorCourt,
		kMemorialRoute1Level1,
		kMemorialRoute1Level2,
		kMemorialRoute1Level3,
		kMemorialRoute1Level4,
		kMemorialRoute2Level1,
		kMemorialRoute2Level2,
		kMemorialRoute2Level3,
		kMemorialRoute2Level4,
		kMemorialRoute3Level1,
		kMemorialRoute3Level2,
		kMemorialRoute3Level3,
		kMemorialRoute3Level4,
		kMemorialRoute4Level1,
		kMemorialRoute4Level2,
		kMemorialRoute4Level3,
		kMemorialRoute4Level4,
		// DIALOG: OPTIONS
		kOptionsTitle = 1000,
		kOptionsLegendOn,
		kOptionsLegendOff,
		kOptionsToggle,
		kOptionsNewGame,
		kOptionsLoadGame,
		kOptionsSaveGame,
		kOptionsQuit,
		kOptionsSound,
		kOptionsMusic,
		kOptionsStickyMouse,
		kOptionsTransitions,
		kOptionsCredits,
		kOptionsHelpAudio,   // TLC-only text key.
		kOptionsTouchSense,  // TLC-only text key; ScummVM does not implement TouchSense.
		kDialogButtonAccept, // v1.0ESPT-only one-button alert label.
		// DIALOG: MsgBox
		kDialogTitleSave = 1100,
		kDialogTitleSaveAs,
		kDialogTitleLoad,
		kDialogBodyGoMapWillLost,
		kDialogBodyNoSavedGames,
		kDialogBodyCreateAndSaveNewGame,
		kDialogBodyReplaceGame,
		kDialogBodySaveCurrentGame,
		kDialogBodySaveDirtyGame,
		kDialogBodyCannotSaveInPractice,
		kDialogBodyCreateNewGame,
		kDialogBodyCannotSaveMoreGame,
		kDialogBodyCannotLoadInPractice,
		kDialogBodyCannotCreateNewInPractice,
		kDialogBodyNewGame,
		kDialogBodyReallyQuit,
		kDialogBodySaveBeforeQuit,
		/**
		 * v2.0US only key.
		 * ScummVM added save delete feature on v1.x, so we need to provide value for this ourselves.
		 */
		kDialogBodyRemoveGame,
		kDialogButtonNewGame = 1200,
		kDialogButtonReplaceTitle,
		kDialogButtonLoseThem,
		kDialogButtonKeepThem,
		kDialogButtonOkay,
		kDialogButtonCancel,
		kDialogButtonYes,
		kDialogButtonNo,
		kDialogButtonQuit,
		kDialogButtonLoad,
		kDialogButtonSave,
		kDialogButtonReplace,
		// DIALOG: Help
		kDialogHelpTitle,
		kDialogHelpLevel,
		kDialogButtonPrev,
		kDialogButtonNext,
		/** (v1.1US/v2.0TLC only) Picker Help update-version easter-egg text key. */
		kDialogHelpPickerUpdateVersion,
		// Notification Box
		kNotiBoxMusicOn = 1300,
		kNotiBoxMusicOff,
		kNotiBoxSoundOn,
		kNotiBoxSoundOff,
		kNotiBoxLessAction,
		kNotiBoxMoreAction,
		kNotiBoxHideCursor,
		kNotiBoxShowCursor,
		kNotiBoxStickeyMouse,
		kNotiBoxNonStickeyMouse,
		kNotiBoxTransitionsOn,
		kNotiBoxTransitionsOff,
		kNotiBoxAutoStickeyOn,
		kNotiBoxAutoStickeyOff,
		/** (v2.0TLC-only) text key. */
		kNotiBoxHelpAudioOn,
		/** (v2.0TLC-only) text key. */
		kNotiBoxHelpAudioOff,
		/** (v2.0TLC-only) text key. ScummVM does not implement TouchSense. */
		kNotiBoxTouchSenseOn,
		/** (v2.0TLC-only) text key. ScummVM does not implement TouchSense. */
		kNotiBoxTouchSenseOff,
		/** (v1.0JP-only) Japanese release notification for procedural Latin names. */
		kNotiBoxEnglishNames,
		/** (v1.0JP-only) Japanese release notification for procedural halfwidth katakana names. */
		kNotiBoxJapaneseNames,
		// ScummVM-only text keys
		kSvmDialogBodyAskSaveAfterDebugStateMutation = 2000,
		kSvmDialogBodyLoadNewerSaveFormat,
		/** Confirm loading a native save from the other platform. */
		kSvmDialogBodyLoadOtherPlatformSave,
		kSvmDialogBodyCannotLoadSave,
		kSvmDialogBodyCannotSaveNameEncoding,
		kSvmDialogBodyAskRegenerateZoombiniNames,
	};

	/** One paragraph of localized credit text. */
	struct CreditParagraph {
		/** Localized lines in this paragraph. */
		Common::Array<Common::U32String> _lines;
		/** Number of blank lines following the paragraph. */
		uint32 _blankLineCount = 1;
		/** Whether the first nonempty line uses the title font and palette. */
		bool _firstLineIsTitle = true;

		/** Build an empty paragraph. */
		CreditParagraph() {}
		/** Build a paragraph from lines and a trailing blank-line count. */
		CreditParagraph(const Common::Array<Common::U32String> &lines, uint32 blankLineCount, bool firstLineIsTitle = true)
			: _lines(lines), _blankLineCount(blankLineCount), _firstLineIsTitle(firstLineIsTitle) {}

		/** Return the total rendered line count including trailing blank lines. */
		uint32 getTotalLineCount() const { return _lines.size() + _blankLineCount; }
	};

	/** One numeric-key localized string. */
	struct LocalizedString {
		/** Numeric localized-string key. */
		uint32 _key = 0;
		/** Decoded localized text. */
		Common::U32String _text;

		/** Build an empty string entry. */
		LocalizedString() {}
		/** Build a string entry from a key and decoded text. */
		LocalizedString(uint32 key, const Common::U32String &text)
			: _key(key), _text(text) {}
	};

	/** Return the normal text font loaded during initialization. */
	const Graphics::Font *getTextFont();
	/** Return the title font loaded during initialization. */
	const Graphics::Font *getTitleFont();
	/** Return the font selected by a renderer-facing usage enum. */
	const Graphics::Font *getFont(ZoombiniFontUsage fontUsage);
	/** Return whether the selected font retains grayscale glyph coverage. */
	bool fontUsesAntialiasing(ZoombiniFontUsage fontUsage) const;

	// Convert strings to @ref Common::U32String with the selected code page.
	enum StringKind {
		kExeString = 0,
		kResString,
	};
	/** Return the code page used for executable strings in this release. */
	Common::CodePage getExeCodePage() const { return _exeCodePage; }
	/** Decode a null-terminated byte string with the selected source code page. */
	Common::U32String toU32String(const byte *buf, StringKind strKind) const;
	/** Decode a bounded byte string with the selected source code page. */
	Common::U32String toU32String(const byte *buf, int32 len, StringKind strKind) const;
	/** Decode a null-terminated char string with the selected source code page. */
	Common::U32String toU32String(const char *str, StringKind strKind) const;
	/** Decode a bounded char string with the selected source code page. */
	Common::U32String toU32String(const char *str, int32 len, StringKind strKind) const;
	/** Decode a ScummVM string with the selected source code page. */
	Common::U32String toU32String(const Common::String &str, StringKind strKind) const;
	/** Encode UTF-32 text into the selected executable or resource code page. */
	Common::String fromU32String(const Common::U32String &ustr, StringKind strKind) const;

	// [*] STRL resource
	/** Decode every string in a STRL resource. */
	bool getStrl(Common::Array<Common::U32String> &outStrs, ZmbResource resource);
	/** Decode one indexed string from a STRL resource. */
	bool getStrl(Common::U32String &outStr, ZmbResource resource, uint16 subStrIdx);

	// [*] Zoombini names
	/** Fixed Japanese name shared by Picker's F1 override and saved-name compatibility checks. */
	static constexpr char32_t kJapaneseF1DebugName[] = U"チョビ";
	/** Fixed Japanese name shared by Picker's F2 override and saved-name compatibility checks. */
	static constexpr char32_t kJapaneseDebugF1Name[] = U"史昭";
	/**
	 * Return whether this release allows the given serialized Zoombini name.
	 * Accept shared ASCII names, Japanese generator outputs or Korean STRL pool names; localized names must round-trip exactly through the release's codec.
	 * Preserve name bytes and never infer their source language.
	 * Korean resource lookup fills the generator's cache without drawing a name or advancing random state.
	 */
	bool isAllowedZoombiniName(const Common::String &name);

	/**
	 * Fixed Japanese name requested by Picker when generating a new name.
	 * Possibly a v1.0JP debug-only feature.
	 */
	enum JPFixedNameKind {
		/**
		 * Generate a random name without a fixed-name override.
		 */
		kJPFixedNameNone,
		/**
		 * Select the fixed Japanese name "Chobi".
		 */
		kJPFixedJPNameChobi,
		/**
		 * Select the fixed Japanese name requested by F2; F1 takes priority if both keys are held.
		 * One possible reading is "Fumiaki".
		 * Likely a kanji-name test using Japanese localization programmer Fumiaki Utsunomiya's given name.
		 */
		kJPFixedJPNameFumiaki
	};
	/** Generate a name without Picker-held key overrides. */
	Common::U32String pickNextZoombiniName();
	/**
	 * Pick the next name for a preview Zoombini.
	 * Different IDs may contain equal strings, and existing Zoombini names do not restrict selection.
	 *
	 * Latin: generates a name procedurally from syllable tables.
	 * Korean: draws each of the 625 STRL name IDs once per cycle, then clears the draw history.
	 * Japanese: generates halfwidth katakana or returns the requested fixed name.
	 *
	 * Ctrl+M selects Latin generation instead; fixed-name requests then have no effect.
	 */
	Common::U32String pickNextZoombiniName(JPFixedNameKind requestedFixedName);
	/** Toggle Japanese versus Latin procedural names and return the selected Japanese-name state. */
	bool toggleJapaneseNames() {
		_useJapaneseNames = !_useJapaneseNames;
		return _useJapaneseNames;
	}

	/** Generate a Latin Zoombini name procedurally from syllable tables. */
	Common::U32String generateLatinZoombiniName();
	/**
	 * Draw an unused Korean name ID, and return its cached or loaded STRL name.
	 * The cycle is restarted when exhausted.
	 */
	Common::U32String getKoreanZoombiniName();
	/** Generate a Japanese Zoombini name from weighted halfwidth katakana or the requested fixed name. */
	Common::U32String generateJapaneseZoombiniName(JPFixedNameKind requestedFixedName);

	// [*] Text getters
	/** Return the localized display name for a page type. */
	Common::U32String getPageName(ZoombiniPageType pageType) const;
	/** Return one localized string by numeric key. */
	Common::U32String getLocalizedString(uint32 textKey) const;
	/** Copy all localized strings in stable key order. */
	void getLocalizedStrings(Common::Array<LocalizedString> &strings) const;
	/** Copy localized credit paragraphs in display order. */
	void getLocalizedCredits(Common::Array<CreditParagraph> &paragraphs) const;

private:
	/** Smallest Japanese target length, before pair extension or an early NUL. */
	static constexpr uint kJapaneseNameMinTargetLength = 3;
	/** Largest Japanese target length, before pair extension or an early NUL. */
	static constexpr uint kJapaneseNameMaxTargetLength = 6;
	/** Shared first-character pool; the terminating NUL is not selectable. */
	static constexpr char32_t kJapaneseInitialKana[] = U"ｱｲｳｴｵｶｷｸｹｺｻｼｽｾｿﾀﾁﾂﾃﾄﾅﾆﾇﾈﾉﾊﾋﾌﾍﾎﾏﾐﾑﾒﾓﾔﾕﾖﾜ";
	/** Shared plain-kana pool for later positions, including nasal N. */
	static constexpr char32_t kJapaneseNormalKana[] = U"ｱｲｳｴｵｶｷｸｹｺｻｼｽｾｿﾀﾁﾂﾃﾄﾅﾆﾇﾈﾉﾊﾋﾌﾍﾎﾏﾐﾑﾒﾓﾔﾕﾖﾜﾝ";
	/** Shared bases that can precede a semi-voiced mark. */
	static constexpr char32_t kJapaneseSemiVoicedKana[] = U"ﾊﾋﾌﾍﾎ";
	/** Shared bases that can precede a voiced mark. */
	static constexpr char32_t kJapaneseVoicedKana[] = U"ｶｷｸｹｺｻｼｽｾｿﾀﾃﾄﾊﾋﾌﾍﾎ";
	/** Shared bases that can precede a contracted small kana. */
	static constexpr char32_t kJapaneseContractedKana[] = U"ｷｼﾁﾆﾋﾐ";
	/** Shared contracted suffixes; generation also permits this array's terminating NUL. */
	static constexpr char32_t kJapaneseSmallKana[] = U"ｬｭｮ";
	/** Return whether the given character is in the specified Japanese name-generation kana table. */
	static bool isKanaInJapaneseNameTable(char32_t character, const char32_t *table);
	/** Recognize any possible Japanese generator output, including fixed names, pair extension and early termination. */
	static bool isGeneratedJapaneseZoombiniName(const Common::U32String &name);
	/** Read one Korean name by ID, reusing the same resource cache as random name generation. */
	Common::U32String getKoreanZoombiniNameById(uint16 nameId);

	/** Localized page-name key indexed by page type. */
	struct PageKeyEntry {
		/** Page whose display name is supplied by this entry. */
		ZoombiniPageType pageType;
		/** Localized string key for the page's display name. */
		Key key;
	};
	/** Display-name keys for the journey's interactive and puzzle pages. */
	static constexpr PageKeyEntry kPageKeyTable[] = {
		{ZoombiniPageType::kPicker, kPicker},
		{ZoombiniPageType::kBridge, kBridge},
		{ZoombiniPageType::kCaves, kCaves},
		{ZoombiniPageType::kPizza, kPizza},
		{ZoombiniPageType::kBasecamp1, kBasecamp1},
		{ZoombiniPageType::kFerry, kFerry},
		{ZoombiniPageType::kLilly, kLilly},
		{ZoombiniPageType::kSlides, kSlides},
		{ZoombiniPageType::kFleens, kFleens},
		{ZoombiniPageType::kHotel, kHotel},
		{ZoombiniPageType::kNet, kNet},
		{ZoombiniPageType::kBasecamp2, kBasecamp2},
		{ZoombiniPageType::kTunnels, kTunnels},
		{ZoombiniPageType::kSmoke, kSmoke},
		{ZoombiniPageType::kMaze, kMaze},
		{ZoombiniPageType::kTown, kTown},
	};

	/** Point size and cached result for one font role. */
	struct FontState {
		/** Size interpreted using the selected loader's character or cell mode. */
		int point = 0;
		/** Name of the font owned by FontMan. */
		Common::String cacheName;
		/** Whether the selected font retains grayscale glyph coverage. */
		bool enableHinting = false;
	};

	/** Internal STRL text-table metadata. */
	struct ResTextEntry {
		/** Localized text key. */
		uint32 key;
		/** STRL resource containing the text. */
		int16 resourceId;
		/** Zero-based string index within the STRL resource. */
		uint16 stringIndex;
	};

	/** Byte images prepared by executable sources and selected by their text tables. */
	enum class ExeTextDataSource {
		/**
		 * Use raw executable file bytes, with offsets measured from the beginning of the file.
		 * Windows NE/PE tables and the Japanese Macintosh demo use this origin.
		 * For Macintosh PPC PEF, this selects the original data-fork container, including literals embedded in PPC code.
		 */
		kRawExecutableFile,
		/**
		 * Use the expanded RLE-compressed DATA 0 from a Macintosh 68K executable's resource fork.
		 * Offsets are relative to the original A5 global-data base, which maps to index 0 in the expanded buffer.
		 *
		 * @remarks Uses a game-specific decoder matching Z1's verified 68K startup routine.
		 * Related enhanced-RLE formats exist in other 68K runtimes; this encoding is not known to be unique to Z1.
		 * @see expandMac68kResForkRleGlobals() for the stream layout and related-format references.
		 */
		kExpandedMac68kA5Globals,
		/**
		 * Use the expanded selected data section of a Macintosh PPC PEF executable.
		 * Offsets are relative to the beginning of that section, including its zero-initialized tail.
		 *
		 * @remarks Expansion follows the PEF standard's pattern-init instructions and zero-init tail rules.
		 * @see Apple, Mac OS Runtime Architectures, chapter 8, pp. 8-7..8-14:
		 * https://developer.apple.com/library/archive/documentation/mac/pdf/MacOS_RT_Architectures.pdf#page=168
		 */
		kExpandedMacPpcPefDataSection,
	};

	/** One Key-offset-size entry in an executable text table. */
	struct ExeTextEntry {
		Key key;
		/** Byte offset from the origin selected by the containing @ref ExeTextTable. */
		uint32 offset;
		/** Number of text bytes, excluding the trailing NUL terminator. */
		uint16 length;
		/** Map a string to a byte range within its table's selected image. */
		constexpr ExeTextEntry(Key srcKey, uint32 srcOffset, uint16 srcLength)
			: key(srcKey), offset(srcOffset), length(srcLength) {}
	};

	/** Executable text entries sharing one byte image and offset origin. */
	struct ExeTextTable {
		/** Static key-offset-size entries in this table. */
		const ExeTextEntry *entries;
		/** Number of entries in @ref entries. */
		uint32 entryCount;
		/** Byte image and offset origin used by every entry in this table. */
		ExeTextDataSource dataSource;

		/** Associate a static text table with its byte image. */
		constexpr ExeTextTable(const ExeTextEntry *srcEntries, uint32 srcEntryCount, ExeTextDataSource srcDataSource)
			: entries(srcEntries), entryCount(srcEntryCount), dataSource(srcDataSource) {}
	};

	typedef void (ZoombiniText::*ExeTextPatchFunction)();

	/** Storage and traversal method owning a release's credit sequence. */
	enum class CreditSourceKind {
		kNone = 0,
		/** Flat executable strings located from a known first-line anchor. */
		kExecutableAnchorSequence,
		/** Executable strings reached through a sequential pointer stream. */
		kExecutablePointerStream,
		/**
		 * Credits from a Macintosh 68K executable's RLE-compressed resource-fork DATA 0.
		 * Big-endian A5-relative offsets address strings in the expanded globals.
		 * Classic Motorola 68k Mac applications use the A5 address register as their global-data base
		 * (the "A5 world"): an offset denotes the address A5 + offset.
		 *
		 * For executables with compressed resource-fork DATA 0, expandMac68kResForkRleGlobals() restores
		 * the globals using the A5-world sizes from CODE 0 before reading the strings.
		 * Its output starts at the original A5 position, so A5 + offset becomes data[offset].
		 * These offsets address expanded globals, not raw resource-fork bytes.
		 * This expansion handles initialized globals in DATA 0; sfnt and STRL resources use their own loaders.
		 *
		 * @see https://developer.apple.com/library/archive/documentation/mac/pdf/Processes/Processes_GL.pdf
		 */
		kMac68kA5RelativePointerStream,
		/**
		 * Credits from a Macintosh PowerPC (PPC) PEF executable, using BE32 offsets into its expanded data section.
		 * Retain TLC leading-space and icon-marker conventions.
		 */
		kMacPpcPefRelativePointerStream,
		/** Flat strings loaded sequentially from TEXTSTR.MHK STRL resources. */
		kTextStrResourceSequence,
	};

	/** Release-specific bounds for one executable-owned credit source. */
	struct CreditLimits {
		/** Maximum number of source entries consumed through the terminator. */
		uint32 entryCount;
		/** Maximum byte length of one NUL-terminated string, excluding the NUL. */
		uint32 lineByteCount;
		/** Maximum aggregate string bytes, excluding NULs and blank pointers. */
		uint32 dataByteCount;
	};

	/**
	 * Internal source and decoding metadata for executable text.
	 * Keyed UI strings use fixed byte ranges in the executable or expanded Macintosh globals.
	 */
	struct ExeTextSource {
		/** Loose executable file name when the text comes from a standalone EXE. */
		const char *fileName = nullptr;
		/** InstallShield v3 .Z archive file name containing the executable image. */
		const char *archiveName = nullptr;
		/** Member name inside the archive that holds the executable image. */
		const char *archiveMemberName = nullptr;
		/** Whether @ref archiveName identifies an InstallShield cabinet instead of a v3 .Z archive. */
		bool archiveIsCabinet = false;
		/** Exact byte size of the release-matched executable image. */
		uint32 expectedSize = 0;
		/** Primary image to prepare for required-byte checks and executable credits. */
		ExeTextDataSource primaryDataSource = ExeTextDataSource::kRawExecutableFile;
		/** Selected data-section index, used only with @ref ExeTextDataSource::kExpandedMacPpcPefDataSection. */
		uint16 macPpcPefDataSection = 0;
		/** Expected PEF totalSize, checked before allocation when preparing the expanded PPC data section. */
		uint32 macPpcPefDataSize = 0;

		/** Code page used to decode the keyed executable strings. */
		Common::CodePage codePage = Common::kWindows1252;
		/** Keyed string tables in decoding order, each selecting its own byte image. */
		Common::Array<ExeTextTable> textTables;
		/** Required ASCII signature bytes used to validate the executable identity. */
		const char *requiredBytes = nullptr;
		/** Offset of @ref requiredBytes within the executable image. */
		uint32 requiredBytesOffset = 0;

		/** Optional patch callback applied to keyed strings after decoding. */
		ExeTextPatchFunction textPatches = nullptr;
		/** Optional patch callback applied to credit lines after decoding. */
		ExeTextPatchFunction creditLinePatches = nullptr;
		/** Whether a MacRoman 0xAA (™) byte must be escaped during decoding. */
		bool escapeMacRomanTrademarkByteAA = false;
		/** Whether executable strings retain a Windows-1252 0xD8 (Ø) byte. */
		bool decodeWindows1252StrokeByteD8 = false;

		/** Storage and traversal method owning the credit sequence. */
		CreditSourceKind creditSourceKind = CreditSourceKind::kNone;
		/** Known first line locating an anchor-based credit string sequence. */
		const char *creditAnchor = nullptr;
		/** Pointer-table offset in the source image; A5-relative for expanded Macintosh globals. */
		uint32 creditPointerTableOffset = 0;
		/** Executable image base address used to translate pointer-table values. */
		uint32 creditPointerBaseAddress = 0;
		/** First pointer-table index consumed by the credits renderer. */
		uint32 creditPointerFirstIndex = 0;
		/** First pointer value in the range marking blank credit entries. */
		uint32 creditPointerBlankAddressFirst = 0;
		/** Last pointer value in the range marking blank credit entries. */
		uint32 creditPointerBlankAddressLast = 0;
		/** Bounds measured from this release's supported executable credit source. */
		CreditLimits creditLimits = {0, 0, 0};

		/** Copy these decoding settings to a candidate in a loose executable file. */
		ExeTextSource inFile(const char *srcFileName, uint32 srcExpectedSize) const;
		/** Select RLE-compressed DATA 0 from a Macintosh 68K executable's resource fork for expansion before text lookup. */
		ExeTextSource inMac68kDataResFork(const char *srcFileName, uint32 srcExpectedSize) const;
		/** Read a Macintosh PPC PEF executable's data fork and expand its selected data section before text lookup. */
		ExeTextSource inMacPpcPefDataFork(const char *srcFileName, uint32 srcExpectedSize, uint16 sectionIndex, uint32 sectionSize) const;
		/** Copy these decoding settings to a candidate in an InstallShield v3 .Z archive. */
		ExeTextSource inArchiveMember(const char *srcArchiveName, const char *srcMemberName, uint32 srcExpectedSize) const;
		/** Copy these decoding settings to a candidate in an InstallShield cabinet. */
		ExeTextSource inCabinetMember(const char *srcArchiveName, const char *srcMemberName, uint32 srcExpectedSize) const;

		/** Set the code page and first static keyed string table by pointer and entry count; file offsets are the default origin. */
		ExeTextSource withTextTable(Common::CodePage srcCodePage, const ExeTextEntry *srcEntries, uint32 srcEntryCount, ExeTextDataSource srcDataSource = ExeTextDataSource::kRawExecutableFile);
		/**
		 * Append a static keyed string table by pointer and entry count, sharing this source's code page.
		 *
		 * @remarks Unlike withTextTable(), this preserves previously registered tables and the code page.
		 * The PPC PEF source needs both expanded-data strings and raw-file code literals such as "Level";
		 * calling withTextTable() again would discard the first table.
		 * @see withTextTable()
		 */
		ExeTextSource withAdditionalTextTable(const ExeTextEntry *srcEntries, uint32 srcEntryCount, ExeTextDataSource srcDataSource = ExeTextDataSource::kRawExecutableFile);
		/** Set the required-bytes identity check, returning the updated descriptor. */
		ExeTextSource withRequiredBytes(uint32 srcOffset, const char *srcBytes);
		/** Set the keyed-string patch callback, returning the updated descriptor. */
		ExeTextSource withTextPatches(ExeTextPatchFunction patch);
		/** Set the credit-line patch callback, returning the updated descriptor. */
		ExeTextSource withCreditLinePatches(ExeTextPatchFunction patch);
		/** Enable the MacRoman trademark-byte (™) escape, returning the updated descriptor. */
		ExeTextSource withMacRomanTrademarkByteAAEscape();
		/** Enable the Windows-1252 0xD8 (Ø) byte quirk, returning the updated descriptor. */
		ExeTextSource withWindows1252StrokeByteD8Decode();
		/** Set the anchor-based credit lookup, returning the updated descriptor. */
		ExeTextSource withCreditAnchor(const char *anchor);

		/** Set the pointer-stream credit lookup and first consumed index, returning the updated descriptor. */
		ExeTextSource withCreditPtrTable(uint32 tableOffset, uint32 baseAddress, uint32 firstIndex);
		/** Select credits whose table offset and BE32 entries are relative to the original 68k A5 register value. */
		ExeTextSource withMacA5RelativeCreditPtrTable(uint32 tableOffset);
		/** Select BE32 credit pointers relative to the start of the expanded PEF data section. */
		ExeTextSource withMacPefRelativeCreditPtrTable(uint32 tableOffset);
		/** Set the blank-entry pointer value, returning the updated descriptor. */
		ExeTextSource withCreditPtrBlankAddress(uint32 address);
		/** Set the inclusive blank-entry pointer range, returning the updated descriptor. */
		ExeTextSource withCreditPtrBlankAddressRange(uint32 firstAddress, uint32 lastAddress);
		/** Set the release-specific executable credit bounds, returning the updated descriptor. */
		ExeTextSource withCreditLimits(const CreditLimits &limits);
	};

	/** Release-selected text and font sources; defaults select no executable, archive or font cabinet. */
	struct SourceSettings {
		/** Preferred Windows font cabinet, or null when no cabinet is selected. */
		const char *fontCabinet = nullptr;
		/** Executable candidates in search order; the first fully valid candidate wins. */
		Common::Array<ExeTextSource> executables;
		/** Load TEXTSTR.MHK after executable and ScummVM-added strings. */
		bool textStrArchive = false;
		/** Exact TEXTSTR.MHK credit entry count, or zero to retain executable credits. */
		uint32 textStrCreditEntryCount = 0;
		/** Whether archive credits retain a MacRoman 0x9F (ü) byte. */
		bool decodeMacRomanUmlautByte9F = false;
	};

	/** One localized replacement and its release applicability. */
	struct StringOverride {
		/** Replacement text; null only in a default-constructed map value. */
		const char32_t *text;
		/** Keep the executable's text when running v2.0US, v2.0PL, or v2.0US Demo. */
		bool preserveTlcText;

		/** Create a replacement, or an empty value for HashMap initialization. */
		constexpr StringOverride(const char32_t *replacementText = nullptr, bool keepTlcText = false)
			: text(replacementText), preserveTlcText(keepTlcText) {}
	};
	/** Available translations for one text key, indexed by language. */
	typedef Common::HashMap<Common::Language, StringOverride, Common::Hash<int>> LanguageStringOverrides;
	/** Localized replacements indexed first by text key, then by language. */
	typedef Common::HashMap<Key, LanguageStringOverrides, Common::Hash<uint32>> StringOverrideMap;

	/** Bounds measured from the Spanish v1.0 executable credit source. */
	static const CreditLimits kV10ESCreditLimits;
	/** Equal bounds measured from the USv1.0 Windows and Macintosh credit sources. */
	static const CreditLimits kV10USCreditLimits;
	/** Bounds measured from the Portuguese v1.0 executable credit source. */
	static const CreditLimits kV10PTCreditLimits;
	/** Equal bounds measured from the US v1.1 16-bit and 32-bit credit sources. */
	static const CreditLimits kV11USCreditLimits;
	/** Equal bounds measured from the Korean v1.11 16-bit and 32-bit credit sources. */
	static const CreditLimits kV111KRCreditLimits;
	/** Bounds measured from the US v2.0 retail executable credit source. */
	static const CreditLimits kV20USCreditLimits;
	/** Bounds measured from the Polish v2.0 retail executable credit source. */
	static const CreditLimits kV20PLCreditLimits;
	/** Exact entry count of the Italian v1.0 STRL 2020 and 2021 credit source. */
	static constexpr uint32 kEuropeITCreditEntryCount = 278;
	/** Exact entry count of the Japanese v1.0 STRL 2020 and 2021 credit source. */
	static constexpr uint32 kEuropeJPCreditEntryCount = 296;
	/** Exact entry count of British v1.0/v1.1 and v1.0 demo STRL 2020/2021 credits. */
	static constexpr uint32 kEuropeBRCreditEntryCount = 282;
	/** Exact entry count of French v1.0/v1.1, including 2002, STRL 2020/2021 credits. */
	static constexpr uint32 kEuropeFRCreditEntryCount = 285;
	/** Exact entry count of German v1.0/v1.1 STRL 2020/2021 credits. */
	static constexpr uint32 kEuropeGECreditEntryCount = 291;
	/** Anchor for flat v1.x executable credit string sequences. */
	static constexpr const char *const kExeCreditParagraphsAnchor = "PRODUCT CONCEPT AND DESIGN";
	/** Anchor for the Spanish v1.0 executable credit string sequence. */
	static constexpr const char *const kExeSpanishCreditParagraphsAnchor = "CONCEPTO Y DISE\xD1O";
	/** Anchor for the Portuguese v1.0 executable credit string sequence. */
	static constexpr const char *const kExePortugueseCreditParagraphsAnchor = "CRIA\xC7\xC3O E DESIGN DO PRODUTO";
	/** Europe TEXTSTR.MHK STRL key-resource-index map. */
	static const ResTextEntry kV10EU_ResTextEntries[];
	/** v1.0BR NE Key-address-size map. */
	static const ExeTextEntry kV10BR_NETextEntries[];
	/** v1.0BR Win16 demo Key-address-size map. */
	static const ExeTextEntry kV10BRDemo_NETextEntries[];
	/** v1.1BR NE Key-address-size map. */
	static const ExeTextEntry kV11BR_NETextEntries[];
	/** v1.0FR NE Key-address-size map. */
	static const ExeTextEntry kV10FR_NETextEntries[];
	/** v1.1FR NE Key-address-size map. */
	static const ExeTextEntry kV11FR_NETextEntries[];
	/** v1.0GE NE Key-address-size map. */
	static const ExeTextEntry kV10GE_NETextEntries[];
	/** v1.1GE NE Key-address-size map. */
	static const ExeTextEntry kV11GE_NETextEntries[];
	/** v1.0ES PE32 Key-address-size map. */
	static const ExeTextEntry kV10ES_PETextEntries[];
	/** v1.0IT NE Key-address-size map. */
	static const ExeTextEntry kV10IT_NETextEntries[];
	/** v1.0JP PE32 default-game name and name-mode notifications. */
	static const ExeTextEntry kV10JP_PETextEntries[];
	/** Japanese Macintosh v1.0 demo strings in the original MacBinary executable. */
	static const ExeTextEntry kV10JPMacDemoTextEntries[];
	/** v1.0PT PE32 Key-address-size map. */
	static const ExeTextEntry kV10PT_PETextEntries[];
	/** v1.1US PE32 Key-address-size map. */
	static const ExeTextEntry kV11US_PETextEntries[];
	/** v1.1US NE Key-address-size map. */
	static const ExeTextEntry kV11US_NETextEntries[];
	/** USv1.0 Windows NE retail executable text, indexed by raw file offset. */
	static const ExeTextEntry kV10US_NETextEntries[];
	/** USv1.0 Windows PE32 retail executable text, indexed by raw file offset. */
	static const ExeTextEntry kV10US_PETextEntries[];
	/** v1.0US Win16 demo Key-address-size map. */
	static const ExeTextEntry kV10USDemo_NETextEntries[];
	/** v1.0US Macintosh demo strings at A5-relative offsets in the expanded resource-fork DATA 0. */
	static const ExeTextEntry kV10USMacDemoTextEntries[];
	/** USv1.0 Macintosh 68K retail text at A5-relative offsets in the expanded resource-fork DATA 0. */
	static const ExeTextEntry kV10USMacTextEntries[];
	/** US v1.1 Macintosh retail text, relative to the expanded A5 globals. */
	static const ExeTextEntry kV11USMacTextEntries[];
	/** v1.0 British Macintosh 68K retail text at A5-relative offsets in expanded DATA 0. */
	static const ExeTextEntry kV10BRMacTextEntries[];
	/** v1.0 French Macintosh 68K retail text at A5-relative offsets in expanded DATA 0. */
	static const ExeTextEntry kV10FRMacTextEntries[];
	/** v1.0 German Macintosh 68K retail text at A5-relative offsets in expanded DATA 0. */
	static const ExeTextEntry kV10GEMacTextEntries[];
	/** v1.1 British Macintosh 68K retail text at A5-relative offsets in expanded DATA 0. */
	static const ExeTextEntry kV11BRMacTextEntries[];
	/** v1.1 French Macintosh 68K retail text at A5-relative offsets in expanded DATA 0. */
	static const ExeTextEntry kV11FRMacTextEntries[];
	/** v1.1 German Macintosh 68K retail text at A5-relative offsets in expanded DATA 0. */
	static const ExeTextEntry kV11GEMacTextEntries[];
	/** v1.0 Japanese Macintosh 68K retail text at A5-relative offsets in expanded DATA 0. */
	static const ExeTextEntry kV10JPMacTextEntries[];
	/** USv2.0 Macintosh PPC PEF text in the expanded data section. */
	static const ExeTextEntry kV20USMacTextEntries[];
	/** USv2.0 Macintosh PPC code literals at raw executable-file offsets. */
	static const ExeTextEntry kV20USMacRawTextEntries[];
	/** v1.11KR PE32 Key-address-size map. */
	static const ExeTextEntry kV111KR_PETextEntries[];
	/** v1.11KR NE Key-address-size map. */
	static const ExeTextEntry kV111KR_NETextEntries[];
	/** v2.0US PE32 Key-address-size map. */
	static const ExeTextEntry kV20US_PETextEntries[];
	/** v2.0PL PE32 Key-address-size map. */
	static const ExeTextEntry kV20PL_PETextEntries[];
	/** v2.0US (demo) PE32 Key-address-size map. */
	static const ExeTextEntry kV20USDemo_PETextEntries[];

	/** Sort localized strings by numeric key. */
	static bool compareLocalizedStrings(const LocalizedString &left, const LocalizedString &right);
	/** Return whether an ASCII byte sequence matches at an offset. */
	static bool bytesMatchAt(const Common::Array<byte> &data, uint32 offset, const char *bytes);
	/** Return whether the string contains exactly one high byte, matching @ref expectedByte. */
	template<typename T>
	static bool hasSingleExtendedByte(const T *bytes, uint32 length, byte expectedByte);
	/** Select CP1252 for the isolated 0xD8 (Ø) byte retained in Korean executable credits. */
	static Common::CodePage selectExeCodePage(const byte *bytes, uint32 length, Common::CodePage codePage);
	/** Select MacRoman for the isolated 0x9F (ü) byte retained in Japanese archive credits. */
	static Common::CodePage selectCreditCodePage(const Common::String &text, Common::CodePage codePage);
	/** Decode the STRL body with the optional source-specific MacRoman 0x9F (ü) quirk. */
	static bool readStrlBody(Common::SeekableReadStream *stream, uint16 stringCount, Common::CodePage codePage, Common::Array<Common::U32String> &texts, bool decodeMacRomanUmlautByte9F = false);
	/** Decode a TextSTR STRL resource using its source code page and optional MacRoman 0x9F (ü) quirk. */
	bool readTextStrStrings(MohawkArchive &archive, int16 resourceId, Common::CodePage codePage, Common::Array<Common::U32String> &texts, bool decodeMacRomanUmlautByte9F = false);
	/** Read one NUL-terminated STRL string while reserving one terminator byte for every following string. */
	static bool readBoundedStrlString(Common::SeekableReadStream *stream, Common::String &text, uint16 remainingStringCount);
	/** Configure release-specific font sources and load the normal and title fonts. */
	void initFonts(const SourceSettings &sources);
	/** Select Macintosh font sources, fallback candidates and release-matched point sizes. */
	void initMacintoshFonts();
	/** Select Windows font sources, fallback candidates and release-specific sizing. */
	void initWindowsFonts(const char *fontCabinet);
	/** Describe the verified retail Macintosh executable's static text image. */
	static ExeTextSource createMacExecutableSource(const ZmbReleaseInfo &relInfo, bool europeV11);
	/** Select and cache one font role, optionally warning when a fallback is used. */
	const Graphics::Font *loadFont(FontState &state, bool showWarnMsgBox);

	/**
	 * Insert or replace ScummVM-provided strings while preserving release-owned text.
	 * Look up the selected language, falling back to English when no applicable translation exists.
	 */
	void applyStringOverrides(const ZmbReleaseInfo &relInfo);
	/** Build the key-to-language replacement map used by @ref ZoombiniText::applyStringOverrides. */
	static StringOverrideMap createStringOverrides();
	/** Consume detector layout/package distinctions once to select text and font sources without reading files. */
	static SourceSettings createSourceSettings(const ZmbReleaseInfo &relInfo, uint32 detectedFeatures);
	/** Load mapped TEXTSTR.MHK strings and archive credits using the selected source settings. */
	bool initArchiveStrings(const SourceSettings &sources);
	/** Try the selected executable candidates in order. */
	bool initExecutableStrings(const Common::Array<ExeTextSource> &sources);
	/** Validate and decode one candidate, committing strings and credits only on success. */
	bool tryExecutableSource(const ExeTextSource &source);
	/** Read a release-matched executable image of the exact configured size into memory. */
	static bool readExecutableData(Common::SeekableReadStream *exeStream, uint32 expectedSize, Common::Array<byte> &data);
	/** Read a Macintosh 68K executable's CODE 0 header: BE32 bytes above A5, then below A5. The caller owns the stream. */
	static bool readMac68kA5WorldSizes(Common::SeekableReadStream *codeStream, uint32 &bytesAboveA5, uint32 &bytesBelowA5);
	/**
	 * Command codes and run-command range bases in a Macintosh 68K executable's enhanced-RLE-compressed resource-fork DATA 0.
	 * Each byte selects one command, not a combination of flags. Codes 1..4 select templates; 5..15 are invalid.
	 * Run commands occupy ranges 16..31, 32..63, 64..127 and 128..255. Select the highest matching range base.
	 * The count is code - base + 1, except RepeatByte, which uses code - base + 2.
	 */
	enum Mac68kDataCommandCode : uint8 {
		/** End the current block. */
		kMac68kDataEndBlock_00 = 0,
		/** First of four eight-byte templates, indexed by code - TemplateFirst. */
		kMac68kDataTemplateFirst_01 = 1,
		/** Preserve four bytes, emit 255, then copy three literal bytes. */
		kMac68kDataTemplate_02 = 2,
		/** Emit 169, 240; preserve two bytes; copy two literals; preserve one byte; copy one literal. */
		kMac68kDataTemplate_03 = 3,
		/** Last template command, inclusive. */
		kMac68kDataTemplateLast_04 = 4,
		/** Codes 16..31 emit 1..16 copies of byte 255. */
		kMac68kDataRepeatFFBase_16 = 16,
		/** Codes 32..63 consume one source byte and repeat it 2..33 times. */
		kMac68kDataRepeatByteBase_32 = 32,
		/** Codes 64..127 preserve/skip 1..64 destination bytes without a source payload. */
		kMac68kDataSkipBase_64 = 64,
		/** Codes 128..255 copy the following 1..128 literal bytes. */
		kMac68kDataCopyLiteralsBase_128 = 128,
	};

	/**
	 * Expand enhanced-RLE-compressed globals from a Macintosh 68K executable's resource-fork DATA 0.
	 * This is the 68K CODE-resource startup format; PPC PEF data sections use expandMacPpcPefDataSection().
	 * Required for text lookup: offsets are relative to the global-data base in the original 68K A5 register.
	 * Keep the bytes at/above A5 with A5 mapped to index 0, so those offsets index data directly.
	 *
	 * The caller reads bytesAboveA5 and bytesBelowA5 from CODE 0 with readMac68kA5WorldSizes(), then
	 * gets the DATA 0 payload stream from MacResManager::getResource() and copies its compressed bytes
	 * into data with readExecutableData(). The sizes describe the executable's memory layout, not the RLE format.
	 *
	 * @remarks
	 * CODE 0 header used by the caller:
	 *   Bytes 0..3: BE32 bytes above A5.
	 *   Bytes 4..7: BE32 bytes below A5.
	 *
	 * DATA 0 input layout:
	 *   Bytes 0..3: prefix skipped by the original startup.
	 *   From byte 4: three blocks, each [signed BE32 offset from A5][RLE commands][end command 0].
	 *   After the third block: relocation streams, which are not parsed here.
	 * Commands may consume literal payload bytes; only a zero read as a command ends a block.
	 *
	 * A5 is the original 68K global-data base.
	 * Decode into a zero-filled temporary image with bytesBelowA5 bytes below A5 and bytesAboveA5 above it;
	 * a block's signed offset n starts at a5World[bytesBelowA5 + n].
	 * Literal copies, repeated-byte runs, skips and eight-byte templates reconstruct the globals in that image.
	 * On success, replace data with bytesAboveA5 bytes at/above A5, mapping A5 + offset to data[offset].
	 * Keyed strings and BE32 credit pointers can then use their A5-relative offsets directly,
	 * without applying the original runtime's address relocations or executing its startup code.
	 *
	 * Related references:
	 * - Apple, Building and Managing Programs in MPW, 2nd ed., p. 8-16: compressed A5-global initialization in MPW.
	 *   https://oldapplestuff.com/download/Macintosh/Macintosh_Garden/manuals/Building_and_Managing_Programs_in_MPW_2ed.pdf#page=182
	 * - Theodore Ts'o, The PRC Format, "The Data Resource": related Palm 68k enhanced-RLE layout and command table.
	 *   https://web.mit.edu/~tytso/www/pilot/prc-format.html
	 * These describe related initialization formats; they do not identify an executable's compiler or runtime version.
	 * Unlike the Palm description's zero-fill, this decoder preserves skipped bytes and template holes, matching the verified Mac startup.
	 */
	static bool expandMac68kResForkRleGlobals(Common::Array<byte> &data, uint32 bytesAboveA5, uint32 bytesBelowA5);
	/**
	 * Supported data-section kind codes in a PPC PEF section header, using Apple's section type names.
	 * @see Mac OS Runtime Architectures, chapter 8, Table 8-1, pp. 8-8..8-9:
	 * https://developer.apple.com/library/archive/documentation/mac/pdf/MacOS_RT_Architectures.pdf#page=170
	 */
	enum MacPpcPefSectionKind : uint8 {
		/** Uncompressed initialized read/write data, followed by zero-initialized data. */
		kPefSectionUnpackedData_01 = 1,
		/** Read/write data constructed by pattern-initialization instructions. */
		kPefSectionPatternInitData_02 = 2
	};
	/** Opcodes in the high three bits of a PPC PEF pattern-initialization instruction (Apple Runtime Architectures, chapter 8). */
	enum MacPpcPefDataOpcode : uint8 {
		/** Emit a run of zero bytes. */
		kPefOpcodeZero_00 = 0,
		/** Copy a literal byte block. */
		kPefOpcodeBlockCopy_01 = 1,
		/** Repeat one literal byte block. */
		kPefOpcodeRepeatedBlock_02 = 2,
		/** Alternate a shared literal block with successive custom literal blocks. */
		kPefOpcodeInterleaveBlocks_03 = 3,
		/** Alternate zero-filled runs with successive custom literal blocks. */
		kPefOpcodeInterleaveZero_04 = 4
	};
	/**
	 * Read one PPC PEF pattern argument: up to five big-endian groups of seven bits, with bit 7 marking continuation.
	 * Used for extended instruction counts and additional pattern operands; advance sourceOffset within the caller-validated sourceEnd.
	 * @see Apple, Mac OS Runtime Architectures, chapter 8, pp. 8-11..8-12:
	 * https://developer.apple.com/library/archive/documentation/mac/pdf/MacOS_RT_Architectures.pdf#page=172
	 */
	static bool readMacPpcPefDataArgument(const Common::Array<byte> &data, uint32 sourceEnd, uint32 &sourceOffset, uint32 &value);
	/**
	 * Expand one data section from a Macintosh PPC PEF data fork to section-relative bytes.
	 * Reproduce the Code Fragment Manager's literal/pattern initialization and zero-filled tail for static text lookup.
	 * Validate the PowerPC ('pwpc') container architecture and leave CFM address relocations unapplied.
	 * @param data Read-only PEF container bytes.
	 * @param sectionIdx Index of the instantiated data section to expand.
	 * @param expectedSize Required expanded section size for the selected release.
	 * @param[out] expandedData Receives the section image on success; unchanged on failure.
	 */
	static bool expandMacPpcPefDataSection(const Common::Array<byte> &data, uint16 sectionIdx, uint32 expectedSize, Common::Array<byte> &expandedData);
	/** Find an ASCII byte sequence in executable data. */
	static bool findBytes(const Common::Array<byte> &data, const char *needle, uint32 &offset);
	/** Decode an executable byte string through its source code page. */
	static Common::U32String decodeExecutableStringBytes(const byte *bytes, uint32 length, const ExeTextSource &source);
	/** Read and decode a bounded executable string at an offset. */
	static bool readExecutableStringAt(const Common::Array<byte> &data, uint32 offset, const ExeTextSource &source, Common::U32String &text, uint32 &byteLength);
	/** Return whether all release-specific executable credit bounds are present. */
	static bool areExecutableCreditLimitsValid(const ExeTextSource &source);
	/** Return whether text terminates an executable credit string table. */
	static bool isCreditTerminator(const Common::U32String &text);
	/** Return whether text is a TLC credit logo marker. */
	static bool isTlcCreditIconMarker(const Common::U32String &text);
	/** Read executable credits located after an anchor string. */
	static bool readCreditStringsFromAnchor(const Common::Array<byte> &data, const ExeTextSource &source, Common::Array<Common::U32String> &creditStrings);
	/** Read executable credits through a pointer table. */
	static bool readCreditStringsFromPointerTable(const Common::Array<byte> &data, const ExeTextSource &source, Common::Array<Common::U32String> &creditStrings);
	/** Build display paragraphs from decoded credit lines. */
	static bool buildCreditParagraphsFromStrings(const Common::Array<Common::U32String> &creditStrings, Common::Array<CreditParagraph> &creditParagraphs, CreditSourceKind kind, uint32 entryCountLimit);
	/** Load and assemble original executable credits. */
	static bool loadOriginalExecutableCredits(const Common::Array<byte> &data, const ExeTextSource &source, Common::Array<CreditParagraph> &creditParagraphs);
	/** Correct the Spanish 1.0 executable text directly in the localized string map. */
	void patchSpanish10ExeText();
	/** Correct the Korean executable credit directly in the loaded paragraphs. */
	void patchKoreanExeCredits();

	/** Engine instance that owns resource and executable text access. */
	MohawkEngine_Zoombini *_vm;
	/** Engine-owned immutable release identity, which outlives this text service. */
	const ZmbReleaseInfo &_relInfo;
	/** Session-wide Japanese procedural-name selection, toggled with Ctrl+M. */
	bool _useJapaneseNames = true;
	/** Cached code page used to decode Zoombini executable strings (EXE). */
	Common::CodePage _exeCodePage;
	/** Cached code page used to decode Zoombini string resources (STRL). */
	Common::CodePage _resCodePage;

	/** Selected normal text font. */
	FontState _textFont;
	/** Selected title font. */
	FontState _titleFont;
	/** Font loaders tried first for the selected language and point size. */
	Common::Array<TTFLoader *> _optimalTTFLoaders;
	/** Fallback font loaders tried when no preferred font can be loaded. */
	Common::Array<TTFLoader *> _fallbackTTFLoaders;

	/** Forward canonical-ID cache for the STRL Zoombini-name pool. */
	Common::HashMap<int16, Common::U32String> _nameCache;

	/** Localized strings keyed by numeric text ID. */
	Common::HashMap<uint32, Common::U32String> _strMap;
	/** Localized credits in display order. */
	Common::Array<CreditParagraph> _creditParagraphs;
};

} // End of namespace Mohawk

#endif
