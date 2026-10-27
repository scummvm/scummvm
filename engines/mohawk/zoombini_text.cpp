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

#include "graphics/fontman.h"
#include "graphics/fonts/ttf.h"
#include "gui/message.h"

#include "common/algorithm.h"
#include "common/compression/installshield_cab.h"
#include "common/compression/installshieldv3_archive.h"
#include "common/config-manager.h"
#include "common/endian.h"
#include "common/file.h"
#include "common/keyboard.h"
#include "common/macresman.h"
#include "common/ptr.h"
#include "common/textconsole.h"
#include "common/translation.h"

#include "mohawk/resource.h"
#include "mohawk/ttfloader.h"
#include "mohawk/zoombini.h"
#include "mohawk/zoombini_page.h"
#include "mohawk/zoombini_random.h"
#include "mohawk/zoombini_state.h"
#include "mohawk/zoombini_text.h"

namespace Mohawk {

constexpr ZoombiniText::PageKeyEntry ZoombiniText::kPageKeyTable[];
constexpr char32_t ZoombiniText::kJapaneseF1DebugName[];
constexpr char32_t ZoombiniText::kJapaneseDebugF1Name[];
constexpr char32_t ZoombiniText::kJapaneseInitialKana[];
constexpr char32_t ZoombiniText::kJapaneseNormalKana[];
constexpr char32_t ZoombiniText::kJapaneseSemiVoicedKana[];
constexpr char32_t ZoombiniText::kJapaneseVoicedKana[];
constexpr char32_t ZoombiniText::kJapaneseContractedKana[];
constexpr char32_t ZoombiniText::kJapaneseSmallKana[];

constexpr const char *const ZoombiniText::kExeCreditParagraphsAnchor;
constexpr const char *const ZoombiniText::kExeSpanishCreditParagraphsAnchor;
constexpr const char *const ZoombiniText::kExePortugueseCreditParagraphsAnchor;
const ZoombiniText::CreditLimits ZoombiniText::kV10ESCreditLimits = {254, 31, 2464};
const ZoombiniText::CreditLimits ZoombiniText::kV10USCreditLimits = {248, 27, 2349};
const ZoombiniText::CreditLimits ZoombiniText::kV10PTCreditLimits = {267, 32, 2622};
const ZoombiniText::CreditLimits ZoombiniText::kV11USCreditLimits = {248, 27, 2355};
const ZoombiniText::CreditLimits ZoombiniText::kV111KRCreditLimits = {258, 35, 2541};
const ZoombiniText::CreditLimits ZoombiniText::kV20USCreditLimits = {360, 49, 4105};
const ZoombiniText::CreditLimits ZoombiniText::kV20PLCreditLimits = {360, 63, 4300};
constexpr uint32 ZoombiniText::kEuropeITCreditEntryCount;
constexpr uint32 ZoombiniText::kEuropeBRCreditEntryCount;
constexpr uint32 ZoombiniText::kEuropeFRCreditEntryCount;
constexpr uint32 ZoombiniText::kEuropeGECreditEntryCount;

ZoombiniText::ExeTextSource ZoombiniText::ExeTextSource::inFile(const char *srcFileName, uint32 srcExpectedSize) const {
	ExeTextSource source = *this;
	source.fileName = srcFileName;
	source.archiveName = nullptr;
	source.archiveMemberName = nullptr;
	source.archiveIsCabinet = false;
	source.expectedSize = srcExpectedSize;
	return source;
}

ZoombiniText::ExeTextSource ZoombiniText::ExeTextSource::inArchiveMember(const char *srcArchiveName, const char *srcMemberName, uint32 srcExpectedSize) const {
	ExeTextSource source = *this;
	source.fileName = nullptr;
	source.archiveName = srcArchiveName;
	source.archiveMemberName = srcMemberName;
	source.archiveIsCabinet = false;
	source.expectedSize = srcExpectedSize;
	return source;
}

ZoombiniText::ExeTextSource ZoombiniText::ExeTextSource::inMac68kDataResFork(const char *srcFileName, uint32 srcExpectedSize) const {
	ExeTextSource source = inFile(srcFileName, srcExpectedSize);
	source.primaryDataSource = ExeTextDataSource::kExpandedMac68kA5Globals;
	return source;
}

ZoombiniText::ExeTextSource ZoombiniText::ExeTextSource::inMacPpcPefDataFork(const char *srcFileName, uint32 srcExpectedSize, uint16 sectionIdx, uint32 sectionSize) const {
	ExeTextSource source = inFile(srcFileName, srcExpectedSize);
	source.primaryDataSource = ExeTextDataSource::kExpandedMacPpcPefDataSection;
	source.macPpcPefDataSection = sectionIdx;
	source.macPpcPefDataSize = sectionSize;
	return source;
}

ZoombiniText::ExeTextSource ZoombiniText::ExeTextSource::inCabinetMember(const char *srcArchiveName, const char *srcMemberName, uint32 srcExpectedSize) const {
	ExeTextSource source = inArchiveMember(srcArchiveName, srcMemberName, srcExpectedSize);
	source.archiveIsCabinet = true;
	return source;
}

ZoombiniText::ExeTextSource ZoombiniText::ExeTextSource::withTextTable(Common::CodePage srcCodePage, const ZoombiniText::ExeTextEntry *srcEntries,
																	   uint32 srcEntryCount, ExeTextDataSource srcDataSource) {
	this->codePage = srcCodePage;
	textTables.clear();
	return withAdditionalTextTable(srcEntries, srcEntryCount, srcDataSource);
}

ZoombiniText::ExeTextSource ZoombiniText::ExeTextSource::withAdditionalTextTable(const ZoombiniText::ExeTextEntry *srcEntries, uint32 srcEntryCount, ExeTextDataSource srcDataSource) {
	textTables.push_back(ExeTextTable(srcEntries, srcEntryCount, srcDataSource));
	return *this;
}

ZoombiniText::ExeTextSource ZoombiniText::ExeTextSource::withRequiredBytes(uint32 srcOffset, const char *srcBytes) {
	this->requiredBytesOffset = srcOffset;
	this->requiredBytes = srcBytes;
	return *this;
}

ZoombiniText::ExeTextSource ZoombiniText::ExeTextSource::withTextPatches(ZoombiniText::ExeTextPatchFunction patch) {
	textPatches = patch;
	return *this;
}

ZoombiniText::ExeTextSource ZoombiniText::ExeTextSource::withCreditLinePatches(ZoombiniText::ExeTextPatchFunction patch) {
	creditLinePatches = patch;
	return *this;
}

ZoombiniText::ExeTextSource ZoombiniText::ExeTextSource::withMacRomanTrademarkByteAAEscape() {
	escapeMacRomanTrademarkByteAA = true;
	return *this;
}

ZoombiniText::ExeTextSource ZoombiniText::ExeTextSource::withWindows1252StrokeByteD8Decode() {
	decodeWindows1252StrokeByteD8 = true;
	return *this;
}

ZoombiniText::ExeTextSource ZoombiniText::ExeTextSource::withCreditAnchor(const char *anchor) {
	creditSourceKind = CreditSourceKind::kExecutableAnchorSequence;
	creditAnchor = anchor;
	return *this;
}

ZoombiniText::ExeTextSource ZoombiniText::ExeTextSource::withCreditPtrTable(uint32 tableOffset, uint32 baseAddress, uint32 firstIndex) {
	creditSourceKind = CreditSourceKind::kExecutablePointerStream;
	creditPointerTableOffset = tableOffset;
	creditPointerBaseAddress = baseAddress;
	creditPointerFirstIndex = firstIndex;
	return *this;
}

ZoombiniText::ExeTextSource ZoombiniText::ExeTextSource::withMacA5RelativeCreditPtrTable(uint32 tableOffset) {
	withCreditPtrTable(tableOffset, 0x0, 0);
	creditSourceKind = CreditSourceKind::kMac68kA5RelativePointerStream;
	return *this;
}

ZoombiniText::ExeTextSource ZoombiniText::ExeTextSource::withMacPefRelativeCreditPtrTable(uint32 tableOffset) {
	withCreditPtrTable(tableOffset, 0x0, 0);
	creditSourceKind = CreditSourceKind::kMacPpcPefRelativePointerStream;
	return *this;
}

ZoombiniText::ExeTextSource ZoombiniText::ExeTextSource::withCreditPtrBlankAddress(uint32 address) {
	creditPointerBlankAddressFirst = address;
	creditPointerBlankAddressLast = address;
	return *this;
}

ZoombiniText::ExeTextSource ZoombiniText::ExeTextSource::withCreditPtrBlankAddressRange(uint32 firstAddress, uint32 lastAddress) {
	creditPointerBlankAddressFirst = firstAddress;
	creditPointerBlankAddressLast = lastAddress;
	return *this;
}

ZoombiniText::ExeTextSource ZoombiniText::ExeTextSource::withCreditLimits(const ZoombiniText::CreditLimits &limits) {
	creditLimits = limits;
	return *this;
}

void ZoombiniText::patchSpanish10ExeText() {
	Common::HashMap<uint32, Common::U32String>::iterator it = _strMap.find(ZoombiniText::kOptionsNewGame);
	if (it == _strMap.end() || it->_value.size() <= 7)
		return;

	// Correct the New Game typo displayed by the Spanish Options dialog.
	it->_value[7] = U'A'; // "NUEVA PERTIDA (CTRL N)" -> "NUEVA PARTIDA (CTRL N)"
}

void ZoombiniText::patchKoreanExeCredits() {
	// One Korean programmer line omitted the closing parenthesis.
	// Patch: "이인선(프로그래머" -> "이인선(프로그래머)"
	const uint pIdx = 36;
	const uint lIdx = 19;
	if (_creditParagraphs.size() <= pIdx || _creditParagraphs[pIdx]._lines.size() <= lIdx)
		return;

	_creditParagraphs[pIdx]._lines[lIdx] += U')';
}

const ZoombiniText::ResTextEntry ZoombiniText::kV10EU_ResTextEntries[] = {
	{ZoombiniText::kPicker, 2000, 0},
	{ZoombiniText::kBridge, 2000, 1},
	{ZoombiniText::kCaves, 2000, 2},
	{ZoombiniText::kPizza, 2000, 3},
	{ZoombiniText::kBasecamp1, 2000, 4},
	{ZoombiniText::kFerry, 2000, 5},
	{ZoombiniText::kLilly, 2000, 6},
	{ZoombiniText::kSlides, 2000, 7},
	{ZoombiniText::kFleens, 2000, 8},
	{ZoombiniText::kHotel, 2000, 9},
	{ZoombiniText::kNet, 2000, 10},
	{ZoombiniText::kBasecamp2, 2000, 11},
	{ZoombiniText::kTunnels, 2000, 12},
	{ZoombiniText::kSmoke, 2000, 13},
	{ZoombiniText::kMaze, 2000, 14},
	{ZoombiniText::kTown, 2000, 15},
	// STRL 2002 journey-stat labels are distinct from the full page names in STRL 2000.
	{ZoombiniText::kJourneyStatZoombiniIsle, 2002, 0},
	{ZoombiniText::kJourneyStatShelterRock, 2002, 1},
	{ZoombiniText::kJourneyStatShadeTree, 2002, 2},
	{ZoombiniText::kJourneyStatZoombiniton, 2002, 3},
	{ZoombiniText::kPracticeMode, 2002, 4},
	// Europe-v1 STRL 2002 return-path blocks; USv1.0 supplies these keys from executable tables.
	{ZoombiniText::kPracticeReturnInitial1, 2002, 5},
	{ZoombiniText::kPracticeReturnInitial2, 2002, 6},
	{ZoombiniText::kPracticeReturnInitial3, 2002, 7},
	{ZoombiniText::kPracticeReturnInitial4, 2002, 8},
	{ZoombiniText::kPracticeReturnShelterRock1, 2002, 9},
	{ZoombiniText::kPracticeReturnShelterRock2, 2002, 10},
	{ZoombiniText::kPracticeReturnShelterRock3, 2002, 11},
	{ZoombiniText::kPracticeReturnShelterRock4, 2002, 12},
	{ZoombiniText::kPracticeReturnShadeTree1, 2002, 13},
	{ZoombiniText::kPracticeReturnShadeTree2, 2002, 14},
	{ZoombiniText::kPracticeReturnShadeTree3, 2002, 15},
	{ZoombiniText::kPracticeReturnShadeTree4, 2002, 16},
	{ZoombiniText::kPracticeReturnZoombiniton1, 2002, 17},
	{ZoombiniText::kPracticeReturnZoombiniton2, 2002, 18},
	{ZoombiniText::kPracticeReturnZoombiniton3, 2002, 19},
	{ZoombiniText::kPracticeReturnZoombiniton4, 2002, 20},
	{ZoombiniText::kTerrainKey, 2004, 0},
	{ZoombiniText::kChooseLevel, 2004, 1},
	{ZoombiniText::kLevel1, 2004, 2},
	{ZoombiniText::kLevel2, 2004, 3},
	{ZoombiniText::kLevel3, 2004, 4},
	{ZoombiniText::kLevel4, 2004, 5},
	{ZoombiniText::kRoute1, 2006, 0},
	{ZoombiniText::kRoute2, 2006, 1},
	{ZoombiniText::kRoute3, 2006, 2},
	{ZoombiniText::kRoute4, 2006, 3},
	{ZoombiniText::kXferVillePopulation, 2008, 0},
	{ZoombiniText::kMemorialJanuary, 2010, 0},
	{ZoombiniText::kMemorialFebruary, 2010, 1},
	{ZoombiniText::kMemorialMarch, 2010, 2},
	{ZoombiniText::kMemorialApril, 2010, 3},
	{ZoombiniText::kMemorialMay, 2010, 4},
	{ZoombiniText::kMemorialJune, 2010, 5},
	{ZoombiniText::kMemorialJuly, 2010, 6},
	{ZoombiniText::kMemorialAugust, 2010, 7},
	{ZoombiniText::kMemorialSeptember, 2010, 8},
	{ZoombiniText::kMemorialOctober, 2010, 9},
	{ZoombiniText::kMemorialNovember, 2010, 10},
	{ZoombiniText::kMemorialDecember, 2010, 11},
	{ZoombiniText::kMemorialWhenLevel, 2012, 0},
	{ZoombiniText::kMemorialHonorMonument, 2014, 0},
	{ZoombiniText::kMemorialHonorWindmill, 2014, 1},
	{ZoombiniText::kMemorialHonorObservatory, 2014, 2},
	{ZoombiniText::kMemorialHonorBowlingAlley, 2014, 3},
	{ZoombiniText::kMemorialHonorGeneralStore, 2014, 4},
	{ZoombiniText::kMemorialHonorSwimmingPool, 2014, 5},
	{ZoombiniText::kMemorialHonorPlayground, 2014, 6},
	{ZoombiniText::kMemorialHonorBandShell, 2014, 7},
	{ZoombiniText::kMemorialHonorSchool, 2014, 8},
	{ZoombiniText::kMemorialHonorLibrary, 2014, 9},
	{ZoombiniText::kMemorialHonorFire, 2014, 10},
	{ZoombiniText::kMemorialHonorOpera, 2014, 11},
	{ZoombiniText::kMemorialHonorCityHall, 2014, 12},
	{ZoombiniText::kMemorialHonorClockTower, 2014, 13},
	{ZoombiniText::kMemorialHonorMuseum, 2014, 14},
	{ZoombiniText::kMemorialHonorCourt, 2014, 15},
	{ZoombiniText::kMemorialRoute1Level1, 2016, 0},
	{ZoombiniText::kMemorialRoute1Level2, 2016, 1},
	{ZoombiniText::kMemorialRoute1Level3, 2016, 2},
	{ZoombiniText::kMemorialRoute1Level4, 2016, 3},
	{ZoombiniText::kMemorialRoute2Level1, 2016, 4},
	{ZoombiniText::kMemorialRoute2Level2, 2016, 5},
	{ZoombiniText::kMemorialRoute2Level3, 2016, 6},
	{ZoombiniText::kMemorialRoute2Level4, 2016, 7},
	{ZoombiniText::kMemorialRoute3Level1, 2016, 8},
	{ZoombiniText::kMemorialRoute3Level2, 2016, 9},
	{ZoombiniText::kMemorialRoute3Level3, 2016, 10},
	{ZoombiniText::kMemorialRoute3Level4, 2016, 11},
	{ZoombiniText::kMemorialRoute4Level1, 2016, 12},
	{ZoombiniText::kMemorialRoute4Level2, 2016, 13},
	{ZoombiniText::kMemorialRoute4Level3, 2016, 14},
	{ZoombiniText::kMemorialRoute4Level4, 2016, 15},
	{ZoombiniText::kDialogBodyGoMapWillLost, 2018, 0},
	{ZoombiniText::kDialogButtonLoseThem, 2018, 1},
	{ZoombiniText::kDialogButtonKeepThem, 2018, 2},
	{ZoombiniText::kDialogButtonOkay, 2018, 3},
	{ZoombiniText::kDialogButtonCancel, 2018, 4},
	{ZoombiniText::kDialogButtonLoad, 2018, 5},
	{ZoombiniText::kDialogButtonSave, 2018, 6},
	{ZoombiniText::kOptionsTitle, 2018, 7},
	{ZoombiniText::kOptionsLegendOn, 2018, 8},
	{ZoombiniText::kOptionsLegendOff, 2018, 9},
	{ZoombiniText::kOptionsToggle, 2018, 10},
	{ZoombiniText::kOptionsNewGame, 2018, 11},
	{ZoombiniText::kOptionsLoadGame, 2018, 12},
	{ZoombiniText::kOptionsSaveGame, 2018, 13},
	{ZoombiniText::kOptionsQuit, 2018, 14},
	{ZoombiniText::kOptionsSound, 2018, 15},
	{ZoombiniText::kOptionsMusic, 2018, 16},
	{ZoombiniText::kOptionsStickyMouse, 2018, 17},
	{ZoombiniText::kOptionsTransitions, 2018, 18},
	{ZoombiniText::kDialogBodyNoSavedGames, 2018, 19},
	{ZoombiniText::kDialogBodyCreateAndSaveNewGame, 2018, 21},
	{ZoombiniText::kNewGame, 2018, 22},
	{ZoombiniText::kDialogButtonNewGame, 2018, 22},
	{ZoombiniText::kDialogButtonReplaceTitle, 2018, 23},
	{ZoombiniText::kDialogButtonReplace, 2018, 23},
	{ZoombiniText::kDialogTitleSave, 2018, 24},
	{ZoombiniText::kDialogTitleSaveAs, 2018, 25},
	{ZoombiniText::kDialogTitleLoad, 2018, 26},
	{ZoombiniText::kDialogBodyReplaceGame, 2018, 27},
	{ZoombiniText::kOptionsCredits, 2018, 28},
	{ZoombiniText::kDialogBodySaveCurrentGame, 2018, 29},
	{ZoombiniText::kDialogButtonYes, 2018, 30},
	{ZoombiniText::kDialogButtonQuit, 2018, 31},
	{ZoombiniText::kDialogBodySaveBeforeQuit, 2018, 32},
	{ZoombiniText::kDialogButtonNo, 2018, 34},
	{ZoombiniText::kDialogBodyCannotSaveInPractice, 2018, 35},
	{ZoombiniText::kDialogBodyCreateNewGame, 2018, 36},
	{ZoombiniText::kDialogBodyCannotSaveMoreGame, 2018, 37},
	{ZoombiniText::kDialogBodyCannotLoadInPractice, 2018, 38},
	{ZoombiniText::kDialogBodyCannotCreateNewInPractice, 2018, 39},
	{ZoombiniText::kDialogBodyNewGame, 2018, 40},
	{ZoombiniText::kDialogBodyReallyQuit, 2018, 41},
	{ZoombiniText::kNotiBoxMusicOn, 2023, 0},
	{ZoombiniText::kNotiBoxMusicOff, 2023, 1},
	{ZoombiniText::kNotiBoxSoundOn, 2023, 2},
	{ZoombiniText::kNotiBoxSoundOff, 2023, 3},
	{ZoombiniText::kNotiBoxLessAction, 2023, 4},
	{ZoombiniText::kNotiBoxMoreAction, 2023, 5},
	{ZoombiniText::kNotiBoxHideCursor, 2023, 6},
	{ZoombiniText::kNotiBoxShowCursor, 2023, 7},
	{ZoombiniText::kNotiBoxStickeyMouse, 2023, 8},
	{ZoombiniText::kNotiBoxNonStickeyMouse, 2023, 9},
	{ZoombiniText::kNotiBoxTransitionsOn, 2023, 10},
	{ZoombiniText::kNotiBoxTransitionsOff, 2023, 11},
	{ZoombiniText::kNotiBoxAutoStickeyOn, 2023, 12},
	{ZoombiniText::kNotiBoxAutoStickeyOff, 2023, 13},
};

const ZoombiniText::ExeTextEntry ZoombiniText::kV10BR_NETextEntries[] = {
	{ZoombiniText::kDialogHelpPickerUpdateVersion, 0x0, 0}, // NO-OP
	{ZoombiniText::kNewGame, 0xD6840, 8},
};

const ZoombiniText::ExeTextEntry ZoombiniText::kV10BRDemo_NETextEntries[] = {
	{ZoombiniText::kDialogHelpPickerUpdateVersion, 0x0, 0}, // NO-OP
	{ZoombiniText::kNewGame, 0xD72C8, 8},
};

const ZoombiniText::ExeTextEntry ZoombiniText::kV11BR_NETextEntries[] = {
	{ZoombiniText::kNewGame, 0xD7CB4, 8},
};

const ZoombiniText::ExeTextEntry ZoombiniText::kV10FR_NETextEntries[] = {
	{ZoombiniText::kDialogHelpPickerUpdateVersion, 0x0, 0}, // NO-OP
	{ZoombiniText::kNewGame, 0xD6849, 15},
};

const ZoombiniText::ExeTextEntry ZoombiniText::kV11FR_NETextEntries[] = {
	{ZoombiniText::kNewGame, 0xD7CBD, 15},
};

const ZoombiniText::ExeTextEntry ZoombiniText::kV10GE_NETextEntries[] = {
	{ZoombiniText::kDialogHelpPickerUpdateVersion, 0x0, 0}, // NO-OP
	{ZoombiniText::kNewGame, 0xD6859, 11},
};

const ZoombiniText::ExeTextEntry ZoombiniText::kV11GE_NETextEntries[] = {
	{ZoombiniText::kNewGame, 0xD7CCD, 11},
};

// Key-address-size map for the Italian v1.0 NE executable.
const ZoombiniText::ExeTextEntry ZoombiniText::kV10IT_NETextEntries[] = {
	{ZoombiniText::kDialogHelpPickerUpdateVersion, 0x0, 0}, // NO-OP
	{ZoombiniText::kNewGame, 0xD78DE, 13},
};

const ZoombiniText::ExeTextEntry ZoombiniText::kV10JP_PETextEntries[] = {
	{ZoombiniText::kDialogHelpPickerUpdateVersion, 0x0, 0}, // NO-OP
	{ZoombiniText::kNewGame, 0x88551, 10},
	{ZoombiniText::kNotiBoxEnglishNames, 0x8D597, 10},
	{ZoombiniText::kNotiBoxJapaneseNames, 0x8D5A2, 12},
};

const ZoombiniText::ExeTextEntry ZoombiniText::kV10JPDemo_PETextEntries[] = {
	{ZoombiniText::kDialogHelpPickerUpdateVersion, 0x0, 0}, // NO-OP
	{ZoombiniText::kNewGame, 0x90575, 10},
	{ZoombiniText::kNotiBoxEnglishNames, 0x9561B, 10},
	{ZoombiniText::kNotiBoxJapaneseNames, 0x95626, 12},
};

const ZoombiniText::ExeTextEntry ZoombiniText::kV10JPMacDemoTextEntries[] = {
	{ZoombiniText::kDialogHelpPickerUpdateVersion, 0x0, 0}, // NO-OP
	{ZoombiniText::kNewGame, 0x249F3, 10},
	{ZoombiniText::kNotiBoxEnglishNames, 0x23764, 10},
	{ZoombiniText::kNotiBoxJapaneseNames, 0x23757, 12},
};

const ZoombiniText::ExeTextEntry ZoombiniText::kV10ES_PETextEntries[] = {
	{ZoombiniText::kDialogHelpPickerUpdateVersion, 0x0, 0}, // NO-OP
	{ZoombiniText::kPicker, 0x899B4, 13},
	{ZoombiniText::kBridge, 0x899C2, 16},
	{ZoombiniText::kCaves, 0x899D3, 21},
	{ZoombiniText::kPizza, 0x899E9, 11},
	{ZoombiniText::kBasecamp1, 0x899F5, 12},
	{ZoombiniText::kFerry, 0x89A02, 23},
	{ZoombiniText::kLilly, 0x89A1A, 24},
	{ZoombiniText::kSlides, 0x89A33, 14},
	{ZoombiniText::kFleens, 0x89A42, 8},
	{ZoombiniText::kHotel, 0x89A4B, 5},
	{ZoombiniText::kNet, 0x89A51, 22},
	{ZoombiniText::kBasecamp2, 0x89A68, 13},
	{ZoombiniText::kTunnels, 0x89A76, 17},
	{ZoombiniText::kSmoke, 0x89A88, 14},
	{ZoombiniText::kMaze, 0x89A97, 22},
	{ZoombiniText::kTown, 0x89AAE, 14},
	{ZoombiniText::kJourneyStatZoombiniIsle, 0x89ABD, 13},
	{ZoombiniText::kJourneyStatShelterRock, 0x89ACB, 12},
	{ZoombiniText::kJourneyStatShadeTree, 0x89AD8, 13},
	{ZoombiniText::kJourneyStatZoombiniton, 0x89AE6, 14},
	{ZoombiniText::kPracticeMode, 0x89AF5, 9},
	{ZoombiniText::kPracticeReturnInitial1, 0x89AFF, 16},
	{ZoombiniText::kPracticeReturnInitial2, 0x89B10, 18},
	{ZoombiniText::kPracticeReturnInitial3, 0x89B23, 1},
	{ZoombiniText::kPracticeReturnInitial4, 0x89B25, 1},
	{ZoombiniText::kPracticeReturnShelterRock1, 0x89B27, 16},
	{ZoombiniText::kPracticeReturnShelterRock2, 0x89B38, 18},
	{ZoombiniText::kPracticeReturnShelterRock3, 0x89B4B, 14},
	{ZoombiniText::kPracticeReturnShelterRock4, 0x89B5A, 1},
	{ZoombiniText::kPracticeReturnShadeTree1, 0x89B5C, 16},
	{ZoombiniText::kPracticeReturnShadeTree2, 0x89B6D, 19},
	{ZoombiniText::kPracticeReturnShadeTree3, 0x89B81, 12},
	{ZoombiniText::kPracticeReturnShadeTree4, 0x89B8E, 13},
	{ZoombiniText::kPracticeReturnZoombiniton1, 0x89B9C, 16},
	{ZoombiniText::kPracticeReturnZoombiniton2, 0x89BAD, 19},
	{ZoombiniText::kPracticeReturnZoombiniton3, 0x89BC1, 25},
	{ZoombiniText::kPracticeReturnZoombiniton4, 0x89BDB, 16},
	{ZoombiniText::kTerrainKey, 0x89BEC, 17},
	{ZoombiniText::kChooseLevel, 0x89BFE, 14},
	{ZoombiniText::kLevel1, 0x89C0D, 12},
	{ZoombiniText::kLevel2, 0x89C1A, 12},
	{ZoombiniText::kLevel3, 0x89C27, 8},
	{ZoombiniText::kLevel4, 0x89C30, 13},
	{ZoombiniText::kRoute1, 0x89C3E, 31},
	{ZoombiniText::kRoute2, 0x89C5E, 14},
	{ZoombiniText::kRoute3, 0x89C6D, 24},
	{ZoombiniText::kRoute4, 0x89C86, 16},
	{ZoombiniText::kXferVillePopulation, 0x89C97, 25},
	{ZoombiniText::kMemorialJanuary, 0x89CB1, 5},
	{ZoombiniText::kMemorialFebruary, 0x89CB7, 7},
	{ZoombiniText::kMemorialMarch, 0x89CBF, 5},
	{ZoombiniText::kMemorialApril, 0x89CC5, 5},
	{ZoombiniText::kMemorialMay, 0x89CCB, 4},
	{ZoombiniText::kMemorialJune, 0x89CD0, 5},
	{ZoombiniText::kMemorialJuly, 0x89CD6, 5},
	{ZoombiniText::kMemorialAugust, 0x89CDC, 6},
	{ZoombiniText::kMemorialSeptember, 0x89CE3, 10},
	{ZoombiniText::kMemorialOctober, 0x89CEE, 7},
	{ZoombiniText::kMemorialNovember, 0x89CF6, 9},
	{ZoombiniText::kMemorialDecember, 0x89D00, 9},
	{ZoombiniText::kMemorialWhenLevel, 0x89D0A, 29},
	{ZoombiniText::kMemorialHonorMonument, 0x89D28, 56},
	{ZoombiniText::kMemorialHonorWindmill, 0x89D61, 61},
	{ZoombiniText::kMemorialHonorObservatory, 0x89D9F, 47},
	{ZoombiniText::kMemorialHonorBowlingAlley, 0x89DCF, 38},
	{ZoombiniText::kMemorialHonorGeneralStore, 0x89DF6, 48},
	{ZoombiniText::kMemorialHonorSwimmingPool, 0x89E27, 40},
	{ZoombiniText::kMemorialHonorPlayground, 0x89E50, 60},
	{ZoombiniText::kMemorialHonorBandShell, 0x89E8D, 62},
	{ZoombiniText::kMemorialHonorSchool, 0x89ECC, 40},
	{ZoombiniText::kMemorialHonorLibrary, 0x89EF5, 57},
	{ZoombiniText::kMemorialHonorFire, 0x89F2F, 52},
	{ZoombiniText::kMemorialHonorOpera, 0x89F64, 47},
	{ZoombiniText::kMemorialHonorCityHall, 0x89F94, 52},
	{ZoombiniText::kMemorialHonorClockTower, 0x89FC9, 60},
	{ZoombiniText::kMemorialHonorMuseum, 0x8A006, 56},
	{ZoombiniText::kMemorialHonorCourt, 0x8A03F, 51},
	{ZoombiniText::kMemorialRoute1Level1, 0x8A073, 118},
	{ZoombiniText::kMemorialRoute1Level2, 0x8A0EA, 139},
	{ZoombiniText::kMemorialRoute1Level3, 0x8A176, 142},
	{ZoombiniText::kMemorialRoute1Level4, 0x8A205, 127},
	{ZoombiniText::kMemorialRoute2Level1, 0x8A285, 76},
	{ZoombiniText::kMemorialRoute2Level2, 0x8A2D2, 78},
	{ZoombiniText::kMemorialRoute2Level3, 0x8A321, 110},
	{ZoombiniText::kMemorialRoute2Level4, 0x8A390, 81},
	{ZoombiniText::kMemorialRoute3Level1, 0x8A3E2, 116},
	{ZoombiniText::kMemorialRoute3Level2, 0x8A457, 102},
	{ZoombiniText::kMemorialRoute3Level3, 0x8A4BE, 110},
	{ZoombiniText::kMemorialRoute3Level4, 0x8A52D, 130},
	{ZoombiniText::kMemorialRoute4Level1, 0x8A5B0, 138},
	{ZoombiniText::kMemorialRoute4Level2, 0x8A63B, 144},
	{ZoombiniText::kMemorialRoute4Level3, 0x8A6CC, 110},
	{ZoombiniText::kMemorialRoute4Level4, 0x8A73B, 150},
	{ZoombiniText::kDialogBodyGoMapWillLost, 0x8A7D2, 58},
	{ZoombiniText::kDialogButtonLoseThem, 0x8A80D, 9},
	{ZoombiniText::kDialogButtonKeepThem, 0x8A817, 9},
	{ZoombiniText::kDialogButtonOkay, 0x8A821, 4},
	{ZoombiniText::kDialogButtonCancel, 0x8A826, 8},
	{ZoombiniText::kDialogButtonLoad, 0x8A82F, 6},
	{ZoombiniText::kDialogButtonSave, 0x8A836, 6},
	{ZoombiniText::kOptionsTitle, 0x8A83D, 8},
	{ZoombiniText::kOptionsLegendOn, 0x8A846, 4},
	{ZoombiniText::kOptionsLegendOff, 0x8A84B, 4},
	{ZoombiniText::kOptionsToggle, 0x8A850, 19},
	{ZoombiniText::kOptionsNewGame, 0x8A864, 22},
	{ZoombiniText::kOptionsLoadGame, 0x8A87B, 23},
	{ZoombiniText::kOptionsSaveGame, 0x8A893, 23},
	{ZoombiniText::kOptionsQuit, 0x8A8AB, 14},
	{ZoombiniText::kOptionsSound, 0x8A8BA, 29},
	{ZoombiniText::kOptionsMusic, 0x8A8D8, 24},
	{ZoombiniText::kOptionsStickyMouse, 0x8A8F1, 25},
	{ZoombiniText::kOptionsTransitions, 0x8A90B, 21},
	{ZoombiniText::kDialogBodyNoSavedGames, 0x8A921, 15},
	{ZoombiniText::kDialogButtonAccept, 0x8A931, 4},
	{ZoombiniText::kDialogBodyCreateAndSaveNewGame, 0x8A936, 63},
	{ZoombiniText::kNewGame, 0x888EA, 13},
	{ZoombiniText::kDialogButtonNewGame, 0x8A976, 5},
	{ZoombiniText::kDialogButtonReplaceTitle, 0x8A97C, 10},
	{ZoombiniText::kDialogButtonReplace, 0x8A97C, 10},
	{ZoombiniText::kDialogTitleSave, 0x8A987, 6},
	{ZoombiniText::kDialogTitleSaveAs, 0x8A98E, 12},
	{ZoombiniText::kDialogTitleLoad, 0x8A99B, 6},
	{ZoombiniText::kDialogBodyReplaceGame, 0x8A9A2, 59},
	{ZoombiniText::kOptionsCredits, 0x8A9DE, 8},
	{ZoombiniText::kDialogBodySaveCurrentGame, 0x8A9E7, 26},
	{ZoombiniText::kDialogButtonYes, 0x8AA02, 2},
	{ZoombiniText::kDialogButtonQuit, 0x8AA05, 8},
	{ZoombiniText::kDialogBodySaveBeforeQuit, 0x8AA0E, 83},
	{ZoombiniText::kDialogButtonNo, 0x8AA65, 2},
	{ZoombiniText::kDialogBodyCannotSaveInPractice, 0x8AA68, 45},
	{ZoombiniText::kDialogBodyCreateNewGame, 0x8AA96, 51},
	{ZoombiniText::kDialogBodyCannotSaveMoreGame, 0x8AACA, 38},
	{ZoombiniText::kDialogBodyCannotLoadInPractice, 0x8AAF1, 45},
	{ZoombiniText::kDialogBodyCannotCreateNewInPractice, 0x8AB1F, 55},
	{ZoombiniText::kDialogBodyNewGame, 0x8AB57, 5},
	{ZoombiniText::kDialogBodyReallyQuit, 0x8AB5D, 25},
	{ZoombiniText::kNotiBoxMusicOn, 0x8B615, 9},
	{ZoombiniText::kNotiBoxMusicOff, 0x8B61F, 9},
	{ZoombiniText::kNotiBoxSoundOn, 0x8B629, 9},
	{ZoombiniText::kNotiBoxSoundOff, 0x8B633, 9},
	{ZoombiniText::kNotiBoxLessAction, 0x8B63D, 12},
	{ZoombiniText::kNotiBoxMoreAction, 0x8B64A, 10},
	{ZoombiniText::kNotiBoxHideCursor, 0x8B655, 13},
	{ZoombiniText::kNotiBoxShowCursor, 0x8B663, 14},
	{ZoombiniText::kNotiBoxStickeyMouse, 0x8B672, 12},
	{ZoombiniText::kNotiBoxNonStickeyMouse, 0x8B67F, 15},
	{ZoombiniText::kNotiBoxTransitionsOn, 0x8B68F, 15},
	{ZoombiniText::kNotiBoxTransitionsOff, 0x8B69F, 15},
	{ZoombiniText::kNotiBoxAutoStickeyOn, 0x8B6AF, 14},
	{ZoombiniText::kNotiBoxAutoStickeyOff, 0x8B6BE, 14},
};

// Key-address-size map for the Portuguese v1.0 PE32 executable.
// Lengths are the text byte counts before each trailing NUL terminator.
const ZoombiniText::ExeTextEntry ZoombiniText::kV10PT_PETextEntries[] = {
	{ZoombiniText::kDialogHelpPickerUpdateVersion, 0x0, 0}, // NO-OP
	{ZoombiniText::kPicker, 0x89A30, 20},
	{ZoombiniText::kBridge, 0x89A45, 21},
	{ZoombiniText::kCaves, 0x89A5B, 30},
	{ZoombiniText::kPizza, 0x89A7A, 19},
	{ZoombiniText::kBasecamp1, 0x89A8E, 29},
	{ZoombiniText::kFerry, 0x89AAC, 26},
	{ZoombiniText::kLilly, 0x89AC7, 26},
	{ZoombiniText::kSlides, 0x89AE2, 19},
	{ZoombiniText::kFleens, 0x89AF6, 10},
	{ZoombiniText::kHotel, 0x89B01, 16},
	{ZoombiniText::kNet, 0x89B12, 24},
	{ZoombiniText::kBasecamp2, 0x89B2B, 29},
	{ZoombiniText::kTunnels, 0x89B49, 14},
	{ZoombiniText::kSmoke, 0x89B58, 20},
	{ZoombiniText::kMaze, 0x89B6D, 26},
	{ZoombiniText::kTown, 0x89B88, 12},
	{ZoombiniText::kJourneyStatZoombiniIsle, 0x89B95, 20},
	{ZoombiniText::kJourneyStatShelterRock, 0x89BAA, 29},
	{ZoombiniText::kJourneyStatShadeTree, 0x89BC8, 29},
	{ZoombiniText::kJourneyStatZoombiniton, 0x89BE6, 12},
	{ZoombiniText::kPracticeMode, 0x89BF3, 11},
	{ZoombiniText::kPracticeReturnInitial1, 0x89BFF, 20},
	{ZoombiniText::kPracticeReturnInitial2, 0x89C14, 28},
	{ZoombiniText::kPracticeReturnInitial3, 0x89C31, 1},
	{ZoombiniText::kPracticeReturnInitial4, 0x89C33, 1},
	{ZoombiniText::kPracticeReturnShelterRock1, 0x89C35, 20},
	{ZoombiniText::kPracticeReturnShelterRock2, 0x89C4A, 28},
	{ZoombiniText::kPracticeReturnShelterRock3, 0x89C67, 37},
	{ZoombiniText::kPracticeReturnShelterRock4, 0x89C8D, 1},
	{ZoombiniText::kPracticeReturnShadeTree1, 0x89C8F, 20},
	{ZoombiniText::kPracticeReturnShadeTree2, 0x89CA4, 29},
	{ZoombiniText::kPracticeReturnShadeTree3, 0x89CC2, 38},
	{ZoombiniText::kPracticeReturnShadeTree4, 0x89CE9, 34},
	{ZoombiniText::kPracticeReturnZoombiniton1, 0x89D0C, 20},
	{ZoombiniText::kPracticeReturnZoombiniton2, 0x89D21, 29},
	{ZoombiniText::kPracticeReturnZoombiniton3, 0x89D3F, 25},
	{ZoombiniText::kPracticeReturnZoombiniton4, 0x89D59, 22},
	{ZoombiniText::kTerrainKey, 0x89D70, 16},
	{ZoombiniText::kChooseLevel, 0x89D81, 16},
	{ZoombiniText::kLevel1, 0x89D92, 15},
	{ZoombiniText::kLevel2, 0x89DA2, 16},
	{ZoombiniText::kLevel3, 0x89DB3, 13},
	{ZoombiniText::kLevel4, 0x89DC1, 10},
	{ZoombiniText::kRoute1, 0x89DCC, 28},
	{ZoombiniText::kRoute2, 0x89DE9, 16},
	{ZoombiniText::kRoute3, 0x89DFA, 26},
	{ZoombiniText::kRoute4, 0x89E15, 26},
	{ZoombiniText::kXferVillePopulation, 0x89E30, 23},
	{ZoombiniText::kMemorialJanuary, 0x89E48, 7},
	{ZoombiniText::kMemorialFebruary, 0x89E50, 9},
	{ZoombiniText::kMemorialMarch, 0x89E5A, 5},
	{ZoombiniText::kMemorialApril, 0x89E60, 5},
	{ZoombiniText::kMemorialMay, 0x89E66, 4},
	{ZoombiniText::kMemorialJune, 0x89E6B, 5},
	{ZoombiniText::kMemorialJuly, 0x89E71, 5},
	{ZoombiniText::kMemorialAugust, 0x89E77, 6},
	{ZoombiniText::kMemorialSeptember, 0x89E7E, 8},
	{ZoombiniText::kMemorialOctober, 0x89E87, 7},
	{ZoombiniText::kMemorialNovember, 0x89E8F, 8},
	{ZoombiniText::kMemorialDecember, 0x89E98, 8},
	{ZoombiniText::kMemorialWhenLevel, 0x89EA1, 17},
	{ZoombiniText::kMemorialHonorMonument, 0x89EB3, 60},
	{ZoombiniText::kMemorialHonorWindmill, 0x89EF0, 69},
	{ZoombiniText::kMemorialHonorObservatory, 0x89F36, 43},
	{ZoombiniText::kMemorialHonorBowlingAlley, 0x89F62, 50},
	{ZoombiniText::kMemorialHonorGeneralStore, 0x89F95, 47},
	{ZoombiniText::kMemorialHonorSwimmingPool, 0x89FC5, 36},
	{ZoombiniText::kMemorialHonorPlayground, 0x89FEA, 61},
	{ZoombiniText::kMemorialHonorBandShell, 0x8A028, 60},
	{ZoombiniText::kMemorialHonorSchool, 0x8A065, 35},
	{ZoombiniText::kMemorialHonorLibrary, 0x8A089, 59},
	{ZoombiniText::kMemorialHonorFire, 0x8A0C5, 57},
	{ZoombiniText::kMemorialHonorOpera, 0x8A0FF, 47},
	{ZoombiniText::kMemorialHonorCityHall, 0x8A12F, 41},
	{ZoombiniText::kMemorialHonorClockTower, 0x8A159, 59},
	{ZoombiniText::kMemorialHonorMuseum, 0x8A195, 67},
	{ZoombiniText::kMemorialHonorCourt, 0x8A1D9, 51},
	{ZoombiniText::kMemorialRoute1Level1, 0x8A20D, 128},
	{ZoombiniText::kMemorialRoute1Level2, 0x8A28E, 143},
	{ZoombiniText::kMemorialRoute1Level3, 0x8A31E, 143},
	{ZoombiniText::kMemorialRoute1Level4, 0x8A3AE, 151},
	{ZoombiniText::kMemorialRoute2Level1, 0x8A446, 99},
	{ZoombiniText::kMemorialRoute2Level2, 0x8A4AA, 86},
	{ZoombiniText::kMemorialRoute2Level3, 0x8A501, 115},
	{ZoombiniText::kMemorialRoute2Level4, 0x8A575, 101},
	{ZoombiniText::kMemorialRoute3Level1, 0x8A5DB, 136},
	{ZoombiniText::kMemorialRoute3Level2, 0x8A664, 119},
	{ZoombiniText::kMemorialRoute3Level3, 0x8A6DC, 104},
	{ZoombiniText::kMemorialRoute3Level4, 0x8A745, 133},
	{ZoombiniText::kMemorialRoute4Level1, 0x8A7CB, 124},
	{ZoombiniText::kMemorialRoute4Level2, 0x8A848, 116},
	{ZoombiniText::kMemorialRoute4Level3, 0x8A8BD, 102},
	{ZoombiniText::kMemorialRoute4Level4, 0x8A924, 141},
	{ZoombiniText::kDialogBodyGoMapWillLost, 0x8A9B2, 65},
	{ZoombiniText::kDialogButtonLoseThem, 0x8A9F4, 9},
	{ZoombiniText::kDialogButtonKeepThem, 0x8A9FE, 9},
	{ZoombiniText::kDialogButtonOkay, 0x8AA08, 2},
	{ZoombiniText::kDialogButtonCancel, 0x8AA0B, 8},
	{ZoombiniText::kDialogButtonLoad, 0x8AA14, 8},
	{ZoombiniText::kDialogButtonSave, 0x8AA1D, 6},
	{ZoombiniText::kOptionsTitle, 0x8AA24, 6},
	{ZoombiniText::kOptionsLegendOn, 0x8AA2B, 5},
	{ZoombiniText::kOptionsLegendOff, 0x8AA31, 5},
	{ZoombiniText::kOptionsToggle, 0x8AA37, 27},
	{ZoombiniText::kOptionsNewGame, 0x8AA53, 18},
	{ZoombiniText::kOptionsLoadGame, 0x8AA66, 22},
	{ZoombiniText::kOptionsSaveGame, 0x8AA7D, 20},
	{ZoombiniText::kOptionsQuit, 0x8AA92, 21},
	{ZoombiniText::kOptionsSound, 0x8AAA8, 25},
	{ZoombiniText::kOptionsMusic, 0x8AAC2, 24},
	{ZoombiniText::kOptionsStickyMouse, 0x8AADB, 22},
	{ZoombiniText::kOptionsTransitions, 0x8AAF2, 19},
	{ZoombiniText::kDialogBodyNoSavedGames, 0x8AB06, 17},
	{ZoombiniText::kDialogButtonAccept, 0x8AB18, 7},
	{ZoombiniText::kDialogBodyCreateAndSaveNewGame, 0x8AB20, 52},
	{ZoombiniText::kNewGame, 0x888EA, 9},
	{ZoombiniText::kDialogButtonNewGame, 0x8AB55, 9},
	{ZoombiniText::kDialogButtonReplaceTitle, 0x8AB5F, 10},
	{ZoombiniText::kDialogButtonReplace, 0x8AB5F, 10},
	{ZoombiniText::kDialogTitleSave, 0x8AB6A, 13},
	{ZoombiniText::kDialogTitleSaveAs, 0x8AB78, 19},
	{ZoombiniText::kDialogTitleLoad, 0x8AB8C, 15},
	{ZoombiniText::kDialogBodyReplaceGame, 0x8AB9C, 59},
	{ZoombiniText::kOptionsCredits, 0x8ABD8, 8},
	{ZoombiniText::kDialogBodySaveCurrentGame, 0x8ABE1, 26},
	{ZoombiniText::kDialogButtonYes, 0x8ABFC, 3},
	{ZoombiniText::kDialogButtonQuit, 0x8AC00, 4},
	{ZoombiniText::kDialogBodySaveBeforeQuit, 0x8AC05, 68},
	{ZoombiniText::kDialogButtonNo, 0x8AC4A, 3},
	{ZoombiniText::kDialogBodyCannotSaveInPractice, 0x8AC52, 44},
	{ZoombiniText::kDialogBodyCreateNewGame, 0x8AC7F, 48},
	{ZoombiniText::kDialogBodyCannotSaveMoreGame, 0x8ACB0, 71},
	{ZoombiniText::kDialogBodyCannotLoadInPractice, 0x8ACF8, 54},
	{ZoombiniText::kDialogBodyCannotCreateNewInPractice, 0x8AD2F, 54},
	{ZoombiniText::kDialogBodyNewGame, 0x8AD66, 9},
	{ZoombiniText::kDialogBodyReallyQuit, 0x8AD70, 28},
	{ZoombiniText::kNotiBoxMusicOn, 0x8B8D6, 13},
	{ZoombiniText::kNotiBoxMusicOff, 0x8B8E4, 16},
	{ZoombiniText::kNotiBoxSoundOn, 0x8B8F5, 10},
	{ZoombiniText::kNotiBoxSoundOff, 0x8B900, 13},
	{ZoombiniText::kNotiBoxLessAction, 0x8B90E, 10},
	{ZoombiniText::kNotiBoxMoreAction, 0x8B919, 9},
	{ZoombiniText::kNotiBoxHideCursor, 0x8B923, 16},
	{ZoombiniText::kNotiBoxShowCursor, 0x8B934, 15},
	{ZoombiniText::kNotiBoxStickeyMouse, 0x8B944, 14},
	{ZoombiniText::kNotiBoxNonStickeyMouse, 0x8B953, 17},
	{ZoombiniText::kNotiBoxTransitionsOn, 0x8B965, 17},
	{ZoombiniText::kNotiBoxTransitionsOff, 0x8B977, 20},
	{ZoombiniText::kNotiBoxAutoStickeyOn, 0x8B98C, 22},
	{ZoombiniText::kNotiBoxAutoStickeyOff, 0x8B9A3, 25},
};

const ZoombiniText::ExeTextEntry ZoombiniText::kV11US_PETextEntries[] = {
	{ZoombiniText::kTown, 0x8AD89, 13},
	{ZoombiniText::kPicker, 0x8ACA4, 13},
	{ZoombiniText::kBridge, 0x8ACB2, 15},
	{ZoombiniText::kTunnels, 0x8ACC2, 16},
	{ZoombiniText::kPizza, 0x8ACD3, 10},
	{ZoombiniText::kBasecamp1, 0x8ACDE, 12},
	{ZoombiniText::kFerry, 0x8ACEB, 25},
	{ZoombiniText::kLilly, 0x8AD05, 22},
	{ZoombiniText::kSlides, 0x8AD1C, 10},
	{ZoombiniText::kFleens, 0x8AD27, 7},
	{ZoombiniText::kHotel, 0x8AD2F, 14},
	{ZoombiniText::kNet, 0x8AD3E, 12},
	{ZoombiniText::kBasecamp2, 0x8AD4B, 10},
	{ZoombiniText::kCaves, 0x8AD56, 15},
	{ZoombiniText::kSmoke, 0x8AD66, 14},
	{ZoombiniText::kMaze, 0x8AD76, 18},
	{ZoombiniText::kNewGame, 0x8B9FC, 8},
	{ZoombiniText::kPracticeMode, 0x8ADCB, 13},
	{ZoombiniText::kContinueJourney, 0x8AEF3, 16},
	{ZoombiniText::kPracticeTitle, 0x8AEE5, 13},
	{ZoombiniText::kPracticeDesc1, 0x8ADD9, 18},
	{ZoombiniText::kPracticeDesc2, 0x8ADEC, 17},
	{ZoombiniText::kPracticeDesc3, 0x8ADFE, 16},
	{ZoombiniText::kPracticeDesc4, 0x8AE0F, 12},
	{ZoombiniText::kTerrainKey, 0x8AF04, 11},
	{ZoombiniText::kChooseLevel, 0x8AF10, 14},
	{ZoombiniText::kLevel1, 0x8AF1F, 11},
	{ZoombiniText::kLevel2, 0x8AF2B, 11},
	{ZoombiniText::kLevel3, 0x8AF37, 9},
	{ZoombiniText::kLevel4, 0x8AF41, 15},
	{ZoombiniText::kRoute1, 0x8AF51, 31},
	{ZoombiniText::kRoute2, 0x8AF71, 11},
	{ZoombiniText::kRoute3, 0x8AF7D, 17},
	{ZoombiniText::kRoute4, 0x8AF8F, 20},
	{ZoombiniText::kXferVillePopulation, 0x8AFA4, 24},
	{ZoombiniText::kMemorialJanuary, 0x8AFBE, 7},
	{ZoombiniText::kMemorialFebruary, 0x8AFC6, 8},
	{ZoombiniText::kMemorialMarch, 0x8AFCF, 5},
	{ZoombiniText::kMemorialApril, 0x8AFD5, 5},
	{ZoombiniText::kMemorialMay, 0x8AFDB, 3},
	{ZoombiniText::kMemorialJune, 0x8AFDF, 4},
	{ZoombiniText::kMemorialJuly, 0x8AFE4, 4},
	{ZoombiniText::kMemorialAugust, 0x8AFE9, 6},
	{ZoombiniText::kMemorialSeptember, 0x8AFF0, 9},
	{ZoombiniText::kMemorialOctober, 0x8AFFA, 7},
	{ZoombiniText::kMemorialNovember, 0x8B002, 8},
	{ZoombiniText::kMemorialDecember, 0x8B00B, 8},
	{ZoombiniText::kMemorialWhenLevel, 0x8B014, 18},
	{ZoombiniText::kMemorialHonorMonument, 0x8B027, 50},
	{ZoombiniText::kMemorialHonorWindmill, 0x8B05A, 53},
	{ZoombiniText::kMemorialHonorObservatory, 0x8B090, 44},
	{ZoombiniText::kMemorialHonorBowlingAlley, 0x8B0BD, 44},
	{ZoombiniText::kMemorialHonorGeneralStore, 0x8B0EA, 53},
	{ZoombiniText::kMemorialHonorSwimmingPool, 0x8B120, 45},
	{ZoombiniText::kMemorialHonorPlayground, 0x8B14E, 55},
	{ZoombiniText::kMemorialHonorBandShell, 0x8B186, 53},
	{ZoombiniText::kMemorialHonorSchool, 0x8B1BC, 43},
	{ZoombiniText::kMemorialHonorLibrary, 0x8B1E8, 51},
	{ZoombiniText::kMemorialHonorFire, 0x8B21C, 40},
	{ZoombiniText::kMemorialHonorOpera, 0x8B245, 52},
	{ZoombiniText::kMemorialHonorCityHall, 0x8B27A, 44},
	{ZoombiniText::kMemorialHonorClockTower, 0x8B2A7, 55},
	{ZoombiniText::kMemorialHonorMuseum, 0x8B2DF, 54},
	{ZoombiniText::kMemorialHonorCourt, 0x8B316, 54},
	{ZoombiniText::kMemorialRoute1Level1, 0x8B34D, 100},
	{ZoombiniText::kMemorialRoute1Level2, 0x8B3B2, 106},
	{ZoombiniText::kMemorialRoute1Level3, 0x8B41D, 102},
	{ZoombiniText::kMemorialRoute1Level4, 0x8B484, 113},
	{ZoombiniText::kMemorialRoute2Level1, 0x8B4F6, 66},
	{ZoombiniText::kMemorialRoute2Level2, 0x8B539, 81},
	{ZoombiniText::kMemorialRoute2Level3, 0x8B58B, 92},
	{ZoombiniText::kMemorialRoute2Level4, 0x8B4F6, 66},
	{ZoombiniText::kMemorialRoute3Level1, 0x8B62B, 106},
	{ZoombiniText::kMemorialRoute3Level2, 0x8B696, 93},
	{ZoombiniText::kMemorialRoute3Level3, 0x8B6F4, 94},
	{ZoombiniText::kMemorialRoute3Level4, 0x8B753, 98},
	{ZoombiniText::kMemorialRoute4Level1, 0x8B7B6, 103},
	{ZoombiniText::kMemorialRoute4Level2, 0x8B81E, 110},
	{ZoombiniText::kMemorialRoute4Level3, 0x8B88D, 91},
	{ZoombiniText::kMemorialRoute4Level4, 0x8B8E9, 134},
	{ZoombiniText::kDialogBodyGoMapWillLost, 0x8B970, 64},
	{ZoombiniText::kDialogButtonLoseThem, 0x8B9B1, 9},
	{ZoombiniText::kDialogButtonKeepThem, 0x8B9BB, 9},
	{ZoombiniText::kDialogButtonOkay, 0x8B9C5, 2},
	{ZoombiniText::kDialogButtonCancel, 0x8B9C8, 6},
	{ZoombiniText::kDialogButtonYes, 0x8BB76, 3},
	{ZoombiniText::kDialogButtonNo, 0x8BBDB, 2},
	{ZoombiniText::kDialogButtonQuit, 0x8BA34, 4},
	{ZoombiniText::kDialogButtonLoad, 0x8B9CF, 4},
	{ZoombiniText::kDialogButtonSave, 0x8B9D4, 4},
	{ZoombiniText::kOptionsTitle, 0x8B9D9, 7},
	{ZoombiniText::kOptionsLegendOn, 0x8B9E1, 4},
	{ZoombiniText::kOptionsLegendOff, 0x8B9E6, 5},
	{ZoombiniText::kOptionsNewGame, 0x8B9FC, 17},
	{ZoombiniText::kOptionsLoadGame, 0x8BA0E, 18},
	{ZoombiniText::kOptionsSaveGame, 0x8BA21, 18},
	{ZoombiniText::kOptionsQuit, 0x8BA34, 13},
	{ZoombiniText::kOptionsToggle, 0x8B9EC, 15},
	{ZoombiniText::kOptionsSound, 0x8BA42, 26},
	{ZoombiniText::kOptionsMusic, 0x8BA5D, 25},
	{ZoombiniText::kOptionsStickyMouse, 0x8BA77, 21},
	{ZoombiniText::kOptionsTransitions, 0x8BA8D, 20},
	{ZoombiniText::kOptionsCredits, 0x8BB56, 7},
	{ZoombiniText::kDialogBodyNoSavedGames, 0x8BAA2, 14},
	{ZoombiniText::kDialogBodyCreateAndSaveNewGame, 0x8BAB4, 56},
	{ZoombiniText::kDialogButtonNewGame, 0x8B9FC, 8},
	{ZoombiniText::kDialogButtonReplaceTitle, 0x8BAF6, 7},
	{ZoombiniText::kDialogTitleSave, 0x8BAFE, 11},
	{ZoombiniText::kDialogTitleSaveAs, 0x8BB0A, 13},
	{ZoombiniText::kDialogTitleLoad, 0x8BB18, 11},
	{ZoombiniText::kDialogBodyReplaceGame, 0x8BB24, 48},
	{ZoombiniText::kDialogBodySaveCurrentGame, 0x8BB5E, 23},
	{ZoombiniText::kDialogBodySaveDirtyGame, 0x8BCFD, 60},
	{ZoombiniText::kDialogBodyCannotSaveInPractice, 0x8BBDE, 42},
	{ZoombiniText::kDialogBodyCreateNewGame, 0x8BC09, 42},
	{ZoombiniText::kDialogBodyCannotSaveMoreGame, 0x8BC34, 70},
	{ZoombiniText::kDialogBodyCannotLoadInPractice, 0x8BC7B, 42},
	{ZoombiniText::kDialogBodyCannotCreateNewInPractice, 0x8BCA6, 48},
	{ZoombiniText::kDialogBodyNewGame, 0x8B9FC, 8},
	{ZoombiniText::kDialogBodyReallyQuit, 0x8BCE0, 28},
	{ZoombiniText::kDialogBodySaveBeforeQuit, 0x8BB7F, 87},
	{ZoombiniText::kDialogHelpTitle, 0x8BD3A, 4},
	{ZoombiniText::kDialogButtonPrev, 0x8BD3F, 8},
	{ZoombiniText::kDialogButtonNext, 0x8BD48, 4},
	{ZoombiniText::kDialogHelpLevel, 0x8636C, 5},
	{ZoombiniText::kDialogHelpPickerUpdateVersion, 0x8D46D, 27},
	{ZoombiniText::kNotiBoxMusicOn, 0x8C77B, 8},
	{ZoombiniText::kNotiBoxMusicOff, 0x8C784, 9},
	{ZoombiniText::kNotiBoxSoundOn, 0x8C78E, 8},
	{ZoombiniText::kNotiBoxSoundOff, 0x8C797, 9},
	{ZoombiniText::kNotiBoxLessAction, 0x8C7A1, 11},
	{ZoombiniText::kNotiBoxMoreAction, 0x8C7AD, 11},
	{ZoombiniText::kNotiBoxHideCursor, 0x8C7B9, 11},
	{ZoombiniText::kNotiBoxShowCursor, 0x8C7C5, 11},
	{ZoombiniText::kNotiBoxStickeyMouse, 0x8C7D1, 12},
	{ZoombiniText::kNotiBoxNonStickeyMouse, 0x8C7DE, 16},
	{ZoombiniText::kNotiBoxTransitionsOn, 0x8C7EF, 14},
	{ZoombiniText::kNotiBoxTransitionsOff, 0x8C7FE, 15},
	{ZoombiniText::kNotiBoxAutoStickeyOn, 0x8C80E, 14},
	{ZoombiniText::kNotiBoxAutoStickeyOff, 0x8C81D, 15},
};

const ZoombiniText::ExeTextEntry ZoombiniText::kV11US_NETextEntries[] = {
	{ZoombiniText::kTown, 0xDBF3F, 13},
	{ZoombiniText::kPicker, 0xDBE5A, 13},
	{ZoombiniText::kBridge, 0xDBE68, 15},
	{ZoombiniText::kTunnels, 0xDBE78, 16},
	{ZoombiniText::kPizza, 0xDBE89, 10},
	{ZoombiniText::kBasecamp1, 0xDBE94, 12},
	{ZoombiniText::kFerry, 0xDBEA1, 25},
	{ZoombiniText::kLilly, 0xDBEBB, 22},
	{ZoombiniText::kSlides, 0xDBED2, 10},
	{ZoombiniText::kFleens, 0xDBEDD, 7},
	{ZoombiniText::kHotel, 0xDBEE5, 14},
	{ZoombiniText::kNet, 0xDBEF4, 12},
	{ZoombiniText::kBasecamp2, 0xDBF01, 10},
	{ZoombiniText::kCaves, 0xDBF0C, 15},
	{ZoombiniText::kSmoke, 0xDBF1C, 14},
	{ZoombiniText::kMaze, 0xDBF2C, 18},
	{ZoombiniText::kNewGame, 0xDCBB2, 8},
	{ZoombiniText::kPracticeMode, 0xDBF81, 13},
	{ZoombiniText::kContinueJourney, 0xDC0A9, 16},
	{ZoombiniText::kPracticeTitle, 0xDC09B, 13},
	{ZoombiniText::kPracticeDesc1, 0xDBF8F, 18},
	{ZoombiniText::kPracticeDesc2, 0xDBFA2, 17},
	{ZoombiniText::kPracticeDesc3, 0xDBFB4, 16},
	{ZoombiniText::kPracticeDesc4, 0xDBFC5, 12},
	{ZoombiniText::kTerrainKey, 0xDC0BA, 11},
	{ZoombiniText::kChooseLevel, 0xDC0C6, 14},
	{ZoombiniText::kLevel1, 0xDC0D5, 11},
	{ZoombiniText::kLevel2, 0xDC0E1, 11},
	{ZoombiniText::kLevel3, 0xDC0ED, 9},
	{ZoombiniText::kLevel4, 0xDC0F7, 15},
	{ZoombiniText::kRoute1, 0xDC107, 31},
	{ZoombiniText::kRoute2, 0xDC127, 11},
	{ZoombiniText::kRoute3, 0xDC133, 17},
	{ZoombiniText::kRoute4, 0xDC145, 20},
	{ZoombiniText::kXferVillePopulation, 0xDC15A, 24},
	{ZoombiniText::kMemorialJanuary, 0xDC174, 7},
	{ZoombiniText::kMemorialFebruary, 0xDC17C, 8},
	{ZoombiniText::kMemorialMarch, 0xDC185, 5},
	{ZoombiniText::kMemorialApril, 0xDC18B, 5},
	{ZoombiniText::kMemorialMay, 0xDC191, 3},
	{ZoombiniText::kMemorialJune, 0xDC195, 4},
	{ZoombiniText::kMemorialJuly, 0xDC19A, 4},
	{ZoombiniText::kMemorialAugust, 0xDC19F, 6},
	{ZoombiniText::kMemorialSeptember, 0xDC1A6, 9},
	{ZoombiniText::kMemorialOctober, 0xDC1B0, 7},
	{ZoombiniText::kMemorialNovember, 0xDC1B8, 8},
	{ZoombiniText::kMemorialDecember, 0xDC1C1, 8},
	{ZoombiniText::kMemorialWhenLevel, 0xDC1CA, 18},
	{ZoombiniText::kMemorialHonorMonument, 0xDC1DD, 50},
	{ZoombiniText::kMemorialHonorWindmill, 0xDC210, 53},
	{ZoombiniText::kMemorialHonorObservatory, 0xDC246, 44},
	{ZoombiniText::kMemorialHonorBowlingAlley, 0xDC273, 44},
	{ZoombiniText::kMemorialHonorGeneralStore, 0xDC2A0, 53},
	{ZoombiniText::kMemorialHonorSwimmingPool, 0xDC2D6, 45},
	{ZoombiniText::kMemorialHonorPlayground, 0xDC304, 55},
	{ZoombiniText::kMemorialHonorBandShell, 0xDC33C, 53},
	{ZoombiniText::kMemorialHonorSchool, 0xDC372, 43},
	{ZoombiniText::kMemorialHonorLibrary, 0xDC39E, 51},
	{ZoombiniText::kMemorialHonorFire, 0xDC3D2, 40},
	{ZoombiniText::kMemorialHonorOpera, 0xDC3FB, 52},
	{ZoombiniText::kMemorialHonorCityHall, 0xDC430, 44},
	{ZoombiniText::kMemorialHonorClockTower, 0xDC45D, 55},
	{ZoombiniText::kMemorialHonorMuseum, 0xDC495, 54},
	{ZoombiniText::kMemorialHonorCourt, 0xDC4CC, 54},
	{ZoombiniText::kMemorialRoute1Level1, 0xDC503, 100},
	{ZoombiniText::kMemorialRoute1Level2, 0xDC568, 106},
	{ZoombiniText::kMemorialRoute1Level3, 0xDC5D3, 102},
	{ZoombiniText::kMemorialRoute1Level4, 0xDC63A, 113},
	{ZoombiniText::kMemorialRoute2Level1, 0xDC6AC, 66},
	{ZoombiniText::kMemorialRoute2Level2, 0xDC6EF, 81},
	{ZoombiniText::kMemorialRoute2Level3, 0xDC741, 92},
	{ZoombiniText::kMemorialRoute2Level4, 0xDC6AC, 66},
	{ZoombiniText::kMemorialRoute3Level1, 0xDC7E1, 106},
	{ZoombiniText::kMemorialRoute3Level2, 0xDC84C, 93},
	{ZoombiniText::kMemorialRoute3Level3, 0xDC8AA, 94},
	{ZoombiniText::kMemorialRoute3Level4, 0xDC909, 98},
	{ZoombiniText::kMemorialRoute4Level1, 0xDC96C, 103},
	{ZoombiniText::kMemorialRoute4Level2, 0xDC9D4, 110},
	{ZoombiniText::kMemorialRoute4Level3, 0xDCA43, 91},
	{ZoombiniText::kMemorialRoute4Level4, 0xDCA9F, 134},
	{ZoombiniText::kDialogBodyGoMapWillLost, 0xDCB26, 64},
	{ZoombiniText::kDialogButtonLoseThem, 0xDCB67, 9},
	{ZoombiniText::kDialogButtonKeepThem, 0xDCB71, 9},
	{ZoombiniText::kDialogButtonOkay, 0xDCB7B, 2},
	{ZoombiniText::kDialogButtonCancel, 0xDCB7E, 6},
	{ZoombiniText::kDialogButtonYes, 0xDCD2C, 3},
	{ZoombiniText::kDialogButtonNo, 0xDCD91, 2},
	{ZoombiniText::kDialogButtonQuit, 0xDCBEA, 4},
	{ZoombiniText::kDialogButtonLoad, 0xDCB85, 4},
	{ZoombiniText::kDialogButtonSave, 0xDCB8A, 4},
	{ZoombiniText::kOptionsTitle, 0xDCB8F, 7},
	{ZoombiniText::kOptionsLegendOn, 0xDCB97, 4},
	{ZoombiniText::kOptionsLegendOff, 0xDCB9C, 5},
	{ZoombiniText::kOptionsNewGame, 0xDCBB2, 17},
	{ZoombiniText::kOptionsLoadGame, 0xDCBC4, 18},
	{ZoombiniText::kOptionsSaveGame, 0xDCBD7, 18},
	{ZoombiniText::kOptionsQuit, 0xDCBEA, 13},
	{ZoombiniText::kOptionsToggle, 0xDCBA2, 15},
	{ZoombiniText::kOptionsSound, 0xDCBF8, 26},
	{ZoombiniText::kOptionsMusic, 0xDCC13, 25},
	{ZoombiniText::kOptionsStickyMouse, 0xDCC2D, 21},
	{ZoombiniText::kOptionsTransitions, 0xDCC43, 20},
	{ZoombiniText::kOptionsCredits, 0xDCD0C, 7},
	{ZoombiniText::kDialogBodyNoSavedGames, 0xDCC58, 14},
	{ZoombiniText::kDialogBodyCreateAndSaveNewGame, 0xDCC6A, 56},
	{ZoombiniText::kDialogButtonNewGame, 0xDCBB2, 8},
	{ZoombiniText::kDialogButtonReplaceTitle, 0xDCCAC, 7},
	{ZoombiniText::kDialogTitleSave, 0xDCCB4, 11},
	{ZoombiniText::kDialogTitleSaveAs, 0xDCCC0, 13},
	{ZoombiniText::kDialogTitleLoad, 0xDCCCE, 11},
	{ZoombiniText::kDialogBodyReplaceGame, 0xDCCDA, 48},
	{ZoombiniText::kDialogBodySaveCurrentGame, 0xDCD14, 23},
	{ZoombiniText::kDialogBodySaveDirtyGame, 0xDCEB3, 60},
	{ZoombiniText::kDialogBodyCannotSaveInPractice, 0xDCD94, 42},
	{ZoombiniText::kDialogBodyCreateNewGame, 0xDCDBF, 42},
	{ZoombiniText::kDialogBodyCannotSaveMoreGame, 0xDCDEA, 70},
	{ZoombiniText::kDialogBodyCannotLoadInPractice, 0xDCE31, 42},
	{ZoombiniText::kDialogBodyCannotCreateNewInPractice, 0xDCE5C, 48},
	{ZoombiniText::kDialogBodyNewGame, 0xDCBB2, 8},
	{ZoombiniText::kDialogBodyReallyQuit, 0xDCE96, 28},
	{ZoombiniText::kDialogBodySaveBeforeQuit, 0xDCD35, 87},
	{ZoombiniText::kDialogHelpTitle, 0xDCEF0, 4},
	{ZoombiniText::kDialogButtonPrev, 0xDCEF5, 8},
	{ZoombiniText::kDialogButtonNext, 0xDCEFE, 4},
	{ZoombiniText::kDialogHelpLevel, 0xD9E9C, 5},
	{ZoombiniText::kDialogHelpPickerUpdateVersion, 0xDE1BF, 27},
	{ZoombiniText::kNotiBoxMusicOn, 0xDD931, 8},
	{ZoombiniText::kNotiBoxMusicOff, 0xDD93A, 9},
	{ZoombiniText::kNotiBoxSoundOn, 0xDD944, 8},
	{ZoombiniText::kNotiBoxSoundOff, 0xDD94D, 9},
	{ZoombiniText::kNotiBoxLessAction, 0xDD957, 11},
	{ZoombiniText::kNotiBoxMoreAction, 0xDD963, 11},
	{ZoombiniText::kNotiBoxHideCursor, 0xDD96F, 11},
	{ZoombiniText::kNotiBoxShowCursor, 0xDD97B, 11},
	{ZoombiniText::kNotiBoxStickeyMouse, 0xDD987, 12},
	{ZoombiniText::kNotiBoxNonStickeyMouse, 0xDD994, 16},
	{ZoombiniText::kNotiBoxTransitionsOn, 0xDD9A5, 14},
	{ZoombiniText::kNotiBoxTransitionsOff, 0xDD9B4, 15},
	{ZoombiniText::kNotiBoxAutoStickeyOn, 0xDD9C4, 14},
	{ZoombiniText::kNotiBoxAutoStickeyOff, 0xDD9D3, 15},
};

const ZoombiniText::ExeTextEntry ZoombiniText::kV10US_NETextEntries[] = {
	{ZoombiniText::kTown, 0xD955D, 13},
	{ZoombiniText::kPicker, 0xD9478, 13},
	{ZoombiniText::kBridge, 0xD9486, 15},
	{ZoombiniText::kTunnels, 0xD9496, 16},
	{ZoombiniText::kPizza, 0xD94A7, 10},
	{ZoombiniText::kBasecamp1, 0xD94B2, 12},
	{ZoombiniText::kFerry, 0xD94BF, 25},
	{ZoombiniText::kLilly, 0xD94D9, 22},
	{ZoombiniText::kSlides, 0xD94F0, 10},
	{ZoombiniText::kFleens, 0xD94FB, 7},
	{ZoombiniText::kHotel, 0xD9503, 14},
	{ZoombiniText::kNet, 0xD9512, 12},
	{ZoombiniText::kBasecamp2, 0xD951F, 10},
	{ZoombiniText::kCaves, 0xD952A, 15},
	{ZoombiniText::kSmoke, 0xD953A, 14},
	{ZoombiniText::kMaze, 0xD954A, 18},
	{ZoombiniText::kNewGame, 0xD0000, 8},
	{ZoombiniText::kJourneyStatZoombiniIsle, 0xD9478, 13},
	{ZoombiniText::kJourneyStatShelterRock, 0xD94B2, 12},
	{ZoombiniText::kJourneyStatShadeTree, 0xD951F, 10},
	{ZoombiniText::kJourneyStatZoombiniton, 0xD955D, 13},
	{ZoombiniText::kPracticeMode, 0xD959F, 13},
	{ZoombiniText::kPracticeReturnInitial1, 0xD95AD, 18},
	{ZoombiniText::kPracticeReturnInitial2, 0xD95C0, 19},
	{ZoombiniText::kPracticeReturnInitial3, 0xD9480, 1},
	{ZoombiniText::kPracticeReturnInitial4, 0xD9480, 1},
	{ZoombiniText::kPracticeReturnShelterRock1, 0xD95AD, 18},
	{ZoombiniText::kPracticeReturnShelterRock2, 0xD95C0, 19},
	{ZoombiniText::kPracticeReturnShelterRock3, 0xD95FF, 15},
	{ZoombiniText::kPracticeReturnShelterRock4, 0xD9480, 1},
	{ZoombiniText::kPracticeReturnShadeTree1, 0xD95AD, 18},
	{ZoombiniText::kPracticeReturnShadeTree2, 0xD9624, 20},
	{ZoombiniText::kPracticeReturnShadeTree3, 0xD9639, 15},
	{ZoombiniText::kPracticeReturnShadeTree4, 0xD951F, 10},
	{ZoombiniText::kPracticeReturnZoombiniton1, 0xD95AD, 18},
	{ZoombiniText::kPracticeReturnZoombiniton2, 0xD9624, 20},
	{ZoombiniText::kPracticeReturnZoombiniton3, 0xD967C, 16},
	{ZoombiniText::kPracticeReturnZoombiniton4, 0xD968D, 16},
	{ZoombiniText::kTerrainKey, 0xD969E, 11},
	{ZoombiniText::kChooseLevel, 0xD96AA, 14},
	{ZoombiniText::kLevel1, 0xD96B9, 11},
	{ZoombiniText::kLevel2, 0xD96C5, 11},
	{ZoombiniText::kLevel3, 0xD96D1, 9},
	{ZoombiniText::kLevel4, 0xD96DB, 15},
	{ZoombiniText::kRoute1, 0xD96EB, 31},
	{ZoombiniText::kRoute2, 0xD970B, 11},
	{ZoombiniText::kRoute3, 0xD9717, 17},
	{ZoombiniText::kRoute4, 0xD9729, 20},
	{ZoombiniText::kXferVillePopulation, 0xD973E, 24},
	{ZoombiniText::kMemorialJanuary, 0xD9758, 7},
	{ZoombiniText::kMemorialFebruary, 0xD9760, 8},
	{ZoombiniText::kMemorialMarch, 0xD9769, 5},
	{ZoombiniText::kMemorialApril, 0xD976F, 5},
	{ZoombiniText::kMemorialMay, 0xD9775, 3},
	{ZoombiniText::kMemorialJune, 0xD9779, 4},
	{ZoombiniText::kMemorialJuly, 0xD977E, 4},
	{ZoombiniText::kMemorialAugust, 0xD9783, 6},
	{ZoombiniText::kMemorialSeptember, 0xD978A, 9},
	{ZoombiniText::kMemorialOctober, 0xD9794, 7},
	{ZoombiniText::kMemorialNovember, 0xD979C, 8},
	{ZoombiniText::kMemorialDecember, 0xD97A5, 8},
	{ZoombiniText::kMemorialWhenLevel, 0xD97AE, 18},
	{ZoombiniText::kMemorialHonorMonument, 0xD97C1, 50},
	{ZoombiniText::kMemorialHonorWindmill, 0xD97F4, 53},
	{ZoombiniText::kMemorialHonorObservatory, 0xD982A, 44},
	{ZoombiniText::kMemorialHonorBowlingAlley, 0xD9857, 44},
	{ZoombiniText::kMemorialHonorGeneralStore, 0xD9884, 53},
	{ZoombiniText::kMemorialHonorSwimmingPool, 0xD98BA, 45},
	{ZoombiniText::kMemorialHonorPlayground, 0xD98E8, 55},
	{ZoombiniText::kMemorialHonorBandShell, 0xD9920, 53},
	{ZoombiniText::kMemorialHonorSchool, 0xD9956, 43},
	{ZoombiniText::kMemorialHonorLibrary, 0xD9982, 51},
	{ZoombiniText::kMemorialHonorFire, 0xD99B6, 40},
	{ZoombiniText::kMemorialHonorOpera, 0xD99DF, 52},
	{ZoombiniText::kMemorialHonorCityHall, 0xD9A14, 44},
	{ZoombiniText::kMemorialHonorClockTower, 0xD9A41, 55},
	{ZoombiniText::kMemorialHonorMuseum, 0xD9A79, 54},
	{ZoombiniText::kMemorialHonorCourt, 0xD9AB0, 54},
	{ZoombiniText::kMemorialRoute1Level1, 0xD9AE7, 100},
	{ZoombiniText::kMemorialRoute1Level2, 0xD9B4C, 106},
	{ZoombiniText::kMemorialRoute1Level3, 0xD9BB7, 102},
	{ZoombiniText::kMemorialRoute1Level4, 0xD9C1E, 113},
	{ZoombiniText::kMemorialRoute2Level1, 0xD9C90, 66},
	{ZoombiniText::kMemorialRoute2Level2, 0xD9CD3, 81},
	{ZoombiniText::kMemorialRoute2Level3, 0xD9D25, 92},
	{ZoombiniText::kMemorialRoute2Level4, 0xD9C90, 66},
	{ZoombiniText::kMemorialRoute3Level1, 0xD9DC5, 106},
	{ZoombiniText::kMemorialRoute3Level2, 0xD9E30, 93},
	{ZoombiniText::kMemorialRoute3Level3, 0xD9E8E, 94},
	{ZoombiniText::kMemorialRoute3Level4, 0xD9EED, 98},
	{ZoombiniText::kMemorialRoute4Level1, 0xD9F50, 103},
	{ZoombiniText::kMemorialRoute4Level2, 0xD9FB8, 110},
	{ZoombiniText::kMemorialRoute4Level3, 0xDA027, 91},
	{ZoombiniText::kMemorialRoute4Level4, 0xDA083, 134},
	{ZoombiniText::kDialogBodyGoMapWillLost, 0xDA10A, 64},
	{ZoombiniText::kDialogButtonLoseThem, 0xDA14B, 9},
	{ZoombiniText::kDialogButtonKeepThem, 0xDA155, 9},
	{ZoombiniText::kDialogButtonOkay, 0xDA15F, 2},
	{ZoombiniText::kDialogButtonCancel, 0xDA162, 6},
	{ZoombiniText::kDialogButtonYes, 0xDA310, 3},
	{ZoombiniText::kDialogButtonNo, 0xD8C04, 2},
	{ZoombiniText::kDialogButtonQuit, 0xDA1CE, 4},
	{ZoombiniText::kDialogButtonLoad, 0xDA169, 4},
	{ZoombiniText::kDialogButtonSave, 0xDA16E, 4},
	{ZoombiniText::kOptionsTitle, 0xDA173, 7},
	{ZoombiniText::kOptionsLegendOn, 0xDA17B, 4},
	{ZoombiniText::kOptionsLegendOff, 0xDA180, 5},
	{ZoombiniText::kOptionsNewGame, 0xDA196, 17},
	{ZoombiniText::kOptionsLoadGame, 0xDA1A8, 18},
	{ZoombiniText::kOptionsSaveGame, 0xDA1BB, 18},
	{ZoombiniText::kOptionsQuit, 0xDA1CE, 13},
	{ZoombiniText::kOptionsToggle, 0xDA186, 15},
	{ZoombiniText::kOptionsSound, 0xDA1DC, 26},
	{ZoombiniText::kOptionsMusic, 0xDA1F7, 25},
	{ZoombiniText::kOptionsStickyMouse, 0xDA211, 21},
	{ZoombiniText::kOptionsTransitions, 0xDA227, 20},
	{ZoombiniText::kOptionsCredits, 0xDA2F0, 7},
	{ZoombiniText::kDialogBodyNoSavedGames, 0xDA23C, 14},
	{ZoombiniText::kDialogBodyCreateAndSaveNewGame, 0xDA24E, 56},
	{ZoombiniText::kDialogButtonNewGame, 0xDA196, 8},
	{ZoombiniText::kDialogButtonReplaceTitle, 0xDA290, 7},
	{ZoombiniText::kDialogTitleSave, 0xDA298, 11},
	{ZoombiniText::kDialogTitleSaveAs, 0xDA2A4, 13},
	{ZoombiniText::kDialogTitleLoad, 0xDA2B2, 11},
	{ZoombiniText::kDialogBodyReplaceGame, 0xDA2BE, 48},
	{ZoombiniText::kDialogBodySaveCurrentGame, 0xDA2F8, 23},
	{ZoombiniText::kDialogBodyCannotSaveInPractice, 0xDA378, 42},
	{ZoombiniText::kDialogBodyCreateNewGame, 0xDA3A3, 42},
	{ZoombiniText::kDialogBodyCannotSaveMoreGame, 0xDA3CE, 70},
	{ZoombiniText::kDialogBodyCannotLoadInPractice, 0xDA415, 42},
	{ZoombiniText::kDialogBodyCannotCreateNewInPractice, 0xDA440, 48},
	{ZoombiniText::kDialogBodyNewGame, 0xDA196, 8},
	{ZoombiniText::kDialogBodyReallyQuit, 0xDA47A, 28},
	{ZoombiniText::kDialogBodySaveBeforeQuit, 0xDA319, 87},
	{ZoombiniText::kDialogHelpLevel, 0xD8A90, 5},
	{ZoombiniText::kNotiBoxMusicOn, 0xDAEBC, 8},
	{ZoombiniText::kNotiBoxMusicOff, 0xDAEC5, 9},
	{ZoombiniText::kNotiBoxSoundOn, 0xDAECF, 8},
	{ZoombiniText::kNotiBoxSoundOff, 0xDAED8, 9},
	{ZoombiniText::kNotiBoxLessAction, 0xDAEE2, 11},
	{ZoombiniText::kNotiBoxMoreAction, 0xDAEEE, 11},
	{ZoombiniText::kNotiBoxHideCursor, 0xDAEFA, 11},
	{ZoombiniText::kNotiBoxShowCursor, 0xDAF06, 11},
	{ZoombiniText::kNotiBoxStickeyMouse, 0xDAF12, 12},
	{ZoombiniText::kNotiBoxNonStickeyMouse, 0xDAF1F, 16},
	{ZoombiniText::kNotiBoxTransitionsOn, 0xDAF30, 14},
	{ZoombiniText::kNotiBoxTransitionsOff, 0xDAF3F, 15},
	{ZoombiniText::kNotiBoxAutoStickeyOn, 0xDAF4F, 14},
	{ZoombiniText::kNotiBoxAutoStickeyOff, 0xDAF5E, 15},
};

const ZoombiniText::ExeTextEntry ZoombiniText::kV10US_PETextEntries[] = {
	{ZoombiniText::kTown, 0x899A9, 13},
	{ZoombiniText::kPicker, 0x898C4, 13},
	{ZoombiniText::kBridge, 0x898D2, 15},
	{ZoombiniText::kTunnels, 0x898E2, 16},
	{ZoombiniText::kPizza, 0x898F3, 10},
	{ZoombiniText::kBasecamp1, 0x898FE, 12},
	{ZoombiniText::kFerry, 0x8990B, 25},
	{ZoombiniText::kLilly, 0x89925, 22},
	{ZoombiniText::kSlides, 0x8993C, 10},
	{ZoombiniText::kFleens, 0x89947, 7},
	{ZoombiniText::kHotel, 0x8994F, 14},
	{ZoombiniText::kNet, 0x8995E, 12},
	{ZoombiniText::kBasecamp2, 0x8996B, 10},
	{ZoombiniText::kCaves, 0x89976, 15},
	{ZoombiniText::kSmoke, 0x89986, 14},
	{ZoombiniText::kMaze, 0x89996, 18},
	{ZoombiniText::kNewGame, 0x888EA, 8},
	{ZoombiniText::kJourneyStatZoombiniIsle, 0x898C4, 13},
	{ZoombiniText::kJourneyStatShelterRock, 0x898FE, 12},
	{ZoombiniText::kJourneyStatShadeTree, 0x8996B, 10},
	{ZoombiniText::kJourneyStatZoombiniton, 0x899A9, 13},
	{ZoombiniText::kPracticeMode, 0x899EB, 13},
	{ZoombiniText::kPracticeReturnInitial1, 0x899F9, 18},
	{ZoombiniText::kPracticeReturnInitial2, 0x89A0C, 19},
	{ZoombiniText::kPracticeReturnInitial3, 0x898CC, 1},
	{ZoombiniText::kPracticeReturnInitial4, 0x898CC, 1},
	{ZoombiniText::kPracticeReturnShelterRock1, 0x899F9, 18},
	{ZoombiniText::kPracticeReturnShelterRock2, 0x89A0C, 19},
	{ZoombiniText::kPracticeReturnShelterRock3, 0x89A4B, 15},
	{ZoombiniText::kPracticeReturnShelterRock4, 0x898CC, 1},
	{ZoombiniText::kPracticeReturnShadeTree1, 0x899F9, 18},
	{ZoombiniText::kPracticeReturnShadeTree2, 0x89A70, 20},
	{ZoombiniText::kPracticeReturnShadeTree3, 0x89A85, 15},
	{ZoombiniText::kPracticeReturnShadeTree4, 0x8996B, 10},
	{ZoombiniText::kPracticeReturnZoombiniton1, 0x899F9, 18},
	{ZoombiniText::kPracticeReturnZoombiniton2, 0x89A70, 20},
	{ZoombiniText::kPracticeReturnZoombiniton3, 0x89AC8, 16},
	{ZoombiniText::kPracticeReturnZoombiniton4, 0x89AD9, 16},
	{ZoombiniText::kTerrainKey, 0x89AEA, 11},
	{ZoombiniText::kChooseLevel, 0x89AF6, 14},
	{ZoombiniText::kLevel1, 0x89B05, 11},
	{ZoombiniText::kLevel2, 0x89B11, 11},
	{ZoombiniText::kLevel3, 0x89B1D, 9},
	{ZoombiniText::kLevel4, 0x89B27, 15},
	{ZoombiniText::kRoute1, 0x89B37, 31},
	{ZoombiniText::kRoute2, 0x89B57, 11},
	{ZoombiniText::kRoute3, 0x89B63, 17},
	{ZoombiniText::kRoute4, 0x89B75, 20},
	{ZoombiniText::kXferVillePopulation, 0x89B8A, 24},
	{ZoombiniText::kMemorialJanuary, 0x89BA4, 7},
	{ZoombiniText::kMemorialFebruary, 0x89BAC, 8},
	{ZoombiniText::kMemorialMarch, 0x89BB5, 5},
	{ZoombiniText::kMemorialApril, 0x89BBB, 5},
	{ZoombiniText::kMemorialMay, 0x89BC1, 3},
	{ZoombiniText::kMemorialJune, 0x89BC5, 4},
	{ZoombiniText::kMemorialJuly, 0x89BCA, 4},
	{ZoombiniText::kMemorialAugust, 0x89BCF, 6},
	{ZoombiniText::kMemorialSeptember, 0x89BD6, 9},
	{ZoombiniText::kMemorialOctober, 0x89BE0, 7},
	{ZoombiniText::kMemorialNovember, 0x89BE8, 8},
	{ZoombiniText::kMemorialDecember, 0x89BF1, 8},
	{ZoombiniText::kMemorialWhenLevel, 0x89BFA, 18},
	{ZoombiniText::kMemorialHonorMonument, 0x89C0D, 50},
	{ZoombiniText::kMemorialHonorWindmill, 0x89C40, 53},
	{ZoombiniText::kMemorialHonorObservatory, 0x89C76, 44},
	{ZoombiniText::kMemorialHonorBowlingAlley, 0x89CA3, 44},
	{ZoombiniText::kMemorialHonorGeneralStore, 0x89CD0, 53},
	{ZoombiniText::kMemorialHonorSwimmingPool, 0x89D06, 45},
	{ZoombiniText::kMemorialHonorPlayground, 0x89D34, 55},
	{ZoombiniText::kMemorialHonorBandShell, 0x89D6C, 53},
	{ZoombiniText::kMemorialHonorSchool, 0x89DA2, 43},
	{ZoombiniText::kMemorialHonorLibrary, 0x89DCE, 51},
	{ZoombiniText::kMemorialHonorFire, 0x89E02, 40},
	{ZoombiniText::kMemorialHonorOpera, 0x89E2B, 52},
	{ZoombiniText::kMemorialHonorCityHall, 0x89E60, 44},
	{ZoombiniText::kMemorialHonorClockTower, 0x89E8D, 55},
	{ZoombiniText::kMemorialHonorMuseum, 0x89EC5, 54},
	{ZoombiniText::kMemorialHonorCourt, 0x89EFC, 54},
	{ZoombiniText::kMemorialRoute1Level1, 0x89F33, 100},
	{ZoombiniText::kMemorialRoute1Level2, 0x89F98, 106},
	{ZoombiniText::kMemorialRoute1Level3, 0x8A003, 102},
	{ZoombiniText::kMemorialRoute1Level4, 0x8A06A, 113},
	{ZoombiniText::kMemorialRoute2Level1, 0x8A0DC, 66},
	{ZoombiniText::kMemorialRoute2Level2, 0x8A11F, 81},
	{ZoombiniText::kMemorialRoute2Level3, 0x8A171, 92},
	{ZoombiniText::kMemorialRoute2Level4, 0x8A0DC, 66},
	{ZoombiniText::kMemorialRoute3Level1, 0x8A211, 106},
	{ZoombiniText::kMemorialRoute3Level2, 0x8A27C, 93},
	{ZoombiniText::kMemorialRoute3Level3, 0x8A2DA, 94},
	{ZoombiniText::kMemorialRoute3Level4, 0x8A339, 98},
	{ZoombiniText::kMemorialRoute4Level1, 0x8A39C, 103},
	{ZoombiniText::kMemorialRoute4Level2, 0x8A404, 110},
	{ZoombiniText::kMemorialRoute4Level3, 0x8A473, 91},
	{ZoombiniText::kMemorialRoute4Level4, 0x8A4CF, 134},
	{ZoombiniText::kDialogBodyGoMapWillLost, 0x8A556, 64},
	{ZoombiniText::kDialogButtonLoseThem, 0x8A597, 9},
	{ZoombiniText::kDialogButtonKeepThem, 0x8A5A1, 9},
	{ZoombiniText::kDialogButtonOkay, 0x8A5AB, 2},
	{ZoombiniText::kDialogButtonCancel, 0x8A5AE, 6},
	{ZoombiniText::kDialogButtonYes, 0x8A75C, 3},
	{ZoombiniText::kDialogButtonNo, 0x88D66, 2},
	{ZoombiniText::kDialogButtonQuit, 0x8A61A, 4},
	{ZoombiniText::kDialogButtonLoad, 0x8A5B5, 4},
	{ZoombiniText::kDialogButtonSave, 0x8A5BA, 4},
	{ZoombiniText::kOptionsTitle, 0x8A5BF, 7},
	{ZoombiniText::kOptionsLegendOn, 0x8A5C7, 4},
	{ZoombiniText::kOptionsLegendOff, 0x8A5CC, 5},
	{ZoombiniText::kOptionsNewGame, 0x8A5E2, 17},
	{ZoombiniText::kOptionsLoadGame, 0x8A5F4, 18},
	{ZoombiniText::kOptionsSaveGame, 0x8A607, 18},
	{ZoombiniText::kOptionsQuit, 0x8A61A, 13},
	{ZoombiniText::kOptionsToggle, 0x8A5D2, 15},
	{ZoombiniText::kOptionsSound, 0x8A628, 26},
	{ZoombiniText::kOptionsMusic, 0x8A643, 25},
	{ZoombiniText::kOptionsStickyMouse, 0x8A65D, 21},
	{ZoombiniText::kOptionsTransitions, 0x8A673, 20},
	{ZoombiniText::kOptionsCredits, 0x8A73C, 7},
	{ZoombiniText::kDialogBodyNoSavedGames, 0x8A688, 14},
	{ZoombiniText::kDialogBodyCreateAndSaveNewGame, 0x8A69A, 56},
	{ZoombiniText::kDialogButtonNewGame, 0x8A5E2, 8},
	{ZoombiniText::kDialogButtonReplaceTitle, 0x8A6DC, 7},
	{ZoombiniText::kDialogTitleSave, 0x8A6E4, 11},
	{ZoombiniText::kDialogTitleSaveAs, 0x8A6F0, 13},
	{ZoombiniText::kDialogTitleLoad, 0x8A6FE, 11},
	{ZoombiniText::kDialogBodyReplaceGame, 0x8A70A, 48},
	{ZoombiniText::kDialogBodySaveCurrentGame, 0x8A744, 23},
	{ZoombiniText::kDialogBodyCannotSaveInPractice, 0x8A7C4, 42},
	{ZoombiniText::kDialogBodyCreateNewGame, 0x8A7EF, 42},
	{ZoombiniText::kDialogBodyCannotSaveMoreGame, 0x8A81A, 70},
	{ZoombiniText::kDialogBodyCannotLoadInPractice, 0x8A861, 42},
	{ZoombiniText::kDialogBodyCannotCreateNewInPractice, 0x8A88C, 48},
	{ZoombiniText::kDialogBodyNewGame, 0x8A5E2, 8},
	{ZoombiniText::kDialogBodyReallyQuit, 0x8A8C6, 28},
	{ZoombiniText::kDialogBodySaveBeforeQuit, 0x8A765, 87},
	{ZoombiniText::kDialogHelpLevel, 0x8887E, 5},
	{ZoombiniText::kNotiBoxMusicOn, 0x8B308, 8},
	{ZoombiniText::kNotiBoxMusicOff, 0x8B311, 9},
	{ZoombiniText::kNotiBoxSoundOn, 0x8B31B, 8},
	{ZoombiniText::kNotiBoxSoundOff, 0x8B324, 9},
	{ZoombiniText::kNotiBoxLessAction, 0x8B32E, 11},
	{ZoombiniText::kNotiBoxMoreAction, 0x8B33A, 11},
	{ZoombiniText::kNotiBoxHideCursor, 0x8B346, 11},
	{ZoombiniText::kNotiBoxShowCursor, 0x8B352, 11},
	{ZoombiniText::kNotiBoxStickeyMouse, 0x8B35E, 12},
	{ZoombiniText::kNotiBoxNonStickeyMouse, 0x8B36B, 16},
	{ZoombiniText::kNotiBoxTransitionsOn, 0x8B37C, 14},
	{ZoombiniText::kNotiBoxTransitionsOff, 0x8B38B, 15},
	{ZoombiniText::kNotiBoxAutoStickeyOn, 0x8B39B, 14},
	{ZoombiniText::kNotiBoxAutoStickeyOff, 0x8B3AA, 15},
};

const ZoombiniText::ExeTextEntry ZoombiniText::kV11USMacTextEntries[] = {
	{ZoombiniText::kTown, 0x9B96, 13},
	{ZoombiniText::kPicker, 0x9E70, 13},
	{ZoombiniText::kBridge, 0x9720, 15},
	{ZoombiniText::kTunnels, 0x973F, 16},
	{ZoombiniText::kPizza, 0x9E43, 10},
	{ZoombiniText::kBasecamp1, 0x9E36, 12},
	{ZoombiniText::kFerry, 0x9E1C, 25},
	{ZoombiniText::kLilly, 0x9E05, 22},
	{ZoombiniText::kSlides, 0x9DFA, 10},
	{ZoombiniText::kFleens, 0x9DF2, 7},
	{ZoombiniText::kHotel, 0x943A, 14},
	{ZoombiniText::kNet, 0x93A0, 12},
	{ZoombiniText::kBasecamp2, 0x9DCA, 10},
	{ZoombiniText::kCaves, 0x9DBA, 15},
	{ZoombiniText::kSmoke, 0x92B8, 14},
	{ZoombiniText::kMaze, 0x933F, 18},
	{ZoombiniText::kNewGame, 0x8EB4, 8},
	{ZoombiniText::kPracticeMode, 0x8EDB, 13},
	{ZoombiniText::kContinueJourney, 0x9CD6, 16},
	{ZoombiniText::kPracticeTitle, 0x9CE8, 13},
	{ZoombiniText::kPracticeDesc1, 0x9D26, 18},
	{ZoombiniText::kPracticeDesc2, 0x9D14, 17},
	{ZoombiniText::kPracticeDesc3, 0x9D03, 16},
	{ZoombiniText::kPracticeDesc4, 0x9CF6, 12},
	{ZoombiniText::kTerrainKey, 0x9C6E, 11},
	{ZoombiniText::kChooseLevel, 0x9C5E, 14},
	{ZoombiniText::kLevel1, 0x9C52, 11},
	{ZoombiniText::kLevel2, 0x9C46, 11},
	{ZoombiniText::kLevel3, 0x9C32, 9},
	{ZoombiniText::kLevel4, 0x9C2C, 15},
	{ZoombiniText::kRoute1, 0x9BF4, 31},
	{ZoombiniText::kRoute2, 0x9BE8, 11},
	{ZoombiniText::kRoute3, 0x9BD6, 17},
	{ZoombiniText::kRoute4, 0x9BC0, 20},
	{ZoombiniText::kXferVillePopulation, 0x9B96, 24},
	{ZoombiniText::kMemorialJanuary, 0x9B8A, 7},
	{ZoombiniText::kMemorialFebruary, 0x9B80, 8},
	{ZoombiniText::kMemorialMarch, 0x9B7A, 5},
	{ZoombiniText::kMemorialApril, 0x9B74, 5},
	{ZoombiniText::kMemorialMay, 0x9B70, 3},
	{ZoombiniText::kMemorialJune, 0x9B6A, 4},
	{ZoombiniText::kMemorialJuly, 0x9B65, 4},
	{ZoombiniText::kMemorialAugust, 0x9B5E, 6},
	{ZoombiniText::kMemorialSeptember, 0x9B54, 9},
	{ZoombiniText::kMemorialOctober, 0x9B4C, 7},
	{ZoombiniText::kMemorialNovember, 0x9B43, 8},
	{ZoombiniText::kMemorialDecember, 0x9B3A, 8},
	{ZoombiniText::kMemorialWhenLevel, 0x9AF6, 18},
	{ZoombiniText::kMemorialHonorMonument, 0x9AAE, 50},
	{ZoombiniText::kMemorialHonorWindmill, 0x9A78, 53},
	{ZoombiniText::kMemorialHonorObservatory, 0x9A4B, 44},
	{ZoombiniText::kMemorialHonorBowlingAlley, 0x9A1E, 44},
	{ZoombiniText::kMemorialHonorGeneralStore, 0x99E8, 53},
	{ZoombiniText::kMemorialHonorSwimmingPool, 0x99BA, 45},
	{ZoombiniText::kMemorialHonorPlayground, 0x9982, 55},
	{ZoombiniText::kMemorialHonorBandShell, 0x994C, 53},
	{ZoombiniText::kMemorialHonorSchool, 0x9920, 43},
	{ZoombiniText::kMemorialHonorLibrary, 0x98EC, 51},
	{ZoombiniText::kMemorialHonorFire, 0x98C2, 40},
	{ZoombiniText::kMemorialHonorOpera, 0x988D, 52},
	{ZoombiniText::kMemorialHonorCityHall, 0x9860, 44},
	{ZoombiniText::kMemorialHonorClockTower, 0x9828, 55},
	{ZoombiniText::kMemorialHonorMuseum, 0x97F1, 54},
	{ZoombiniText::kMemorialHonorCourt, 0x97BA, 54},
	{ZoombiniText::kMemorialRoute1Level1, 0x9714, 100},
	{ZoombiniText::kMemorialRoute1Level2, 0x96A9, 106},
	{ZoombiniText::kMemorialRoute1Level3, 0x9642, 102},
	{ZoombiniText::kMemorialRoute1Level4, 0x95D0, 113},
	{ZoombiniText::kMemorialRoute2Level1, 0x958C, 66},
	{ZoombiniText::kMemorialRoute2Level2, 0x953A, 81},
	{ZoombiniText::kMemorialRoute2Level3, 0x94DD, 92},
	{ZoombiniText::kMemorialRoute2Level4, 0x958C, 66},
	{ZoombiniText::kMemorialRoute3Level1, 0x9472, 106},
	{ZoombiniText::kMemorialRoute3Level2, 0x9414, 93},
	{ZoombiniText::kMemorialRoute3Level3, 0x93B5, 94},
	{ZoombiniText::kMemorialRoute3Level4, 0x9352, 98},
	{ZoombiniText::kMemorialRoute4Level1, 0x92EA, 103},
	{ZoombiniText::kMemorialRoute4Level2, 0x927A, 110},
	{ZoombiniText::kMemorialRoute4Level3, 0x921E, 91},
	{ZoombiniText::kMemorialRoute4Level4, 0x9196, 134},
	{ZoombiniText::kDialogBodyGoMapWillLost, 0x9114, 64},
	{ZoombiniText::kDialogButtonLoseThem, 0x910A, 9},
	{ZoombiniText::kDialogButtonKeepThem, 0x9100, 9},
	{ZoombiniText::kDialogButtonOkay, 0x90FD, 2},
	{ZoombiniText::kDialogButtonCancel, 0x90F6, 6},
	{ZoombiniText::kDialogButtonYes, 0x8F4C, 3},
	{ZoombiniText::kDialogButtonNo, 0x8EEA, 2},
	{ZoombiniText::kDialogButtonQuit, 0x8DEA, 4},
	{ZoombiniText::kDialogButtonLoad, 0x8FA2, 4},
	{ZoombiniText::kDialogButtonSave, 0x8FAE, 4},
	{ZoombiniText::kOptionsTitle, 0x90E4, 7},
	{ZoombiniText::kOptionsLegendOn, 0x90DE, 4},
	{ZoombiniText::kOptionsLegendOff, 0x90D8, 5},
	{ZoombiniText::kOptionsNewGame, 0x90B6, 17},
	{ZoombiniText::kOptionsLoadGame, 0x90A3, 18},
	{ZoombiniText::kOptionsSaveGame, 0x9090, 18},
	{ZoombiniText::kOptionsQuit, 0x9082, 13},
	{ZoombiniText::kOptionsToggle, 0x90C8, 15},
	{ZoombiniText::kOptionsSound, 0x9066, 26},
	{ZoombiniText::kOptionsMusic, 0x904C, 25},
	{ZoombiniText::kOptionsStickyMouse, 0x9036, 21},
	{ZoombiniText::kOptionsTransitions, 0x9021, 20},
	{ZoombiniText::kOptionsCredits, 0x8F68, 7},
	{ZoombiniText::kDialogBodyNoSavedGames, 0x9012, 14},
	{ZoombiniText::kDialogBodyCreateAndSaveNewGame, 0x8FD9, 56},
	{ZoombiniText::kDialogButtonNewGame, 0x8EB4, 8},
	{ZoombiniText::kDialogButtonReplaceTitle, 0x8FC8, 7},
	{ZoombiniText::kDialogTitleSave, 0x8FBC, 11},
	{ZoombiniText::kDialogTitleSaveAs, 0x8FAE, 13},
	{ZoombiniText::kDialogTitleLoad, 0x8FA2, 11},
	{ZoombiniText::kDialogBodyReplaceGame, 0x8F70, 48},
	{ZoombiniText::kDialogBodySaveCurrentGame, 0x8F50, 23},
	{ZoombiniText::kDialogBodySaveDirtyGame, 0x8D97, 60},
	{ZoombiniText::kDialogBodyCannotSaveInPractice, 0x8EBF, 42},
	{ZoombiniText::kDialogBodyCreateNewGame, 0x8E94, 42},
	{ZoombiniText::kDialogBodyCannotSaveMoreGame, 0x8E4D, 70},
	{ZoombiniText::kDialogBodyCannotLoadInPractice, 0x8E22, 42},
	{ZoombiniText::kDialogBodyCannotCreateNewInPractice, 0x8DF1, 48},
	{ZoombiniText::kDialogBodyNewGame, 0x8EB4, 8},
	{ZoombiniText::kDialogBodyReallyQuit, 0x8DD4, 28},
	{ZoombiniText::kDialogBodySaveBeforeQuit, 0x8EEE, 87},
	{ZoombiniText::kDialogHelpTitle, 0x8D92, 4},
	{ZoombiniText::kDialogButtonPrev, 0x8D89, 8},
	{ZoombiniText::kDialogButtonNext, 0x8D84, 4},
	{ZoombiniText::kDialogHelpLevel, 0x7E40, 5},
	{ZoombiniText::kDialogHelpPickerUpdateVersion, 0x7968, 27},
	{ZoombiniText::kNotiBoxMusicOn, 0x80A6, 8},
	{ZoombiniText::kNotiBoxMusicOff, 0x809C, 9},
	{ZoombiniText::kNotiBoxSoundOn, 0x8092, 8},
	{ZoombiniText::kNotiBoxSoundOff, 0x8088, 9},
	{ZoombiniText::kNotiBoxLessAction, 0x807C, 11},
	{ZoombiniText::kNotiBoxMoreAction, 0x8070, 11},
	{ZoombiniText::kNotiBoxHideCursor, 0x8064, 11},
	{ZoombiniText::kNotiBoxShowCursor, 0x8058, 11},
	{ZoombiniText::kNotiBoxStickeyMouse, 0x804A, 12},
	{ZoombiniText::kNotiBoxNonStickeyMouse, 0x8039, 16},
	{ZoombiniText::kNotiBoxTransitionsOn, 0x802A, 14},
	{ZoombiniText::kNotiBoxTransitionsOff, 0x801A, 15},
	{ZoombiniText::kNotiBoxAutoStickeyOn, 0x800A, 14},
	{ZoombiniText::kNotiBoxAutoStickeyOff, 0x7FFA, 15},
};

const ZoombiniText::ExeTextEntry ZoombiniText::kV10USMacTextEntries[] = {
	{ZoombiniText::kTown, 0x8E1E, 13},
	{ZoombiniText::kPicker, 0x8F8E, 13},
	{ZoombiniText::kBridge, 0x89A8, 15},
	{ZoombiniText::kTunnels, 0x89C7, 16},
	{ZoombiniText::kPizza, 0x90E1, 10},
	{ZoombiniText::kBasecamp1, 0x8F78, 12},
	{ZoombiniText::kFerry, 0x90BA, 25},
	{ZoombiniText::kLilly, 0x90A3, 22},
	{ZoombiniText::kSlides, 0x9098, 10},
	{ZoombiniText::kFleens, 0x9090, 7},
	{ZoombiniText::kHotel, 0x86C2, 14},
	{ZoombiniText::kNet, 0x8628, 12},
	{ZoombiniText::kBasecamp2, 0x9068, 10},
	{ZoombiniText::kCaves, 0x9058, 15},
	{ZoombiniText::kSmoke, 0x8540, 14},
	{ZoombiniText::kMaze, 0x85C7, 18},
	{ZoombiniText::kNewGame, 0x6E29, 8},
	{ZoombiniText::kJourneyStatZoombiniIsle, 0x8F8E, 13},
	{ZoombiniText::kJourneyStatShelterRock, 0x8F78, 12},
	{ZoombiniText::kJourneyStatShadeTree, 0x9068, 10},
	{ZoombiniText::kJourneyStatZoombiniton, 0x8E1E, 13},
	{ZoombiniText::kPracticeMode, 0x809B, 13},
	{ZoombiniText::kPracticeReturnInitial1, 0x8FC4, 18},
	{ZoombiniText::kPracticeReturnInitial2, 0x8F88, 19},
	{ZoombiniText::kPracticeReturnInitial3, 0x3D, 1},
	{ZoombiniText::kPracticeReturnInitial4, 0x3D, 1},
	{ZoombiniText::kPracticeReturnShelterRock1, 0x8FC4, 18},
	{ZoombiniText::kPracticeReturnShelterRock2, 0x8F88, 19},
	{ZoombiniText::kPracticeReturnShelterRock3, 0x8F9E, 15},
	{ZoombiniText::kPracticeReturnShelterRock4, 0x3D, 1},
	{ZoombiniText::kPracticeReturnShadeTree1, 0x8FC4, 18},
	{ZoombiniText::kPracticeReturnShadeTree2, 0x8F88, 20},
	{ZoombiniText::kPracticeReturnShadeTree3, 0x8F78, 15},
	{ZoombiniText::kPracticeReturnShadeTree4, 0x9068, 10},
	{ZoombiniText::kPracticeReturnZoombiniton1, 0x8FC4, 18},
	{ZoombiniText::kPracticeReturnZoombiniton2, 0x8F88, 20},
	{ZoombiniText::kPracticeReturnZoombiniton3, 0x8F67, 16},
	{ZoombiniText::kPracticeReturnZoombiniton4, 0x8F56, 16},
	{ZoombiniText::kTerrainKey, 0x8EF6, 11},
	{ZoombiniText::kChooseLevel, 0x8EE6, 14},
	{ZoombiniText::kLevel1, 0x8EDA, 11},
	{ZoombiniText::kLevel2, 0x8ECE, 11},
	{ZoombiniText::kLevel3, 0x8EBA, 9},
	{ZoombiniText::kLevel4, 0x8EB4, 15},
	{ZoombiniText::kRoute1, 0x8E7C, 31},
	{ZoombiniText::kRoute2, 0x8E70, 11},
	{ZoombiniText::kRoute3, 0x8E5E, 17},
	{ZoombiniText::kRoute4, 0x8E48, 20},
	{ZoombiniText::kXferVillePopulation, 0x8E1E, 24},
	{ZoombiniText::kMemorialJanuary, 0x8E12, 7},
	{ZoombiniText::kMemorialFebruary, 0x8E08, 8},
	{ZoombiniText::kMemorialMarch, 0x8E02, 5},
	{ZoombiniText::kMemorialApril, 0x8DFC, 5},
	{ZoombiniText::kMemorialMay, 0x8DF8, 3},
	{ZoombiniText::kMemorialJune, 0x8DF2, 4},
	{ZoombiniText::kMemorialJuly, 0x8DED, 4},
	{ZoombiniText::kMemorialAugust, 0x8DE6, 6},
	{ZoombiniText::kMemorialSeptember, 0x8DDC, 9},
	{ZoombiniText::kMemorialOctober, 0x8DD4, 7},
	{ZoombiniText::kMemorialNovember, 0x8DCB, 8},
	{ZoombiniText::kMemorialDecember, 0x8DC2, 8},
	{ZoombiniText::kMemorialWhenLevel, 0x8D7E, 18},
	{ZoombiniText::kMemorialHonorMonument, 0x8D36, 50},
	{ZoombiniText::kMemorialHonorWindmill, 0x8D00, 53},
	{ZoombiniText::kMemorialHonorObservatory, 0x8CD3, 44},
	{ZoombiniText::kMemorialHonorBowlingAlley, 0x8CA6, 44},
	{ZoombiniText::kMemorialHonorGeneralStore, 0x8C70, 53},
	{ZoombiniText::kMemorialHonorSwimmingPool, 0x8C42, 45},
	{ZoombiniText::kMemorialHonorPlayground, 0x8C0A, 55},
	{ZoombiniText::kMemorialHonorBandShell, 0x8BD4, 53},
	{ZoombiniText::kMemorialHonorSchool, 0x8BA8, 43},
	{ZoombiniText::kMemorialHonorLibrary, 0x8B74, 51},
	{ZoombiniText::kMemorialHonorFire, 0x8B4A, 40},
	{ZoombiniText::kMemorialHonorOpera, 0x8B15, 52},
	{ZoombiniText::kMemorialHonorCityHall, 0x8AE8, 44},
	{ZoombiniText::kMemorialHonorClockTower, 0x8AB0, 55},
	{ZoombiniText::kMemorialHonorMuseum, 0x8A79, 54},
	{ZoombiniText::kMemorialHonorCourt, 0x8A42, 54},
	{ZoombiniText::kMemorialRoute1Level1, 0x899C, 100},
	{ZoombiniText::kMemorialRoute1Level2, 0x8931, 106},
	{ZoombiniText::kMemorialRoute1Level3, 0x88CA, 102},
	{ZoombiniText::kMemorialRoute1Level4, 0x8858, 113},
	{ZoombiniText::kMemorialRoute2Level1, 0x8814, 66},
	{ZoombiniText::kMemorialRoute2Level2, 0x87C2, 81},
	{ZoombiniText::kMemorialRoute2Level3, 0x8765, 92},
	{ZoombiniText::kMemorialRoute2Level4, 0x8814, 66},
	{ZoombiniText::kMemorialRoute3Level1, 0x86FA, 106},
	{ZoombiniText::kMemorialRoute3Level2, 0x869C, 93},
	{ZoombiniText::kMemorialRoute3Level3, 0x863D, 94},
	{ZoombiniText::kMemorialRoute3Level4, 0x85DA, 98},
	{ZoombiniText::kMemorialRoute4Level1, 0x8572, 103},
	{ZoombiniText::kMemorialRoute4Level2, 0x8502, 110},
	{ZoombiniText::kMemorialRoute4Level3, 0x84A6, 91},
	{ZoombiniText::kMemorialRoute4Level4, 0x841E, 134},
	{ZoombiniText::kDialogBodyGoMapWillLost, 0x839C, 64},
	{ZoombiniText::kDialogButtonLoseThem, 0x8392, 9},
	{ZoombiniText::kDialogButtonKeepThem, 0x8388, 9},
	{ZoombiniText::kDialogButtonOkay, 0x2915, 2},
	{ZoombiniText::kDialogButtonCancel, 0x837E, 6},
	{ZoombiniText::kDialogButtonYes, 0x81D4, 3},
	{ZoombiniText::kDialogButtonNo, 0x24B2, 2},
	{ZoombiniText::kDialogButtonQuit, 0x8072, 4},
	{ZoombiniText::kDialogButtonLoad, 0x822A, 4},
	{ZoombiniText::kDialogButtonSave, 0x8236, 4},
	{ZoombiniText::kOptionsTitle, 0x836C, 7},
	{ZoombiniText::kOptionsLegendOn, 0x8366, 4},
	{ZoombiniText::kOptionsLegendOff, 0x8360, 5},
	{ZoombiniText::kOptionsNewGame, 0x833E, 17},
	{ZoombiniText::kOptionsLoadGame, 0x832B, 18},
	{ZoombiniText::kOptionsSaveGame, 0x8318, 18},
	{ZoombiniText::kOptionsQuit, 0x830A, 13},
	{ZoombiniText::kOptionsToggle, 0x8350, 15},
	{ZoombiniText::kOptionsSound, 0x82EE, 26},
	{ZoombiniText::kOptionsMusic, 0x82D4, 25},
	{ZoombiniText::kOptionsStickyMouse, 0x82BE, 21},
	{ZoombiniText::kOptionsTransitions, 0x82A9, 20},
	{ZoombiniText::kOptionsCredits, 0x81F0, 7},
	{ZoombiniText::kDialogBodyNoSavedGames, 0x829A, 14},
	{ZoombiniText::kDialogBodyCreateAndSaveNewGame, 0x8261, 56},
	{ZoombiniText::kDialogButtonNewGame, 0x813C, 8},
	{ZoombiniText::kDialogButtonReplaceTitle, 0x8250, 7},
	{ZoombiniText::kDialogTitleSave, 0x8244, 11},
	{ZoombiniText::kDialogTitleSaveAs, 0x8236, 13},
	{ZoombiniText::kDialogTitleLoad, 0x822A, 11},
	{ZoombiniText::kDialogBodyReplaceGame, 0x81F8, 48},
	{ZoombiniText::kDialogBodySaveCurrentGame, 0x81D8, 23},
	{ZoombiniText::kDialogBodyCannotSaveInPractice, 0x8147, 42},
	{ZoombiniText::kDialogBodyCreateNewGame, 0x811C, 42},
	{ZoombiniText::kDialogBodyCannotSaveMoreGame, 0x80D5, 70},
	{ZoombiniText::kDialogBodyCannotLoadInPractice, 0x80AA, 42},
	{ZoombiniText::kDialogBodyCannotCreateNewInPractice, 0x8079, 48},
	{ZoombiniText::kDialogBodyNewGame, 0x813C, 8},
	{ZoombiniText::kDialogBodyReallyQuit, 0x805C, 28},
	{ZoombiniText::kDialogBodySaveBeforeQuit, 0x8176, 87},
	{ZoombiniText::kDialogHelpLevel, 0x3270, 5},
	{ZoombiniText::kNotiBoxMusicOn, 0x7398, 8},
	{ZoombiniText::kNotiBoxMusicOff, 0x738E, 9},
	{ZoombiniText::kNotiBoxSoundOn, 0x7384, 8},
	{ZoombiniText::kNotiBoxSoundOff, 0x737A, 9},
	{ZoombiniText::kNotiBoxLessAction, 0x736E, 11},
	{ZoombiniText::kNotiBoxMoreAction, 0x7362, 11},
	{ZoombiniText::kNotiBoxHideCursor, 0x7356, 11},
	{ZoombiniText::kNotiBoxShowCursor, 0x734A, 11},
	{ZoombiniText::kNotiBoxStickeyMouse, 0x732F, 12},
	{ZoombiniText::kNotiBoxNonStickeyMouse, 0x732B, 16},
	{ZoombiniText::kNotiBoxTransitionsOn, 0x731C, 14},
	{ZoombiniText::kNotiBoxTransitionsOff, 0x730C, 15},
	{ZoombiniText::kNotiBoxAutoStickeyOn, 0x72FC, 14},
	{ZoombiniText::kNotiBoxAutoStickeyOff, 0x72EC, 15},
};

const ZoombiniText::ExeTextEntry ZoombiniText::kV10BRMacTextEntries[] = {
	{ZoombiniText::kNewGame, 0x836A, 8},
};

const ZoombiniText::ExeTextEntry ZoombiniText::kV10FRMacTextEntries[] = {
	{ZoombiniText::kNewGame, 0x835A, 15},
};

const ZoombiniText::ExeTextEntry ZoombiniText::kV10GEMacTextEntries[] = {
	{ZoombiniText::kNewGame, 0x834E, 11},
};

const ZoombiniText::ExeTextEntry ZoombiniText::kV11BRMacTextEntries[] = {
	{ZoombiniText::kNewGame, 0x89C0, 8},
};

const ZoombiniText::ExeTextEntry ZoombiniText::kV11FRMacTextEntries[] = {
	{ZoombiniText::kNewGame, 0x89B0, 15},
};

const ZoombiniText::ExeTextEntry ZoombiniText::kV11GEMacTextEntries[] = {
	{ZoombiniText::kNewGame, 0x89A4, 11},
};

const ZoombiniText::ExeTextEntry ZoombiniText::kV10JPMacTextEntries[] = {
	{ZoombiniText::kDialogHelpPickerUpdateVersion, 0x0, 0},
	{ZoombiniText::kNewGame, 0x85D2, 10},
	{ZoombiniText::kNotiBoxEnglishNames, 0x723B, 10},
	{ZoombiniText::kNotiBoxJapaneseNames, 0x722E, 12},
};

const ZoombiniText::ExeTextEntry ZoombiniText::kV20USMacTextEntries[] = {
	{ZoombiniText::kTown, 0x7DD4, 13},
	{ZoombiniText::kPicker, 0x7CF0, 13},
	{ZoombiniText::kBridge, 0x7CFE, 15},
	{ZoombiniText::kTunnels, 0x7D0E, 16},
	{ZoombiniText::kPizza, 0x7D1F, 10},
	{ZoombiniText::kBasecamp1, 0x7D2A, 12},
	{ZoombiniText::kFerry, 0x7D37, 25},
	{ZoombiniText::kLilly, 0x7D51, 22},
	{ZoombiniText::kSlides, 0x7D68, 10},
	{ZoombiniText::kFleens, 0x7D73, 7},
	{ZoombiniText::kHotel, 0x7D7B, 14},
	{ZoombiniText::kNet, 0x7D8A, 12},
	{ZoombiniText::kBasecamp2, 0x7D97, 10},
	{ZoombiniText::kCaves, 0x7DA2, 15},
	{ZoombiniText::kSmoke, 0x7DB2, 14},
	{ZoombiniText::kMaze, 0x7DC1, 18},
	{ZoombiniText::kNewGame, 0x8AAF, 8},
	{ZoombiniText::kPracticeMode, 0x7E24, 13},
	{ZoombiniText::kContinueJourney, 0x7E83, 16},
	{ZoombiniText::kPracticeTitle, 0x7E75, 13},
	{ZoombiniText::kPracticeDesc1, 0x7E32, 18},
	{ZoombiniText::kPracticeDesc2, 0x7E45, 17},
	{ZoombiniText::kPracticeDesc3, 0x7E57, 16},
	{ZoombiniText::kPracticeDesc4, 0x7E68, 12},
	{ZoombiniText::kTerrainKey, 0x7EF0, 11},
	{ZoombiniText::kChooseLevel, 0x7EFC, 14},
	{ZoombiniText::kLevel1, 0x7F0B, 11},
	{ZoombiniText::kLevel2, 0x7F17, 11},
	{ZoombiniText::kLevel3, 0x7F23, 9},
	{ZoombiniText::kLevel4, 0x7F2D, 15},
	{ZoombiniText::kRoute1, 0x7F58, 31},
	{ZoombiniText::kRoute2, 0x7F78, 11},
	{ZoombiniText::kRoute3, 0x7F84, 17},
	{ZoombiniText::kRoute4, 0x7F96, 20},
	{ZoombiniText::kRodMapOptionsButton, 0x7FBC, 7},
	{ZoombiniText::kXferVillePopulation, 0x7FC8, 24},
	{ZoombiniText::kMemorialJanuary, 0x7FE8, 7},
	{ZoombiniText::kMemorialFebruary, 0x7FF0, 8},
	{ZoombiniText::kMemorialMarch, 0x7FF9, 5},
	{ZoombiniText::kMemorialApril, 0x7FFF, 5},
	{ZoombiniText::kMemorialMay, 0x2506, 3},
	{ZoombiniText::kMemorialJune, 0x8005, 4},
	{ZoombiniText::kMemorialJuly, 0x800A, 4},
	{ZoombiniText::kMemorialAugust, 0x800F, 6},
	{ZoombiniText::kMemorialSeptember, 0x8016, 9},
	{ZoombiniText::kMemorialOctober, 0x8020, 7},
	{ZoombiniText::kMemorialNovember, 0x8028, 8},
	{ZoombiniText::kMemorialDecember, 0x8031, 8},
	{ZoombiniText::kMemorialWhenLevel, 0x806C, 18},
	{ZoombiniText::kMemorialHonorMonument, 0x8094, 50},
	{ZoombiniText::kMemorialHonorWindmill, 0x80C7, 53},
	{ZoombiniText::kMemorialHonorObservatory, 0x80FD, 44},
	{ZoombiniText::kMemorialHonorBowlingAlley, 0x812A, 44},
	{ZoombiniText::kMemorialHonorGeneralStore, 0x8157, 53},
	{ZoombiniText::kMemorialHonorSwimmingPool, 0x818D, 45},
	{ZoombiniText::kMemorialHonorPlayground, 0x81BB, 55},
	{ZoombiniText::kMemorialHonorBandShell, 0x81F3, 53},
	{ZoombiniText::kMemorialHonorSchool, 0x8229, 43},
	{ZoombiniText::kMemorialHonorLibrary, 0x8255, 51},
	{ZoombiniText::kMemorialHonorFire, 0x8289, 40},
	{ZoombiniText::kMemorialHonorOpera, 0x82B2, 52},
	{ZoombiniText::kMemorialHonorCityHall, 0x82E7, 44},
	{ZoombiniText::kMemorialHonorClockTower, 0x8314, 55},
	{ZoombiniText::kMemorialHonorMuseum, 0x834C, 54},
	{ZoombiniText::kMemorialHonorCourt, 0x8383, 54},
	{ZoombiniText::kMemorialRoute1Level1, 0x83FC, 100},
	{ZoombiniText::kMemorialRoute1Level2, 0x8461, 106},
	{ZoombiniText::kMemorialRoute1Level3, 0x84CC, 102},
	{ZoombiniText::kMemorialRoute1Level4, 0x8533, 113},
	{ZoombiniText::kMemorialRoute2Level1, 0x85A5, 66},
	{ZoombiniText::kMemorialRoute2Level2, 0x85E8, 81},
	{ZoombiniText::kMemorialRoute2Level3, 0x863A, 92},
	{ZoombiniText::kMemorialRoute2Level4, 0x85A5, 66},
	{ZoombiniText::kMemorialRoute3Level1, 0x8697, 106},
	{ZoombiniText::kMemorialRoute3Level2, 0x8702, 93},
	{ZoombiniText::kMemorialRoute3Level3, 0x8760, 94},
	{ZoombiniText::kMemorialRoute3Level4, 0x87BF, 98},
	{ZoombiniText::kMemorialRoute4Level1, 0x8822, 103},
	{ZoombiniText::kMemorialRoute4Level2, 0x888A, 110},
	{ZoombiniText::kMemorialRoute4Level3, 0x88F9, 91},
	{ZoombiniText::kMemorialRoute4Level4, 0x8955, 134},
	{ZoombiniText::kDialogBodyGoMapWillLost, 0x8A1C, 64},
	{ZoombiniText::kDialogButtonLoseThem, 0x8A5D, 9},
	{ZoombiniText::kDialogButtonKeepThem, 0x8A67, 9},
	{ZoombiniText::kDialogButtonOkay, 0x250A, 2},
	{ZoombiniText::kDialogButtonCancel, 0x8A71, 6},
	{ZoombiniText::kDialogButtonYes, 0x250D, 3},
	{ZoombiniText::kDialogButtonNo, 0x2511, 2},
	{ZoombiniText::kDialogButtonQuit, 0x8AE7, 4},
	{ZoombiniText::kDialogButtonLoad, 0x8A78, 4},
	{ZoombiniText::kDialogButtonSave, 0x8A7D, 4},
	{ZoombiniText::kOptionsTitle, 0x8A82, 7},
	{ZoombiniText::kOptionsLegendOn, 0x8A94, 4},
	{ZoombiniText::kOptionsLegendOff, 0x8A99, 5},
	{ZoombiniText::kOptionsNewGame, 0x8AAF, 17},
	{ZoombiniText::kOptionsLoadGame, 0x8AC1, 18},
	{ZoombiniText::kOptionsSaveGame, 0x8AD4, 18},
	{ZoombiniText::kOptionsQuit, 0x8AE7, 13},
	{ZoombiniText::kOptionsToggle, 0x8A9F, 15},
	{ZoombiniText::kOptionsSound, 0x8AF5, 26},
	{ZoombiniText::kOptionsMusic, 0x8B10, 25},
	{ZoombiniText::kOptionsStickyMouse, 0x8B2A, 21},
	{ZoombiniText::kOptionsTransitions, 0x8B40, 20},
	{ZoombiniText::kOptionsCredits, 0x8C06, 7},
	{ZoombiniText::kDialogBodyNoSavedGames, 0x8B55, 14},
	{ZoombiniText::kDialogBodyCreateAndSaveNewGame, 0x8B64, 56},
	{ZoombiniText::kDialogButtonNewGame, 0x8AAF, 8},
	{ZoombiniText::kDialogButtonReplaceTitle, 0x8BA6, 7},
	{ZoombiniText::kDialogTitleSave, 0x8BAE, 11},
	{ZoombiniText::kDialogTitleSaveAs, 0x8BBA, 13},
	{ZoombiniText::kDialogTitleLoad, 0x8BC8, 11},
	{ZoombiniText::kDialogBodyReplaceGame, 0x8BD4, 48},
	{ZoombiniText::kDialogBodySaveCurrentGame, 0x8C0E, 23},
	{ZoombiniText::kDialogBodySaveDirtyGame, 0x8D99, 60},
	{ZoombiniText::kDialogBodyCannotSaveInPractice, 0x8C83, 42},
	{ZoombiniText::kDialogBodyCreateNewGame, 0x8CAE, 42},
	{ZoombiniText::kDialogBodyCannotSaveMoreGame, 0x8CD9, 70},
	{ZoombiniText::kDialogBodyCannotLoadInPractice, 0x8D20, 42},
	{ZoombiniText::kDialogBodyCannotCreateNewInPractice, 0x8D4B, 48},
	{ZoombiniText::kDialogBodyNewGame, 0x8AAF, 8},
	{ZoombiniText::kDialogBodyReallyQuit, 0x8D7C, 28},
	{ZoombiniText::kDialogBodySaveBeforeQuit, 0x8C2B, 87},
	{ZoombiniText::kDialogHelpTitle, 0x8E08, 4},
	{ZoombiniText::kDialogButtonPrev, 0x8DDB, 8},
	{ZoombiniText::kDialogHelpPickerUpdateVersion, 0xB0FD, 27},
	{ZoombiniText::kNotiBoxMusicOn, 0xA3E4, 8},
	{ZoombiniText::kNotiBoxMusicOff, 0xA3ED, 9},
	{ZoombiniText::kNotiBoxSoundOn, 0xA3F7, 8},
	{ZoombiniText::kNotiBoxSoundOff, 0xA400, 9},
	{ZoombiniText::kNotiBoxLessAction, 0xA40A, 11},
	{ZoombiniText::kNotiBoxMoreAction, 0xA416, 11},
	{ZoombiniText::kNotiBoxHideCursor, 0xA422, 11},
	{ZoombiniText::kNotiBoxShowCursor, 0xA42E, 11},
	{ZoombiniText::kNotiBoxStickeyMouse, 0xA43A, 12},
	{ZoombiniText::kNotiBoxNonStickeyMouse, 0xA447, 16},
	{ZoombiniText::kNotiBoxTransitionsOn, 0xA458, 14},
	{ZoombiniText::kNotiBoxTransitionsOff, 0xA467, 15},
	{ZoombiniText::kNotiBoxAutoStickeyOn, 0xA477, 14},
	{ZoombiniText::kNotiBoxAutoStickeyOff, 0xA486, 15},
	{ZoombiniText::kOptionsTitle, 0x8A82, 17},
	{ZoombiniText::kOptionsHelpAudio, 0x8E08, 19},
	{ZoombiniText::kOptionsTouchSense, 0x8DE9, 30},
	{ZoombiniText::kDialogHelpTitle, 0x78CF, 4},
	{ZoombiniText::kDialogButtonOkay, 0x2514, 2},
	{ZoombiniText::kDialogButtonNext, 0x8DE4, 4},
	{ZoombiniText::kDialogBodyRemoveGame, 0x8E1C, 42},
	{ZoombiniText::kNotiBoxHelpAudioOn, 0xA4B5, 13},
	{ZoombiniText::kNotiBoxHelpAudioOff, 0xA4C3, 14},
	{ZoombiniText::kNotiBoxTouchSenseOn, 0xA496, 14},
	{ZoombiniText::kNotiBoxTouchSenseOff, 0xA4A5, 15},
};

const ZoombiniText::ExeTextEntry ZoombiniText::kV20USMacRawTextEntries[] = {
	{ZoombiniText::kDialogHelpLevel, 0x9A704, 5},
};

const ZoombiniText::ExeTextEntry ZoombiniText::kV10USMacDemoTextEntries[] = {
	{ZoombiniText::kTown, 0x8DAE, 13},
	{ZoombiniText::kPicker, 0x8F1E, 13},
	{ZoombiniText::kBridge, 0x8938, 15},
	{ZoombiniText::kTunnels, 0x8957, 16},
	{ZoombiniText::kPizza, 0x9071, 10},
	{ZoombiniText::kBasecamp1, 0x8F08, 12},
	{ZoombiniText::kFerry, 0x904A, 25},
	{ZoombiniText::kLilly, 0x9033, 22},
	{ZoombiniText::kSlides, 0x9028, 10},
	{ZoombiniText::kFleens, 0x9020, 7},
	{ZoombiniText::kHotel, 0x8652, 14},
	{ZoombiniText::kNet, 0x85B8, 12},
	{ZoombiniText::kBasecamp2, 0x8FF8, 10},
	{ZoombiniText::kCaves, 0x8FE8, 15},
	{ZoombiniText::kSmoke, 0x84D0, 14},
	{ZoombiniText::kMaze, 0x8557, 18},
	{ZoombiniText::kNewGame, 0x6DB9, 8},
	{ZoombiniText::kJourneyStatZoombiniIsle, 0x8F1E, 13},
	{ZoombiniText::kJourneyStatShelterRock, 0x8F08, 12},
	{ZoombiniText::kJourneyStatShadeTree, 0x8FF8, 10},
	{ZoombiniText::kJourneyStatZoombiniton, 0x8DAE, 13},
	{ZoombiniText::kPracticeMode, 0x802B, 13},
	{ZoombiniText::kPracticeReturnInitial1, 0x8F54, 18},
	{ZoombiniText::kPracticeReturnInitial2, 0x8F18, 19},
	{ZoombiniText::kPracticeReturnInitial3, 0x3D, 1},
	{ZoombiniText::kPracticeReturnInitial4, 0x3D, 1},
	{ZoombiniText::kPracticeReturnShelterRock1, 0x8F54, 18},
	{ZoombiniText::kPracticeReturnShelterRock2, 0x8F18, 19},
	{ZoombiniText::kPracticeReturnShelterRock3, 0x8F2E, 15},
	{ZoombiniText::kPracticeReturnShelterRock4, 0x3D, 1},
	{ZoombiniText::kPracticeReturnShadeTree1, 0x8F54, 18},
	{ZoombiniText::kPracticeReturnShadeTree2, 0x8F18, 20},
	{ZoombiniText::kPracticeReturnShadeTree3, 0x8F08, 15},
	{ZoombiniText::kPracticeReturnShadeTree4, 0x8FF8, 10},
	{ZoombiniText::kPracticeReturnZoombiniton1, 0x8F54, 18},
	{ZoombiniText::kPracticeReturnZoombiniton2, 0x8F18, 20},
	{ZoombiniText::kPracticeReturnZoombiniton3, 0x8EF7, 16},
	{ZoombiniText::kPracticeReturnZoombiniton4, 0x8EE6, 16},
	{ZoombiniText::kTerrainKey, 0x8E86, 11},
	{ZoombiniText::kChooseLevel, 0x8E76, 14},
	{ZoombiniText::kLevel1, 0x8E6A, 11},
	{ZoombiniText::kLevel2, 0x8E5E, 11},
	{ZoombiniText::kLevel3, 0x8E4A, 9},
	{ZoombiniText::kLevel4, 0x8E44, 15},
	{ZoombiniText::kRoute1, 0x8E0C, 31},
	{ZoombiniText::kRoute2, 0x8E00, 11},
	{ZoombiniText::kRoute3, 0x8DEE, 17},
	{ZoombiniText::kRoute4, 0x8DD8, 20},
	{ZoombiniText::kXferVillePopulation, 0x8DAE, 24},
	{ZoombiniText::kMemorialJanuary, 0x8DA2, 7},
	{ZoombiniText::kMemorialFebruary, 0x8D98, 8},
	{ZoombiniText::kMemorialMarch, 0x8D92, 5},
	{ZoombiniText::kMemorialApril, 0x8D8C, 5},
	{ZoombiniText::kMemorialMay, 0x8D88, 3},
	{ZoombiniText::kMemorialJune, 0x8D82, 4},
	{ZoombiniText::kMemorialJuly, 0x8D7D, 4},
	{ZoombiniText::kMemorialAugust, 0x8D76, 6},
	{ZoombiniText::kMemorialSeptember, 0x8D6C, 9},
	{ZoombiniText::kMemorialOctober, 0x8D64, 7},
	{ZoombiniText::kMemorialNovember, 0x8D5B, 8},
	{ZoombiniText::kMemorialDecember, 0x8D52, 8},
	{ZoombiniText::kMemorialWhenLevel, 0x8D0E, 18},
	{ZoombiniText::kMemorialHonorMonument, 0x8CC6, 50},
	{ZoombiniText::kMemorialHonorWindmill, 0x8C90, 53},
	{ZoombiniText::kMemorialHonorObservatory, 0x8C63, 44},
	{ZoombiniText::kMemorialHonorBowlingAlley, 0x8C36, 44},
	{ZoombiniText::kMemorialHonorGeneralStore, 0x8C00, 53},
	{ZoombiniText::kMemorialHonorSwimmingPool, 0x8BD2, 45},
	{ZoombiniText::kMemorialHonorPlayground, 0x8B9A, 55},
	{ZoombiniText::kMemorialHonorBandShell, 0x8B64, 53},
	{ZoombiniText::kMemorialHonorSchool, 0x8B38, 43},
	{ZoombiniText::kMemorialHonorLibrary, 0x8B04, 51},
	{ZoombiniText::kMemorialHonorFire, 0x8ADA, 40},
	{ZoombiniText::kMemorialHonorOpera, 0x8AA5, 52},
	{ZoombiniText::kMemorialHonorCityHall, 0x8A78, 44},
	{ZoombiniText::kMemorialHonorClockTower, 0x8A40, 55},
	{ZoombiniText::kMemorialHonorMuseum, 0x8A09, 54},
	{ZoombiniText::kMemorialHonorCourt, 0x89D2, 54},
	{ZoombiniText::kMemorialRoute1Level1, 0x892C, 100},
	{ZoombiniText::kMemorialRoute1Level2, 0x88C1, 106},
	{ZoombiniText::kMemorialRoute1Level3, 0x885A, 102},
	{ZoombiniText::kMemorialRoute1Level4, 0x87E8, 113},
	{ZoombiniText::kMemorialRoute2Level1, 0x87A4, 66},
	{ZoombiniText::kMemorialRoute2Level2, 0x8752, 81},
	{ZoombiniText::kMemorialRoute2Level3, 0x86F5, 92},
	{ZoombiniText::kMemorialRoute2Level4, 0x87A4, 66},
	{ZoombiniText::kMemorialRoute3Level1, 0x868A, 106},
	{ZoombiniText::kMemorialRoute3Level2, 0x862C, 93},
	{ZoombiniText::kMemorialRoute3Level3, 0x85CD, 94},
	{ZoombiniText::kMemorialRoute3Level4, 0x856A, 98},
	{ZoombiniText::kMemorialRoute4Level1, 0x8502, 103},
	{ZoombiniText::kMemorialRoute4Level2, 0x8492, 110},
	{ZoombiniText::kMemorialRoute4Level3, 0x8436, 91},
	{ZoombiniText::kMemorialRoute4Level4, 0x83AE, 134},
	{ZoombiniText::kDialogBodyGoMapWillLost, 0x832C, 64},
	{ZoombiniText::kDialogButtonLoseThem, 0x8322, 9},
	{ZoombiniText::kDialogButtonKeepThem, 0x8318, 9},
	{ZoombiniText::kDialogButtonOkay, 0x290D, 2},
	{ZoombiniText::kDialogButtonCancel, 0x830E, 6},
	{ZoombiniText::kDialogButtonYes, 0x8164, 3},
	{ZoombiniText::kDialogButtonNo, 0x24AA, 2},
	{ZoombiniText::kDialogButtonQuit, 0x8002, 4},
	{ZoombiniText::kDialogButtonLoad, 0x81BA, 4},
	{ZoombiniText::kDialogButtonSave, 0x81C6, 4},
	{ZoombiniText::kOptionsTitle, 0x82FC, 7},
	{ZoombiniText::kOptionsLegendOn, 0x82F6, 4},
	{ZoombiniText::kOptionsLegendOff, 0x82F0, 5},
	{ZoombiniText::kOptionsNewGame, 0x82CE, 17},
	{ZoombiniText::kOptionsLoadGame, 0x82BB, 18},
	{ZoombiniText::kOptionsSaveGame, 0x82A8, 18},
	{ZoombiniText::kOptionsQuit, 0x829A, 13},
	{ZoombiniText::kOptionsToggle, 0x82E0, 15},
	{ZoombiniText::kOptionsSound, 0x827E, 26},
	{ZoombiniText::kOptionsMusic, 0x8264, 25},
	{ZoombiniText::kOptionsStickyMouse, 0x824E, 21},
	{ZoombiniText::kOptionsTransitions, 0x8239, 20},
	{ZoombiniText::kOptionsCredits, 0x8180, 7},
	{ZoombiniText::kDialogBodyNoSavedGames, 0x822A, 14},
	{ZoombiniText::kDialogBodyCreateAndSaveNewGame, 0x81F1, 56},
	{ZoombiniText::kDialogButtonNewGame, 0x80CC, 8},
	{ZoombiniText::kDialogButtonReplaceTitle, 0x81E0, 7},
	{ZoombiniText::kDialogTitleSave, 0x81D4, 11},
	{ZoombiniText::kDialogTitleSaveAs, 0x81C6, 13},
	{ZoombiniText::kDialogTitleLoad, 0x81BA, 11},
	{ZoombiniText::kDialogBodyReplaceGame, 0x8188, 48},
	{ZoombiniText::kDialogBodySaveCurrentGame, 0x8168, 23},
	{ZoombiniText::kDialogBodyCannotSaveInPractice, 0x80D7, 42},
	{ZoombiniText::kDialogBodyCreateNewGame, 0x80AC, 42},
	{ZoombiniText::kDialogBodyCannotSaveMoreGame, 0x8065, 70},
	{ZoombiniText::kDialogBodyCannotLoadInPractice, 0x803A, 42},
	{ZoombiniText::kDialogBodyCannotCreateNewInPractice, 0x8009, 48},
	{ZoombiniText::kDialogBodyNewGame, 0x80CC, 8},
	{ZoombiniText::kDialogBodyReallyQuit, 0x7FEC, 28},
	{ZoombiniText::kDialogBodySaveBeforeQuit, 0x8106, 87},
	{ZoombiniText::kDialogHelpLevel, 0x3268, 5},
	{ZoombiniText::kNotiBoxMusicOn, 0x7328, 8},
	{ZoombiniText::kNotiBoxMusicOff, 0x731E, 9},
	{ZoombiniText::kNotiBoxSoundOn, 0x7314, 8},
	{ZoombiniText::kNotiBoxSoundOff, 0x730A, 9},
	{ZoombiniText::kNotiBoxLessAction, 0x72FE, 11},
	{ZoombiniText::kNotiBoxMoreAction, 0x72F2, 11},
	{ZoombiniText::kNotiBoxHideCursor, 0x72E6, 11},
	{ZoombiniText::kNotiBoxShowCursor, 0x72DA, 11},
	{ZoombiniText::kNotiBoxStickeyMouse, 0x72BF, 12},
	{ZoombiniText::kNotiBoxNonStickeyMouse, 0x72BB, 16},
	{ZoombiniText::kNotiBoxTransitionsOn, 0x72AC, 14},
	{ZoombiniText::kNotiBoxTransitionsOff, 0x729C, 15},
	{ZoombiniText::kNotiBoxAutoStickeyOn, 0x728C, 14},
	{ZoombiniText::kNotiBoxAutoStickeyOff, 0x727C, 15},
};

const ZoombiniText::ExeTextEntry ZoombiniText::kV10USDemo_NETextEntries[] = {
	{ZoombiniText::kTown, 0xD8B1F, 13},
	{ZoombiniText::kPicker, 0xD8A3A, 13},
	{ZoombiniText::kBridge, 0xD8A48, 15},
	{ZoombiniText::kTunnels, 0xD8A58, 16},
	{ZoombiniText::kPizza, 0xD8A69, 10},
	{ZoombiniText::kBasecamp1, 0xD8A74, 12},
	{ZoombiniText::kFerry, 0xD8A81, 25},
	{ZoombiniText::kLilly, 0xD8A9B, 22},
	{ZoombiniText::kSlides, 0xD8AB2, 10},
	{ZoombiniText::kFleens, 0xD8ABD, 7},
	{ZoombiniText::kHotel, 0xD8AC5, 14},
	{ZoombiniText::kNet, 0xD8AD4, 12},
	{ZoombiniText::kBasecamp2, 0xD8AE1, 10},
	{ZoombiniText::kCaves, 0xD8AEC, 15},
	{ZoombiniText::kSmoke, 0xD8AFC, 14},
	{ZoombiniText::kMaze, 0xD8B0C, 18},
	{ZoombiniText::kNewGame, 0xD9758, 8},
	{ZoombiniText::kJourneyStatZoombiniIsle, 0xD8B2D, 13},
	{ZoombiniText::kJourneyStatShelterRock, 0xD8B3B, 12},
	{ZoombiniText::kJourneyStatShadeTree, 0xD8B48, 10},
	{ZoombiniText::kJourneyStatZoombiniton, 0xD8B53, 13},
	{ZoombiniText::kPracticeMode, 0xD8B61, 13},
	{ZoombiniText::kPracticeReturnInitial1, 0xD8B6F, 18},
	{ZoombiniText::kPracticeReturnInitial2, 0xD8B82, 19},
	{ZoombiniText::kPracticeReturnInitial3, 0xD8B96, 1},
	{ZoombiniText::kPracticeReturnInitial4, 0xD8B98, 1},
	{ZoombiniText::kPracticeReturnShelterRock1, 0xD8B9A, 18},
	{ZoombiniText::kPracticeReturnShelterRock2, 0xD8BAD, 19},
	{ZoombiniText::kPracticeReturnShelterRock3, 0xD8BC1, 15},
	{ZoombiniText::kPracticeReturnShelterRock4, 0xD8BD1, 1},
	{ZoombiniText::kPracticeReturnShadeTree1, 0xD8BD3, 18},
	{ZoombiniText::kPracticeReturnShadeTree2, 0xD8BE6, 20},
	{ZoombiniText::kPracticeReturnShadeTree3, 0xD8BFB, 15},
	{ZoombiniText::kPracticeReturnShadeTree4, 0xD8C0B, 10},
	{ZoombiniText::kPracticeReturnZoombiniton1, 0xD8C16, 18},
	{ZoombiniText::kPracticeReturnZoombiniton2, 0xD8C29, 20},
	{ZoombiniText::kPracticeReturnZoombiniton3, 0xD8C3E, 16},
	{ZoombiniText::kPracticeReturnZoombiniton4, 0xD8C4F, 16},
	{ZoombiniText::kTerrainKey, 0xD8C60, 11},
	{ZoombiniText::kChooseLevel, 0xD8C6C, 14},
	{ZoombiniText::kLevel1, 0xD8C7B, 11},
	{ZoombiniText::kLevel2, 0xD8C87, 11},
	{ZoombiniText::kLevel3, 0xD8C93, 9},
	{ZoombiniText::kLevel4, 0xD8C9D, 15},
	{ZoombiniText::kRoute1, 0xD8CAD, 31},
	{ZoombiniText::kRoute2, 0xD8CCD, 11},
	{ZoombiniText::kRoute3, 0xD8CD9, 17},
	{ZoombiniText::kRoute4, 0xD8CEB, 20},
	{ZoombiniText::kXferVillePopulation, 0xD8D00, 24},
	{ZoombiniText::kMemorialJanuary, 0xD8D1A, 7},
	{ZoombiniText::kMemorialFebruary, 0xD8D22, 8},
	{ZoombiniText::kMemorialMarch, 0xD8D2B, 5},
	{ZoombiniText::kMemorialApril, 0xD8D31, 5},
	{ZoombiniText::kMemorialMay, 0xD8D37, 3},
	{ZoombiniText::kMemorialJune, 0xD8D3B, 4},
	{ZoombiniText::kMemorialJuly, 0xD8D40, 4},
	{ZoombiniText::kMemorialAugust, 0xD8D45, 6},
	{ZoombiniText::kMemorialSeptember, 0xD8D4C, 9},
	{ZoombiniText::kMemorialOctober, 0xD8D56, 7},
	{ZoombiniText::kMemorialNovember, 0xD8D5E, 8},
	{ZoombiniText::kMemorialDecember, 0xD8D67, 8},
	{ZoombiniText::kMemorialWhenLevel, 0xD8D70, 18},
	{ZoombiniText::kMemorialHonorMonument, 0xD8D83, 50},
	{ZoombiniText::kMemorialHonorWindmill, 0xD8DB6, 53},
	{ZoombiniText::kMemorialHonorObservatory, 0xD8DEC, 44},
	{ZoombiniText::kMemorialHonorBowlingAlley, 0xD8E19, 44},
	{ZoombiniText::kMemorialHonorGeneralStore, 0xD8E46, 53},
	{ZoombiniText::kMemorialHonorSwimmingPool, 0xD8E7C, 45},
	{ZoombiniText::kMemorialHonorPlayground, 0xD8EAA, 55},
	{ZoombiniText::kMemorialHonorBandShell, 0xD8EE2, 53},
	{ZoombiniText::kMemorialHonorSchool, 0xD8F18, 43},
	{ZoombiniText::kMemorialHonorLibrary, 0xD8F44, 51},
	{ZoombiniText::kMemorialHonorFire, 0xD8F78, 40},
	{ZoombiniText::kMemorialHonorOpera, 0xD8FA1, 52},
	{ZoombiniText::kMemorialHonorCityHall, 0xD8FD6, 44},
	{ZoombiniText::kMemorialHonorClockTower, 0xD9003, 55},
	{ZoombiniText::kMemorialHonorMuseum, 0xD903B, 54},
	{ZoombiniText::kMemorialHonorCourt, 0xD9072, 54},
	{ZoombiniText::kMemorialRoute1Level1, 0xD90A9, 100},
	{ZoombiniText::kMemorialRoute1Level2, 0xD910E, 106},
	{ZoombiniText::kMemorialRoute1Level3, 0xD9179, 102},
	{ZoombiniText::kMemorialRoute1Level4, 0xD91E0, 113},
	{ZoombiniText::kMemorialRoute2Level1, 0xD9252, 66},
	{ZoombiniText::kMemorialRoute2Level2, 0xD9295, 81},
	{ZoombiniText::kMemorialRoute2Level3, 0xD92E7, 92},
	{ZoombiniText::kMemorialRoute2Level4, 0xD9252, 66},
	{ZoombiniText::kMemorialRoute3Level1, 0xD9387, 106},
	{ZoombiniText::kMemorialRoute3Level2, 0xD93F2, 93},
	{ZoombiniText::kMemorialRoute3Level3, 0xD9450, 94},
	{ZoombiniText::kMemorialRoute3Level4, 0xD94AF, 98},
	{ZoombiniText::kMemorialRoute4Level1, 0xD9512, 103},
	{ZoombiniText::kMemorialRoute4Level2, 0xD957A, 110},
	{ZoombiniText::kMemorialRoute4Level3, 0xD95E9, 91},
	{ZoombiniText::kMemorialRoute4Level4, 0xD9645, 134},
	{ZoombiniText::kDialogBodyGoMapWillLost, 0xD96CC, 64},
	{ZoombiniText::kDialogButtonLoseThem, 0xD970D, 9},
	{ZoombiniText::kDialogButtonKeepThem, 0xD9717, 9},
	{ZoombiniText::kDialogButtonOkay, 0xD9721, 2},
	{ZoombiniText::kDialogButtonCancel, 0xD9724, 6},
	{ZoombiniText::kDialogButtonYes, 0xD98D2, 3},
	{ZoombiniText::kDialogButtonNo, 0xD9937, 2},
	{ZoombiniText::kDialogButtonQuit, 0xD9790, 4},
	{ZoombiniText::kDialogButtonLoad, 0xD972B, 4},
	{ZoombiniText::kDialogButtonSave, 0xD9730, 4},
	{ZoombiniText::kOptionsTitle, 0xD9735, 7},
	{ZoombiniText::kOptionsLegendOn, 0xD973D, 4},
	{ZoombiniText::kOptionsLegendOff, 0xD9742, 5},
	{ZoombiniText::kOptionsNewGame, 0xD9758, 17},
	{ZoombiniText::kOptionsLoadGame, 0xD976A, 18},
	{ZoombiniText::kOptionsSaveGame, 0xD977D, 18},
	{ZoombiniText::kOptionsQuit, 0xD9790, 13},
	{ZoombiniText::kOptionsToggle, 0xD9748, 15},
	{ZoombiniText::kOptionsSound, 0xD979E, 26},
	{ZoombiniText::kOptionsMusic, 0xD97B9, 25},
	{ZoombiniText::kOptionsStickyMouse, 0xD97D3, 21},
	{ZoombiniText::kOptionsTransitions, 0xD97E9, 20},
	{ZoombiniText::kOptionsCredits, 0xD98B2, 7},
	{ZoombiniText::kDialogBodyNoSavedGames, 0xD97FE, 14},
	{ZoombiniText::kDialogBodyCreateAndSaveNewGame, 0xD9810, 56},
	{ZoombiniText::kDialogButtonNewGame, 0xD9758, 8},
	{ZoombiniText::kDialogButtonReplaceTitle, 0xD9852, 7},
	{ZoombiniText::kDialogTitleSave, 0xD985A, 11},
	{ZoombiniText::kDialogTitleSaveAs, 0xD9866, 13},
	{ZoombiniText::kDialogTitleLoad, 0xD9874, 11},
	{ZoombiniText::kDialogBodyReplaceGame, 0xD9880, 48},
	{ZoombiniText::kDialogBodySaveCurrentGame, 0xD98BA, 23},
	{ZoombiniText::kDialogBodyCannotSaveInPractice, 0xD993A, 42},
	{ZoombiniText::kDialogBodyCreateNewGame, 0xD9965, 42},
	{ZoombiniText::kDialogBodyCannotSaveMoreGame, 0xD9990, 70},
	{ZoombiniText::kDialogBodyCannotLoadInPractice, 0xD99D7, 42},
	{ZoombiniText::kDialogBodyCannotCreateNewInPractice, 0xD9A02, 48},
	{ZoombiniText::kDialogBodyNewGame, 0xD9758, 8},
	{ZoombiniText::kDialogBodyReallyQuit, 0xD9A3C, 28},
	{ZoombiniText::kDialogBodySaveBeforeQuit, 0xD98DB, 87},
	{ZoombiniText::kDialogHelpLevel, 0xD6C6E, 5},
	{ZoombiniText::kNotiBoxMusicOn, 0xDA47E, 8},
	{ZoombiniText::kNotiBoxMusicOff, 0xDA487, 9},
	{ZoombiniText::kNotiBoxSoundOn, 0xDA491, 8},
	{ZoombiniText::kNotiBoxSoundOff, 0xDA49A, 9},
	{ZoombiniText::kNotiBoxLessAction, 0xDA4A4, 11},
	{ZoombiniText::kNotiBoxMoreAction, 0xDA4B0, 11},
	{ZoombiniText::kNotiBoxHideCursor, 0xDA4BC, 11},
	{ZoombiniText::kNotiBoxShowCursor, 0xDA4C8, 11},
	{ZoombiniText::kNotiBoxStickeyMouse, 0xDA4D4, 12},
	{ZoombiniText::kNotiBoxNonStickeyMouse, 0xDA4E1, 16},
	{ZoombiniText::kNotiBoxTransitionsOn, 0xDA4F2, 14},
	{ZoombiniText::kNotiBoxTransitionsOff, 0xDA501, 15},
	{ZoombiniText::kNotiBoxAutoStickeyOn, 0xDA511, 14},
	{ZoombiniText::kNotiBoxAutoStickeyOff, 0xDA520, 15},
};

const ZoombiniText::ExeTextEntry ZoombiniText::kV20US_PETextEntries[] = {
	{ZoombiniText::kTown, 0x92680, 13},
	{ZoombiniText::kPicker, 0x928C4, 13},
	{ZoombiniText::kBridge, 0x92264, 15},
	{ZoombiniText::kTunnels, 0x92283, 16},
	{ZoombiniText::kPizza, 0x92894, 10},
	{ZoombiniText::kBasecamp1, 0x92884, 12},
	{ZoombiniText::kFerry, 0x92868, 25},
	{ZoombiniText::kLilly, 0x92850, 22},
	{ZoombiniText::kSlides, 0x92844, 10},
	{ZoombiniText::kFleens, 0x9283C, 7},
	{ZoombiniText::kHotel, 0x91F72, 14},
	{ZoombiniText::kNet, 0x91ED6, 12},
	{ZoombiniText::kBasecamp2, 0x92810, 10},
	{ZoombiniText::kCaves, 0x92800, 15},
	{ZoombiniText::kSmoke, 0x91DEE, 14},
	{ZoombiniText::kMaze, 0x91E75, 18},
	{ZoombiniText::kNewGame, 0x919F0, 8},
	{ZoombiniText::kPracticeMode, 0x927BC, 13},
	{ZoombiniText::kContinueJourney, 0x9274C, 16},
	{ZoombiniText::kPracticeTitle, 0x92760, 13},
	{ZoombiniText::kPracticeDesc1, 0x927A8, 18},
	{ZoombiniText::kPracticeDesc2, 0x92794, 17},
	{ZoombiniText::kPracticeDesc3, 0x92780, 16},
	{ZoombiniText::kPracticeDesc4, 0x92770, 12},
	{ZoombiniText::kTerrainKey, 0x92740, 11},
	{ZoombiniText::kChooseLevel, 0x92730, 14},
	{ZoombiniText::kLevel1, 0x92724, 11},
	{ZoombiniText::kLevel2, 0x92718, 11},
	{ZoombiniText::kLevel3, 0x92702, 9},
	{ZoombiniText::kLevel4, 0x926FC, 15},
	{ZoombiniText::kRoute1, 0x926DC, 31},
	{ZoombiniText::kRoute2, 0x926D0, 11},
	{ZoombiniText::kRoute3, 0x926BC, 17},
	{ZoombiniText::kRoute4, 0x926A4, 20},
	{ZoombiniText::kRodMapOptionsButton, 0x9269C, 7},
	{ZoombiniText::kXferVillePopulation, 0x92680, 24},
	{ZoombiniText::kMemorialJanuary, 0x92678, 7},
	{ZoombiniText::kMemorialFebruary, 0x9266C, 8},
	{ZoombiniText::kMemorialMarch, 0x92664, 5},
	{ZoombiniText::kMemorialApril, 0x9265C, 5},
	{ZoombiniText::kMemorialMay, 0x92658, 3},
	{ZoombiniText::kMemorialJune, 0x92650, 4},
	{ZoombiniText::kMemorialJuly, 0x92648, 4},
	{ZoombiniText::kMemorialAugust, 0x92640, 6},
	{ZoombiniText::kMemorialSeptember, 0x92634, 9},
	{ZoombiniText::kMemorialOctober, 0x9262C, 7},
	{ZoombiniText::kMemorialNovember, 0x92620, 8},
	{ZoombiniText::kMemorialDecember, 0x92614, 8},
	{ZoombiniText::kMemorialWhenLevel, 0x92600, 18},
	{ZoombiniText::kMemorialHonorMonument, 0x925CC, 50},
	{ZoombiniText::kMemorialHonorWindmill, 0x92594, 53},
	{ZoombiniText::kMemorialHonorObservatory, 0x92564, 44},
	{ZoombiniText::kMemorialHonorBowlingAlley, 0x92534, 44},
	{ZoombiniText::kMemorialHonorGeneralStore, 0x924FC, 53},
	{ZoombiniText::kMemorialHonorSwimmingPool, 0x924CC, 45},
	{ZoombiniText::kMemorialHonorPlayground, 0x92494, 55},
	{ZoombiniText::kMemorialHonorBandShell, 0x9245C, 53},
	{ZoombiniText::kMemorialHonorSchool, 0x92430, 43},
	{ZoombiniText::kMemorialHonorLibrary, 0x923FC, 51},
	{ZoombiniText::kMemorialHonorFire, 0x923D0, 40},
	{ZoombiniText::kMemorialHonorOpera, 0x92398, 52},
	{ZoombiniText::kMemorialHonorCityHall, 0x92368, 44},
	{ZoombiniText::kMemorialHonorClockTower, 0x92330, 55},
	{ZoombiniText::kMemorialHonorMuseum, 0x922F8, 54},
	{ZoombiniText::kMemorialHonorCourt, 0x922C0, 54},
	{ZoombiniText::kMemorialRoute1Level1, 0x92258, 100},
	{ZoombiniText::kMemorialRoute1Level2, 0x921EC, 106},
	{ZoombiniText::kMemorialRoute1Level3, 0x92184, 102},
	{ZoombiniText::kMemorialRoute1Level4, 0x92110, 113},
	{ZoombiniText::kMemorialRoute2Level1, 0x920CC, 66},
	{ZoombiniText::kMemorialRoute2Level2, 0x92078, 81},
	{ZoombiniText::kMemorialRoute2Level3, 0x92018, 92},
	{ZoombiniText::kMemorialRoute2Level4, 0x920CC, 66},
	{ZoombiniText::kMemorialRoute3Level1, 0x91FAC, 106},
	{ZoombiniText::kMemorialRoute3Level2, 0x91F4C, 93},
	{ZoombiniText::kMemorialRoute3Level3, 0x91EEC, 94},
	{ZoombiniText::kMemorialRoute3Level4, 0x91E88, 98},
	{ZoombiniText::kMemorialRoute4Level1, 0x91E20, 103},
	{ZoombiniText::kMemorialRoute4Level2, 0x91DB0, 110},
	{ZoombiniText::kMemorialRoute4Level3, 0x91D54, 91},
	{ZoombiniText::kMemorialRoute4Level4, 0x91CCC, 134},
	{ZoombiniText::kDialogBodyGoMapWillLost, 0x91C88, 64},
	{ZoombiniText::kDialogButtonLoseThem, 0x91C7C, 9},
	{ZoombiniText::kDialogButtonKeepThem, 0x91C70, 9},
	{ZoombiniText::kDialogButtonOkay, 0x91C6C, 2},
	{ZoombiniText::kDialogButtonCancel, 0x91C64, 6},
	{ZoombiniText::kDialogButtonYes, 0x91A8C, 3},
	{ZoombiniText::kDialogButtonNo, 0x91A28, 2},
	{ZoombiniText::kDialogButtonQuit, 0x9191E, 4},
	{ZoombiniText::kDialogButtonLoad, 0x91AE4, 4},
	{ZoombiniText::kDialogButtonSave, 0x91AF0, 4},
	{ZoombiniText::kOptionsTitle, 0x91C40, 7},
	{ZoombiniText::kOptionsLegendOn, 0x91C38, 4},
	{ZoombiniText::kOptionsLegendOff, 0x91C30, 5},
	{ZoombiniText::kOptionsNewGame, 0x91C0C, 17},
	{ZoombiniText::kOptionsLoadGame, 0x91BF8, 18},
	{ZoombiniText::kOptionsSaveGame, 0x91BE4, 18},
	{ZoombiniText::kOptionsQuit, 0x91BD4, 13},
	{ZoombiniText::kOptionsToggle, 0x91C20, 15},
	{ZoombiniText::kOptionsSound, 0x91BB8, 26},
	{ZoombiniText::kOptionsMusic, 0x91B9C, 25},
	{ZoombiniText::kOptionsStickyMouse, 0x91B84, 21},
	{ZoombiniText::kOptionsTransitions, 0x91B6C, 20},
	{ZoombiniText::kOptionsCredits, 0x91AA8, 7},
	{ZoombiniText::kDialogBodyNoSavedGames, 0x91B5C, 14},
	{ZoombiniText::kDialogBodyCreateAndSaveNewGame, 0x91B20, 56},
	{ZoombiniText::kDialogButtonNewGame, 0x919F0, 8},
	{ZoombiniText::kDialogButtonReplaceTitle, 0x91B0C, 7},
	{ZoombiniText::kDialogTitleSave, 0x91B00, 11},
	{ZoombiniText::kDialogTitleSaveAs, 0x91AF0, 13},
	{ZoombiniText::kDialogTitleLoad, 0x91AE4, 11},
	{ZoombiniText::kDialogBodyReplaceGame, 0x91AB0, 48},
	{ZoombiniText::kDialogBodySaveCurrentGame, 0x91A90, 23},
	{ZoombiniText::kDialogBodySaveDirtyGame, 0x918C8, 60},
	{ZoombiniText::kDialogBodyCannotSaveInPractice, 0x919FC, 42},
	{ZoombiniText::kDialogBodyCreateNewGame, 0x919D0, 42},
	{ZoombiniText::kDialogBodyCannotSaveMoreGame, 0x91988, 70},
	{ZoombiniText::kDialogBodyCannotLoadInPractice, 0x9195C, 42},
	{ZoombiniText::kDialogBodyCannotCreateNewInPractice, 0x91928, 48},
	{ZoombiniText::kDialogBodyNewGame, 0x919F0, 8},
	{ZoombiniText::kDialogBodyReallyQuit, 0x91908, 28},
	{ZoombiniText::kDialogBodySaveBeforeQuit, 0x91A2C, 87},
	{ZoombiniText::kDialogHelpTitle, 0x91874, 4},
	{ZoombiniText::kDialogButtonPrev, 0x918B4, 8},
	{ZoombiniText::kDialogHelpLevel, 0x8B9D0, 5},
	{ZoombiniText::kDialogHelpPickerUpdateVersion, 0x932EC, 27},
	{ZoombiniText::kNotiBoxMusicOn, 0x907B4, 8},
	{ZoombiniText::kNotiBoxMusicOff, 0x907A8, 9},
	{ZoombiniText::kNotiBoxSoundOn, 0x9079C, 8},
	{ZoombiniText::kNotiBoxSoundOff, 0x90790, 9},
	{ZoombiniText::kNotiBoxLessAction, 0x90784, 11},
	{ZoombiniText::kNotiBoxMoreAction, 0x90778, 11},
	{ZoombiniText::kNotiBoxHideCursor, 0x9076C, 11},
	{ZoombiniText::kNotiBoxShowCursor, 0x90760, 11},
	{ZoombiniText::kNotiBoxStickeyMouse, 0x90740, 12},
	{ZoombiniText::kNotiBoxNonStickeyMouse, 0x9073C, 16},
	{ZoombiniText::kNotiBoxTransitionsOn, 0x9072C, 14},
	{ZoombiniText::kNotiBoxTransitionsOff, 0x9071C, 15},
	{ZoombiniText::kNotiBoxAutoStickeyOn, 0x9070C, 14},
	{ZoombiniText::kNotiBoxAutoStickeyOff, 0x906FC, 15},
	{ZoombiniText::kOptionsTitle, 0x91C40, 17},
	{ZoombiniText::kOptionsHelpAudio, 0x91874, 19},
	{ZoombiniText::kOptionsTouchSense, 0x91888, 30},
	{ZoombiniText::kDialogHelpTitle, 0x918C0, 4},
	{ZoombiniText::kDialogButtonOkay, 0x918A8, 2},
	{ZoombiniText::kDialogButtonNext, 0x918AC, 4},
	{ZoombiniText::kDialogBodyRemoveGame, 0x91848, 42},
	{ZoombiniText::kNotiBoxHelpAudioOn, 0x906CC, 13},
	{ZoombiniText::kNotiBoxHelpAudioOff, 0x906BC, 14},
	{ZoombiniText::kNotiBoxTouchSenseOn, 0x906EC, 14},
	{ZoombiniText::kNotiBoxTouchSenseOff, 0x906DC, 15},
};

const ZoombiniText::ExeTextEntry ZoombiniText::kV20PL_PETextEntries[] = {
	// One route pair shares a source buffer without a NUL between its logical entries.
	// Short dialog button labels use a prefix when the executable has no standalone copy.
	{ZoombiniText::kTown, 0xD012C, 8},
	{ZoombiniText::kPicker, 0xD0000, 17},
	{ZoombiniText::kBridge, 0xD0014, 18},
	{ZoombiniText::kTunnels, 0xD0028, 17},
	{ZoombiniText::kPizza, 0xD003C, 16},
	{ZoombiniText::kBasecamp1, 0xD0050, 17},
	{ZoombiniText::kFerry, 0xD0064, 26},
	{ZoombiniText::kLilly, 0xD0080, 17},
	{ZoombiniText::kSlides, 0xD0094, 14},
	{ZoombiniText::kFleens, 0xD00A4, 11},
	{ZoombiniText::kHotel, 0xD00B0, 11},
	{ZoombiniText::kNet, 0xD00BC, 16},
	{ZoombiniText::kBasecamp2, 0xD00D0, 20},
	{ZoombiniText::kCaves, 0xD00E8, 22},
	{ZoombiniText::kSmoke, 0xD0100, 21},
	{ZoombiniText::kMaze, 0xD0118, 16},
	{ZoombiniText::kNewGame, 0xC9AF4, 9},
	{ZoombiniText::kPracticeMode, 0xD0184, 13},
	{ZoombiniText::kContinueJourney, 0xD02E4, 12},
	{ZoombiniText::kPracticeTitle, 0xD02D4, 13},
	{ZoombiniText::kPracticeDesc1, 0xD0194, 16},
	{ZoombiniText::kPracticeDesc2, 0xD01A8, 14},
	{ZoombiniText::kPracticeDesc3, 0xD01B8, 22},
	{ZoombiniText::kPracticeDesc4, 0xD01D0, 17},
	{ZoombiniText::kPolishJourneyStatPicker, 0xD0138, 17},
	{ZoombiniText::kPolishJourneyStatBasecamp1, 0xD014C, 17},
	{ZoombiniText::kPolishJourneyStatBasecamp2, 0xD0160, 20},
	{ZoombiniText::kPolishJourneyStatTown, 0xD0178, 8},
	{ZoombiniText::kTerrainKey, 0xD02F4, 16},
	{ZoombiniText::kChooseLevel, 0xD0308, 24},
	{ZoombiniText::kLevel1, 0xD0324, 12},
	{ZoombiniText::kLevel2, 0xD0334, 17},
	{ZoombiniText::kLevel3, 0xD0348, 19},
	{ZoombiniText::kLevel4, 0xD035C, 22},
	{ZoombiniText::kRoute1, 0xD0374, 30},
	{ZoombiniText::kRoute2, 0xD0394, 14},
	{ZoombiniText::kRoute3, 0xD03A4, 10},
	{ZoombiniText::kRoute4, 0xD03B0, 16},
	{ZoombiniText::kRodMapOptionsButton, 0xD03C4, 5},
	{ZoombiniText::kXferVillePopulation, 0xD03CC, 28},
	{ZoombiniText::kMemorialJanuary, 0xD03EC, 7},
	{ZoombiniText::kMemorialFebruary, 0xD03F4, 4},
	{ZoombiniText::kMemorialMarch, 0xD03FC, 6},
	{ZoombiniText::kMemorialApril, 0xD0404, 8},
	{ZoombiniText::kMemorialMay, 0xD0410, 3},
	{ZoombiniText::kMemorialJune, 0xD0414, 8},
	{ZoombiniText::kMemorialJuly, 0xD0420, 6},
	{ZoombiniText::kMemorialAugust, 0xD0428, 8},
	{ZoombiniText::kMemorialSeptember, 0xD0434, 8},
	{ZoombiniText::kMemorialOctober, 0xD0440, 11},
	{ZoombiniText::kMemorialNovember, 0xD044C, 8},
	{ZoombiniText::kMemorialDecember, 0xD0458, 8},
	{ZoombiniText::kMemorialWhenLevel, 0xD0464, 23},
	{ZoombiniText::kMemorialHonorMonument, 0xD047C, 47},
	{ZoombiniText::kMemorialHonorWindmill, 0xD04AC, 48},
	{ZoombiniText::kMemorialHonorObservatory, 0xD04E0, 55},
	{ZoombiniText::kMemorialHonorBowlingAlley, 0xD0518, 50},
	{ZoombiniText::kMemorialHonorGeneralStore, 0xD054C, 46},
	{ZoombiniText::kMemorialHonorSwimmingPool, 0xD057C, 51},
	{ZoombiniText::kMemorialHonorPlayground, 0xD05B0, 51},
	{ZoombiniText::kMemorialHonorBandShell, 0xD05E4, 57},
	{ZoombiniText::kMemorialHonorSchool, 0xD0620, 55},
	{ZoombiniText::kMemorialHonorLibrary, 0xD0658, 53},
	{ZoombiniText::kMemorialHonorFire, 0xD0690, 67},
	{ZoombiniText::kMemorialHonorOpera, 0xD06D4, 50},
	{ZoombiniText::kMemorialHonorCityHall, 0xD0708, 52},
	{ZoombiniText::kMemorialHonorClockTower, 0xD0740, 58},
	{ZoombiniText::kMemorialHonorMuseum, 0xD077C, 56},
	{ZoombiniText::kMemorialHonorCourt, 0xD07B8, 52},
	{ZoombiniText::kMemorialRoute1Level1, 0xD07F0, 124},
	{ZoombiniText::kMemorialRoute1Level2, 0xD0870, 135},
	{ZoombiniText::kMemorialRoute1Level3, 0xD08F8, 141},
	{ZoombiniText::kMemorialRoute1Level4, 0xD0988, 152},
	{ZoombiniText::kMemorialRoute2Level1, 0xD0A20, 131},
	{ZoombiniText::kMemorialRoute2Level2, 0xD0AA4, 101},
	{ZoombiniText::kMemorialRoute2Level3, 0xD0B0C, 118},
	{ZoombiniText::kMemorialRoute2Level4, 0xD0B84, 128},
	{ZoombiniText::kMemorialRoute3Level1, 0xD0C08, 109},
	{ZoombiniText::kMemorialRoute3Level2, 0xD0C78, 109},
	{ZoombiniText::kMemorialRoute3Level3, 0xD0CE8, 118},
	{ZoombiniText::kMemorialRoute3Level4, 0xD0D60, 106},
	{ZoombiniText::kMemorialRoute4Level1, 0xD0DCC, 142},
	{ZoombiniText::kMemorialRoute4Level2, 0xD0E5C, 147},
	{ZoombiniText::kMemorialRoute4Level3, 0xD0EF0, 115},
	{ZoombiniText::kMemorialRoute4Level4, 0xD0F64, 163},
	{ZoombiniText::kDialogBodyGoMapWillLost, 0xD1008, 64},
	{ZoombiniText::kDialogButtonLoseThem, 0xD104C, 6},
	{ZoombiniText::kDialogButtonKeepThem, 0xD1054, 13},
	{ZoombiniText::kDialogButtonOkay, 0xD1064, 4},
	{ZoombiniText::kDialogButtonCancel, 0xD106C, 7},
	{ZoombiniText::kDialogButtonYes, 0xD125C, 3},
	{ZoombiniText::kDialogButtonNo, 0xD12B4, 3},
	{ZoombiniText::kDialogButtonQuit, 0xD1260, 5},
	{ZoombiniText::kDialogButtonLoad, 0xD1074, 7},
	{ZoombiniText::kDialogButtonSave, 0xD11F0, 6},
	{ZoombiniText::kOptionsTitle, 0xD1088, 15},
	{ZoombiniText::kOptionsLegendOn, 0xD1098, 5},
	{ZoombiniText::kOptionsLegendOff, 0xD10A0, 6},
	{ZoombiniText::kOptionsNewGame, 0xD10CC, 17},
	{ZoombiniText::kOptionsLoadGame, 0xD10E0, 20},
	{ZoombiniText::kOptionsSaveGame, 0xD10F8, 19},
	{ZoombiniText::kOptionsQuit, 0xD110C, 14},
	{ZoombiniText::kOptionsToggle, 0xD10A8, 34},
	{ZoombiniText::kOptionsSound, 0xD111C, 35},
	{ZoombiniText::kOptionsMusic, 0xD1140, 15},
	{ZoombiniText::kOptionsStickyMouse, 0xD1150, 33},
	{ZoombiniText::kOptionsTransitions, 0xD1174, 18},
	{ZoombiniText::kOptionsCredits, 0xD1240, 6},
	{ZoombiniText::kDialogBodyNoSavedGames, 0xD1188, 20},
	{ZoombiniText::kDialogBodyCreateAndSaveNewGame, 0xD11A8, 48},
	{ZoombiniText::kDialogButtonNewGame, 0xD11DC, 8},
	{ZoombiniText::kDialogButtonReplaceTitle, 0xD11E8, 6},
	{ZoombiniText::kDialogTitleSave, 0xD11F0, 10},
	{ZoombiniText::kDialogTitleSaveAs, 0xD11FC, 16},
	{ZoombiniText::kDialogTitleLoad, 0xD1210, 11},
	{ZoombiniText::kDialogBodyReplaceGame, 0xD121C, 34},
	{ZoombiniText::kDialogBodySaveCurrentGame, 0xD1248, 16},
	{ZoombiniText::kDialogBodySaveDirtyGame, 0xD1268, 71},
	{ZoombiniText::kDialogBodyCannotSaveInPractice, 0xD12B8, 40},
	{ZoombiniText::kDialogBodyCreateNewGame, 0xD12E4, 36},
	{ZoombiniText::kDialogBodyCannotSaveMoreGame, 0xD130C, 67},
	{ZoombiniText::kDialogBodyCannotLoadInPractice, 0xD1350, 40},
	{ZoombiniText::kDialogBodyCannotCreateNewInPractice, 0xD137C, 47},
	{ZoombiniText::kDialogBodyNewGame, 0xD13AC, 8},
	{ZoombiniText::kDialogBodyReallyQuit, 0xD13B8, 29},
	{ZoombiniText::kDialogBodySaveBeforeQuit, 0xD13D8, 51},
	{ZoombiniText::kDialogHelpTitle, 0xD140C, 5},
	{ZoombiniText::kDialogButtonPrev, 0xD1414, 4},
	{ZoombiniText::kDialogHelpLevel, 0xD02F4, 6},
	{ZoombiniText::kDialogHelpPickerUpdateVersion, 0xCDBC0, 27},
	{ZoombiniText::kNotiBoxMusicOn, 0xD27E0, 12},
	{ZoombiniText::kNotiBoxMusicOff, 0xD27F0, 13},
	{ZoombiniText::kNotiBoxSoundOn, 0xD2800, 12},
	{ZoombiniText::kNotiBoxSoundOff, 0xD2810, 13},
	{ZoombiniText::kNotiBoxLessAction, 0xD2820, 11},
	{ZoombiniText::kNotiBoxMoreAction, 0xD282C, 12},
	{ZoombiniText::kNotiBoxHideCursor, 0xD283C, 12},
	{ZoombiniText::kNotiBoxShowCursor, 0xD284C, 12},
	{ZoombiniText::kNotiBoxStickeyMouse, 0xD285C, 24},
	{ZoombiniText::kNotiBoxNonStickeyMouse, 0xD2878, 27},
	{ZoombiniText::kNotiBoxTransitionsOn, 0xD2894, 15},
	{ZoombiniText::kNotiBoxTransitionsOff, 0xD28A4, 16},
	{ZoombiniText::kNotiBoxAutoStickeyOn, 0xD28B8, 22},
	{ZoombiniText::kNotiBoxAutoStickeyOff, 0xD28D0, 23},
	{ZoombiniText::kOptionsTitle, 0xD1088, 15},
	{ZoombiniText::kOptionsHelpAudio, 0xD144C, 20},
	{ZoombiniText::kOptionsTouchSense, 0xD142C, 30},
	{ZoombiniText::kDialogHelpTitle, 0xD140C, 5},
	{ZoombiniText::kDialogButtonOkay, 0xD1424, 4},
	{ZoombiniText::kDialogButtonNext, 0xD141C, 5},
	{ZoombiniText::kDialogBodyRemoveGame, 0xD1464, 30},
	{ZoombiniText::kNotiBoxHelpAudioOn, 0xD2910, 17},
	{ZoombiniText::kNotiBoxHelpAudioOff, 0xD2924, 18},
	{ZoombiniText::kNotiBoxTouchSenseOn, 0xD28E8, 17},
	{ZoombiniText::kNotiBoxTouchSenseOff, 0xD28FC, 18},
};

const ZoombiniText::ExeTextEntry ZoombiniText::kV20USDemo_PETextEntries[] = {
	{ZoombiniText::kTown, 0x91648, 13},
	{ZoombiniText::kPicker, 0x9188C, 13},
	{ZoombiniText::kBridge, 0x9122C, 15},
	{ZoombiniText::kTunnels, 0x9124B, 16},
	{ZoombiniText::kPizza, 0x9185C, 10},
	{ZoombiniText::kBasecamp1, 0x9184C, 12},
	{ZoombiniText::kFerry, 0x91830, 25},
	{ZoombiniText::kLilly, 0x91818, 22},
	{ZoombiniText::kSlides, 0x9180C, 10},
	{ZoombiniText::kFleens, 0x91804, 7},
	{ZoombiniText::kHotel, 0x90F3A, 14},
	{ZoombiniText::kNet, 0x90E9E, 12},
	{ZoombiniText::kBasecamp2, 0x917D8, 10},
	{ZoombiniText::kCaves, 0x917C8, 15},
	{ZoombiniText::kSmoke, 0x90DB6, 14},
	{ZoombiniText::kMaze, 0x90E3D, 18},
	{ZoombiniText::kNewGame, 0x909B8, 8},
	{ZoombiniText::kPracticeMode, 0x91784, 13},
	{ZoombiniText::kContinueJourney, 0x91714, 16},
	{ZoombiniText::kPracticeTitle, 0x91728, 13},
	{ZoombiniText::kPracticeDesc1, 0x91770, 18},
	{ZoombiniText::kPracticeDesc2, 0x9175C, 17},
	{ZoombiniText::kPracticeDesc3, 0x91748, 16},
	{ZoombiniText::kPracticeDesc4, 0x91738, 12},
	{ZoombiniText::kTerrainKey, 0x91708, 11},
	{ZoombiniText::kChooseLevel, 0x916F8, 14},
	{ZoombiniText::kLevel1, 0x916EC, 11},
	{ZoombiniText::kLevel2, 0x916E0, 11},
	{ZoombiniText::kLevel3, 0x916CA, 9},
	{ZoombiniText::kLevel4, 0x916C4, 15},
	{ZoombiniText::kRoute1, 0x916A4, 31},
	{ZoombiniText::kRoute2, 0x91698, 11},
	{ZoombiniText::kRoute3, 0x91684, 17},
	{ZoombiniText::kRoute4, 0x9166C, 20},
	{ZoombiniText::kRodMapOptionsButton, 0x91664, 7},
	{ZoombiniText::kXferVillePopulation, 0x91648, 24},
	{ZoombiniText::kMemorialJanuary, 0x91640, 7},
	{ZoombiniText::kMemorialFebruary, 0x91634, 8},
	{ZoombiniText::kMemorialMarch, 0x9162C, 5},
	{ZoombiniText::kMemorialApril, 0x91624, 5},
	{ZoombiniText::kMemorialMay, 0x91620, 3},
	{ZoombiniText::kMemorialJune, 0x91618, 4},
	{ZoombiniText::kMemorialJuly, 0x91610, 4},
	{ZoombiniText::kMemorialAugust, 0x91608, 6},
	{ZoombiniText::kMemorialSeptember, 0x915FC, 9},
	{ZoombiniText::kMemorialOctober, 0x915F4, 7},
	{ZoombiniText::kMemorialNovember, 0x915E8, 8},
	{ZoombiniText::kMemorialDecember, 0x915DC, 8},
	{ZoombiniText::kMemorialWhenLevel, 0x915C8, 18},
	{ZoombiniText::kMemorialHonorMonument, 0x91594, 50},
	{ZoombiniText::kMemorialHonorWindmill, 0x9155C, 53},
	{ZoombiniText::kMemorialHonorObservatory, 0x9152C, 44},
	{ZoombiniText::kMemorialHonorBowlingAlley, 0x914FC, 44},
	{ZoombiniText::kMemorialHonorGeneralStore, 0x914C4, 53},
	{ZoombiniText::kMemorialHonorSwimmingPool, 0x91494, 45},
	{ZoombiniText::kMemorialHonorPlayground, 0x9145C, 55},
	{ZoombiniText::kMemorialHonorBandShell, 0x91424, 53},
	{ZoombiniText::kMemorialHonorSchool, 0x913F8, 43},
	{ZoombiniText::kMemorialHonorLibrary, 0x913C4, 51},
	{ZoombiniText::kMemorialHonorFire, 0x91398, 40},
	{ZoombiniText::kMemorialHonorOpera, 0x91360, 52},
	{ZoombiniText::kMemorialHonorCityHall, 0x91330, 44},
	{ZoombiniText::kMemorialHonorClockTower, 0x912F8, 55},
	{ZoombiniText::kMemorialHonorMuseum, 0x912C0, 54},
	{ZoombiniText::kMemorialHonorCourt, 0x91288, 54},
	{ZoombiniText::kMemorialRoute1Level1, 0x91220, 100},
	{ZoombiniText::kMemorialRoute1Level2, 0x911B4, 106},
	{ZoombiniText::kMemorialRoute1Level3, 0x9114C, 102},
	{ZoombiniText::kMemorialRoute1Level4, 0x910D8, 113},
	{ZoombiniText::kMemorialRoute2Level1, 0x91094, 66},
	{ZoombiniText::kMemorialRoute2Level2, 0x91040, 81},
	{ZoombiniText::kMemorialRoute2Level3, 0x90FE0, 92},
	{ZoombiniText::kMemorialRoute2Level4, 0x91094, 66},
	{ZoombiniText::kMemorialRoute3Level1, 0x90F74, 106},
	{ZoombiniText::kMemorialRoute3Level2, 0x90F14, 93},
	{ZoombiniText::kMemorialRoute3Level3, 0x90EB4, 94},
	{ZoombiniText::kMemorialRoute3Level4, 0x90E50, 98},
	{ZoombiniText::kMemorialRoute4Level1, 0x90DE8, 103},
	{ZoombiniText::kMemorialRoute4Level2, 0x90D78, 110},
	{ZoombiniText::kMemorialRoute4Level3, 0x90D1C, 91},
	{ZoombiniText::kMemorialRoute4Level4, 0x90C94, 134},
	{ZoombiniText::kDialogBodyGoMapWillLost, 0x90C50, 64},
	{ZoombiniText::kDialogButtonLoseThem, 0x90C44, 9},
	{ZoombiniText::kDialogButtonKeepThem, 0x90C38, 9},
	{ZoombiniText::kDialogButtonOkay, 0x90C34, 2},
	{ZoombiniText::kDialogButtonCancel, 0x90C2C, 6},
	{ZoombiniText::kDialogButtonYes, 0x90A54, 3},
	{ZoombiniText::kDialogButtonNo, 0x909F0, 2},
	{ZoombiniText::kDialogButtonQuit, 0x908E6, 4},
	{ZoombiniText::kDialogButtonLoad, 0x90AAC, 4},
	{ZoombiniText::kDialogButtonSave, 0x90AB8, 4},
	{ZoombiniText::kOptionsTitle, 0x90C08, 7},
	{ZoombiniText::kOptionsLegendOn, 0x90C00, 4},
	{ZoombiniText::kOptionsLegendOff, 0x90BF8, 5},
	{ZoombiniText::kOptionsNewGame, 0x90BD4, 17},
	{ZoombiniText::kOptionsLoadGame, 0x90BC0, 18},
	{ZoombiniText::kOptionsSaveGame, 0x90BAC, 18},
	{ZoombiniText::kOptionsQuit, 0x90B9C, 13},
	{ZoombiniText::kOptionsToggle, 0x90BE8, 15},
	{ZoombiniText::kOptionsSound, 0x90B80, 26},
	{ZoombiniText::kOptionsMusic, 0x90B64, 25},
	{ZoombiniText::kOptionsStickyMouse, 0x90B4C, 21},
	{ZoombiniText::kOptionsTransitions, 0x90B34, 20},
	{ZoombiniText::kOptionsCredits, 0x90A70, 7},
	{ZoombiniText::kDialogBodyNoSavedGames, 0x90B24, 14},
	{ZoombiniText::kDialogBodyCreateAndSaveNewGame, 0x90AE8, 56},
	{ZoombiniText::kDialogButtonNewGame, 0x909B8, 8},
	{ZoombiniText::kDialogButtonReplaceTitle, 0x90AD4, 7},
	{ZoombiniText::kDialogTitleSave, 0x90AC8, 11},
	{ZoombiniText::kDialogTitleSaveAs, 0x90AB8, 13},
	{ZoombiniText::kDialogTitleLoad, 0x90AAC, 11},
	{ZoombiniText::kDialogBodyReplaceGame, 0x90A78, 48},
	{ZoombiniText::kDialogBodySaveCurrentGame, 0x90A58, 23},
	{ZoombiniText::kDialogBodySaveDirtyGame, 0x90890, 60},
	{ZoombiniText::kDialogBodyCannotSaveInPractice, 0x909C4, 42},
	{ZoombiniText::kDialogBodyCreateNewGame, 0x90998, 42},
	{ZoombiniText::kDialogBodyCannotSaveMoreGame, 0x90950, 70},
	{ZoombiniText::kDialogBodyCannotLoadInPractice, 0x90924, 42},
	{ZoombiniText::kDialogBodyCannotCreateNewInPractice, 0x908F0, 48},
	{ZoombiniText::kDialogBodyNewGame, 0x909B8, 8},
	{ZoombiniText::kDialogBodyReallyQuit, 0x908D0, 28},
	{ZoombiniText::kDialogBodySaveBeforeQuit, 0x909F4, 87},
	{ZoombiniText::kDialogHelpTitle, 0x9083C, 4},
	{ZoombiniText::kDialogButtonPrev, 0x9087C, 8},
	{ZoombiniText::kDialogHelpLevel, 0x8A9D0, 5},
	{ZoombiniText::kDialogHelpPickerUpdateVersion, 0x92284, 27},
	{ZoombiniText::kNotiBoxMusicOn, 0x8F77C, 8},
	{ZoombiniText::kNotiBoxMusicOff, 0x8F770, 9},
	{ZoombiniText::kNotiBoxSoundOn, 0x8F764, 8},
	{ZoombiniText::kNotiBoxSoundOff, 0x8F758, 9},
	{ZoombiniText::kNotiBoxLessAction, 0x8F74C, 11},
	{ZoombiniText::kNotiBoxMoreAction, 0x8F740, 11},
	{ZoombiniText::kNotiBoxHideCursor, 0x8F734, 11},
	{ZoombiniText::kNotiBoxShowCursor, 0x8F728, 11},
	{ZoombiniText::kNotiBoxStickeyMouse, 0x8F708, 12},
	{ZoombiniText::kNotiBoxNonStickeyMouse, 0x8F704, 16},
	{ZoombiniText::kNotiBoxTransitionsOn, 0x8F6F4, 14},
	{ZoombiniText::kNotiBoxTransitionsOff, 0x8F6E4, 15},
	{ZoombiniText::kNotiBoxAutoStickeyOn, 0x8F6D4, 14},
	{ZoombiniText::kNotiBoxAutoStickeyOff, 0x8F6C4, 15},
	{ZoombiniText::kOptionsTitle, 0x90C08, 17},
	{ZoombiniText::kOptionsHelpAudio, 0x9083C, 19},
	{ZoombiniText::kOptionsTouchSense, 0x90850, 30},
	{ZoombiniText::kDialogHelpTitle, 0x90888, 4},
	{ZoombiniText::kDialogButtonOkay, 0x90870, 2},
	{ZoombiniText::kDialogButtonNext, 0x90874, 4},
	{ZoombiniText::kDialogBodyRemoveGame, 0x90810, 42},
	{ZoombiniText::kNotiBoxHelpAudioOn, 0x8F694, 13},
	{ZoombiniText::kNotiBoxHelpAudioOff, 0x8F684, 14},
	{ZoombiniText::kNotiBoxTouchSenseOn, 0x8F6B4, 14},
	{ZoombiniText::kNotiBoxTouchSenseOff, 0x8F6A4, 15},
};

const ZoombiniText::ExeTextEntry ZoombiniText::kV111KR_PETextEntries[] = {
	{ZoombiniText::kPicker, 0x8B8EC, 9},
	{ZoombiniText::kBridge, 0x8B8F6, 13},
	{ZoombiniText::kCaves, 0x8B97D, 9},
	{ZoombiniText::kPizza, 0x8B915, 9},
	{ZoombiniText::kBasecamp1, 0x8B91F, 13},
	{ZoombiniText::kFerry, 0x8B92D, 9},
	{ZoombiniText::kLilly, 0x8B937, 11},
	{ZoombiniText::kSlides, 0x8B943, 9},
	{ZoombiniText::kFleens, 0x8B94D, 11},
	{ZoombiniText::kHotel, 0x8B959, 9},
	{ZoombiniText::kNet, 0x8B963, 11},
	{ZoombiniText::kBasecamp2, 0x8B96F, 13},
	{ZoombiniText::kTunnels, 0x8B904, 16},
	{ZoombiniText::kSmoke, 0x8B987, 9},
	{ZoombiniText::kMaze, 0x8B991, 11},
	{ZoombiniText::kTown, 0x8B99D, 11},
	{ZoombiniText::kNewGame, 0x8A866, 7},
	{ZoombiniText::kPracticeMode, 0x8BAFD, 11},
	{ZoombiniText::kContinueJourney, 0x8BB09, 14},
	{ZoombiniText::kPracticeTitle, 0x8B9DB, 9},
	{ZoombiniText::kPracticeDesc1, 0x8B9E5, 15},
	{ZoombiniText::kPracticeDesc2, 0x8B9F5, 16},
	{ZoombiniText::kPracticeDesc3, 0x8BA06, 18},
	{ZoombiniText::kPracticeDesc4, 0x8BA19, 17},
	{ZoombiniText::kTerrainKey, 0x8BB18, 11},
	{ZoombiniText::kChooseLevel, 0x8BB24, 11},
	{ZoombiniText::kLevel1, 0x8BB30, 5},
	{ZoombiniText::kLevel2, 0x8BB36, 5},
	{ZoombiniText::kLevel3, 0x8BB3C, 5},
	{ZoombiniText::kLevel4, 0x8BB42, 5},
	{ZoombiniText::kRoute1, 0x8BB48, 11},
	{ZoombiniText::kRoute2, 0x8BB54, 11},
	{ZoombiniText::kRoute3, 0x8BB60, 16},
	{ZoombiniText::kRoute4, 0x8BB71, 11},
	{ZoombiniText::kXferVillePopulation, 0x8BB7D, 17},
	{ZoombiniText::kMemorialJanuary, 0x8BB8F, 3},
	{ZoombiniText::kMemorialFebruary, 0x8BB93, 3},
	{ZoombiniText::kMemorialMarch, 0x8BB97, 3},
	{ZoombiniText::kMemorialApril, 0x8BB9B, 3},
	{ZoombiniText::kMemorialMay, 0x8BB9F, 3},
	{ZoombiniText::kMemorialJune, 0x8BBA3, 3},
	{ZoombiniText::kMemorialJuly, 0x8BBA7, 3},
	{ZoombiniText::kMemorialAugust, 0x8BBAB, 3},
	{ZoombiniText::kMemorialSeptember, 0x8BBAF, 3},
	{ZoombiniText::kMemorialOctober, 0x8BBB3, 4},
	{ZoombiniText::kMemorialNovember, 0x8BBB8, 4},
	{ZoombiniText::kMemorialDecember, 0x8BBBD, 4},
	{ZoombiniText::kMemorialWhenLevel, 0x8BBC2, 26},
	{ZoombiniText::kMemorialHonorMonument, 0x8BBDD, 53},
	{ZoombiniText::kMemorialHonorWindmill, 0x8BC13, 51},
	{ZoombiniText::kMemorialHonorObservatory, 0x8BC47, 53},
	{ZoombiniText::kMemorialHonorBowlingAlley, 0x8BC7D, 53},
	{ZoombiniText::kMemorialHonorGeneralStore, 0x8BCB3, 51},
	{ZoombiniText::kMemorialHonorSwimmingPool, 0x8BCE7, 53},
	{ZoombiniText::kMemorialHonorPlayground, 0x8BD1D, 53},
	{ZoombiniText::kMemorialHonorBandShell, 0x8BD53, 53},
	{ZoombiniText::kMemorialHonorSchool, 0x8BD89, 51},
	{ZoombiniText::kMemorialHonorLibrary, 0x8BDBD, 53},
	{ZoombiniText::kMemorialHonorFire, 0x8BDF3, 53},
	{ZoombiniText::kMemorialHonorOpera, 0x8BE29, 60},
	{ZoombiniText::kMemorialHonorCityHall, 0x8BE66, 51},
	{ZoombiniText::kMemorialHonorClockTower, 0x8BE9A, 53},
	{ZoombiniText::kMemorialHonorMuseum, 0x8BED0, 53},
	{ZoombiniText::kMemorialHonorCourt, 0x8BF06, 51},
	{ZoombiniText::kMemorialRoute1Level1, 0x8BF3A, 109},
	{ZoombiniText::kMemorialRoute1Level2, 0x8BFA8, 109},
	{ZoombiniText::kMemorialRoute1Level3, 0x8C016, 124},
	{ZoombiniText::kMemorialRoute1Level4, 0x8C093, 109},
	{ZoombiniText::kMemorialRoute2Level1, 0x8C22C, 91},
	{ZoombiniText::kMemorialRoute2Level2, 0x8C15C, 88},
	{ZoombiniText::kMemorialRoute2Level3, 0x8C1B5, 118},
	{ZoombiniText::kMemorialRoute2Level4, 0x8C22C, 91},
	{ZoombiniText::kMemorialRoute3Level1, 0x8C288, 138},
	{ZoombiniText::kMemorialRoute3Level2, 0x8C313, 118},
	{ZoombiniText::kMemorialRoute3Level3, 0x8C38A, 100},
	{ZoombiniText::kMemorialRoute3Level4, 0x8C3EF, 107},
	{ZoombiniText::kMemorialRoute4Level1, 0x8C45B, 103},
	{ZoombiniText::kMemorialRoute4Level2, 0x8C4C3, 118},
	{ZoombiniText::kMemorialRoute4Level3, 0x8C53A, 117},
	{ZoombiniText::kMemorialRoute4Level4, 0x8C5B0, 116},
	{ZoombiniText::kDialogBodyGoMapWillLost, 0x8C625, 52},
	{ZoombiniText::kDialogButtonLoseThem, 0x8C65A, 9},
	{ZoombiniText::kDialogButtonKeepThem, 0x8C664, 10},
	{ZoombiniText::kDialogButtonOkay, 0x8C66F, 4},
	{ZoombiniText::kDialogButtonCancel, 0x8C674, 4},
	{ZoombiniText::kDialogButtonYes, 0x8C82C, 2},
	{ZoombiniText::kDialogButtonNo, 0x8C82F, 6},
	{ZoombiniText::kDialogButtonLoad, 0x8C679, 8},
	{ZoombiniText::kDialogButtonSave, 0x8C682, 8},
	{ZoombiniText::kOptionsTitle, 0x8C68B, 4},
	{ZoombiniText::kOptionsLegendOn, 0x8C690, 4},
	{ZoombiniText::kOptionsLegendOff, 0x8C695, 4},
	{ZoombiniText::kOptionsToggle, 0x8C69A, 7},
	{ZoombiniText::kOptionsNewGame, 0x8C6A2, 18},
	{ZoombiniText::kOptionsLoadGame, 0x8C6B5, 17},
	{ZoombiniText::kOptionsSaveGame, 0x8C6C7, 17},
	{ZoombiniText::kOptionsQuit, 0x8C6D9, 15},
	{ZoombiniText::kOptionsSound, 0x8C6E9, 18},
	{ZoombiniText::kOptionsMusic, 0x8C6FC, 18},
	{ZoombiniText::kOptionsStickyMouse, 0x8C70F, 22},
	{ZoombiniText::kOptionsTransitions, 0x8C726, 18},
	{ZoombiniText::kOptionsCredits, 0x8C803, 10},
	{ZoombiniText::kDialogBodyNoSavedGames, 0x8C739, 23},
	{ZoombiniText::kDialogBodyCreateAndSaveNewGame, 0x8C756, 58},
	{ZoombiniText::kDialogButtonNewGame, 0x8C791, 11},
	{ZoombiniText::kDialogButtonReplaceTitle, 0x8C79D, 8},
	{ZoombiniText::kDialogTitleSave, 0x8C682, 8},
	{ZoombiniText::kDialogTitleSaveAs, 0x8C7AF, 18},
	{ZoombiniText::kDialogTitleLoad, 0x8C679, 8},
	{ZoombiniText::kDialogBodyReplaceGame, 0x8C7CB, 55},
	{ZoombiniText::kDialogBodySaveCurrentGame, 0x8C80E, 29},
	{ZoombiniText::kDialogBodySaveDirtyGame, 0x8C967, 62},
	{ZoombiniText::kDialogBodyCannotSaveInPractice, 0x8C884, 41},
	{ZoombiniText::kDialogBodyCreateNewGame, 0x8C8AE, 25},
	{ZoombiniText::kDialogBodyCannotSaveMoreGame, 0x8C8C8, 35},
	{ZoombiniText::kDialogBodyCannotLoadInPractice, 0x8C8EC, 48},
	{ZoombiniText::kDialogBodyCannotCreateNewInPractice, 0x8C91D, 44},
	{ZoombiniText::kDialogBodyNewGame, 0x8C8AE, 7},
	{ZoombiniText::kDialogBodyReallyQuit, 0x8C952, 20},
	{ZoombiniText::kDialogBodySaveBeforeQuit, 0x8C836, 67},
	{ZoombiniText::kDialogHelpTitle, 0x8C9A6, 6},
	{ZoombiniText::kDialogButtonPrev, 0x8C9AD, 4},
	{ZoombiniText::kDialogButtonNext, 0x8C9B2, 4},
	{ZoombiniText::kDialogHelpLevel, 0x8B9F6, 4},
	{ZoombiniText::kDialogHelpPickerUpdateVersion, 0x8E1AB, 27},
	{ZoombiniText::kNotiBoxMusicOn, 0x8D4AB, 12},
	{ZoombiniText::kNotiBoxMusicOff, 0x8D4B8, 12},
	{ZoombiniText::kNotiBoxSoundOn, 0x8D4C5, 12},
	{ZoombiniText::kNotiBoxSoundOff, 0x8D4D2, 12},
	{ZoombiniText::kNotiBoxLessAction, 0x8D4DF, 11},
	{ZoombiniText::kNotiBoxMoreAction, 0x8D4EB, 11},
	{ZoombiniText::kNotiBoxHideCursor, 0x8D4F7, 11},
	{ZoombiniText::kNotiBoxShowCursor, 0x8D503, 11},
	{ZoombiniText::kNotiBoxStickeyMouse, 0x8D50F, 13},
	{ZoombiniText::kNotiBoxNonStickeyMouse, 0x8D51D, 11},
	{ZoombiniText::kNotiBoxTransitionsOn, 0x8D529, 11},
	{ZoombiniText::kNotiBoxTransitionsOff, 0x8D535, 11},
	{ZoombiniText::kNotiBoxAutoStickeyOn, 0x8D541, 16},
	{ZoombiniText::kNotiBoxAutoStickeyOff, 0x8D552, 16},
};

const ZoombiniText::ExeTextEntry ZoombiniText::kV111KR_NETextEntries[] = {
	{ZoombiniText::kPicker, 0x78400, 9},
	{ZoombiniText::kBridge, 0x7840A, 13},
	{ZoombiniText::kCaves, 0x78491, 9},
	{ZoombiniText::kPizza, 0x78429, 9},
	{ZoombiniText::kBasecamp1, 0x78433, 13},
	{ZoombiniText::kFerry, 0x78441, 9},
	{ZoombiniText::kLilly, 0x7844B, 11},
	{ZoombiniText::kSlides, 0x78457, 9},
	{ZoombiniText::kFleens, 0x78461, 11},
	{ZoombiniText::kHotel, 0x7846D, 9},
	{ZoombiniText::kNet, 0x78477, 11},
	{ZoombiniText::kBasecamp2, 0x78483, 13},
	{ZoombiniText::kTunnels, 0x78418, 16},
	{ZoombiniText::kSmoke, 0x7849B, 9},
	{ZoombiniText::kMaze, 0x784A5, 11},
	{ZoombiniText::kTown, 0x784B1, 11},
	{ZoombiniText::kNewGame, 0x793C2, 7},
	{ZoombiniText::kPracticeMode, 0x78611, 11},
	{ZoombiniText::kContinueJourney, 0x7861D, 14},
	{ZoombiniText::kPracticeTitle, 0x784EF, 9},
	{ZoombiniText::kPracticeDesc1, 0x784F9, 15},
	{ZoombiniText::kPracticeDesc2, 0x78509, 16},
	{ZoombiniText::kPracticeDesc3, 0x7851A, 18},
	{ZoombiniText::kPracticeDesc4, 0x7852D, 17},
	{ZoombiniText::kTerrainKey, 0x7862C, 11},
	{ZoombiniText::kChooseLevel, 0x78638, 11},
	{ZoombiniText::kLevel1, 0x78644, 5},
	{ZoombiniText::kLevel2, 0x7864A, 5},
	{ZoombiniText::kLevel3, 0x78650, 5},
	{ZoombiniText::kLevel4, 0x78656, 5},
	{ZoombiniText::kRoute1, 0x7865C, 11},
	{ZoombiniText::kRoute2, 0x78668, 11},
	{ZoombiniText::kRoute3, 0x78674, 16},
	{ZoombiniText::kRoute4, 0x78685, 11},
	{ZoombiniText::kXferVillePopulation, 0x78691, 17},
	{ZoombiniText::kMemorialJanuary, 0x786A3, 3},
	{ZoombiniText::kMemorialFebruary, 0x786A7, 3},
	{ZoombiniText::kMemorialMarch, 0x786AB, 3},
	{ZoombiniText::kMemorialApril, 0x786AF, 3},
	{ZoombiniText::kMemorialMay, 0x786B3, 3},
	{ZoombiniText::kMemorialJune, 0x786B7, 3},
	{ZoombiniText::kMemorialJuly, 0x786BB, 3},
	{ZoombiniText::kMemorialAugust, 0x786BF, 3},
	{ZoombiniText::kMemorialSeptember, 0x786C3, 3},
	{ZoombiniText::kMemorialOctober, 0x786C7, 4},
	{ZoombiniText::kMemorialNovember, 0x786CC, 4},
	{ZoombiniText::kMemorialDecember, 0x786D1, 4},
	{ZoombiniText::kMemorialWhenLevel, 0x786D6, 26},
	{ZoombiniText::kMemorialHonorMonument, 0x786F1, 53},
	{ZoombiniText::kMemorialHonorWindmill, 0x78727, 51},
	{ZoombiniText::kMemorialHonorObservatory, 0x7875B, 53},
	{ZoombiniText::kMemorialHonorBowlingAlley, 0x78791, 53},
	{ZoombiniText::kMemorialHonorGeneralStore, 0x787C7, 51},
	{ZoombiniText::kMemorialHonorSwimmingPool, 0x787FB, 53},
	{ZoombiniText::kMemorialHonorPlayground, 0x78831, 53},
	{ZoombiniText::kMemorialHonorBandShell, 0x78867, 53},
	{ZoombiniText::kMemorialHonorSchool, 0x7889D, 51},
	{ZoombiniText::kMemorialHonorLibrary, 0x788D1, 53},
	{ZoombiniText::kMemorialHonorFire, 0x78907, 53},
	{ZoombiniText::kMemorialHonorOpera, 0x7893D, 60},
	{ZoombiniText::kMemorialHonorCityHall, 0x7897A, 51},
	{ZoombiniText::kMemorialHonorClockTower, 0x789AE, 53},
	{ZoombiniText::kMemorialHonorMuseum, 0x789E4, 53},
	{ZoombiniText::kMemorialHonorCourt, 0x78A1A, 51},
	{ZoombiniText::kMemorialRoute1Level1, 0x78A4E, 109},
	{ZoombiniText::kMemorialRoute1Level2, 0x78ABC, 109},
	{ZoombiniText::kMemorialRoute1Level3, 0x78B2A, 124},
	{ZoombiniText::kMemorialRoute1Level4, 0x78BA7, 109},
	{ZoombiniText::kMemorialRoute2Level1, 0x78D40, 91},
	{ZoombiniText::kMemorialRoute2Level2, 0x78C70, 88},
	{ZoombiniText::kMemorialRoute2Level3, 0x78CC9, 118},
	{ZoombiniText::kMemorialRoute2Level4, 0x78D40, 91},
	{ZoombiniText::kMemorialRoute3Level1, 0x78D9C, 138},
	{ZoombiniText::kMemorialRoute3Level2, 0x78E27, 118},
	{ZoombiniText::kMemorialRoute3Level3, 0x78E9E, 100},
	{ZoombiniText::kMemorialRoute3Level4, 0x78F03, 107},
	{ZoombiniText::kMemorialRoute4Level1, 0x78F6F, 103},
	{ZoombiniText::kMemorialRoute4Level2, 0x78FD7, 118},
	{ZoombiniText::kMemorialRoute4Level3, 0x7904E, 117},
	{ZoombiniText::kMemorialRoute4Level4, 0x790C4, 116},
	{ZoombiniText::kDialogBodyGoMapWillLost, 0x79139, 52},
	{ZoombiniText::kDialogButtonLoseThem, 0x7916E, 9},
	{ZoombiniText::kDialogButtonKeepThem, 0x79178, 10},
	{ZoombiniText::kDialogButtonOkay, 0x79183, 4},
	{ZoombiniText::kDialogButtonCancel, 0x79188, 4},
	{ZoombiniText::kDialogButtonYes, 0x79340, 2},
	{ZoombiniText::kDialogButtonNo, 0x79343, 6},
	{ZoombiniText::kDialogButtonLoad, 0x7918D, 8},
	{ZoombiniText::kDialogButtonSave, 0x79196, 8},
	{ZoombiniText::kOptionsTitle, 0x7919F, 4},
	{ZoombiniText::kOptionsLegendOn, 0x791A4, 4},
	{ZoombiniText::kOptionsLegendOff, 0x791A9, 4},
	{ZoombiniText::kOptionsToggle, 0x791AE, 7},
	{ZoombiniText::kOptionsNewGame, 0x791B6, 18},
	{ZoombiniText::kOptionsLoadGame, 0x791C9, 17},
	{ZoombiniText::kOptionsSaveGame, 0x791DB, 17},
	{ZoombiniText::kOptionsQuit, 0x791ED, 15},
	{ZoombiniText::kOptionsSound, 0x791FD, 18},
	{ZoombiniText::kOptionsMusic, 0x79210, 18},
	{ZoombiniText::kOptionsStickyMouse, 0x79223, 22},
	{ZoombiniText::kOptionsTransitions, 0x7923A, 18},
	{ZoombiniText::kOptionsCredits, 0x79317, 10},
	{ZoombiniText::kDialogBodyNoSavedGames, 0x7924D, 23},
	{ZoombiniText::kDialogBodyCreateAndSaveNewGame, 0x7926A, 58},
	{ZoombiniText::kDialogButtonNewGame, 0x792A5, 11},
	{ZoombiniText::kDialogButtonReplaceTitle, 0x792B1, 8},
	{ZoombiniText::kDialogTitleSave, 0x79196, 8},
	{ZoombiniText::kDialogTitleSaveAs, 0x792C3, 18},
	{ZoombiniText::kDialogTitleLoad, 0x7918D, 8},
	{ZoombiniText::kDialogBodyReplaceGame, 0x792DF, 55},
	{ZoombiniText::kDialogBodySaveCurrentGame, 0x79322, 29},
	{ZoombiniText::kDialogBodySaveDirtyGame, 0x7947B, 62},
	{ZoombiniText::kDialogBodyCannotSaveInPractice, 0x79398, 41},
	{ZoombiniText::kDialogBodyCreateNewGame, 0x793C2, 25},
	{ZoombiniText::kDialogBodyCannotSaveMoreGame, 0x793DC, 35},
	{ZoombiniText::kDialogBodyCannotLoadInPractice, 0x79400, 48},
	{ZoombiniText::kDialogBodyCannotCreateNewInPractice, 0x79431, 44},
	{ZoombiniText::kDialogBodyNewGame, 0x793C2, 7},
	{ZoombiniText::kDialogBodyReallyQuit, 0x79466, 20},
	{ZoombiniText::kDialogBodySaveBeforeQuit, 0x7934A, 67},
	{ZoombiniText::kDialogHelpTitle, 0x794BA, 6},
	{ZoombiniText::kDialogButtonPrev, 0x794C1, 4},
	{ZoombiniText::kDialogButtonNext, 0x794C6, 4},
	{ZoombiniText::kDialogHelpLevel, 0x7850A, 4},
	{ZoombiniText::kDialogHelpPickerUpdateVersion, 0x864CE, 27},
	{ZoombiniText::kNotiBoxMusicOn, 0x79FBF, 12},
	{ZoombiniText::kNotiBoxMusicOff, 0x79FCC, 12},
	{ZoombiniText::kNotiBoxSoundOn, 0x79FD9, 12},
	{ZoombiniText::kNotiBoxSoundOff, 0x79FE6, 12},
	{ZoombiniText::kNotiBoxLessAction, 0x79FF3, 11},
	{ZoombiniText::kNotiBoxMoreAction, 0x79FFF, 11},
	{ZoombiniText::kNotiBoxHideCursor, 0x7A00B, 11},
	{ZoombiniText::kNotiBoxShowCursor, 0x7A017, 11},
	{ZoombiniText::kNotiBoxStickeyMouse, 0x7A023, 13},
	{ZoombiniText::kNotiBoxNonStickeyMouse, 0x7A031, 11},
	{ZoombiniText::kNotiBoxTransitionsOn, 0x7A03D, 11},
	{ZoombiniText::kNotiBoxTransitionsOff, 0x7A049, 11},
	{ZoombiniText::kNotiBoxAutoStickeyOn, 0x7A055, 16},
	{ZoombiniText::kNotiBoxAutoStickeyOff, 0x7A066, 16},
};

bool ZoombiniText::bytesMatchAt(const Common::Array<byte> &data, uint32 offset, const char *bytes) {
	const uint32 length = static_cast<uint32>(strlen(bytes));
	if (data.size() < static_cast<uint64>(offset) + length)
		return false;

	return memcmp(data.data() + offset, bytes, length) == 0;
}

bool ZoombiniText::readBoundedStrlString(Common::SeekableReadStream *stream, Common::String &text, uint16 remainingStringCount) {
	text.clear();
	if (!stream || remainingStringCount == 0 || stream->pos() < 0 || stream->size() < stream->pos())
		return false;

	const int64 remainingByteCount = stream->size() - stream->pos();
	if (remainingByteCount < remainingStringCount)
		return false;
	const int64 currentStringByteCount = remainingByteCount - (remainingStringCount - 1);
	for (int64 byteCount = 0; byteCount < currentStringByteCount; byteCount++) {
		if (!ZmbResource::hasBytes(stream, 1))
			return false;
		const byte value = stream->readByte();
		if (value == 0)
			return true;
		text += static_cast<char>(value);
	}

	return false;
}

template<typename T>
bool ZoombiniText::hasSingleExtendedByte(const T *bytes, uint32 length, byte expectedByte) {
	// Match an isolated high byte; this is not general MBCS detection.
	bool foundExtendedByte = false;
	for (uint32 byteIdx = 0; byteIdx < length; byteIdx++) {
		const byte value = static_cast<byte>(bytes[byteIdx]);
		if (value < 0x80)
			continue;
		if (foundExtendedByte || value != expectedByte)
			return false;
		foundExtendedByte = true;
	}
	return foundExtendedByte;
}

Common::CodePage ZoombiniText::selectExeCodePage(const byte *bytes, uint32 length, Common::CodePage codePage) {
	// Korean EXE credits retain CP1252 0xD8 (Ø) in "BrØderbund".
	if (codePage == Common::kWindows949 && hasSingleExtendedByte(bytes, length, 0xD8))
		return Common::kWindows1252;
	return codePage;
}

Common::CodePage ZoombiniText::selectCreditCodePage(const Common::String &text, Common::CodePage codePage) {
	// Japanese credits retain MacRoman 0x9F (ü) in the "Jürgen Kiel".
	if (codePage == Common::kWindows932 && hasSingleExtendedByte(text.c_str(), text.size(), 0x9F))
		return Common::kMacRoman;
	return codePage;
}

bool ZoombiniText::readStrlBody(Common::SeekableReadStream *stream, uint16 stringCount, Common::CodePage codePage, Common::Array<Common::U32String> &texts, bool decodeMacRomanUmlautByte9F) {
	texts.clear();
	texts.reserve(stringCount);
	for (uint16 strIdx = 0; strIdx < stringCount; strIdx++) {
		Common::String str;
		if (!readBoundedStrlString(stream, str, stringCount - strIdx))
			return false;
		Common::CodePage stringCodePage = codePage;
		if (decodeMacRomanUmlautByte9F)
			stringCodePage = selectCreditCodePage(str, codePage);
		texts.push_back(str.decode(stringCodePage));
	}
	return true;
}

bool ZoombiniText::readTextStrStrings(MohawkArchive &archive, int16 resourceId, Common::CodePage codePage, Common::Array<Common::U32String> &texts, bool decodeMacRomanUmlautByte9F) {
	texts.clear();
	if (!archive.hasResource(ID_STRL, resourceId))
		return false;

	Common::SeekableReadStream *resStream = archive.getResource(ID_STRL, resourceId);
	if (!resStream)
		return false;
	if (!ZmbResource::hasBytes(resStream, 1)) {
		delete resStream;
		return false;
	}

	const byte stringCount = resStream->readByte();
	Common::Array<Common::U32String> parsedTexts;
	if (!readStrlBody(resStream, stringCount, codePage, parsedTexts, decodeMacRomanUmlautByte9F)) {
		delete resStream;
		return false;
	}

	bool consumedExactly = resStream->pos() == resStream->size();
	// The v1.0BR/v1.1BR TEXTSTR.MHK credit STRL 2022 has an incorrect string count in its header.
	// It declares 142 strings but contains 145, including two trailing empty strings and the final '*' marker.
	// Accept only this known suffix so the malformed original resource remains readable.
	const bool allowBritishCreditTerminator = _relInfo.isFamily(ZmbReleaseInfo::Family::kEuropeV1x) &&
											  _relInfo.getLanguage() == Common::EN_GRB && resourceId == 2022 && stringCount == 142;
	if (!consumedExactly && allowBritishCreditTerminator && ZmbResource::hasBytes(resStream, 4)) {
		const bool hasCreditTerminatorSuffix = resStream->readByte() == 0 && resStream->readByte() == 0 && resStream->readByte() == '*' &&
											   resStream->readByte() == 0 && resStream->pos() == resStream->size();
		consumedExactly = hasCreditTerminatorSuffix;
	}
	delete resStream;
	if (!consumedExactly || parsedTexts.size() != stringCount)
		return false;
	texts = parsedTexts;
	return true;
}

bool ZoombiniText::readExecutableData(Common::SeekableReadStream *exeStream, uint32 expectedSize, Common::Array<byte> &data) {
	if (!exeStream || expectedSize == 0 || exeStream->size() != expectedSize)
		return false;

	data.resize(expectedSize);
	return exeStream->seek(0) && exeStream->read(data.data(), expectedSize) == expectedSize && exeStream->pos() == exeStream->size();
}

bool ZoombiniText::readMacPpcPefDataArgument(const Common::Array<byte> &data, uint32 sourceEnd, uint32 &sourceOffset, uint32 &value) {
	// PEF argument: one to five bytes, each [continuation bit | seven payload bits], most significant group first.
	// Pattern instructions use this for extended counts, repeat counts and custom-block sizes.
	// The caller validates sourceEnd as the packed section's exclusive bound; never consume its next section.
	static constexpr uint kMaxArgumentBytes = 5;
	static constexpr uint kPayloadBitCount = 7;
	static constexpr byte kPayloadMask = 0x7F;
	static constexpr byte kContinuationBit = 1 << kPayloadBitCount;

	value = 0;
	for (uint bIdx = 0; bIdx < kMaxArgumentBytes; bIdx++) {
		if (sourceEnd <= sourceOffset)
			return false;
		const byte encodedByte = data[sourceOffset];
		sourceOffset += 1;
		const uint32 payloadBits = encodedByte & kPayloadMask;
		// Unsigned 32-bit accumulation retains the low 32 bits when all five groups are present.
		value = (value << kPayloadBitCount) | payloadBits;
		// A clear high bit ends this argument; other high payload bits do not imply continuation.
		if ((encodedByte & kContinuationBit) == 0)
			return true;
	}
	// A fifth continuation byte cannot form a valid 32-bit PEF argument.
	return false;
}

bool ZoombiniText::expandMacPpcPefDataSection(const Common::Array<byte> &data, uint16 sectionIdx, uint32 expectedSize, Common::Array<byte> &expandedData) {
	// Reproduce the data-section preparation performed by classic Mac OS's Code Fragment Manager.
	// A packed section stores pattern instructions, not directly readable initialized globals.
	// Build its memory image here so static strings and credit offsets can be read without executing PPC code.

	// PPC PEF data-fork layout (header fields are BE):
	//   [Container header: 40 bytes]
	//   [Section headers: 28 bytes * sectionCount]
	//     Fields used here, at offsets within each section header:
	//       +8  [totalSize: BE32]
	//       +12 [unpackedSize: BE32]
	//       +16 [packedSize: BE32]
	//       +20 [containerOffset: BE32]
	//       +24 [sectionKind: byte]
	//   [Section name table: variable length]
	//   [Section contents, with alignment gaps]
	//     kind 1: unpacked data
	//     kind 2: pattern-initialized data
	//       [opcode: 3 bits | count: 5 bits][arguments][literal bytes]
	//       count == 0: read the count from the first variable-length argument
	//       interleaves: additional arguments are customSize and repeatCount
	//       repeatedBlock: additional repeatCount stores the number of copies minus one
	// Each header's containerOffset locates its contents; content order may differ from header order.
	//
	// Expanded selected section (offsets start at 0):
	//   [Initialized data: unpackedSize bytes][Zero-filled BSS: totalSize - unpackedSize bytes]
	// Original BE32 credit pointers are relative to this buffer; static text lookup needs neither imports nor CFM address relocation.
	//
	// Apple, Mac OS Runtime Architectures, chapter 8, pp. 8-4..8-14:
	// https://developer.apple.com/library/archive/documentation/mac/pdf/MacOS_RT_Architectures.pdf
	static constexpr uint32 kPefHeaderSize = 40;
	static constexpr uint32 kSectionHeaderSize = 28;
	if (data.size() < kPefHeaderSize)
		return false;

	// [*] Parse PEF Header
	const uint32 magicTag = READ_BE_UINT32(data.data());
	const uint32 formatTag = READ_BE_UINT32(data.data() + 4);
	const uint32 archTag = READ_BE_UINT32(data.data() + 8);
	const uint32 formatVer = READ_BE_UINT32(data.data() + 12);
	if (magicTag != MKTAG('J', 'o', 'y', '!') || formatTag != MKTAG('p', 'e', 'f', 'f') || archTag != MKTAG('p', 'w', 'p', 'c') || formatVer != 1)
		return false;
	// In PEF spec, instantiated section headers precede noninstantiated headers in the section header array.
	const uint16 sectionCount = READ_BE_UINT16(data.data() + 32);
	const uint16 instedCount = READ_BE_UINT16(data.data() + 34);
	if (sectionCount < instedCount || instedCount <= sectionIdx)
		return false;
	// Dividing the remaining bytes by 28 counts complete section headers; require the full declared table before indexing the selected section.
	const uint32 maxSectionHeaderCount = (data.size() - kPefHeaderSize) / kSectionHeaderSize;
	if (maxSectionHeaderCount < sectionCount)
		return false;

	// [*] Parse Section Header
	const byte *header = data.data() + kPefHeaderSize + sectionIdx * kSectionHeaderSize;
	const uint32 totalSize = READ_BE_UINT32(header + 8);
	const uint32 unpackedSize = READ_BE_UINT32(header + 12);
	const uint32 packedSize = READ_BE_UINT32(header + 16);
	const uint32 sectionOffset = READ_BE_UINT32(header + 20);
	const MacPpcPefSectionKind sectionKind = static_cast<MacPpcPefSectionKind>(header[24]);

	// Match the release's expected memory size before allocating; packed bytes and initialized bytes have separate bounds.
	if (!expectedSize || totalSize != expectedSize || totalSize < unpackedSize || data.size() < sectionOffset || data.size() - sectionOffset < packedSize)
		return false;

	// [*] Expand Section Data
	// totalSize includes the zero-initialized tail after unpackedSize. Zero opcodes can also reuse this cleared storage.
	Common::Array<byte> expandedBytes;
	switch (sectionKind) {
	case kPefSectionUnpackedData_01:
		// [*] Copy Unpacked Data
		// Unpacked data already contains its initialized memory bytes; leave the remaining tail zeroed.
		if (packedSize != unpackedSize)
			return false;
		expandedBytes.resize(totalSize);
		memset(expandedBytes.data(), 0, totalSize);
		memcpy(expandedBytes.data(), data.data() + sectionOffset, unpackedSize);
		break;
	case kPefSectionPatternInitData_02: {
		// [*] Decode Pattern-Initialized Data
		expandedBytes.resize(totalSize);
		memset(expandedBytes.data(), 0, totalSize);
		// Read commands/payload from file offsets while writing to section-relative destination offsets.
		uint32 srcOffset = sectionOffset;
		const uint32 srcEnd = sectionOffset + packedSize;
		uint32 destOffset = 0;
		while (srcOffset < srcEnd) {
			const byte cmdByte = data[srcOffset];
			srcOffset += 1;
			// Split the command byte: opcode = high 3 bits (7..5), inline count = low 5 bits (4..0).
			const MacPpcPefDataOpcode opcode = static_cast<MacPpcPefDataOpcode>(cmdByte >> 5);
			uint32 count = cmdByte & 31;
			// A zero inline count escapes to the variable-length argument representation.
			if (!count && !readMacPpcPefDataArgument(data, srcEnd, srcOffset, count))
				return false;
			uint32 customSize = 0;
			uint32 repeatCount = 0;
			switch (opcode) {
			case kPefOpcodeZero_00:
			case kPefOpcodeBlockCopy_01:
				// Both advance count output bytes; only blockCopy consumes literal input bytes.
				if (unpackedSize - destOffset < count)
					return false;
				if (opcode == kPefOpcodeBlockCopy_01) {
					if (srcEnd - srcOffset < count)
						return false;
					memcpy(expandedBytes.data() + destOffset, data.data() + srcOffset, count);
					srcOffset += count;
				}
				destOffset += count;
				break;
			case kPefOpcodeRepeatedBlock_02: {
				// The stored repeat count is one less than the number of copies; consume the literal block only once.
				if (!readMacPpcPefDataArgument(data, srcEnd, srcOffset, repeatCount) || srcEnd - srcOffset < count)
					return false;
				if (count) {
					// Check the number of whole copies that fit before adding the omitted first copy.
					const uint32 maxCopies = (unpackedSize - destOffset) / count;
					if (maxCopies <= repeatCount)
						return false;
					const uint32 repetitions = repeatCount + 1;
					for (uint32 index = 0; index < repetitions; index++) {
						memcpy(expandedBytes.data() + destOffset, data.data() + srcOffset, count);
						destOffset += count;
					}
				}
				srcOffset += count;
				break;
			}
			case kPefOpcodeInterleaveBlocks_03:
			case kPefOpcodeInterleaveZero_04: {
				// Packed input holds one common block (or implicit zeros), then all custom blocks.
				// Output alternates common/custom blocks and ends with one final common block.
				if (!readMacPpcPefDataArgument(data, srcEnd, srcOffset, customSize) ||
					!readMacPpcPefDataArgument(data, srcEnd, srcOffset, repeatCount))
					return false;
				// Reserve the final common block, then bound both repeated portions before multiplying.
				uint32 remainingOutput = unpackedSize - destOffset;
				if (remainingOutput < count)
					return false;
				remainingOutput -= count;
				uint32 customBytes = 0;
				if (repeatCount) {
					const uint32 maxCommonBlockSize = remainingOutput / repeatCount;
					if (maxCommonBlockSize < count)
						return false;
					remainingOutput -= count * repeatCount;
					const uint32 maxCustomBlockSize = remainingOutput / repeatCount;
					if (maxCustomBlockSize < customSize)
						return false;
					customBytes = customSize * repeatCount;
				}
				const uint32 commonOffset = srcOffset;
				uint32 commonPayloadSize = 0;
				if (opcode == kPefOpcodeInterleaveBlocks_03)
					commonPayloadSize = count;
				if (srcEnd - srcOffset < commonPayloadSize || srcEnd - srcOffset - commonPayloadSize < customBytes)
					return false;
				srcOffset += commonPayloadSize;
				// Empty patterns consume no payload and must not loop repeatCount times.
				if (count || customSize) {
					for (uint32 i = 0; i < repeatCount; i++) {
						if (commonPayloadSize)
							memcpy(expandedBytes.data() + destOffset, data.data() + commonOffset, count);
						destOffset += count;
						memcpy(expandedBytes.data() + destOffset, data.data() + srcOffset, customSize);
						destOffset += customSize;
						srcOffset += customSize;
					}
				}
				// Emit the last common block separately, without forming repeatCount + 1.
				if (commonPayloadSize)
					memcpy(expandedBytes.data() + destOffset, data.data() + commonOffset, count);
				destOffset += count;
				break;
			}
			default: // Opcodes 5..7 are reserved.
				return false;
			}
		}
		// [*] Validate Expanded Size
		// Patterns must produce exactly the initialized region; the remaining totalSize bytes stay zero.
		if (destOffset != unpackedSize)
			return false;
		break;
	}
	default:
		// Handle only unpacked and pattern-initted data sections.
		return false;
	}
	// [*] Publish Expanded Section
	// Transfer the validated image without copying; failures leave the caller's output unchanged.
	expandedData.swap(expandedBytes);
	return true;
}

bool ZoombiniText::readMac68kA5WorldSizes(Common::SeekableReadStream *codeStream, uint32 &bytesAboveA5, uint32 &bytesBelowA5) {
	if (!codeStream || codeStream->size() < 8 || !codeStream->seek(0))
		return false;

	bytesAboveA5 = codeStream->readUint32BE();
	bytesBelowA5 = codeStream->readUint32BE();
	return !codeStream->err();
}

bool ZoombiniText::expandMac68kResForkRleGlobals(Common::Array<byte> &data, uint32 bytesAboveA5, uint32 bytesBelowA5) {
	const uint64 a5WorldSize = static_cast<uint64>(bytesBelowA5) + bytesAboveA5;
	if (bytesAboveA5 == 0 || UINT32_MAX < a5WorldSize)
		return false;

	// Commands 1..4 expand the original eight-byte pointer/jump-table templates.
	// -1 preserves the destination byte, -2 consumes one literal byte, and other entries emit a fixed byte.
	static constexpr int16 kTemplates[4][8] = {
		{-1, -1, -1, -1, 255, 255, -2, -2},
		{-1, -1, -1, -1, 255, -2, -2, -2},
		{169, 240, -1, -1, -2, -2, -1, -2},
		{169, 240, -1, -2, -2, -2, -1, -2},
	};
	Common::Array<byte> a5World;
	a5World.resize(static_cast<uint32>(a5WorldSize));
	// Skipped destinations retain zeros or bytes written by an earlier block.
	memset(a5World.data(), 0, a5World.size());
	// Decode all three blocks, including empty ones.
	uint32 srcOffset = 4;
	for (uint block = 0; block < 3; block++) {
		if (data.size() < srcOffset + 4)
			return false;
		// Each block starts with a signed BE32 displacement from A5, which lies bytesBelowA5 bytes into a5World.
		const int64 blockOffset = static_cast<int64>(bytesBelowA5) + READ_BE_INT32(data.data() + srcOffset);
		srcOffset += 4;
		if (blockOffset < 0 || static_cast<int64>(a5World.size()) < blockOffset)
			return false;
		uint32 dest = static_cast<uint32>(blockOffset);
		while (true) {
			if (data.size() <= srcOffset)
				return false;
			const byte cmd = data[srcOffset];
			srcOffset += 1;
			// Zero ends this block, not the entire three-block stream.
			if (cmd == kMac68kDataEndBlock_00)
				break;
			if (cmd <= kMac68kDataTemplateLast_04) {
				if (a5World.size() - dest < 8)
					return false;
				for (uint i = 0; i < 8; i++) {
					const int16 value = kTemplates[cmd - kMac68kDataTemplateFirst_01][i];
					if (value == -2) {
						if (data.size() <= srcOffset)
							return false;
						a5World[dest + i] = data[srcOffset];
						srcOffset += 1;
					} else if (0 <= value) {
						a5World[dest + i] = static_cast<byte>(value);
					}
				}
				dest += 8;
				continue;
			}
			// Run commands encode their length as an offset from the command range's base.
			// Check bases in descending order so each byte selects exactly one command.
			uint32 count;
			byte value = 255;
			if (kMac68kDataCopyLiteralsBase_128 <= cmd) {
				count = cmd - kMac68kDataCopyLiteralsBase_128 + 1;
			} else if (kMac68kDataSkipBase_64 <= cmd) {
				count = cmd - kMac68kDataSkipBase_64 + 1;
			} else if (kMac68kDataRepeatByteBase_32 <= cmd) {
				count = cmd - kMac68kDataRepeatByteBase_32 + 2;
				if (data.size() <= srcOffset)
					return false;
				value = data[srcOffset];
				srcOffset += 1;
			} else if (kMac68kDataRepeatFFBase_16 <= cmd) {
				count = cmd - kMac68kDataRepeatFFBase_16 + 1;
			} else {
				// Commands 5..15 are invalid; the original reports SysError 15.
				return false;
			}
			if (a5World.size() - dest < count)
				return false;
			if (kMac68kDataCopyLiteralsBase_128 <= cmd) {
				if (data.size() - srcOffset < count)
					return false;
				memcpy(a5World.data() + dest, data.data() + srcOffset, count);
				srcOffset += count;
			} else if (cmd < kMac68kDataSkipBase_64) {
				memset(a5World.data() + dest, value, count);
			}
			dest += count;
		}
	}
	// Relocation streams follow these blocks. Static text uses the unrelocated A5-relative offsets.
	// Drop the below-A5 region so string offsets and credit pointers index data directly from A5 at index 0.
	data.resize(bytesAboveA5);
	memcpy(data.data(), a5World.data() + bytesBelowA5, bytesAboveA5);
	return true;
}

bool ZoombiniText::findBytes(const Common::Array<byte> &data, const char *needle, uint32 &offset) {
	const uint32 needleLength = static_cast<uint32>(strlen(needle));
	if (needleLength < 1 || data.size() < needleLength)
		return false;

	for (uint32 byteIndex = 0; byteIndex <= data.size() - needleLength; byteIndex++) {
		if (memcmp(data.data() + byteIndex, needle, needleLength) == 0) {
			offset = byteIndex;
			return true;
		}
	}
	return false;
}

Common::U32String ZoombiniText::decodeExecutableStringBytes(const byte *bytes, uint32 length, const ZoombiniText::ExeTextSource &source) {
	Common::CodePage decodeCodePage = source.codePage;
	if (source.decodeWindows1252StrokeByteD8)
		decodeCodePage = selectExeCodePage(bytes, length, source.codePage);

	// The table stores a fixed byte span, not a NUL-terminated C string.
	// Copy it to a temporary buffer and add a terminator only when the span does
	// not already end with one before using the null-terminated decoder.
	// This keeps padding and terminators out of the resulting text without
	// triggering Common::String's embedded-NUL warning.
	char *terminatedBytes = new char[length + 1];
	memcpy(terminatedBytes, bytes, length);
	if (length == 0 || terminatedBytes[length - 1] != '\0')
		terminatedBytes[length] = '\0';
	Common::U32String text(terminatedBytes, decodeCodePage);
	delete[] terminatedBytes;
	if (source.escapeMacRomanTrademarkByteAA) {
		// TLC v2.0 US, Polish and US demo strings retain MacRoman 0xAA (™).
		// Decode other bytes with the source's Windows-1252 or Windows-1250 code page.
		for (uint32 byteIdx = 0; byteIdx < length && byteIdx < text.size(); byteIdx++) {
			if (bytes[byteIdx] == 0xAA)
				text.setChar(0x2122, byteIdx);
		}
	}

	return text;
}

bool ZoombiniText::readExecutableStringAt(const Common::Array<byte> &data, uint32 offset, const ZoombiniText::ExeTextSource &source,
										  Common::U32String &text, uint32 &byteLength) {
	if (data.size() <= offset || source.creditLimits.lineByteCount == 0)
		return false;

	uint32 endOffset = offset;
	while (endOffset < data.size() && endOffset - offset < source.creditLimits.lineByteCount && data[endOffset] != 0)
		endOffset += 1;
	if (data.size() <= endOffset || data[endOffset] != 0)
		return false;

	byteLength = endOffset - offset;
	text = decodeExecutableStringBytes(data.data() + offset, byteLength, source);
	return true;
}

bool ZoombiniText::areExecutableCreditLimitsValid(const ZoombiniText::ExeTextSource &source) {
	return source.creditLimits.entryCount != 0 && source.creditLimits.lineByteCount != 0 && source.creditLimits.dataByteCount != 0;
}

bool ZoombiniText::isCreditTerminator(const Common::U32String &text) {
	return text.size() == 1 && text[0] == U'*';
}

bool ZoombiniText::isTlcCreditIconMarker(const Common::U32String &text) {
	if (text.size() != 5)
		return false;

	// Support both uppercase 'ICON?' and lowercase 'icon?'
	// As Polish release mixed up both: 'ICON' in data, 'icon' in renderer.
	return (text[0] == U'i' || text[0] == U'I') &&
		   (text[1] == U'c' || text[1] == U'C') &&
		   (text[2] == U'o' || text[2] == U'O') &&
		   (text[3] == U'n' || text[3] == U'N') &&
		   U'1' <= text[4] && text[4] <= U'4';
}

bool ZoombiniText::readCreditStringsFromAnchor(const Common::Array<byte> &data, const ZoombiniText::ExeTextSource &source, Common::Array<Common::U32String> &creditStrings) {
	uint32 offset = 0;
	if (!areExecutableCreditLimitsValid(source) || !source.creditAnchor || !findBytes(data, source.creditAnchor, offset))
		return false;

	creditStrings.clear();
	uint32 totalByteCount = 0;
	for (uint32 entryCount = 0; entryCount < source.creditLimits.entryCount; entryCount++) {
		Common::U32String text;
		uint32 byteLength = 0;
		if (!readExecutableStringAt(data, offset, source, text, byteLength))
			return false;
		if (source.creditLimits.dataByteCount < byteLength || source.creditLimits.dataByteCount - byteLength < totalByteCount)
			return false;
		totalByteCount += byteLength;

		creditStrings.push_back(text);
		if (isCreditTerminator(text))
			return true;

		offset += byteLength + 1;
	}

	return false;
}

bool ZoombiniText::readCreditStringsFromPointerTable(const Common::Array<byte> &data, const ZoombiniText::ExeTextSource &source, Common::Array<Common::U32String> &creditStrings) {
	if (!areExecutableCreditLimitsValid(source))
		return false;

	creditStrings.clear();
	bool reachedIcons = false;
	uint32 nextIconIdx = 1;
	uint32 pointerIndex = source.creditPointerFirstIndex;
	uint32 totalByteCount = 0;
	for (uint32 entryCount = 0; entryCount < source.creditLimits.entryCount; entryCount++) {
		const uint32 tableEntryOffset = source.creditPointerTableOffset + pointerIndex * 4;
		if (data.size() < tableEntryOffset + 4)
			return false;

		uint32 stringAddress;
		const bool srcMac68kA5RelativePtrs = source.creditSourceKind == CreditSourceKind::kMac68kA5RelativePointerStream;
		switch (source.creditSourceKind) {
		case CreditSourceKind::kMac68kA5RelativePointerStream:
		case CreditSourceKind::kMacPpcPefRelativePointerStream:
			stringAddress = READ_BE_UINT32(data.data() + tableEntryOffset);
			break;
		default:
			stringAddress = READ_LE_UINT32(data.data() + tableEntryOffset);
			break;
		}
		if (source.creditPointerBlankAddressFirst &&
			source.creditPointerBlankAddressFirst <= stringAddress && stringAddress <= source.creditPointerBlankAddressLast) {
			if (!reachedIcons)
				creditStrings.push_back(Common::U32String());
			pointerIndex += 1;
			continue;
		}
		if (stringAddress < source.creditPointerBaseAddress)
			return false;

		Common::U32String text;
		uint32 byteLength = 0;
		if (!readExecutableStringAt(data, stringAddress - source.creditPointerBaseAddress, source, text, byteLength))
			return false;
		if (source.creditLimits.dataByteCount < byteLength || source.creditLimits.dataByteCount - byteLength < totalByteCount)
			return false;
		totalByteCount += byteLength;
		if (isCreditTerminator(text)) {
			if ((!srcMac68kA5RelativePtrs && nextIconIdx != 5) || creditStrings.empty())
				return false;
			creditStrings.push_back(text);
			return true;
		}
		if (!srcMac68kA5RelativePtrs && isTlcCreditIconMarker(text)) {
			const uint32 iconIndex = static_cast<uint32>(text[4] - U'0');
			if (iconIndex != nextIconIdx)
				return false;
			if (!reachedIcons) {
				while (!creditStrings.empty() && creditStrings.back().empty())
					creditStrings.pop_back();
				reachedIcons = true;
			}
			nextIconIdx += 1;
			pointerIndex += 1;
			continue;
		}
		if (reachedIcons)
			return false;

		creditStrings.push_back(text);
		pointerIndex += 1;
	}

	return false;
}

bool ZoombiniText::buildCreditParagraphsFromStrings(const Common::Array<Common::U32String> &creditStrings, Common::Array<CreditParagraph> &creditParagraphs, CreditSourceKind kind, uint32 entryCountLimit) {
	creditParagraphs.clear();
	if (kind == CreditSourceKind::kNone || entryCountLimit == 0 || entryCountLimit < creditStrings.size())
		return false;

	Common::Array<Common::U32String> lines;
	uint32 blankLineCount = 0;
	bool firstLineIsTitle = true;
	bool reachedTerminator = false;
	bool leadingSpaceMarksBody;
	switch (kind) {
	case CreditSourceKind::kExecutablePointerStream:
	case CreditSourceKind::kMacPpcPefRelativePointerStream:
		leadingSpaceMarksBody = true;
		break;
	default:
		leadingSpaceMarksBody = false;
		break;
	}

	for (const Common::U32String &text : creditStrings) {
		if (isCreditTerminator(text)) {
			reachedTerminator = true;
			break;
		}

		if (text.empty()) {
			if (!lines.empty())
				blankLineCount += 1;
			continue;
		}

		if (!lines.empty() && blankLineCount != 0) {
			creditParagraphs.push_back(CreditParagraph(lines, blankLineCount, firstLineIsTitle));
			lines.clear();
			blankLineCount = 0;
			firstLineIsTitle = true;
		}

		if (lines.empty() && leadingSpaceMarksBody && text[0] == ' ')
			firstLineIsTitle = false;
		lines.push_back(text);
	}

	if (!lines.empty())
		creditParagraphs.push_back(CreditParagraph(lines, blankLineCount, firstLineIsTitle));

	return reachedTerminator && !creditParagraphs.empty();
}

bool ZoombiniText::loadOriginalExecutableCredits(const Common::Array<byte> &data, const ZoombiniText::ExeTextSource &source, Common::Array<CreditParagraph> &creditParagraphs) {
	Common::Array<Common::U32String> creditStrings;
	switch (source.creditSourceKind) {
	case CreditSourceKind::kExecutableAnchorSequence:
		if (!readCreditStringsFromAnchor(data, source, creditStrings))
			return false;
		break;
	case CreditSourceKind::kExecutablePointerStream:
	case CreditSourceKind::kMac68kA5RelativePointerStream:
	case CreditSourceKind::kMacPpcPefRelativePointerStream:
		if (!readCreditStringsFromPointerTable(data, source, creditStrings))
			return false;
		break;
	default:
		return false;
	}

	if (!buildCreditParagraphsFromStrings(creditStrings, creditParagraphs, source.creditSourceKind, source.creditLimits.entryCount))
		return false;

	return true;
}

bool ZoombiniText::compareLocalizedStrings(const LocalizedString &left, const LocalizedString &right) {
	return left._key < right._key;
}

bool ZoombiniText::initArchiveStrings(const SourceSettings &sources) {
	const Common::Path textStrMhkPath = Common::Path(_vm->getArchiveRoot()).append(ZMB_MHK_TEXTSTR);
	MohawkArchive textStrArchive;
	if (!textStrArchive.openFile(textStrMhkPath))
		return false;

	Common::HashMap<int16, Common::Array<Common::U32String>> resStringCache;
	Common::Array<Common::U32String> archiveStrings;
	archiveStrings.reserve(ARRAYSIZE(kV10EU_ResTextEntries));
	for (const ResTextEntry &entry : kV10EU_ResTextEntries) {
		if (!resStringCache.contains(entry.resourceId)) {
			Common::Array<Common::U32String> texts;
			if (!readTextStrStrings(textStrArchive, entry.resourceId, _resCodePage, texts)) {
				warning("ZoombiniText: failed to load Europe TEXTSTR.MHK STRL %d string %u", entry.resourceId, entry.stringIndex);
				return false;
			}
			resStringCache[entry.resourceId] = texts;
		}
		const Common::Array<Common::U32String> &texts = resStringCache[entry.resourceId];
		if (texts.size() <= entry.stringIndex) {
			warning("ZoombiniText: failed to load Europe TEXTSTR.MHK STRL %d string %u", entry.resourceId, entry.stringIndex);
			return false;
		}
		archiveStrings.push_back(texts[entry.stringIndex]);
	}

	Common::Array<CreditParagraph> archiveCredits;
	if (sources.textStrCreditEntryCount) {
		static constexpr int16 creditResourceIds[] = {2020, 2021};
		Common::Array<Common::U32String> creditStrings;
		for (uint resourceIndex = 0; resourceIndex < ARRAYSIZE(creditResourceIds); resourceIndex += 1) {
			Common::Array<Common::U32String> resourceStrings;
			if (!readTextStrStrings(textStrArchive, creditResourceIds[resourceIndex], _resCodePage, resourceStrings, sources.decodeMacRomanUmlautByte9F)) {
				warning("ZoombiniText: failed to load TEXTSTR.MHK credit STRL %d", creditResourceIds[resourceIndex]);
				return false;
			}

			for (const Common::U32String &text : resourceStrings)
				creditStrings.push_back(text);
		}

		if (!buildCreditParagraphsFromStrings(creditStrings, archiveCredits, CreditSourceKind::kTextStrResourceSequence, sources.textStrCreditEntryCount)) {
			warning("ZoombiniText: failed to build TEXTSTR.MHK credit paragraphs");
			return false;
		}
	}

	for (uint entryIndex = 0; entryIndex < ARRAYSIZE(kV10EU_ResTextEntries); entryIndex++) {
		const ResTextEntry &entry = kV10EU_ResTextEntries[entryIndex];
		// The executable's New Game label names an unsaved game; the STRL value
		// with the same key is the startup dialog button and has its own key.
		if (entry.key != kNewGame)
			_strMap[entry.key] = archiveStrings[entryIndex];
	}
	if (sources.textStrCreditEntryCount)
		_creditParagraphs = archiveCredits;

	return true;
}

ZoombiniText::ZoombiniText(MohawkEngine_Zoombini *vm)
	: _vm(vm), _relInfo(vm->getRelInfo()), _exeCodePage(_relInfo.getExeCodePage()), _resCodePage(_relInfo.getResCodePage()) {
	const SourceSettings sources = createSourceSettings(_relInfo, _vm->getFeatures());
	initFonts(sources);
	// Load the release-specific strings embedded in the game executable.
	if (!initExecutableStrings(sources.executables)) {
		error("ZoombiniText: the original executable is missing, malformed, or does not match this release");
		return;
	}

	applyStringOverrides(_relInfo);

	// Load mapped strings after the generic fallback maps so archive text wins.
	if (sources.textStrArchive && !initArchiveStrings(sources)) {
		error("ZoombiniText: required TEXTSTR.MHK is missing or malformed");
		return;
	}
}

void ZoombiniText::initFonts(const SourceSettings &sources) {
	switch (_relInfo.getPlatform()) {
	case Common::kPlatformMacintosh:
		initMacintoshFonts();
		break;
	case Common::kPlatformWindows:
		initWindowsFonts(sources.fontCabinet);
		break;
	default:
		error("ZoombiniText::initFonts: unsupported platform %d", static_cast<int>(_relInfo.getPlatform()));
	}

	// Check if ScummVM can access required fonts, and print warning message box if they are not found.
	loadFont(_textFont, true);
	loadFont(_titleFont, false);
}

void ZoombiniText::initMacintoshFonts() {
	static constexpr Graphics::TTFSizeMode kSizeMode = Graphics::kTTFSizeModeCharacter;

	_textFont.point = 12;
	_titleFont.point = 18;
	switch (_relInfo.getLanguage()) {
	case Common::FR_FRA:
	case Common::DE_DEU:
		_titleFont.point = 16;
		break;
	case Common::JA_JPN:
		_titleFont.point = 16;
		// We need specifically Osaka 4.2 from the Mac OS 9 Japanese Language Kit, not from the KanjiTalk 7.5.3.
		// The latter does not contain Unicode cmap, which is required by FreeType.
		_optimalTTFLoaders.push_back(new MacResForkTTFLoader("Osaka", "Osaka", 16384, kSizeMode));
		_optimalTTFLoaders.push_back(new FileTTFLoader("Osaka.ttf", "Osaka", false, 0, kSizeMode));
		_fallbackTTFLoaders.push_back(new FileTTFLoader("msgothic.ttc", "MS Gothic", false, 0, kSizeMode));
#if defined(WIN32)
		_fallbackTTFLoaders.push_back(new WinSysTTFLoader("msgothic.ttc", "MS Gothic", false, 0, kSizeMode));
#endif
		// Without hinting, quality of modern CJK font rendering in low resolution is terrible.
		// Thus, prefer light hinting for modern CJK fonts when RGB rendering is available.
		_fallbackTTFLoaders.push_back(new ArchiveTTFLoader("VL-Gothic-Regular.ttf", "VL Gothic", Graphics::kTTFRenderModeLight, kSizeMode));
		_fallbackTTFLoaders.push_back(new ArchiveTTFLoader("NotoSansJP-Bold.otf", "Noto Sans JP Bold", Graphics::kTTFRenderModeLight, kSizeMode));
		return;
	case Common::EN_USA:
	case Common::EN_GRB:
	default:
		break;
	}
	_optimalTTFLoaders.push_back(new MacResForkTTFLoader(_relInfo.getMacApplicationName(), "Cornerstone", 2863, kSizeMode));
	if (_fallbackTTFLoaders.empty())
		_fallbackTTFLoaders.push_back(new ArchiveTTFLoader("LiberationMono-Bold.ttf", "Liberation Mono", false, 0, kSizeMode));
}

void ZoombiniText::initWindowsFonts(const char *fontCabinet) {
	Graphics::TTFSizeMode fontSizeMode = Graphics::kTTFSizeModeCharacter;
	if (_relInfo.isFamily(ZmbReleaseInfo::Family::kTlcV20, Common::PL_POL))
		fontSizeMode = Graphics::kTTFSizeModeCell;
	const Common::Language language = _relInfo.getLanguage();
	static constexpr const char *kFontNameCornerstone = "CornerStone";
	static constexpr const char *kFontNameGulimChe = "GulimChe";  // for v1.11KR
	static constexpr const char *kFontNameMsGothic = "MS Gothic"; // for v1.0JP
	static constexpr const char *kFontNameTahoma = "Tahoma";      // for v2.0PL
	// I18N: %s is a required file name or a list of alternative paths within the installer disk directory.
	const Common::U32String srcInstTemplate = _("Please provide an installer disk directory containing: %s.");
#if defined(WIN32)
	// FIXME: Is loading required fonts from the Windows font archive acceptable in ScummVM?
	// I18N: %s is a font file name; %S is the complete localized instruction for obtaining that font.
	const Common::U32String systemFontWarnTemplate = _("ScummVM found required '%s' in the Windows font archive.\n"
													   "This setup is not portable and not recommended.\n\n"
													   "%S");
	Common::String systemFontWarn;
#endif
	Common::String fontSrcInst;

	// Default font sizes for English Zoombinis.
	_textFont.point = 13;
	_titleFont.point = 18;

	// Zoombini bundles its preferred font in InstallShield archives or the install location.
	// - v1.0US, v1.0EU and v1.1EU: found in /CORNER.TTF
	// - 1.1 US: found in /ZBARC16.Z, /ZBARC32.Z, or /SETUP/data1.cab
	// - 2.0 TLC: found in /INSTALL/HD/CORNER.TTF
	//   * Polish release contains Tahoma in name of CORNER.TTF.
	// - 2.0 TLC demo: found in /CORNER.TTF
	if (_relInfo.isFamily(ZmbReleaseInfo::Family::kUsV10) || _relInfo.isFamily(ZmbReleaseInfo::Family::kEuropeV1x)) {
		if (_relInfo.isFamily(ZmbReleaseInfo::Family::kUsV10, Common::PT_PRT)) {
			// FIXME: CornerStone's ascender is 904/1000 em, or 9.04 pixels at size 10.
			// GDI rounds this to 9 pixels; FreeType rounds up to 10 and uses that ascent for cached glyph Y offsets.
			// Both have an 11-pixel line height, but FreeType places the Portuguese glyph baseline one pixel lower.
			// This font-metric difference remains a known issue; do not compensate with layout rectangles.
			_textFont.point = 10;
			_titleFont.point = 14;
		} else if (_relInfo.isFamily(ZmbReleaseInfo::Family::kEuropeV1x, Common::JA_JPN)) {
			_textFont.point = 12;
			_titleFont.point = 15;
		} else {
			switch (language) {
			case Common::FR_FRA:
			case Common::DE_DEU:
			case Common::IT_ITA:
				_titleFont.point = 16;
				break;
			default: // Use default
				break;
			}
		}

		if (fontCabinet) {
			const Common::String cabinetPath = Common::String::format("'/%s'", fontCabinet);
			fontSrcInst = Common::U32String::format(srcInstTemplate, cabinetPath.c_str()).encode(Common::kUtf8);
			_optimalTTFLoaders.push_back(new ISCabTTFLoader(fontCabinet, "CORNER.TTF", kFontNameCornerstone, fontSrcInst, false, 0, fontSizeMode));
			_optimalTTFLoaders.push_back(new FileTTFLoader("CORNER.TTF", kFontNameCornerstone, fontSrcInst, false, 0, fontSizeMode));
		} else if (_relInfo.isFamily(ZmbReleaseInfo::Family::kEuropeV1x, Common::JA_JPN)) { // v1.0JP
			// Japanese Zoombini depends on 'MS Gothic' font named as 'msgothic.ttc'.
			// The font is distributed with Windows and is not included on the game CD.
			// Users have to source the required font themselves!
			fontSrcInst = Common::U32String::format(srcInstTemplate, "'msgothic.ttc'").encode(Common::kUtf8);
			_optimalTTFLoaders.push_back(new FileTTFLoader("msgothic.ttc", kFontNameMsGothic, fontSrcInst, false, 0, fontSizeMode));
#if defined(WIN32)
			systemFontWarn = Common::U32String::format(systemFontWarnTemplate, "msgothic.ttc", fontSrcInst.decode(Common::kUtf8).c_str()).encode(Common::kUtf8);
			_optimalTTFLoaders.push_back(new WinSysTTFLoader("msgothic.ttc", kFontNameMsGothic, systemFontWarn, false, 0, fontSizeMode));
#endif
			// Without hinting, quality of modern CJK font rendering in low resolution is terrible.
			// Thus, prefer light hinting for modern CJK fonts when RGB rendering is available.
			_fallbackTTFLoaders.push_back(new ArchiveTTFLoader("VL-Gothic-Regular.ttf", "VL Gothic", Graphics::kTTFRenderModeLight, fontSizeMode));
			_fallbackTTFLoaders.push_back(new ArchiveTTFLoader("NotoSansJP-Bold.otf", "Noto Sans JP Bold", Graphics::kTTFRenderModeLight, fontSizeMode));
		} else {
			fontSrcInst = Common::U32String::format(srcInstTemplate, "'/CORNER.TTF'").encode(Common::kUtf8);
			_optimalTTFLoaders.push_back(new FileTTFLoader("CORNER.TTF", kFontNameCornerstone, fontSrcInst, false, 0, fontSizeMode));
		}
	} else if (_relInfo.isFamily(ZmbReleaseInfo::Family::kUsV11)) { // Broderbund v1.1 US release
		if (language == Common::EN_USA) {
			fontSrcInst = Common::U32String::format(srcInstTemplate, "'/ZBARC32.Z', '/ZBARC16.Z', or '/SETUP/data1.cab'").encode(Common::kUtf8);
			_optimalTTFLoaders.push_back(new ISZTTFLoader("ZBARC32.Z", "CORNER.TTF", kFontNameCornerstone, fontSrcInst, false, 0, fontSizeMode));
			_optimalTTFLoaders.push_back(new ISZTTFLoader("ZBARC16.Z", "CORNER.TTF", kFontNameCornerstone, fontSrcInst, false, 0, fontSizeMode));
			_optimalTTFLoaders.push_back(new ISCabTTFLoader("SETUP/data1.cab", "CORNER.TTF", kFontNameCornerstone, fontSrcInst, false, 0, fontSizeMode));
			_optimalTTFLoaders.push_back(new FileTTFLoader("CORNER.TTF", kFontNameCornerstone, fontSrcInst, false, 0, fontSizeMode));
		} else if (language == Common::KO_KOR) {
			// Korean Zoombini depends on GulimChe font named as 'gulim.ttc'.
			// The font is distributed with Windows and is not included on the game CD.
			// Users have to source the required font themselves!

			// FreeType loads TrueType scaler bitmaps (sbits) from gulim.ttc's EBDT/EBLC tables.
			// It uses the matching strike (the bitmap set for a given size) when available.
			fontSrcInst = Common::U32String::format(srcInstTemplate, "'gulim.ttc'").encode(Common::kUtf8);
			_optimalTTFLoaders.push_back(new FileTTFLoader("gulim.ttc", kFontNameGulimChe, fontSrcInst, true, 1, fontSizeMode));
#if defined(WIN32)
			systemFontWarn = Common::U32String::format(systemFontWarnTemplate, "gulim.ttc", fontSrcInst.decode(Common::kUtf8).c_str()).encode(Common::kUtf8);
			_optimalTTFLoaders.push_back(new WinSysTTFLoader("gulim.ttc", kFontNameGulimChe, systemFontWarn, true, 1, fontSizeMode));
#endif
			_textFont.point = 12;
			_titleFont.point = 18;
			// Without hinting, quality of Noto Sans rendering in low resolution is terrible.
			// Thus, prefer light hinting for Noto Sans when RGB rendering is available.
			_fallbackTTFLoaders.push_back(new ArchiveTTFLoader("NotoSansKR-Bold.otf", "Noto Sans KR Bold", Graphics::kTTFRenderModeLight, fontSizeMode));
			// TODO: Unifont support? The font is small while supporting every possible Unicode glyphs.
		} else {
			error("ZoombiniText: unsupported language (%s) for Broderbund v1.1 US-based release", Common::getLanguageDescription(language));
			return;
		}
	} else if (_relInfo.isFamily(ZmbReleaseInfo::Family::kTlcV20)) { // The Learning Company v2.0 release family
		if (_relInfo.isDemo(ZmbReleaseInfo::Family::kTlcV20)) {
			fontSrcInst = Common::U32String::format(srcInstTemplate, "'/CORNER.TTF'").encode(Common::kUtf8);
			_optimalTTFLoaders.push_back(new FileTTFLoader("CORNER.TTF", kFontNameCornerstone, fontSrcInst, false, 0, fontSizeMode));
		} else if (language == Common::EN_USA) {
			fontSrcInst = Common::U32String::format(srcInstTemplate, "'/INSTALL/HD/CORNER.TTF'").encode(Common::kUtf8);
			_optimalTTFLoaders.push_back(new FileTTFLoader("INSTALL/HD/CORNER.TTF", kFontNameCornerstone, fontSrcInst, false, 0, fontSizeMode));
			_optimalTTFLoaders.push_back(new FileTTFLoader("CORNER.TTF", kFontNameCornerstone, fontSrcInst, false, 0, fontSizeMode));
		} else if (language == Common::PL_POL) {
			// Polish Zoombini uses Tahoma, a font distributed with Windows.
			// Strange enough, Tahoma is embedded in CD-ROM named as 'CORNER.TTF'.
			fontSrcInst = Common::U32String::format(srcInstTemplate, "'/INSTALL/HD/CORNER.TTF'").encode(Common::kUtf8);
			_optimalTTFLoaders.push_back(new FileTTFLoader("INSTALL/HD/CORNER.TTF", kFontNameTahoma, fontSrcInst, false, 0, fontSizeMode));
			_optimalTTFLoaders.push_back(new FileTTFLoader("CORNER.TTF", kFontNameTahoma, fontSrcInst, false, 0, fontSizeMode));
			_optimalTTFLoaders.push_back(new FileTTFLoader("tahoma.ttf", kFontNameTahoma, fontSrcInst, false, 0, fontSizeMode));
#if defined(WIN32)
			systemFontWarn = Common::U32String::format(systemFontWarnTemplate, "tahoma.ttf", fontSrcInst.decode(Common::kUtf8).c_str()).encode(Common::kUtf8);
			_optimalTTFLoaders.push_back(new WinSysTTFLoader("tahoma.ttf", kFontNameTahoma, systemFontWarn, false, 0, fontSizeMode));
#endif
		} else {
			error("ZoombiniText: unsupported language (%s) for The Learning Company v2.0 based release", Common::getLanguageDescription(language));
			return;
		}
	} else {
		error("ZoombiniText: unsupported release (%s, %s)", Common::getLanguageDescription(language), _vm->_gameDescription->desc.extra);
		return;
	}
	if (_fallbackTTFLoaders.empty())
		_fallbackTTFLoaders.push_back(new ArchiveTTFLoader("LiberationMono-Bold.ttf", "Liberation Mono", false, 0, fontSizeMode));
}

ZoombiniText::~ZoombiniText() {
	// FontMan outlives game engines, so release this game's font names before
	// another Zoombini target can load fonts with the same face and size.
	FontMan.removeFontName(_textFont.cacheName);
	FontMan.removeFontName(_titleFont.cacheName);

	for (TTFLoader *loader : _optimalTTFLoaders)
		delete loader;
	_optimalTTFLoaders.clear();

	for (TTFLoader *loader : _fallbackTTFLoaders)
		delete loader;
	_fallbackTTFLoaders.clear();
}

ZoombiniText::ExeTextSource ZoombiniText::createMacExecutableSource(const ZmbReleaseInfo &relInfo, bool europeV11) {
	const Common::CodePage codePage = relInfo.getExeCodePage();
	const char *application = relInfo.getMacApplicationName();

	// USv10, USv11, TLCv20 family
	if (relInfo.isFamily(ZmbReleaseInfo::Family::kUsV10, Common::EN_USA)) {
		// 68K CODE: text offsets are A5-relative within the expanded resource-fork DATA 0.
		return ExeTextSource()
			.withTextTable(codePage, kV10USMacTextEntries, ARRAYSIZE(kV10USMacTextEntries), ExeTextDataSource::kExpandedMac68kA5Globals)
			.withRequiredBytes(0x833E, "NEW GAME (CTRL N)")
			.withMacA5RelativeCreditPtrTable(0x73A2)
			.withCreditLimits(kV10USCreditLimits)
			.inMac68kDataResFork(application, 33222);
	}
	if (relInfo.isFamily(ZmbReleaseInfo::Family::kUsV11)) {
		// 68K CODE: text offsets are A5-relative within the expanded resource-fork DATA 0.
		return ExeTextSource()
			.withTextTable(codePage, kV11USMacTextEntries, ARRAYSIZE(kV11USMacTextEntries), ExeTextDataSource::kExpandedMac68kA5Globals)
			.withRequiredBytes(0x90B6, "NEW GAME (CTRL N)")
			.withMacA5RelativeCreditPtrTable(0x80B0)
			.withCreditLimits(kV11USCreditLimits)
			.inMac68kDataResFork(application, 40327);
	}
	if (relInfo.isFamily(ZmbReleaseInfo::Family::kTlcV20)) {
		// PPC PEF: expand data-fork section 1; code literals retain raw-file offsets.
		return ExeTextSource()
			.withTextTable(codePage, kV20USMacTextEntries, ARRAYSIZE(kV20USMacTextEntries), ExeTextDataSource::kExpandedMacPpcPefDataSection)
			.withAdditionalTextTable(kV20USMacRawTextEntries, ARRAYSIZE(kV20USMacRawTextEntries))
			.withRequiredBytes(0x8AAF, "NEW GAME (CTRL N)")
			.withMacPefRelativeCreditPtrTable(0x9E44)
			.withCreditPtrBlankAddress(0x2517)
			.withCreditLimits(kV20USCreditLimits)
			.inMacPpcPefDataFork(application, 682084, 1, 96256);
	}

	// EUv10 family
	if (relInfo.isFamily(ZmbReleaseInfo::Family::kEuropeV1x, Common::JA_JPN)) {
		// 68K CODE: text offsets are A5-relative within the expanded resource-fork DATA 0.
		return ExeTextSource()
			.withTextTable(codePage, kV10JPMacTextEntries, ARRAYSIZE(kV10JPMacTextEntries), ExeTextDataSource::kExpandedMac68kA5Globals)
			.withRequiredBytes(0x860A, "New Game")
			.inMac68kDataResFork(application, 29190);
	}

	// EUv10, EUv11
	ExeTextSource source;
	uint32 dataSize = 0;
	if (relInfo.isFamily(ZmbReleaseInfo::Family::kEuropeV1x) && !europeV11) {
		// 68K CODE: text offsets are A5-relative within the expanded resource-fork DATA 0.
		dataSize = 28317;
		switch (relInfo.getLanguage()) {
		case Common::EN_GRB:
			source = source.withTextTable(codePage, kV10BRMacTextEntries, ARRAYSIZE(kV10BRMacTextEntries), ExeTextDataSource::kExpandedMac68kA5Globals)
						 .withRequiredBytes(0x836A, "New Game");
			break;
		case Common::FR_FRA:
			source = source.withTextTable(codePage, kV10FRMacTextEntries, ARRAYSIZE(kV10FRMacTextEntries), ExeTextDataSource::kExpandedMac68kA5Globals)
						 .withRequiredBytes(0x835A, "Nouvelle Partie");
			break;
		case Common::DE_DEU:
			source = source.withTextTable(codePage, kV10GEMacTextEntries, ARRAYSIZE(kV10GEMacTextEntries), ExeTextDataSource::kExpandedMac68kA5Globals)
						 .withRequiredBytes(0x834E, "Neues Spiel");
			break;
		default:
			error("ZoombiniText: unsupported Macintosh v1.0 language");
		}
	} else if (relInfo.isFamily(ZmbReleaseInfo::Family::kEuropeV1x) && europeV11) {
		// 68K CODE: text offsets are A5-relative within the expanded resource-fork DATA 0.
		dataSize = 39145;
		switch (relInfo.getLanguage()) {
		case Common::EN_GRB:
			source = source.withTextTable(codePage, kV11BRMacTextEntries, ARRAYSIZE(kV11BRMacTextEntries), ExeTextDataSource::kExpandedMac68kA5Globals)
						 .withRequiredBytes(0x89C0, "New Game");
			break;
		case Common::FR_FRA:
			source = source.withTextTable(codePage, kV11FRMacTextEntries, ARRAYSIZE(kV11FRMacTextEntries), ExeTextDataSource::kExpandedMac68kA5Globals)
						 .withRequiredBytes(0x89B0, "Nouvelle Partie");
			break;
		case Common::DE_DEU:
			source = source.withTextTable(codePage, kV11GEMacTextEntries, ARRAYSIZE(kV11GEMacTextEntries), ExeTextDataSource::kExpandedMac68kA5Globals)
						 .withRequiredBytes(0x89A4, "Neues Spiel");
			break;
		default:
			error("ZoombiniText: unsupported Macintosh v1.1 language");
		}
	} else {
		error("ZoombiniText: unsupported Macintosh retail executable");
	}
	return source.inMac68kDataResFork(application, dataSize);
}

ZoombiniText::SourceSettings ZoombiniText::createSourceSettings(const ZmbReleaseInfo &relInfo, uint32 detectedFeatures) {
	static constexpr uint32 kV10USNEExeSize = 939536;
	static constexpr uint32 kV10USPEExeSize = 633888;
	static constexpr uint32 kV10EUExeSize = 934416;
	static constexpr uint32 kV10BRDemoExeSize = 932368;
	static constexpr uint32 kV10ITExeSize = 935952;
	static constexpr uint32 kV10JPExeSize = 1273147;
	static constexpr uint32 kV10ESExeSize = 634400;
	static constexpr uint32 kV10PTExeSize = 635424;
	static constexpr uint32 kV11EUExeSize = 934928;
	static constexpr uint32 kV11USPEExeSize = 642592;
	static constexpr uint32 kV11USNEExeSize = 951824;
	static constexpr uint32 kV10USDemoNEExeSize = 936976;
	static constexpr uint32 kV111KRPEExeSize = 647168;
	static constexpr uint32 kV111KRNEExeSize = 977920;
	static constexpr uint32 kV20USExeSize = 614400;
	static constexpr uint32 kV20PLExeSize = 892928;
	static constexpr uint32 kV20USDemoExeSize = 610304;

	// Exact executable revisions and packaging belong to source selection, not gameplay identity.
	const bool europeV11 = (detectedFeatures & (GF_ZMB_11_EU | GF_ZMB_11_FR_2002)) != 0;
	const bool cabinetRelease = (detectedFeatures & GF_ZMB_11_FR_2002) != 0;
	SourceSettings sources;
	const Common::Language language = relInfo.getLanguage();
	const Common::Platform platform = relInfo.getPlatform();
	const Common::CodePage exeCodePage = relInfo.getExeCodePage();
	if (relInfo.isFamily(ZmbReleaseInfo::Family::kEuropeV1x)) {
		sources.textStrArchive = true;
		// Europe-v1 credits concat TEXTSTR.MHK STRL 2020/2021, including blanks and the final '*' marker.
		switch (language) {
		case Common::EN_GRB:
			sources.textStrCreditEntryCount = kEuropeBRCreditEntryCount;
			break;
		case Common::FR_FRA:
			sources.textStrCreditEntryCount = kEuropeFRCreditEntryCount;
			break;
		case Common::DE_DEU:
			sources.textStrCreditEntryCount = kEuropeGECreditEntryCount;
			break;
		case Common::IT_ITA:
			sources.textStrCreditEntryCount = kEuropeITCreditEntryCount;
			break;
		case Common::JA_JPN:
			sources.textStrCreditEntryCount = kEuropeJPCreditEntryCount;
			sources.decodeMacRomanUmlautByte9F = true;
			break;
		default:
			break;
		}
		if (cabinetRelease)
			sources.fontCabinet = "data1.cab";
	}
	switch (platform) {
	case Common::kPlatformMacintosh:
		if (!relInfo.isDemo()) {
			sources.executables = {createMacExecutableSource(relInfo, europeV11)};
			return sources;
		}
		break;
	case Common::kPlatformWindows:
		break;
	default:
		error("ZoombiniText::createSourceSettings: unsupported platform %d", static_cast<int>(platform));
	}
	if (relInfo.isDemo(ZmbReleaseInfo::Family::kUsV10)) {
		switch (platform) {
		case Common::kPlatformMacintosh: {
			static constexpr CreditLimits kMacCreditLimits = {248, 27, 2349};
			const ExeTextSource source = ExeTextSource()
											 .withTextTable(exeCodePage, kV10USMacDemoTextEntries, ARRAYSIZE(kV10USMacDemoTextEntries),
															ExeTextDataSource::kExpandedMac68kA5Globals)
											 .withRequiredBytes(0x82CE, "NEW GAME (CTRL N)")
											 .withMacA5RelativeCreditPtrTable(0x7332)
											 .withCreditLimits(kMacCreditLimits);
			sources.executables = {source.inMac68kDataResFork("Zoom", 33120)};
			break;
		}
		case Common::kPlatformWindows: {
			const ExeTextSource source = ExeTextSource()
											 .withTextTable(exeCodePage, kV10USDemo_NETextEntries, ARRAYSIZE(kV10USDemo_NETextEntries))
											 .withRequiredBytes(0xD9758, "NEW GAME")
											 .withCreditAnchor(kExeCreditParagraphsAnchor)
											 .withCreditLimits(kV11USCreditLimits);
			sources.executables = {source.inFile("ZOOM.EXE", kV10USDemoNEExeSize)};
			break;
		}
		default:
			error("ZoombiniText::createSourceSettings: unknown platform(%d) for v1.0US Demo", platform);
			break;
		}
		return sources;
	}
	if (relInfo.isFamily(ZmbReleaseInfo::Family::kUsV10, Common::EN_USA)) {
		const ExeTextSource peSource = ExeTextSource()
										   .withTextTable(exeCodePage, kV10US_PETextEntries, ARRAYSIZE(kV10US_PETextEntries))
										   .withRequiredBytes(0x8A5E2, "NEW GAME (CTRL N)")
										   .withCreditPtrTable(0x894A4, 0x41C000, 0)
										   .withCreditLimits(kV10USCreditLimits);
		const ExeTextSource neSource = ExeTextSource()
										   .withTextTable(exeCodePage, kV10US_NETextEntries, ARRAYSIZE(kV10US_NETextEntries))
										   .withCreditAnchor(kExeCreditParagraphsAnchor)
										   .withCreditLimits(kV10USCreditLimits);
		sources.executables = {
			peSource.inArchiveMember("ZBARCHIV.Z", "zoombi32.exe", kV10USPEExeSize),
			neSource.inFile("ZOOMBINI._EX", kV10USNEExeSize),
			peSource.inFile("ZOOMBI32.EXE", kV10USPEExeSize),
			neSource.inFile("ZOOMBINI.EXE", kV10USNEExeSize),
		};
	} else if (relInfo.isFamily(ZmbReleaseInfo::Family::kUsV10)) {
		switch (language) {
		case Common::ES_ESP: {
			const ExeTextSource source = ExeTextSource()
											 .withTextTable(exeCodePage, kV10ES_PETextEntries, ARRAYSIZE(kV10ES_PETextEntries))
											 .withRequiredBytes(0x888EA, "Nueva Partida")
											 .withTextPatches(&ZoombiniText::patchSpanish10ExeText)
											 .withCreditAnchor(kExeSpanishCreditParagraphsAnchor)
											 .withCreditLimits(kV10ESCreditLimits);
			sources.executables = {
				source.inFile("ZOOMBI32._EX", kV10ESExeSize),
				source.inFile("ZOOMBI32.EXE", kV10ESExeSize),
			};
			break;
		}
		case Common::PT_PRT: {
			const ExeTextSource source = ExeTextSource()
											 .withTextTable(exeCodePage, kV10PT_PETextEntries, ARRAYSIZE(kV10PT_PETextEntries))
											 .withRequiredBytes(0x888EA, "Novo Jogo")
											 .withCreditAnchor(kExePortugueseCreditParagraphsAnchor)
											 .withCreditLimits(kV10PTCreditLimits);
			sources.executables = {
				source.inArchiveMember("ZBARCHIV.Z", "zoombi32.exe", kV10PTExeSize),
				source.inFile("zoombi32.exe", kV10PTExeSize),
			};
			break;
		}
		default:
			return sources;
		}
	} else if (relInfo.isFamily(ZmbReleaseInfo::Family::kEuropeV1x)) {
		if (relInfo.isDemo(ZmbReleaseInfo::Family::kEuropeV1x, Common::EN_GRB)) { // v1.0BR Demo
			const ExeTextSource source = ExeTextSource()
											 .withTextTable(exeCodePage, kV10BRDemo_NETextEntries, ARRAYSIZE(kV10BRDemo_NETextEntries))
											 .withRequiredBytes(0xD72C8, "New Game");
			sources.executables = {source.inFile("ZOOMBINI.EXE", kV10BRDemoExeSize)};
		} else if (relInfo.isFamily(ZmbReleaseInfo::Family::kEuropeV1x, Common::JA_JPN)) {
			if (relInfo.isDemo() && platform == Common::kPlatformWindows) {
				const ExeTextSource source = ExeTextSource()
												 .withTextTable(exeCodePage, kV10JPDemo_PETextEntries, ARRAYSIZE(kV10JPDemo_PETextEntries))
												 .withRequiredBytes(0x9044B, "Japanese");
				sources.executables = {source.inFile("ZOOMBI32.EXE", 679936)};
				return sources;
			} else if (relInfo.isDemo()) {
				const ExeTextSource source = ExeTextSource()
												 .withTextTable(exeCodePage, kV10JPMacDemoTextEntries, ARRAYSIZE(kV10JPMacDemoTextEntries))
												 .withRequiredBytes(0x16D99, "1.0  1996 Broderbund Software.");
				sources.executables = {source.inFile("Zoombini", 722944)};
				return sources;
			}
			const ExeTextSource source = ExeTextSource()
											 .withTextTable(exeCodePage, kV10JP_PETextEntries, ARRAYSIZE(kV10JP_PETextEntries))
											 .withRequiredBytes(0x88427, "Japanese");
			sources.executables = {
				source.inArchiveMember("ZBARCHIV.Z", "zoombi32.exe", kV10JPExeSize),
				source.inFile("zoombi32.exe", kV10JPExeSize),
			};
		} else if (cabinetRelease) {
			const ExeTextSource source = ExeTextSource()
											 .withTextTable(exeCodePage, kV11FR_NETextEntries, ARRAYSIZE(kV11FR_NETextEntries))
											 .withRequiredBytes(0xD7CBD, "Nouvelle Partie");
			sources.executables = {
				source.inCabinetMember("data1.cab", "ZOOMBINI.EXE", kV11EUExeSize),
			};
		} else if (!europeV11) {
			ExeTextSource source;
			uint32 executableSize = kV10EUExeSize;
			switch (language) {
			case Common::EN_GRB:
				source = ExeTextSource()
							 .withTextTable(exeCodePage, kV10BR_NETextEntries, ARRAYSIZE(kV10BR_NETextEntries))
							 .withRequiredBytes(0xD6840, "New Game");
				break;
			case Common::FR_FRA:
				source = ExeTextSource()
							 .withTextTable(exeCodePage, kV10FR_NETextEntries, ARRAYSIZE(kV10FR_NETextEntries))
							 .withRequiredBytes(0xD6849, "Nouvelle Partie");
				break;
			case Common::DE_DEU:
				source = ExeTextSource()
							 .withTextTable(exeCodePage, kV10GE_NETextEntries, ARRAYSIZE(kV10GE_NETextEntries))
							 .withRequiredBytes(0xD6859, "Neues Spiel");
				break;
			case Common::IT_ITA:
				source = ExeTextSource()
							 .withTextTable(exeCodePage, kV10IT_NETextEntries, ARRAYSIZE(kV10IT_NETextEntries))
							 .withRequiredBytes(0xD78DE, "Nuova partita");
				executableSize = kV10ITExeSize;
				break;
			default:
				return sources;
			}
			sources.executables = {
				source.inFile("ZOOMBINI.EXE", executableSize),
				source.inFile("ZOOMBINI._EX", executableSize)};
		} else {
			ExeTextSource source;
			switch (language) {
			case Common::EN_GRB:
				source = ExeTextSource()
							 .withTextTable(exeCodePage, kV11BR_NETextEntries, ARRAYSIZE(kV11BR_NETextEntries))
							 .withRequiredBytes(0xD7CB4, "New Game");
				break;
			case Common::FR_FRA:
				source = ExeTextSource()
							 .withTextTable(exeCodePage, kV11FR_NETextEntries, ARRAYSIZE(kV11FR_NETextEntries))
							 .withRequiredBytes(0xD7CBD, "Nouvelle Partie");
				break;
			case Common::DE_DEU:
				source = ExeTextSource()
							 .withTextTable(exeCodePage, kV11GE_NETextEntries, ARRAYSIZE(kV11GE_NETextEntries))
							 .withRequiredBytes(0xD7CCD, "Neues Spiel");
				break;
			default:
				return sources;
			}
			sources.executables = {source.inFile("ZOOMBINI._EX", kV11EUExeSize)};
		}
	} else if (relInfo.isFamily(ZmbReleaseInfo::Family::kUsV11)) {
		if (language == Common::EN_USA) {
			const ExeTextSource peSource = ExeTextSource()
											   .withTextTable(exeCodePage, kV11US_PETextEntries, ARRAYSIZE(kV11US_PETextEntries))
											   .withRequiredBytes(0x8B9FC, "NEW GAME")
											   .withCreditAnchor(kExeCreditParagraphsAnchor)
											   .withCreditLimits(kV11USCreditLimits);
			const ExeTextSource neSource = ExeTextSource()
											   .withTextTable(exeCodePage, kV11US_NETextEntries, ARRAYSIZE(kV11US_NETextEntries))
											   .withRequiredBytes(0xDCBB2, "NEW GAME")
											   .withCreditAnchor(kExeCreditParagraphsAnchor)
											   .withCreditLimits(kV11USCreditLimits);
			sources.executables = {
				peSource.inArchiveMember("ZBARC32.Z", "Zoombi32.exe", kV11USPEExeSize),
				neSource.inArchiveMember("ZBARC16.Z", "ZOOMBINI.EXE", kV11USNEExeSize),
				peSource.inFile("Zoombi32.exe", kV11USPEExeSize),
				neSource.inFile("ZOOMBINI.EXE", kV11USNEExeSize),
			};
		} else if (language == Common::KO_KOR) {
			const ExeTextSource peSource = ExeTextSource()
											   .withTextTable(exeCodePage, kV111KR_PETextEntries, ARRAYSIZE(kV111KR_PETextEntries))
											   .withWindows1252StrokeByteD8Decode()
											   .withCreditLinePatches(&ZoombiniText::patchKoreanExeCredits)
											   .withCreditAnchor(kExeCreditParagraphsAnchor)
											   .withCreditLimits(kV111KRCreditLimits);
			const ExeTextSource neSource = ExeTextSource()
											   .withTextTable(exeCodePage, kV111KR_NETextEntries, ARRAYSIZE(kV111KR_NETextEntries))
											   .withWindows1252StrokeByteD8Decode()
											   .withCreditLinePatches(&ZoombiniText::patchKoreanExeCredits)
											   .withCreditAnchor(kExeCreditParagraphsAnchor)
											   .withCreditLimits(kV111KRCreditLimits);
			sources.executables = {
				peSource.inFile("SETUP/data1/data32/Zoombi32.exe", kV111KRPEExeSize),
				neSource.inFile("SETUP/data1/data16/Zoombini.exe", kV111KRNEExeSize),
				peSource.inFile("Zoombi32.exe", kV111KRPEExeSize),
				neSource.inFile("Zoombini.exe", kV111KRNEExeSize),
			};
		}
	} else if (relInfo.isFamily(ZmbReleaseInfo::Family::kTlcV20)) {
		if (relInfo.isDemo(ZmbReleaseInfo::Family::kTlcV20)) {
			const ExeTextSource source = ExeTextSource()
											 .withTextTable(exeCodePage, kV20USDemo_PETextEntries, ARRAYSIZE(kV20USDemo_PETextEntries))
											 .withRequiredBytes(0x909B8, "NEW GAME")
											 .withMacRomanTrademarkByteAAEscape()
											 .withCreditPtrTable(0x8F09C, 0x400000, 0)
											 .withCreditPtrBlankAddress(0x4A180C)
											 .withCreditLimits(kV20USCreditLimits);
			sources.executables = {
				source.inFile("Zoom.exe", kV20USDemoExeSize)};
		} else if (language == Common::PL_POL) {
			const ExeTextSource source = ExeTextSource()
											 .withTextTable(exeCodePage, kV20PL_PETextEntries, ARRAYSIZE(kV20PL_PETextEntries))
											 .withRequiredBytes(0xC9AF4, "Bez nazwy")
											 .withMacRomanTrademarkByteAAEscape()
											 .withCreditPtrTable(0xCA190, 0x413000, 21)
											 .withCreditPtrBlankAddressRange(0x4DD234, 0x4DD3F4)
											 .withCreditLimits(kV20PLCreditLimits);
			sources.executables = {
				source.inFile("INSTALL/HD/Zoombinis Logical Journey.exe", kV20PLExeSize),
			};
		} else {
			const ExeTextSource source = ExeTextSource()
											 .withTextTable(exeCodePage, kV20US_PETextEntries, ARRAYSIZE(kV20US_PETextEntries))
											 .withMacRomanTrademarkByteAAEscape()
											 .withCreditPtrTable(0x90080, 0x400000, 21)
											 .withCreditPtrBlankAddress(0x4A286C)
											 .withCreditLimits(kV20USCreditLimits);
			sources.executables = {
				source.inFile("INSTALL/HD/Zoombinis Logical Journey.exe", kV20USExeSize),
			};
		}
	}
	return sources;
}

bool ZoombiniText::initExecutableStrings(const Common::Array<ExeTextSource> &sources) {
	for (const ExeTextSource &source : sources) {
		if (tryExecutableSource(source))
			return true;
	}
	return false;
}

bool ZoombiniText::tryExecutableSource(const ExeTextSource &source) {
	Common::SeekableReadStream *exeStream = nullptr;
	Common::MacResManager macApplication;
	uint32 bytesAboveA5 = 0;
	uint32 bytesBelowA5 = 0;

	switch (source.primaryDataSource) {
	case ExeTextDataSource::kExpandedMac68kA5Globals: {
		if (!macApplication.open(Common::Path(source.fileName)))
			return false;
		Common::ScopedPtr<Common::SeekableReadStream> codeStream(macApplication.getResource(MKTAG('C', 'O', 'D', 'E'), 0));
		if (!readMac68kA5WorldSizes(codeStream.get(), bytesAboveA5, bytesBelowA5))
			return false;
		exeStream = macApplication.getResource(MKTAG('D', 'A', 'T', 'A'), 0);
		break;
	}
	case ExeTextDataSource::kExpandedMacPpcPefDataSection:
		exeStream = Common::MacResManager::openFileOrDataFork(Common::Path(source.fileName));
		break;
	case ExeTextDataSource::kRawExecutableFile:
		if (source.fileName) {
			Common::File *file = new Common::File();
			if (file->open(Common::Path(source.fileName)))
				exeStream = file;
			else
				delete file;
		}
		break;
	default:
		return false;
	}

	if (!exeStream && source.archiveName) {
		Common::Archive *archive = nullptr;
		if (source.archiveIsCabinet) {
			// FIXME: Pass source.expectedSize after the shared InstallShield reader supports caller-provided exact extraction sizes.
			archive = Common::makeInstallShieldArchive(Common::Path(source.archiveName));
		} else {
			Common::InstallShieldV3 *v3Archive = new Common::InstallShieldV3();
			if (v3Archive->open(Common::Path(source.archiveName)))
				archive = v3Archive;
			else
				delete v3Archive;
		}
		if (archive) {
			exeStream = archive->createReadStreamForMember(Common::Path(source.archiveMemberName));
			delete archive;
		}
	}

	if (!exeStream)
		return false;

	Common::Array<byte> exeFileData;          // Raw file stream.
	Common::Array<byte> mac68kA5Globals;      // Expanded data section of Mac 68K binary.
	Common::Array<byte> macPpcPefDataSection; // Expanded data section of Mac PPC PEF binary.
	bool resultSuccess;
	switch (source.primaryDataSource) {
	case ExeTextDataSource::kExpandedMac68kA5Globals:
		resultSuccess = readExecutableData(exeStream, source.expectedSize, mac68kA5Globals);
		break;
	case ExeTextDataSource::kRawExecutableFile:
	case ExeTextDataSource::kExpandedMacPpcPefDataSection:
		resultSuccess = readExecutableData(exeStream, source.expectedSize, exeFileData);
		break;
	default:
		delete exeStream;
		return false;
	}
	if (!resultSuccess) {
		delete exeStream;
		return false;
	}

	switch (source.primaryDataSource) {
	case ExeTextDataSource::kRawExecutableFile:
		resultSuccess = !source.requiredBytes || bytesMatchAt(exeFileData, source.requiredBytesOffset, source.requiredBytes);
		break;
	case ExeTextDataSource::kExpandedMac68kA5Globals:
		if (!expandMac68kResForkRleGlobals(mac68kA5Globals, bytesAboveA5, bytesBelowA5)) {
			delete exeStream;
			return false;
		}
		resultSuccess = !source.requiredBytes || bytesMatchAt(mac68kA5Globals, source.requiredBytesOffset, source.requiredBytes);
		break;
	case ExeTextDataSource::kExpandedMacPpcPefDataSection:
		if (!expandMacPpcPefDataSection(exeFileData, source.macPpcPefDataSection, source.macPpcPefDataSize, macPpcPefDataSection)) {
			delete exeStream;
			return false;
		}
		resultSuccess = !source.requiredBytes || bytesMatchAt(macPpcPefDataSection, source.requiredBytesOffset, source.requiredBytes);
		break;
	default:
		delete exeStream;
		return false;
	}

	if (!resultSuccess) {
		delete exeStream;
		return false;
	}

	Common::Array<Common::U32String> strings;
	uint entryCount = 0;
	for (const ExeTextTable &table : source.textTables)
		entryCount += table.entryCount;
	strings.reserve(entryCount);
	bool loaded = true;

	for (const ExeTextTable &table : source.textTables) {
		const Common::Array<byte> *textData = nullptr;
		switch (table.dataSource) {
		case ExeTextDataSource::kRawExecutableFile:
			textData = &exeFileData;
			break;
		case ExeTextDataSource::kExpandedMac68kA5Globals:
			textData = &mac68kA5Globals;
			break;
		case ExeTextDataSource::kExpandedMacPpcPefDataSection:
			textData = &macPpcPefDataSection;
			break;
		default:
			loaded = false;
			break;
		}
		if (!loaded || textData->empty()) {
			loaded = false;
			break;
		}

		for (uint entryIndex = 0; entryIndex < table.entryCount; entryIndex++) {
			const ExeTextEntry &entry = table.entries[entryIndex];
			if (textData->size() < static_cast<uint64>(entry.offset) + entry.length) {
				loaded = false;
				break;
			}
			strings.push_back(decodeExecutableStringBytes(textData->data() + entry.offset, entry.length, source));
		}
		if (!loaded)
			break;
	}

	const bool sourceHasCredits = source.creditSourceKind != CreditSourceKind::kNone;
	Common::Array<CreditParagraph> executableCredits;
	bool creditsLoaded = false;
	if (loaded && sourceHasCredits) {
		switch (source.primaryDataSource) {
		case ExeTextDataSource::kRawExecutableFile:
			creditsLoaded = loadOriginalExecutableCredits(exeFileData, source, executableCredits);
			break;
		case ExeTextDataSource::kExpandedMac68kA5Globals:
			creditsLoaded = loadOriginalExecutableCredits(mac68kA5Globals, source, executableCredits);
			break;
		case ExeTextDataSource::kExpandedMacPpcPefDataSection:
			creditsLoaded = loadOriginalExecutableCredits(macPpcPefDataSection, source, executableCredits);
			break;
		default:
			break;
		}
	}
	if (loaded && sourceHasCredits && !creditsLoaded)
		loaded = false;

	delete exeStream;

	if (!loaded)
		return false;

	uint stringIdx = 0;
	for (const ExeTextTable &table : source.textTables) {
		for (uint entryIdx = 0; entryIdx < table.entryCount; entryIdx++) {
			_strMap[table.entries[entryIdx].key] = strings[stringIdx];
			stringIdx += 1;
		}
	}
	if (source.textPatches)
		(this->*source.textPatches)();

	if (creditsLoaded) {
		_creditParagraphs = executableCredits;
		if (source.creditLinePatches)
			(this->*source.creditLinePatches)();
	}

	return true;
}

Common::Array<Common::String> ZoombiniText::tokenizeLines(const Common::String &text) {
	Common::Array<Common::String> lines;
	for (size_t lastIdx = 0; lastIdx < text.size();) {
		size_t chIdx = text.findFirstOf(Common::String("\r\n"), lastIdx);
		if (chIdx == Common::String::npos) {
			chIdx = text.findFirstOf('\r', lastIdx);
		}
		if (chIdx == Common::String::npos) {
			chIdx = text.findFirstOf('\n', lastIdx);
		}

		if (chIdx != Common::String::npos) {
			lines.push_back(text.substr(lastIdx, chIdx - lastIdx));
			lastIdx = chIdx + 1;
			if (text[chIdx] == '\r' && lastIdx < text.size() && text[lastIdx] == '\n')
				lastIdx += 1;
		} else {
			lines.push_back(text.substr(lastIdx, text.size() - lastIdx));
			break;
		}
	}
	return lines;
}

Common::Array<Common::U32String> ZoombiniText::tokenizeLines(const Common::U32String &text) {
	Common::Array<Common::U32String> lines;
	for (size_t lastIdx = 0; lastIdx < text.size();) {
		size_t chIdx = text.findFirstOf(Common::U32String("\r\n"), lastIdx);
		if (chIdx == Common::U32String::npos) {
			chIdx = text.findFirstOf('\r', lastIdx);
		}
		if (chIdx == Common::U32String::npos) {
			chIdx = text.findFirstOf('\n', lastIdx);
		}

		if (chIdx != Common::U32String::npos) {
			lines.push_back(text.substr(lastIdx, chIdx - lastIdx));
			lastIdx = chIdx + 1;
			if (text[chIdx] == '\r' && lastIdx < text.size() && text[lastIdx] == '\n')
				lastIdx += 1;
		} else {
			lines.push_back(text.substr(lastIdx, text.size() - lastIdx));
			break;
		}
	}
	return lines;
}

Common::String ZoombiniText::formatCreditLineKey(uint32 paragraphIndex, uint32 lineIndex) {
	return Common::String::format("credit-g%02u-%03u", paragraphIndex, lineIndex);
}

const Graphics::Font *ZoombiniText::getTextFont() {
	return FontMan.getFontByName(_textFont.cacheName);
}

const Graphics::Font *ZoombiniText::getTitleFont() {
	return FontMan.getFontByName(_titleFont.cacheName);
}

const Graphics::Font *ZoombiniText::getFont(ZoombiniFontUsage fontUsage) {
	switch (fontUsage) {
	case ZoombiniFontUsage::kFontDebugTitle: // For debug console
		return FontMan.getFontByUsage(Graphics::FontManager::kBigGUIFont);
	case ZoombiniFontUsage::kFontDebugText:
		return FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
	case ZoombiniFontUsage::kFontTitle:
		return getTitleFont();
	case ZoombiniFontUsage::kFontText:
		return getTextFont();
	default:
		error("ZoombiniText: not supported ZmbFontKind %u", static_cast<uint32>(fontUsage));
		return nullptr;
	}
}

bool ZoombiniText::fontUsesAntialiasing(ZoombiniFontUsage fontUsage) const {
	switch (fontUsage) {
	case ZoombiniFontUsage::kFontTitle:
		return _titleFont.enableHinting;
	case ZoombiniFontUsage::kFontText:
		return _textFont.enableHinting;
	case ZoombiniFontUsage::kFontDebugTitle:
	case ZoombiniFontUsage::kFontDebugText:
	default:
		return false;
	}
}

const Graphics::Font *ZoombiniText::loadFont(FontState &state, bool showWarnMsgBox) {
	TTFLoader *firstOptimalLoader = nullptr;
	if (!_optimalTTFLoaders.empty())
		firstOptimalLoader = _optimalTTFLoaders.front();
	if (!firstOptimalLoader) {
		error("ZoombiniText: no primary font loader is configured");
		return nullptr;
	}
	for (TTFLoader *loader : _optimalTTFLoaders) {
		const Graphics::Font *font = loader->loadFont(state.point);
		if (!font)
			continue;

		state.cacheName = loader->getCacheName(state.point);
		state.enableHinting = loader->usesAntialiasing();
		return font;
	}

	Common::U32String noFontMsg;
	Common::U32String fontSourceMsg;
	switch (_relInfo.getPlatform()) {
	case Common::kPlatformMacintosh:
		switch (_relInfo.getLanguage()) {
		case Common::JA_JPN:
			if (_relInfo.isFamily(ZmbReleaseInfo::Family::kEuropeV1x)) {
				// I18N: Osaka is a font name and must not be translated.
				noFontMsg = _("ScummVM cannot load the Osaka font used by the Japanese Macintosh version.");
				// I18N: Osaka is a font name; quoted font file names must not be translated.
				fontSourceMsg = _("Obtain the font in one of the following ways:\n"
								  "- Export Osaka 4.2 from classic Mac OS 9.2.2 Japanese Language Kit in MacBinary as 'Osaka.bin'.\n"
								  "- Export 'Osaka.ttf' from modern macOS.\n\n"
								  "Place the file in the game directory or the configured Extra Path directory.\n");
			}
			break;
		default:
			break;
		}
		break;
	case Common::kPlatformWindows:
		switch (_relInfo.getLanguage()) {
		case Common::KO_KOR:
			// I18N: 'GulimChe' is a font name; 'gulim.ttc' is a literal file name and must not be translated.
			fontSourceMsg = _("The Korean release requires the GulimChe ('gulim.ttc') font from the Windows font archive.");
			break;
		case Common::JA_JPN:
			// I18N: 'MS Gothic' is a font name; 'msgothic.ttc' is a literal file name and must not be translated.
			fontSourceMsg = _("The Japanese release requires the MS Gothic ('msgothic.ttc') font from the Windows font archive.");
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
	if (noFontMsg.empty()) {
		// I18N: The three %s placeholders are the game language, release label (e.g. v1.11KR), and required font file name, in that order.
		noFontMsg = Common::U32String::format(_("Zoombini (%s, %s) requires the font '%s' to display text correctly, but ScummVM cannot access it."),
											  Common::getLanguageDescription(_relInfo.getLanguage()), _vm->_gameDescription->desc.extra,
											  firstOptimalLoader->_filePath.baseName().c_str());
		// I18N: %s is the file name of the required font.
		Common::U32String copyMsg = Common::U32String::format(_("Copy the required font '%s' into the game directory or the configured Extra Path directory."),
															  firstOptimalLoader->_filePath.baseName().c_str());
		if (!fontSourceMsg.empty()) {
			copyMsg += U"\n";
			copyMsg += fontSourceMsg;
		}
		fontSourceMsg = copyMsg;
	}
	if (!fontSourceMsg.empty()) {
		noFontMsg += U"\n\n";
		noFontMsg += fontSourceMsg;
	}

	const char *primaryFontSourceInstruction = firstOptimalLoader->_srcInst.c_str();

	// Look up fallback fonts when the optimal font is unavailable.
	for (TTFLoader *loader : _fallbackTTFLoaders) {
		const Graphics::Font *font = loader->loadFont(state.point);
		if (!font)
			continue;

		if (showWarnMsgBox) {
			const char *srcInst = primaryFontSourceInstruction;
			if (srcInst[0] == '\0')
				srcInst = loader->_srcInst.c_str();

			Common::U32String fontSourceInstruction = Common::String(srcInst).decode(Common::kUtf8);
			// I18N: The first %S is the complete localized missing-font message; %s is the fallback font file name.
			// The final %S is the complete localized instruction for obtaining the required font.
			Common::U32String warnMsg = Common::U32String::format(_("%S\nScummVM will use the fallback font '%s' instead.\nText layout may be incorrect.\n\n%S"),
																  noFontMsg.c_str(), loader->_filePath.baseName().c_str(), fontSourceInstruction.c_str());
			warning("%s", warnMsg.encode(Common::kUtf8).c_str());
			GUI::MessageDialog dialog(warnMsg);
			dialog.runModal();
		}

		state.cacheName = loader->getCacheName(state.point);
		state.enableHinting = loader->usesAntialiasing();
		return font;
	}

	error("ZoombiniText: failed to load required font\n\n%s\n\n%s", noFontMsg.encode(Common::kUtf8).c_str(), primaryFontSourceInstruction);
	return nullptr;
}

Common::U32String ZoombiniText::toU32String(const byte *buf, StringKind strKind) const {
	return toU32String(reinterpret_cast<const char *>(buf), strKind);
}

Common::U32String ZoombiniText::toU32String(const byte *buf, int32 len, StringKind strKind) const {
	return toU32String(reinterpret_cast<const char *>(buf), len, strKind);
}

Common::U32String ZoombiniText::toU32String(const char *str, StringKind strKind) const {
	return Common::String(str).decode(strKind == kExeString ? _exeCodePage : _resCodePage);
}

Common::U32String ZoombiniText::toU32String(const char *str, int32 len, StringKind strKind) const {
	// Create a temporary null-terminated buffer to avoid warning of
	// "WARNING: Adding \0 to String. This is permitted, but can have unwanted consequences"
	char *buf = new char[len + 1];
	memcpy(buf, str, len);
	buf[len] = '\0';
	const Common::U32String &u32str = Common::String(buf).decode(strKind == kExeString ? _exeCodePage : _resCodePage);
	delete[] buf;
	return u32str;
}

Common::U32String ZoombiniText::toU32String(const Common::String &str, StringKind strKind) const {
	return str.decode(strKind == kExeString ? _exeCodePage : _resCodePage);
}

Common::String ZoombiniText::fromU32String(const Common::U32String &ustr, StringKind strKind) const {
	return ustr.encode(strKind == kExeString ? _exeCodePage : _resCodePage);
}

bool ZoombiniText::getStrl(Common::Array<Common::U32String> &outStrs, ZmbResource resource) {
	// STRL format: <COUNT: uint8> <null-terminated string> ...
	outStrs.clear();
	Common::SeekableReadStream *stringStream = _vm->getResource(ID_STRL, resource);
	if (!ZmbResource::hasBytes(stringStream, 1)) {
		delete stringStream;
		return false;
	}
	const byte subStrCount = stringStream->readByte();

	Common::Array<Common::U32String> parsedStrs;
	if (resource._archiveKind == ZmbResource::kSystem && resource._id == ZoombiniPage::kSysResStrl2900_HelpPickerUpdate && subStrCount == 1 &&
		stringStream->size() == 1 && _relInfo.isFamily(ZmbReleaseInfo::Family::kTlcV20, Common::PL_POL)) {
		// Polish v2.0 STRL s:2900 (Picker update help) contains only the count byte (01), with no string or NUL terminator.
		// Treat this specific resource as one empty string.
		parsedStrs.push_back(Common::U32String());
		delete stringStream;
		outStrs = parsedStrs;
		return true;
	}
	if (!readStrlBody(stringStream, subStrCount, _resCodePage, parsedStrs)) {
		delete stringStream;
		return false;
	}

	const bool consumedExactly = stringStream->pos() == stringStream->size();
	delete stringStream;
	if (!consumedExactly || subStrCount == 0 || parsedStrs.size() != subStrCount)
		return false;
	outStrs = parsedStrs;
	return true;
}

bool ZoombiniText::getStrl(Common::U32String &outStr, ZmbResource resource, uint16 subStrIdx) {
	outStr.clear();
	Common::Array<Common::U32String> strings;
	if (!getStrl(strings, resource) || strings.size() <= subStrIdx)
		return false;
	outStr = strings[subStrIdx];
	return true;
}

bool ZoombiniText::isAllowedZoombiniName(const Common::String &name) {
	bool asciiOnly = true;
	for (uint i = 0; i < name.size(); i++) {
		if (0x7F < static_cast<byte>(name[i])) {
			asciiOnly = false;
			break;
		}
	}
	if (asciiOnly)
		return true;
	if (!_relInfo.supportsLocalizedNames())
		return false;

	const Common::U32String decoded(name, _exeCodePage);
	Common::String encoded;
	if (decoded.encode(encoded, _exeCodePage, '?') != Common::kStringEncodingResultSucceeded || encoded != name)
		return false;

	switch (_exeCodePage) {
	case Common::kWindows932:
		// FIXME: Use MacJapanese for Macintosh once the ScummVM codec supports it.
		return isGeneratedJapaneseZoombiniName(decoded);
	case Common::kWindows949:
		for (uint16 nameId = 0; nameId < 625; nameId++) {
			if (getKoreanZoombiniNameById(nameId) == decoded)
				return true;
		}
		return false;
	default:
		return false;
	}
}

bool ZoombiniText::isKanaInJapaneseNameTable(char32_t character, const char32_t *table) {
	for (uint i = 0; table[i]; i++) {
		if (table[i] == character)
			return true;
	}
	return false;
}

bool ZoombiniText::isGeneratedJapaneseZoombiniName(const Common::U32String &name) {
	// Fixed F1/F2 names bypass the random halfwidth-kana grammar.
	if (name == kJapaneseF1DebugName || name == kJapaneseDebugF1Name)
		return true;
	const uint length = name.size();
	// A small-kana NUL can leave two visible characters; a final pair can exceed the target by one.
	if (length < 2 || kJapaneseNameMaxTargetLength + 1 < length || !isKanaInJapaneseNameTable(name[0], kJapaneseInitialKana))
		return false;

	// Match generator choices with dynamic programming, without drawing random numbers or enumerating seeds.
	// Transitions share the same prefixes for every target length, so compute them once up to the largest target.
	// reachable[pos][contracted][longVowel] matches name[0..pos), with the next write at pos and the two generator flags.
	// The flags mirror previousContractedKana/previousLongVowel; a set flag blocks that special choice.
	// Kana pools overlap, so retain every matching path. The checked initial kana leaves both flags clear.
	bool reachable[kJapaneseNameMaxTargetLength + 2][2][2] = {};
	reachable[1][0][0] = true;
	// A choice must start before the largest target, but a two-character choice may finish one position beyond it.
	for (uint pos = 1; pos < kJapaneseNameMaxTargetLength && pos < length; pos++) {
		for (uint contracted = 0; contracted < 2; contracted++) {
			for (uint longVowel = 0; longVowel < 2; longVowel++) {
				if (!reachable[pos][contracted][longVowel])
					continue;
				const char32_t ch = name[pos];
				// Plain kana and voiced pairs clear both flags; a long vowel retains the contracted flag.
				if (isKanaInJapaneseNameTable(ch, kJapaneseNormalKana))
					reachable[pos + 1][0][0] = true;
				if (!longVowel && ch == U'ｰ')
					reachable[pos + 1][contracted][1] = true;
				if (pos + 1 < length) {
					if ((isKanaInJapaneseNameTable(ch, kJapaneseSemiVoicedKana) && name[pos + 1] == U'ﾟ') ||
						(isKanaInJapaneseNameTable(ch, kJapaneseVoicedKana) && name[pos + 1] == U'ﾞ'))
						reachable[pos + 2][0][0] = true;
				}
				// A contracted pair sets its flag while retaining the long-vowel flag.
				if (!contracted && isKanaInJapaneseNameTable(ch, kJapaneseContractedKana)) {
					// Selecting NUL as the small kana ends the visible name; later generated bytes stay hidden.
					if (pos + 1 == length)
						return true;
					if (isKanaInJapaneseNameTable(name[pos + 1], kJapaneseSmallKana))
						reachable[pos + 2][1][longVowel] = true;
				}
			}
		}
	}
	// A full match can select its length as the target, or the largest target when the final pair extends it by one.
	// Shorter visible names require the early-NUL path above; any final flag combination is valid here.
	if (kJapaneseNameMinTargetLength <= length) {
		for (uint contracted = 0; contracted < 2; contracted++) {
			for (uint longVowel = 0; longVowel < 2; longVowel++) {
				if (reachable[length][contracted][longVowel])
					return true;
			}
		}
	}
	return false;
}

Common::U32String ZoombiniText::pickNextZoombiniName() {
	return pickNextZoombiniName(kJPFixedNameNone);
}

Common::U32String ZoombiniText::pickNextZoombiniName(JPFixedNameKind jpFixedName) {
	if (_relInfo.isFamily(ZmbReleaseInfo::Family::kEuropeV1x, Common::JA_JPN) && _useJapaneseNames)
		return generateJapaneseZoombiniName(jpFixedName);

	// Only v1.11KR release has the STRL name pool.
	// Use resource presence rather than a version check so the fallback remains capability-based.
	if (_vm->hasResource(ID_STRL, ZmbResource(ZmbResource::kSystem, ZoombiniPage::kSysResStrl30000_ZoombiniNames)))
		return getKoreanZoombiniName();
	else
		return generateLatinZoombiniName();
}

/**
 * Generate a Latin Zoombini name from the original syllable tables.
 *
 * Releases without STRL 30000-30006 generate names from these syllable tables.
 *
 * The algorithm alternates consonant and vowel syllables:
 *   - 39% chance to start with a vowel, 61% consonant.
 *   - Consonants at position <= 1: 67% single, 33% blend.
 *   - Consonants at position > 1: always blend.
 *   - If a blend lands last, replace the final character with a simple vowel.
 *   - If the first two characters are identical, retry from position 1.
 *   - Name length: random [4, 8].
 */

Common::U32String ZoombiniText::generateLatinZoombiniName() {
	// 31 entries, 2 bytes each.  When the 2nd byte is a space (0x20),
	// only the 1st byte is emitted (weighted single vowel).
	static constexpr char kVowelPairs[] =
		"a a a "   // a x3
		"e e e e " // e x4
		"i i i "   // i x3
		"o o o "   // o x3
		"u u "     // u x2
		"y "       // y x1
		"ee"
		"oo"
		"yo"
		"ya"
		"ye"
		"ei"
		"ie"
		"ai"
		"ia"
		"au"
		"ua"
		"uo"
		"ou"
		"ae"
		"ea";

	// 32 weighted single consonants (some appear multiple times).
	static constexpr char kSingleConsonants[] = "bbccdddfghjkkllmmnnprrssssttvwxz";

	// 6 simple vowels, used to replace a trailing consonant blend.
	static constexpr char kSimpleVowels[] = "aeiouy";

	// 40 consonant blends, 2 bytes each.
	static constexpr char kConsonantBlends[] =
		"bl"
		"br"
		"ch"
		"cl"
		"cr"
		"dr"
		"dw"
		"fl"
		"fr"
		"gh"
		"gl"
		"gr"
		"kl"
		"kn"
		"kr"
		"kw"
		"ld"
		"mp"
		"nd"
		"nh"
		"nn"
		"ph"
		"pl"
		"pr"
		"qu"
		"qu"
		"rh"
		"rn"
		"sc"
		"sl"
		"sm"
		"sn"
		"sp"
		"sr"
		"st"
		"sw"
		"th"
		"tr"
		"tw"
		"wr";

	static constexpr int kNumVowelPairs = 31;
	static constexpr int kNumSingleConsonants = 32;
	static constexpr int kNumSimpleVowels = 6;
	static constexpr int kNumConsonantBlends = 40;
	static constexpr int kMaxNameLen = 10;

	char buf[kMaxNameLen];
	memset(buf, 0, sizeof(buf));

	// Choose a name length from 4 through 8, bounded by kMaxNameLen.
	int16 nameLen = _vm->_rnd->getRandomNumber(4, kMaxNameLen - 2);
	int pos = 0;

	// 39% chance to start with a vowel.
	bool nextIsVowel = _vm->_rnd->getRandomNumber(1, 100) < 40;

	while (pos < nameLen) {
		bool usedBlend = false;

		if (nextIsVowel) {
			nextIsVowel = false;
			int pairCharIdx = 2 * _vm->_rnd->getRandomNumber(0, kNumVowelPairs - 1);
			char c1 = kVowelPairs[pairCharIdx];
			if (kVowelPairs[pairCharIdx + 1] != ' ') {
				// Digraph vowel (ee, oo, yo, etc.)
				buf[pos] = c1;
				pos += 1;
				c1 = kVowelPairs[pairCharIdx + 1];
			}
			buf[pos] = c1;
			pos += 1;
		} else {
			nextIsVowel = true;
			if (pos <= 1 && 33 < _vm->_rnd->getRandomNumber(1, 100)) {
				// 67% chance: single consonant at the start of the name
				buf[pos] = kSingleConsonants[_vm->_rnd->getRandomNumber(0, kNumSingleConsonants - 1)];
				pos += 1;
			} else {
				// Consonant blend
				int pairCharIdx = 2 * _vm->_rnd->getRandomNumber(0, kNumConsonantBlends - 1);
				buf[pos] = kConsonantBlends[pairCharIdx];
				pos += 1;
				buf[pos] = kConsonantBlends[pairCharIdx + 1];
				pos += 1;
				usedBlend = true;
			}
		}

		// If a blend pushed us to/past the target length, replace the last
		// character with a simple vowel so the name ends softly.
		if (usedBlend && nameLen <= pos) {
			buf[pos - 1] = kSimpleVowels[_vm->_rnd->getRandomNumber(0, kNumSimpleVowels - 1)];
		}

		// If the first two characters are identical, back up and retry.
		if (pos == 2 && buf[0] == buf[1]) {
			pos = 1;
		}
	}

	return Common::U32String(buf, Common::kASCII);
}

Common::U32String ZoombiniText::getKoreanZoombiniName() {
	// Track drawn IDs for one complete cycle, independently of names assigned to saved Zoombinis.
	byte *drawnFlags = _vm->_state->_nameIdDrawnFlags;

	bool allGenerated = true;
	for (int i = 0; i < 625; i++) {
		if (!drawnFlags[i]) {
			allGenerated = false;
			break;
		}
	}
	if (allGenerated)
		memset(drawnFlags, 0, sizeof(_vm->_state->_nameIdDrawnFlags));

	uint16 nameId;
	do {
		nameId = static_cast<uint16>(_vm->_rnd->getRandomNumber(624));
	} while (drawnFlags[nameId]);
	drawnFlags[nameId] = 1;
	return getKoreanZoombiniNameById(nameId);
}

Common::U32String ZoombiniText::getKoreanZoombiniNameById(uint16 nameId) {
	Common::HashMap<int16, Common::U32String>::iterator it = _nameCache.find(nameId);
	if (it != _nameCache.end())
		return it->_value;

	// Each STRL resource stores up to 100 names; 30000-30006 cover IDs 0-624.
	int16 nameResId = static_cast<int16>(nameId / 100 + ZoombiniPage::kSysResStrl30000_ZoombiniNames); // 30000 ~ 30006
	uint nameStrId = static_cast<uint>(nameId % 100);

	Common::Array<Common::U32String> zmbNames;
	if (!getStrl(zmbNames, ZmbResource(ZmbResource::kSystem, nameResId)) || zmbNames.size() <= nameStrId) {
		error("ZoombiniText: cannot get Korean Zoombini name for ID %d", nameId);
		return Common::U32String();
	}

	int16 baseKey = nameId - (nameId % 100);
	for (uint i = 0; i < zmbNames.size(); i++)
		_nameCache[baseKey + i] = zmbNames[i];

	return zmbNames[nameStrId];
}

Common::U32String ZoombiniText::generateJapaneseZoombiniName(JPFixedNameKind jpFixedName) {
	if (jpFixedName == kJPFixedJPNameChobi)
		return Common::U32String(kJapaneseF1DebugName); // "Chobi", a Japanese nickname suggesting "little one".

	if (jpFixedName == kJPFixedJPNameFumiaki)
		return Common::U32String(kJapaneseDebugF1Name); // Likely a kanji-name test using localizer Fumiaki Utsunomiya's name.

	char32_t name[kJapaneseNameMaxTargetLength + 2] = {};
	const int16 targetLength = kJapaneseNameMaxTargetLength - _vm->_rnd->getRandomNumber(0, kJapaneseNameMaxTargetLength - kJapaneseNameMinTargetLength);
	bool previousContractedKana = false;
	bool previousLongVowel = false;
	int pos;
	// The inclusive bound ARRAYSIZE(table) - 2 excludes the terminating NUL from all kana tables except kJapaneseSmallKana.
	for (pos = 0; pos < targetLength; pos++) {
		if (pos == 0) {
			name[pos] = kJapaneseInitialKana[_vm->_rnd->getRandomNumber(0, ARRAYSIZE(kJapaneseInitialKana) - 2)];
			continue;
		}
		const int16 roll = _vm->_rnd->getRandomNumber(0, 100);
		if (90 < roll) {
			name[pos] = kJapaneseSemiVoicedKana[_vm->_rnd->getRandomNumber(0, ARRAYSIZE(kJapaneseSemiVoicedKana) - 2)];
			if (pos < targetLength) {
				pos += 1;
				name[pos] = U'ﾟ';
			}
		} else if (80 < roll) {
			name[pos] = kJapaneseVoicedKana[_vm->_rnd->getRandomNumber(0, ARRAYSIZE(kJapaneseVoicedKana) - 2)];
			if (pos < targetLength) {
				pos += 1;
				name[pos] = U'ﾞ';
			}
		} else if (60 < roll && !previousLongVowel) {
			name[pos] = U'ｰ';
			previousLongVowel = true;
			continue;
		} else if (40 < roll && roll <= 60 && !previousContractedKana) {
			previousContractedKana = true;
			name[pos] = kJapaneseContractedKana[_vm->_rnd->getRandomNumber(0, ARRAYSIZE(kJapaneseContractedKana) - 2)];
			if (pos < targetLength) {
				pos += 1;
				// The inclusive bound ARRAYSIZE(kJapaneseSmallKana) - 1 includes NUL, preserving the original early-termination choice.
				name[pos] = kJapaneseSmallKana[_vm->_rnd->getRandomNumber(0, ARRAYSIZE(kJapaneseSmallKana) - 1)];
			}
			continue;
		} else {
			name[pos] = kJapaneseNormalKana[_vm->_rnd->getRandomNumber(0, ARRAYSIZE(kJapaneseNormalKana) - 2)];
		}
		previousContractedKana = false;
		previousLongVowel = false;
	}
	name[pos] = 0;
	return Common::U32String(name);
}

Common::U32String ZoombiniText::getPageName(ZoombiniPageType pageType) const {
	for (const PageKeyEntry &entry : kPageKeyTable) {
		if (entry.pageType == pageType)
			return getLocalizedString(entry.key);
	}
	return Common::U32String("Unimplemented", Common::kUtf8);
}

Common::U32String ZoombiniText::getLocalizedString(uint32 textKey) const {
	Common::HashMap<uint32, Common::U32String>::const_iterator it = _strMap.find(textKey);
	if (it != _strMap.end())
		return it->_value;

	return Common::U32String("Unimplemented", Common::kUtf8);
}

void ZoombiniText::getLocalizedStrings(Common::Array<LocalizedString> &strings) const {
	strings.clear();
	for (Common::HashMap<uint32, Common::U32String>::const_iterator it = _strMap.begin(); it != _strMap.end(); it++)
		strings.push_back(LocalizedString(it->_key, it->_value));
	Common::sort(strings.begin(), strings.end(), compareLocalizedStrings);
}

void ZoombiniText::getLocalizedCredits(Common::Array<CreditParagraph> &paragraphs) const {
	paragraphs = _creditParagraphs;
}

ZoombiniText::StringOverrideMap ZoombiniText::createStringOverrides() {
	StringOverrideMap overrides;
	overrides[kSvmDialogBodyLoadNewerSaveFormat][Common::EN_USA] = StringOverride(
		U"loading a savefile created\rfrom a newer version.\rSome progress may be lost.\rContinue?");
	overrides[kSvmDialogBodyLoadNewerSaveFormat][Common::KO_KOR] = StringOverride(
		U"더 최신 버전에서 저장한 게임을\r불러오면 일부 진행 상황이\r손실될 수 있습니다.\r계속하시겠습니까?");

	overrides[kSvmDialogBodyLoadOtherPlatformSave][Common::EN_USA] = StringOverride(
		U"loading a savefile created\rfrom other platform.\rSome progress may be lost.\rContinue?");
	overrides[kSvmDialogBodyLoadOtherPlatformSave][Common::KO_KOR] = StringOverride(
		U"다른 플랫폼에서 저장한 게임을\r불러오면 일부 진행 상황이\r손실될 수 있습니다.\r계속하시겠습니까?");

	overrides[kSvmDialogBodyCannotLoadSave][Common::EN_USA] = StringOverride(
		U"cannot load this game:\rthe saved game is damaged\ror incompatible.");
	overrides[kSvmDialogBodyCannotLoadSave][Common::KO_KOR] = StringOverride(
		U"저장된 게임이 손상되었거나\r이 버전과 호환되지 않아\r불러올 수 없습니다.");

	overrides[kSvmDialogBodyAskSaveAfterDebugStateMutation][Common::EN_USA] = StringOverride(
		U"a debug command modified\rthe game state. Saving\rmay corrupt this savefile.\rDo you want to save anyway?");
	overrides[kSvmDialogBodyAskSaveAfterDebugStateMutation][Common::KO_KOR] = StringOverride(
		U"디버그 명령으로 게임 상태를\r수정하여 저장 파일이 손상될\r우려가 있습니다.\r그래도 저장하시겠습니까?");

	overrides[kSvmDialogBodyCannotSaveNameEncoding][Common::EN_USA] = StringOverride(
		U"cannot save this game:\rthe save name contains\rcharacters unsupported\rby the active code page.");
	overrides[kSvmDialogBodyCannotSaveNameEncoding][Common::KO_KOR] = StringOverride(
		U"저장파일 이름에 활성 코드\r페이지로 표현할 수 없는 문자가\r있어 게임을 저장할 수 없습니다.");

	overrides[kSvmDialogBodyAskRegenerateZoombiniNames][Common::EN_USA] = StringOverride(
		U"some Zoombini names cannot\rbe read in this version.\rGenerate new names for\rall Zoombinis?");
	overrides[kSvmDialogBodyAskRegenerateZoombiniNames][Common::KO_KOR] = StringOverride(
		U"일부 줌비니 이름을 이 버전에서\r읽을 수 없습니다.\r모든 줌비니 이름을\r새로 생성하시겠습니까?");

	overrides[kDialogBodyRemoveGame][Common::EN_USA] = StringOverride(
		U"are you sure you want to\rremove this game?", true);
	overrides[kDialogBodyRemoveGame][Common::KO_KOR] = StringOverride(
		U"이 저장된 게임을\r삭제하시겠습니까?", true);
	return overrides;
}

void ZoombiniText::applyStringOverrides(const ZmbReleaseInfo &relInfo) {
	static const StringOverrideMap overrides = createStringOverrides();
	const Common::Language language = relInfo.getLanguage();
	const bool tlc = relInfo.isFamily(ZmbReleaseInfo::Family::kTlcV20);
	for (StringOverrideMap::const_iterator entry = overrides.begin(); entry != overrides.end(); entry++) {
		const LanguageStringOverrides &translations = entry->_value;
		LanguageStringOverrides::const_iterator text = translations.find(language);
		if (text == translations.end() || (text->_value.preserveTlcText && tlc))
			text = translations.find(Common::EN_USA);
		if (text == translations.end() || (text->_value.preserveTlcText && tlc))
			continue;
		_strMap[entry->_key] = text->_value.text;
	}
}

} // End of namespace Mohawk
