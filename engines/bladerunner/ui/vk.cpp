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

#include "bladerunner/ui/vk.h"

#include "bladerunner/actor.h"
#include "bladerunner/ambient_sounds.h"
#include "bladerunner/audio_player.h"
#include "bladerunner/bladerunner.h"
#include "bladerunner/combat.h"
#include "bladerunner/font.h"
#include "bladerunner/game_constants.h"
#include "bladerunner/game_flags.h"
#include "bladerunner/game_info.h"
#include "bladerunner/mouse.h"
#include "bladerunner/music.h"
#include "bladerunner/scene.h"
#include "bladerunner/script/vk_script.h"
#include "bladerunner/shape.h"
#include "bladerunner/slice_renderer.h"
#include "bladerunner/subtitles.h"
#include "bladerunner/text_resource.h"
#include "bladerunner/time.h"
#include "bladerunner/ui/ui_image_picker.h"
#include "bladerunner/vqa_player.h"

#include "common/str.h"
#include "common/keyboard.h"
#include "bladerunner/debugger.h"

namespace BladeRunner {

VK::VK(BladeRunnerEngine *vm) {
	_vm = vm;

	reset();
}

VK::~VK() {
	reset();
}

void VK::open(int actorId, int calibrationRatio) {
	if (!_vm->openArchive("MODE.MIX")) {
		return;
	}

	reset();

	_questions.resize(3);
	for (int i = 0; i < (int)_questions.size(); ++i) {
		_questions[i].resize(18);
		for (int j = 0; j < (int)_questions[i].size(); ++j) {
			_questions[i][j].isPresent = false;
			_questions[i][j].wasAsked = false;
		}
	}

	_volumeAmbient = _vm->_ambientSounds->getVolume();
	_volumeMusic   = _vm->_music->getVolume();

	_actorId          = actorId;
	_calibrationRatio = calibrationRatio;
	_calibration      = 0;

	_buttons = new UIImagePicker(_vm, 8);

	_shapes.resize(15);
	for (int i = 0; i < (int)_shapes.size(); ++i) {
		_shapes[i] = new Shape(_vm);
		_shapes[i]->open("VK.SHP", i);
	}

	_vqaPlayerMain = new VQAPlayer(_vm, &_vm->_surfaceBack, "VK.VQA");
	if (!_vqaPlayerMain->open()) {
		return;
	}

	Common::String eyeVqa;
	switch (actorId) {
	case kActorDektora:
		eyeVqa = "VKDEKT.VQA";
		break;
	case kActorLucy:
		eyeVqa = "VKLUCY.VQA";
		break;
	case kActorGrigorian:
		eyeVqa = "VKKASH.VQA";
		break;
	case kActorBulletBob:
		eyeVqa = "VKBOB.VQA";
		break;
	case kActorRunciter:
		eyeVqa = "VKRUNC.VQA";
		break;
	default:
		return;
	}

	_surfaceEye.create(172, 116, screenPixelFormat());
	_vqaPlayerEye = new VQAPlayer(_vm, &_surfaceEye, eyeVqa);
	if (!_vqaPlayerEye->open()) {
		return;
	}
	if (!_vqaPlayerEye->setLoop(0, -1, kLoopSetModeEnqueue, nullptr, nullptr)) {
		return;
	}

	_isOpen = true;

	_script = new VKScript(_vm);

	_vm->_time->pause();

	init();
}

bool VK::isOpen() const {
	return _isOpen;
}

void VK::close() {
	if (_vm->_audioPlayer->isActive(_soundTrackId1)) {
		_vm->_audioPlayer->stop(_soundTrackId1, false);
	}

	if (_vm->_audioPlayer->isActive(_soundTrackId3)) {
		_vm->_audioPlayer->stop(_soundTrackId3, false);
	}

	_vm->_audioPlayer->playAud(_vm->_gameInfo->getSfxTrack(kSfxBR018_1P), 33, 0, 0, 50, 0);

	_script->shutdown(_actorId, _humanProbability, _replicantProbability, _anxiety);

	delete _script;
	_script = nullptr;


	if (_buttons) {
		_buttons->deactivate();
		_buttons->resetImages();
		delete _buttons;
		_buttons = nullptr;
	}

	if (_vqaPlayerEye) {
		_vqaPlayerEye->close();
		delete _vqaPlayerEye;
		_vqaPlayerEye = nullptr;
	}

	if (_vqaPlayerMain) {
		_vqaPlayerMain->close();
		delete _vqaPlayerMain;
		_vqaPlayerMain = nullptr;
	}

	_questions.clear();

	for (int i = 0; i < (int)_shapes.size(); ++i) {
		delete _shapes[i];
	}
	_shapes.clear();

	_vm->closeArchive("MODE.MIX");
	_vm->_music->setVolume(_volumeMusic);
	_vm->_ambientSounds->setVolume(_volumeAmbient);

	_vm->_time->resume();
	_vm->_scene->resume();
}

void VK::tick() {
	int mouseX, mouseY;
	_vm->_mouse->getXY(&mouseX, &mouseY);
	if (!_vm->_mouse->isDisabled()) {
		_buttons->handleMouseAction(mouseX, mouseY, false, false, false);
	}

	draw();

	if ( _vm->_debugger->_showStatsVk
		&& !_vm->_actors[_actorId]->isSpeeching()
		&& !_vm->_actors[kActorMcCoy]->isSpeeching()
		&& !_vm->_actors[kActorAnsweringMachine]->isSpeeching()
		&& !_isClosing
	) {
		_vm->_subtitles->setGameSubsText(Common::String::format("Calibration: %02d Ratio: %02d Anxiety: %02d%%\nReplicant: %02d%% Human: %02d%%", _calibration, _calibrationRatio, _anxiety, _replicantProbability, _humanProbability), true);
		_vm->_subtitles->show();
	}

	_vm->_subtitles->tick(_vm->_surfaceFront);

	_vm->blitToScreen(_vm->_surfaceFront);

	// unsigned difference is intentional
	if (_isClosing && (_vm->_time->current() - _timeCloseStart >= 3000u) && !_script->isInsideScript()) {
		close();
		_vm->_mouse->enable();
		reset();
	}
}

void VK::handleMouseDown(int mouseX, int mouseY, bool mainButton) {
	if (!_vm->_mouse->isDisabled()) {
		if (!_buttons->handleMouseAction(mouseX, mouseY, true, false, false)) {
			tick();
		}
	}
}

void VK::handleMouseUp(int mouseX, int mouseY, bool mainButton) {
	if (!_vm->_mouse->isDisabled()) {
		_buttons->handleMouseAction(mouseX, mouseY, false, true, false);
	}
}

void VK::addQuestion(int intensity, int sentenceId, int relatedSentenceId) {
	for (int i = 0; i < (int)_questions[intensity].size(); ++i) {
		if (!_questions[intensity][i].isPresent) {
			_questions[intensity][i].isPresent = true;
			_questions[intensity][i].sentenceId = sentenceId;
			_questions[intensity][i].relatedSentenceId = relatedSentenceId;
			return;
		}
	}
}

void VK::playSpeechLine(int actorId, int sentenceId, float duration) {
	_vm->gameWaitForActive();

	_vm->_mouse->disable();

	Actor *actor = _vm->_actors[actorId];
	actor->speechPlay(sentenceId, true);

	while (_vm->_gameIsRunning) {
		_vm->_actorIsSpeaking = true;
		_vm->_actorSpeakStopIsRequested = false;
		_vm->gameTick();
		_vm->_actorIsSpeaking = false;
		if (_vm->_actorSpeakStopIsRequested || !actor->isSpeeching()) {
			actor->speechStop();
			break;
		}
	}

	if (duration > 0.0f && !_vm->_actorSpeakStopIsRequested) {
		uint32  timeStart = _vm->_time->current();
		uint32  timeUntil = duration * 1000.0f;
		while ((_vm->_time->current() - timeStart < timeUntil) && _vm->_gameIsRunning) {
			_vm->gameTick();
		}
	}

	_vm->_actorSpeakStopIsRequested = false;

	_vm->_mouse->enable();
}

void VK::subjectReacts(int intensity, int humanResponse, int replicantResponse, int anxiety) {
	humanResponse     = CLIP(humanResponse, -20, 20);
	replicantResponse = CLIP(replicantResponse, -20, 20);

	uint32 timeNow = _vm->_time->current();
	bool closeVK   = false;

	if (intensity > 0) {
		_needleValueTarget = 78 * intensity / 100;
		_needleValueDelta = (_needleValueTarget - _needleValue) / 10;
		_timeNextNeedleStepStart = timeNow;
	}

	if (humanResponse != 0) {
		_humanProbability = CLIP(_humanProbability + humanResponse + _calibration, 0, 100);
		// debug("Human probability is %d, human response is %d, calibration is %d", _humanProbability, humanResponse,_calibration);
		if (_humanProbability >= 80 && !_isClosing) {
			closeVK = false;
			if (_vm->_debugger->_playFullVk
			    && intensity == 5
			    && (humanResponse == 0 || humanResponse == 20 )
			    && replicantResponse == 0
			    && anxiety == 100
			) {	// only close if 5, 0, 0, 100 argument (the actual anxiety ending of VK)
				// force Human result
				_replicantProbability = 0;
				closeVK = true;
			} else if (!_vm->_debugger->_playFullVk) {
				closeVK = true;
			}

			if (closeVK == true) {
				_isClosing = true;
				_timeCloseStart = timeNow;
				_vm->_mouse->disable();
			}
		}
		_humanGaugeTarget = humanResponse;
		_humanGaugeDelta = humanResponse / 10;
		if (_humanGaugeDelta == 0) {
			_humanGaugeDelta = humanResponse / abs(humanResponse);
		}
	}

	if (replicantResponse != 0) {
		_replicantProbability = CLIP(_replicantProbability + replicantResponse - _calibration, 0, 100);
		// debug("Replicant probability is %d, replicant response is %d, calibration is %d", _replicantProbability, replicantResponse, _calibration);
		if (_replicantProbability >= 80 && !_isClosing) {
			closeVK = false;
			if (_vm->_debugger->_playFullVk
			    && intensity == 5
			    && humanResponse == 0
			    && (replicantResponse == 0 || replicantResponse == 20 )
			    && anxiety == 100
			) { // only close if 5, 0, 0, 100 argument (the actual anxiety ending of VK)
				// force Rep result
				_humanProbability = 0;
				closeVK = true;
			} else if (!_vm->_debugger->_playFullVk) {
				closeVK = true;
			}

			if (closeVK == true) {
				_isClosing = true;
				_timeCloseStart = timeNow;
				_vm->_mouse->disable();
			}
		}
		_replicantGaugeTarget = replicantResponse;
		_replicantGauge = replicantResponse / 10;
		if (_replicantGaugeDelta == 0) {
			_replicantGaugeDelta = replicantResponse / abs(replicantResponse);
		}
	}

	closeVK = false;
	if (_vm->_debugger->_playFullVk
	    && intensity == 5
	    && humanResponse == 0
	    && replicantResponse == 0
	    && anxiety == 100 && !_isClosing
	) {
		closeVK = true;
	} else if (!_vm->_debugger->_playFullVk) {
		_anxiety = CLIP(_anxiety + anxiety, 0, 100);
		if (_anxiety == 100 && !_isClosing) {
			closeVK = true;
		}
	}

	if (closeVK == true) {
		_isClosing = true;
		_timeCloseStart = timeNow;
		_vm->_mouse->disable();
	}
}

void VK::eyeAnimates(int loopId) {
	_vqaPlayerEye->setLoop(loopId, -1, kLoopSetModeImmediate, nullptr, nullptr);
	_vqaPlayerEye->setLoop(0, -1, kLoopSetModeEnqueue, nullptr, nullptr);
}

void VK::mouseDownCallback(int buttonId, void *callbackData) {
	VK *self = (VK *)callbackData;

	switch (buttonId) {
	case 1:
		self->startAdjustement();
		break;
	case 2:
	case 3:
	case 4:
		if (self->_calibrationStarted) {
			self->_vm->_audioPlayer->playAud(self->_vm->_gameInfo->getSfxTrack(kSfxBUTN6), 100, 0, 0, 50, 0);
		}
		break;
	case 5:
		self->_vm->_audioPlayer->playAud(self->_vm->_gameInfo->getSfxTrack(kSfxBUTN6), 100, 0, 0, 50, 0);
		break;
	default:
		return;
	}
}

void VK::mouseUpCallback(int buttonId, void *callbackData) {
	VK *self = (VK *)callbackData;

	switch (buttonId) {
	case 0:
		self->calibrate();
		break;
	case 1:
		self->stopAdjustement();
		break;
	case 2:
		self->askQuestion(0);
		break;
	case 3:
		self->askQuestion(1);
		break;
	case 4:
		self->askQuestion(2);
		break;
	case 5:
		self->_isClosing = true;
		break;
	default:
		return;
	}
}

void VK::loopEnded(void *callbackData, int frame, int loopId) {
	VK *self = (VK *)callbackData;

	self->_vqaLoopEnded = true;
	self->_vqaPlayerMain->setLoop(2, -1, kLoopSetModeJustStart, nullptr, nullptr);
}

void VK::reset() {
	_actorId = -1;

	_buttons       = nullptr;
	_vqaPlayerMain = nullptr;
	_vqaPlayerEye  = nullptr;
	_vqaFrameMain  = -1;

	_script = nullptr;

	_isOpen = false;

	_shapes.clear();

	_volumeAmbient = 0;
	_volumeMusic   = 0;

	_calibrationRatio   = 0;
	_calibrationCounter = 0;
	_calibrationStarted = false;
	_calibration        = 0;

	_testStarted = false;

	_needleValue             = 0;
	_needleValueTarget       = 0;
	_needleValueDelta        = 0;
	_needleValueMax          = 0;

	_timeNextNeedleStepStart      = 0u;
	_timeNeedleReturnStart        = 0u;
	_timeNextNeedleOscillateStart = 0u;

	_humanProbability                = 0;
	_humanGauge                      = 0;
	_humanGaugeTarget                = 0;
	_humanGaugeDelta                 = 0;
	_timeNextHumanGaugeStepDiff      = 0u;
	_timeNextHumanGaugeStepStart     = 0u;

	_replicantProbability            = 0;
	_replicantGauge                  = 0;
	_replicantGaugeTarget            = 0;
	_replicantGaugeDelta             = 0;
	_timeNextReplicantGaugeStepDiff  = 0u;
	_timeNextReplicantGaugeStepStart = 0u;

	_anxiety = 0;

	_blinkState               = 0;
	_timeNextBlinkStart       = 0u;
	_timeNextGaugesBlinkStart = 0u;

	_isClosing      = false;
	_timeCloseStart = 0u;

	_isAdjusting                  = false;
	_adjustment                   = 154;
	_adjustmentTarget             = 154;
	_adjustmentDelta              = 0;
	_timeNextAdjustementStepStart = 0u;

	_eyeLineSelected           = 1;
	_eyeLineX                  = 315;
	_eyeLineXLast              = 315;
	_eyeLineY                  = 281;
	_eyeLineYLast              = 281;
	_eyeLineXDelta             = 8;
	_eyeLineYDelta             = 8;
	_timeNextEyeLineStepStart  = 0u;
	_timeNextEyeLineStart      = 0u;

	_soundTrackId1 = -1;
	_soundTrackId2 = -1;
	_soundTrackId3 = -1;
	_vqaLoopEnded = false;

	_surfaceEye.free();
}

void VK::init() {
	_vm->_mouse->disable();

	_buttons->activate(nullptr, nullptr, mouseDownCallback, mouseUpCallback, this);
	_buttons->defineImage(0, Common::Rect(191, 364, 218, 373), nullptr,    _shapes[2],  _shapes[3],  _vm->_textVK->getText(1));
	_buttons->defineImage(1, Common::Rect(154, 258, 161, 265), _shapes[4], _shapes[4],  _shapes[5],  _vm->_textVK->getText(2));
	_buttons->defineImage(2, Common::Rect(515, 368, 538, 398), nullptr,    _shapes[6],  _shapes[7],  nullptr);
	_buttons->defineImage(3, Common::Rect(548, 368, 571, 398), nullptr,    _shapes[8],  _shapes[9],  nullptr);
	_buttons->defineImage(4, Common::Rect(581, 368, 604, 398), nullptr,    _shapes[10], _shapes[11], nullptr);
	_buttons->defineImage(5, Common::Rect( 31, 363,  65, 392), nullptr,    _shapes[0],  _shapes[1], _vm->_textVK->getText(0));
	_buttons->defineImage(6, Common::Rect( 59, 262,  87, 277), nullptr,    nullptr,     nullptr,    _vm->_textVK->getText(6));
	_buttons->defineImage(7, Common::Rect( 59, 306,  87, 322), nullptr,    nullptr,     nullptr,    _vm->_textVK->getText(7));

	_script->initialize(_actorId);

	_vqaPlayerMain->setLoop(0, -1, kLoopSetModeJustStart, nullptr, nullptr);
	tick();
	_vqaPlayerMain->setLoop(1, -1, kLoopSetModeEnqueue, loopEnded, this);
}

void VK::draw() {
	if (!_isOpen || !_vm->_windowIsActive) {
		return;
	}

	int frame = _vqaPlayerMain->update();
	if (frame >= 0) {
		_vqaFrameMain = frame;
	}

	if (frame == 0) {
		_soundTrackId2 = _vm->_audioPlayer->playAud(_vm->_gameInfo->getSfxTrack(kSfxBR011_2A), 33, 0, 0, 50, 0);
		_vm->_audioPlayer->playAud(_vm->_gameInfo->getSfxTrack(kSfxBR016_2B), 50, 0, 0, 50, 0);
	} else if (frame == 26) {
		setAdjustment(158);
		_vm->_audioPlayer->stop(_soundTrackId2, false);
		_soundTrackId1 = _vm->_audioPlayer->playAud(_vm->_gameInfo->getSfxTrack(kSfxBR014_5A), 50, 30, 30, 50, kAudioPlayerLoop);
	} else if (frame == 40) {
		_vm->_audioPlayer->playAud(_vm->_gameInfo->getSfxTrack(kSfxBR013_3D), 33, 0, 0, 50, 0);
		eyeAnimates(1);
	} else if (frame == 59) {
		_vm->_mouse->enable();
		_buttons->setImageShapeHovered(2, nullptr);
		_buttons->setImageShapeDown(2, nullptr);
		_buttons->setImageShapeHovered(3, nullptr);
		_buttons->setImageShapeDown(3, nullptr);
		_buttons->setImageShapeHovered(4, nullptr);
		_buttons->setImageShapeDown(4, nullptr);
	} else if (frame == 100) {
		if (_vm->_rnd.getRandomNumberRng(0, 100) > 60) {
			eyeAnimates(1);
		}
	} else if (frame == 140) {
		if (_vm->_rnd.getRandomNumberRng(0, 10) > 6) {
			_soundTrackId3 = _vm->_audioPlayer->playAud(_vm->_gameInfo->getSfxTrack(kSfxBR003_1A), 83, 0, 0, 50, 0);
		}
	}

	blit(_vm->_surfaceBack, _vm->_surfaceFront);

	Graphics::Surface &surface = _vm->_surfaceFront;

	uint32 timeNow = _vm->_time->current();

	if (_isAdjusting && !_testStarted && !_vm->isMouseButtonDown()) {
		_isAdjusting = false;
	}

	if (_vqaFrameMain >= 26) {
		// unsigned difference is intentional
		if (_isClosing && timeNow - _timeNextGaugesBlinkStart >= 600u) {
			if (_blinkState) {
				_buttons->setImageShapeUp(6, nullptr);
				_buttons->setImageShapeUp(7, nullptr);
				_blinkState = 0;
			} else {
				if (_humanProbability >= 80) {
					_buttons->setImageShapeUp(6, _shapes[13]);
					_vm->_audioPlayer->playAud(_vm->_gameInfo->getSfxTrack(kSfxCROSLOCK), 100, 0, 0, 50, 0);
				}
				if (_replicantProbability >= 80) {
					_buttons->setImageShapeUp(7, _shapes[14]);
					_vm->_audioPlayer->playAud(_vm->_gameInfo->getSfxTrack(kSfxCROSLOCK), 100, 0, 0, 50, 0);
				}
				_blinkState = 1;
			}
			_timeNextGaugesBlinkStart = timeNow;
		}

		_buttons->draw(surface);

		// unsigned difference is intentional
		if (_humanGaugeDelta != 0 && timeNow - _timeNextHumanGaugeStepStart >= _timeNextHumanGaugeStepDiff) {
			_humanGauge += _humanGaugeDelta;

			if ((_humanGaugeDelta > 0 && _humanGauge >= _humanGaugeTarget)
			 || (_humanGaugeDelta < 0 && _humanGauge <= _humanGaugeTarget)) {
				_humanGauge = _humanGaugeTarget;
			}

			if (_humanGauge == _humanGaugeTarget) {
				if (_humanGaugeTarget != 0) {
					_humanGaugeTarget = 0;
					_humanGaugeDelta = -_humanGaugeDelta;
					_timeNextHumanGaugeStepDiff  = 500u;
					_timeNextHumanGaugeStepStart = timeNow;
				} else {
					_humanGaugeDelta = 0;
				}
			} else {
				_timeNextHumanGaugeStepDiff  = 66u;
				_timeNextHumanGaugeStepStart = timeNow;
			}
		}
		drawHumanGauge(surface);

		// unsigned difference is intentional
		if (_replicantGaugeDelta != 0 && timeNow - _timeNextReplicantGaugeStepStart >= _timeNextReplicantGaugeStepDiff) {
			_replicantGauge += _replicantGaugeDelta;

			if ((_replicantGaugeDelta > 0 && _replicantGauge >= _replicantGaugeTarget)
			 || (_replicantGaugeDelta < 0 && _replicantGauge <= _replicantGaugeTarget)) {
				_replicantGauge = _replicantGaugeTarget;
			}

			if (_replicantGauge == _replicantGaugeTarget) {
				if (_replicantGaugeTarget != 0) {
					_replicantGaugeTarget = 0;
					_replicantGaugeDelta = -_replicantGaugeDelta;
					_timeNextReplicantGaugeStepDiff  = 500u;
					_timeNextReplicantGaugeStepStart = timeNow;
				} else {
					_replicantGaugeDelta = 0;
				}
			} else {
				_timeNextReplicantGaugeStepDiff  = 66u;
				_timeNextReplicantGaugeStepStart = timeNow;
			}
		}
		drawReplicantGauge(surface);

		// unsigned difference is intentional
		if (!_calibrationStarted && _vqaFrameMain >= 59 && timeNow - _timeNextBlinkStart >= 600u) {
			if (_blinkState) {
				_buttons->setImageShapeUp(0, nullptr);
				_blinkState = false;
			} else {
				_buttons->setImageShapeUp(0, _shapes[2]);
				_vm->_audioPlayer->playAud(_vm->_gameInfo->getSfxTrack(kSfxVKBEEP1), 50, 0, 0, 50, 0);
				_blinkState = true;
			}
			_timeNextBlinkStart = timeNow;
		}

		// unsigned difference is intentional
		if (_adjustmentDelta != 0 && timeNow - _timeNextAdjustementStepStart >= 50u) {
			if (_adjustmentDelta > 0) {
				_adjustment += 3;
				if (_adjustment >= _adjustmentTarget) {
					_adjustment = _adjustmentTarget;
					_adjustmentDelta = 0;
				}
			} else {
				_adjustment -= 3;
				if (_adjustment <= _adjustmentTarget) {
					_adjustment = _adjustmentTarget;
					_adjustmentDelta = 0;
				}
			}
			setAdjustment(_adjustment + 4);
		}
		setAdjustmentFromMouse();

		// unsigned difference is intentional
		if (_calibrationStarted && !_testStarted && timeNow - _timeNextBlinkStart >= 600u) {
			if (_blinkState) {
				_buttons->setImageShapeUp(2, nullptr);
				_buttons->setImageShapeUp(3, nullptr);
				_buttons->setImageShapeUp(4, nullptr);
				_blinkState = 0;
			} else {
				_buttons->setImageShapeUp(2, _shapes[7]);
				_buttons->setImageShapeUp(3, _shapes[9]);
				_buttons->setImageShapeUp(4, _shapes[11]);
				_blinkState = 1;

				_vm->_audioPlayer->playAud(_vm->_gameInfo->getSfxTrack(kSfxVKBEEP2), 33, 0, 0, 50, 0);
			}
			_timeNextBlinkStart = timeNow;
		}

		// unsigned difference is intentional
		if (_needleValueDelta != 0 && (timeNow - _timeNextNeedleStepStart >= 66u)) {
			if (_needleValueDelta > 0) {
				_needleValue += 4;
				if (_needleValue >= _needleValueTarget) {
					_needleValue = _needleValueTarget;
					_needleValueMax = _needleValueTarget;
					_needleValueDelta = -_needleValueDelta;
					_needleValueTarget = 0;

					_timeNeedleReturnStart = timeNow;

					if (!_testStarted) {
						animateAdjustment(_needleValueMax + 165);
					}
				}
			} else if (timeNow - _timeNeedleReturnStart >= 1800u) {
				// unsigned difference is intentional
				_needleValue -= 4;
				if (_needleValue <= _needleValueTarget) {
					_needleValue = _needleValueTarget;
					_needleValueDelta = 0;
				}
			}
			_vm->_audioPlayer->playAud(_vm->_gameInfo->getSfxTrack(kSfxBR027_3P), 20, 0, 0, 50, 0);
			_timeNextNeedleStepStart = timeNow;
		}

		drawNeedle(surface);
		drawEye(surface);
		drawEyeCrosshair(surface, timeNow);
		// unsigned difference is intentional
		if (timeNow - _timeNextNeedleOscillateStart >= 66u) {
			_timeNextNeedleOscillateStart = timeNow;
		}
		int mouseX, mouseY;
		_vm->_mouse->getXY(&mouseX, &mouseY);
		_buttons->drawTooltip(surface, mouseX, mouseY);
	}

	drawMouse(surface);
}

void VK::drawNeedle(Graphics::Surface &surface) {
	int x = _needleValue + 165;
	// unsigned difference is intentional
	if ((_vm->_time->current() - _timeNextNeedleOscillateStart >= 66u) && (x > 165)) {
		x = CLIP(x + (int)_vm->_rnd.getRandomNumberRng(0, 4) - 2, 165, 245);
	}

	int needleOffset = abs(38 - _needleValue);
	int y = 345 - sqrt(72 * 72 - needleOffset * needleOffset);

	float colorIntensity = MIN(78.0f, _needleValue + 39.0f) / 78.0f;

	uint32 color1 = surface.format.RGBToColor(56 - 48 * colorIntensity, 144 - 64 * colorIntensity, 184 - 96 * colorIntensity);
	uint32 color2 = surface.format.RGBToColor(56 - 24 * colorIntensity, 144 - 32 * colorIntensity, 184 - 48 * colorIntensity);

	surface.drawLine(203, 324, x - 2, y,     color1);
	surface.drawLine(203, 324, x + 2, y,     color1);
	surface.drawLine(203, 324, x - 1, y,     color2);
	surface.drawLine(203, 324, x + 1, y,     color2);
	surface.drawLine(203, 324, x,     y - 1, color2);
	surface.drawLine(203, 324, x,     y,     surface.format.RGBToColor(56, 144, 184));
}

void VK::drawEye(Graphics::Surface &surface) {
	_vqaPlayerEye->update(true);
	surface.copyRectToSurface(_surfaceEye, 315, 281, Common::Rect(0, 0, _surfaceEye.w, _surfaceEye.h));
}

void VK::drawEyeCrosshair(Graphics::Surface &surface, uint32 timeNow) {
	surface.drawLine(315,                                        _eyeLineY,     486,                                        _eyeLineY,     surface.format.RGBToColor(16, 16, 64));
	surface.drawLine(315,                                        _eyeLineY - 1, 486,                                        _eyeLineY - 1, surface.format.RGBToColor(16, 16, 64));
	surface.drawLine(315,                                        _eyeLineY,     _vm->_rnd.getRandomNumberRng(10, 20) + 315, _eyeLineY,     surface.format.RGBToColor(16, 16, 80));
	surface.drawLine(486 - _vm->_rnd.getRandomNumberRng(10, 20), _eyeLineY,     486,                                        _eyeLineY,     surface.format.RGBToColor(16, 16, 80));
	surface.drawLine(486 - _vm->_rnd.getRandomNumberRng(10, 20), _eyeLineY - 1, 486,                                        _eyeLineY - 1, surface.format.RGBToColor(16, 16, 48));
	surface.drawLine(315,                                        _eyeLineY - 1, _vm->_rnd.getRandomNumberRng(10, 20) + 315, _eyeLineY - 1, surface.format.RGBToColor(16, 16, 48));

	surface.drawLine(_eyeLineX,     281,                                        _eyeLineX,     396,                                        surface.format.RGBToColor(16, 16, 64));
	surface.drawLine(_eyeLineX - 1, 281,                                        _eyeLineX - 1, 396,                                        surface.format.RGBToColor(16, 16, 64));
	surface.drawLine(_eyeLineX,     281,                                        _eyeLineX,     _vm->_rnd.getRandomNumberRng(10, 20) + 281, surface.format.RGBToColor(16, 16, 48));
	surface.drawLine(_eyeLineX,     396 - _vm->_rnd.getRandomNumberRng(10, 20), _eyeLineX,     396,                                        surface.format.RGBToColor(16, 16, 48));
	surface.drawLine(_eyeLineX - 1, 396 - _vm->_rnd.getRandomNumberRng(10, 20), _eyeLineX - 1, 396,                                        surface.format.RGBToColor(16, 16, 80));
	surface.drawLine(_eyeLineX - 1, 281,                                        _eyeLineX - 1, _vm->_rnd.getRandomNumberRng(10, 20) + 281, surface.format.RGBToColor(16, 16, 80));

	// unsigned difference is intentional
	if (timeNow - _timeNextEyeLineStart >= 1000u) {
		if (_eyeLineSelected) {
			if (_eyeLineYLast != _eyeLineY) {
				surface.drawLine(315, _eyeLineYLast, 486, _eyeLineYLast, surface.format.RGBToColor(16, 16, 32));
			}
			_eyeLineYLast = _eyeLineY;
			// unsigned difference is intentional
			if (timeNow - _timeNextEyeLineStepStart >= 50u) {
				_eyeLineY += _eyeLineYDelta;
				if (_eyeLineYDelta > 0) {
					if (_eyeLineY >= 396) {
						_eyeLineY = 396;
						_eyeLineYDelta = -_eyeLineYDelta;
					}
				} else if (_eyeLineY <= 281) {
					_eyeLineY = 281;
					_eyeLineYDelta = -_eyeLineYDelta;
					_eyeLineSelected = 0;
					_timeNextEyeLineStart = timeNow;
				}
				_timeNextEyeLineStepStart = timeNow;
			}
		} else {
			if (_eyeLineXLast != _eyeLineX) {
				surface.drawLine(_eyeLineXLast, 281, _eyeLineXLast, 396, surface.format.RGBToColor(16, 16, 32));
			}
			_eyeLineXLast = _eyeLineX;
			// unsigned difference is intentional
			if (timeNow - _timeNextEyeLineStepStart >= 50u) {
				_eyeLineX += _eyeLineXDelta;
				if ( _eyeLineXDelta > 0) {
					if (_eyeLineX >= 486) {
						_eyeLineX = 486;
						_eyeLineXDelta = -_eyeLineXDelta;
					}
				} else if (_eyeLineX <= 315) {
					_eyeLineX = 315;
					_eyeLineXDelta = -_eyeLineXDelta;
					_eyeLineSelected = 1;
					_timeNextEyeLineStart = timeNow;
				}
				_timeNextEyeLineStepStart = timeNow;
			}
		}
	}
}

void VK::drawMouse(Graphics::Surface &surface) {
	if (_vm->_mouse->isDisabled()) {
		return;
	}

	Common::Point p = _vm->getMousePos();

	if (_buttons->hasHoveredImage()) {
		_vm->_mouse->setCursor(1);
	} else {
		_vm->_mouse->setCursor(0);
	}

	_vm->_mouse->draw(surface, p.x, p.y);
}

void VK::drawGauge(Graphics::Surface &surface, int value, int x, int y, int width) {
	_shapes[12]->draw(surface, x + (width / 2) * value / 20 , y);
}

void VK::drawHumanGauge(Graphics::Surface &surface) {
	drawGauge(surface, _humanGauge, 72, 271, 87);
}

void VK::drawReplicantGauge(Graphics::Surface &surface) {
	drawGauge(surface, _replicantGauge, 72, 293, 87);
}

void VK::calibrate() {
	if (_calibrationCounter >= 3 || _testStarted) {
		_vm->_audioPlayer->playAud(_vm->_gameInfo->getSfxTrack(kSfxBRTARGET), 100, 0, 0, 50, 0);
	} else {
		_vm->_mouse->disable();
		_script->calibrate(_actorId);
		_vm->_mouse->enable();
		++_calibrationCounter;
		if (_calibrationCounter == 3) {
			_buttons->setImageShapeHovered(0, nullptr);
			_buttons->setImageShapeDown(0, nullptr);
			_buttons->setImageTooltip(0, nullptr);
		}
	}
	_calibrationStarted = true;
	_buttons->setImageShapeUp(0, nullptr);

	_buttons->setImageShapeHovered(2, _shapes[6]);
	_buttons->setImageShapeDown(2, _shapes[7]);
	_buttons->setImageTooltip(2, _vm->_textVK->getText(3));

	_buttons->setImageShapeHovered(3, _shapes[8]);
	_buttons->setImageShapeDown(3, _shapes[9]);
	_buttons->setImageTooltip(3, _vm->_textVK->getText(4));

	_buttons->setImageShapeHovered(4, _shapes[10]);
	_buttons->setImageShapeDown(4, _shapes[11]);
	_buttons->setImageTooltip(4, _vm->_textVK->getText(5));
}

void VK::beginTest() {
	if (_calibrationStarted && !_testStarted) {
		_vm->_mouse->disable();
		_calibration = ((100.0f / (100.0f - 4.0f) * (_adjustment - 154.0f)) - _calibrationRatio) / 5.0f;
		_script->beginTest(_actorId);
		_testStarted = true;
		_buttons->setImageShapeHovered(0, nullptr);
		_buttons->setImageShapeDown(0, nullptr);
		_buttons->setImageTooltip(0, nullptr);
		_buttons->setImageShapeDown(1, _shapes[4]);
		_buttons->setImageTooltip(1, nullptr);
		_buttons->setImageShapeUp(2, nullptr);
		_buttons->setImageShapeUp(3, nullptr);
		_buttons->setImageShapeUp(4, nullptr);
		_vm->_mouse->enable();
	} else {
		_vm->_audioPlayer->playAud(_vm->_gameInfo->getSfxTrack(kSfxBRTARGET), 100, 0, 0, 50, 0);
	}
}

void VK::startAdjustement() {
	if (_testStarted) {
		_vm->_audioPlayer->playAud(_vm->_gameInfo->getSfxTrack(kSfxBRTARGET), 100, 0, 0, 50, 0);
	} else {
		_isAdjusting = true;
	}
}

void VK::stopAdjustement() {
	if (_testStarted) {
		_isAdjusting = false;
	}
}

void VK::animateAdjustment(int target) {
	_adjustmentTarget = MAX(target - 4, 154);
	_adjustmentDelta = (_adjustmentTarget - _adjustment) / 5;
	_timeNextAdjustementStepStart = _vm->_time->current();
}

void VK::setAdjustment(int x) {
	_adjustment = CLIP(x - 4, 154, 246);
	int offset = abs(199 - _adjustment);
	int y = sqrt(88 * 88 - offset * offset);
	_buttons->setImageLeft(1, _adjustment);
	_buttons->setImageTop(1, 345 - y);
}

void VK::setAdjustmentFromMouse() {
	if (_isAdjusting && !_testStarted) {
		int mouseX, mouseY;
		_vm->_mouse->getXY(&mouseX, &mouseY);
		setAdjustment(mouseX);
		if (_adjustmentTarget != _adjustment) {
			_vm->_audioPlayer->playAud(_vm->_gameInfo->getSfxTrack(kSfxBRWIND2), 100, 0, 0, 50, 0);
		}
		_adjustmentTarget = _adjustment;
		_adjustmentDelta = 0;
	}
}

/**
* This is ScummVM's version of this function (original name: findQuestionById)
* It will search through all questions to find a related question Id and its intensity
*/
void VK::findRelatedQuestionBySentenceId(int inSentenceId, int &outRelatedQuestionId, int &outRelatedIntensity) {
    outRelatedQuestionId = -1;
    outRelatedIntensity  = -1;

	for (int intensity = 0; intensity < 3; ++intensity) {
		for (int i = 0; i < (int)_questions[intensity].size(); ++i) {
			if (_questions[intensity][i].isPresent
				&& _questions[intensity][i].sentenceId == inSentenceId
			) {
				outRelatedQuestionId = i;
				outRelatedIntensity  = intensity;
				return;
			}
		}
	}
}

void VK::askQuestion(int intensity) {
	assert(intensity < (int)_questions.size());

	if (!_testStarted) {
		beginTest();
	}

	if (!_testStarted) {
		return;
	}

	int foundQuestionIndex     = -1;
	int foundQuestionIndexLast = -1;

	if (_vm->_debugger->_playFullVk) {
		// loop backwards to maintain proper order in sound files
		for (int i = (int)_questions[intensity].size() - 1; i >= 0; --i) {

			if (_questions[intensity][i].isPresent && !_questions[intensity][i].wasAsked) {
				// In full VK debug mode we don't need the question dependencies (related questions)
				// We also assign the new question id deterministically from an intensity "pack"
				// we don't use randomness here
				foundQuestionIndexLast = i;
				foundQuestionIndex     = i;
			}
		}
	} else {
		// original code
		for (int i = 0; i < (int)_questions[intensity].size(); ++i) {
			if (_questions[intensity][i].isPresent && !_questions[intensity][i].wasAsked) {
				// cut content? related questions are not used in game
				//
				// Note: There are questions that seem related and a subject may reference them
				// (eg Bullet Bob does that with the "hamster" - VK Low 05 and VK Medium 14)
				//
				// This was probably meant to be a mechanism to prevent asking a related question before asking the original one
				// to avoid inconsistencies. However, this seems it would restrict relevant questions within the same intensity
				// which should be changed.
				//
				// An issue with this issue is that if in a pair of related questions from different intensities
				// (eg in Bob's case L05 -> M14), if M14 is the last available question from the Medium intensity,
				// and L05 has not been asked, then what question should McCoy ask?
				//
				// The original code (if it worked) would simply not make any question and that's the simplest solution to adopt.
				// This issue is only likely to occur in vanilla game (with very low probability)
				// with High intensity questions that depend on questions of other intensity
				//
				int relatedSentenceId = _questions[intensity][i].relatedSentenceId;
				int relatedQuestionId = -1;
				int relatedQuestionIntensity = -1;

				if (relatedSentenceId >= 0) {
					findRelatedQuestionBySentenceId(relatedSentenceId, relatedQuestionId, relatedQuestionIntensity);
				}

				if (relatedQuestionId < 0 || _questions[relatedQuestionIntensity][relatedQuestionId].wasAsked) {
					foundQuestionIndexLast = i;
					if (_vm->_rnd.getRandomNumberRng(0, 100) < 20) {
						foundQuestionIndex = i;
						break;
					}
				}
			}
		}
	}

	if (foundQuestionIndex < 0) {
		foundQuestionIndex = foundQuestionIndexLast;
	}

	if (foundQuestionIndex >= 0) {
		_vm->_mouse->disable();
		_questions[intensity][foundQuestionIndex].wasAsked = true;
		_script->mcCoyAsksQuestion(_actorId, _questions[intensity][foundQuestionIndex].sentenceId);
		_script->questionAsked(_actorId, _questions[intensity][foundQuestionIndex].sentenceId);
		_vm->_mouse->enable();
	} else if (!_isClosing && !_script->isInsideScript()
	           && !_vm->_debugger->_playFullVk
	) {
		_isClosing = true;
		_vm->_mouse->disable();
		_timeCloseStart = _vm->_time->current();
	}
}

} // End of namespace BladeRunner
