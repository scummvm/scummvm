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

#include "common/algorithm.h"
#include "common/array.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/fs.h"
#include "common/language.h"
#include "common/memstream.h"
#include "common/savefile.h"
#include "common/serializer.h"
#include "common/system.h"
#include "common/translation.h"
#include "common/util.h"

#include "gui/browser.h"
#include "gui/message.h"

#include "mohawk/sound.h"

#include "mohawk/zoombini.h"
#include "mohawk/zoombini_page.h"
#include "mohawk/zoombini_pages/interactive_base.h"
#include "mohawk/zoombini_random.h"
#include "mohawk/zoombini_sound.h"
#include "mohawk/zoombini_state.h"
#include "mohawk/zoombini_text.h"

namespace Mohawk {

constexpr byte ZoombiniGameState::kLittleEndianMagicBytes[2];
constexpr byte ZoombiniGameState::kBigEndianMagicBytes[2];
constexpr uint32 ZoombiniGameState::kZmbRosterFileSize;
constexpr uint32 ZmbRosterFile::kSerializedHeaderSize;
constexpr uint32 ZmbRosterFile::kSerializedEntrySize;

Common::Error ZoombiniGameState::getSaveFileWriteError(Common::SaveFileManager *saveFileMan, Common::ErrorCode fallbackErrorCode) {
	if (saveFileMan) {
		const Common::Error managerError = saveFileMan->getError();
		if (managerError.getCode() != Common::kNoError)
			return managerError;
	}
	return Common::Error(fallbackErrorCode);
}

Common::U32String ZoombiniGameState::getSaveFileWriteFailureReason(Common::SaveFileManager *saveFileMan, Common::ErrorCode fallbackErrorCode) {
	const Common::Error writeError = getSaveFileWriteError(saveFileMan, fallbackErrorCode);
	Common::U32String reason = writeError.getTranslatedDesc();
	if (saveFileMan) {
		const Common::String managerDescription = saveFileMan->getErrorDesc();
		if (!managerDescription.empty() && managerDescription != writeError.getDesc()) {
			reason += U"\n";
			reason += Common::U32String(managerDescription);
		}
	}
	return reason;
}

void ZoombiniGameState::showStateFileWriteFailure(Common::SaveFileManager *saveFileMan, const Common::String &filename, Common::ErrorCode fallbackErrorCode) {
	const Common::U32String reason = getSaveFileWriteFailureReason(saveFileMan, fallbackErrorCode);
	// I18N: %s is the saved-game filename. %S is the write failure reason reported by ScummVM.
	const Common::U32String message = Common::U32String::format(
		_("Failed to write the Zoombini saved-game file: '%s'.\n\nReason: %S"), filename.c_str(), reason.c_str());
	GUI::MessageDialog dialog(message);
	dialog.runModal();
}

void ZoombiniGameState::showRosterFileWriteFailure(Common::SaveFileManager *saveFileMan, const Common::String &filename, Common::ErrorCode fallbackErrorCode) {
	const Common::U32String reason = getSaveFileWriteFailureReason(saveFileMan, fallbackErrorCode);
	// I18N: %s is the save roster filename. %S is the write failure reason reported by ScummVM.
	const Common::U32String message = Common::U32String::format(
		_("Failed to write the Zoombini save roster file: '%s'.\n\nReason: %S"), filename.c_str(), reason.c_str());
	GUI::MessageDialog dialog(message);
	dialog.runModal();
}

const ZmbXferRouteInfo ZmbXferRouteInfo::kXferRouteInfos[] = {
	{"bridge", ZmbDestPageKind::kBridge_07, ZmbXferRouteInfo::kXferPageNameAllergicCliffs, ZmbSrcPageKind::kPicker_01, ZmbDestPageKind::kUnk_00, nullptr, ZmbRouteId::kBigBadHungry, ZmbXferRouteInfo::kXferRouteNameFromIsle},
	{"tunnels", ZmbDestPageKind::kTunnels_08, ZmbXferRouteInfo::kXferPageNameStoneColdCaves, ZmbSrcPageKind::kBridge_02, ZmbDestPageKind::kBridge_07, ZmbXferRouteInfo::kXferPageNameAllergicCliffs, ZmbRouteId::kBigBadHungry, ZmbXferRouteInfo::kXferRouteNameBigBadHungry},
	{"pizza", ZmbDestPageKind::kPizza_09, ZmbXferRouteInfo::kXferPageNamePizzaPass, ZmbSrcPageKind::kTunnels_03, ZmbDestPageKind::kTunnels_08, ZmbXferRouteInfo::kXferPageNameStoneColdCaves, ZmbRouteId::kBigBadHungry, ZmbXferRouteInfo::kXferRouteNameBigBadHungry},
	{"bc1", ZmbDestPageKind::kBC1_04, ZmbXferRouteInfo::kXferPageNameShelterRock, ZmbSrcPageKind::kPizza_04, ZmbDestPageKind::kPizza_09, ZmbXferRouteInfo::kXferPageNamePizzaPass, ZmbRouteId::kBigBadHungry, ZmbXferRouteInfo::kXferRouteNameBigBadHungry},
	{"ferry", ZmbDestPageKind::kFerry_10, ZmbXferRouteInfo::kXferPageNameCaptainCajunsFerryboat, ZmbSrcPageKind::kBC1North_05, ZmbDestPageKind::kUnk_00, ZmbXferRouteInfo::kXferPageNameShelterRock, ZmbRouteId::kWhosBayou, ZmbXferRouteInfo::kXferRouteNameWhosBayou},
	{"lilly", ZmbDestPageKind::kLilly_11, ZmbXferRouteInfo::kXferPageNameTitanicTattooedToads, ZmbSrcPageKind::kFerry_07, ZmbDestPageKind::kFerry_10, ZmbXferRouteInfo::kXferPageNameCaptainCajunsFerryboat, ZmbRouteId::kWhosBayou, ZmbXferRouteInfo::kXferRouteNameWhosBayou},
	{"slides", ZmbDestPageKind::kSlides_12, ZmbXferRouteInfo::kXferPageNameStoneRise, ZmbSrcPageKind::kLilly_08, ZmbDestPageKind::kLilly_11, ZmbXferRouteInfo::kXferPageNameTitanicTattooedToads, ZmbRouteId::kWhosBayou, ZmbXferRouteInfo::kXferRouteNameWhosBayou},
	{"bc2north", ZmbDestPageKind::kBC2_05, ZmbXferRouteInfo::kXferPageNameShadeTree, ZmbSrcPageKind::kSlides_09, ZmbDestPageKind::kSlides_12, ZmbXferRouteInfo::kXferPageNameStoneRise, ZmbRouteId::kWhosBayou, ZmbXferRouteInfo::kXferRouteNameWhosBayou},
	{"fleens", ZmbDestPageKind::kFleens_13, ZmbXferRouteInfo::kXferPageNameFleens, ZmbSrcPageKind::kBC1South_06, ZmbDestPageKind::kUnk_00, ZmbXferRouteInfo::kXferPageNameShelterRock, ZmbRouteId::kDeepDarkForest, ZmbXferRouteInfo::kXferRouteNameDeepDarkForest},
	{"hotel", ZmbDestPageKind::kHotel_14, ZmbXferRouteInfo::kXferPageNameHotelDimensia, ZmbSrcPageKind::kFleens_10, ZmbDestPageKind::kFleens_13, ZmbXferRouteInfo::kXferPageNameFleens, ZmbRouteId::kDeepDarkForest, ZmbXferRouteInfo::kXferRouteNameDeepDarkForest},
	{"net", ZmbDestPageKind::kNet_15, ZmbXferRouteInfo::kXferPageNameMudballWall, ZmbSrcPageKind::kHotel_11, ZmbDestPageKind::kHotel_14, ZmbXferRouteInfo::kXferPageNameHotelDimensia, ZmbRouteId::kDeepDarkForest, ZmbXferRouteInfo::kXferRouteNameDeepDarkForest},
	{"bc2south", ZmbDestPageKind::kBC2_05, ZmbXferRouteInfo::kXferPageNameShadeTree, ZmbSrcPageKind::kNet_12, ZmbDestPageKind::kNet_15, ZmbXferRouteInfo::kXferPageNameMudballWall, ZmbRouteId::kDeepDarkForest, ZmbXferRouteInfo::kXferRouteNameDeepDarkForest},
	{"caves", ZmbDestPageKind::kCaves_16, ZmbXferRouteInfo::kXferPageNameLionsLair, ZmbSrcPageKind::kBC2_13, ZmbDestPageKind::kUnk_00, ZmbXferRouteInfo::kXferPageNameShadeTree, ZmbRouteId::kMontDespair, ZmbXferRouteInfo::kXferRouteNameMountainsOfDespair},
	{"smoke", ZmbDestPageKind::kSmoke_17, ZmbXferRouteInfo::kXferPageNameMirrorMachine, ZmbSrcPageKind::kCaves_14, ZmbDestPageKind::kCaves_16, ZmbXferRouteInfo::kXferPageNameLionsLair, ZmbRouteId::kMontDespair, ZmbXferRouteInfo::kXferRouteNameMountainsOfDespair},
	{"maze", ZmbDestPageKind::kMaze_18, ZmbXferRouteInfo::kXferPageNameBubblewonderAbyss, ZmbSrcPageKind::kSmoke_15, ZmbDestPageKind::kSmoke_17, ZmbXferRouteInfo::kXferPageNameMirrorMachine, ZmbRouteId::kMontDespair, ZmbXferRouteInfo::kXferRouteNameMountainsOfDespair},
	{"town", ZmbDestPageKind::kTown_06, ZmbXferRouteInfo::kXferPageNameZoombiniville, ZmbSrcPageKind::kMaze_16, ZmbDestPageKind::kMaze_18, ZmbXferRouteInfo::kXferPageNameBubblewonderAbyss, ZmbRouteId::kMontDespair, ZmbXferRouteInfo::kXferRouteNameToTown},
};

const ZmbXferRouteInfo *ZmbXferRouteInfo::getZmbXferRouteInfos(uint32 &routeCount) {
	routeCount = ARRAYSIZE(kXferRouteInfos);
	return kXferRouteInfos;
}

const ZmbXferRouteInfo *ZmbXferRouteInfo::getZmbXferRouteInfo(ZmbSrcPageKind srcPage) {
	for (uint i = 0; i < ARRAYSIZE(kXferRouteInfos); i++) {
		if (kXferRouteInfos[i].srcPage == srcPage)
			return &kXferRouteInfos[i];
	}

	return nullptr;
}

bool ZmbXferRouteInfo::entersContainer() const {
	return destPage == ZmbDestPageKind::kBC1_04 ||
		   destPage == ZmbDestPageKind::kBC2_05 ||
		   destPage == ZmbDestPageKind::kTown_06;
}

Common::CodePage ZoombiniGameState::getSaveNameCodePage(const Common::String &target) {
	const Common::Language language = Common::parseLanguage(ConfMan.get("language", target));
	return ZoombiniText::getExeCodePage(language);
}

Common::U32String ZoombiniGameState::trimSaveName(const Common::U32String &name) {
	uint32 begin = 0;
	uint32 end = name.size();
	while (begin < end && name[begin] == ' ')
		begin += 1;
	while (begin < end && name[end - 1] == ' ')
		end -= 1;

	Common::U32String trimmed;
	for (uint32 i = begin; i < end; i += 1)
		trimmed += name[i];
	return trimmed;
}

bool ZoombiniGameState::encodeRosterSaveName(const Common::U32String &saveName, Common::CodePage codePage, byte *output, uint32 outputSize) {
	if (saveName.empty())
		return false;

	Common::String encoded;
	if (!ZmbRosterEntry::encodeSaveName(saveName, codePage, encoded) || outputSize <= encoded.size())
		return false;

	memset(output, 0, outputSize);
	memcpy(output, encoded.c_str(), encoded.size());
	return true;
}

bool ZoombiniGameState::renameRosterEntry(const Common::String &target, ZmbRosterFile &roster, int slot, const Common::U32String &saveName) {
	if (!roster.hasEntry(slot) || saveName.empty())
		return false;

	const Common::CodePage codePage = ZoombiniGameState::getSaveNameCodePage(target);
	byte encodedName[23];
	if (!ZoombiniGameState::encodeRosterSaveName(saveName, codePage, encodedName, ARRAYSIZE(encodedName)))
		return false;

	for (int i = 0; i < roster.getEntryCount(); i++) {
		if (i == slot)
			continue;

		const ZmbRosterEntry &existingEntry = roster.getEntry(i);
		const uint16 existingNameLength = existingEntry.getSaveNameLength();
		if (ARRAYSIZE(existingEntry._saveName) <= existingNameLength)
			return false;
		const Common::String encodedExistingName(reinterpret_cast<const char *>(existingEntry._saveName), existingNameLength);
		const Common::U32String existingName(encodedExistingName, codePage);
		if (existingName.equals(saveName))
			return false;
	}

	ZmbRosterEntry &entry = roster.getEntry(slot);
	memcpy(entry._saveName, encodedName, ARRAYSIZE(entry._saveName));
	return true;
}

ZmbStateFile::ZmbStateFile() : _magic006B(ZoombiniGameState::kEndianMagic) {
}

ZmbRosterFile::ZmbRosterFile() : _magic006B(ZoombiniGameState::kEndianMagic) {
}

ZoombiniGameState::ZmbSaveFormat ZoombiniGameState::getSaveFormatFromSize(int32 size) {
	switch (size) {
	case kStateFileSizeBrV10:
		// The 16-bit Europe v1.1 executable retained the v1.0 layout.
		return ZmbSaveFormat::kEuV10;
	case kStateFileSizeUsV11:
		return ZmbSaveFormat::kUsV11;
	case kStateFileSizeTlcV20Small:
		return ZmbSaveFormat::kTlcV20Small;
	case kStateFileSizeTlcV20:
		return ZmbSaveFormat::kTlcV20;
	default:
		return ZmbSaveFormat::kInvalid;
	}
}

const char *ZoombiniGameState::getSaveFormatName(ZmbSaveFormat format) {
	switch (format) {
	case ZmbSaveFormat::kEuV10:
		return "EUv1";
	case ZmbSaveFormat::kUsV11:
		return "USv1";
	case ZmbSaveFormat::kTlcV20Small:
		return "TLCv2s";
	case ZmbSaveFormat::kTlcV20:
		return "TLCv2";
	default:
		return "INVALID";
	}
}

bool ZoombiniGameState::isTlcSaveFormat(ZmbSaveFormat format) {
	return format == ZmbSaveFormat::kTlcV20Small || format == ZmbSaveFormat::kTlcV20;
}

bool ZoombiniGameState::hasCompletionCounters(ZmbSaveFormat format) {
	return format == ZmbSaveFormat::kUsV11 || format == ZmbSaveFormat::kTlcV20;
}

ZoombiniGameState::ZmbSaveFormat ZoombiniGameState::getTargetSaveFormat(const MohawkEngine_Zoombini *vm) {
	if (vm->isVersionFamilyTlcV2())
		return ZmbSaveFormat::kTlcV20;

	// The native 16-bit Europe v1.1 executable keeps the v1.0 save layout.
	// ScummVM still accepts other recognized source layouts when loading.
	if (vm->isVersionFamilyUsV1() && !vm->isVersionFamilyEuV1())
		return ZmbSaveFormat::kUsV11;

	return ZmbSaveFormat::kEuV10;
}

bool ZoombiniGameState::canLoadSaveFormat(ZmbSaveFormat sourceFormat) {
	// Deliberate extension beyond the release executables:
	// normalize every known layout into ZmbStateFile so any supported release can load it.
	// The target release selects its own layout when the state is saved again.
	return sourceFormat != ZmbSaveFormat::kInvalid;
}

int ZoombiniGameState::getSaveFormatMajorVersion(ZmbSaveFormat format) {
	switch (format) {
	case ZmbSaveFormat::kEuV10:
		return 10;
	case ZmbSaveFormat::kUsV11:
		return 11;
	case ZmbSaveFormat::kTlcV20Small:
	case ZmbSaveFormat::kTlcV20:
		return 20;
	default:
		return 0;
	}
}

bool ZoombiniGameState::isNewerThanTargetSaveFormat(ZmbSaveFormat sourceFormat, ZmbSaveFormat targetFormat) {
	return getSaveFormatMajorVersion(targetFormat) < getSaveFormatMajorVersion(sourceFormat);
}

void ZoombiniGameState::initializeCompletionCounters(ZmbStateFile &state) {
	for (int32 i = 0; i < ARRAYSIZE(state._routePerfectCounters); i++)
		state._routePerfectCounters[i] = 0;
	state._townDevelopLevel = 0;
}

void ZoombiniGameState::upgradeStateForVariant(ZmbStateFile &state, ZmbSaveFormat sourceFormat, ZmbSaveFormat targetFormat) {
	if (!hasCompletionCounters(sourceFormat))
		initializeCompletionCounters(state);

	const bool sourceIsTlc = isTlcSaveFormat(sourceFormat);
	const bool targetIsTlc = isTlcSaveFormat(targetFormat);
	if (sourceIsTlc == targetIsTlc)
		return;

	if (!targetIsTlc) {
		state.setV1TransitionsDisabled(state.getV2TransitionsDisabled());
		for (int32 i = 0; i < ARRAYSIZE(state._v1FleensTraitValueRotations); i++)
			state._v1FleensTraitValueRotations[i] = state._v2FleensTraitValueRotations[i];
		for (int32 i = 0; i < ARRAYSIZE(state._v1FleensTraitDestSlots); i++)
			state._v1FleensTraitDestSlots[i] = state._v2FleensTraitDestSlots[i];
		return;
	}

	// TLC moves the transition flag to 0x000C.
	// It also introduces TouchSense and Help Audio.
	// Use TLC new-game defaults for fields omitted by v1.x.
	state.setTouchSenseEnabled(true);
	state.setHelpAudioEnabled(true);
	state.setV2TransitionsDisabled(state.getV1TransitionsDisabled());
	for (int32 i = 0; i < ARRAYSIZE(state._v2FleensTraitValueRotations); i++)
		state._v2FleensTraitValueRotations[i] = state._v1FleensTraitValueRotations[i];
	for (int32 i = 0; i < ARRAYSIZE(state._v2FleensTraitDestSlots); i++)
		state._v2FleensTraitDestSlots[i] = state._v1FleensTraitDestSlots[i];
	state._v2PreviousPage = state._currentPage;
	state._v2TwinGenStatusPad = 0;
}

ZoombiniGameState::ZmbSaveFormat ZoombiniGameState::getLoadSaveFormatFromSize(int32 size, ZmbSaveFormat targetFormat) {
	switch (targetFormat) {
	case ZmbSaveFormat::kEuV10:
		// Europe v1.x reads its fixed native prefix and ignores trailing bytes.
		return kStateFileSizeBrV10 <= size ? ZmbSaveFormat::kEuV10 : ZmbSaveFormat::kInvalid;
	case ZmbSaveFormat::kUsV11:
		// US v1.1 accepts the exact v1.0 length and clears its five added words.
		if (kStateFileSizeUsV11 <= size)
			return ZmbSaveFormat::kUsV11;
		return size == kStateFileSizeBrV10 ? ZmbSaveFormat::kEuV10 : ZmbSaveFormat::kInvalid;
	case ZmbSaveFormat::kTlcV20:
		// TLC v2.0 accepts the exact short layout and clears its five added words.
		if (kStateFileSizeTlcV20 <= size)
			return ZmbSaveFormat::kTlcV20;
		return size == kStateFileSizeTlcV20Small ? ZmbSaveFormat::kTlcV20Small : ZmbSaveFormat::kInvalid;
	default:
		return ZmbSaveFormat::kInvalid;
	}
}

bool ZoombiniGameState::hasValidStateFileMagic(Common::SeekableReadStream &stream) {
	const uint16 magic = stream.readUint16BE();
	return !stream.err() && magic == kEndianMagic;
}

bool ZmbStateFile::isLoadablePage(ZmbDestPageKind page) {
	return page == ZmbDestPageKind::kMap_01 || (ZmbDestPageKind::kIsle_03 <= page && page <= ZmbDestPageKind::kMaze_18);
}

bool ZmbStateFile::hasValidSerializedScalars() const {
	if (1 < _flagSfxEnable || 1 < _flagBgmEnable || 1 < _flagStickyMouseEnable ||
		1 < _flagCursorVisible || 1 < _flagDebug || 1 < _flagAutoStickyMouse ||
		1 < _v1TransitionsDisable || 1 < _tlcTouchSenseEnable ||
		1 < _tlcHelpAudioEnable || 1 < _v2TransitionsDisable || 1 < _lessActionFlag)
		return false;

	for (uint i = 0; i < ARRAYSIZE(_v1FleensTraitValueRotations); i++) {
		if (5 < _v1FleensTraitValueRotations[i])
			return false;
	}
	for (uint i = 0; i < ARRAYSIZE(_v1FleensTraitDestSlots); i++) {
		if (4 < _v1FleensTraitDestSlots[i])
			return false;
	}
	for (uint i = 0; i < ARRAYSIZE(_v2FleensTraitValueRotations); i++) {
		if (5 < _v2FleensTraitValueRotations[i])
			return false;
	}
	for (uint i = 0; i < ARRAYSIZE(_v2FleensTraitDestSlots); i++) {
		if (4 < _v2FleensTraitDestSlots[i])
			return false;
	}
	for (uint i = 0; i < ARRAYSIZE(_bcOneMushroomColors); i++) {
		if (4 < _bcOneMushroomColors[i])
			return false;
	}
	if (5 < _townScrollCol)
		return false;

	int32 generatedCount = 0;
	for (uint i = 0; i < ARRAYSIZE(_twinGenStatus); i++) {
		if (2 < _twinGenStatus[i])
			return false;
		generatedCount += _twinGenStatus[i];
	}
	return generatedCount == _zmbGeneratedCount && generatedCount <= 625;
}

void ZmbStateFile::recoverSerializedScalars() {
	setSfxEnabled(getSfxEnabled());
	setBgmEnabled(getBgmEnabled());
	setStickyMouseEnabled(getStickyMouseEnabled());
	setCursorVisible(getCursorVisible());
	setDebugEnabled(getDebugEnabled());
	setAutoStickyMouseEnabled(getAutoStickyMouseEnabled());
	setV1TransitionsDisabled(getV1TransitionsDisabled());
	setTouchSenseEnabled(getTouchSenseEnabled());
	setHelpAudioEnabled(getHelpAudioEnabled());
	setV2TransitionsDisabled(getV2TransitionsDisabled());
	setLessActionEnabled(getLessActionEnabled());

	for (uint i = 0; i < ARRAYSIZE(_v1FleensTraitValueRotations); i++) {
		if (5 < _v1FleensTraitValueRotations[i])
			_v1FleensTraitValueRotations[i] = 0;
	}
	for (uint i = 0; i < ARRAYSIZE(_v1FleensTraitDestSlots); i++) {
		if (4 < _v1FleensTraitDestSlots[i])
			_v1FleensTraitDestSlots[i] = 0;
	}
	for (uint i = 0; i < ARRAYSIZE(_v2FleensTraitValueRotations); i++) {
		if (5 < _v2FleensTraitValueRotations[i])
			_v2FleensTraitValueRotations[i] = 0;
	}
	for (uint i = 0; i < ARRAYSIZE(_v2FleensTraitDestSlots); i++) {
		if (4 < _v2FleensTraitDestSlots[i])
			_v2FleensTraitDestSlots[i] = 0;
	}
	for (uint i = 0; i < ARRAYSIZE(_bcOneMushroomColors); i++) {
		if (4 < _bcOneMushroomColors[i])
			_bcOneMushroomColors[i] = 0;
	}
	if (5 < _townScrollCol)
		_townScrollCol = 0;

	int16 remainingGeneratedCount = 625;
	for (uint i = 0; i < ARRAYSIZE(_twinGenStatus); i++) {
		int16 generationCount = _twinGenStatus[i] <= 2 ? _twinGenStatus[i] : 0;
		generationCount = MIN<int16>(generationCount, remainingGeneratedCount);
		_twinGenStatus[i] = static_cast<byte>(generationCount);
		remainingGeneratedCount -= generationCount;
	}
	_zmbGeneratedCount = 625 - remainingGeneratedCount;
}

bool ZmbStateFile::hasValidRodMapProgress() const {
	const byte routeProgress[4][4] = {
		{static_cast<byte>(_pageLevelFlags[3] & 0x0F),
		 static_cast<byte>(_pageLevelFlags[4] & 0x0F),
		 static_cast<byte>(_pageLevelFlags[5] & 0x0F),
		 static_cast<byte>(_levelFlagRouteBigBadHungry & 0x0F)},
		{static_cast<byte>(_pageLevelFlags[6] & 0x0F),
		 static_cast<byte>(_pageLevelFlags[7] & 0x0F),
		 static_cast<byte>(_pageLevelFlags[8] & 0x0F),
		 static_cast<byte>(_levelFlagLoWhosBayouHiDeepDarkForest & 0x0F)},
		{static_cast<byte>(_pageLevelFlags[9] & 0x0F),
		 static_cast<byte>(_pageLevelFlags[10] & 0x0F),
		 static_cast<byte>(_pageLevelFlags[11] & 0x0F),
		 static_cast<byte>((_levelFlagLoWhosBayouHiDeepDarkForest & 0xF0) >> 4)},
		{static_cast<byte>(_pageLevelFlags[12] & 0x0F),
		 static_cast<byte>(_pageLevelFlags[13] & 0x0F),
		 static_cast<byte>(_pageLevelFlags[14] & 0x0F),
		 static_cast<byte>(_levelFlagRouteMontDespair & 0x0F)},
	};

	for (uint routeIdx = 0; routeIdx < ARRAYSIZE(_routeLevels); routeIdx++) {
		if (_routeLevels[routeIdx] != 0)
			continue;
		for (uint progressIdx = 0; progressIdx < ARRAYSIZE(routeProgress[routeIdx]); progressIdx++) {
			if (4 < routeProgress[routeIdx][progressIdx])
				return false;
		}
	}
	return true;
}

void ZmbStateFile::recoverRodMapProgress() {
	const byte routeProgress[4][4] = {
		{static_cast<byte>(_pageLevelFlags[3] & 0x0F),
		 static_cast<byte>(_pageLevelFlags[4] & 0x0F),
		 static_cast<byte>(_pageLevelFlags[5] & 0x0F),
		 static_cast<byte>(_levelFlagRouteBigBadHungry & 0x0F)},
		{static_cast<byte>(_pageLevelFlags[6] & 0x0F),
		 static_cast<byte>(_pageLevelFlags[7] & 0x0F),
		 static_cast<byte>(_pageLevelFlags[8] & 0x0F),
		 static_cast<byte>(_levelFlagLoWhosBayouHiDeepDarkForest & 0x0F)},
		{static_cast<byte>(_pageLevelFlags[9] & 0x0F),
		 static_cast<byte>(_pageLevelFlags[10] & 0x0F),
		 static_cast<byte>(_pageLevelFlags[11] & 0x0F),
		 static_cast<byte>((_levelFlagLoWhosBayouHiDeepDarkForest & 0xF0) >> 4)},
		{static_cast<byte>(_pageLevelFlags[12] & 0x0F),
		 static_cast<byte>(_pageLevelFlags[13] & 0x0F),
		 static_cast<byte>(_pageLevelFlags[14] & 0x0F),
		 static_cast<byte>(_levelFlagRouteMontDespair & 0x0F)},
	};

	for (uint routeIdx = 0; routeIdx < ARRAYSIZE(_routeLevels); routeIdx++) {
		if (_routeLevels[routeIdx] != 0)
			continue;
		for (uint progressIdx = 0; progressIdx < ARRAYSIZE(routeProgress[routeIdx]); progressIdx++) {
			if (4 < routeProgress[routeIdx][progressIdx]) {
				_routeLevels[routeIdx] = 3;
				break;
			}
		}
	}
}

int16 ZoombiniGameState::countSnoidsInPack(const ZmbStateActivePack &pack) {
	return CLIP<int16>(pack.getPackZmbCount(), 0, pack.getEntryCapacity());
}

int16 ZoombiniGameState::countStoredSnoidsInChunk(const ZmbStateStoredChunk &chunk) {
	return chunk.getPopulatedEntryCount();
}

static bool isShelterSnoidInPack(const ZmbStateActivePack &pack, const ZmbStateActiveEntry &entry) {
	const bool isOccupied = entry.getIsOccupied();
	return !((isOccupied && pack.getSkipOccupiedEntries()) ||
			 (!isOccupied && pack.getSkipUnoccupiedEntries()));
}

int16 ZoombiniGameState::countShelterSnoidsInPack(const ZmbStateActivePack &pack) {
	int16 residentCount = 0;
	const int16 entryLimit = countSnoidsInPack(pack);

	for (int16 entryIdx = 0; entryIdx < entryLimit; entryIdx++) {
		const ZmbStateActiveEntry &entry = pack.getEntry(entryIdx);
		if (isShelterSnoidInPack(pack, entry))
			residentCount += 1;
	}
	return residentCount;
}

int16 ZoombiniGameState::countOccupiedSnoidsInPack(const ZmbStateActivePack &pack) {
	int16 occupiedCount = 0;
	const int16 entryLimit = countSnoidsInPack(pack);

	for (int16 entryIdx = 0; entryIdx < entryLimit; entryIdx++) {
		const ZmbStateActiveEntry &entry = pack.getEntry(entryIdx);
		if (entry.getIsOccupied())
			occupiedCount += 1;
	}
	return occupiedCount;
}

static bool hasLoadablePuzzlePack(const ZmbStateFile &state) {
	if (state._currentPage < ZmbDestPageKind::kBridge_07 || ZmbDestPageKind::kMaze_18 < state._currentPage)
		return true;

	const ZmbStateActivePack &activePack = state._zmbPackActive;
	if (activePack.getPackZmbCount() < 1 || 16 < activePack.getPackZmbCount())
		return false;
	if (state._currentPage == ZmbDestPageKind::kCaves_16 && activePack.getSkipOccupiedEntries())
		return false;
	return true;
}

void ZoombiniGameState::limitStoredSnoidsInChunk(ZmbStateStoredChunk &chunk, int16 maximumCount) {
	maximumCount = MAX<int16>(maximumCount, 0);

	int16 retainedCount = 0;
	for (int16 entryIdx = 0; entryIdx < chunk.getEntryCapacity(); entryIdx++) {
		ZmbStateStoredEntry &entry = chunk.getEntry(entryIdx);
		if (!entry.getTraits().isComplete())
			continue;
		if (retainedCount < maximumCount) {
			retainedCount += 1;
			continue;
		}
		entry.setTraits(ZmbTrait());
	}
	chunk.setStoredCount(retainedCount);
}

void ZoombiniGameState::limitShelterSnoidsInPack(ZmbStateActivePack &pack, int16 maximumCount) {
	maximumCount = MAX<int16>(maximumCount, 0);

	ZmbStateActivePack limitedPack;
	limitedPack.setSkipOccupiedEntries(pack.getSkipOccupiedEntries());
	limitedPack.setSkipUnoccupiedEntries(pack.getSkipUnoccupiedEntries());
	int16 retainedCount = 0;
	const int16 entryLimit = countSnoidsInPack(pack);
	for (int16 entryIdx = 0; entryIdx < entryLimit; entryIdx++) {
		const ZmbStateActiveEntry &entry = pack.getEntry(entryIdx);
		const bool isResident = isShelterSnoidInPack(pack, entry);
		if (isResident && maximumCount <= retainedCount)
			continue;
		if (!limitedPack.appendEntry(entry))
			break;
		if (isResident)
			retainedCount += 1;
	}
	pack = limitedPack;
}

void ZoombiniGameState::limitOccupiedSnoidsInPack(ZmbStateActivePack &pack, int16 maximumCount) {
	maximumCount = MAX<int16>(maximumCount, 0);

	ZmbStateActivePack limitedPack;
	limitedPack.setSkipOccupiedEntries(pack.getSkipOccupiedEntries());
	limitedPack.setSkipUnoccupiedEntries(pack.getSkipUnoccupiedEntries());
	int16 retainedCount = 0;
	const int16 entryLimit = countSnoidsInPack(pack);
	for (int16 entryIdx = 0; entryIdx < entryLimit; entryIdx++) {
		const ZmbStateActiveEntry &entry = pack.getEntry(entryIdx);
		const bool isOccupied = entry.getIsOccupied();
		if (isOccupied && maximumCount <= retainedCount)
			continue;
		if (!limitedPack.appendEntry(entry))
			break;
		if (isOccupied)
			retainedCount += 1;
	}
	pack = limitedPack;
}

bool ZoombiniGameState::recoverState(ZmbStateFile &state) {
	state._magic006B = kEndianMagic;
	state.recoverSerializedScalars();
	state._storedChunkBC1.recoverSerializedState();
	state._storedChunkBC2.recoverSerializedState();
	state._storedChunkTown.recoverSerializedState();
	state._zmbPackIsle.recoverSerializedState();
	state._zmbPackBC1.recoverSerializedState();
	state._zmbPackBC2.recoverSerializedState();
	state._zmbPackActive.recoverSerializedState();

	for (uint routeIdx = 0; routeIdx < ARRAYSIZE(state._routeLevels); routeIdx++) {
		state._routeLevels[routeIdx] = CLIP<int16>(state._routeLevels[routeIdx], 0, 3);
	}
	state.recoverRodMapProgress();
	if (state._currentRoute < 0 || static_cast<int16>(ZmbDestPageKind::kMaze_18) < state._currentRoute)
		state._currentRoute = 0;
	if (!state.hasLoadableCurrentPage())
		state._currentPage = ZmbDestPageKind::kMap_01;
	if (state._v2PreviousPage != ZmbDestPageKind::kUnk_00 &&
		state._v2PreviousPage != ZmbDestPageKind::kMap_01 &&
		!(ZmbDestPageKind::kIsle_03 <= state._v2PreviousPage && state._v2PreviousPage <= ZmbDestPageKind::kMaze_18))
		state._v2PreviousPage = ZmbDestPageKind::kUnk_00;
	for (uint routeIdx = 0; routeIdx < ARRAYSIZE(state._routePerfectCounters); routeIdx++) {
		state._routePerfectCounters[routeIdx] = CLIP<int16>(state._routePerfectCounters[routeIdx], 0, 2);
	}
	state._townDevelopLevel = CLIP<int16>(state._townDevelopLevel, 0, 6);

	int32 storedBasecamp1Count = countStoredSnoidsInChunk(state._storedChunkBC1);
	int32 storedBasecamp2Count = countStoredSnoidsInChunk(state._storedChunkBC2);
	int32 storedTownCount = countStoredSnoidsInChunk(state._storedChunkTown);
	int32 residentBasecamp1Count = countShelterSnoidsInPack(state._zmbPackBC1);
	int32 residentBasecamp2Count = countShelterSnoidsInPack(state._zmbPackBC2);
	int32 activePackCount = countOccupiedSnoidsInPack(state._zmbPackActive);

	int32 basecamp1Count = CLIP<int32>(state._zmbStoredBC1Count, storedBasecamp1Count, storedBasecamp1Count + residentBasecamp1Count);
	int32 basecamp2Count = CLIP<int32>(state._zmbStoredBC2Count, storedBasecamp2Count, storedBasecamp2Count + residentBasecamp2Count);

	if (625 < activePackCount + storedTownCount + basecamp2Count + basecamp1Count) {
		// Preserve the active party and the furthest completed destinations first.
		int32 remainingCount = 625;
		const int16 activeLimit = static_cast<int16>(MIN<int32>(activePackCount, remainingCount));
		limitOccupiedSnoidsInPack(state._zmbPackActive, activeLimit);
		remainingCount -= activeLimit;

		const int16 townLimit = static_cast<int16>(MIN<int32>(storedTownCount, remainingCount));
		limitStoredSnoidsInChunk(state._storedChunkTown, townLimit);
		remainingCount -= townLimit;

		const int16 basecamp2Limit = static_cast<int16>(MIN<int32>(basecamp2Count, remainingCount));
		const int16 basecamp2StorageLimit = static_cast<int16>(MIN<int32>(storedBasecamp2Count, basecamp2Limit));
		limitStoredSnoidsInChunk(state._storedChunkBC2, basecamp2StorageLimit);
		limitShelterSnoidsInPack(state._zmbPackBC2, basecamp2Limit - basecamp2StorageLimit);
		remainingCount -= basecamp2Limit;

		const int16 basecamp1Limit = static_cast<int16>(MIN<int32>(basecamp1Count, remainingCount));
		const int16 basecamp1StorageLimit = static_cast<int16>(MIN<int32>(storedBasecamp1Count, basecamp1Limit));
		limitStoredSnoidsInChunk(state._storedChunkBC1, basecamp1StorageLimit);
		limitShelterSnoidsInPack(state._zmbPackBC1, basecamp1Limit - basecamp1StorageLimit);

		storedBasecamp1Count = countStoredSnoidsInChunk(state._storedChunkBC1);
		storedBasecamp2Count = countStoredSnoidsInChunk(state._storedChunkBC2);
		storedTownCount = countStoredSnoidsInChunk(state._storedChunkTown);
		residentBasecamp1Count = countShelterSnoidsInPack(state._zmbPackBC1);
		residentBasecamp2Count = countShelterSnoidsInPack(state._zmbPackBC2);
		activePackCount = countOccupiedSnoidsInPack(state._zmbPackActive);
		basecamp1Count = storedBasecamp1Count + residentBasecamp1Count;
		basecamp2Count = storedBasecamp2Count + residentBasecamp2Count;
	}

	state._zmbStoredBC1Count = static_cast<int16>(basecamp1Count);
	state._zmbStoredBC2Count = static_cast<int16>(basecamp2Count);
	state._zmbStoredTownCount = static_cast<int16>(storedTownCount);
	if (!hasLoadablePuzzlePack(state))
		state._currentPage = ZmbDestPageKind::kMap_01;
	state._isDirty = false;

	ZoombiniSaveSummary summary;
	return summarizeState(state, summary);
}

bool ZoombiniGameState::summarizeState(const ZmbStateFile &state, ZoombiniSaveSummary &summary) {
	const auto rejectState = [&summary](ZmbSaveIssue issue, const char *reason) {
		summary._stateIssue = issue;
		warning("state: corrupted save state: %s", reason);
		return false;
	};
	if (state._magic006B != kEndianMagic)
		return rejectState(ZmbSaveIssue::kMagic, "magic");
	if (!state.hasLoadableCurrentPage() || !state.hasValidV2PreviousPage())
		return rejectState(ZmbSaveIssue::kPage, "page");
	if (!state.hasValidSerializedScalars())
		return rejectState(ZmbSaveIssue::kScalarFields, "scalar fields");
	if (!state._storedChunkBC1.isSerializedStateValid())
		return rejectState(ZmbSaveIssue::kBasecamp1Storage, "Basecamp 1 storage");
	if (!state._storedChunkBC2.isSerializedStateValid())
		return rejectState(ZmbSaveIssue::kBasecamp2Storage, "Basecamp 2 storage");
	if (!state._storedChunkTown.isSerializedStateValid())
		return rejectState(ZmbSaveIssue::kTownStorage, "Town storage");
	if (!state._zmbPackIsle.isSerializedStateValid())
		return rejectState(ZmbSaveIssue::kIslePack, "Isle pack");
	if (!state._zmbPackBC1.isSerializedStateValid())
		return rejectState(ZmbSaveIssue::kBasecamp1Pack, "Basecamp 1 pack");
	if (!state._zmbPackBC2.isSerializedStateValid())
		return rejectState(ZmbSaveIssue::kBasecamp2Pack, "Basecamp 2 pack");
	if (!state._zmbPackActive.isSerializedStateValid())
		return rejectState(ZmbSaveIssue::kActivePack, "active pack");
	if (!hasLoadablePuzzlePack(state))
		return rejectState(ZmbSaveIssue::kPuzzlePack, "puzzle pack");
	if (!(0 <= state._currentRoute && state._currentRoute <= static_cast<int16>(ZmbDestPageKind::kMaze_18)) ||
		!(0 <= state._townDevelopLevel && state._townDevelopLevel <= 6))
		return rejectState(ZmbSaveIssue::kProgressFields, "progress fields");
	for (uint routeIdx = 0; routeIdx < ARRAYSIZE(state._routeLevels); routeIdx++) {
		if (!(0 <= state._routeLevels[routeIdx] && state._routeLevels[routeIdx] <= 3))
			return rejectState(ZmbSaveIssue::kRouteLevel, "route level");
	}
	if (!state.hasValidRodMapProgress())
		return rejectState(ZmbSaveIssue::kRodMapProgress, "RodMap progress");
	for (uint routeIdx = 0; routeIdx < ARRAYSIZE(state._routePerfectCounters); routeIdx++) {
		if (!(0 <= state._routePerfectCounters[routeIdx] && state._routePerfectCounters[routeIdx] <= 2))
			return rejectState(ZmbSaveIssue::kPerfectCounter, "perfect counter");
	}

	const int32 basecamp1Count = state._zmbStoredBC1Count;
	const int32 basecamp2Count = state._zmbStoredBC2Count;
	const int32 townCount = state._zmbStoredTownCount;
	if (!(0 <= basecamp1Count && basecamp1Count <= 625) ||
		!(0 <= basecamp2Count && basecamp2Count <= 625) ||
		!(0 <= townCount && townCount <= 625))
		return rejectState(ZmbSaveIssue::kLogicalLocationCount, "logical location count");

	const int32 rodMapIsleCount = 625 - (basecamp1Count + basecamp2Count + townCount);
	const int32 activePackCount = countOccupiedSnoidsInPack(state._zmbPackActive);
	const int32 isleCount = rodMapIsleCount - activePackCount;
	if (!(0 <= isleCount && isleCount <= 625) ||
		!(0 <= activePackCount && activePackCount <= 625))
		return rejectState(ZmbSaveIssue::kIsleActiveCount, "Isle or active count");

	const int32 storedBasecamp1Count = countStoredSnoidsInChunk(state._storedChunkBC1);
	const int32 storedBasecamp2Count = countStoredSnoidsInChunk(state._storedChunkBC2);
	const int32 storedTownCount = countStoredSnoidsInChunk(state._storedChunkTown);
	const int32 residentBasecamp1Capacity = countShelterSnoidsInPack(state._zmbPackBC1);
	const int32 residentBasecamp2Capacity = countShelterSnoidsInPack(state._zmbPackBC2);
	if (basecamp1Count < storedBasecamp1Count || storedBasecamp1Count + residentBasecamp1Capacity < basecamp1Count ||
		basecamp2Count < storedBasecamp2Count || storedBasecamp2Count + residentBasecamp2Capacity < basecamp2Count ||
		storedTownCount != townCount)
		return rejectState(ZmbSaveIssue::kPhysicalLocationCount, "physical location count");

	summary._isleCount = static_cast<int16>(isleCount);
	summary._basecamp1Count = static_cast<int16>(basecamp1Count);
	summary._basecamp2Count = static_cast<int16>(basecamp2Count);
	summary._townCount = static_cast<int16>(townCount);
	summary._activePackCount = static_cast<int16>(activePackCount);

#ifndef NDEBUG
	// The save-management columns must reconstruct RodMap's four values.
	assert(summary._isleCount + summary._activePackCount == rodMapIsleCount);
	assert(summary._basecamp1Count == state._zmbStoredBC1Count);
	assert(summary._basecamp2Count == state._zmbStoredBC2Count);
	assert(summary._townCount == state._zmbStoredTownCount);

	// Every displayed row is a strict partition of all 625 Zoombinis.
	assert(0 <= summary._isleCount && summary._isleCount <= 625);
	assert(0 <= summary._basecamp1Count && summary._basecamp1Count <= 625);
	assert(0 <= summary._basecamp2Count && summary._basecamp2Count <= 625);
	assert(0 <= summary._townCount && summary._townCount <= 625);
	assert(0 <= summary._activePackCount && summary._activePackCount <= 625);
	assert(summary._isleCount + summary._basecamp1Count + summary._basecamp2Count +
			   summary._townCount + summary._activePackCount ==
		   625);
#endif

	return true;
}

void ZoombiniGameState::compactActivePack(ZmbStateActivePack &pack) {
	const int16 entryLimit = CLIP<int16>(pack.getPackZmbCount(), 0, pack.getEntryCapacity());

	int16 destIdx = 0;
	for (int16 srcIdx = 0; srcIdx < entryLimit; srcIdx++) {
		ZmbStateActiveEntry &entry = pack.getEntry(srcIdx);
		if (!entry.getIsOccupied() || !entry.getTraits().isComplete())
			continue;

		if (destIdx != srcIdx)
			pack.getEntry(destIdx) = entry;
		destIdx += 1;
	}

	for (int16 entryIdx = destIdx; entryIdx < pack.getEntryCapacity(); entryIdx++)
		pack.getEntry(entryIdx) = ZmbStateActiveEntry();
	pack.removeEntriesFromBack(pack.getPackZmbCount() - destIdx);
}

ZoombiniGameState::ZoombiniGameState(MohawkEngine_Zoombini *vm, Common::SaveFileManager *saveFileMan) : _vm(vm), _saveFileMan(saveFileMan) {
	initVariantDefaults();
}

ZoombiniGameState::~ZoombiniGameState() {
}

void ZoombiniGameState::beginPracticeState() {
	if (_practiceModeActive)
		return;

	_practiceState = _journeyState;
	_practiceModeActive = true;
}

void ZoombiniGameState::endPracticeState() {
	_practiceModeActive = false;
}

bool ZoombiniGameState::parseRoster(const Common::Array<byte> &bytes, ZmbRosterFile &roster) {
	if (bytes.size() != kZmbRosterFileSize)
		return false;

	Common::MemoryReadStream stream(bytes.begin(), bytes.size());
	return parseRoster(&stream, roster);
}

bool ZoombiniGameState::parseRoster(Common::SeekableReadStream *stream, ZmbRosterFile &roster) {
	if (!stream || stream->size() != kZmbRosterFileSize)
		return false;

	ZmbRosterFile parsedRoster;
	Common::Serializer serializer(stream, nullptr);
	parsedRoster.sync(serializer);
	const uint16 serializedCapacity = parsedRoster.getEntryCapacityForFileSize(static_cast<uint32>(stream->size()));
	if (stream->err() || serializedCapacity != parsedRoster.getEntryCapacity() ||
		parsedRoster._magic006B != kEndianMagic)
		return false;

	roster = parsedRoster;
	return true;
}

bool ZoombiniGameState::loadRosterFile(Common::SaveFileManager *saveFileMan, const Common::String &filename, ZmbRosterFile &roster) {
	if (!saveFileMan)
		return false;

	Common::InSaveFile *file = saveFileMan->openForLoading(filename);
	if (!file)
		return false;

	const bool result = parseRoster(file, roster);
	delete file;
	return result;
}

bool ZoombiniGameState::writeRosterFile(Common::SaveFileManager *saveFileMan, const Common::String &filename, const ZmbRosterFile &roster,
										Common::ErrorCode *fallbackErrorCode) {
	if (fallbackErrorCode)
		*fallbackErrorCode = Common::kWritingFailed;
	if (!saveFileMan || !roster.isSerializedStateValid())
		return false;

	saveFileMan->clearError();
	Common::OutSaveFile *file = saveFileMan->openForSaving(filename);
	if (!file) {
		if (fallbackErrorCode)
			*fallbackErrorCode = Common::kCreatingFileFailed;
		return false;
	}

	ZmbRosterFile rosterCopy = roster;
	Common::Serializer serializer(nullptr, file);
	rosterCopy.sync(serializer);
	file->finalize();
	const bool result = !file->err();
	delete file;
	if (!result && fallbackErrorCode)
		*fallbackErrorCode = Common::kWritingFailed;
	return result;
}

bool ZoombiniGameState::saveRosterFile(Common::SaveFileManager *saveFileMan, const Common::String &filename, const ZmbRosterFile &roster) {
	return writeRosterFile(saveFileMan, filename, roster, nullptr);
}

static bool getTargetStateFileNumber(const Common::String &target, const Common::String &filename, Common::String &number) {
	const Common::String prefix = target + "-";
	if (filename.size() != prefix.size() + 8 ||
		!filename.substr(0, prefix.size()).equalsIgnoreCase(prefix) ||
		!filename.substr(filename.size() - 4).equalsIgnoreCase(".ZMB"))
		return false;

	number = filename.substr(prefix.size(), 4);
	for (uint digitIdx = 0; digitIdx < number.size(); digitIdx++) {
		if (number[digitIdx] < '0' || '9' < number[digitIdx])
			return false;
	}
	return true;
}

static Common::StringArray listTargetStateFiles(Common::SaveFileManager *saveFileMan, const Common::String &target) {
	Common::StringArray result;
	if (!saveFileMan)
		return result;

	const Common::StringArray candidates = saveFileMan->listSavefiles(target + "-*.ZMB");
	for (uint fileIdx = 0; fileIdx < candidates.size(); fileIdx++) {
		Common::String number;
		if (getTargetStateFileNumber(target, candidates[fileIdx], number))
			result.push_back(candidates[fileIdx]);
	}
	Common::sort(result.begin(), result.end());
	return result;
}

static int findTargetStateFile(const Common::StringArray &filenames, const Common::String &filename) {
	for (uint fileIdx = 0; fileIdx < filenames.size(); fileIdx++) {
		if (filenames[fileIdx].equalsIgnoreCase(filename))
			return static_cast<int>(fileIdx);
	}
	return -1;
}

static bool decodeAvailableRosterEntries(Common::InSaveFile *file, ZmbRosterFile &roster,
										 uint16 &availableEntryCapacity) {
	availableEntryCapacity = 0;
	if (!file || file->size() < static_cast<int64>(ZmbRosterFile::kSerializedHeaderSize))
		return false;

	// Only complete serialized fields inside the bounded input prefix are decoded.
	// A short final entry is ignored, and bytes beyond the fixed 50-entry format
	// are never read even when an untrusted file has trailing data.
	const uint32 readSize = static_cast<uint32>(MIN<int64>(file->size(), ZoombiniGameState::kZmbRosterFileSize));
	Common::Array<byte> bytes;
	bytes.resize(ZoombiniGameState::kZmbRosterFileSize);
	memset(bytes.begin(), 0, bytes.size());
	if (!file->seek(0) || file->read(bytes.begin(), readSize) != readSize)
		return false;

	Common::MemoryReadStream stream(bytes.begin(), bytes.size());
	Common::Serializer serializer(&stream, nullptr);
	roster.sync(serializer);
	if (stream.err())
		return false;

	availableEntryCapacity = roster.getEntryCapacityForFileSize(readSize);
	return true;
}

uint32 ZoombiniGameState::getRosterIssueFlagsForTarget(const Common::String &target) {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	const Common::StringArray stateFiles = listTargetStateFiles(saveFileMan, target);
	Common::InSaveFile *file = saveFileMan->openForLoading(target + ".WHO");
	if (!file)
		return stateFiles.empty() ? kRosterIssueNone : kRosterIssueMissing | kRosterIssueOrphanState;

	uint32 issues = kRosterIssueNone;
	const int64 sourceSize = file->size();
	if (sourceSize != kZmbRosterFileSize)
		issues |= kRosterIssueSize;

	ZmbRosterFile roster;
	uint16 sourceCapacity = 0;
	const bool decoded = decodeAvailableRosterEntries(file, roster, sourceCapacity);
	delete file;
	if (!decoded)
		return issues | kRosterIssueRead |
			   (stateFiles.empty() ? kRosterIssueNone : kRosterIssueOrphanState);

	if (roster._magic006B != kEndianMagic)
		issues |= kRosterIssueMagic;
	if (sourceCapacity < roster._saveEntryCount)
		issues |= kRosterIssueEntryCount;
	if (!roster.hasValidNextSaveFileNameCounter())
		issues |= kRosterIssueNextCounter;

	const uint16 entryCount = MIN<uint16>(roster._saveEntryCount, sourceCapacity);
	Common::Array<byte> referencedStateFiles;
	referencedStateFiles.resize(stateFiles.size());
	if (!referencedStateFiles.empty())
		memset(referencedStateFiles.begin(), 0, referencedStateFiles.size());
	for (uint16 entryIdx = 0; entryIdx < entryCount; entryIdx++) {
		const ZmbRosterEntry &entry = roster._entries[entryIdx];
		const uint16 saveNameLength = entry.getSaveNameLength();
		if (saveNameLength == 0 || ARRAYSIZE(entry._saveName) <= saveNameLength)
			issues |= kRosterIssueSaveName;

		const Common::String fileNumber = entry.getSaveFileNumStr();
		if (fileNumber.empty()) {
			issues |= kRosterIssueFileStem;
			continue;
		}

		const Common::String stateFilename = makeSaveFilename(target, entry._fileName);
		const int stateFileIdx = findTargetStateFile(stateFiles, stateFilename);
		if (stateFileIdx < 0) {
			issues |= kRosterIssueMissingState;
			continue;
		}
		if (referencedStateFiles[stateFileIdx] != 0) {
			issues |= kRosterIssueDuplicateFileStem;
			continue;
		}
		referencedStateFiles[stateFileIdx] = 1;
	}

	if (entryCount < roster.getEntryCapacity()) {
		for (uint fileIdx = 0; fileIdx < referencedStateFiles.size(); fileIdx++) {
			if (referencedStateFiles[fileIdx] == 0) {
				issues |= kRosterIssueOrphanState;
				break;
			}
		}
	}
	return issues;
}

ZoombiniGameState::ZmbRosterRecoverResult ZoombiniGameState::recoverRosterForTarget(const Common::String &target) {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	const uint32 originalIssues = getRosterIssueFlagsForTarget(target);
	if (originalIssues == kRosterIssueNone)
		return ZmbRosterRecoverResult::kNotNeeded;

	const Common::String rosterFilename = target + ".WHO";
	const Common::StringArray stateFiles = listTargetStateFiles(saveFileMan, target);
	ZmbRosterFile sourceRoster;
	uint16 sourceEntryCount = 0;
	Common::InSaveFile *sourceFile = saveFileMan->openForLoading(rosterFilename);
	const bool hadRoster = sourceFile != nullptr;
	const int64 sourceFileSize = sourceFile ? sourceFile->size() : -1;
	if (sourceFile) {
		uint16 sourceCapacity = 0;
		if (decodeAvailableRosterEntries(sourceFile, sourceRoster, sourceCapacity))
			sourceEntryCount = MIN<uint16>(sourceRoster._saveEntryCount, sourceCapacity);
		delete sourceFile;
	}

	ZmbRosterFile recoveredRoster;
	Common::Array<byte> usedStateFiles;
	usedStateFiles.resize(stateFiles.size());
	if (!usedStateFiles.empty())
		memset(usedStateFiles.begin(), 0, usedStateFiles.size());
	for (uint16 sourceIdx = 0; sourceIdx < sourceEntryCount && recoveredRoster._saveEntryCount < recoveredRoster.getEntryCapacity(); sourceIdx++) {
		ZmbRosterEntry entry = sourceRoster._entries[sourceIdx];
		if (ARRAYSIZE(entry._saveName) <= entry.getSaveNameLength())
			entry._saveName[ARRAYSIZE(entry._saveName) - 1] = 0;
		if (entry.getSaveNameLength() == 0) {
			const Common::String recoveredName = Common::String::format("Recovered Save %u", static_cast<uint>(recoveredRoster._saveEntryCount + 1));
			memset(entry._saveName, 0, ARRAYSIZE(entry._saveName));
			memcpy(entry._saveName, recoveredName.c_str(),
				   MIN<uint32>(recoveredName.size(), ARRAYSIZE(entry._saveName) - 1));
		}

		int stateFileIdx = -1;
		if (!entry.getSaveFileNumStr().empty())
			stateFileIdx = findTargetStateFile(stateFiles, makeSaveFilename(target, entry._fileName));
		if (0 <= stateFileIdx && usedStateFiles[stateFileIdx] != 0)
			stateFileIdx = -1;
		if (stateFileIdx < 0) {
			for (uint candidateIdx = 0; candidateIdx < stateFiles.size(); candidateIdx++) {
				if (usedStateFiles[candidateIdx] == 0) {
					stateFileIdx = static_cast<int>(candidateIdx);
					break;
				}
			}
		}
		if (stateFileIdx < 0)
			continue;

		Common::String stateNumber;
		if (!getTargetStateFileNumber(target, stateFiles[stateFileIdx], stateNumber))
			continue;
		const Common::String repairedStem = "ZOOM" + stateNumber;
		memset(entry._fileName, 0, ARRAYSIZE(entry._fileName));
		memcpy(entry._fileName, repairedStem.c_str(), repairedStem.size());
		recoveredRoster._entries[recoveredRoster._saveEntryCount] = entry;
		recoveredRoster._saveEntryCount += 1;
		usedStateFiles[stateFileIdx] = 1;
	}

	for (uint stateFileIdx = 0; stateFileIdx < stateFiles.size() && recoveredRoster._saveEntryCount < recoveredRoster.getEntryCapacity(); stateFileIdx++) {
		if (usedStateFiles[stateFileIdx] != 0)
			continue;
		Common::String stateNumber;
		if (!getTargetStateFileNumber(target, stateFiles[stateFileIdx], stateNumber))
			continue;
		ZmbRosterEntry entry;
		const Common::String recoveredName = Common::String::format("Recovered Save %u", static_cast<uint>(recoveredRoster._saveEntryCount + 1));
		memcpy(entry._saveName, recoveredName.c_str(), MIN<uint32>(recoveredName.size(), ARRAYSIZE(entry._saveName) - 1));
		const Common::String repairedStem = "ZOOM" + stateNumber;
		memcpy(entry._fileName, repairedStem.c_str(), repairedStem.size());
		recoveredRoster._entries[recoveredRoster._saveEntryCount] = entry;
		recoveredRoster._saveEntryCount += 1;
		usedStateFiles[stateFileIdx] = 1;
	}

	recoveredRoster._nextSaveFileNameCounter = 10000;
	for (uint32 counter = 0; counter <= 9999; counter++) {
		const Common::String stem = Common::String::format("ZOOM%04u", counter);
		bool usedByRoster = false;
		for (uint16 entryIdx = 0; entryIdx < recoveredRoster.getEntryCount(); entryIdx++) {
			if (recoveredRoster._entries[entryIdx].getSaveFileNumStr() == stem.substr(4)) {
				usedByRoster = true;
				break;
			}
		}
		const Common::String filename = makeSaveFilename(target, reinterpret_cast<const byte *>(stem.c_str()));
		saveFileMan->clearError();
		if (!usedByRoster && !saveFileMan->exists(filename)) {
			recoveredRoster._nextSaveFileNameCounter = static_cast<uint16>(counter);
			break;
		}
	}

	Common::String temporaryFilename;
	for (uint32 attempt = 0; attempt < 1024 && temporaryFilename.empty(); attempt++) {
		const Common::String candidate = Common::String::format("%s-roster-recovery-%u.tmp", target.c_str(), attempt);
		saveFileMan->clearError();
		if (!saveFileMan->exists(candidate))
			temporaryFilename = candidate;
	}
	if (temporaryFilename.empty() || !saveRosterFile(saveFileMan, temporaryFilename, recoveredRoster)) {
		if (!temporaryFilename.empty()) {
			saveFileMan->clearError();
			saveFileMan->removeSavefile(temporaryFilename);
		}
		return ZmbRosterRecoverResult::kWriteFailed;
	}
	ZmbRosterFile verifiedTemporaryRoster;
	if (!loadRosterFile(saveFileMan, temporaryFilename, verifiedTemporaryRoster) ||
		!verifiedTemporaryRoster.isSerializedStateValid()) {
		saveFileMan->clearError();
		saveFileMan->removeSavefile(temporaryFilename);
		return ZmbRosterRecoverResult::kWriteFailed;
	}

	Common::String backupFilename;
	if (hadRoster) {
		for (uint32 attempt = 0; attempt < 1024 && backupFilename.empty(); attempt++) {
			const Common::String candidate = Common::String::format("%s-roster-recovery-%u.bak", target.c_str(), attempt);
			saveFileMan->clearError();
			if (!saveFileMan->exists(candidate))
				backupFilename = candidate;
		}
		if (backupFilename.empty() || !copySaveFile(saveFileMan, rosterFilename, backupFilename)) {
			saveFileMan->clearError();
			saveFileMan->removeSavefile(temporaryFilename);
			return ZmbRosterRecoverResult::kBackupFailed;
		}
		Common::InSaveFile *backupFile = saveFileMan->openForLoading(backupFilename);
		const bool backupValid = 0 <= sourceFileSize && backupFile &&
								 backupFile->size() == sourceFileSize && !backupFile->err();
		delete backupFile;
		if (!backupValid) {
			saveFileMan->clearError();
			saveFileMan->removeSavefile(temporaryFilename);
			saveFileMan->clearError();
			saveFileMan->removeSavefile(backupFilename);
			return ZmbRosterRecoverResult::kBackupFailed;
		}
	}

	if (hadRoster) {
		saveFileMan->clearError();
		if (!saveFileMan->removeSavefile(rosterFilename)) {
			saveFileMan->clearError();
			saveFileMan->removeSavefile(temporaryFilename);
			return ZmbRosterRecoverResult::kWriteFailed;
		}
	}

	const bool replaced = moveSaveFile(saveFileMan, temporaryFilename, rosterFilename);
	if (replaced) {
		ZmbRosterFile verifiedRoster;
		if (loadRosterFile(saveFileMan, rosterFilename, verifiedRoster) &&
			verifiedRoster.isSerializedStateValid() && getRosterIssueFlagsForTarget(target) == kRosterIssueNone) {
			warning("state: recovered roster '%s' from available save files", rosterFilename.c_str());
			return ZmbRosterRecoverResult::kSuccess;
		}
	}

	saveFileMan->clearError();
	if (saveFileMan->exists(rosterFilename)) {
		saveFileMan->clearError();
		saveFileMan->removeSavefile(rosterFilename);
	}
	if (!hadRoster) {
		saveFileMan->clearError();
		if (saveFileMan->exists(temporaryFilename)) {
			saveFileMan->clearError();
			saveFileMan->removeSavefile(temporaryFilename);
		}
		return ZmbRosterRecoverResult::kWriteFailed;
	}
	if (!copySaveFile(saveFileMan, backupFilename, rosterFilename))
		return ZmbRosterRecoverResult::kRollbackFailed;
	saveFileMan->clearError();
	if (saveFileMan->exists(temporaryFilename)) {
		saveFileMan->clearError();
		saveFileMan->removeSavefile(temporaryFilename);
	}
	return ZmbRosterRecoverResult::kWriteFailed;
}

bool ZoombiniGameState::containsSaveFilename(const Common::Array<Common::String> &filenames, const Common::String &filename) {
	for (uint i = 0; i < filenames.size(); i++) {
		if (filenames[i].equalsIgnoreCase(filename))
			return true;
	}

	return false;
}

bool ZoombiniGameState::copySaveFile(Common::SaveFileManager *saveFileMan, const Common::String &source, const Common::String &destination) {
	if (!saveFileMan || source.equalsIgnoreCase(destination))
		return false;

	saveFileMan->clearError();
	if (saveFileMan->exists(destination))
		return false;

	saveFileMan->clearError();
	if (saveFileMan->copySavefile(source, destination))
		return true;

	saveFileMan->clearError();
	if (saveFileMan->exists(destination)) {
		saveFileMan->clearError();
		saveFileMan->removeSavefile(destination);
	}
	return false;
}

bool ZoombiniGameState::moveSaveFile(Common::SaveFileManager *saveFileMan, const Common::String &source, const Common::String &destination) {
	if (!copySaveFile(saveFileMan, source, destination))
		return false;

	saveFileMan->clearError();
	if (saveFileMan->removeSavefile(source))
		return true;

	saveFileMan->clearError();
	saveFileMan->removeSavefile(destination);
	return false;
}

bool ZoombiniGameState::restoreCompactedSaveFiles(Common::SaveFileManager *saveFileMan,
												  const Common::Array<Common::String> &originalFilenames,
												  const Common::Array<Common::String> &temporaryFilenames,
												  const Common::Array<Common::String> &compactedFilenames,
												  int stagedCount, int compactedCount) {
	bool restored = true;
	for (int i = compactedCount - 1; 0 <= i; i--) {
		if (!moveSaveFile(saveFileMan, compactedFilenames[i], temporaryFilenames[i]))
			restored = false;
	}

	Common::Array<Common::String> restoredOriginalFilenames;
	restoredOriginalFilenames.reserve(stagedCount);
	for (int i = 0; i < stagedCount; i++) {
		// A damaged roster can have several temporary files for one original.
		// Restore that shared original once and discard the extra staged copies.
		if (containsSaveFilename(restoredOriginalFilenames, originalFilenames[i])) {
			saveFileMan->clearError();
			if (saveFileMan->exists(temporaryFilenames[i])) {
				saveFileMan->clearError();
				if (!saveFileMan->removeSavefile(temporaryFilenames[i]))
					restored = false;
			}
			continue;
		}

		if (moveSaveFile(saveFileMan, temporaryFilenames[i], originalFilenames[i]))
			restoredOriginalFilenames.push_back(originalFilenames[i]);
		else
			restored = false;
	}
	return restored;
}

ZoombiniGameState::ZmbSaveCompactResult ZoombiniGameState::compactRosterSaveFiles(Common::SaveFileManager *saveFileMan, const Common::String &target,
																				  ZmbRosterFile &roster) {
	if (!saveFileMan || !roster.isSerializedStateValid())
		return ZmbSaveCompactResult::kInvalidRoster;

	const int entryCount = roster.getEntryCount();
	Common::Array<Common::String> originalFilenames;
	Common::Array<Common::String> temporaryFilenames;
	Common::Array<Common::String> compactedFilenames;
	Common::Array<Common::String> compactedStems;
	originalFilenames.reserve(entryCount);
	temporaryFilenames.reserve(entryCount);
	compactedFilenames.reserve(entryCount);
	compactedStems.reserve(entryCount);

	bool fileNamesNeedCompaction = false;
	for (int i = 0; i < entryCount; i++) {
		const ZmbRosterEntry &entry = roster.getEntry(i);
		if (entry.getSaveFileNumStr().empty())
			return ZmbSaveCompactResult::kInvalidRoster;
		const Common::String storedStem = entry.getSaveFileStem();

		const Common::String originalFilename = makeSaveFilename(target, entry._fileName);
		saveFileMan->clearError();
		if (!saveFileMan->exists(originalFilename))
			return ZmbSaveCompactResult::kMissingSaveFile;

		const Common::String compactedStem = Common::String::format("ZOOM%04d", i);
		const Common::String compactedFilename = makeSaveFilename(target, reinterpret_cast<const byte *>(compactedStem.c_str()));
		if (storedStem != compactedStem)
			fileNamesNeedCompaction = true;

		originalFilenames.push_back(originalFilename);
		compactedFilenames.push_back(compactedFilename);
		compactedStems.push_back(compactedStem);
	}

	for (int i = 0; i < entryCount; i++) {
		saveFileMan->clearError();
		if (saveFileMan->exists(compactedFilenames[i]) && !containsSaveFilename(originalFilenames, compactedFilenames[i]))
			return ZmbSaveCompactResult::kFileNameConflict;
	}

	const Common::String nextStem = Common::String::format("ZOOM%04d", entryCount);
	const Common::String nextFilename = makeSaveFilename(target, reinterpret_cast<const byte *>(nextStem.c_str()));
	saveFileMan->clearError();
	if (saveFileMan->exists(nextFilename) && !containsSaveFilename(originalFilenames, nextFilename))
		return ZmbSaveCompactResult::kFileNameConflict;

	if (!fileNamesNeedCompaction && roster._nextSaveFileNameCounter == static_cast<uint16>(entryCount))
		return ZmbSaveCompactResult::kNoChanges;

	const ZmbRosterFile originalRoster = roster;
	if (fileNamesNeedCompaction) {
		bool temporaryNamesAvailable = false;
		for (uint32 attempt = 0; !temporaryNamesAvailable; attempt++) {
			temporaryNamesAvailable = true;
			temporaryFilenames.clear();
			for (int i = 0; i < entryCount; i++) {
				const Common::String temporaryFilename = Common::String::format("%s-compact-%u-%d.tmp", target.c_str(), attempt, i);
				saveFileMan->clearError();
				if (saveFileMan->exists(temporaryFilename) || containsSaveFilename(originalFilenames, temporaryFilename) ||
					containsSaveFilename(compactedFilenames, temporaryFilename)) {
					temporaryNamesAvailable = false;
					break;
				}
				temporaryFilenames.push_back(temporaryFilename);
			}
		}

		int stagedCount = 0;
		for (int i = 0; i < entryCount; i++) {
			int duplicateSourceIdx = -1;
			for (int sourceIdx = 0; sourceIdx < i; sourceIdx++) {
				if (originalFilenames[sourceIdx].equalsIgnoreCase(originalFilenames[i])) {
					duplicateSourceIdx = sourceIdx;
					break;
				}
			}

			// Preserve every damaged roster row. The first reference moves the
			// shared state into staging; later references receive staged copies.
			bool staged;
			if (0 <= duplicateSourceIdx)
				staged = copySaveFile(saveFileMan, temporaryFilenames[duplicateSourceIdx], temporaryFilenames[i]);
			else
				staged = moveSaveFile(saveFileMan, originalFilenames[i], temporaryFilenames[i]);
			if (!staged) {
				const Common::String *sourceFilename;
				if (0 <= duplicateSourceIdx)
					sourceFilename = &temporaryFilenames[duplicateSourceIdx];
				else
					sourceFilename = &originalFilenames[i];
				saveFileMan->clearError();
				const bool sourcePreserved = saveFileMan->exists(*sourceFilename);
				saveFileMan->clearError();
				const bool temporaryAbsent = !saveFileMan->exists(temporaryFilenames[i]);
				if (!restoreCompactedSaveFiles(saveFileMan, originalFilenames, temporaryFilenames, compactedFilenames, stagedCount, 0) ||
					!sourcePreserved || !temporaryAbsent)
					return ZmbSaveCompactResult::kRollbackFailed;
				return ZmbSaveCompactResult::kFileMoveFailed;
			}
			stagedCount += 1;
		}

		int compactedCount = 0;
		for (int i = 0; i < entryCount; i++) {
			if (!moveSaveFile(saveFileMan, temporaryFilenames[i], compactedFilenames[i])) {
				saveFileMan->clearError();
				const bool temporaryPreserved = saveFileMan->exists(temporaryFilenames[i]);
				saveFileMan->clearError();
				const bool compactedAbsent = !saveFileMan->exists(compactedFilenames[i]);
				if (!restoreCompactedSaveFiles(saveFileMan, originalFilenames, temporaryFilenames, compactedFilenames, stagedCount, compactedCount) ||
					!temporaryPreserved || !compactedAbsent)
					return ZmbSaveCompactResult::kRollbackFailed;
				return ZmbSaveCompactResult::kFileMoveFailed;
			}
			compactedCount += 1;
		}
	}

	for (int i = 0; i < entryCount; i++) {
		ZmbRosterEntry &entry = roster.getEntry(i);
		memset(entry._fileName, 0, ARRAYSIZE(entry._fileName));
		memcpy(entry._fileName, compactedStems[i].c_str(), compactedStems[i].size());
	}
	roster._nextSaveFileNameCounter = static_cast<uint16>(entryCount);

	const Common::String rosterFilename = target + ".WHO";
	Common::String rosterBackupFilename;
	for (uint32 attempt = 0; rosterBackupFilename.empty(); attempt++) {
		const Common::String candidate = Common::String::format("%s-compact-roster-%u.tmp", target.c_str(), attempt);
		saveFileMan->clearError();
		if (!saveFileMan->exists(candidate))
			rosterBackupFilename = candidate;
	}

	saveFileMan->clearError();
	if (!saveFileMan->copySavefile(rosterFilename, rosterBackupFilename)) {
		saveFileMan->clearError();
		if (saveFileMan->exists(rosterBackupFilename)) {
			saveFileMan->clearError();
			if (!saveFileMan->removeSavefile(rosterBackupFilename))
				warning("Could not remove incomplete Zoombini roster backup '%s'", rosterBackupFilename.c_str());
		}
		roster = originalRoster;
		if (fileNamesNeedCompaction &&
			!restoreCompactedSaveFiles(saveFileMan, originalFilenames, temporaryFilenames, compactedFilenames, entryCount, entryCount))
			return ZmbSaveCompactResult::kRollbackFailed;
		return ZmbSaveCompactResult::kRosterWriteFailed;
	}

	if (!saveRosterFile(saveFileMan, rosterFilename, roster)) {
		roster = originalRoster;
		const bool filesRestored = !fileNamesNeedCompaction ||
								   restoreCompactedSaveFiles(saveFileMan, originalFilenames, temporaryFilenames, compactedFilenames, entryCount, entryCount);
		saveFileMan->clearError();
		const bool rosterRestored = saveFileMan->copySavefile(rosterBackupFilename, rosterFilename);
		if (rosterRestored) {
			saveFileMan->clearError();
			if (!saveFileMan->removeSavefile(rosterBackupFilename))
				warning("Could not remove Zoombini roster backup '%s'", rosterBackupFilename.c_str());
		}
		if (!filesRestored || !rosterRestored)
			return ZmbSaveCompactResult::kRollbackFailed;
		return ZmbSaveCompactResult::kRosterWriteFailed;
	}

	saveFileMan->clearError();
	if (!saveFileMan->removeSavefile(rosterBackupFilename))
		warning("Could not remove Zoombini roster backup '%s'", rosterBackupFilename.c_str());

	return ZmbSaveCompactResult::kSuccess;
}

Common::Array<ZoombiniSaveSummary> ZoombiniGameState::listSaveSummaries(const Common::String &target) {
	Common::Array<ZoombiniSaveSummary> summaries;
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	ZmbRosterFile roster;
	if (!loadRosterFile(saveFileMan, target + ".WHO", roster))
		return summaries;

	const bool rosterHeaderValid = roster.hasValidEntryCount() && roster.hasValidNextSaveFileNameCounter();
	const int entryCount = roster.getEntryCount();
	for (int i = 0; i < entryCount; i++) {
		const ZmbRosterEntry &rosterEntry = roster.getEntry(i);
		ZoombiniSaveSummary summary;
		const uint16 saveNameLength = rosterEntry.getSaveNameLength();
		const bool saveNameValid = saveNameLength < ARRAYSIZE(rosterEntry._saveName);
		const uint16 boundedSaveNameLength = MIN<uint16>(saveNameLength, static_cast<uint16>(ARRAYSIZE(rosterEntry._saveName)));
		summary._saveName = Common::String(reinterpret_cast<const char *>(rosterEntry._saveName), boundedSaveNameLength);
		summary._saveFileNumber = rosterEntry.getSaveFileNumStr();
		if (!rosterHeaderValid || !saveNameValid || summary._saveFileNumber.empty()) {
			summaries.push_back(summary);
			continue;
		}
		summary._fileName = makeSaveFilename(target, rosterEntry._fileName);
		if (summary._fileName.empty()) {
			summaries.push_back(summary);
			continue;
		}
		Common::InSaveFile *stateFile = saveFileMan->openForLoading(summary._fileName);
		if (stateFile) {
			const ZmbSaveFormat format = getSaveFormatFromSize(stateFile->size());
			summary._saveFormat = format;
			if (format != ZmbSaveFormat::kInvalid) {
				ZmbStateFile state;
				Common::Serializer serializer(stateFile, nullptr);
				state.sync(serializer, isTlcSaveFormat(format), hasCompletionCounters(format));
				if (!stateFile->err()) {
					if (summarizeState(state, summary)) {
						summary._stateValid = true;
					} else {
						ZmbStateFile recoveredState = state;
						summary._stateRecoverable = recoverState(recoveredState);
					}
				}
			}
			delete stateFile;
		}

		summaries.push_back(summary);
	}

	return summaries;
}

bool ZoombiniGameState::deleteSaveForTarget(const Common::String &target, int slot) {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	ZmbRosterFile roster;
	if (!loadRosterFile(saveFileMan, target + ".WHO", roster) || !roster.hasEntry(slot))
		return false;

	const Common::String filename = makeSaveFilename(target, roster.getEntry(slot)._fileName);
	if (!roster.removeEntryAt(slot))
		return false;
	if (!saveRosterFile(saveFileMan, target + ".WHO", roster))
		return false;
	if (filename.empty())
		return true;

	bool filenameStillReferenced = false;
	for (int i = 0; i < roster.getEntryCount(); i++) {
		if (filename.equalsIgnoreCase(makeSaveFilename(target, roster.getEntry(i)._fileName))) {
			filenameStillReferenced = true;
			break;
		}
	}
	if (!filenameStillReferenced && !saveFileMan->removeSavefile(filename))
		warning("Could not remove deleted Zoombini save file '%s'", filename.c_str());

	return true;
}

bool ZoombiniGameState::renameSaveForTarget(const Common::String &target, int slot, const Common::U32String &saveName) {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	ZmbRosterFile roster;
	if (!loadRosterFile(saveFileMan, target + ".WHO", roster))
		return false;

	if (!ZoombiniGameState::renameRosterEntry(target, roster, slot, saveName))
		return false;

	return saveRosterFile(saveFileMan, target + ".WHO", roster);
}

bool ZoombiniGameState::moveSaveForTarget(const Common::String &target, int slot, int destinationSlot) {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	ZmbRosterFile roster;
	if (!loadRosterFile(saveFileMan, target + ".WHO", roster) || slot < 0 || destinationSlot < 0 ||
		!roster.hasEntry(slot) || !roster.hasEntry(destinationSlot) ||
		(slot + 1 != destinationSlot && destinationSlot + 1 != slot))
		return false;

	if (!roster.swapEntries(slot, destinationSlot))
		return false;

	return saveRosterFile(saveFileMan, target + ".WHO", roster);
}

ZoombiniGameState::ZmbSaveRecoverResult ZoombiniGameState::recoverSaveForTarget(const Common::String &target, int slot) {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	ZmbRosterFile roster;
	if (!loadRosterFile(saveFileMan, target + ".WHO", roster) || !roster.isSerializedStateValid() || !roster.hasEntry(slot))
		return ZmbSaveRecoverResult::kNotRecoverable;

	const ZmbRosterEntry &rosterEntry = roster.getEntry(slot);
	const uint16 saveNameLength = rosterEntry.getSaveNameLength();
	if (ARRAYSIZE(rosterEntry._saveName) <= saveNameLength || rosterEntry.getSaveFileNumStr().empty())
		return ZmbSaveRecoverResult::kNotRecoverable;

	const Common::String filename = makeSaveFilename(target, rosterEntry._fileName);
	Common::InSaveFile *stateFile = saveFileMan->openForLoading(filename);
	if (!stateFile)
		return ZmbSaveRecoverResult::kNotRecoverable;

	const ZmbSaveFormat format = getSaveFormatFromSize(stateFile->size());
	ZmbStateFile state;
	if (format != ZmbSaveFormat::kInvalid) {
		Common::Serializer serializer(stateFile, nullptr);
		state.sync(serializer, isTlcSaveFormat(format), hasCompletionCounters(format));
	}
	const bool stateRead = format != ZmbSaveFormat::kInvalid && !stateFile->err();
	delete stateFile;
	if (!stateRead)
		return ZmbSaveRecoverResult::kNotRecoverable;

	ZoombiniSaveSummary summary;
	if (summarizeState(state, summary) || !recoverState(state))
		return ZmbSaveRecoverResult::kNotRecoverable;

	Common::String temporaryFilename;
	for (uint32 attempt = 0; attempt < 1024 && temporaryFilename.empty(); attempt++) {
		const Common::String candidate = Common::String::format("%s-recovery-%u.tmp", filename.c_str(), attempt);
		saveFileMan->clearError();
		if (!saveFileMan->exists(candidate))
			temporaryFilename = candidate;
	}
	if (temporaryFilename.empty())
		return ZmbSaveRecoverResult::kTemporaryWriteFailed;

	Common::OutSaveFile *outputFile = saveFileMan->openForSaving(temporaryFilename);
	if (!outputFile) {
		saveFileMan->clearError();
		saveFileMan->removeSavefile(temporaryFilename);
		return ZmbSaveRecoverResult::kTemporaryWriteFailed;
	}
	Common::Serializer outputSerializer(nullptr, outputFile);
	state.sync(outputSerializer, isTlcSaveFormat(format), hasCompletionCounters(format));
	outputFile->finalize();
	const bool writeSucceeded = !outputFile->err();
	delete outputFile;
	if (!writeSucceeded) {
		saveFileMan->clearError();
		saveFileMan->removeSavefile(temporaryFilename);
		return ZmbSaveRecoverResult::kTemporaryWriteFailed;
	}

	const auto hasExpectedFormat = [saveFileMan, format](const Common::String &candidate) {
		saveFileMan->clearError();
		Common::InSaveFile *file = saveFileMan->openForLoading(candidate);
		if (!file)
			return false;
		const bool valid = getSaveFormatFromSize(file->size()) == format && !file->err();
		delete file;
		return valid;
	};
	const auto isRecoveredFileValid = [saveFileMan, format](const Common::String &candidate) {
		saveFileMan->clearError();
		Common::InSaveFile *file = saveFileMan->openForLoading(candidate);
		if (!file)
			return false;
		const ZmbSaveFormat candidateFormat = getSaveFormatFromSize(file->size());
		ZmbStateFile candidateState;
		if (candidateFormat == format) {
			Common::Serializer serializer(file, nullptr);
			candidateState.sync(serializer, isTlcSaveFormat(format), hasCompletionCounters(format));
		}
		const bool readSucceeded = candidateFormat == format && !file->err();
		delete file;
		ZoombiniSaveSummary candidateSummary;
		return readSucceeded && summarizeState(candidateState, candidateSummary);
	};
	if (!isRecoveredFileValid(temporaryFilename)) {
		saveFileMan->clearError();
		saveFileMan->removeSavefile(temporaryFilename);
		return ZmbSaveRecoverResult::kTemporaryWriteFailed;
	}

	Common::String backupFilename;
	for (uint32 attempt = 0; attempt < 1024 && backupFilename.empty(); attempt++) {
		const Common::String candidate = Common::String::format("%s-recovery-%u.bak", filename.c_str(), attempt);
		saveFileMan->clearError();
		if (!saveFileMan->exists(candidate))
			backupFilename = candidate;
	}
	if (backupFilename.empty() || !copySaveFile(saveFileMan, filename, backupFilename) || !hasExpectedFormat(backupFilename)) {
		saveFileMan->clearError();
		saveFileMan->removeSavefile(temporaryFilename);
		if (!backupFilename.empty()) {
			saveFileMan->clearError();
			saveFileMan->removeSavefile(backupFilename);
		}
		return ZmbSaveRecoverResult::kBackupFailed;
	}

	saveFileMan->clearError();
	if (!saveFileMan->removeSavefile(filename)) {
		saveFileMan->clearError();
		saveFileMan->removeSavefile(temporaryFilename);
		saveFileMan->clearError();
		saveFileMan->removeSavefile(backupFilename);
		return ZmbSaveRecoverResult::kReplaceFailed;
	}

	const bool replaced = moveSaveFile(saveFileMan, temporaryFilename, filename);
	if (replaced && isRecoveredFileValid(filename)) {
		warning("state: recovered save file '%s'; backup is '%s'", filename.c_str(), backupFilename.c_str());
		return ZmbSaveRecoverResult::kSuccess;
	}

	saveFileMan->clearError();
	if (saveFileMan->exists(filename)) {
		saveFileMan->clearError();
		saveFileMan->removeSavefile(filename);
	}
	const bool restored = copySaveFile(saveFileMan, backupFilename, filename);
	saveFileMan->clearError();
	if (saveFileMan->exists(temporaryFilename)) {
		saveFileMan->clearError();
		saveFileMan->removeSavefile(temporaryFilename);
	}
	if (!restored)
		return ZmbSaveRecoverResult::kRollbackFailed;

	saveFileMan->clearError();
	if (!saveFileMan->removeSavefile(backupFilename))
		warning("state: could not remove recovery backup '%s'", backupFilename.c_str());
	return ZmbSaveRecoverResult::kReplaceFailed;
}

ZoombiniGameState::ZmbSaveCompactResult ZoombiniGameState::compactSaveFilesForTarget(const Common::String &target) {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	ZmbRosterFile roster;
	if (!loadRosterFile(saveFileMan, target + ".WHO", roster))
		return ZmbSaveCompactResult::kInvalidRoster;

	return compactRosterSaveFiles(saveFileMan, target, roster);
}

bool ZoombiniGameState::isSaveCompactionNeededForTarget(const Common::String &target) {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	ZmbRosterFile roster;
	if (!loadRosterFile(saveFileMan, target + ".WHO", roster))
		return false;

	if (!roster.isSerializedStateValid())
		return false;

	const int entryCount = roster.getEntryCount();
	Common::Array<Common::String> originalFilenames;
	originalFilenames.reserve(entryCount);
	bool fileNamesNeedCompaction = false;
	for (int i = 0; i < entryCount; i++) {
		const ZmbRosterEntry &entry = roster.getEntry(i);
		if (entry.getSaveFileNumStr().empty())
			return false;

		const Common::String originalFilename = makeSaveFilename(target, entry._fileName);
		saveFileMan->clearError();
		if (!saveFileMan->exists(originalFilename))
			return false;
		originalFilenames.push_back(originalFilename);

		const Common::String storedStem = entry.getSaveFileStem();
		const Common::String compactedStem = Common::String::format("ZOOM%04d", i);
		if (storedStem != compactedStem)
			fileNamesNeedCompaction = true;
	}

	const bool counterNeedsCompaction = roster._nextSaveFileNameCounter != static_cast<uint16>(entryCount);
	if (!fileNamesNeedCompaction && !counterNeedsCompaction)
		return false;

	// Do not offer an operation that the compactor would reject because an
	// unlisted state file already owns one of the destination names.
	for (int i = 0; i < entryCount; i++) {
		const Common::String compactedStem = Common::String::format("ZOOM%04d", i);
		const Common::String compactedFilename = makeSaveFilename(target, reinterpret_cast<const byte *>(compactedStem.c_str()));
		saveFileMan->clearError();
		if (saveFileMan->exists(compactedFilename) &&
			!containsSaveFilename(originalFilenames, compactedFilename))
			return false;
	}

	const Common::String nextStem = Common::String::format("ZOOM%04d", entryCount);
	const Common::String nextFilename = makeSaveFilename(target, reinterpret_cast<const byte *>(nextStem.c_str()));
	saveFileMan->clearError();
	if (saveFileMan->exists(nextFilename) &&
		!containsSaveFilename(originalFilenames, nextFilename))
		return false;

	return true;
}

bool ZoombiniGameState::loadGame(int slot) {
	if (!loadState(slot)) {
		return false;
	}

	_gameStateReadyFlag = true;
	if (_journeyState._isDirty)
		markSaveBeforeQuitPending();
	else
		clearSaveBeforeQuitPending();
	suppressSaveBeforeQuitMarkForLoad();
	_vm->syncSoundSettings();

	return true;
}

bool ZoombiniGameState::saveGame(int slot) {
	if (inPracticeMode()) {
		warning("Cannot save Zoombini game while in practice mode");
		return false;
	}
	if (_debugUnsafeSyntheticStateFlag) {
		warning("Cannot save synthetic Zoombini debug state");
		return false;
	}

	if (!_r.isSerializedStateValid() || !_r.hasEntry(slot))
		return false;

	if (!saveState(slot)) {
		return false;
	}
	clearSaveBeforeQuitPending();
	clearDebugStateMutation();
	return true;
}

bool ZoombiniGameState::saveNewGame(const Common::U32String &saveName) {
	if (_debugUnsafeSyntheticStateFlag) {
		warning("Cannot save synthetic Zoombini debug state");
		return false;
	}

	Common::String encodedSaveName;
	if (!_r.isSerializedStateValid() ||
		!ZmbRosterEntry::encodeSaveName(saveName, _vm->_text->getExeCodePage(), encodedSaveName) ||
		0 <= searchSaveSlotByName(saveName))
		return false;

	const int slot = getAvailableSaveSlot();
	if (slot < 0)
		return false;

	const uint16 oldFileNameCounter = _r._nextSaveFileNameCounter;
	const int32 oldCurrentSaveSlot = _currentSaveSlot;
	const bool oldStateDirty = _journeyState._isDirty;

	ZmbRosterEntry entry;
	memcpy(entry._saveName, encodedSaveName.c_str(), encodedSaveName.size());
	if (!appendNewSaveEntryForTarget(_saveFileMan, getTargetName(), _r, entry)) {
		_r._nextSaveFileNameCounter = oldFileNameCounter;
		return false;
	}

	if (!saveState(slot)) {
		_r.removeEntryAt(slot);
		_r._nextSaveFileNameCounter = oldFileNameCounter;
		_currentSaveSlot = oldCurrentSaveSlot;
		return false;
	}

	if (saveRoster()) {
		clearSaveBeforeQuitPending();
		clearDebugStateMutation();
		return true;
	}

	_saveFileMan->removeSavefile(getSaveFilename(slot));
	_r.removeEntryAt(slot);
	_r._nextSaveFileNameCounter = oldFileNameCounter;
	_currentSaveSlot = oldCurrentSaveSlot;
	_journeyState._isDirty = oldStateDirty;
	return false;
}

bool ZoombiniGameState::renameGame(int slot, const Common::U32String &saveName) {
	Common::String encodedSaveName;
	if (!_r.isSerializedStateValid() || !_r.hasEntry(slot) || saveName.empty() ||
		!ZmbRosterEntry::encodeSaveName(saveName, _vm->_text->getExeCodePage(), encodedSaveName))
		return false;

	const int duplicateSlot = searchSaveSlotByName(saveName);
	if (0 <= duplicateSlot && duplicateSlot != slot)
		return false;

	ZmbRosterEntry &entry = _r.getEntry(slot);
	byte oldSaveName[ARRAYSIZE(entry._saveName)];
	memcpy(oldSaveName, entry._saveName, ARRAYSIZE(oldSaveName));

	memset(entry._saveName, 0, ARRAYSIZE(entry._saveName));
	memcpy(entry._saveName, encodedSaveName.c_str(), encodedSaveName.size());
	if (saveRoster())
		return true;

	memcpy(entry._saveName, oldSaveName, ARRAYSIZE(oldSaveName));
	return false;
}

bool ZoombiniGameState::moveGame(int slot, int destinationSlot) {
	if (!_r.isSerializedStateValid() || !_r.hasEntry(slot) || !_r.hasEntry(destinationSlot) ||
		(slot + 1 != destinationSlot && destinationSlot + 1 != slot))
		return false;

	if (!_r.swapEntries(slot, destinationSlot))
		return false;

	if (!saveRoster()) {
		_r.swapEntries(slot, destinationSlot);
		return false;
	}

	if (_currentSaveSlot == slot)
		_currentSaveSlot = destinationSlot;
	else if (_currentSaveSlot == destinationSlot)
		_currentSaveSlot = slot;

	return true;
}

void ZoombiniGameState::markDebugStateMutation() {
	_debugStateMutationFlag = true;
	_journeyState._isDirty = true;
}

void ZoombiniGameState::markUnsafeSyntheticDebugState() {
	_debugUnsafeSyntheticStateFlag = true;
	markDebugStateMutation();
}

bool ZoombiniGameState::deleteGameAndShiftRoster(int slot) {
	if (!_r.isSerializedStateValid() || !_r.hasEntry(slot))
		return false;

	const Common::String filename = getSaveFilename(slot);
	const ZmbRosterFile originalRoster = _r;
	const int32 originalCurrentSaveSlot = _currentSaveSlot;
	_r.removeEntryAt(slot);

	if (_currentSaveSlot == slot) {
		_currentSaveSlot = kUnsavedNewGame;
	} else if (slot < _currentSaveSlot) {
		_currentSaveSlot -= 1;
	}

	if (!saveRoster()) {
		_r = originalRoster;
		_currentSaveSlot = originalCurrentSaveSlot;
		return false;
	}

	bool filenameStillReferenced = false;
	for (int i = 0; i < _r.getEntryCount(); i++) {
		if (filename.equalsIgnoreCase(getSaveFilename(i))) {
			filenameStillReferenced = true;
			break;
		}
	}
	if (!filenameStillReferenced && !_saveFileMan->removeSavefile(filename))
		warning("Could not remove deleted Zoombini save file '%s'", filename.c_str());
	return true;
}

bool ZoombiniGameState::loadState(int slot) {
	_lastLoadCancelledFlag = false;
	if (!_r.isSerializedStateValid() || !_r.hasEntry(slot))
		return false;

	Common::String filename = getSaveFilename(slot);
	if (filename.empty())
		return false;
	Common::InSaveFile *loadFile = _saveFileMan->openForLoading(filename);
	if (!loadFile) {
		return false;
	}

	debugC(1, MohawkEngine_Zoombini::kDebugSaveLoad01, "state: loading game from '%s'", filename.c_str());

	const int32 size = loadFile->size();
	const ZmbSaveFormat targetFormat = getTargetSaveFormat(_vm);
	ZmbSaveFormat sourceFormat = getSaveFormatFromSize(size);
	if (!canLoadSaveFormat(sourceFormat))
		sourceFormat = getLoadSaveFormatFromSize(size, targetFormat);
	if (!canLoadSaveFormat(sourceFormat)) {
		warning("Cannot load Zoombini save file '%s': size %d is incompatible with target format %s",
				filename.c_str(), size, getSaveFormatName(targetFormat));
		delete loadFile;
		return false;
	}

	if (!hasValidStateFileMagic(*loadFile)) {
		warning("Cannot load Zoombini save file '%s': invalid state-file magic", filename.c_str());
		delete loadFile;
		return false;
	}
	if (!loadFile->seek(0)) {
		warning("Cannot load Zoombini save file '%s': could not rewind after checking state-file magic", filename.c_str());
		delete loadFile;
		return false;
	}

	if (isNewerThanTargetSaveFormat(sourceFormat, targetFormat)) {
		if (_vm->openConfirmMsgBoxDialog(_vm->_text->getLocalizedString(ZoombiniText::kSvmDialogBodyLoadNewerSaveFormat)) != ZoombiniDialogResult::kYes) {
			_lastLoadCancelledFlag = true;
			delete loadFile;
			return false;
		}
	}

	ZmbStateFile loadedState;
	Common::Serializer s(loadFile, nullptr);
	loadedState.sync(s, isTlcSaveFormat(sourceFormat), hasCompletionCounters(sourceFormat));
	const bool readFailed = loadFile->err();
	delete loadFile;

	ZoombiniSaveSummary ignoredSummary;
	if (readFailed || !summarizeState(loadedState, ignoredSummary)) {
		warning("state: corrupted save file '%s'", filename.c_str());
		return false;
	}

	upgradeStateForVariant(loadedState, sourceFormat, targetFormat);
	loadedState._isDirty = false;

	const bool usesKoreanNamePool = _vm->_text->getExeCodePage() == Common::kWindows949;
	const Common::U32String convPromptBody = _vm->_text->getLocalizedString(ZoombiniText::kSvmDialogBodyAskConvertUnreadableZoombiniNames);
	if (!usesKoreanNamePool && hasNonAsciiZoombiniName(loadedState) &&
		_vm->openConfirmMsgBoxDialog(convPromptBody) == ZoombiniDialogResult::kYes) {
		replaceZoombiniNamesWithStandardNames(loadedState);
		loadedState._isDirty = true;
	}
	_journeyState = loadedState;

	_currentSaveSlot = slot;
	clearDebugStateMutation();
	_debugUnsafeSyntheticStateFlag = false;
	return true;
}

void ZoombiniGameState::syncGameState(Common::Serializer &s, bool isTlcLayout, bool hasCompletionCounters) {
	_journeyState.sync(s, isTlcLayout, hasCompletionCounters);
}

void ZoombiniGameState::dumpCurrentState(Common::WriteStream &out) {
	Common::Serializer s(nullptr, &out);
	const ZmbSaveFormat targetFormat = getTargetSaveFormat(_vm);
	_journeyState.sync(s, isTlcSaveFormat(targetFormat), hasCompletionCounters(targetFormat));
}

Common::String ZoombiniGameState::getTargetName() const {
	return ConfMan.getActiveDomainName();
}

Common::String ZoombiniGameState::makeSaveFilename(const Common::String &targetName, const byte *baseName) {
	// The roster stores the legacy "ZOOM####" base name; the four-digit counter
	// suffix uniquely identifies the save within this target's namespace.
	if (!baseName || baseName[0] != 'Z' || baseName[1] != 'O' || baseName[2] != 'O' || baseName[3] != 'M' ||
		baseName[8] != 0)
		return Common::String();

	char counter[5] = {};
	for (uint i = 0; i < 4; i++) {
		const byte digit = baseName[i + 4];
		if (digit < '0' || '9' < digit)
			return Common::String();
		counter[i] = static_cast<char>(digit);
	}
	return Common::String::format("%s-%s.ZMB", targetName.c_str(), counter);
}

bool ZoombiniGameState::appendNewSaveEntryForTarget(Common::SaveFileManager *saveFileMan, const Common::String &target, ZmbRosterFile &roster,
													ZmbRosterEntry entry) {
	if (!saveFileMan || !roster.isSerializedStateValid() || roster.getEntryCapacity() <= roster.getEntryCount())
		return false;

	const uint16 originalCounter = roster._nextSaveFileNameCounter;
	uint32 candidateCounter = originalCounter;
	while (candidateCounter <= 9999) {
		const Common::String stem = Common::String::format("ZOOM%04u", candidateCounter);
		bool usedByRoster = false;
		for (int i = 0; i < roster.getEntryCount(); i++) {
			const Common::String saveFileNumber = roster.getEntry(i).getSaveFileNumStr();
			if (saveFileNumber.empty())
				return false;
			const Common::String existingStem = Common::String::format("ZOOM%s", saveFileNumber.c_str());
			if (existingStem == stem) {
				usedByRoster = true;
				break;
			}
		}

		const Common::String filename = makeSaveFilename(target, reinterpret_cast<const byte *>(stem.c_str()));
		saveFileMan->clearError();
		if (!usedByRoster && !saveFileMan->exists(filename)) {
			roster._nextSaveFileNameCounter = static_cast<uint16>(candidateCounter);
			if (roster.appendNewSaveEntry(entry))
				return true;

			roster._nextSaveFileNameCounter = originalCounter;
			return false;
		}

		candidateCounter += 1;
	}

	return false;
}

Common::String ZoombiniGameState::getSaveFilename(int slot) const {
	if (!_r.isSerializedStateValid() || !_r.hasEntry(slot))
		return Common::String();
	return makeSaveFilename(getTargetName(), _r.getEntry(slot)._fileName);
}

Common::String ZoombiniGameState::getRosterFilename() const {
	return getTargetName() + ".WHO";
}

bool ZoombiniGameState::hasNonAsciiZoombiniName(const ZmbStateFile &state) const {
	const ZmbStateActivePack *packs[] = {
		&state._zmbPackIsle,
		&state._zmbPackBC1,
		&state._zmbPackBC2,
		&state._zmbPackActive,
	};
	for (uint32 packIdx = 0; packIdx < ARRAYSIZE(packs); packIdx++) {
		for (int16 entryIdx = 0; entryIdx < packs[packIdx]->getPackZmbCount(); entryIdx++) {
			if (packs[packIdx]->getEntry(entryIdx).hasNonAsciiName())
				return true;
		}
	}

	const ZmbStateStoredChunk *chunks[] = {
		&state._storedChunkBC1,
		&state._storedChunkBC2,
		&state._storedChunkTown,
	};
	for (uint32 chunkIdx = 0; chunkIdx < ARRAYSIZE(chunks); chunkIdx++) {
		for (int16 entryIdx = 0; entryIdx < chunks[chunkIdx]->getEntryCapacity(); entryIdx++) {
			if (chunks[chunkIdx]->getEntry(entryIdx).hasNonAsciiName())
				return true;
		}
	}

	return false;
}

void ZoombiniGameState::replaceZoombiniNamesWithStandardNames(ZmbStateFile &state) {
	ZmbStateActivePack *packs[] = {
		&state._zmbPackIsle,
		&state._zmbPackBC1,
		&state._zmbPackBC2,
		&state._zmbPackActive,
	};
	for (uint32 packIdx = 0; packIdx < ARRAYSIZE(packs); packIdx++) {
		for (int16 entryIdx = 0; entryIdx < packs[packIdx]->getPackZmbCount(); entryIdx++) {
			ZmbStateActiveEntry &entry = packs[packIdx]->getEntry(entryIdx);
			if (entry.hasSerializedName())
				entry.setU32Name(_vm, _vm->_text->pickNextZoombiniName());
		}
	}

	ZmbStateStoredChunk *chunks[] = {
		&state._storedChunkBC1,
		&state._storedChunkBC2,
		&state._storedChunkTown,
	};
	for (uint32 chunkIdx = 0; chunkIdx < ARRAYSIZE(chunks); chunkIdx++) {
		for (int16 entryIdx = 0; entryIdx < chunks[chunkIdx]->getEntryCapacity(); entryIdx++) {
			ZmbStateStoredEntry &entry = chunks[chunkIdx]->getEntry(entryIdx);
			if (entry.hasSerializedName())
				entry.setName(_vm, _vm->_text->pickNextZoombiniName());
		}
	}
}

bool ZoombiniGameState::saveState(int slot) {
	const Common::String filename = getSaveFilename(slot);
	if (filename.empty())
		return false;

	ZoombiniPage *page = _vm->getActivePage();
	if (!page)
		return false;

	const ZoombiniPageType pageType = page->getPageType();
	ZmbStateActivePack *temporaryDestPack = nullptr;
	bool clearActiveBeforeWrite = false;
	bool writeState = true;

	switch (pageType) {
	case ZoombiniPageType::kRodMap:
	case ZoombiniPageType::kTown:
		// These pages suppress runner serialization.
		clearActiveBeforeWrite = true;
		break;
	case ZoombiniPageType::kPicker:
	case ZoombiniPageType::kBasecamp1:
	case ZoombiniPageType::kBasecamp2:
	case ZoombiniPageType::kXfer:
	case ZoombiniPageType::kBridge:
	case ZoombiniPageType::kTunnels:
	case ZoombiniPageType::kPizza:
	case ZoombiniPageType::kFerry:
	case ZoombiniPageType::kLilly:
	case ZoombiniPageType::kSlides:
	case ZoombiniPageType::kFleens:
	case ZoombiniPageType::kHotel:
	case ZoombiniPageType::kNet:
	case ZoombiniPageType::kCaves:
	case ZoombiniPageType::kSmoke:
	case ZoombiniPageType::kMaze:
		page->saveSnoidsToPack(true);
		break;
	default:
		writeState = false;
		break;
	}

	if (writeState) {
		switch (pageType) {
		case ZoombiniPageType::kPicker:
			_journeyState._zmbPackActive.copyTo(_journeyState._zmbPackIsle);
			temporaryDestPack = &_journeyState._zmbPackIsle;
			clearActiveBeforeWrite = true;
			break;
		case ZoombiniPageType::kBasecamp1:
			_journeyState._zmbPackActive.copyTo(_journeyState._zmbPackBC1);
			temporaryDestPack = &_journeyState._zmbPackBC1;
			clearActiveBeforeWrite = true;
			break;
		case ZoombiniPageType::kBasecamp2:
			_journeyState._zmbPackActive.copyTo(_journeyState._zmbPackBC2);
			temporaryDestPack = &_journeyState._zmbPackBC2;
			clearActiveBeforeWrite = true;
			break;
		default:
			break;
		}
	}

	if (clearActiveBeforeWrite || !writeState)
		_journeyState._zmbPackActive.clearEntries();
	if (!writeState)
		return false;

	_saveFileMan->clearError();
	Common::OutSaveFile *saveFile = _saveFileMan->openForSaving(filename);
	if (!saveFile) {
		if (temporaryDestPack)
			temporaryDestPack->clearEntries();
		_journeyState._zmbPackActive.clearEntries();
		warning("Cannot create Zoombini state save file '%s': %s", filename.c_str(),
				getSaveFileWriteError(_saveFileMan, Common::kCreatingFileFailed).getDesc().c_str());
		showStateFileWriteFailure(_saveFileMan, filename, Common::kCreatingFileFailed);
		return false;
	}

	debugC(1, MohawkEngine_Zoombini::kDebugSaveLoad01, "state: saving game to '%s'", filename.c_str());

	Common::Serializer s(nullptr, saveFile);
	const ZmbSaveFormat targetFormat = getTargetSaveFormat(_vm);
	syncGameState(s, isTlcSaveFormat(targetFormat), hasCompletionCounters(targetFormat));
	saveFile->finalize();
	const bool writeFailed = saveFile->err();
	delete saveFile;

	if (temporaryDestPack)
		temporaryDestPack->clearEntries();
	_journeyState._zmbPackActive.clearEntries();
	if (writeFailed) {
		warning("Cannot write Zoombini state save file '%s': %s", filename.c_str(),
				getSaveFileWriteError(_saveFileMan, Common::kWritingFailed).getDesc().c_str());
		showStateFileWriteFailure(_saveFileMan, filename, Common::kWritingFailed);
		return false;
	}

	_journeyState._isDirty = false;
	_currentSaveSlot = slot;
	return true;
}

void ZoombiniGameState::loadRoster() {
	Common::String filename = getRosterFilename();
	Common::InSaveFile *rosterFile = _saveFileMan->openForLoading(filename);
	if (!rosterFile) {
		return;
	}

	debugC(1, MohawkEngine_Zoombini::kDebugSaveLoad01, "state: loading roster from '%s'", filename.c_str());

	ZmbRosterFile parsedRoster;
	if (!parseRoster(rosterFile, parsedRoster) || !parsedRoster.isSerializedStateValid()) {
		warning("Invalid or corrupted Zoombini roster");
		delete rosterFile;
		return;
	}

	_r = parsedRoster;
	delete rosterFile;
}

bool ZoombiniGameState::saveRoster() {
	Common::String filename = getRosterFilename();
	debugC(1, MohawkEngine_Zoombini::kDebugSaveLoad01, "state: saving roster to '%s'", filename.c_str());
	Common::ErrorCode fallbackErrorCode;
	if (writeRosterFile(_saveFileMan, filename, _r, &fallbackErrorCode))
		return true;

	if (_r.isSerializedStateValid()) {
		warning("Cannot write Zoombini save roster '%s': %s", filename.c_str(), getSaveFileWriteError(_saveFileMan, fallbackErrorCode).getDesc().c_str());
		showRosterFileWriteFailure(_saveFileMan, filename, fallbackErrorCode);
	}
	return false;
}

int16 ZmbTrait::snoidId() const {
	if (!isComplete())
		return SNOID_INCOMPLETE; // No trait, invalid id

	return (_hair - 1) * 125 + (_eyes - 1) * 25 + (_nose - 1) * 5 + (_feet - 1);
}

void ZmbTrait::recoverSerializedValues() {
	if (5 < _hair)
		_hair = TRAIT_NONE;
	if (5 < _eyes)
		_eyes = TRAIT_NONE;
	if (5 < _nose)
		_nose = TRAIT_NONE;
	if (5 < _feet)
		_feet = TRAIT_NONE;
}

void ZmbTrait::sync(Common::Serializer &s) {
	s.syncAsByte(_hair);
	s.syncAsByte(_eyes);
	s.syncAsByte(_nose);
	s.syncAsByte(_feet);

	if (s.isSaving())
		assert(hasValidValues());
}

void ZmbStateStoredEntry::sync(Common::Serializer &s, bool isTlcLayout) {
	_traits.sync(s);
	s.syncAsUint16LE(_rect.bottom);
	s.syncAsUint16LE(_rect.right);
	s.syncAsUint16LE(_rect.top);
	s.syncAsUint16LE(_rect.left);
	s.syncBytes(_name, ARRAYSIZE(_name));
	if (isTlcLayout)
		s.skip(2);
}

Common::U32String ZmbStateStoredEntry::getName(MohawkEngine_Zoombini *vm) const {
	const uint32 nameLength = getNameLength();
	if (ARRAYSIZE(_name) <= nameLength)
		return Common::U32String();
	return vm->_text->toU32String(_name, nameLength, ZoombiniText::kExeString);
}

void ZmbStateStoredEntry::setName(MohawkEngine_Zoombini *vm, const Common::U32String &name) {
	const Common::String encodedName = vm->_text->fromU32String(name, ZoombiniText::kExeString);
	memset(_name, 0, sizeof(_name));
	const uint32 nameLength = MIN<uint32>(encodedName.size(), ARRAYSIZE(_name) - 1);
	memcpy(_name, encodedName.c_str(), nameLength);
}

uint32 ZmbStateStoredEntry::getNameLength() const {
	for (uint32 length = 0; length < ARRAYSIZE(_name); length++) {
		if (_name[length] == 0)
			return length;
	}
	return ARRAYSIZE(_name);
}

bool ZmbStateStoredEntry::isSerializedStateValid() const {
	return _traits.hasValidValues() && getNameLength() < ARRAYSIZE(_name);
}

void ZmbStateStoredEntry::recoverSerializedState() {
	ZmbTrait traits = _traits;
	traits.recoverSerializedValues();
	_traits = traits;
	_name[ARRAYSIZE(_name) - 1] = 0;
}

bool ZmbStateStoredEntry::hasSerializedName() const {
	return _name[0] != 0;
}

bool ZmbStateStoredEntry::hasNonAsciiName() const {
	for (uint32 i = 0; i < ARRAYSIZE(_name) && _name[i] != 0; i++) {
		if (0x7F < _name[i])
			return true;
	}
	return false;
}

void Mohawk::ZmbStateStoredChunk::sync(Common::Serializer &s, bool isTlcLayout) {
	s.syncAsSint16LE(_leftmostColumnIdx);
	s.syncAsSint16LE(_storedCount);
	for (int i = 0; i < ARRAYSIZE(_entries); i++)
		_entries[i].sync(s, isTlcLayout);
}

int16 Mohawk::ZmbStateStoredChunk::getPopulatedEntryCount() const {
	int16 populatedCount = 0;
	for (int16 entryIdx = 0; entryIdx < getEntryCapacity(); entryIdx++) {
		if (_entries[entryIdx].getTraits().isComplete())
			populatedCount += 1;
	}
	return populatedCount;
}

bool Mohawk::ZmbStateStoredChunk::isSerializedStateValid() const {
	if (!(0 <= _leftmostColumnIdx && _leftmostColumnIdx <= 120) ||
		!(0 <= _storedCount && _storedCount <= getEntryCapacity()))
		return false;

	for (int16 i = 0; i < getEntryCapacity(); i++) {
		if (!_entries[i].isSerializedStateValid())
			return false;
	}
	return true;
}

void Mohawk::ZmbStateStoredChunk::recoverSerializedState() {
	_leftmostColumnIdx = CLIP<int16>(_leftmostColumnIdx, 0, 120);

	const bool storedCountValid = 0 <= _storedCount && _storedCount <= getEntryCapacity();
	int16 recoveredCount = 0;
	for (int16 i = 0; i < getEntryCapacity(); i++) {
		_entries[i].recoverSerializedState();
		if (_entries[i].getTraits().isComplete())
			recoveredCount += 1;
	}
	// A lower header is valid input because the original shelter drag restore
	// could repopulate a slot without restoring it. Consumers scan physical
	// slots, and shelter pages normalize the header before writing a new save.
	if (!storedCountValid || recoveredCount < _storedCount)
		_storedCount = recoveredCount;
}

void ZmbStateActiveEntry::sync(Common::Serializer &s, bool isTlcLayout) {
	_traits.sync(s);
	s.syncAsUint16LE(_posX);
	s.syncAsUint16LE(_posY);
	s.syncAsByte(_bIsOccupied);
	s.syncBytes(_name, ARRAYSIZE(_name));
	if (isTlcLayout)
		s.skip(1);
}

bool ZmbStateActiveEntry::isSerializedStateValid() const {
	return _traits.hasValidValues() && _bIsOccupied <= 1 && getNameLength() < ARRAYSIZE(_name);
}

void ZmbStateActiveEntry::recoverSerializedState() {
	_traits.recoverSerializedValues();
	_bIsOccupied = _bIsOccupied != 0 ? 1 : 0;
	if (_bIsOccupied != 0 && !_traits.isComplete())
		_bIsOccupied = 0;
	_name[ARRAYSIZE(_name) - 1] = 0;
}

Common::U32String ZmbStateActiveEntry::getU32Name(MohawkEngine_Zoombini *vm) const {
	const uint32 nameLength = getNameLength();
	if (ARRAYSIZE(_name) <= nameLength)
		return Common::U32String();
	return vm->_text->toU32String(_name, nameLength, ZoombiniText::kExeString);
}

void ZmbStateActiveEntry::setU32Name(MohawkEngine_Zoombini *vm, const Common::U32String &name) {
	const Common::String encodedName = vm->_text->fromU32String(name, ZoombiniText::kExeString);
	memset(_name, 0, sizeof(_name));
	const uint32 nameLength = MIN<uint32>(encodedName.size(), ARRAYSIZE(_name) - 1);
	memcpy(_name, encodedName.c_str(), nameLength);
}

uint32 ZmbStateActiveEntry::getNameLength() const {
	for (uint32 length = 0; length < ARRAYSIZE(_name); length++) {
		if (_name[length] == 0)
			return length;
	}
	return ARRAYSIZE(_name);
}

bool ZmbStateActiveEntry::hasSerializedName() const {
	return _name[0] != 0;
}

bool ZmbStateActiveEntry::hasNonAsciiName() const {
	for (uint32 i = 0; i < ARRAYSIZE(_name) && _name[i] != 0; i++) {
		if (0x7F < _name[i])
			return true;
	}
	return false;
}

void ZmbStateActivePack::sync(Common::Serializer &s, bool isTlcLayout) {
	s.syncAsSint16LE(_wPackZmbCount);
	s.syncAsSint16LE(_bSkipOccupiedEntries);
	s.syncAsSint16LE(_bSkipUnoccupiedEntries);
	for (int i = 0; i < ARRAYSIZE(_entries); i++)
		_entries[i].sync(s, isTlcLayout);
}

bool ZmbStateActivePack::isSerializedStateValid() const {
	if (!(0 <= _wPackZmbCount && _wPackZmbCount <= getEntryCapacity()) ||
		!(0 <= _bSkipOccupiedEntries && _bSkipOccupiedEntries <= 1) ||
		!(0 <= _bSkipUnoccupiedEntries && _bSkipUnoccupiedEntries <= 1))
		return false;

	for (int16 i = 0; i < getEntryCapacity(); i++) {
		if (!_entries[i].isSerializedStateValid())
			return false;
	}
	for (int16 i = 0; i < _wPackZmbCount; i++) {
		if (!_entries[i].getTraits().isComplete())
			return false;
	}
	return true;
}

void ZmbStateActivePack::recoverSerializedState() {
	const bool packCountValid = 0 <= _wPackZmbCount && _wPackZmbCount <= getEntryCapacity();
	_bSkipOccupiedEntries = _bSkipOccupiedEntries != 0 ? 1 : 0;
	_bSkipUnoccupiedEntries = _bSkipUnoccupiedEntries != 0 ? 1 : 0;
	for (int16 i = 0; i < getEntryCapacity(); i++)
		_entries[i].recoverSerializedState();

	bool usedEntriesComplete = packCountValid;
	if (packCountValid) {
		for (int16 i = 0; i < _wPackZmbCount; i++) {
			if (!_entries[i].getTraits().isComplete()) {
				usedEntriesComplete = false;
				break;
			}
		}
	}
	if (usedEntriesComplete)
		return;

	int16 entryLimit = _wPackZmbCount;
	if (!packCountValid) {
		entryLimit = 0;
		for (int16 i = 0; i < getEntryCapacity(); i++) {
			if (_entries[i].getIsOccupied() || _entries[i].getTraits().isComplete())
				entryLimit = i + 1;
		}
	}

	ZmbStateActivePack recoveredPack;
	recoveredPack.setSkipOccupiedEntries(getSkipOccupiedEntries());
	recoveredPack.setSkipUnoccupiedEntries(getSkipUnoccupiedEntries());
	for (int16 i = 0; i < entryLimit; i++) {
		if (_entries[i].getTraits().isComplete())
			recoveredPack.appendEntry(_entries[i]);
	}
	*this = recoveredPack;
}

bool ZmbRosterFile::appendNewSaveEntry(ZmbRosterEntry entry) {
	if (!isSerializedStateValid() || getEntryCapacity() <= _saveEntryCount || 9999 < _nextSaveFileNameCounter ||
		ARRAYSIZE(entry._saveName) <= entry.getSaveNameLength())
		return false;

	const Common::String fileName = Common::String::format("ZOOM%04u", static_cast<uint32>(_nextSaveFileNameCounter));
	for (int i = 0; i < _saveEntryCount; i++) {
		const Common::String saveFileNumber = _entries[i].getSaveFileNumStr();
		if (saveFileNumber.empty())
			return false;
		const Common::String existingFileName = Common::String::format("ZOOM%s", saveFileNumber.c_str());
		if (existingFileName == fileName)
			return false;
	}

	const uint32 fileNameLength = MIN<uint32>(fileName.size(), ARRAYSIZE(entry._fileName) - 1);
	memset(entry._fileName, 0, ARRAYSIZE(entry._fileName));
	memcpy(entry._fileName, fileName.c_str(), fileNameLength);

	_entries[_saveEntryCount] = entry;
	_saveEntryCount += 1;
	_nextSaveFileNameCounter += 1;
	return true;
}

void ZmbRosterFile::sync(Common::Serializer &r) {
	r.syncAsUint16LE(_magic006B);
	r.syncAsUint16LE(_nextSaveFileNameCounter);
	r.syncAsUint16LE(_saveEntryCount);
	for (int32 i = 0; i < ARRAYSIZE(_entries); i++) {
		_entries[i].sync(r);
	}
}

bool ZmbRosterFile::isSerializedStateValid() const {
	if (_magic006B != ZoombiniGameState::kEndianMagic || !hasValidEntryCount() ||
		!hasValidNextSaveFileNameCounter())
		return false;

	for (int16 i = 0; i < getEntryCount(); i++) {
		if (!_entries[i].isSerializedStateValid())
			return false;
	}
	return true;
}

bool ZmbRosterFile::hasValidEntryCount() const {
	return _saveEntryCount <= getEntryCapacityForFileSize(ZoombiniGameState::kZmbRosterFileSize);
}

uint16 ZmbRosterFile::getEntryCount() const {
	return MIN<uint16>(_saveEntryCount, getEntryCapacityForFileSize(ZoombiniGameState::kZmbRosterFileSize));
}

void ZmbRosterEntry::sync(Common::Serializer &r) {
	r.syncBytes(_saveName, ARRAYSIZE(_saveName));
	r.syncBytes(_fileName, ARRAYSIZE(_fileName));
}

Common::U32String ZmbRosterEntry::getSaveName(MohawkEngine_Zoombini *vm) const {
	const uint16 saveNameLength = getSaveNameLength();
	if (ARRAYSIZE(_saveName) <= saveNameLength)
		return Common::U32String();
	return vm->_text->toU32String(_saveName, saveNameLength, ZoombiniText::kExeString);
}

Common::U32String ZmbRosterEntry::getSaveName(Common::CodePage codePage) const {
	const uint16 saveNameLength = getSaveNameLength();
	if (ARRAYSIZE(_saveName) <= saveNameLength)
		return Common::U32String();
	const Common::String encodedName(reinterpret_cast<const char *>(_saveName), saveNameLength);
	return Common::U32String(encodedName, codePage);
}

uint16 ZmbRosterEntry::getSaveNameLength() const {
	for (uint16 length = 0; length < ARRAYSIZE(_saveName); length++) {
		if (_saveName[length] == 0)
			return length;
	}
	return static_cast<uint16>(ARRAYSIZE(_saveName));
}

uint16 ZmbRosterEntry::getFileNameLength() const {
	for (uint16 length = 0; length < ARRAYSIZE(_fileName); length++) {
		if (_fileName[length] == 0)
			return length;
	}
	return static_cast<uint16>(ARRAYSIZE(_fileName));
}

Common::String ZmbRosterEntry::getSaveFileStem() const {
	const uint16 fileNameLength = getFileNameLength();
	if (fileNameLength != ARRAYSIZE(_fileName) - 1)
		return Common::String();

	const Common::String fileStem(reinterpret_cast<const char *>(_fileName), fileNameLength);
	if (fileStem.size() != 8 || fileStem.substr(0, 4) != "ZOOM")
		return Common::String();

	for (uint i = 4; i < fileStem.size(); i++) {
		if (fileStem[i] < '0' || '9' < fileStem[i])
			return Common::String();
	}

	return fileStem;
}

Common::String ZmbRosterEntry::getSaveFileNumStr() const {
	const Common::String fileStem = getSaveFileStem();
	return fileStem.empty() ? Common::String() : fileStem.substr(4, 4);
}

bool ZmbRosterEntry::isSerializedStateValid() const {
	return getSaveNameLength() < ARRAYSIZE(_saveName) && !getSaveFileStem().empty();
}

bool ZmbRosterEntry::isSaveNameEncodingValid(const Common::U32String &uSaveName, Common::CodePage codePage) {
	Common::String encodedSaveName;
	if (uSaveName.encode(encodedSaveName, codePage, '?') != Common::kStringEncodingResultSucceeded)
		return false;

	const Common::U32String roundTrip(encodedSaveName, codePage);
	return roundTrip.equals(uSaveName);
}

bool ZmbRosterEntry::encodeSaveName(const Common::U32String &uSaveName, Common::CodePage codePage, Common::String &encodedSaveName) {
	if (!isSaveNameEncodingValid(uSaveName, codePage))
		return false;

	if (uSaveName.encode(encodedSaveName, codePage, '?') != Common::kStringEncodingResultSucceeded)
		return false;

	return encodedSaveName.size() < ARRAYSIZE(_saveName);
}

bool ZmbRosterEntry::checkSaveNameSize(MohawkEngine_Zoombini *vm, const Common::U32String &uSaveName) {
	Common::String encodedSaveName;
	return encodeSaveName(uSaveName, vm->_text->getExeCodePage(), encodedSaveName);
}

ZoombiniPageType ZmbStateFile::getCurrentPageType() const {
	switch (_currentPage) {
	case ZmbDestPageKind::kMap_01:
		return ZoombiniPageType::kRodMap;
	case ZmbDestPageKind::kIsle_03:
		return ZoombiniPageType::kPicker;
	case ZmbDestPageKind::kBC1_04:
		return ZoombiniPageType::kBasecamp1;
	case ZmbDestPageKind::kBC2_05:
		return ZoombiniPageType::kBasecamp2;
	case ZmbDestPageKind::kTown_06:
		return ZoombiniPageType::kTown;
	case ZmbDestPageKind::kBridge_07:
		return ZoombiniPageType::kBridge;
	case ZmbDestPageKind::kTunnels_08:
		return ZoombiniPageType::kTunnels;
	case ZmbDestPageKind::kPizza_09:
		return ZoombiniPageType::kPizza;
	case ZmbDestPageKind::kFerry_10:
		return ZoombiniPageType::kFerry;
	case ZmbDestPageKind::kLilly_11:
		return ZoombiniPageType::kLilly;
	case ZmbDestPageKind::kSlides_12:
		return ZoombiniPageType::kSlides;
	case ZmbDestPageKind::kFleens_13:
		return ZoombiniPageType::kFleens;
	case ZmbDestPageKind::kHotel_14:
		return ZoombiniPageType::kHotel;
	case ZmbDestPageKind::kNet_15:
		return ZoombiniPageType::kNet;
	case ZmbDestPageKind::kCaves_16:
		return ZoombiniPageType::kCaves;
	case ZmbDestPageKind::kSmoke_17:
		return ZoombiniPageType::kSmoke;
	case ZmbDestPageKind::kMaze_18:
		return ZoombiniPageType::kMaze;
	default:
		error("state: invalid currentPage value: %d", static_cast<int16>(_currentPage));
		return ZoombiniPageType::kNone;
	}
}

void ZmbStateFile::setCurrentPageType(ZoombiniPageType pageType) {
	ZmbDestPageKind lastPage = _currentPage;

	switch (pageType) {
	case ZoombiniPageType::kRodMap:
		_currentPage = ZmbDestPageKind::kMap_01;
		break;
	case ZoombiniPageType::kPicker:
		_currentPage = ZmbDestPageKind::kIsle_03;
		break;
	case ZoombiniPageType::kBasecamp1:
		_currentPage = ZmbDestPageKind::kBC1_04;
		break;
	case ZoombiniPageType::kBasecamp2:
		_currentPage = ZmbDestPageKind::kBC2_05;
		break;
	case ZoombiniPageType::kTown:
		_currentPage = ZmbDestPageKind::kTown_06;
		break;
	case ZoombiniPageType::kBridge:
		_currentPage = ZmbDestPageKind::kBridge_07;
		break;
	case ZoombiniPageType::kTunnels:
		_currentPage = ZmbDestPageKind::kTunnels_08;
		break;
	case ZoombiniPageType::kPizza:
		_currentPage = ZmbDestPageKind::kPizza_09;
		break;
	case ZoombiniPageType::kFerry:
		_currentPage = ZmbDestPageKind::kFerry_10;
		break;
	case ZoombiniPageType::kLilly:
		_currentPage = ZmbDestPageKind::kLilly_11;
		break;
	case ZoombiniPageType::kSlides:
		_currentPage = ZmbDestPageKind::kSlides_12;
		break;
	case ZoombiniPageType::kFleens:
		_currentPage = ZmbDestPageKind::kFleens_13;
		break;
	case ZoombiniPageType::kHotel:
		_currentPage = ZmbDestPageKind::kHotel_14;
		break;
	case ZoombiniPageType::kNet:
		_currentPage = ZmbDestPageKind::kNet_15;
		break;
	case ZoombiniPageType::kCaves:
		_currentPage = ZmbDestPageKind::kCaves_16;
		break;
	case ZoombiniPageType::kSmoke:
		_currentPage = ZmbDestPageKind::kSmoke_17;
		break;
	case ZoombiniPageType::kMaze:
		_currentPage = ZmbDestPageKind::kMaze_18;
		break;
	default:
		error("state: invalid pageType value: %d", static_cast<int32>(pageType));
		break;
	}

	if (lastPage != _currentPage)
		_v2PreviousPage = lastPage;
	_isDirty |= (lastPage != _currentPage);
}

void ZmbStateFile::setRouteCompletionFlag(ZmbRouteId routeId, int16 routeLevel) {
	if (ZmbRouteId::kMontDespair < routeId)
		return;

	const byte bitmask = static_cast<byte>(1 << (routeLevel & 3));
	switch (routeId) {
	case ZmbRouteId::kBigBadHungry:
		_levelFlagRouteBigBadHungry |= bitmask;
		break;
	case ZmbRouteId::kWhosBayou:
		_levelFlagLoWhosBayouHiDeepDarkForest |= bitmask;
		break;
	case ZmbRouteId::kDeepDarkForest:
		_levelFlagLoWhosBayouHiDeepDarkForest |= static_cast<byte>(bitmask << 4);
		break;
	case ZmbRouteId::kMontDespair:
		_levelFlagRouteMontDespair |= bitmask;
		break;
	default:
		break;
	}
}

void ZmbStateFile::sync(Common::Serializer &s, bool isTlcLayout, bool hasCompletionCounters) {
	// 0x0000: The state file stores the magic and auto-sticky delay big-endian
	// (on-disk bytes 00 6B and 00 1E).
	// Every field from 0x0004 onward is little-endian.
	s.syncAsUint16BE(_magic006B);
	s.syncAsUint16BE(_autoStickyDelay);

	// 0x0004: Flags
	s.syncAsByte(_flagSfxEnable);
	s.syncAsByte(_flagBgmEnable);
	s.syncAsByte(_flagStickyMouseEnable);
	s.syncAsByte(_flagCursorVisible);
	s.syncAsByte(_flagDebug);
	s.syncAsByte(_flagAutoStickyMouse);
	if (isTlcLayout) {
		// TLC adds the eight-byte Fleens transform tables at 0x000E-0x0015.
		// The v1 header has six corresponding bytes, so later scalar fields move by two bytes.
		s.syncAsByte(_tlcTouchSenseEnable);
		s.syncAsByte(_tlcHelpAudioEnable);
		s.syncAsUint16LE(_v2TransitionsDisable);
		for (int32 i = 0; i < ARRAYSIZE(_v2FleensTraitValueRotations); i++)
			s.syncAsByte(_v2FleensTraitValueRotations[i]);
		for (int32 i = 0; i < ARRAYSIZE(_v2FleensTraitDestSlots); i++)
			s.syncAsByte(_v2FleensTraitDestSlots[i]);
	} else {
		s.syncAsUint16LE(_v1TransitionsDisable);
		for (int32 i = 0; i < ARRAYSIZE(_v1FleensTraitValueRotations); i++)
			s.syncAsByte(_v1FleensTraitValueRotations[i]);
		for (int32 i = 0; i < ARRAYSIZE(_v1FleensTraitDestSlots); i++)
			s.syncAsByte(_v1FleensTraitDestSlots[i]);
	}
	for (int32 i = 0; i < ARRAYSIZE(_bcOneMushroomColors); i++)
		s.syncAsUint16LE(_bcOneMushroomColors[i]);
	s.syncAsUint16LE(_townScrollCol);
	s.syncAsUint16LE(_lessActionFlag);
	s.syncAsUint16LE(_fleensHighScore);
	s.syncAsUint16LE(_mudballHighScore);
	s.syncAsUint16LE(_pickerWaveBoatAnimationState);

	// v1.x 0x0028 / TLC 0x002A: Page Flags
	_pageFlagIsle.sync(s);
	_pageFlagBridge.sync(s);
	_pageFlagTunnels.sync(s);
	_pageFlagPizza.sync(s);
	_pageFlagBasecamp1.sync(s);
	_pageFlagFerry.sync(s);
	_pageFlagLilly.sync(s);
	_pageFlagSlides.sync(s);
	_pageFlagFleens.sync(s);
	_pageFlagHotel.sync(s);
	_pageFlagNet.sync(s);
	_pageFlagBasecamp2.sync(s);
	_pageFlagCaves.sync(s);
	_pageFlagSmoke.sync(s);
	_pageFlagMaze.sync(s);
	_pageFlagTown.sync(s);

	// v1.x 0x0048 / TLC 0x004A: Generated and Stored Zoombini Count
	s.syncAsSint16LE(_zmbGeneratedCount);
	s.syncAsSint16LE(_zmbStoredBC1Count);
	s.syncAsSint16LE(_zmbStoredBC2Count);
	s.syncAsSint16LE(_zmbStoredTownCount);

	// v1.x 0x0050 / TLC 0x0052: Level Flags
	s.syncAsByte(_levelFlagRouteBigBadHungry);
	s.syncAsByte(_levelFlagRouteMontDespair);
	s.syncAsByte(_levelFlagLoWhosBayouHiDeepDarkForest);
	s.syncBytes(_pageLevelFlags, ARRAYSIZE(_pageLevelFlags));

	// v1.x 0x0062 / TLC 0x0064: Memorial Stone Records
	for (int32 i = 0; i < ARRAYSIZE(_memorialYears); i++)
		s.syncAsUint16LE(_memorialYears[i]);
	for (int32 i = 0; i < ARRAYSIZE(_memorialMonths); i++)
		s.syncAsByte(_memorialMonths[i]);
	for (int32 i = 0; i < ARRAYSIZE(_memorialDays); i++)
		s.syncAsByte(_memorialDays[i]);
	for (int32 i = 0; i < ARRAYSIZE(_memorialRoutes); i++)
		s.syncAsByte(_memorialRoutes[i]);
	for (int32 i = 0; i < ARRAYSIZE(_memorialLevels); i++)
		s.syncAsByte(_memorialLevels[i]);

	// v1.x 0x00C2 / TLC 0x00C4: Route Levels
	for (int32 i = 0; i < ARRAYSIZE(_routeLevels); i++) {
		s.syncAsSint16LE(_routeLevels[i]);
	}
	if (!isTlcLayout)
		s.syncAsSint16LE(_currentRoute);
	int16 currentPage = static_cast<int16>(_currentPage);
	if (isTlcLayout) {
		// TLC moved the current page from 0x00CC to 0x00CE.
		// It stores the previous page at 0x00CC.
		// Keep @ref ZmbStateFile::_currentPage independent of the source layout.
		int16 tlcPreviousPage = static_cast<int16>(_v2PreviousPage);
		s.syncAsSint16LE(tlcPreviousPage);
		s.syncAsSint16LE(currentPage);

		if (s.isLoading()) {
			_v2PreviousPage = static_cast<ZmbDestPageKind>(tlcPreviousPage);
			_currentPage = static_cast<ZmbDestPageKind>(currentPage);
			_currentRoute = static_cast<int16>(_v2PreviousPage);
		}
	} else {
		s.syncAsSint16LE(currentPage);
		if (s.isLoading())
			_currentPage = static_cast<ZmbDestPageKind>(currentPage);
	}

	// v1.x 0x00CE / TLC 0x00D0: Stored Zoombinis on Basecamp 1
	_storedChunkBC1.sync(s, isTlcLayout);

	// v1.x 0x3688 / TLC 0x3B6C: Stored Zoombinis on Basecamp 2
	_storedChunkBC2.sync(s, isTlcLayout);

	// v1.x 0x6C42 / TLC 0x7608: Stored Zoombinis on Town
	_storedChunkTown.sync(s, isTlcLayout);

	// v1.x 0xA1FC / TLC 0xB0A4: Active Zoombini Packs
	_zmbPackIsle.sync(s, isTlcLayout);
	_zmbPackBC1.sync(s, isTlcLayout);
	_zmbPackBC2.sync(s, isTlcLayout);
	_zmbPackActive.sync(s, isTlcLayout);

	// v1.x 0xAB94 / TLC 0xBABC: Zoombini Twin Status
	s.syncBytes(_twinGenStatus, ARRAYSIZE(_twinGenStatus));
	if (isTlcLayout)
		s.syncAsByte(_v2TwinGenStatusPad);

	if (hasCompletionCounters) {
		// v1.x 0xAE05 / TLC 0xBD2E: Per-route perfect completion counters.
		for (int32 i = 0; i < ARRAYSIZE(_routePerfectCounters); i++) {
			s.syncAsSint16LE(_routePerfectCounters[i]);
		}
		s.syncAsSint16LE(_townDevelopLevel);
	}

	// v1.x 0xAE0F / TLC 0xBD38: EOF
}

ZmbSfxGroupFlags ZoombiniGameState::getSfxGroupFlagsFromPageFlag(ZmbStateFile::PageFlag &pageFlag) {
	return getSfxGroupFlagsFromPageFlag(pageFlag, getCurrentState().getCurrentPageType());
}

ZmbSfxGroupFlags ZoombiniGameState::getSfxGroupFlagsFromPageFlag(ZmbStateFile::PageFlag &pageFlag,
																 ZoombiniPageType pageType) {
	// Practice mode has its own SFX selector and does not update the page flag.
	if (0 < _practiceLevel)
		return ZmbSfxGroupFlags::kPractice_05;

	// Increment the saturating visit count in the low 12 bits.
	pageFlag.incVisitCount();

	const int16 routeLevel = readPageRouteLevel(pageType);
	// Group 1 represents Puzzle Level 1.
	if (routeLevel == 0)
		return ZmbSfxGroupFlags::kEasy_01;

	// Group 2 represents Puzzle Levels 2-4. Its first two selectors are emitted
	// while the internal route level is 1; later visits use random selection.
	if (routeLevel == 1) {
		if (pageFlag.hasFirstHardGroupTrigger()) {
			if (!pageFlag.hasSecondHardGroupTrigger()) {
				pageFlag.setSecondHardGroupTrigger();
				return ZmbSfxGroupFlags::kHardSecondTrigger_12;
			}
		} else {
			pageFlag.setFirstHardGroupTrigger();
			return ZmbSfxGroupFlags::kHardFirstTrigger_02;
		}
	}

	// Later Group 2 visits use random selection.
	return ZmbSfxGroupFlags::kRandom_00;
}

ZmbStateFile::PageFlag &ZmbStateFile::getPageFlagFromPageType(ZoombiniPageType pageType) {
	switch (pageType) {
	case ZoombiniPageType::kPicker:
		return _pageFlagIsle;
	case ZoombiniPageType::kBridge:
		return _pageFlagBridge;
	case ZoombiniPageType::kCaves:
		return _pageFlagCaves;
	case ZoombiniPageType::kPizza:
		return _pageFlagPizza;
	case ZoombiniPageType::kBasecamp1:
		return _pageFlagBasecamp1;
	case ZoombiniPageType::kFerry:
		return _pageFlagFerry;
	case ZoombiniPageType::kLilly:
		return _pageFlagLilly;
	case ZoombiniPageType::kSlides:
		return _pageFlagSlides;
	case ZoombiniPageType::kFleens:
		return _pageFlagFleens;
	case ZoombiniPageType::kHotel:
		return _pageFlagHotel;
	case ZoombiniPageType::kNet:
		return _pageFlagNet;
	case ZoombiniPageType::kBasecamp2:
		return _pageFlagBasecamp2;
	case ZoombiniPageType::kTunnels:
		return _pageFlagTunnels;
	case ZoombiniPageType::kSmoke:
		return _pageFlagSmoke;
	case ZoombiniPageType::kMaze:
		return _pageFlagMaze;
	case ZoombiniPageType::kTown:
		return _pageFlagTown;
	default:
		error("state: invalid pageType: %u", static_cast<uint32>(pageType));
		return _pageFlagIsle; // Avoid compiler warning
	}
}

ZmbSfxGroupFlags ZoombiniGameState::getSfxGroupFlagsFromPageType(ZoombiniPageType pageType) {
	ZmbStateFile::PageFlag &pageFlag = getCurrentState().getPageFlagFromPageType(pageType);
	return getSfxGroupFlagsFromPageFlag(pageFlag, pageType);
}

int16 ZoombiniGameState::readActivePageRouteLevel() {
	return readPageRouteLevel(getCurrentState().getCurrentPageType());
}

int16 ZoombiniGameState::readPageRouteLevel(ZoombiniPageType pageType) {
	if (1 <= _practiceLevel && _practiceLevel <= 4) {
		return _practiceLevel - 1;
	}

	const ZmbStateFile &state = getCurrentState();

	// Puzzle pages 7-18 map in groups of three to @ref ZmbStateFile::_routeLevels.
	// Pages 7-9 use route 0, 10-12 route 1, 13-15 route 2, and 16-18 route 3.
	if (ZoombiniPageType::kBridge <= pageType && pageType <= ZoombiniPageType::kMaze) {
		const uint16 routeIdx =
			(static_cast<uint16>(pageType) - static_cast<uint16>(ZoombiniPageType::kBridge)) / 3;
		return state._routeLevels[routeIdx];
	}

	// Container/storage pages
	switch (pageType) {
	case ZoombiniPageType::kPicker:
	case ZoombiniPageType::kBasecamp1: // BIG BAD AND HUNGRY
		return state._routeLevels[0];
	case ZoombiniPageType::kBasecamp2: // WHO'S BAYOU or DEEP DARK FOREST
		return MAX(state._routeLevels[1], state._routeLevels[2]);
	case ZoombiniPageType::kTown: // MOUNTAIN OF DESPAIR
		return state._routeLevels[3];
	default:
		break;
	}
	return 0;
}

int16 ZoombiniGameState::readActivePageRouteId() {
	const ZmbDestPageKind currentPage = getCurrentState()._currentPage;
	if (currentPage < ZmbDestPageKind::kBridge_07 || ZmbDestPageKind::kMaze_18 < currentPage)
		return -1;
	return (static_cast<int16>(currentPage) - static_cast<int16>(ZmbDestPageKind::kBridge_07)) / 3;
}

int16 ZoombiniGameState::readRouteLevel(ZmbRouteId routeId) {
	const ZmbStateFile &state = getCurrentState();
	switch (routeId) {
	case ZmbRouteId::kBigBadHungry:
		return state._routeLevels[0];
	case ZmbRouteId::kWhosBayou:
		return state._routeLevels[1];
	case ZmbRouteId::kDeepDarkForest:
		return state._routeLevels[2];
	case ZmbRouteId::kMontDespair:
		return state._routeLevels[3];
	default:
		error("state: invalid route level for routeId(%hd)", static_cast<int16>(routeId));
		break;
	}
	return 0;
}

bool ZoombiniGameState::isValidMemorialDate(uint16 year, byte month, byte day) {
	if (year < 1 || 9999 < year || month < 1 || 12 < month || day < 1 || 31 < day)
		return false;

	static constexpr byte daysInMonth[12] = {
		31,
		28,
		31,
		30,
		31,
		30,
		31,
		31,
		30,
		31,
		30,
		31,
	};
	byte maximumDay = daysInMonth[month - 1];
	const bool leapYear = year % 400 == 0 || (year % 4 == 0 && year % 100 != 0);
	if (month == 2 && leapYear)
		maximumDay = 29;

	return day <= maximumDay;
}

int16 ZoombiniGameState::findFirstClearMemorialSlot(ZmbRouteId routeId, int16 difficultyLevel) const {
	if (ZmbRouteId::kMontDespair < routeId || difficultyLevel < 1 || 4 < difficultyLevel)
		return -1;

	const ZmbStateFile &state = getCurrentState();
	const byte memorialRoute = static_cast<byte>(static_cast<uint16>(routeId) + 1);
	const byte memorialLevel = static_cast<byte>(difficultyLevel);
	for (uint16 slotIdx = 0; slotIdx < ARRAYSIZE(state._memorialRoutes); slotIdx += 1) {
		if (state._memorialRoutes[slotIdx] == memorialRoute &&
			state._memorialLevels[slotIdx] == memorialLevel)
			return static_cast<int16>(slotIdx);
	}

	return -1;
}

bool ZoombiniGameState::addFirstClearMemorial(ZmbRouteId routeId, int16 difficultyLevel, uint16 year, byte month, byte day) {
	if (ZmbRouteId::kMontDespair < routeId || difficultyLevel < 1 || 4 < difficultyLevel ||
		!isValidMemorialDate(year, month, day))
		return false;

	if (0 <= findFirstClearMemorialSlot(routeId, difficultyLevel))
		return false;

	const byte memorialRoute = static_cast<byte>(static_cast<uint16>(routeId) + 1);
	const byte memorialLevel = static_cast<byte>(difficultyLevel);
	ZmbStateFile &state = getCurrentState();

	for (uint16 slotIdx = 0; slotIdx < ARRAYSIZE(state._memorialRoutes); slotIdx += 1) {
		if (state._memorialRoutes[slotIdx] != 0)
			continue;

		state._memorialYears[slotIdx] = year;
		state._memorialMonths[slotIdx] = month;
		state._memorialDays[slotIdx] = day;
		state._memorialRoutes[slotIdx] = memorialRoute;
		state._memorialLevels[slotIdx] = memorialLevel;
		return true;
	}

	return false;
}

void ZoombiniGameState::clearFirstClearMemorial(uint16 slotIdx) {
	ZmbStateFile &state = getCurrentState();
	if (ARRAYSIZE(state._memorialRoutes) <= slotIdx)
		return;

	state._memorialYears[slotIdx] = 0;
	state._memorialMonths[slotIdx] = 0;
	state._memorialDays[slotIdx] = 0;
	state._memorialRoutes[slotIdx] = 0;
	state._memorialLevels[slotIdx] = 0;
}

bool ZoombiniGameState::recordFirstClearMemorial(ZmbRouteId routeId, int16 difficultyLevel) {
	TimeDate date;
	g_system->getTimeAndDate(date);
	return addFirstClearMemorial(routeId, difficultyLevel,
								 static_cast<uint16>(date.tm_year + 1900),
								 static_cast<byte>(date.tm_mon + 1),
								 static_cast<byte>(date.tm_mday));
}

bool ZoombiniGameState::readMemorialActive(ZmbRouteId routeId, int16 difficultyLevel) const {
	return 0 <= findFirstClearMemorialSlot(routeId, difficultyLevel);
}

bool ZoombiniGameState::readMemorialDate(ZmbRouteId routeId, int16 difficultyLevel, uint16 &year, byte &month, byte &day) const {
	year = 0;
	month = 0;
	day = 0;

	const int16 slotIdx = findFirstClearMemorialSlot(routeId, difficultyLevel);
	if (slotIdx < 0)
		return false;

	const ZmbStateFile &state = getCurrentState();
	year = state._memorialYears[slotIdx];
	month = state._memorialMonths[slotIdx];
	day = state._memorialDays[slotIdx];
	return true;
}

bool ZoombiniGameState::setMemorialActive(ZmbRouteId routeId, int16 difficultyLevel, bool active) {
	if (ZmbRouteId::kMontDespair < routeId || difficultyLevel < 1 || 4 < difficultyLevel)
		return false;

	const int16 slotIdx = findFirstClearMemorialSlot(routeId, difficultyLevel);
	if (active) {
		if (0 <= slotIdx)
			return true;

		return recordFirstClearMemorial(routeId, difficultyLevel);
	}

	if (slotIdx < 0)
		return true;

	clearFirstClearMemorial(static_cast<uint16>(slotIdx));
	return true;
}

bool ZoombiniGameState::setMemorialDate(ZmbRouteId routeId, int16 difficultyLevel, uint16 year, byte month, byte day) {
	if (ZmbRouteId::kMontDespair < routeId || difficultyLevel < 1 || 4 < difficultyLevel ||
		!isValidMemorialDate(year, month, day))
		return false;

	const int16 slotIdx = findFirstClearMemorialSlot(routeId, difficultyLevel);
	if (slotIdx < 0)
		return addFirstClearMemorial(routeId, difficultyLevel, year, month, day);

	ZmbStateFile &state = getCurrentState();
	state._memorialYears[slotIdx] = year;
	state._memorialMonths[slotIdx] = month;
	state._memorialDays[slotIdx] = day;
	return true;
}

void ZoombiniGameState::syncRouteProgressFlags(ZmbRouteId routeId, int16 difficultyLevel) {
	if (ZmbRouteId::kMontDespair < routeId || difficultyLevel < 1 || 4 < difficultyLevel)
		return;

	// The selected level is its starting state, so only lower levels are complete.
	const uint routeIndex = static_cast<uint>(routeId);
	const byte completedLevelMask = static_cast<byte>((1 << (difficultyLevel - 1)) - 1);
	ZmbStateFile &state = getCurrentState();

	// Page flags store ordinary clears in the low nibble and perfect clears in the high nibble.
	const byte pageFlags = static_cast<byte>(completedLevelMask | (completedLevelMask << 4));

	// Each route owns three consecutive puzzle flags after the first three reserved entries.
	const uint firstPageFlagIndex = 3 + routeIndex * 3;
	for (uint puzzleIndex = 0; puzzleIndex < 3; puzzleIndex++)
		state._pageLevelFlags[firstPageFlagIndex + puzzleIndex] = pageFlags;

	// Endpoint flags use the same mask; Routes 2 and 3 occupy opposite nibbles of one byte.
	switch (routeId) {
	case ZmbRouteId::kBigBadHungry:
		state._levelFlagRouteBigBadHungry = static_cast<byte>((state._levelFlagRouteBigBadHungry & 0xF0) | completedLevelMask);
		break;
	case ZmbRouteId::kWhosBayou:
		state._levelFlagLoWhosBayouHiDeepDarkForest = static_cast<byte>((state._levelFlagLoWhosBayouHiDeepDarkForest & 0xF0) | completedLevelMask);
		break;
	case ZmbRouteId::kDeepDarkForest:
		state._levelFlagLoWhosBayouHiDeepDarkForest = static_cast<byte>((state._levelFlagLoWhosBayouHiDeepDarkForest & 0x0F) | (completedLevelMask << 4));
		break;
	case ZmbRouteId::kMontDespair:
		state._levelFlagRouteMontDespair = static_cast<byte>((state._levelFlagRouteMontDespair & 0xF0) | completedLevelMask);
		break;
	default:
		break;
	}
}

void ZoombiniGameState::syncRouteFirstClearMemorials(ZmbRouteId routeId, int16 difficultyLevel, bool removeAtOrAboveTarget) {
	if (ZmbRouteId::kMontDespair < routeId || difficultyLevel < 1 || 4 < difficultyLevel)
		return;

	// Memorial records use one-based route values.
	const byte memorialRoute = static_cast<byte>(static_cast<uint16>(routeId) + 1);
	const ZmbStateFile &state = getCurrentState();

	// A level decrease removes this route's records at the selected level and above.
	// Malformed zero-level records are removed during the same cleanup.
	for (uint16 slotIdx = 0; slotIdx < ARRAYSIZE(state._memorialRoutes); slotIdx += 1) {
		if (state._memorialRoutes[slotIdx] != memorialRoute)
			continue;

		const int16 memorialLevel = state._memorialLevels[slotIdx];
		if (removeAtOrAboveTarget && (memorialLevel == 0 || difficultyLevel <= memorialLevel))
			clearFirstClearMemorial(slotIdx);
	}

	// Ensure every lower level has a memorial, using one date for newly added records.
	TimeDate date;
	g_system->getTimeAndDate(date);
	const uint16 year = static_cast<uint16>(date.tm_year + 1900);
	const byte month = static_cast<byte>(date.tm_mon + 1);
	const byte day = static_cast<byte>(date.tm_mday);

	for (int16 memorialLevel = 1; memorialLevel < difficultyLevel; memorialLevel += 1)
		addFirstClearMemorial(routeId, memorialLevel, year, month, day);
}

bool ZoombiniGameState::advanceRouteLevel(ZmbRouteId routeId) {
	if (ZmbRouteId::kMontDespair < routeId)
		return false;

	const uint routeIndex = static_cast<uint>(routeId);
	ZmbStateFile &state = getCurrentState();
	int16 &routeLevel = state._routeLevels[routeIndex];
	if (3 <= routeLevel)
		return false;

	routeLevel += 1;
	if (_vm->hasRoutePerfectCounterState())
		state._routePerfectCounters[routeIndex] = 0;
	_routeLevelJustAdvanced = true;
	return true;
}

bool ZoombiniGameState::setRouteDifficultyLevel(ZmbRouteId routeId, int16 difficultyLevel) {
	if (ZmbRouteId::kMontDespair < routeId || difficultyLevel < 1 || 4 < difficultyLevel)
		return false;

	const uint routeIndex = static_cast<uint>(routeId);
	ZmbStateFile &state = getCurrentState();
	int16 &routeLevel = state._routeLevels[routeIndex];
	const int16 targetRouteLevel = difficultyLevel - 1;
	if (routeLevel == targetRouteLevel)
		return true;

	const bool routeLevelDecreased = targetRouteLevel < routeLevel;
	if (routeLevel < targetRouteLevel) {
		while (routeLevel < targetRouteLevel) {
			if (!advanceRouteLevel(routeId))
				return false;
		}
	} else {
		routeLevel = targetRouteLevel;
	}

	if (_vm->hasRoutePerfectCounterState())
		state._routePerfectCounters[routeIndex] = 0;
	syncRouteProgressFlags(routeId, difficultyLevel);
	syncRouteFirstClearMemorials(routeId, difficultyLevel, routeLevelDecreased);

	// A debugger level change does not sit inside executeDeparture(). Do not
	// leave its one-shot completion marker for a later real puzzle departure.
	_routeLevelJustAdvanced = false;

	return true;
}

bool ZoombiniGameState::isNextPageContainer() {
	const ZmbDestPageKind currentPage = getCurrentState()._currentPage;
	return currentPage == ZmbDestPageKind::kPizza_09 || currentPage == ZmbDestPageKind::kSlides_12 ||
		   currentPage == ZmbDestPageKind::kNet_15 || currentPage == ZmbDestPageKind::kMaze_18;
}

void ZoombiniGameState::markXferContainerArrival(ZmbSrcPageKind srcPage, const ZmbXferRouteInfo &routeInfo) {
	if (routeInfo.entersContainer())
		_lastPageBeforeContainer = static_cast<uint16>(srcPage);
}

ZmbRosterEntry *ZoombiniGameState::getActiveSaveRosterEntry() {
	if (_r.isSerializedStateValid() && _r.hasEntry(_currentSaveSlot))
		return &_r.getEntry(_currentSaveSlot);

	// Unsaved new game
	if (_currentSaveSlot == kUnsavedNewGame)
		return nullptr;

	warning("Invalid current Zoombini save slot: %d", _currentSaveSlot);
	return nullptr;
}

Common::U32String ZoombiniGameState::getActiveSaveName() {
	if (_currentSaveSlot == kUnsavedNewGame)
		return _vm->_text->getLocalizedString(ZoombiniText::kNewGame);

	ZmbRosterEntry *entry = _vm->_state->getActiveSaveRosterEntry();
	return entry ? entry->getSaveName(_vm) : Common::U32String();
}

int ZoombiniGameState::searchSaveSlotByName(const Common::U32String &saveName) {
	if (!_r.isSerializedStateValid())
		return -1;

	for (uint16 i = 0; i < _r.getEntryCount(); i++) {
		const ZmbRosterEntry &entry = _r.getEntry(i);

		// Zoombini save names are case sensitive
		if (entry.getSaveName(_vm).equals(saveName))
			return i;
	}

	return -1;
}

int ZoombiniGameState::getAvailableSaveSlot() {
	if (_r.isSerializedStateValid() && _r.getEntryCount() < _r.getEntryCapacity())
		return _r.getEntryCount();

	// No available slot
	return -1;
}

void ZoombiniGameState::startNewGame(bool askSaveCurrentGame) {
	ZoombiniPage *activePage = _vm->getActivePage();
	if (!activePage || activePage->getPageCategory() != ZoombiniPageCategory::kInteractive || _journeyState._zmbGeneratedCount == 0)
		return;

	// Ctrl+N owns the save-current-game preflight. The Options New button does not.
	if (askSaveCurrentGame &&
		_vm->openMsgBoxDialog(ZoombiniMsgBoxType::kAskSaveCurrentGame) == ZoombiniDialogResult::kYes)
		_vm->openSaveDialog();

	if (inPracticeMode()) {
		_vm->openMsgBoxDialog(ZoombiniMsgBoxType::kAlertCannotCreateNewInPractice);
		return;
	}

	ZoombiniMsgBoxType promptType;
	if (needsSaveBeforeQuit())
		promptType = ZoombiniMsgBoxType::kAskCreateAndSaveNewGame;
	else
		promptType = ZoombiniMsgBoxType::kAskCreateNewGame;
	ZoombiniDialogResult result = _vm->openMsgBoxDialog(promptType);
	if (result != ZoombiniDialogResult::kYes) {
		return;
	}

	// Initialize a new game state
	_journeyState = ZmbStateFile();
	initVariantDefaults();
	_journeyState._isDirty = true;
	clearDebugStateMutation();
	_debugUnsafeSyntheticStateFlag = false;
	_currentSaveSlot = kUnsavedNewGame;
	_gameStateReadyFlag = true;
	markSaveBeforeQuitPending();
	_ferryRuntimeState.resetForNewGame();
	_vm->syncSoundSettings();

	endPracticeState();
	_practiceLevel = 0;
	ZoombiniPageType nextPageType;
	if (activePage->getPageType() == ZoombiniPageType::kRodMap)
		nextPageType = ZoombiniPageType::kRodMap;
	else
		nextPageType = ZoombiniPageType::kPicker;
	_vm->setNextPage(nextPageType);
	activePage->close();
}

void ZoombiniGameState::initVariantDefaults() {
	if (!_vm->isVersionFamilyTlcV2())
		return;

	_journeyState.setTouchSenseEnabled(true);
	_journeyState.setHelpAudioEnabled(true);
	_journeyState.setV2TransitionsDisabled(false);
}

bool ZoombiniGameState::getEnableTransitions() {
	if (_vm->isVersionFamilyTlcV2())
		return !_journeyState.getV2TransitionsDisabled();

	return !_journeyState.getV1TransitionsDisabled();
}

bool ZoombiniGameState::getEnableTouchSense() {
	return _vm->isVersionFamilyTlcV2() && _journeyState.getTouchSenseEnabled();
}

bool ZoombiniGameState::getEnableHelpAudio() {
	return !_vm->isVersionFamilyTlcV2() || _journeyState.getHelpAudioEnabled();
}

void ZoombiniGameState::setEnableSound(bool val, bool showNotification) {
	_journeyState.setSfxEnabled(val);
	_vm->syncSoundSettings();

	ZoombiniPage *page = _vm->getActivePage();
	if (showNotification && page && page->getPageCategory() == ZoombiniPageCategory::kInteractive) {
		ZoombiniInteractive *interactive = dynamic_cast<ZoombiniInteractive *>(page);
		if (interactive)
			interactive->showNotiBoxLong(val ? ZoombiniText::kNotiBoxSoundOn : ZoombiniText::kNotiBoxSoundOff);
	}
}

void ZoombiniGameState::setEnableMusic(bool val, bool showNotification) {
	_journeyState.setBgmEnabled(val);
	_vm->syncSoundSettings();

	ZoombiniPage *page = _vm->getActivePage();
	if (showNotification && page && page->getPageCategory() == ZoombiniPageCategory::kInteractive) {
		ZoombiniInteractive *interactive = dynamic_cast<ZoombiniInteractive *>(page);
		if (interactive)
			interactive->showNotiBoxLong(val ? ZoombiniText::kNotiBoxMusicOn : ZoombiniText::kNotiBoxMusicOff);
	}
}

void ZoombiniGameState::setEnableStickyMouse(bool val, bool showNotification) {
	_journeyState.setStickyMouseEnabled(val);

	ZoombiniPage *page = _vm->getActivePage();
	if (showNotification && page && page->getPageCategory() == ZoombiniPageCategory::kInteractive) {
		ZoombiniInteractive *interactive = dynamic_cast<ZoombiniInteractive *>(page);
		if (interactive)
			interactive->showNotiBoxLong(val ? ZoombiniText::kNotiBoxStickeyMouse : ZoombiniText::kNotiBoxNonStickeyMouse);
	}
}

void ZoombiniGameState::setEnableAutoStickyMouse(bool val) {
	_journeyState.setAutoStickyMouseEnabled(val);

	ZoombiniPage *page = _vm->getActivePage();
	if (page && page->getPageCategory() == ZoombiniPageCategory::kInteractive) {
		ZoombiniInteractive *interactive = dynamic_cast<ZoombiniInteractive *>(page);
		if (interactive)
			interactive->showNotiBoxLong(val ? ZoombiniText::kNotiBoxAutoStickeyOn : ZoombiniText::kNotiBoxAutoStickeyOff);
	}
}

void ZoombiniGameState::setEnableTransitions(bool val, bool showNotification) {
	if (_vm->isVersionFamilyTlcV2()) {
		_journeyState.setV2TransitionsDisabled(!val);
	} else {
		_journeyState.setV1TransitionsDisabled(!val);
	}

	ZoombiniPage *page = _vm->getActivePage();
	if (showNotification && page && page->getPageCategory() == ZoombiniPageCategory::kInteractive) {
		ZoombiniInteractive *interactive = dynamic_cast<ZoombiniInteractive *>(page);
		if (interactive)
			interactive->showNotiBoxLong(val ? ZoombiniText::kNotiBoxTransitionsOn : ZoombiniText::kNotiBoxTransitionsOff);
	}
}

void ZoombiniGameState::setEnableTouchSense(bool val, bool showNotification) {
	if (!_vm->isVersionFamilyTlcV2())
		return;

	_journeyState.setTouchSenseEnabled(val);

	ZoombiniPage *page = _vm->getActivePage();
	if (showNotification && page && page->getPageCategory() == ZoombiniPageCategory::kInteractive) {
		ZoombiniInteractive *interactive = dynamic_cast<ZoombiniInteractive *>(page);
		if (interactive)
			interactive->showNotiBoxLong(val ? ZoombiniText::kNotiBoxTouchSenseOn : ZoombiniText::kNotiBoxTouchSenseOff);
	}

	GUI::MessageDialog dialog(Common::U32String("[WARN] Hardware feedback is not implemented in ScummVM.", Common::kUtf8));
	dialog.runModal();
}

void ZoombiniGameState::setEnableHelpAudio(bool val, bool showNotification) {
	if (!_vm->isVersionFamilyTlcV2())
		return;

	_journeyState.setHelpAudioEnabled(val);

	ZoombiniPage *page = _vm->getActivePage();
	if (showNotification && page && page->getPageCategory() == ZoombiniPageCategory::kInteractive) {
		ZoombiniInteractive *interactive = dynamic_cast<ZoombiniInteractive *>(page);
		if (interactive)
			interactive->showNotiBoxLong(val ? ZoombiniText::kNotiBoxHelpAudioOn : ZoombiniText::kNotiBoxHelpAudioOff);
	}
}

void ZoombiniGameState::setLessActionEnabled(bool val) {
	_journeyState.setLessActionEnabled(val);

	ZoombiniPage *page = _vm->getActivePage();
	if (page && page->getPageCategory() == ZoombiniPageCategory::kInteractive) {
		ZoombiniInteractive *interactive = dynamic_cast<ZoombiniInteractive *>(page);
		if (interactive)
			interactive->showNotiBoxLong(val ? ZoombiniText::kNotiBoxLessAction : ZoombiniText::kNotiBoxMoreAction);
	}
}

void ZoombiniGameState::setCursorVisible(bool val) {
	_flagCursorVisible = val;

	ZoombiniPage *page = _vm->getActivePage();
	if (page && page->getPageCategory() == ZoombiniPageCategory::kInteractive) {
		ZoombiniInteractive *interactive = dynamic_cast<ZoombiniInteractive *>(page);
		if (interactive)
			interactive->showNotiBoxLong(val ? ZoombiniText::kNotiBoxShowCursor : ZoombiniText::kNotiBoxHideCursor);
	}
}

int16 ZoombiniGameState::generateRandomPack(int16 count) {
	return generateRandomPack(count, false);
}

int16 ZoombiniGameState::generatePickerRandomPack() {
	ZmbStateFile &state = getCurrentState();
	// A partial Isle pack represents the Picker-side Snoids that the debug
	// replacement is allowed to consume. Limit the replacement to that count.
	const int16 isleCount = countOccupiedSnoidsInPack(state._zmbPackIsle);
	int16 maxCount = 16;
	if (0 < isleCount && isleCount < maxCount)
		maxCount = isleCount;

	// Picker stops creating Snoids at the 625-entry roster limit.
	const int16 remainingGenerated = 625 - state._zmbGeneratedCount;
	if (remainingGenerated < maxCount)
		maxCount = remainingGenerated;
	maxCount = MAX<int16>(maxCount, 0);

	return generateRandomPack(maxCount, true);
}

int16 ZoombiniGameState::generateRandomPack(int16 maxCount, bool usePickerGenerationRules) {
	ZmbStateFile &state = getCurrentState();
	ZmbStateActivePack &pack = state._zmbPackActive;
	const int16 previousOccupiedCount = countOccupiedSnoidsInPack(pack);
	pack.clearEntries();
	pack.setSkipOccupiedEntries(false);

	int16 generatedCount = 0;
	for (int16 snoidIdx = 0; snoidIdx < maxCount; snoidIdx++) {
		ZmbStateActiveEntry entry;
		ZmbTrait traits;
		bool canGenerate = false;
		for (int16 attempt = 0; usePickerGenerationRules && attempt < 64 && !canGenerate; attempt++) {
			traits._hair = static_cast<byte>(_vm->_rnd->getRandomNumber(1, 5));
			traits._eyes = static_cast<byte>(_vm->_rnd->getRandomNumber(1, 5));
			traits._nose = static_cast<byte>(_vm->_rnd->getRandomNumber(1, 5));
			traits._feet = static_cast<byte>(_vm->_rnd->getRandomNumber(1, 5));
			canGenerate = traits.isComplete() && state._twinGenStatus[traits.snoidId()] < 2;
		}

		if (!usePickerGenerationRules) {
			traits._hair = static_cast<byte>(_vm->_rnd->getRandomNumber(1, 5));
			traits._eyes = static_cast<byte>(_vm->_rnd->getRandomNumber(1, 5));
			traits._nose = static_cast<byte>(_vm->_rnd->getRandomNumber(1, 5));
			traits._feet = static_cast<byte>(_vm->_rnd->getRandomNumber(1, 5));
			canGenerate = true;
		}

		if (usePickerGenerationRules && !canGenerate) {
			// Match Picker's exhaustive fallback when random attempts do not find
			// a trait combination that can still be generated.
			for (int16 hair = 0; hair < 5 && !canGenerate; hair++) {
				for (int16 eye = 0; eye < 5 && !canGenerate; eye++) {
					for (int16 nose = 0; nose < 5 && !canGenerate; nose++) {
						for (int16 feet = 0; feet < 5; feet++) {
							traits._hair = static_cast<byte>(hair + 1);
							traits._eyes = static_cast<byte>(eye + 1);
							traits._nose = static_cast<byte>(nose + 1);
							traits._feet = static_cast<byte>(feet + 1);
							if (state._twinGenStatus[traits.snoidId()] < 2) {
								canGenerate = true;
								break;
							}
						}
					}
				}
			}
		}

		if (!canGenerate)
			break;

		entry.setTraits(traits);
		entry.setIsOccupied(true);
		entry.setU32Name(_vm, Common::U32String());
		if (!pack.appendEntry(entry))
			break;

		if (usePickerGenerationRules) {
			state._twinGenStatus[traits.snoidId()] += 1;
			state._zmbGeneratedCount += 1;
		}
		generatedCount += 1;
	}

	if (usePickerGenerationRules)
		return generatedCount;

	return 16 - previousOccupiedCount;
}

int16 ZoombiniGameState::subtractDebugGeneratedSnoidsFromIsle(int16 generatedCount) {
	if (generatedCount < 1)
		return 0;

	ZmbStateFile &state = getCurrentState();
	ZmbStateActivePack &islePack = state._zmbPackIsle;
	const int16 removeLimit = MIN<int16>(generatedCount, countOccupiedSnoidsInPack(islePack));
	int16 removedCount = 0;
	const int16 entryLimit = CLIP<int16>(islePack.getPackZmbCount(), 0, islePack.getEntryCapacity());

	for (int16 entryIdx = entryLimit; 0 < entryIdx && removedCount < removeLimit;) {
		entryIdx -= 1;
		ZmbStateActiveEntry &entry = islePack.getEntry(entryIdx);
		if (!entry.getIsOccupied() || !entry.getTraits().isComplete())
			continue;

		const int16 snoidTraitId = entry.getTraits().snoidId();
		if (0 < state._twinGenStatus[snoidTraitId])
			state._twinGenStatus[snoidTraitId] -= 1;
		if (0 < state._zmbGeneratedCount)
			state._zmbGeneratedCount -= 1;
		entry = ZmbStateActiveEntry();
		removedCount += 1;
	}

	compactActivePack(islePack);
	return removedCount;
}

// True if the (decompressed) length matches one of the four save layouts.
bool ZoombiniSaveTransfer::zmbIsValidStateSize(uint32 size) {
	return size == static_cast<uint32>(ZoombiniGameState::kStateFileSizeBrV10) ||
		   size == static_cast<uint32>(ZoombiniGameState::kStateFileSizeUsV11) ||
		   size == static_cast<uint32>(ZoombiniGameState::kStateFileSizeTlcV20Small) ||
		   size == static_cast<uint32>(ZoombiniGameState::kStateFileSizeTlcV20);
}

bool ZoombiniSaveTransfer::zmbReadAll(Common::SeekableReadStream *stream, Common::Array<byte> &out, uint32 maximumSize) {
	if (!stream)
		return false;
	const int64 size = stream->size();
	if (size < 0 || maximumSize < static_cast<uint64>(size))
		return false;
	out.resize(static_cast<uint32>(size));
	return size == 0 || stream->read(out.begin(), static_cast<uint32>(size)) == static_cast<uint32>(size);
}

// Win 3.1/9x save folders are usually upper-case on FAT.
// Match a child case-insensitively.
// Return a node with the canonical name when it is absent.
Common::FSNode ZoombiniSaveTransfer::zmbFindChild(const Common::FSNode &dir, const Common::String &name) {
	Common::FSNode direct = dir.getChild(name);
	if (direct.exists())
		return direct;

	Common::FSList children;
	if (dir.getChildren(children, Common::FSNode::kListFilesOnly)) {
		for (Common::FSList::const_iterator it = children.begin(); it != children.end(); it++) {
			if (it->getName().equalsIgnoreCase(name))
				return *it;
		}
	}
	return dir.getChild(name);
}

bool ZoombiniSaveTransfer::zmbSaveFileExists(Common::SaveFileManager *saveFileMan, const Common::String &name) {
	Common::InSaveFile *file = saveFileMan->openForLoading(name);
	const bool exists = (file != nullptr);
	delete file;
	return exists;
}

// Write into the ScummVM save directory using its default compression.
bool ZoombiniSaveTransfer::zmbWriteToSaveFile(Common::SaveFileManager *saveFileMan, const Common::String &name, const Common::Array<byte> &bytes) {
	Common::OutSaveFile *out = saveFileMan->openForSaving(name);
	if (!out)
		return false;
	out->write(bytes.begin(), bytes.size());
	const bool ok = !out->err();
	out->finalize();
	delete out;
	return ok;
}

// Write raw, uncompressed bytes into the selected external folder.
bool ZoombiniSaveTransfer::zmbWriteToFolder(const Common::FSNode &node, const Common::Array<byte> &bytes) {
	Common::SeekableWriteStream *out = node.createWriteStream(false);
	if (!out)
		return false;
	out->write(bytes.begin(), bytes.size());
	const bool ok = !out->err();
	out->finalize();
	delete out;
	return ok;
}

void ZoombiniSaveTransfer::exportOneSave(const Common::String &target, int slot) {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	ZmbRosterFile roster;
	if (!ZoombiniGameState::loadRosterFile(saveFileMan, target + ".WHO", roster) ||
		!roster.isSerializedStateValid() || !roster.hasEntry(slot)) {
		GUI::MessageDialog msg(_("The selected saved game could not be found."));
		msg.runModal();
		return;
	}

	const ZmbRosterEntry &entry = roster.getEntry(slot);
	const Common::String sourceName = ZoombiniGameState::makeSaveFilename(target, entry._fileName);
	if (sourceName.empty()) {
		GUI::MessageDialog msg(_("The selected saved game is missing or invalid."));
		msg.runModal();
		return;
	}
	Common::Array<byte> stateBytes;
	Common::InSaveFile *source = saveFileMan->openForLoading(sourceName);
	const bool read = zmbReadAll(source, stateBytes, static_cast<uint32>(ZoombiniGameState::kStateFileSizeTlcV20));
	delete source;
	if (!read || !zmbIsValidStateSize(stateBytes.size())) {
		GUI::MessageDialog msg(_("The selected saved game is missing or invalid."));
		msg.runModal();
		return;
	}

	// I18N: Title of a directory chooser for exporting one Zoombini save.
	GUI::BrowserDialog browser(_("Select the directory for the exported .TXT save"), true);
	if (browser.runModal() <= 0)
		return;

	const Common::FSNode dir = browser.getResult();
	if (!dir.isDirectory()) {
		GUI::MessageDialog msg(_("The selected destination is not a directory."));
		msg.runModal();
		return;
	}

	const Common::String baseName = entry.getSaveFileStem();
	if (baseName.empty()) {
		GUI::MessageDialog msg(_("The selected saved game is missing or invalid."));
		msg.runModal();
		return;
	}
	const Common::FSNode destination = zmbFindChild(dir, baseName + ".TXT");
	if (destination.exists()) {
		GUI::MessageDialog confirm(_("The .TXT file already exists. Overwrite it?"), _("Yes"), _("No"));
		if (confirm.runModal() != GUI::kMessageOK)
			return;
	}

	if (!zmbWriteToFolder(destination, stateBytes)) {
		GUI::MessageDialog msg(_("Unable to export the selected saved game."));
		msg.runModal();
		return;
	}

	GUI::MessageDialog done(_("The saved game was exported as an original .TXT format."));
	done.runModal();
}

void ZoombiniSaveTransfer::importFromOriginalFolder(const Common::String &target) {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();

	// I18N: Title of a directory chooser for importing saves from an installed game.
	GUI::BrowserDialog browser(_("Select the original game save directory with ZOOMBINI.WHO"), true);
	if (browser.runModal() <= 0)
		return;
	const Common::FSNode dir = browser.getResult();

	Common::FSNode rosterNode = zmbFindChild(dir, "ZOOMBINI.WHO");
	if (!rosterNode.exists()) {
		GUI::MessageDialog msg(_("The selected directory has no ZOOMBINI.WHO roster file."));
		msg.runModal();
		return;
	}

	Common::Array<byte> rosterBytes;
	ZmbRosterFile roster;
	{
		Common::SeekableReadStream *in = rosterNode.createReadStream();
		const bool read = zmbReadAll(in, rosterBytes, ZoombiniGameState::kZmbRosterFileSize);
		delete in;
		if (!read || !ZoombiniGameState::parseRoster(rosterBytes, roster) || !roster.isSerializedStateValid()) {
			GUI::MessageDialog msg(_("ZOOMBINI.WHO is not a valid Zoombinis roster file."));
			msg.runModal();
			return;
		}
	}

	// Collect the roster-referenced state files that are present in the folder.
	Common::Array<Common::FSNode> stateSrc;
	Common::Array<Common::String> stateDst;
	const int entryCount = roster.getEntryCount();
	for (int i = 0; i < entryCount; i++) {
		const Common::String base = roster.getEntry(i).getSaveFileStem();
		if (base.empty())
			continue;
		const Common::FSNode src = zmbFindChild(dir, base + ".TXT");
		if (!src.exists())
			continue;
		stateSrc.push_back(src);
		stateDst.push_back(ZoombiniGameState::makeSaveFilename(target, roster.getEntry(i)._fileName));
	}

	// Confirm before overwriting existing ScummVM saves for this game.
	bool overwrite = zmbSaveFileExists(saveFileMan, target + ".WHO");
	for (uint i = 0; i < stateDst.size() && !overwrite; i++)
		overwrite = zmbSaveFileExists(saveFileMan, stateDst[i]);
	if (overwrite) {
		GUI::MessageDialog confirm(_("This will overwrite existing ScummVM saves for this game. Continue?"), _("Yes"), _("No"));
		if (confirm.runModal() != GUI::kMessageOK)
			return;
	}

	int imported = 0;
	int failed = 0;
	if (!zmbWriteToSaveFile(saveFileMan, target + ".WHO", rosterBytes))
		failed += 1;
	for (uint i = 0; i < stateSrc.size(); i++) {
		Common::Array<byte> bytes;
		Common::SeekableReadStream *in = stateSrc[i].createReadStream();
		const bool read = zmbReadAll(in, bytes, static_cast<uint32>(ZoombiniGameState::kStateFileSizeTlcV20));
		delete in;
		if (!read || !zmbIsValidStateSize(bytes.size())) {
			failed += 1;
			continue;
		}
		if (zmbWriteToSaveFile(saveFileMan, stateDst[i], bytes))
			imported += 1;
		else
			failed += 1;
	}

	Common::U32String result = Common::U32String::format(_("Imported %d saved game(s) from the original game directory."), imported);
	if (failed)
		result += Common::U32String("\n") + _("Some files could not be copied.");
	GUI::MessageDialog done(result);
	done.runModal();
}

void ZoombiniSaveTransfer::exportToOriginalFolder(const Common::String &target) {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();

	// Read this target's roster from the ScummVM save directory.
	Common::Array<byte> rosterBytes;
	ZmbRosterFile roster;
	{
		Common::InSaveFile *in = saveFileMan->openForLoading(target + ".WHO");
		const bool read = zmbReadAll(in, rosterBytes, ZoombiniGameState::kZmbRosterFileSize);
		delete in;
		if (!read || !ZoombiniGameState::parseRoster(rosterBytes, roster) || !roster.isSerializedStateValid()) {
			GUI::MessageDialog msg(_("This game has no ScummVM saved games to export yet."));
			msg.runModal();
			return;
		}
	}

	// I18N: Title of a directory chooser for exporting saves to an installed game.
	GUI::BrowserDialog browser(_("Select the original game directory to export into"), true);
	if (browser.runModal() <= 0)
		return;
	const Common::FSNode dir = browser.getResult();
	if (!dir.isDirectory()) {
		GUI::MessageDialog msg(_("The selected destination is not a directory."));
		msg.runModal();
		return;
	}

	// Collect the roster-referenced saves that exist in the save directory.
	Common::Array<Common::String> stateSrc;
	Common::Array<Common::FSNode> stateDst;
	const int entryCount = roster.getEntryCount();
	for (int i = 0; i < entryCount; i++) {
		const Common::String base = roster.getEntry(i).getSaveFileStem();
		if (base.empty())
			continue;
		const Common::String srcName = ZoombiniGameState::makeSaveFilename(target, roster.getEntry(i)._fileName);
		if (!zmbSaveFileExists(saveFileMan, srcName))
			continue;
		stateSrc.push_back(srcName);
		stateDst.push_back(zmbFindChild(dir, base + ".TXT"));
	}

	// Confirm before overwriting existing files in the destination folder.
	bool overwrite = zmbFindChild(dir, "ZOOMBINI.WHO").exists();
	for (uint i = 0; i < stateDst.size() && !overwrite; i++)
		overwrite = stateDst[i].exists();
	if (overwrite) {
		GUI::MessageDialog confirm(_("This will overwrite existing save files in the selected directory. Continue?"), _("Yes"), _("No"));
		if (confirm.runModal() != GUI::kMessageOK)
			return;
	}

	int exported = 0;
	int failed = 0;
	if (!zmbWriteToFolder(zmbFindChild(dir, "ZOOMBINI.WHO"), rosterBytes))
		failed += 1;
	for (uint i = 0; i < stateSrc.size(); i++) {
		Common::Array<byte> bytes;
		Common::InSaveFile *in = saveFileMan->openForLoading(stateSrc[i]);
		const bool read = zmbReadAll(in, bytes, static_cast<uint32>(ZoombiniGameState::kStateFileSizeTlcV20));
		delete in;
		if (!read || !zmbIsValidStateSize(bytes.size())) {
			failed += 1;
			continue;
		}
		if (zmbWriteToFolder(stateDst[i], bytes))
			exported += 1;
		else
			failed += 1;
	}

	Common::U32String result = Common::U32String::format(_("Exported %d saved game(s) to the original game directory."), exported);
	if (failed)
		result += Common::U32String("\n") + _("Some files could not be copied.");
	GUI::MessageDialog done(result);
	done.runModal();
}

} // End of namespace Mohawk
