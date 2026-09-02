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

#include "common/hash-str.h"
#include "common/hashmap.h"
#include "common/language.h"
#include "common/scummsys.h"
#include "common/str-enc.h"
#include "graphics/fontman.h"

#include "mohawk/resource.h"

namespace Common {

class SeekableReadStream;

} // End of namespace Common

namespace Mohawk {

class TTFLoader;
class MohawkEngine_Zoombini;

/** Extract the localized hard-coded strings from the Zoombinis executable, and provide access to them by key. */
class ZoombiniText {
public:
	struct CreditLineAddress {
		/** Source paragraph index in the extracted credit list. */
		int groupIndex = -1;
		/** Line index within the source paragraph. */
		int inGroupLineIndex = -1;

		CreditLineAddress() {}
		CreditLineAddress(int sourceGroupIndex, int sourceInGroupLineIndex) : groupIndex(sourceGroupIndex), inGroupLineIndex(sourceInGroupLineIndex) {}

		bool isValid() const {
			return 0 <= groupIndex && 0 <= inGroupLineIndex;
		}
	};

	/** Load localized text sources for the selected engine language. */
	ZoombiniText(MohawkEngine_Zoombini *vm, Common::Language lang);
	/** Release loaded fonts, strings, and name caches. */
	~ZoombiniText();

	/** Tokenize a @ref Common::String by CRLF, CR, or LF. */
	static Common::Array<Common::String> tokenizeLines(const Common::String &text);
	/** Tokenize a @ref Common::U32String by CRLF, CR, or LF. */
	static Common::Array<Common::U32String> tokenizeLines(const Common::U32String &text);
	/** Format a credit address as a stable patch key. */
	static Common::String formatCreditLineKey(const CreditLineAddress &address);
	/** Format paragraph and line indices as a stable patch key. */
	static Common::String formatCreditLineKey(uint32 paragraphIndex, uint32 lineIndex);
	/** Parse a stable credit key into a line address. */
	static bool parseCreditLineKey(const Common::String &creditKey, CreditLineAddress &address);
	/** Parse a stable credit key into paragraph and line indices. */
	static bool parseCreditLineKey(const Common::String &creditKey, uint32 &paragraphIndex, uint32 &lineIndex);

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
		kEuropeJourneyStatZoombiniIsle,
		kEuropeJourneyStatShelterRock,
		kEuropeJourneyStatShadeTree,
		kEuropeJourneyStatZoombiniton,
		// RODMAP: (European-only) Practice return-path descriptions.
		// STRL 2002 has four progress-dependent four-line blocks (5-8, 9-12, 13-16, and 17-20).
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

	enum FontKind : uint32 {
		kFontDebugTitle = 0,
		kFontDebugText,
		kFontTitle,
		kFontText,
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

		/** Build a source descriptor for a loose executable file. */
		static ExeTextSource fromFile(const char *sourceFileName, uint32 sourceExpectedSize);
		/** Build a source descriptor for an executable member inside an InstallShield v3 .Z archive. */
		static ExeTextSource fromArchiveMember(const char *sourceArchiveName, const char *sourceArchiveMemberName, uint32 sourceExpectedSize);
		/** Build a source descriptor for an executable member inside an InstallShield cabinet. */
		static ExeTextSource fromCabinetMember(const char *sourceArchiveName, const char *sourceArchiveMemberName, uint32 sourceExpectedSize);

		/** Set the code page and keyed string table, returning the updated descriptor. */
		template<size_t size>
		ExeTextSource withTextTable(Common::CodePage sourceCodePage, const ExeTextEntry (&sourceEntries)[size]);
		/** Set the required-bytes identity check, returning the updated descriptor. */
		ExeTextSource withRequiredBytes(uint32 sourceRequiredBytesOffset, const char *sourceRequiredBytes);
		/** Set the keyed-string patch callback, returning the updated descriptor. */
		ExeTextSource withTextPatches(ExeTextPatchFunction sourceTextPatch);
		/** Set the credit-line patch callback, returning the updated descriptor. */
		ExeTextSource withCreditLinePatches(ExeTextPatchFunction sourceCreditLinePatch);
		/** Enable the MacRoman trademark-byte escape, returning the updated descriptor. */
		ExeTextSource withMacRomanTrademarkByteAAEscape();
		/** Set the anchor-based credit lookup, returning the updated descriptor. */
		ExeTextSource withCreditAnchor(const char *sourceCreditAnchor);

