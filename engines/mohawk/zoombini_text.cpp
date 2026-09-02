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

constexpr const char *const ZoombiniText::kExeCreditParagraphsAnchor;
constexpr const char *const ZoombiniText::kExeSpanishCreditParagraphsAnchor;
constexpr const char *const ZoombiniText::kExePortugueseCreditParagraphsAnchor;
const ZoombiniText::CreditLimits ZoombiniText::kV10ESCreditLimits = {254, 31, 2464};
const ZoombiniText::CreditLimits ZoombiniText::kV10PTCreditLimits = {267, 32, 2622};
const ZoombiniText::CreditLimits ZoombiniText::kV11USCreditLimits = {248, 27, 2355};
const ZoombiniText::CreditLimits ZoombiniText::kV111KRCreditLimits = {258, 35, 2541};
const ZoombiniText::CreditLimits ZoombiniText::kV20USCreditLimits = {360, 49, 4105};
const ZoombiniText::CreditLimits ZoombiniText::kV20PLCreditLimits = {360, 63, 4300};
constexpr uint32 ZoombiniText::kV10ITCreditEntryCount;

ZoombiniText::ExeTextSource ZoombiniText::ExeTextSource::fromFile(const char *sourceFileName, uint32 sourceExpectedSize) {
	ZoombiniText::ExeTextSource source;
	source.fileName = sourceFileName;
	source.expectedSize = sourceExpectedSize;
	return source;
}

ZoombiniText::ExeTextSource ZoombiniText::ExeTextSource::fromArchiveMember(const char *sourceArchiveName, const char *sourceArchiveMemberName, uint32 sourceExpectedSize) {
	ZoombiniText::ExeTextSource source;
	source.archiveName = sourceArchiveName;
	source.archiveMemberName = sourceArchiveMemberName;
	source.expectedSize = sourceExpectedSize;
	return source;
}

ZoombiniText::ExeTextSource ZoombiniText::ExeTextSource::fromCabinetMember(const char *sourceArchiveName, const char *sourceArchiveMemberName, uint32 sourceExpectedSize) {
	ZoombiniText::ExeTextSource source = fromArchiveMember(sourceArchiveName, sourceArchiveMemberName, sourceExpectedSize);
	source.archiveIsCabinet = true;
	return source;
}

template<size_t size>
ZoombiniText::ExeTextSource ZoombiniText::ExeTextSource::withTextTable(Common::CodePage sourceCodePage, const ZoombiniText::ExeTextEntry (&sourceEntries)[size]) {
	codePage = sourceCodePage;
	entries = sourceEntries;
	entryCount = size;
	return *this;
}

ZoombiniText::ExeTextSource ZoombiniText::ExeTextSource::withRequiredBytes(uint32 sourceRequiredBytesOffset, const char *sourceRequiredBytes) {
	requiredBytesOffset = sourceRequiredBytesOffset;
	requiredBytes = sourceRequiredBytes;
	return *this;
}

ZoombiniText::ExeTextSource ZoombiniText::ExeTextSource::withTextPatches(ZoombiniText::ExeTextPatchFunction sourceTextPatch) {
	textPatches = sourceTextPatch;
	return *this;
}

ZoombiniText::ExeTextSource ZoombiniText::ExeTextSource::withCreditLinePatches(ZoombiniText::ExeTextPatchFunction sourceCreditLinePatch) {
	creditLinePatches = sourceCreditLinePatch;
	return *this;
}

ZoombiniText::ExeTextSource ZoombiniText::ExeTextSource::withMacRomanTrademarkByteAAEscape() {
	escapeMacRomanTrademarkByteAA = true;
	return *this;
}

ZoombiniText::ExeTextSource ZoombiniText::ExeTextSource::withCreditAnchor(const char *sourceCreditAnchor) {
	creditSourceKind = CreditSourceKind::kExecutableAnchorSequence;
	creditAnchor = sourceCreditAnchor;
	return *this;
}

ZoombiniText::ExeTextSource ZoombiniText::ExeTextSource::withCreditPointerTable(uint32 sourceCreditPointerTableOffset, uint32 sourceCreditPointerBaseAddress,
																				uint32 sourceCreditPointerFirstIndex) {
	creditSourceKind = CreditSourceKind::kExecutablePointerStream;
	creditPointerTableOffset = sourceCreditPointerTableOffset;
	creditPointerBaseAddress = sourceCreditPointerBaseAddress;
	creditPointerFirstIndex = sourceCreditPointerFirstIndex;
	return *this;
}

ZoombiniText::ExeTextSource ZoombiniText::ExeTextSource::withCreditPointerBlankAddress(uint32 sourceCreditPointerBlankAddress) {
	creditPointerBlankAddressFirst = sourceCreditPointerBlankAddress;
	creditPointerBlankAddressLast = sourceCreditPointerBlankAddress;
	return *this;
}

ZoombiniText::ExeTextSource ZoombiniText::ExeTextSource::withCreditPointerBlankAddressRange(uint32 sourceCreditPointerBlankAddressFirst,
																							uint32 sourceCreditPointerBlankAddressLast) {
	creditPointerBlankAddressFirst = sourceCreditPointerBlankAddressFirst;
	creditPointerBlankAddressLast = sourceCreditPointerBlankAddressLast;
	return *this;
}

ZoombiniText::ExeTextSource ZoombiniText::ExeTextSource::withCreditLimits(const ZoombiniText::CreditLimits &sourceCreditLimits) {
	creditLimits = sourceCreditLimits;
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
	const uint paragraphIndex = 36;
	const uint lineIndex = 19;
	if (_creditParagraphs.size() <= paragraphIndex || _creditParagraphs[paragraphIndex]._lines.size() <= lineIndex)
		return;

	// One Korean programmer line omitted the closing parenthesis.
	_creditParagraphs[paragraphIndex]._lines[lineIndex] = U"이인선(프로그래머)";
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
	{ZoombiniText::kEuropeJourneyStatZoombiniIsle, 2002, 0},
	{ZoombiniText::kEuropeJourneyStatShelterRock, 2002, 1},
	{ZoombiniText::kEuropeJourneyStatShadeTree, 2002, 2},
	{ZoombiniText::kEuropeJourneyStatZoombiniton, 2002, 3},
	{ZoombiniText::kPracticeMode, 2002, 4},
	// Four progress-dependent return-path blocks. (Only in europe releases)
	{ZoombiniText::kEuropePracticeReturnInitial1, 2002, 5},
	{ZoombiniText::kEuropePracticeReturnInitial2, 2002, 6},
	{ZoombiniText::kEuropePracticeReturnInitial3, 2002, 7},
	{ZoombiniText::kEuropePracticeReturnInitial4, 2002, 8},
	{ZoombiniText::kEuropePracticeReturnShelterRock1, 2002, 9},
	{ZoombiniText::kEuropePracticeReturnShelterRock2, 2002, 10},
	{ZoombiniText::kEuropePracticeReturnShelterRock3, 2002, 11},
	{ZoombiniText::kEuropePracticeReturnShelterRock4, 2002, 12},
	{ZoombiniText::kEuropePracticeReturnShadeTree1, 2002, 13},
	{ZoombiniText::kEuropePracticeReturnShadeTree2, 2002, 14},
	{ZoombiniText::kEuropePracticeReturnShadeTree3, 2002, 15},
	{ZoombiniText::kEuropePracticeReturnShadeTree4, 2002, 16},
	{ZoombiniText::kEuropePracticeReturnZoombiniton1, 2002, 17},
	{ZoombiniText::kEuropePracticeReturnZoombiniton2, 2002, 18},
	{ZoombiniText::kEuropePracticeReturnZoombiniton3, 2002, 19},
	{ZoombiniText::kEuropePracticeReturnZoombiniton4, 2002, 20},
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
	{ZoombiniText::kDialogHelpPickerUpdateVersion, 0, 0}, // NO-OP
	{ZoombiniText::kNewGame, 0xD6840, 8},
};

const ZoombiniText::ExeTextEntry ZoombiniText::kV11BR_NETextEntries[] = {
	{ZoombiniText::kNewGame, 883892, 8},
};

const ZoombiniText::ExeTextEntry ZoombiniText::kV10FR_NETextEntries[] = {
	{ZoombiniText::kDialogHelpPickerUpdateVersion, 0, 0}, // NO-OP
	{ZoombiniText::kNewGame, 0xD6849, 15},
};

const ZoombiniText::ExeTextEntry ZoombiniText::kV11FR_NETextEntries[] = {
	{ZoombiniText::kNewGame, 883901, 15},
};

const ZoombiniText::ExeTextEntry ZoombiniText::kV10GE_NETextEntries[] = {
	{ZoombiniText::kDialogHelpPickerUpdateVersion, 0, 0}, // NO-OP
	{ZoombiniText::kNewGame, 0xD6859, 11},
};

