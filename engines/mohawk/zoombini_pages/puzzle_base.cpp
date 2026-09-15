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

#include "mohawk/zoombini_pages/puzzle_base.h"

#include "mohawk/zoombini.h"
#include "mohawk/zoombini_random.h"
#include "mohawk/zoombini_state.h"

namespace Mohawk {

ZoombiniPuzzle::ZoombiniPuzzle(MohawkEngine_Zoombini *vm, ZoombiniPageType pageType, ZmbSrcPageKind departXferSrcSiPage)
	: ZoombiniInteractive(vm, pageType),
	  _difficultyLevel(static_cast<ZmbPuzzleLevel>(vm->_state->readPageRouteLevel(pageType) + 1)),
	  _entrySfxGroupFlags(ZmbSfxGroupFlags::kRandom_00) {
	_departXferSrcSiPage = departXferSrcSiPage;

	// Every puzzle starts from one active pack containing 1 to 16 Snoids.
	const int16 packZmbCount = vm->_state->getCurrentState()._zmbPackActive.getPackZmbCount();
	assert(0 < packZmbCount && packZmbCount <= 16);
}

ZoombiniPuzzle::~ZoombiniPuzzle() {
}

Common::String ZoombiniPuzzle::getDebugBanner() const {
	return Common::String::format("%s L%d (Route%d-%d)", getPageName(), static_cast<int>(_difficultyLevel), getRouteNumber(), getRoutePuzzleIdx());
}

void ZoombiniPuzzle::initSfxGroupFlags() {
	if (_entrySfxGroupFlagsInitialized)
		return;

	_entrySfxGroupFlags = _vm->_state->getSfxGroupFlagsFromPageType(getPageType());
	_entrySfxGroupFlagsInitialized = true;
}

void ZoombiniPuzzle::configureStandardPuzzleControlRects() {
	configureStandardPuzzleControlRects(Common::Rect(600, 365, 639, 402));
}

void ZoombiniPuzzle::configureStandardPuzzleControlRects(const Common::Rect &helpRect) {
	setGoButton(Common::Rect(600, 441, 639, 478), 1, 2, 3);
	setMapButton(Common::Rect(600, 403, 639, 440), 5, 6);
	setHelpButton(helpRect);
}

void ZoombiniPuzzle::loadStandardPuzzleControlFeatures(int16 bitmapResId) {
	loadGoMapButtonsFeature(bitmapResId);
	loadHelpButtonFeature();
}

ZmbFeature *ZoombiniPuzzle::createPuzzleMainFeatureHead() {
	return createMainFeatureHead(ZmbFeature::FLAG_00004000_NO_DIRTY_MERGE | ZmbFeature::FLAG_00008000_LOOP_ANIM |
								 ZmbFeature::FLAG_00020000_SKIP_RENDER | ZmbFeature::FLAG_04000000_OVERLAY);
}

int16 ZoombiniPuzzle::selectCompletionNarratorSoundId() {
	return _vm->_rnd->getRandomNumber(kSysResSound20055_CompletionBase, kSysResSound20063_CompletionLast);
}

int16 ZoombiniPuzzle::selectPartialSuccessNarratorSoundId() {
	return _vm->_rnd->getRandomNumber(kSysResSound20045_PartialSuccessBase, kSysResSound20048_PartialSuccessLast);
}

void ZoombiniPuzzle::queueNarratorSound(int16 soundId) {
	queueScriptSoundForNextRenderPass(ZmbResource(ZmbResource::kSystem, soundId), Audio::Mixer::kSpeechSoundType);
}

bool ZoombiniPuzzle::passesPartialResultFeedbackGate() {
	const ZmbStateFile::PageFlag &pageFlag = _vm->_state->getCurrentState().getPageFlagFromPageType(getPageType());
	return _difficultyLevel - 1 < _vm->_rnd->getRandomNumber(0, 4) ||
		   _vm->_state->getPageVisitCountFromPageFlag(pageFlag) <= kPartialResultFeedbackEarlyVisitCount;
}

void ZoombiniPuzzle::queueCompletionNarratorSound() {
	queueNarratorSound(selectCompletionNarratorSoundId());
}

void ZoombiniPuzzle::queuePartialSuccessNarratorSound() {
	queueNarratorSound(selectPartialSuccessNarratorSoundId());
}

ZmbChanceInfo ZoombiniPuzzle::debugGetChances() const {
	ZmbChanceInfo info;
	info.type = ZmbChanceInfo::ZmbChanceType::kAmorphous;
	return info;
}

bool ZoombiniPuzzle::confirmMapTransition() {
	// Outside practice mode, confirm the map transition.
	// Clear every pack Snoid's occupied bit before shared cleanup.
	const bool isPractice = _vm->_state->inPracticeMode();
	if (!isPractice) {
		ZoombiniDialogResult result = _vm->openMsgBoxDialog(ZoombiniMsgBoxType::kAskGoMapWillLost);
		if (result != ZoombiniDialogResult::kYes)
			return false;

		for (ZmbFeatureList<ZmbSnoid>::const_iterator it = _snoidMap.begin(); it != _snoidMap.end(); it++) {
			ZmbSnoid *snoid = *it;
			if (snoid && snoid->isPackSnoid())
				snoid->_packIsOccupied = false;
		}
	}
	return true;
}

void ZoombiniPuzzle::saveStateBeforeMapTransition() {
	// Practice mode discards its temporary pack and skips serialization.
	// Besides matching the saved state, this avoids reactivating hidden pack
	// Snoids solely for serialization during a practice-page fade.
	if (_vm->_state->inPracticeMode()) {
		ZmbStateActivePack &activePack = _vm->_state->getCurrentState()._zmbPackActive;
		activePack.clearEntries();
		activePack.setSkipOccupiedEntries(true);
		activePack.setSkipUnoccupiedEntries(true);
		return;
	}

	// Persist the active runners, then route failed Snoids to their resting pack.
	saveSnoidsToPack();
	routeNonOccupiedToRestingPack();
}

int16 ZoombiniPuzzle::loadOccupiedSnoidsFromActivePack(const Common::Point *positions,
													   uint16 positionCount,
													   Common::Array<ZmbSnoid *> *loadedSnoids) {
	ZmbStateActivePack &activePack = _vm->_state->getCurrentState()._zmbPackActive;
	assert(activePack.getPackZmbCount() <= 16);
	return loadSnoidsFromPack(activePack, positions, positionCount, false, 10000, loadedSnoids);
}

int16 ZoombiniPuzzle::loadZoombinisFromPack(const Common::Point *positions, uint16 positionCount) {
	_pageLoadedZmbCount = loadOccupiedSnoidsFromActivePack(positions, positionCount);
	return _pageLoadedZmbCount;
}

void ZoombiniPuzzle::collectPackSnoids(Common::Array<ZmbSnoid *> &snoids, bool occupiedOnly) const {
	snoids.clear();
	for (ZmbFeatureList<ZmbSnoid>::const_iterator it = _snoidMap.begin(); it != _snoidMap.end(); it++) {
		ZmbSnoid *snoid = *it;
		if (!snoid || !snoid->isPackSnoid())
			continue;
		if (occupiedOnly && !snoid->_packIsOccupied)
			continue;
		snoids.push_back(snoid);
	}
}

bool ZoombiniPuzzle::handleBodyArrangementScriptEvent(ZmbSnoid &snoid, int16 eventCode) {
	if (kAnimEvent240_BodyArrangePendFirst <= eventCode &&
		eventCode <= kAnimEvent243_BodyArrangePendLast) {
		_pendingBodyArrangement = eventCode - (kAnimEvent240_BodyArrangePendFirst - 1);
		return true;
	}
	if (kAnimEvent250_BodyArrangeDirectFirst <= eventCode &&
		eventCode <= kAnimEvent253_BodyArrangeDirectLast) {
		snoid.setTraitLayout(static_cast<ZmbScriptDecoder::TraitLayout>(eventCode - kAnimEvent250_BodyArrangeDirectFirst));
		return true;
	}
	return false;
}

bool ZoombiniPuzzle::applyPendingBodyArrangement(ZmbSnoid &snoid) {
	if (_pendingBodyArrangement == 0)
		return false;

	snoid.setTraitLayout(static_cast<ZmbScriptDecoder::TraitLayout>(_pendingBodyArrangement - 1));
	_pendingBodyArrangement = 0;
	return true;
}

} // End of namespace Mohawk