		/** Set the pointer-stream credit lookup and first consumed index, returning the updated descriptor. */
		ExeTextSource withCreditPointerTable(uint32 sourceCreditPointerTableOffset, uint32 sourceCreditPointerBaseAddress,
											 uint32 sourceCreditPointerFirstIndex);
		/** Set the blank-entry pointer value, returning the updated descriptor. */
		ExeTextSource withCreditPointerBlankAddress(uint32 sourceCreditPointerBlankAddress);
		/** Set the inclusive blank-entry pointer range, returning the updated descriptor. */
		ExeTextSource withCreditPointerBlankAddressRange(uint32 sourceCreditPointerBlankAddressFirst,
														 uint32 sourceCreditPointerBlankAddressLast);
		/** Set the release-specific executable credit bounds, returning the updated descriptor. */
		ExeTextSource withCreditLimits(const CreditLimits &sourceCreditLimits);
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
		CreditParagraph(const Common::Array<Common::U32String> &lines, uint32 blankLineCount, bool firstLineIsTitle = true) : _lines(lines), _blankLineCount(blankLineCount), _firstLineIsTitle(firstLineIsTitle) {}

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
		LocalizedString(uint32 key, const Common::U32String &text) : _key(key), _text(text) {}
	};

	/** Return the code page used by executable strings for a Zoombini language. */
	static Common::CodePage getExeCodePage(Common::Language language);
	/** Return the code page used by string resources for a Zoombini language. */
	static Common::CodePage getResCodePage(Common::Language language);
	/** Return the display name for a code page used by Zoombini text and save names. */
	static const char *getCodePageName(Common::CodePage codePage);
	/** Return the normal text font, loading it on first use. */
	const Graphics::Font *getTextFont();
	/** Return the title font, loading it on first use. */
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
	/**
	 * Load all 625 Zoombini names into the forward cache from snoid ID to name,
	 * then build the reverse index from name to snoid ID in one pass.
	 * Must be called before @ref ZoombiniText::findZoombiniNameId().
	 */
	void cacheAllZoombiniNames();
	/**
	 * Return the snoid ID from 0 through 624 for the given name, or -1 if it is not found.
	 * Requires @ref ZoombiniText::cacheAllZoombiniNames() to have been called first.
	 */
	int16 findZoombiniNameId(const Common::U32String &name) const;
	/** Clear forward and reverse Zoombini-name caches. */
	void clearNameCache();

	/**
	 * Pick the next name for a preview Zoombini.
	 * Korean: draws from the 625-slot STRL name pool,
	 * tracking used names in @ref ZoombiniGameState::_zoombiniNameGeneratedTable and rebuilding it when exhausted.
	 * English: generates a name procedurally from syllable tables.
	 */
	Common::U32String pickNextZoombiniName();

	/** Return the localized display name for a page type. */
	Common::U32String getPageName(ZoombiniPageType pageType) const;
	/** Return one localized string by numeric key. */
	Common::U32String getLocalizedString(uint32 textKey) const;
	/** Copy all localized strings in stable key order. */
	void getLocalizedStrings(Common::Array<LocalizedString> &strings) const;
	/** Copy localized credit paragraphs in display order. */
	void getLocalizedCredits(Common::Array<CreditParagraph> &paragraphs) const;

	/** Patch one named localized text entry with UTF-32 text. */
	bool patchLocalizedText(const Common::String &textKey, const Common::U32String &text);
	/** Patch one named localized text entry with UTF-8 text. */
	bool patchLocalizedText(const Common::String &textKey, const char *utf8Text);
	/** Apply a UTF-32 patch map to named localized strings. */
	bool patchLocalizedTexts(const Common::HashMap<Common::String, Common::U32String> &patches);
	/** Apply a UTF-8 patch map to named localized strings. */
	bool patchLocalizedTexts(const Common::HashMap<Common::String, Common::String> &patches);
	/** Replace one numeric localized string with UTF-32 text. */
	void patchLocalizedString(uint32 textKey, const Common::U32String &text);
	/** Replace one numeric localized string with UTF-8 text. */
	void patchLocalizedString(uint32 textKey, const char *utf8Text);
	/** Replace a credit line addressed by paragraph and line indices. */
	bool patchCreditLine(uint32 paragraphIndex, uint32 lineIndex, const Common::U32String &text);
	/** Replace a UTF-8 credit line addressed by paragraph and line indices. */
	bool patchCreditLine(uint32 paragraphIndex, uint32 lineIndex, const char *utf8Text);
	/** Replace a credit line addressed by a stable key. */
	bool patchCreditLine(const Common::String &creditKey, const Common::U32String &text);
	/** Replace a UTF-8 credit line addressed by a stable key. */
	bool patchCreditLine(const Common::String &creditKey, const char *utf8Text);
	/** Split a credit paragraph after one line and insert blank lines. */
	bool splitCreditParagraph(uint32 paragraphIndex, uint32 lineIndex, uint32 newParagraphBlankLineCount);
	/** Split a credit paragraph addressed by a stable line key. */
	bool splitCreditParagraph(const Common::String &creditKey, uint32 newParagraphBlankLineCount);
	/** Replace one complete credit paragraph. */
	bool patchCreditParagraph(uint32 paragraphIndex, const CreditParagraph &paragraph);
	/** Replace the complete localized credit list. */
	void patchLocalizedCredits(const Common::Array<CreditParagraph> &paragraphs);

private:
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
	/** Decode one TextSTR entry using a specified code page. */
	bool readTextStrString(MohawkArchive &archive, const ResTextEntry &entry, Common::CodePage codePage, Common::U32String &text);
	/** Decode every string in one TextSTR STRL resource using a specified code page. */
	bool readTextStrStrings(MohawkArchive &archive, int16 resourceId, Common::CodePage codePage, Common::Array<Common::U32String> &texts);
	/** Read one NUL-terminated STRL string while reserving one terminator byte for every following string. */
	static bool readBoundedStrlString(Common::SeekableReadStream *stream, Common::String &text, uint16 remainingStringCount);
	/** Engine instance that owns resource and executable text access. */
	MohawkEngine_Zoombini *_vm;
	/** Load and cache a font from preferred and fallback TTF loaders. */
	const Graphics::Font *loadFont(const Common::Array<TTFLoader *> &optimalTTFLoaders, const Common::Array<TTFLoader *> &fallbackTTFLoaders, int point, bool showWarnMsgBox, Common::String &cacheName, bool &usesAntialiasing);