const ZoombiniText::ExeTextEntry ZoombiniText::kV11GE_NETextEntries[] = {
	{ZoombiniText::kNewGame, 883917, 11},
};

// Key-address-size map for the Italian v1.0 NE executable.
const ZoombiniText::ExeTextEntry ZoombiniText::kV10IT_NETextEntries[] = {
	{ZoombiniText::kDialogHelpPickerUpdateVersion, 0, 0}, // NO-OP
	{ZoombiniText::kNewGame, 0xD78DE, 13},
};

const ZoombiniText::ExeTextEntry ZoombiniText::kV10ES_PETextEntries[] = {
	{ZoombiniText::kDialogHelpPickerUpdateVersion, 0, 0}, // NO-OP
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
	{ZoombiniText::kEuropeJourneyStatZoombiniIsle, 0x89ABD, 13},
	{ZoombiniText::kEuropeJourneyStatShelterRock, 0x89ACB, 12},
	{ZoombiniText::kEuropeJourneyStatShadeTree, 0x89AD8, 13},
	{ZoombiniText::kEuropeJourneyStatZoombiniton, 0x89AE6, 14},
	{ZoombiniText::kPracticeMode, 0x89AF5, 9},
	{ZoombiniText::kEuropePracticeReturnInitial1, 0x89AFF, 16},
	{ZoombiniText::kEuropePracticeReturnInitial2, 0x89B10, 18},
	{ZoombiniText::kEuropePracticeReturnInitial3, 0x89B23, 1},
	{ZoombiniText::kEuropePracticeReturnInitial4, 0x89B25, 1},
	{ZoombiniText::kEuropePracticeReturnShelterRock1, 0x89B27, 16},
	{ZoombiniText::kEuropePracticeReturnShelterRock2, 0x89B38, 18},
	{ZoombiniText::kEuropePracticeReturnShelterRock3, 0x89B4B, 14},
	{ZoombiniText::kEuropePracticeReturnShelterRock4, 0x89B5A, 1},
	{ZoombiniText::kEuropePracticeReturnShadeTree1, 0x89B5C, 16},
	{ZoombiniText::kEuropePracticeReturnShadeTree2, 0x89B6D, 19},
	{ZoombiniText::kEuropePracticeReturnShadeTree3, 0x89B81, 12},
	{ZoombiniText::kEuropePracticeReturnShadeTree4, 0x89B8E, 13},
	{ZoombiniText::kEuropePracticeReturnZoombiniton1, 0x89B9C, 16},
	{ZoombiniText::kEuropePracticeReturnZoombiniton2, 0x89BAD, 19},
	{ZoombiniText::kEuropePracticeReturnZoombiniton3, 0x89BC1, 25},
	{ZoombiniText::kEuropePracticeReturnZoombiniton4, 0x89BDB, 16},
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
	{ZoombiniText::kEuropeJourneyStatZoombiniIsle, 0x89B95, 20},
	{ZoombiniText::kEuropeJourneyStatShelterRock, 0x89BAA, 29},
	{ZoombiniText::kEuropeJourneyStatShadeTree, 0x89BC8, 29},
	{ZoombiniText::kEuropeJourneyStatZoombiniton, 0x89BE6, 12},
	{ZoombiniText::kPracticeMode, 0x89BF3, 11},
	{ZoombiniText::kEuropePracticeReturnInitial1, 0x89BFF, 20},
	{ZoombiniText::kEuropePracticeReturnInitial2, 0x89C14, 28},
	{ZoombiniText::kEuropePracticeReturnInitial3, 0x89C31, 1},
	{ZoombiniText::kEuropePracticeReturnInitial4, 0x89C33, 1},
	{ZoombiniText::kEuropePracticeReturnShelterRock1, 0x89C35, 20},
	{ZoombiniText::kEuropePracticeReturnShelterRock2, 0x89C4A, 28},
	{ZoombiniText::kEuropePracticeReturnShelterRock3, 0x89C67, 37},
	{ZoombiniText::kEuropePracticeReturnShelterRock4, 0x89C8D, 1},
	{ZoombiniText::kEuropePracticeReturnShadeTree1, 0x89C8F, 20},
	{ZoombiniText::kEuropePracticeReturnShadeTree2, 0x89CA4, 29},
	{ZoombiniText::kEuropePracticeReturnShadeTree3, 0x89CC2, 38},
	{ZoombiniText::kEuropePracticeReturnShadeTree4, 0x89CE9, 34},
	{ZoombiniText::kEuropePracticeReturnZoombiniton1, 0x89D0C, 20},
	{ZoombiniText::kEuropePracticeReturnZoombiniton2, 0x89D21, 29},
	{ZoombiniText::kEuropePracticeReturnZoombiniton3, 0x89D3F, 25},
	{ZoombiniText::kEuropePracticeReturnZoombiniton4, 0x89D59, 22},
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

bool ZoombiniText::readTextStrString(MohawkArchive &archive, const ResTextEntry &entry, Common::CodePage codePage, Common::U32String &text) {
	Common::Array<Common::U32String> strings;
	if (!readTextStrStrings(archive, entry.resourceId, codePage, strings) || strings.size() <= entry.stringIndex)
		return false;
	text = strings[entry.stringIndex];
	return true;
}

bool ZoombiniText::readTextStrStrings(MohawkArchive &archive, int16 resourceId, Common::CodePage codePage, Common::Array<Common::U32String> &texts) {
	texts.clear();
	if (!archive.hasResource(ID_STRL, resourceId))
		return false;

	Common::SeekableReadStream *stringStream = archive.getResource(ID_STRL, resourceId);
	if (!stringStream)
		return false;
	if (!ZmbResource::hasBytes(stringStream, 1)) {
		delete stringStream;
		return false;
	}

	const byte stringCount = stringStream->readByte();
	Common::Array<Common::U32String> parsedTexts;
	parsedTexts.reserve(stringCount);
	for (uint16 stringIndex = 0; stringIndex < stringCount; stringIndex += 1) {
		Common::String str;
		if (!readBoundedStrlString(stringStream, str, stringCount - stringIndex)) {
			delete stringStream;
			return false;
		}
		parsedTexts.push_back(str.decode(codePage));
	}

	bool consumedExactly = stringStream->pos() == stringStream->size();
	const bool allowBrazilianCreditTerminator = _vm->isVersionFamilyEuV1() && _lang == Common::PT_BRA && resourceId == 2022 && stringCount == 142;
	if (!consumedExactly && allowBrazilianCreditTerminator && ZmbResource::hasBytes(stringStream, 4)) {
		const bool hasCreditTerminatorSuffix = stringStream->readByte() == 0 && stringStream->readByte() == 0 && stringStream->readByte() == '*' &&
											   stringStream->readByte() == 0 && stringStream->pos() == stringStream->size();
		consumedExactly = hasCreditTerminatorSuffix;
	}
	delete stringStream;
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
	if (source.codePage == Common::kWindows949) {
		uint32 extendedByteCount = 0;
		for (uint32 byteIndex = 0; byteIndex < length; byteIndex++) {
			if (0x80 <= bytes[byteIndex])
				extendedByteCount += 1;
		}

		// If there is just one extended byte, the text is likely Windows-1252.
		// Example: byte 0xD8 in the Broderbund QA credit line.
		if (extendedByteCount <= 1)
			decodeCodePage = Common::kWindows1252;
	}

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
		// Z1-20U stores TouchSense's U+2122 as byte 0xAA. The source is otherwise
		// Windows-1252, where 0xAA decodes as U+00AA, so keep this as a narrow
		// source-specific escape instead of decoding all v2.0 text as MacRoman.
		for (uint32 byteIndex = 0; byteIndex < length && byteIndex < text.size(); byteIndex++) {
			if (bytes[byteIndex] == 0xAA)
				text.setChar(0x2122, byteIndex);
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
	uint32 nextIconIndex = 1;
	uint32 pointerIndex = source.creditPointerFirstIndex;
	uint32 totalByteCount = 0;
	for (uint32 entryCount = 0; entryCount < source.creditLimits.entryCount; entryCount++) {
		const uint32 tableEntryOffset = source.creditPointerTableOffset + pointerIndex * 4;
		if (data.size() < tableEntryOffset + 4)
			return false;

		const uint32 stringAddress = READ_LE_UINT32(data.data() + tableEntryOffset);
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
			if (nextIconIndex != 5 || creditStrings.empty())
				return false;
			creditStrings.push_back(text);
			return true;
		}
		if (isTlcCreditIconMarker(text)) {
			const uint32 iconIndex = static_cast<uint32>(text[4] - U'0');
			if (iconIndex != nextIconIndex)
				return false;
			if (!reachedIcons) {
				while (!creditStrings.empty() && creditStrings.back().empty())
					creditStrings.pop_back();
				reachedIcons = true;
			}
			nextIconIndex += 1;
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

bool ZoombiniText::buildCreditParagraphsFromStrings(const Common::Array<Common::U32String> &creditStrings,
													Common::Array<CreditParagraph> &creditParagraphs,
													CreditSourceKind sourceKind, uint32 entryCountLimit) {
	creditParagraphs.clear();
	if (sourceKind == CreditSourceKind::kNone || entryCountLimit == 0 || entryCountLimit < creditStrings.size())
		return false;

	Common::Array<Common::U32String> lines;
	uint32 blankLineCount = 0;
	bool firstLineIsTitle = true;
	bool reachedTerminator = false;
	const bool leadingSpaceMarksBody = sourceKind == CreditSourceKind::kExecutablePointerStream;

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

bool ZoombiniText::applyCreditParagraphSplit(Common::Array<CreditParagraph> &creditParagraphs, const CreditLineAddress &address, uint32 newParagraphBlankLineCount) {
	if (!address.isValid())
		return false;

	const uint32 paragraphIndex = static_cast<uint32>(address.groupIndex);
	const uint32 lineIndex = static_cast<uint32>(address.inGroupLineIndex);
	if (creditParagraphs.size() <= paragraphIndex)
		return false;

	CreditParagraph &paragraph = creditParagraphs[paragraphIndex];
	if (lineIndex == 0 || paragraph._lines.size() <= lineIndex)
		return false;

	Common::Array<Common::U32String> retainedLines;
	retainedLines.reserve(lineIndex);
	for (uint32 currentLineIndex = 0; currentLineIndex < lineIndex; currentLineIndex++)
		retainedLines.push_back(paragraph._lines[currentLineIndex]);

	Common::Array<Common::U32String> movedLines;
	movedLines.reserve(paragraph._lines.size() - lineIndex);
	for (uint32 currentLineIndex = lineIndex; currentLineIndex < paragraph._lines.size(); currentLineIndex++)
		movedLines.push_back(paragraph._lines[currentLineIndex]);

	paragraph._lines = retainedLines;
	creditParagraphs.insert_at(paragraphIndex + 1, CreditParagraph(movedLines, newParagraphBlankLineCount));
	return true;
}

bool ZoombiniText::parseUnsignedDecimalString(const Common::String &text, uint32 &value) {
	if (text.empty())
		return false;

	uint32 parsedValue = 0;
	for (uint charIndex = 0; charIndex < text.size(); charIndex++) {
		const char ch = text[charIndex];
		if (ch < '0' || '9' < ch)
			return false;

		const uint32 digit = static_cast<uint32>(ch - '0');
		if ((0xFFFFFFFFu - digit) / 10 < parsedValue)
			return false;
		parsedValue = parsedValue * 10 + digit;
	}

	value = parsedValue;
	return true;
}

bool ZoombiniText::initOriginalEuropeArchiveStrings() {
	if (!_vm->isVersionFamilyEuV1())
		return false;

	const Common::Path textStrMhkPath = Common::Path(_vm->getArchiveRoot()).append(ZMB_MHK_TEXTSTR);
	MohawkArchive textStrArchive;
	if (!textStrArchive.openFile(textStrMhkPath))
		return false;

	Common::Array<Common::U32String> archiveStrings;
	archiveStrings.reserve(ARRAYSIZE(kV10EU_ResTextEntries));
	for (const ResTextEntry &entry : kV10EU_ResTextEntries) {
		Common::U32String text;
		if (!readTextStrString(textStrArchive, entry, _resCodePage, text)) {
			warning("ZoombiniText: failed to load Europe TEXTSTR.MHK STRL %d string %u", entry.resourceId, entry.stringIndex);
			return false;
		}

		archiveStrings.push_back(text);
	}

	Common::Array<CreditParagraph> archiveCredits;
	bool hasArchiveCredits = false;
	// The Italian v1.0 NE executable loads its credit STRL resources during
	// startup; the executable itself contains no Italian credit string block.
	if (_vm->isGameVariant(MohawkGameFeatures::GF_ZMB_10_EU) && _lang == Common::IT_ITA) {
		const int16 creditResourceIds[] = {2020, 2021};
		Common::Array<Common::U32String> creditStrings;
		for (uint resourceIndex = 0; resourceIndex < ARRAYSIZE(creditResourceIds); resourceIndex += 1) {
			Common::Array<Common::U32String> resourceStrings;
			if (!readTextStrStrings(textStrArchive, creditResourceIds[resourceIndex], _resCodePage, resourceStrings)) {
				warning("ZoombiniText: failed to load Italian TEXTSTR.MHK credit STRL %d", creditResourceIds[resourceIndex]);
				return false;
			}

			for (const Common::U32String &text : resourceStrings)
				creditStrings.push_back(text);
		}

		if (!buildCreditParagraphsFromStrings(creditStrings, archiveCredits, CreditSourceKind::kTextStrResourceSequence,
											  kV10ITCreditEntryCount)) {
			warning("ZoombiniText: failed to build Italian TEXTSTR.MHK credit paragraphs");
			return false;
		}
		hasArchiveCredits = true;
	}

	for (uint entryIndex = 0; entryIndex < ARRAYSIZE(kV10EU_ResTextEntries); entryIndex++) {
		const ResTextEntry &entry = kV10EU_ResTextEntries[entryIndex];
		// The executable's New Game label names an unsaved game; the STRL value
		// with the same key is the startup dialog button and has its own key.
		if (entry.key != kNewGame)
			_strMap[entry.key] = archiveStrings[entryIndex];
	}
	if (hasArchiveCredits)
		_creditParagraphs = archiveCredits;

	return true;
}

Common::CodePage ZoombiniText::getExeCodePage(Common::Language language) {
	switch (language) {
	case Common::KO_KOR:
		return Common::kWindows949;
	case Common::PL_POL:
		return Common::kWindows1250;
	default:
		return Common::kWindows1252;
	}
}

const char *ZoombiniText::getCodePageName(Common::CodePage codePage) {
	// FIXME: Move it to the ScummVM core?
	switch (codePage) {
	case Common::kWindows949:
		return "CP949";
	case Common::kWindows1250:
		return "CP1250";
	case Common::kWindows1252:
		return "CP1252";
	case Common::kMacRoman:
		return "MacRoman";
	default:
		return "Unknown";
	}
}

Common::CodePage ZoombiniText::getResCodePage(Common::Language language) {
	switch (language) {
	case Common::EN_GRB:
	case Common::FR_FRA:
	case Common::DE_DEU:
	case Common::IT_ITA:
		// Europe TEXTSTR.MHK strings are encoded as MacRoman.
		return Common::kMacRoman;
	case Common::KO_KOR:
		return Common::kWindows949;
	case Common::PL_POL:
		return Common::kWindows1250;
	case Common::ES_ESP:
	case Common::EN_USA:
	default:
		return Common::kWindows1252;
	}
}

ZoombiniText::ZoombiniText(MohawkEngine_Zoombini *vm, Common::Language lang) : _vm(vm),
																			   _lang(lang),
																			   _exeCodePage(getExeCodePage(lang)),
																			   _resCodePage(getResCodePage(lang)) {
	static constexpr const char *kFontNameCornerstone = "CornerStone";
	static constexpr const char *kFontNameGulimChe = "GulimChe";
	static constexpr const char *kFontNameTahoma = "Tahoma";
	const Common::U32String srcInstTemplate = _("Please provide an installer disk directory containing: %s.");
	Graphics::TTFSizeMode fontSizeMode;
	if (_vm->isVersionFamilyTlcV2() && _lang == Common::PL_POL)
		fontSizeMode = Graphics::kTTFSizeModeCell;
	else
		fontSizeMode = Graphics::kTTFSizeModeCharacter;

#if defined(WIN32)
	Common::String systemFontWarn;
#endif
	Common::String fontSrcInst;

	// Default font sizes for English and European Zoombini.
	_textFontPoint = 13;
	_titleFontPoint = 18;
	_fallbackTTFLoaders.push_back(new ArchiveTTFLoader("LiberationMono-Bold.ttf", "Liberation Mono", false, 0, fontSizeMode));

	// Zoombini bundles its preferred font in InstallShield archives or the install location.
	// - 1.0/1.1 Europe: found in /CORNER.TTF
	// - 1.1 US: found in /ZBARC16.Z, /ZBARC32.Z, or /SETUP/data1.cab
	// - 2.0 TLC: found in /INSTALL/HD/CORNER.TTF
	//   * Polish release contains Tahoma in name of CORNER.TTF.
	// - 2.0 TLC demo: found in /CORNER.TTF
	if (_vm->isVersionFamilyTlcV2()) { // The Learning Company v2.0 release family
		if (_vm->isDemo()) {
			// I18N: %s is one or more paths that the installer directory must contain.
			fontSrcInst = Common::U32String::format(srcInstTemplate, "'/CORNER.TTF'").encode(Common::kUtf8);
			_optimalTTFLoaders.push_back(new FileTTFLoader("CORNER.TTF", kFontNameCornerstone, fontSrcInst, false, 0, fontSizeMode));
		} else if (_lang == Common::EN_USA) {
			// I18N: %s is one or more paths that the installer directory must contain.
			fontSrcInst = Common::U32String::format(srcInstTemplate, "'/INSTALL/HD/CORNER.TTF'").encode(Common::kUtf8);
			_optimalTTFLoaders.push_back(new FileTTFLoader("INSTALL/HD/CORNER.TTF", kFontNameCornerstone, fontSrcInst, false, 0, fontSizeMode));
			_optimalTTFLoaders.push_back(new FileTTFLoader("CORNER.TTF", kFontNameCornerstone, fontSrcInst, false, 0, fontSizeMode));
		} else if (_lang == Common::PL_POL) {
			// Polish Zoombini uses Tahoma, a font distributed with Windows.
			// Strange enough, Tahoma is embedded in CD-ROM named as 'CORNER.TTF'.
			fontSrcInst = Common::U32String::format(srcInstTemplate, "'/INSTALL/HD/CORNER.TTF'").encode(Common::kUtf8);
			_optimalTTFLoaders.push_back(new FileTTFLoader("INSTALL/HD/CORNER.TTF", kFontNameTahoma, fontSrcInst, false, 0, fontSizeMode));
			_optimalTTFLoaders.push_back(new FileTTFLoader("CORNER.TTF", kFontNameTahoma, fontSrcInst, false, 0, fontSizeMode));
			_optimalTTFLoaders.push_back(new FileTTFLoader("tahoma.ttf", kFontNameTahoma, fontSrcInst, false, 0, fontSizeMode));
#if defined(WIN32)
			systemFontWarn = Common::U32String::format(
								 _("ScummVM found required '%s' in the Windows font archive.\n"
								   "This setup is not portable and not recommended.\n\n"
								   "%S"),
								 "tahoma.ttf", fontSrcInst.c_str())
								 .encode(Common::kUtf8);
			_optimalTTFLoaders.push_back(new WinSysTTFLoader("tahoma.ttf", kFontNameTahoma, systemFontWarn, false, 0, fontSizeMode));
#endif
		} else {
			error("ZoombiniText: unsupported language (%s) for The Learning Company v2.0 based release", Common::getLanguageDescription(_lang));
			return;
		}
	} else if (_vm->isVersionFamilyUsV1()) { // Broderbund v1.1 US release
		if (_lang == Common::KO_KOR) {
			// Korean Zoombini used GulimChe font named as 'gulim.ttc'.
			// The font is distributed with Windows and is not included on the game CD.
			// Users have to source the required font themselves!

			// GulimChe combines vector TTF outlines with TBM bitmaps for common sizes.
			// The original engine uses the 12-point TBM bitmap via GDI.
			// FreeType cannot load a TBM bitmaps, so this path cannot be pixel-identical.

			fontSrcInst = Common::U32String::format(srcInstTemplate, "'gulim.ttc'").encode(Common::kUtf8);
			_optimalTTFLoaders.push_back(new FileTTFLoader("gulim.ttc", kFontNameGulimChe, fontSrcInst, true, 1, fontSizeMode));
#if defined(WIN32)
			systemFontWarn = Common::U32String::format(
								 _("ScummVM found required '%s' in the Windows font archive.\n"
								   "This setup is not portable and not recommended.\n\n"
								   "%S"),
								 "gulim.ttc", fontSrcInst.c_str())
								 .encode(Common::kUtf8);
			_optimalTTFLoaders.push_back(new WinSysTTFLoader("gulim.ttc", kFontNameGulimChe, systemFontWarn, true, 1, fontSizeMode));
#endif
			_textFontPoint = 12;
			_titleFontPoint = 18;
			_fallbackTTFLoaders.clear();
			_fallbackTTFLoaders.push_back(new FileTTFLoader("D2CodingBold.ttf", "D2Coding", true, 0, fontSizeMode));
			// Without hinting, quality of Noto Sans KR rendering in low resolution is terrible.
			_fallbackTTFLoaders.push_back(new ArchiveTTFLoader("NotoSansKR-Bold.otf", "Noto Sans KR Bold", Graphics::kTTFRenderModeLight, fontSizeMode));
			// TODO: Unifont support? The font is small while supporting every possible Unicode glyphs.
		} else if (_lang == Common::EN_USA) {
			fontSrcInst = Common::U32String::format(srcInstTemplate, "'/ZBARC32.Z', '/ZBARC16.Z', or '/SETUP/data1.cab'").encode(Common::kUtf8);
			_optimalTTFLoaders.push_back(new ISZTTFLoader("ZBARC32.Z", "CORNER.TTF", kFontNameCornerstone, fontSrcInst, false, 0, fontSizeMode));
			_optimalTTFLoaders.push_back(new ISZTTFLoader("ZBARC16.Z", "CORNER.TTF", kFontNameCornerstone, fontSrcInst, false, 0, fontSizeMode));
			_optimalTTFLoaders.push_back(new ISCabTTFLoader("SETUP/data1.cab", "CORNER.TTF", kFontNameCornerstone, fontSrcInst, false, 0, fontSizeMode));
			_optimalTTFLoaders.push_back(new FileTTFLoader("CORNER.TTF", kFontNameCornerstone, fontSrcInst, false, 0, fontSizeMode));
		} else {
			error("ZoombiniText: unsupported language (%s) for Broderbund v1.1 US-based release", Common::getLanguageDescription(_lang));
			return;
		}
	} else if (_vm->isVersionFamilyEuV1()) { // Europe v1.0, v1.1 releases
		// English releases load CornerStone at 13/18 pt for text/title.
		// French, German, and Italian branches use 13/16 pt for text/title.
		// Portuguese v1.0 uses 10/14 pt for text/title.
		if (_vm->isGameVariant(MohawkGameFeatures::GF_ZMB_10_ESPT) && _lang == Common::PT_PRT) {
			_textFontPoint = 10;
			_titleFontPoint = 14;
		} else {
			switch (_lang) {
			case Common::FR_FRA:
			case Common::DE_DEU:
			case Common::IT_ITA:
				_titleFontPoint = 16;
				break;
			default: // Use default
				break;
			}
		}

		if (_vm->isGameVariant(MohawkGameFeatures::GF_ZMB_11_FR_2002)) { // French 2002 v1.1 release
			fontSrcInst = Common::U32String::format(srcInstTemplate, "'/data1.cab'").encode(Common::kUtf8);
			_optimalTTFLoaders.push_back(new ISCabTTFLoader("data1.cab", "CORNER.TTF", kFontNameCornerstone, fontSrcInst, false, 0, fontSizeMode));
			_optimalTTFLoaders.push_back(new FileTTFLoader("CORNER.TTF", kFontNameCornerstone, fontSrcInst, false, 0, fontSizeMode));
		} else {
			fontSrcInst = Common::U32String::format(srcInstTemplate, "'/CORNER.TTF'").encode(Common::kUtf8);
			_optimalTTFLoaders.push_back(new FileTTFLoader("CORNER.TTF", kFontNameCornerstone, fontSrcInst, false, 0, fontSizeMode));
		}
	} else {
		error("ZoombiniText: unsupported release (%s, %s)", Common::getLanguageDescription(_lang), _vm->_gameDescription->desc.extra);
		return;
	}

	// Check if ScummVM can access required fonts, and print warning message box if they are not found.
	loadFont(_optimalTTFLoaders, _fallbackTTFLoaders, _textFontPoint, true, _textFontCacheName, _textFontUsesAntialiasing);
	loadFont(_optimalTTFLoaders, _fallbackTTFLoaders, _titleFontPoint, false, _titleFontCacheName, _titleFontUsesAntialiasing);

	// Load the release-specific strings embedded in the game executable.
	if (!initOriginalExecutableStrings()) {
		error("ZoombiniText: the original executable is missing, malformed, or does not match this release");
		return;
	}

	// Initialize ScummVM additional string maps
	switch (_lang) {
	case Common::KO_KOR:
		initKoreanStrings();
		break;
	default:
		initEnglishStrings();
		break;
	}

	// The classic v1.0 Europe releases store their UI text in TEXTSTR.MHK.
	// Load mapped strings after the generic fallback maps so archive text wins.
	if (_vm->isVersionFamilyEuV1() && !_vm->isGameVariant(MohawkGameFeatures::GF_ZMB_10_ESPT) && !initOriginalEuropeArchiveStrings()) {
		error("ZoombiniText: required TEXTSTR.MHK is missing or malformed");
		return;
	}

	// Map page type to text
	initPageKeyMap();
}

ZoombiniText::~ZoombiniText() {
	for (TTFLoader *loader : _optimalTTFLoaders)
		delete loader;
	_optimalTTFLoaders.clear();

	for (TTFLoader *loader : _fallbackTTFLoaders)
		delete loader;
	_fallbackTTFLoaders.clear();
}

bool ZoombiniText::initOriginalExecutableStrings() {
	// Keep Common containers automatic. Their runtime construction is not allowed
	// for global or function-static Zoombini data in ScummVM.
	static constexpr uint32 kV10EUExecutableSize = 934416;
	static constexpr uint32 kV10ITExecutableSize = 935952;
	static constexpr uint32 kV10ESExecutableSize = 634400;
	static constexpr uint32 kV10PTExecutableSize = 635424;
	static constexpr uint32 kV11EUExecutableSize = 934928;
	static constexpr uint32 kV11USPEExecutableSize = 642592;
	static constexpr uint32 kV11USNEExecutableSize = 951824;
	static constexpr uint32 kV111KRPEExecutableSize = 647168;
	static constexpr uint32 kV111KRNEExecutableSize = 977920;
	static constexpr uint32 kV20USExecutableSize = 614400;
	static constexpr uint32 kV20PLExecutableSize = 892928;
	static constexpr uint32 kV20USDemoExecutableSize = 610304;
	const ZoombiniText::ExeTextSource *sources = nullptr;
	uint sourceCount = 0;

	if (_vm->isVersionFamilyEuV1()) {
		if (_vm->isGameVariant(MohawkGameFeatures::GF_ZMB_10_EU)) {
			switch (_lang) {
			case Common::EN_GRB: {
				const ZoombiniText::ExeTextSource english10EuropeSources[] = {
					ZoombiniText::ExeTextSource::fromFile("ZOOMBINI.EXE", kV10EUExecutableSize)
						.withTextTable(Common::kWindows1252, kV10BR_NETextEntries)
						.withRequiredBytes(0xD6840, "New Game"),
					ZoombiniText::ExeTextSource::fromFile("ZOOMBINI._EX", kV10EUExecutableSize)
						.withTextTable(Common::kWindows1252, kV10BR_NETextEntries)
						.withRequiredBytes(0xD6840, "New Game")};
				sources = english10EuropeSources;
				sourceCount = ARRAYSIZE(english10EuropeSources);
				break;
			}
			case Common::FR_FRA: {
				const ZoombiniText::ExeTextSource french10EuropeSources[] = {
					ZoombiniText::ExeTextSource::fromFile("ZOOMBINI.EXE", kV10EUExecutableSize)
						.withTextTable(Common::kWindows1252, kV10FR_NETextEntries)
						.withRequiredBytes(0xD6849, "Nouvelle Partie"),
					ZoombiniText::ExeTextSource::fromFile("ZOOMBINI._EX", kV10EUExecutableSize)
						.withTextTable(Common::kWindows1252, kV10FR_NETextEntries)
						.withRequiredBytes(0xD6849, "Nouvelle Partie")};
				sources = french10EuropeSources;
				sourceCount = ARRAYSIZE(french10EuropeSources);
				break;
			}
			case Common::DE_DEU: {
				const ZoombiniText::ExeTextSource german10EuropeSources[] = {
					ZoombiniText::ExeTextSource::fromFile("ZOOMBINI.EXE", kV10EUExecutableSize)
						.withTextTable(Common::kWindows1252, kV10GE_NETextEntries)
						.withRequiredBytes(0xD6859, "Neues Spiel"),
					ZoombiniText::ExeTextSource::fromFile("ZOOMBINI._EX", kV10EUExecutableSize)
						.withTextTable(Common::kWindows1252, kV10GE_NETextEntries)
						.withRequiredBytes(0xD6859, "Neues Spiel")};
				sources = german10EuropeSources;
				sourceCount = ARRAYSIZE(german10EuropeSources);
				break;
			}
			case Common::IT_ITA: {
				const ZoombiniText::ExeTextSource italian10EuropeSources[] = {
					ZoombiniText::ExeTextSource::fromFile("ZOOMBINI.EXE", kV10ITExecutableSize)
						.withTextTable(Common::kWindows1252, kV10IT_NETextEntries)
						.withRequiredBytes(0xD78DE, "Nuova partita"),
					ZoombiniText::ExeTextSource::fromFile("ZOOMBINI._EX", kV10ITExecutableSize)
						.withTextTable(Common::kWindows1252, kV10IT_NETextEntries)
						.withRequiredBytes(0xD78DE, "Nuova partita")};
				sources = italian10EuropeSources;
				sourceCount = ARRAYSIZE(italian10EuropeSources);
				break;
			}
			default:
				return false;
			}
		} else if (_vm->isGameVariant(MohawkGameFeatures::GF_ZMB_10_ESPT)) {
			switch (_lang) {
			case Common::ES_ESP: {
				const ZoombiniText::ExeTextSource spanish10Sources[] = {
					ZoombiniText::ExeTextSource::fromFile("ZOOMBI32._EX", kV10ESExecutableSize)
						.withTextTable(Common::kWindows1252, kV10ES_PETextEntries)
						.withRequiredBytes(0x888EA, "Nueva Partida")
						.withTextPatches(&ZoombiniText::patchSpanish10ExeText)
						.withCreditAnchor(kExeSpanishCreditParagraphsAnchor)
						.withCreditLimits(kV10ESCreditLimits),
					ZoombiniText::ExeTextSource::fromFile("ZOOMBI32.EXE", kV10ESExecutableSize)
						.withTextTable(Common::kWindows1252, kV10ES_PETextEntries)
						.withRequiredBytes(0x888EA, "Nueva Partida")
						.withTextPatches(&ZoombiniText::patchSpanish10ExeText)
						.withCreditAnchor(kExeSpanishCreditParagraphsAnchor)
						.withCreditLimits(kV10ESCreditLimits)};
				sources = spanish10Sources;
				sourceCount = ARRAYSIZE(spanish10Sources);
				break;
			}
			case Common::PT_PRT: {
				const ZoombiniText::ExeTextSource portuguese10Sources[] = {
					ZoombiniText::ExeTextSource::fromArchiveMember("ZBARCHIV.Z", "zoombi32.exe", kV10PTExecutableSize)
						.withTextTable(Common::kWindows1252, kV10PT_PETextEntries)
						.withRequiredBytes(0x888EA, "Novo Jogo")
						.withCreditAnchor(kExePortugueseCreditParagraphsAnchor)
						.withCreditLimits(kV10PTCreditLimits),
					ZoombiniText::ExeTextSource::fromFile("zoombi32.exe", kV10PTExecutableSize)
						.withTextTable(Common::kWindows1252, kV10PT_PETextEntries)
						.withRequiredBytes(0x888EA, "Novo Jogo")
						.withCreditAnchor(kExePortugueseCreditParagraphsAnchor)
						.withCreditLimits(kV10PTCreditLimits)};
				sources = portuguese10Sources;
				sourceCount = ARRAYSIZE(portuguese10Sources);
				break;
			}
			default:
				return false;
			}
		} else if (_vm->isGameVariant(MohawkGameFeatures::GF_ZMB_11_EU)) {
			switch (_lang) {
			case Common::EN_GRB: {
				const ZoombiniText::ExeTextSource english11EuropeSources[] = {
					ZoombiniText::ExeTextSource::fromFile("ZOOMBINI._EX", kV11EUExecutableSize)
						.withTextTable(Common::kWindows1252, kV11BR_NETextEntries)
						.withRequiredBytes(883892, "New Game")};
				sources = english11EuropeSources;
				sourceCount = ARRAYSIZE(english11EuropeSources);
				break;
			}
			case Common::FR_FRA: {
				const ZoombiniText::ExeTextSource french11EuropeSources[] = {
					ZoombiniText::ExeTextSource::fromFile("ZOOMBINI._EX", kV11EUExecutableSize)
						.withTextTable(Common::kWindows1252, kV11FR_NETextEntries)
						.withRequiredBytes(883901, "Nouvelle Partie")};
				sources = french11EuropeSources;
				sourceCount = ARRAYSIZE(french11EuropeSources);
				break;
			}
			case Common::DE_DEU: {
				const ZoombiniText::ExeTextSource german11EuropeSources[] = {
					ZoombiniText::ExeTextSource::fromFile("ZOOMBINI._EX", kV11EUExecutableSize)
						.withTextTable(Common::kWindows1252, kV11GE_NETextEntries)
						.withRequiredBytes(883917, "Neues Spiel")};
				sources = german11EuropeSources;
				sourceCount = ARRAYSIZE(german11EuropeSources);
				break;
			}
			default:
				return false;
			}
		} else if (_vm->isGameVariant(MohawkGameFeatures::GF_ZMB_11_FR_2002)) {
			const ZoombiniText::ExeTextSource french11RereleaseSources[] = {
				ZoombiniText::ExeTextSource::fromCabinetMember("data1.cab", "ZOOMBINI.EXE", kV11EUExecutableSize)
					.withTextTable(Common::kWindows1252, kV11FR_NETextEntries)
					.withRequiredBytes(883901, "Nouvelle Partie")};
			sources = french11RereleaseSources;
			sourceCount = ARRAYSIZE(french11RereleaseSources);
		} else {
			return false;
		}
	} else if (_vm->isVersionFamilyUsV1()) {
		switch (_lang) {
		case Common::EN_USA: {
			const ZoombiniText::ExeTextSource english11Sources[] = {
				ZoombiniText::ExeTextSource::fromArchiveMember("ZBARC32.Z", "Zoombi32.exe", kV11USPEExecutableSize)
					.withTextTable(Common::kWindows1252, kV11US_PETextEntries)
					.withRequiredBytes(0x8B9FC, "NEW GAME")
					.withCreditAnchor(kExeCreditParagraphsAnchor)
					.withCreditLimits(kV11USCreditLimits),
				ZoombiniText::ExeTextSource::fromArchiveMember("ZBARC16.Z", "ZOOMBINI.EXE", kV11USNEExecutableSize)
					.withTextTable(Common::kWindows1252, kV11US_NETextEntries)
					.withRequiredBytes(0xDCBB2, "NEW GAME")
					.withCreditAnchor(kExeCreditParagraphsAnchor)
					.withCreditLimits(kV11USCreditLimits),
				ZoombiniText::ExeTextSource::fromFile("Zoombi32.exe", kV11USPEExecutableSize)
					.withTextTable(Common::kWindows1252, kV11US_PETextEntries)
					.withRequiredBytes(0x8B9FC, "NEW GAME")
					.withCreditAnchor(kExeCreditParagraphsAnchor)
					.withCreditLimits(kV11USCreditLimits),
				ZoombiniText::ExeTextSource::fromFile("ZOOMBINI.EXE", kV11USNEExecutableSize)
					.withTextTable(Common::kWindows1252, kV11US_NETextEntries)
					.withRequiredBytes(0xDCBB2, "NEW GAME")
					.withCreditAnchor(kExeCreditParagraphsAnchor)
					.withCreditLimits(kV11USCreditLimits)};
			sources = english11Sources;
			sourceCount = ARRAYSIZE(english11Sources);
			break;
		}
		case Common::KO_KOR: {
			const ZoombiniText::ExeTextSource korean11Sources[] = {
				ZoombiniText::ExeTextSource::fromFile("SETUP/data1/data32/Zoombi32.exe", kV111KRPEExecutableSize)
					.withTextTable(Common::kWindows949, kV111KR_PETextEntries)
					.withCreditLinePatches(&ZoombiniText::patchKoreanExeCredits)
					.withCreditAnchor(kExeCreditParagraphsAnchor)
					.withCreditLimits(kV111KRCreditLimits),
				ZoombiniText::ExeTextSource::fromFile("SETUP/data1/data16/Zoombini.exe", kV111KRNEExecutableSize)
					.withTextTable(Common::kWindows949, kV111KR_NETextEntries)
					.withCreditLinePatches(&ZoombiniText::patchKoreanExeCredits)
					.withCreditAnchor(kExeCreditParagraphsAnchor)
					.withCreditLimits(kV111KRCreditLimits),
				ZoombiniText::ExeTextSource::fromFile("Zoombi32.exe", kV111KRPEExecutableSize)
					.withTextTable(Common::kWindows949, kV111KR_PETextEntries)
					.withCreditLinePatches(&ZoombiniText::patchKoreanExeCredits)
					.withCreditAnchor(kExeCreditParagraphsAnchor)
					.withCreditLimits(kV111KRCreditLimits),
				ZoombiniText::ExeTextSource::fromFile("Zoombini.exe", kV111KRNEExecutableSize)
					.withTextTable(Common::kWindows949, kV111KR_NETextEntries)
					.withCreditLinePatches(&ZoombiniText::patchKoreanExeCredits)
					.withCreditAnchor(kExeCreditParagraphsAnchor)
					.withCreditLimits(kV111KRCreditLimits)};
			sources = korean11Sources;
			sourceCount = ARRAYSIZE(korean11Sources);
			break;
		}
		default:
			return false;
		}
	} else if (_vm->isVersionFamilyTlcV2()) {
		const ZoombiniText::ExeTextSource polish20RetailSources[] = {
			ZoombiniText::ExeTextSource::fromFile("INSTALL/HD/Zoombinis Logical Journey.exe", kV20PLExecutableSize)
				.withTextTable(Common::kWindows1250, kV20PL_PETextEntries)
				.withRequiredBytes(0xC9AF4, "Bez nazwy")
				.withCreditPointerTable(0xCA190, 0x413000, 21)
				.withCreditPointerBlankAddressRange(0x4DD234, 0x4DD3F4)
				.withCreditLimits(kV20PLCreditLimits)};
		const ZoombiniText::ExeTextSource english20RetailSources[] = {
			ZoombiniText::ExeTextSource::fromFile("INSTALL/HD/Zoombinis Logical Journey.exe", kV20USExecutableSize)
				.withTextTable(Common::kWindows1252, kV20US_PETextEntries)
				.withMacRomanTrademarkByteAAEscape()
				.withCreditPointerTable(0x90080, 0x400000, 21)
				.withCreditPointerBlankAddress(0x4A286C)
				.withCreditLimits(kV20USCreditLimits)};
		const ZoombiniText::ExeTextSource english20DemoSources[] = {
			ZoombiniText::ExeTextSource::fromFile("Zoom.exe", kV20USDemoExecutableSize)
				.withTextTable(Common::kWindows1252, kV20USDemo_PETextEntries)
				.withRequiredBytes(0x909B8, "NEW GAME")
				.withMacRomanTrademarkByteAAEscape()};

		if (_vm->isDemo()) {
			sources = english20DemoSources;
			sourceCount = ARRAYSIZE(english20DemoSources);
		} else if (_lang == Common::PL_POL) {
			sources = polish20RetailSources;
			sourceCount = ARRAYSIZE(polish20RetailSources);
		} else {
			sources = english20RetailSources;
			sourceCount = ARRAYSIZE(english20RetailSources);
		}
	} else {
		return false;
	}

	for (uint sourceIndex = 0; sourceIndex < sourceCount; sourceIndex++) {
		const ZoombiniText::ExeTextSource &source = sources[sourceIndex];
		Common::SeekableReadStream *exeStream = nullptr;

		if (source.fileName) {
			Common::File *file = new Common::File();
			if (file->open(Common::Path(source.fileName)))
				exeStream = file;
			else
				delete file;
		}

		if (!exeStream && source.archiveName) {
			Common::Archive *archive = nullptr;
			if (source.archiveIsCabinet) {
				// TODO: Pass source.expectedSize after the shared InstallShield reader supports caller-provided exact extraction sizes.
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
			continue;

		Common::Array<byte> exeData;
		if (!readExecutableData(exeStream, source.expectedSize, exeData)) {
			delete exeStream;
			continue;
		}

		if (source.requiredBytes && !bytesMatchAt(exeData, source.requiredBytesOffset, source.requiredBytes)) {
			delete exeStream;
			continue;
		}

		Common::Array<Common::U32String> strings;
		strings.reserve(source.entryCount);
		bool loaded = true;

		for (uint entryIndex = 0; entryIndex < source.entryCount; entryIndex++) {
			const ZoombiniText::ExeTextEntry &entry = source.entries[entryIndex];
			const uint32 entryOffset = entry.offset;
			if (exeData.size() < static_cast<uint64>(entryOffset) + entry.length) {
				loaded = false;
				break;
			}

			strings.push_back(decodeExecutableStringBytes(exeData.data() + entryOffset, entry.length, source));
		}

		const bool sourceHasCredits = source.creditSourceKind != CreditSourceKind::kNone;
		Common::Array<CreditParagraph> executableCredits;
		bool creditsLoaded = loaded && sourceHasCredits && loadOriginalExecutableCredits(exeData, source, executableCredits);
		if (loaded && sourceHasCredits && !creditsLoaded)
			loaded = false;

		delete exeStream;

		if (!loaded)
			continue;

		for (uint entryIndex = 0; entryIndex < source.entryCount; entryIndex++)
			_strMap[source.entries[entryIndex].key] = strings[entryIndex];
		if (source.textPatches)
			(this->*source.textPatches)();

		if (creditsLoaded) {
			_creditParagraphs = executableCredits;
			if (source.creditLinePatches)
				(this->*source.creditLinePatches)();
		}

		return true;
	}

	return false;
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

Common::String ZoombiniText::formatCreditLineKey(const CreditLineAddress &address) {
	return Common::String::format("credit-g%02d-%03d", address.groupIndex, address.inGroupLineIndex);
}

Common::String ZoombiniText::formatCreditLineKey(uint32 paragraphIndex, uint32 lineIndex) {
	return formatCreditLineKey(CreditLineAddress(static_cast<int>(paragraphIndex), static_cast<int>(lineIndex)));
}

bool ZoombiniText::parseCreditLineKey(const Common::String &creditKey, CreditLineAddress &address) {
	const Common::String prefix = "credit-g";
	if (creditKey.size() <= prefix.size() || creditKey.find(prefix) != 0)
		return false;

	const size_t separatorIndex = creditKey.find('-', prefix.size());
	if (separatorIndex == Common::String::npos || separatorIndex == prefix.size() || creditKey.size() <= separatorIndex + 1)
		return false;

	uint32 groupIndex = 0;
	uint32 lineIndex = 0;
	if (!parseUnsignedDecimalString(creditKey.substr(prefix.size(), separatorIndex - prefix.size()), groupIndex) ||
		!parseUnsignedDecimalString(creditKey.substr(separatorIndex + 1, creditKey.size() - separatorIndex - 1), lineIndex))
		return false;

	address = CreditLineAddress(static_cast<int>(groupIndex), static_cast<int>(lineIndex));
	return true;
}

bool ZoombiniText::parseCreditLineKey(const Common::String &creditKey, uint32 &paragraphIndex, uint32 &lineIndex) {
	CreditLineAddress address;
	if (!parseCreditLineKey(creditKey, address))
		return false;

	paragraphIndex = static_cast<uint32>(address.groupIndex);
	lineIndex = static_cast<uint32>(address.inGroupLineIndex);
	return true;
}

const Graphics::Font *ZoombiniText::getTextFont() {
	return FontMan.getFontByName(_textFontCacheName);
}

const Graphics::Font *ZoombiniText::getTitleFont() {
	return FontMan.getFontByName(_titleFontCacheName);
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
		return _titleFontUsesAntialiasing;
	case ZoombiniFontUsage::kFontText:
		return _textFontUsesAntialiasing;
	case ZoombiniFontUsage::kFontDebugTitle:
	case ZoombiniFontUsage::kFontDebugText:
	default:
		return false;
	}
}

const Graphics::Font *ZoombiniText::loadFont(const Common::Array<TTFLoader *> &optimalTTFLoaders, const Common::Array<TTFLoader *> &fallbackTTFLoaders, int point, bool showWarnMsgBox, Common::String &cacheName, bool &usesAntialiasing) {
	if (optimalTTFLoaders.empty() || !optimalTTFLoaders.front()) {
		error("ZoombiniText: no primary font loader is configured");
		return nullptr;
	}
	TTFLoader *firstOptimalLoader = optimalTTFLoaders.front();
	for (TTFLoader *loader : optimalTTFLoaders) {
		const Graphics::Font *font = loader->loadFont(point);
		if (!font)
			continue;

		cacheName = loader->getCacheName(point);
		usesAntialiasing = loader->usesAntialiasing();
		return font;
	}

	// Look up fallback fonts when the optimal font is unavailable.
	Common::U32String noFontMsg = Common::U32String::format(_("Zoombini (%s, %s) requires the font '%s' to display text correctly, but ScummVM cannot access it."),
															Common::getLanguageDescription(_lang), _vm->_gameDescription->desc.extra,
															firstOptimalLoader->_filePath.baseName().c_str());
	noFontMsg += U"\n\n";
	// I18N: %s is the file name of the required font.
	noFontMsg += Common::U32String::format(_("Copy the required font '%s' into ScummVM's extras directory, or into the game directory."), firstOptimalLoader->_filePath.baseName().c_str());
	if (_lang == Common::KO_KOR && _vm->isVersionFamilyUsV1()) {
		noFontMsg += U"\n";
		noFontMsg += _("The Korean release requires the GulimChe ('gulim.ttc') font from the Windows font archive.");
	}

	for (TTFLoader *loader : fallbackTTFLoaders) {
		const Graphics::Font *font = loader->loadFont(point);
		if (!font)
			continue;

		if (showWarnMsgBox) {
			const char *srcInst = firstOptimalLoader->_srcInst.c_str();
			if (srcInst[0] == '\0')
				srcInst = loader->_srcInst.c_str();

			Common::U32String fontSourceInstruction = Common::String(srcInst).decode(Common::kUtf8);
			Common::U32String warnMsg = Common::U32String::format(_("%S\nScummVM will use the fallback font '%s' instead. Text layout may be incorrect.\n\n%S"),
																  noFontMsg.c_str(), loader->_filePath.baseName().c_str(), fontSourceInstruction.c_str());
			warning("%s", warnMsg.encode(Common::kUtf8).c_str());
			GUI::MessageDialog dialog(warnMsg);
			dialog.runModal();
		}

		cacheName = loader->getCacheName(point);
		usesAntialiasing = loader->usesAntialiasing();
		return font;
	}

	error("ZoombiniText: failed to load required font\n\n%s\n\n%s", noFontMsg.encode(Common::kUtf8).c_str(), firstOptimalLoader->_srcInst.c_str());
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
	parsedStrs.reserve(subStrCount);
	if (resource._archiveKind == ZmbResource::kSystem && resource._id == ZoombiniPage::kSysResStrl2900_HelpPickerUpdate && subStrCount == 1 &&
		stringStream->size() == 1 && _vm->isVersionFamilyTlcV2() && _lang == Common::PL_POL) {
		// The Polish v2.0 update entry is an authored one-byte placeholder for one empty string.
		parsedStrs.push_back(Common::U32String());
		delete stringStream;
		outStrs = parsedStrs;
		return true;
	}
	for (uint16 i = 0; i < subStrCount; i++) {
		// Read until the null terminator (consumed but not returned)
		Common::String str;
		if (!readBoundedStrlString(stringStream, str, subStrCount - i)) {
			delete stringStream;
			return false;
		}
		const Common::U32String &ustr = toU32String(str, kResString);
		parsedStrs.push_back(ustr);
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

Common::U32String ZoombiniText::getZoombiniName(int16 snoidId) {
	// @p snoidId ranges from 0 through 624.
	if (snoidId < 0 || 625 <= snoidId) {
		error("ZoombiniText: invalid SnoidID(%d); expected a value between 0 and 624", snoidId);
		return Common::U32String();
	}

	Common::HashMap<int16, Common::U32String>::iterator it = _nameCache.find(snoidId);
	if (it != _nameCache.end())
		return it->_value;

	int16 nameResId = static_cast<int16>(snoidId / 100 + ZoombiniPage::kSysResStrl30000_ZoombiniNames); // 30000 ~ 30006
	uint nameStrId = static_cast<uint>(snoidId % 100);

	Common::Array<Common::U32String> zmbNames;
	if (!getStrl(zmbNames, ZmbResource(ZmbResource::kSystem, nameResId)) || zmbNames.size() <= nameStrId) {
		error("ZoombiniText: cannot get name of SnoidID(%d)", snoidId);
		return Common::U32String();
	}

	int16 baseKey = snoidId - (snoidId % 100);
	for (uint i = 0; i < zmbNames.size(); i++)
		_nameCache[baseKey + i] = zmbNames[i];

	return zmbNames[nameStrId];
}

void ZoombiniText::cacheAllZoombiniNames() {
	if (!_nameIndexCache.empty())
		return; // Already built
	for (int16 i = 0; i < 625; i++) {
		Common::U32String name = getZoombiniName(i); // populates _nameCache in batches
		_nameIndexCache[name] = i;
	}
}

int16 ZoombiniText::findZoombiniNameId(const Common::U32String &name) const {
	Common::HashMap<Common::U32String, int16>::const_iterator it = _nameIndexCache.find(name);
	if (it == _nameIndexCache.end())
		return -1;
	return it->_value;
}

void ZoombiniText::clearNameCache() {
	_nameCache.clear();
	_nameIndexCache.clear();
}

/**
 * Procedural Zoombini name generation tables.
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

Common::U32String ZoombiniText::generateRandomName() {
	char buf[kMaxNameLen];
	memset(buf, 0, sizeof(buf));

	// Choose a name length from 4 through 8, bounded by @ref ZoombiniText::kMaxNameLen.
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

Common::U32String ZoombiniText::pickNextZoombiniName() {
	if (!_vm->hasResource(ID_STRL, ZmbResource(ZmbResource::kSystem, ZoombiniPage::kSysResStrl30000_ZoombiniNames)))
		return generateRandomName();

	// Korean: draw from the 625-slot name pool, tracking which names have been used.
	byte *nameTable = _vm->_state->_zoombiniNameGeneratedTable;

	bool allGenerated = true;
	for (int i = 0; i < 625; i++) {
		if (!nameTable[i]) {
			allGenerated = false;
			break;
		}
	}
	if (allGenerated)
		_vm->_state->buildNameGeneratedTable();

	uint16 nameId;
	do {
		nameId = static_cast<uint16>(_vm->_rnd->getRandomNumber(624));
	} while (nameTable[nameId]);
	nameTable[nameId] = 1;

	return getZoombiniName(nameId);
}

Common::U32String ZoombiniText::getPageName(ZoombiniPageType pageType) const {
	Common::HashMap<ZoombiniPageType, Key>::const_iterator it = _pageKeyMap.find(pageType);
	if (it == _pageKeyMap.end())
		return Common::U32String("Unimplemented", Common::kUtf8);
	return getLocalizedString(it->_value);
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

bool ZoombiniText::patchLocalizedText(const Common::String &textKey, const Common::U32String &text) {
	CreditLineAddress address;
	if (parseCreditLineKey(textKey, address))
		return patchCreditLine(static_cast<uint32>(address.groupIndex), static_cast<uint32>(address.inGroupLineIndex), text);

	uint32 numericKey = 0;
	if (!parseUnsignedDecimalString(textKey, numericKey))
		return false;

	patchLocalizedString(numericKey, text);
	return true;
}

bool ZoombiniText::patchLocalizedText(const Common::String &textKey, const char *utf8Text) {
	return patchLocalizedText(textKey, Common::U32String(utf8Text, Common::kUtf8));
}

bool ZoombiniText::patchLocalizedTexts(const Common::HashMap<Common::String, Common::U32String> &patches) {
	bool allPatched = true;
	for (Common::HashMap<Common::String, Common::U32String>::const_iterator it = patches.begin(); it != patches.end(); it++)
		allPatched = patchLocalizedText(it->_key, it->_value) && allPatched;
	return allPatched;
}

bool ZoombiniText::patchLocalizedTexts(const Common::HashMap<Common::String, Common::String> &patches) {
	bool allPatched = true;
	for (Common::HashMap<Common::String, Common::String>::const_iterator it = patches.begin(); it != patches.end(); it++)
		allPatched = patchLocalizedText(it->_key, it->_value.c_str()) && allPatched;
	return allPatched;
}

void ZoombiniText::patchLocalizedString(uint32 textKey, const Common::U32String &text) {
	_strMap[textKey] = text;
}

void ZoombiniText::patchLocalizedString(uint32 textKey, const char *utf8Text) {
	patchLocalizedString(textKey, Common::U32String(utf8Text, Common::kUtf8));
}

bool ZoombiniText::patchCreditLine(uint32 paragraphIndex, uint32 lineIndex, const Common::U32String &text) {
	if (_creditParagraphs.size() <= paragraphIndex || _creditParagraphs[paragraphIndex]._lines.size() <= lineIndex)
		return false;

	_creditParagraphs[paragraphIndex]._lines[lineIndex] = text;
	return true;
}

bool ZoombiniText::patchCreditLine(uint32 paragraphIndex, uint32 lineIndex, const char *utf8Text) {
	return patchCreditLine(paragraphIndex, lineIndex, Common::U32String(utf8Text, Common::kUtf8));
}

bool ZoombiniText::patchCreditLine(const Common::String &creditKey, const Common::U32String &text) {
	CreditLineAddress address;
	if (!parseCreditLineKey(creditKey, address))
		return false;

	return patchCreditLine(static_cast<uint32>(address.groupIndex), static_cast<uint32>(address.inGroupLineIndex), text);
}

bool ZoombiniText::patchCreditLine(const Common::String &creditKey, const char *utf8Text) {
	return patchCreditLine(creditKey, Common::U32String(utf8Text, Common::kUtf8));
}

bool ZoombiniText::splitCreditParagraph(uint32 paragraphIndex, uint32 lineIndex, uint32 newParagraphBlankLineCount) {
	return applyCreditParagraphSplit(_creditParagraphs, CreditLineAddress(static_cast<int>(paragraphIndex), static_cast<int>(lineIndex)), newParagraphBlankLineCount);
}

bool ZoombiniText::splitCreditParagraph(const Common::String &creditKey, uint32 newParagraphBlankLineCount) {
	CreditLineAddress address;
	if (!parseCreditLineKey(creditKey, address))
		return false;

	return applyCreditParagraphSplit(_creditParagraphs, address, newParagraphBlankLineCount);
}

bool ZoombiniText::patchCreditParagraph(uint32 paragraphIndex, const CreditParagraph &paragraph) {
	if (_creditParagraphs.size() <= paragraphIndex)
		return false;

	_creditParagraphs[paragraphIndex] = paragraph;
	return true;
}

void ZoombiniText::patchLocalizedCredits(const Common::Array<CreditParagraph> &paragraphs) {
	_creditParagraphs = paragraphs;
}

void ZoombiniText::initLocalizedCredits() {
	_creditParagraphs.clear();
}

void ZoombiniText::initPageKeyMap() {
	_pageKeyMap[ZoombiniPageType::kPicker] = ZoombiniText::kPicker;
	_pageKeyMap[ZoombiniPageType::kBridge] = ZoombiniText::kBridge;
	_pageKeyMap[ZoombiniPageType::kCaves] = ZoombiniText::kCaves;
	_pageKeyMap[ZoombiniPageType::kPizza] = ZoombiniText::kPizza;
	_pageKeyMap[ZoombiniPageType::kBasecamp1] = ZoombiniText::kBasecamp1;
	_pageKeyMap[ZoombiniPageType::kFerry] = ZoombiniText::kFerry;
	_pageKeyMap[ZoombiniPageType::kLilly] = ZoombiniText::kLilly;
	_pageKeyMap[ZoombiniPageType::kSlides] = ZoombiniText::kSlides;
	_pageKeyMap[ZoombiniPageType::kFleens] = ZoombiniText::kFleens;
	_pageKeyMap[ZoombiniPageType::kHotel] = ZoombiniText::kHotel;
	_pageKeyMap[ZoombiniPageType::kNet] = ZoombiniText::kNet;
	_pageKeyMap[ZoombiniPageType::kBasecamp2] = ZoombiniText::kBasecamp2;
	_pageKeyMap[ZoombiniPageType::kTunnels] = ZoombiniText::kTunnels;
	_pageKeyMap[ZoombiniPageType::kSmoke] = ZoombiniText::kSmoke;
	_pageKeyMap[ZoombiniPageType::kMaze] = ZoombiniText::kMaze;
	_pageKeyMap[ZoombiniPageType::kTown] = ZoombiniText::kTown;
}

void ZoombiniText::initEnglishStrings() {
	_strMap[kSvmDialogBodyLoadNewerSaveFormat] = U"loading a savefile created\rfrom a newer version.\rSome progress may be lost.\rContinue?";
	_strMap[kSvmDialogBodyCannotLoadSave] = U"cannot load this game:\rthe saved game is damaged\ror incompatible.";
	_strMap[kSvmDialogBodyAskSaveAfterDebugStateMutation] = U"a debug command modified\rthe game state. Saving\rmay corrupt this savefile.\rDo you want to save anyway?";
	_strMap[kSvmDialogBodyCannotSaveNameEncoding] = U"cannot save this game:\rthe save name contains\rcharacters unsupported\rby the active code page.";
	_strMap[kSvmDialogBodyAskConvertUnreadableZoombiniNames] = U"the active code page cannot\rread the Zoombini names in\rthis save. Convert them to\rstandard names?";

	if (!_vm->isVersionFamilyTlcV2()) {
		// ScummVM addition for v1.x version
		_strMap[kDialogBodyRemoveGame] = U"are you sure you want to\rremove this game?";
	}
}

void ZoombiniText::initKoreanStrings() {
	_strMap[kSvmDialogBodyLoadNewerSaveFormat] = U"더 최신 버전에서 저장한 게임을\r불러오면 일부 진행 상황이\r손실될 수 있습니다.\r계속하시겠습니까?";
	_strMap[kSvmDialogBodyCannotLoadSave] = U"저장된 게임이 손상되었거나\r이 버전과 호환되지 않아\r불러올 수 없습니다.";
	_strMap[kSvmDialogBodyAskSaveAfterDebugStateMutation] = U"디버그 명령으로 게임 상태를\r수정하여 저장 파일이 손상될\r우려가 있습니다.\r그래도 저장하시겠습니까?";
	_strMap[kSvmDialogBodyCannotSaveNameEncoding] = U"저장파일 이름에 활성 코드\r페이지로 표현할 수 없는 문자가\r있어 게임을 저장할 수 없습니다.";
	_strMap[kSvmDialogBodyAskConvertUnreadableZoombiniNames] = U"활성 코드페이지로 저장 파일 내\r줌비니 이름을 읽을 수 없습니다.\r표준 이름으로 변환하시겠습니까?";

	if (!_vm->isVersionFamilyTlcV2()) {
		// ScummVM addition for v1.x version
		_strMap[kDialogBodyRemoveGame] = U"이 저장된 게임을\r삭제하시겠습니까?";
	}
}

} // End of namespace Mohawk
