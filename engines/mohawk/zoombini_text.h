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
	/** Load localized text sources for the selected engine language. */
	ZoombiniText(MohawkEngine_Zoombini *vm, Common::Language lang);
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
		// RODMAP: (European-only) Journey-stat location descriptions.
		// These are distinct from the page-name keys: STRL 2002 uses abbreviated stat labels,
		// while STRL 2000 contains the full page names (for example, French Shelter Rock).
		// v1.1US demo also use this layout.
		kEuropeJourneyStatZoombiniIsle,
		kEuropeJourneyStatShelterRock,
		kEuropeJourneyStatShadeTree,
		kEuropeJourneyStatZoombiniton,
		// RODMAP: (European-only) Practice return-path descriptions.
		// STRL 2002 has four progress-dependent four-line blocks (5-8, 9-12, 13-16, and 17-20).
		// v1.1US demo also use this layout.
		kEuropePracticeReturnInitial1,
		kEuropePracticeReturnInitial2,
		kEuropePracticeReturnInitial3,
		kEuropePracticeReturnInitial4,
		kEuropePracticeReturnShelterRock1,
		kEuropePracticeReturnShelterRock2,
		kEuropePracticeReturnShelterRock3,
		kEuropePracticeReturnShelterRock4,
		kEuropePracticeReturnShadeTree1,
		kEuropePracticeReturnShadeTree2,
		kEuropePracticeReturnShadeTree3,
		kEuropePracticeReturnShadeTree4,
		kEuropePracticeReturnZoombiniton1,
		kEuropePracticeReturnZoombiniton2,
		kEuropePracticeReturnZoombiniton3,
		kEuropePracticeReturnZoombiniton4,
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
		// ScummVM-only text keys
		kSvmDialogBodyAskSaveAfterDebugStateMutation = 2000,
		kSvmDialogBodyLoadNewerSaveFormat,
		kSvmDialogBodyCannotLoadSave,
		kSvmDialogBodyCannotSaveNameEncoding,
		kSvmDialogBodyAskConvertUnreadableZoombiniNames,
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

	/** Return the code page used by executable strings for a Zoombini language. */
	static Common::CodePage getExeCodePage(Common::Language lang);
	/** Return the code page used by string resources for a Zoombini language. */
	static Common::CodePage getResCodePage(Common::Language lang);
	/** Return the display name for a code page used by Zoombini text and save names. */
	static const char *getCodePageName(Common::CodePage codePage);
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
	/** Return the localized name for a canonical Zoombini ID. */
	Common::U32String getZoombiniName(int16 zmbid);

	// [*] Zoombini names
	/**
	 * Pick the next name for a preview Zoombini.
	 * Korean: draws each of the 625 STRL name IDs once per cycle, then clears the draw history.
	 * Different IDs may contain equal strings, and existing Zoombini names do not restrict selection.
	 * English: generates a name procedurally from syllable tables.
	 */
	Common::U32String pickNextZoombiniName();

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

	/** One Key-address-size entry in an executable text table. */
	struct ExeTextEntry {
		Key key;
		/** Raw file offset of the first text byte. */
		uint32 offset;
		/** Number of text bytes, excluding the trailing NUL terminator. */
		uint16 length;
	};

	typedef void (ZoombiniText::*ExeTextPatchFunction)();

	/** Storage and traversal method owning a release's credit sequence. */
	enum class CreditSourceKind {
		kNone = 0,
		/** Flat executable strings located from a known first-line anchor. */
		kExecutableAnchorSequence,
		/** Executable strings reached through a sequential pointer stream. */
		kExecutablePointerStream,
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
	 * Keyed UI strings are read from fixed byte ranges in the executable.
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

		/** Code page used to decode the keyed executable strings. */
		Common::CodePage codePage = Common::kWindows1252;
		/** Table of keyed string offsets and lengths within the executable image. */
		const ExeTextEntry *entries = nullptr;
		/** Number of entries in @ref entries. */
		uint entryCount = 0;
		/** Required ASCII signature bytes used to validate the executable identity. */
		const char *requiredBytes = nullptr;
		/** Offset of @ref requiredBytes within the executable image. */
		uint32 requiredBytesOffset = 0;

		/** Optional patch callback applied to keyed strings after decoding. */
		ExeTextPatchFunction textPatches = nullptr;
		/** Optional patch callback applied to credit lines after decoding. */
		ExeTextPatchFunction creditLinePatches = nullptr;
		/** Whether a Mac Roman 0xAA trademark byte must be escaped during decoding. */
		bool escapeMacRomanTrademarkByteAA = false;

		/** Storage and traversal method owning the credit sequence. */
		CreditSourceKind creditSourceKind = CreditSourceKind::kNone;
		/** Known first line locating an anchor-based credit string sequence. */
		const char *creditAnchor = nullptr;
		/** File offset of the address-to-string pointer table for pointer-based credits. */
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
		/** Copy these decoding settings to a candidate in an InstallShield v3 .Z archive. */
		ExeTextSource inArchiveMember(const char *srcArchiveName, const char *srcMemberName, uint32 srcExpectedSize) const;
		/** Copy these decoding settings to a candidate in an InstallShield cabinet. */
		ExeTextSource inCabinetMember(const char *srcArchiveName, const char *srcMemberName, uint32 srcExpectedSize) const;

		/** Set the code page and keyed string table, returning the updated descriptor. */
		template<size_t size>
		ExeTextSource withTextTable(Common::CodePage srcCodePage, const ExeTextEntry (&srcEntries)[size]);
		/** Set the required-bytes identity check, returning the updated descriptor. */
		ExeTextSource withRequiredBytes(uint32 srcOffset, const char *srcBytes);
		/** Set the keyed-string patch callback, returning the updated descriptor. */
		ExeTextSource withTextPatches(ExeTextPatchFunction patch);
		/** Set the credit-line patch callback, returning the updated descriptor. */
		ExeTextSource withCreditLinePatches(ExeTextPatchFunction patch);
		/** Enable the MacRoman trademark-byte escape, returning the updated descriptor. */
		ExeTextSource withMacRomanTrademarkByteAAEscape();
		/** Set the anchor-based credit lookup, returning the updated descriptor. */
		ExeTextSource withCreditAnchor(const char *anchor);

		/** Set the pointer-stream credit lookup and first consumed index, returning the updated descriptor. */
		ExeTextSource withCreditPtrTable(uint32 tableOffset, uint32 baseAddress, uint32 firstIndex);
		/** Set the blank-entry pointer value, returning the updated descriptor. */
		ExeTextSource withCreditPtrBlankAddress(uint32 address);
		/** Set the inclusive blank-entry pointer range, returning the updated descriptor. */
		ExeTextSource withCreditPtrBlankAddressRange(uint32 firstAddress, uint32 lastAddress);
		/** Set the release-specific executable credit bounds, returning the updated descriptor. */
		ExeTextSource withCreditLimits(const CreditLimits &limits);
	};

	/** Release-selected string sources; defaults select no executable or archive. */
	struct StringSources {
		/** Executable candidates in search order; the first fully valid candidate wins. */
		Common::Array<ExeTextSource> executables;
		/** Load TEXTSTR.MHK after executable and ScummVM-added strings. */
		bool textStrArchive = false;
		/** Exact TEXTSTR.MHK credit entry count, or zero to retain executable credits. */
		uint32 textStrCreditEntryCount = 0;
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
	static constexpr uint32 kV10ITCreditEntryCount = 278;
	/** Exact entry count of the British v1.0 demo STRL 2020 and 2021 credit source. */
	static constexpr uint32 kV10BRDemoCreditEntryCount = 282;
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
	/** v1.0ES NE Key-address-size map. */
	static const ExeTextEntry kV10ES_PETextEntries[];
	/** v1.0IT NE Key-address-size map. */
	static const ExeTextEntry kV10IT_NETextEntries[];
	/** v1.0PT PE32 Key-address-size map. */
	static const ExeTextEntry kV10PT_PETextEntries[];
	/** v1.1US PE32 Key-address-size map. */
	static const ExeTextEntry kV11US_PETextEntries[];
	/** v1.1US NE Key-address-size map. */
	static const ExeTextEntry kV11US_NETextEntries[];
	/** v1.1US Win16 demo Key-address-size map. */
	static const ExeTextEntry kV11USDemo_NETextEntries[];
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
	/** Decode the declared STRL body, leaving resource-specific trailing-byte checks to the caller. */
	static bool readStrlBody(Common::SeekableReadStream *stream, uint16 stringCount, Common::CodePage codePage, Common::Array<Common::U32String> &texts);
	/** Decode every string in one TextSTR STRL resource using a specified code page. */
	bool readTextStrStrings(MohawkArchive &archive, int16 resourceId, Common::CodePage codePage, Common::Array<Common::U32String> &texts);
	/** Read one NUL-terminated STRL string while reserving one terminator byte for every following string. */
	static bool readBoundedStrlString(Common::SeekableReadStream *stream, Common::String &text, uint16 remainingStringCount);
	/** Configure release-specific font sources and load the normal and title fonts. */
	void initFonts();
	/** Select and cache one font role, optionally warning when a fallback is used. */
	const Graphics::Font *loadFont(FontState &state, bool showWarnMsgBox);

	/**
	 * Insert or replace ScummVM-provided strings while preserving release-owned text.
	 * Look up the selected language, falling back to English when no applicable translation exists.
	 */
	void applyStringOverrides(const ZmbReleaseInfo &relInfo);
	/** Build the key-to-language replacement map used by @ref ZoombiniText::applyStringOverrides. */
	static StringOverrideMap createStringOverrides();
	/** Select executable candidates and TEXTSTR.MHK settings without reading files. */
	static StringSources createStringSources(const ZmbReleaseInfo &relInfo);
	/** Load mapped TEXTSTR.MHK strings and, for a nonzero entry count, archive credits. */
	bool initArchiveStrings(uint32 creditEntryCount);
	/** Try the selected executable candidates in order. */
	bool initExecutableStrings(const Common::Array<ExeTextSource> &sources);
	/** Validate and decode one candidate, committing strings and credits only on success. */
	bool tryExecutableSource(const ExeTextSource &source);
	/** Read a release-matched executable image of the exact configured size into memory. */
	static bool readExecutableData(Common::SeekableReadStream *exeStream, uint32 expectedSize, Common::Array<byte> &data);
	/** Find an ASCII byte sequence in executable data. */
	static bool findBytes(const Common::Array<byte> &data, const char *needle, uint32 &offset);
	/** Decode an executable byte string through its source code page. */
	static Common::U32String decodeExecutableStringBytes(const byte *bytes, uint32 length, const ExeTextSource &source);
	/** Read and decode a bounded executable string at an offset. */
	static bool readExecutableStringAt(const Common::Array<byte> &data, uint32 offset, const ExeTextSource &source,
									   Common::U32String &text, uint32 &byteLength);
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

	/** Generate a localized procedural Zoombini name. */
	Common::U32String generateRandomName();

	/** Engine instance that owns resource and executable text access. */
	MohawkEngine_Zoombini *_vm;
	/** Language selected for this engine instance. */
	Common::Language _lang;
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
