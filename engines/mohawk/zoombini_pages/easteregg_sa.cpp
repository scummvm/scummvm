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
#include "mohawk/zoombini_pages/easteregg_sa.h"

#include "mohawk/cursors.h"
#include "mohawk/zoombini.h"
#include "mohawk/zoombini_random.h"
#include "mohawk/zoombini_sound.h"

namespace Mohawk {

constexpr Common::Point ZoombiniEasterEggSA::kCrosshairHotspot;

ZoombiniEasterEggSA::ZoombiniEasterEggSA(MohawkEngine_Zoombini *vm)
	: ZoombiniInteractive(vm, ZoombiniPageType::kEasterEggSA) {
}

ZoombiniEasterEggSA::~ZoombiniEasterEggSA() {
	_vm->_cursor->showCursor();
}

void ZoombiniEasterEggSA::open() {
	openArchive(ZMB_MHK_PICKER);
}

void ZoombiniEasterEggSA::setBackgroundMusic() {
	if (!_vm->getRelInfo().isFamily(ZmbReleaseInfo::Family::kTlcV20))
		_vm->_midi->playMidi(ZmbResource(ZmbResource::kSystem, kSysResMidi30025_CavesBgmBase));
}

void ZoombiniEasterEggSA::setBackgroundBitmap() {
	_vm->_gfx->setPalette(kResBackground1001);
	_vm->_gfx->drawBackground(kResBackground1001);
}

const ZoombiniPage::ScriptSoundPriorityRanges &ZoombiniEasterEggSA::getScriptSoundPriorityRanges() const {
	static const ScriptSoundPriorityRanges kRanges = {
		{kResSound1200_Hit, kResSound1201_Miss},
		{kSysResSoundRange20000_Base, kSysResSoundRange29999_Last},
	};
	return kRanges;
}

void ZoombiniEasterEggSA::loadFeatures() {
	const ZmbResource scenery(ZmbResource::kPage, kResBitmapShape1100_Scenery);
	const ZmbResource feedback(ZmbResource::kPage, kResBitmapShape1200_Feedback);
	loadShapeOffsetRegs(ZmbResource::kPage, kResBitmapShape1200_Feedback);

	for (int16 scrbId = kResScrb1100_SceneryBase; scrbId <= kResScrb1104_SceneryLast; scrbId++)
		loadScrbFeature(scenery, scrbId, 0, ZmbFeature::FLAG_00000000_TYPE_SHAPES);

	const Common::Point crosshairPos = _vm->getEventManager()->getMousePos() - kCrosshairHotspot;
	_crosshairFeature = loadScrbFeature(scenery, kResScrb1105_Crosshair, 1, crosshairPos,
										ZmbFeature::FLAG_00001000_TOPMOST | ZmbFeature::FLAG_00800000_POS_DELTA);

	static constexpr uint32 kFeedbackFlags = ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE |
											 ZmbFeature::FLAG_00800000_POS_DELTA | ZmbFeature::FLAG_01000000_DEFER_RENDER;
	_hitFeedbackFeature = loadScrbFeature(feedback, kResScrb1200_Hit, 1, Common::Point(0, 480), kFeedbackFlags);
	_missFeedbackFeature = loadScrbFeature(feedback, kResScrb1201_Miss, 1, Common::Point(0, 480), kFeedbackFlags | ZmbFeature::FLAG_00001000_TOPMOST);
	ZmbFeature *scorePanelFeature = loadScrbFeature(feedback, kResScrb1202_ScorePanel, 0, ZmbFeature::FLAG_00000000_TYPE_SHAPES);
	scorePanelFeature->setShapeOffsetRegs(_shapeOffsetRegsMap[kResBitmapShape1200_Feedback]);

	ZmbFeature::EventHooks scoreHooks;
	scoreHooks.setPreRenderShapeFunc(&ZoombiniEasterEggSA::updateScoreDigits);
	_scoreDigitsFeature = loadScrbFeature(feedback, kResScrb1203_ScoreDigits, 1, ZmbFeature::FLAG_00100000_PLAY_ONCE, scoreHooks);
	refreshScore();

	_vm->_cursor->hideCursor();
}

void ZoombiniEasterEggSA::close() {
	_vm->_cursor->showCursor();
	ZoombiniInteractive::close();
}

void ZoombiniEasterEggSA::onMapButtonActivated() {
	_vm->setNextPage(ZoombiniPageType::kRodMap);
	close();
}

void ZoombiniEasterEggSA::onAnimFrame() {
	if (_shots) {
		const Common::Point crosshairPos = _vm->getEventManager()->getMousePos() - kCrosshairHotspot;
		if (_crosshairFeature->getPointLoc() != crosshairPos) {
			_crosshairFeature->setPointLoc(crosshairPos);
			_crosshairFeature->requestVisualRematerialization();
		}
	}
	ZoombiniInteractive::onAnimFrame();
}

void ZoombiniEasterEggSA::onPostRenderFrame() {
	for (ZmbSnoid *&walkSnoid : _walkingSnoids) {
		if (walkSnoid && walkSnoid->getAnimState() == kSnoidAnimState000_Idle) {
			markFeatureVisualCoverageDirty(walkSnoid, false);
			unloadSnoid(walkSnoid->getId());
			walkSnoid = nullptr;
		}
	}

	if (!_shots || _shotFeedbackFeature == _hitFeedbackFeature || getCurrentFrameCounter() <= _nextSpawnFrame)
		return;

	for (uint16 slot = 0; slot < getWalkerLimit(); slot++) {
		if (!_walkingSnoids[slot]) {
			_walkingSnoids[slot] = spawnWalker();
			_nextSpawnFrame = getCurrentFrameCounter() + _vm->_rnd->getRandomNumber(20, 120);
		}
	}
}

ZmbEventHandleResult ZoombiniEasterEggSA::onLButtonDown(const Common::Point &absPos, const Common::Point &relPos) {
	if (Common::Rect(7, 7, 41, 41).contains(absPos)) {
		onMapButtonActivated();
	} else if (!_shots) {
		if (_restartPromptFeature && _restartClickRect.contains(absPos))
			resetRound();
	} else if (!_shotFeedbackFeature) {
		playShot(findCaughtWalker(absPos), absPos);
	}
	return ZmbEventHandleResult::kConsumed;
}

ZmbSnoid *ZoombiniEasterEggSA::findCaughtWalker(const Common::Point &point) const {
	static const Common::Rect kSceneryRects[] = {
		Common::Rect(529, 125, 639, 197),
		Common::Rect(227, 155, 356, 205),
		Common::Rect(415, 425, 639, 479),
	};
	for (const Common::Rect &scenery : kSceneryRects) {
		if (scenery.contains(point))
			return nullptr;
	}

	Common::Array<ZmbSnoid *> ordered;
	collectSnoidsInRunnerOrder(ordered);
	for (uint32 i = ordered.size(); 0 < i; i -= 1) {
		ZmbSnoid *walker = ordered[i - 1];
		const Common::Rect rect = walker->getClickRect();
		if (!rect.contains(point))
			continue;
		if (12 < ABS((rect.left + rect.right) / 2 - point.x) || 12 < ABS((rect.top + rect.bottom) / 2 - point.y))
			return nullptr;
		return walker;
	}
	return nullptr;
}

void ZoombiniEasterEggSA::playShot(ZmbSnoid *walker, const Common::Point &point) {
	_shots -= 1;
	int16 script = kResScrb1201_Miss;
	_shotFeedbackFeature = _missFeedbackFeature;

	if (walker) {
		_streak += 1;
		_shots += MIN<int16>(_shotBonusCapacity, MIN<int16>(20, 5 * (_streak / 2)));
		_score += 1;

		switch (_score) {
		case 5:
			_walkInterval = 7;
			break;
		case 10:
			_walkInterval = 6;
			break;
		case 20:
			_walkInterval = 5;
			break;
		case 40:
			_walkInterval = 4;
			break;
		case 60:
			_walkInterval = 3;
			break;
		case 80:
			_walkInterval = 2;
			break;
		case 90:
			_walkInterval = 1;
			break;
		default:
			break;
		}

		_caughtSnoid = walker;
		_caughtSnoid->setDelayUntilFrame(getCurrentFrameCounter() + 240);
		for (ZmbSnoid *&slot : _walkingSnoids) {
			if (slot == walker)
				slot = nullptr;
		}
		_shotFeedbackFeature = _hitFeedbackFeature;
		script = kResScrb1200_Hit;
		manualLinkAfter(_hitFeedbackFeature, walker);
		setRunnerZSortEnabled(false);
	} else {
		_streak = 0;
	}

	loadScrbOntoFeature(_shotFeedbackFeature, script);
	_shotFeedbackFeature->setPointRef(Common::Point(320, 240));
	_shotFeedbackFeature->setPointLoc(point);
	_shotFeedbackFeature->activateRender();
	_shotFeedbackFeature->activateAnimate();
	_shotBonusCapacity = 99 - _score - _shots;
	refreshScore();

	if (!_shots) {
		_restartPromptFeature = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape1200_Feedback),
												kResScrb1204_Restart, 1, ZmbFeature::FLAG_00000000_TYPE_SHAPES);
		_restartPromptFeature->setShapeOffsetRegs(_shapeOffsetRegsMap[kResBitmapShape1200_Feedback]);
		_restartClickRect = _crosshairFeature->getClickRect();
		_vm->_cursor->showCursor();
	}
}