	/** Build the page-type to localized-key lookup. */
	void initPageKeyMap();
	/** Initialize localized credits from the selected source family. */
	void initLocalizedCredits();
	/** Initialize English resource and fallback strings. */
	void initEnglishStrings();
	/** Load Europe-language strings from the original archive family. */
	bool initOriginalEuropeArchiveStrings();
	/** Load hard-coded strings from the original executable family. */
	bool initOriginalExecutableStrings();
	/** Apply one paragraph split patch. */
	static bool applyCreditParagraphSplit(Common::Array<CreditParagraph> &creditParagraphs, const CreditLineAddress &address, uint32 newParagraphBlankLineCount);
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
	static bool buildCreditParagraphsFromStrings(const Common::Array<Common::U32String> &creditStrings,
												 Common::Array<CreditParagraph> &creditParagraphs,
												 CreditSourceKind sourceKind, uint32 entryCountLimit);
	/** Load and assemble original executable credits. */
	static bool loadOriginalExecutableCredits(const Common::Array<byte> &data, const ExeTextSource &source, Common::Array<CreditParagraph> &creditParagraphs);
	/** Parse an unsigned decimal text value. */
	static bool parseUnsignedDecimalString(const Common::String &text, uint32 &value);
	/** Correct the Spanish 1.0 executable text directly in the localized string map. */
	void patchSpanish10ExeText();
	/** Correct the Korean executable credit directly in the loaded paragraphs. */
	void patchKoreanExeCredits();
	template<size_t size>
	static void addCreditParagraph(Common::Array<CreditParagraph> &paragraphs, const char *const (&creditLines)[size], uint32 blankLineCount) {
		Common::Array<Common::U32String> lines;
		for (uint i = 0; i < size; i++)
			lines.push_back(Common::U32String(creditLines[i], Common::kUtf8));
		paragraphs.push_back(CreditParagraph(lines, blankLineCount));
	}
	/** Initialize Korean resource and fallback strings. */
	void initKoreanStrings();

	/** Generate a localized procedural Zoombini name. */
	Common::U32String generateRandomName();

	/** Language selected for this engine instance. */
	Common::Language _lang;
	/** Cached code page used to decode Zoombini executable strings (EXE). */
	Common::CodePage _exeCodePage;
	/** Cached code page used to decode Zoombini string resources (STRL). */
	Common::CodePage _resCodePage;

	/** Cache key for the normal text font. */
	Common::String _textFontCacheName;
	/** Cache key for the title font. */
	Common::String _titleFontCacheName;
	/** Point size selected for the normal text font. */
	int _textFontPoint = 0;
	/** Point size selected for the title font. */
	int _titleFontPoint = 0;
	/** Whether the selected normal text font retains grayscale glyph coverage. */
	bool _textFontUsesAntialiasing = false;
	/** Whether the selected title font retains grayscale glyph coverage. */
	bool _titleFontUsesAntialiasing = false;
	/** Font loaders tried first for the selected language and point size. */
	Common::Array<TTFLoader *> _optimalTTFLoaders;
	/** Fallback font loaders used when the preferred font lacks a glyph. */
	Common::Array<TTFLoader *> _fallbackTTFLoaders;

	/** Forward canonical-ID to name cache. */
	Common::HashMap<int16, Common::U32String> _nameCache;
	/** Reverse name-to-canonical-ID cache. */
	Common::HashMap<Common::U32String, int16> _nameIndexCache;

	/** Localized strings keyed by numeric text ID. */
	Common::HashMap<uint32, Common::U32String> _strMap;
	/** Localized credits in display order. */
	Common::Array<CreditParagraph> _creditParagraphs;
	/** Page-type to localized page-name key map. */
	Common::HashMap<ZoombiniPageType, Key> _pageKeyMap;
};

} // End of namespace Mohawk

#endif