void ZoombiniEasterEggSA::onFeatureAnimEvent(ZmbFeature *feature, int16 eventCode) {
	if (feature == _hitFeedbackFeature && eventCode == kFeedbackEventCode000_RemoveCaught && _caughtSnoid) {
		// SCRB 1200 removes the caught Snoid before drawing the frame that plays the hit sound.
		markFeatureVisualCoverageDirty(_caughtSnoid, false);
		unloadSnoid(_caughtSnoid->getId());
		_caughtSnoid = nullptr;
	}
	if (feature == _shotFeedbackFeature && eventCode == kAnimEventM1_End) {
		if (feature == _hitFeedbackFeature)
			setRunnerZSortEnabled(true);
		_shotFeedbackFeature = nullptr;
	}
}

void ZoombiniEasterEggSA::resetRound() {
	_score = 0;
	_shots = 9;
	_streak = 0;
	_shotBonusCapacity = 90;
	_walkInterval = 8;
	unloadScrbFeature(_restartPromptFeature);
	_restartPromptFeature = nullptr;
	refreshScore();
	_vm->_cursor->hideCursor();
}

void ZoombiniEasterEggSA::refreshScore() {
	if (99 < _score)
		_score = 0;
	ZmbStateFile &state = _vm->_state->getCurrentState();
	if (state._fleensHighScore < _score) {
		state._fleensHighScore = _score;
		_vm->_state->markSaveBeforeQuitPending();
	}
	loadScrbOntoFeature(_scoreDigitsFeature, kResScrb1203_ScoreDigits);
}

void ZoombiniEasterEggSA::updateScoreDigits(ZmbFeature *feature, ZmbHotspotGroup *group, Common::Array<ZmbHotspot> &hotspots) {
	if (hotspots.size() < 6)
		return;
	const int16 values[] = {
		static_cast<int16>(MIN<uint16>(99, _vm->_state->getCurrentState()._fleensHighScore)),
		_score,
		_shots};
	const int16 zero = hotspots[0]._shapeIdx;
	for (uint16 i = 0; i < ARRAYSIZE(values); i++) {
		hotspots[2 * i]._shapeIdx = zero + values[i] / 10;
		hotspots[2 * i + 1]._shapeIdx = zero + values[i] % 10;
	}
}

uint16 ZoombiniEasterEggSA::getWalkerLimit() const {
	if ((15 <= _score && _score < 20) || _score == 80)
		return 3;
	if ((10 <= _score && _score < 40) || (60 <= _score && _score < 80))
		return 2;
	return 1;
}

ZmbSnoid *ZoombiniEasterEggSA::spawnWalker() {
	ZmbTrait trait;
	trait._hair = static_cast<ZmbTrait::HairKind>(_vm->_rnd->getRandomNumber(1, 5));
	trait._eyes = static_cast<ZmbTrait::EyeKind>(_vm->_rnd->getRandomNumber(1, 5));
	trait._nose = static_cast<ZmbTrait::NoseKind>(_vm->_rnd->getRandomNumber(1, 5));
	trait._feet = static_cast<ZmbTrait::FeetKind>(_vm->_rnd->getRandomNumber(1, 5));

	struct WalkRange {
		/** Left to right endpoints. */
		int16 startX, endX;
		/** Inclusive vertical range for the walk. */
		int16 minY, maxY;
	};

	static constexpr WalkRange kWalks[] = {
		{-20, 660, 230, 270},
		{-20, 660, 380, 450},
		{325, 660, 325, 335},
		{328, 557, 165, 185},
		{-20, 557, 165, 185},
		{-20, 250, 165, 185},
	};
	const uint16 half = _vm->getEventManager()->getMousePos().y <= 240 ? 0 : 3;
	const WalkRange &range = kWalks[half + _vm->_rnd->getRandomNumber(0, 2)];
	const int16 y = _vm->_rnd->getRandomNumber(range.minY, range.maxY);
	Common::Point start(range.startX, y);
	Common::Point end(range.endX, y);
	if (_vm->_rnd->getRandomNumber(1, 100) <= 50)
		SWAP(start, end);

	const uint16 id = allocateDynamicPackSnoidId();
	ZmbSnoid *walkSnoid = new ZmbSnoid(_vm, id, ZmbFeature::FLAG_00000001_TYPE_SNOID);
	_snoidMap.insert(id, walkSnoid);
	walkSnoid->setRegistrationIndex(_nextRegistrationIndex);
	_nextRegistrationIndex += 1;
	walkSnoid->setPointLoc(start);
	walkSnoid->setSortRect(Common::Rect(start.x, start.y, start.x + 1, start.y + 1));
	walkSnoid->setResource(ZmbResource(ZmbResource::kSystem, kSysResShapeBitmap3000_SnoidNormal));
	walkSnoid->initValues();
	walkSnoid->_trait = trait;
	walkSnoid->setIdleTickCounter(_vm->_rnd->getRandomNumber(0, 64));
	walkSnoid->setCommonImageIndex(1);
	walkSnoid->setFrameInterval(_walkInterval);
	walkSnoid->initDirectWalkToTarget(end, this);

	if (1 < _walkInterval && _vm->_rnd->getRandomNumber(1, 100) <= 25)
		walkSnoid->setFrameInterval(_walkInterval - 1);
	static constexpr int16 kWalkVoices[] = {6, 12, 10, 9, 0, 1, 11, 2, 8};
	const int16 voiceId = walkSnoid->getVoiceResId(kWalkVoices[_vm->_rnd->getRandomNumber(0, ARRAYSIZE(kWalkVoices) - 1)]);
	if (0 < voiceId)
		queueScriptSoundForNextRenderPass(resolveSnoidVoiceResource(voiceId), Audio::Mixer::kSpeechSoundType);
	return walkSnoid;
}

} // End of namespace Mohawk
