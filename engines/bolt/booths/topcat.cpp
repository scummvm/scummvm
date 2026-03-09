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

#include "bolt/bolt.h"

namespace Bolt {

int16 Bolt::BoltEngine::topCatGame(int16 prevBooth) {
	int16 savedTimer = _xp->setInactivityTimer(30);

	if (!initTopCat()) {
		cleanUpTopCat();
		return 8;
	}

	int16 result = playTopCat();
	cleanUpTopCat();

	_xp->setInactivityTimer(savedTimer);
	return result;
}

bool BoltEngine::initTopCat() {
	_topCatRtfHandle = openRTF(assetPath(_displayMode != 0 ? "topcatcr.av" : "topcatog.av"));
	if (!_topCatRtfHandle)
		return false;

	_topCatAvHandle = openRTF(assetPath("topcatau.av"));
	if (!_topCatAvHandle)
		return false;

	if (!openBOLTLib(&_topCatBoltLib, &_topCatBoltCallbacks, assetPath("topcat.blt")))
		return false;

	if (!getBOLTGroup(_topCatBoltLib, 0, 1))
		return false;

	_topCatBackgroundPalette = memberAddr(_topCatBoltLib, 0x20);
	_topCatBackground = memberAddr(_topCatBoltLib, _displayMode != 0 ? 0x22 : 0x21);
	_topCatBackgroundAnimationPalette = memberAddr(_topCatBoltLib, 0x00);
	_topCatGraphicsAssets = memberAddr(_topCatBoltLib, _displayMode != 0 ? 0x1E : 0x1D);

	_topCatBackgroundAnimFrame = 0;

	// Count background anim frames...
	_topCatMaxBackgroundAnimFrames = 0;
	while (getResolvedPtr(_topCatGraphicsAssets, _topCatMaxBackgroundAnimFrames * 4) != 0) {
		_topCatMaxBackgroundAnimFrames++;
	}

	_topCatCurrentAnimType = 0;
	_topCatAnimStateMachineStep = 0;
	_topCatQueuedSoundFrames = 0;
	_topCatAnimQueueSize = 0;

	_topCatButtonsPalette = memberAddr(_topCatBoltLib, 0x3D);
	_topCatBlinkEntries = memberAddr(_topCatBoltLib, 0x3E);

	_topCatLightsPalette = memberAddr(_topCatBoltLib, 0x31);
	_topCatBlinkSeqPtr = nullptr;

	_topCatSelectedChoiceOverlayGfx = memberAddr(_topCatBoltLib, 0x26);
	_topCatCycleData = memberAddr(_topCatBoltLib, 0x27);
	boltCycleToXPCycle(_topCatCycleData, _topCatCycleSpecs);

	_topCatChoiceCycleState->startIndex = 0;
	_topCatChoiceCycleState->endIndex = 0;
	_topCatChoiceCycleState->delay = 0;
	_topCatChoiceCycleState->nextFire = 0;
	_topCatChoiceCycleState->active = false;

	_topCatShuffledQuestions = memberAddr(_topCatBoltLib, 0x32); // [60]
	_topCatShuffledAnswers = memberAddr(_topCatBoltLib, 0x33); // [60]
	_topCatAnswersPermutations = memberAddr(_topCatBoltLib, 0x34); // [60 * 3]
	_topCatAnswers = memberAddr(_topCatBoltLib, 0x35);
	_topCatAnswersScreenPositions = memberAddr(_topCatBoltLib, 0x36);

	if (vLoad(&_topCatSavedScore, "TopCatBF")) {
		_topCatScore = _topCatSavedScore;
	} else {
		_topCatScore = 0;
	}

	if (vLoad(_topCatSaveBuffer, "TopCatBFStatic")) {
		_topCatShuffledQuestionsArrayIdx = READ_BE_INT16(_topCatSaveBuffer);

		int offset;
		for (int i = 0; i < ARRAYSIZE(_topCatSavedShuffledQuestions); i++) {
			offset = sizeof(int16) + i;
			_topCatSavedShuffledQuestions[i] = _topCatSaveBuffer[offset];
		}

		for (int i = 0; i < ARRAYSIZE(_topCatSavedShuffledAnswers); i++) {
			offset = sizeof(_topCatSavedShuffledQuestions) + sizeof(int16) + i;
			_topCatSavedShuffledAnswers[i] = _topCatSaveBuffer[offset];
		}

		for (int i = 0; i < ARRAYSIZE(_topCatSavedAnswersPermutations); i++) {
			offset = sizeof(_topCatSavedShuffledAnswers) + sizeof(_topCatSavedShuffledQuestions) + sizeof(int16) + i;
			_topCatSavedAnswersPermutations[i] = _topCatSaveBuffer[offset];
		}

		int16 answerOff = 0;
		for (int16 i = 0; i < 60; i++) {
			_topCatShuffledQuestions[i] = _topCatSavedShuffledQuestions[i];
			_topCatShuffledAnswers[i] = _topCatSavedShuffledAnswers[i];

			for (int16 j = 0; j < 3; j++) {
				_topCatAnswersPermutations[answerOff + j] = _topCatSavedAnswersPermutations[answerOff + j];
			}

			answerOff += 3;
		}
	} else {
		_topCatShuffledQuestionsArrayIdx = -1;
		shuffleTopCatQuestions();
		shuffleTopCatPermutations();
	}

	_topCatBlinkTimer = 0;

	getTopCatSoundInfo(_topCatBoltLib, 0x1F, &_topCatSoundInfo);

	uint32 dummy;
	while (_xp->getEvent(etTimer, &dummy) != etTimer);

	_xp->stopCycle();
	_xp->setTransparency(false);

	displayColors(_topCatBackgroundPalette, stFront, 0);
	displayPic(_topCatBackground, _displayX, _displayY, stFront);
	_xp->updateDisplay();

	_xp->setTransparency(true);
	displayColors(_topCatBackgroundAnimationPalette, stFront, 0);
	displayPic(_topCatBackground, _displayX, _displayY, stBack);

	byte *firstBackgroundAnimFrame = getResolvedPtr(_topCatGraphicsAssets, 0);
	displayPic(firstBackgroundAnimFrame, _displayX, _displayY, stFront);

	displayColors(_topCatBackgroundPalette, stBack, 0);

	if (!setupNextQuestion())
		return false;

	_xp->updateDisplay();

	_topCatHoveredEntry = nullptr;

	// Init button palettes...
	int16 off = 0;
	byte *entry;
	while (true) {
		entry = getResolvedPtr(_topCatButtonsPalette, off);
		if (!entry)
			break;

		if (READ_UINT32(entry) == 1)
			_topCatHelpButton = entry;

		if (READ_UINT32(entry) == 2)
			_topCatBackButton = entry;

		_xp->getPalette(READ_UINT16(entry + 0x0E), READ_UINT16(entry + 0x10), entry + 0x2E);
		unpackColors(READ_UINT16(entry + 0x10), entry + 0x12);

		off += 4;
	}

	// Init light palelttes...
	int16 lightOff = 0;
	for (int16 i = 0; i < 6; i++) {
		byte *lightEntry = getResolvedPtr(_topCatLightsPalette, lightOff);

		int16 colorCount = READ_UINT16(lightEntry + 4);
		int16 palStart = READ_UINT16(lightEntry);
		_xp->getPalette(palStart + 0x80, colorCount, lightEntry + colorCount * 4 + 8);

		for (int16 j = 0; j < colorCount; j++) {
			WRITE_UINT32(lightEntry + j * 4 + 8, READ_BE_UINT32(lightEntry + j * 4 + 8));
		}

		unpackColors(colorCount, lightEntry + 8);

		lightOff += 4;
	}

	// Set lights based on saved score...
	int16 litMask = (1 << _topCatScore) - 1;
	setScoreLights(litMask);

	return true;
}

void BoltEngine::cleanUpTopCat() {
	_xp->stopSound();
	_xp->stopCycle();

	if (_topCatScore < 6) {
		byte questionIdx = _topCatShuffledQuestions[_topCatShuffledQuestionsArrayIdx];
		_topCatShuffledAnswers[questionIdx]--;
		_topCatShuffledQuestionsArrayIdx--;
	} else {
		_topCatScore = 0;
	}

	_topCatSavedScore = _topCatScore;
	_topCatSaveHistory = _topCatShuffledQuestionsArrayIdx;

	int16 answerOff = 0;
	for (int16 i = 0; i < 60; i++) {
		_topCatSavedShuffledQuestions[i] = _topCatShuffledQuestions[i];
		_topCatSavedShuffledAnswers[i] = _topCatShuffledAnswers[i];

		for (int16 j = 0; j < 3; j++) {
			_topCatSavedAnswersPermutations[answerOff + j] = _topCatAnswersPermutations[answerOff + j];
		}

		answerOff += 3;
	}

	memset(_topCatSaveBuffer, 0, sizeof(_topCatSaveBuffer));

	WRITE_BE_INT16(_topCatSaveBuffer, _topCatSaveHistory);

	int offset;
	for (int i = 0; i < ARRAYSIZE(_topCatSavedShuffledQuestions); i++) {
		offset = sizeof(int16) + i;
		_topCatSaveBuffer[offset] = _topCatSavedShuffledQuestions[i];
	}

	for (int i = 0; i < ARRAYSIZE(_topCatSavedShuffledAnswers); i++) {
		offset = sizeof(_topCatSavedShuffledQuestions) + sizeof(int16) + i;
		_topCatSaveBuffer[offset] = _topCatSavedShuffledAnswers[i];
	}

	for (int i = 0; i < ARRAYSIZE(_topCatSavedAnswersPermutations); i++) {
		offset = sizeof(_topCatSavedShuffledAnswers) + sizeof(_topCatSavedShuffledQuestions) + sizeof(int16) + i;
		_topCatSaveBuffer[offset] = _topCatSavedAnswersPermutations[i];
	}

	vSave(&_topCatSavedScore, sizeof(_topCatSavedScore), "TopCatBF");
	vSave(_topCatSaveBuffer, sizeof(_topCatSaveBuffer), "TopCatBFStatic");

	freeBOLTGroup(_topCatBoltLib, 0, 1);
	closeBOLTLib(&_topCatBoltLib);

	if (_topCatRtfHandle) {
		closeRTF(_topCatRtfHandle);
		_topCatRtfHandle = nullptr;
	}

	if (_topCatAvHandle) {
		closeRTF(_topCatAvHandle);
		_topCatAvHandle = nullptr;
	}

	// These currently erroneously clear some graphics while on the "win a letter"
	// animation, unsure why the issue is not on the original...
#if 0
	_xp->fillDisplay(0, stFront);
	_xp->updateDisplay();
#endif
}

int16 BoltEngine::playTopCat() {
	int16 result = 0;
	int16 exitFlag = 0;

	setAnimType(1);
	queueAnim(3, 0); // Play question audio

	int16 cursorX = 120;
	int16 cursorY = 192;
	_xp->setCursorPos(cursorY, cursorX);
	_xp->setCursorColor(0x00, 0xFF, 0xFF);
	_xp->showCursor();

	while (true) {
		uint32 eventData;
		byte *soundDataPtr;
		int16 eventType = _xp->getEvent(etEmpty, &eventData, &soundDataPtr);

		switch (eventType) {
		case etEmpty: {
			if (!maintainAnim(0)) {
				return 8;
			}

			break;
		}

		case etTimer: { // Timer expired
			if (eventData != _topCatBlinkTimer)
				break;

			if (!_topCatBlinkEntry)
				break;

			_topCatBlinkTimer = _xp->startTimer(500);

			// Toggle highlight...
			int16 highlight;
			if (READ_UINT16(_topCatBlinkEntry + 0x0C) & 1) {
				highlight = 0;
			} else {
				highlight = 1;
			}

			highlightObject(_topCatBlinkEntry, highlight);
			break;
		}

		case etMouseMove: {
			cursorY = (int16)(eventData >> 16);
			cursorX = (int16)(eventData & 0xFFFF);

			// Hit-test against buttons...
			int16 entryIdx = 0;
			byte *entry = nullptr;
			int16 off = 0;

			while (true) {
				entry = getResolvedPtr(_topCatButtonsPalette, off);
				if (!entry)
					break;

				int16 entryY = READ_UINT16(entry + 4);
				int16 entryX = READ_UINT16(entry + 6);
				int16 entryH = READ_UINT16(entry + 8);
				int16 entryW = READ_UINT16(entry + 0x0A);

				if (cursorY > entryY &&
					cursorY < entryY + entryH &&
					cursorX > entryX &&
					cursorX < entryX + entryW)
					break;

				off += 4;
				entryIdx++;
			}

			// Unhighlight previous entry if needed...
			if (_topCatHoveredEntry != entry &&
				_topCatHoveredEntry != nullptr &&
				_topCatHoveredEntry != _topCatBlinkEntry &&
				_topCatHoveredEntry != _topCatBackButton) {

				if (_topCatHoveredEntry == _topCatHelpButton) {
					if (_topCatCurrentAnimType != 2)
						highlightObject(_topCatHoveredEntry, 0);
				} else {
					byte *hovered = _topCatHoveredEntry;
					if (READ_UINT32(hovered) < 3 || READ_UINT32(hovered) >= 6)
						highlightObject(_topCatHoveredEntry, 0);
				}
			}

			_topCatHoveredEntry = entry;

			// Highlight new entry if applicable...
			if (!entry)
				break;

			if (entry == _topCatBlinkEntry)
				break;

			if (entry == _topCatBackButton)
				break;

			if (READ_UINT32(entry) >= 3 && READ_UINT32(entry) < 6)
				break;

			highlightObject(_topCatHoveredEntry, 1);
			break;
		}

		case etMouseDown: {
			if (exitFlag)
				break;

			exitFlag = handleActionButton(&result);
			break;
		}

		case etSound: {
			int16 soundType;
			if (soundDataPtr >= _topCatSoundInfo.data && soundDataPtr < _topCatSoundInfo.data + _topCatSoundInfo.size) {
				soundType = 1;
			} else {
				soundType = 2;
			}

			if (!maintainAnim(soundType)) {
				return 8;
			}

			break;
		}

		case etInactivity: {
			// Toggle blinking highlight...
			int16 highlight;
			if (_topCatBlinkEntry == _topCatHoveredEntry &&
				_topCatBlinkEntry != 0) {
				if (READ_UINT32(_topCatBlinkEntry) < 3 || READ_UINT32(_topCatBlinkEntry) >= 6) {
					highlight = 1;
				} else {
					highlight = 0;
				}
			} else {
				highlight = 0;
			}

			highlightObject(_topCatBlinkEntry, highlight);

			// Set help button as new blinking entry...
			_topCatBlinkEntry = _topCatHelpButton;

			if (_topCatHelpButton != 0) {
				if (READ_UINT32(_topCatBlinkEntry) < 3 || READ_UINT32(_topCatBlinkEntry) >= 6) {
					_topCatBlinkTimer = _xp->startTimer(500);
				} else {
					_topCatBlinkTimer = 0;
				}
			} else {
				_topCatBlinkTimer = 0;
			}

			highlightObject(_topCatBlinkEntry, 1);
			break;
		}

		case etTrigger: { // Palette cycle event
			if (_topCatCurrentAnimType == 2) {
				// Toggle blinking highlight...
				int16 highlight;
				if (_topCatBlinkEntry == _topCatHoveredEntry &&
					_topCatBlinkEntry != 0) {
					if (READ_UINT32(_topCatBlinkEntry) < 3 || READ_UINT32(_topCatBlinkEntry) >= 6) {
						highlight = 1;
					} else {
						highlight = 0;
					}
				} else {
					highlight = 0;
				}

				highlightObject(_topCatBlinkEntry, highlight);

				// Advance to next blinking entry...
				if (_topCatCycleStep & 1) {
					_topCatBlinkEntry = nullptr;
				} else {
					_topCatBlinkEntry = getResolvedPtr(_topCatBlinkEntries, (_topCatCycleStep >> 1) * 4);
				}

				if (_topCatBlinkEntry != 0) {
					if (READ_UINT32(_topCatBlinkEntry) < 3 || READ_UINT32(_topCatBlinkEntry) >= 6) {
						_topCatBlinkTimer = _xp->startTimer(500);
					} else {
						_topCatBlinkTimer = 0;
					}
				} else {
					_topCatBlinkTimer = 0;
				}

				highlightObject(_topCatBlinkEntry, 1);
				_topCatCycleStep++;

			} else if (_topCatCurrentAnimType == 3) {
				// Unhighlight previous...
				if (_topCatCycleStep > 3) {
					byte *prevEntry = getResolvedPtr(_topCatButtonsPalette, (_topCatCycleStep - 1) * 4);
					highlightObject(prevEntry, 0);
				} else {
					highlightObject(_topCatBackButton, 0);
				}

				// Highlight next...
				byte *nextEntry = getResolvedPtr(_topCatButtonsPalette, _topCatCycleStep * 4);
				highlightObject(nextEntry, 1);
				_topCatCycleStep++;
			} else if (_topCatCurrentAnimType == 6) {
				// Light blinking animation...
				int16 mask = READ_UINT16(_topCatBlinkSeqPtr);
				_topCatBlinkSeqPtr++;

				setScoreLights(mask);
			}

			break;
		}

		default:
			break;
		}

		if (exitFlag && _topCatAnimQueueSize == 0 && _topCatCurrentAnimType == 1) {
			if (_topCatScore != 6)
				_xp->hideCursor();

			return result;
		}
	}
}

int16 BoltEngine::handleActionButton(int16 *result) {
	int16 prevState = _topCatCurrentAnimType;

	switch (_topCatCurrentAnimType) {
	case 2:
	case 3:
		stopAnimation();
		_xp->stopSound();
		_topCatQueuedSoundFrames = 0;
		setAnimType(1);
		break;

	case 4:
	case 5:
	case 6:
		return 0;
	default:
		break;
	}

	if (_topCatAnimQueueSize != 0 && _topCatHoveredEntry != 0) {
		if (READ_UINT32(_topCatHoveredEntry) != 0)
			return 0;
	}

	if (!_topCatHoveredEntry)
		return 0;

	_xp->setInactivityTimer(30);

	// If blinking entry is the help button, stop blinking...
	if (_topCatBlinkEntry == _topCatHelpButton) {
		int16 highlight;
		if (_topCatBlinkEntry == _topCatHoveredEntry &&
			_topCatBlinkEntry != 0) {
			if (READ_UINT32(_topCatBlinkEntry) < 3 || READ_UINT32(_topCatBlinkEntry) >= 6) {
				highlight = 1;
			} else {
				highlight = 0;
			}
		} else {
			highlight = 0;
		}

		highlightObject(_topCatBlinkEntry, highlight);
		_topCatBlinkEntry = 0;
		_topCatBlinkTimer = 0;
		highlightObject(_topCatBlinkEntry, 1);
	}

	int32 entryType = READ_UINT32(_topCatHoveredEntry);

	if (entryType == 0) {
		*result = 8;
		return 1;
	}

	if (entryType == 1) {
		if (prevState == 2)
			return 0;

		queueAnim(2, 0); // Play help audio
		return 0;
	}

	if (entryType == 2) {
		if (prevState == 3)
			return 0;

		queueAnim(3, 0); // Play question audio
		return 0;
	}

	if (entryType < 3 || entryType >= 6)
		return 0;

	// Answer choice clicked...
	int16 answerSlot = (int16)entryType - 3;

	// Display overlay pic on the answer slot...
	int16 selectedChoiceOverlayX = READ_UINT16(_topCatAnswersScreenPositions + answerSlot * 4);
	int16 selectedChoiceOverlayY = READ_UINT16(_topCatAnswersScreenPositions + answerSlot * 4 + 2);
	byte *selectedChoiceOverlayPic = getResolvedPtr(_topCatSelectedChoiceOverlayGfx, answerSlot * 4);
	displayPic(selectedChoiceOverlayPic, selectedChoiceOverlayX, selectedChoiceOverlayY, stBack);

	_xp->updateDisplay();

	// Start palette cycle on the answer slot overlay...
	_topCatChoiceCycleState[0].startIndex = _topCatCycleSpecs[answerSlot].startIndex;
	_topCatChoiceCycleState[0].endIndex = _topCatCycleSpecs[answerSlot].endIndex;
	_topCatChoiceCycleState[0].delay = _topCatCycleSpecs[answerSlot].delay;
	_topCatChoiceCycleState[0].nextFire = _topCatCycleSpecs[answerSlot].nextFire;
	_topCatChoiceCycleState[0].active = _topCatCycleSpecs[answerSlot].active;

	_xp->startCycle(_topCatChoiceCycleState);

	// Check answer...
	int8 correctAnswer = (int8)_topCatAnswers[_topCatCurrentAnswerIdx * 3];

	if (correctAnswer == answerSlot) {
		// Correct! :-)
		queueAnim(4, 0); // Correct answer animation
		queueAnim(6, 0); // Blink the lights

		_topCatShuffledQuestions[_topCatShuffledQuestionsArrayIdx] |= 0x80; // Answered!
		_topCatScore++;

		if (_topCatScore == 6) {
			queueAnim(6, 0); // Blink the lights
			*result = 0x10;
			return 1;
		}

		queueAnim(3, 1); // Transition to next question
	} else {
		// Wrong... :-(
		queueAnim(5, 0); // Wrong answer animation
		queueAnim(3, 1); // Transition to next question
	}

	return 0;
}

void BoltEngine::queueAnim(int16 animType, int16 param) {
	int16 slot = _topCatAnimQueueSize;
	_topCatAnimQueueSize++;

	TopCatAnim *entry = &_topCatAnimQueue[slot];

	if (_topCatAnimQueueSize > 3) {
		_topCatAnimQueueSize--;
		return;
	}

	entry->animType = animType;

	switch (animType) {
	case 2: // Help audio
		entry->animIndex = 0x1F;
		break;

	case 3: // Question audio
		entry->transitionToNextQuestionFlag = param;
		break;

	case 4: // Correct answer
		entry->animIndex = _topCatCorrectAnimIdx;
		_topCatCorrectAnimIdx++;
		_topCatCorrectAnimIdx %= 3;
		break;

	case 5: // Wrong answer
		entry->animIndex = _topCatWrongAnimIdx + 3;
		_topCatWrongAnimIdx++;
		_topCatWrongAnimIdx %= 3;
		break;

	case 6: // Score light
		if (_topCatScore < 6)
			entry->animIndex = _topCatScore + 1;
		else
			entry->animIndex = 0;

		// Store blink sequence pointer from table
		entry->seqPtr = _topCatBlinkSeqTable[_topCatScore];
		break;
	}
}

bool BoltEngine::maintainAnim(int16 soundEvent) {
	int16 animDone = 0;

	switch (_topCatAnimStateMachineStep) {
	case 0: // Waiting to start next anim
		if (_topCatAnimQueueSize == 0)
			break;

		if (!startNextAnim(&_topCatShouldPlayAnswerAnim))
			return false;

		_topCatAnimStateMachineStep = 1;
		break;
	case 1: // Filling RTF buffer
		if (fillRTFBuffer()) {
			if (fillRTFBuffer())
				break;
		}

		_topCatAnimStateMachineStep = 2;
		break;
	case 2: // Waiting for sound sync
		if (!soundEvent)
			break;

		if (_topCatMaxBackgroundAnimFrames - _topCatQueuedSoundFrames != _topCatBackgroundAnimFrame)
			break;

		flushRTFSoundQueue();
		_topCatAnimStateMachineStep = 3;
		break;
	case 3: // Waiting for AV end
	case 4: // Playing AV
		if (_topCatAnimStateMachineStep == 3) {
			if (soundEvent != 2)
				break;

			_topCatAnimStateMachineStep = 4;
			_topCatMaintainSoundFlag = 1;
		}

		int16 avResult;
		if (_topCatShouldPlayAnswerAnim != 0)
			avResult = maintainAV(soundEvent == 2 ? 1 : 0);
		else
			avResult = maintainAudioPlay(soundEvent == 2 ? 1 : 0);

		if (!avResult) { // AV finished
			if (_topCatShouldPlayAnswerAnim != 0) {
				// Display background anim frame...
				byte *animFrame = getResolvedPtr(_topCatGraphicsAssets, _topCatBackgroundAnimFrame * 4);
				displayPic(animFrame, _displayX, _displayY, stFront);
			}

			animDone = 1;
		} else {
			// Check if idle sound needs maintaining...
			if (_topCatMaintainSoundFlag != 0) {
				if (!isRTFPlaying()) {
					int16 savedFrame = _topCatBackgroundAnimFrame;
					_topCatBackgroundAnimFrame = 0;
					maintainIdleSound(0);
					_topCatBackgroundAnimFrame = savedFrame;
					_topCatMaintainSoundFlag = 0;
				}
			}
		}

		break;
	}

	// Advance audio and the background animation together...
	if (soundEvent == 1 || (soundEvent == 2 && _topCatShouldPlayAnswerAnim == 0)) {
		_topCatBackgroundAnimFrame++;
		if (_topCatBackgroundAnimFrame >= _topCatMaxBackgroundAnimFrames)
			_topCatBackgroundAnimFrame = 0;

		if (soundEvent == 1) {
			if (_topCatAnimStateMachineStep == 3 || _topCatAnimStateMachineStep == 4) {
				_topCatQueuedSoundFrames--;
			} else {
				maintainIdleSound(1);
			}
		}

		// If no pending sound event, display the next background anim frame...
		uint32 peekData;
		if (_xp->peekEvent(etSound, &peekData) != etSound) {
			byte *backgroundFrame = getResolvedPtr(_topCatGraphicsAssets, _topCatBackgroundAnimFrame * 4);
			displayPic(backgroundFrame, _displayX, _displayY, stFront);
			_xp->updateDisplay();
		}
	}

	if (animDone)
		setAnimType(1);

	return true;
}

void BoltEngine::maintainIdleSound(int16 decrement) {
	byte *soundData = _topCatSoundInfo.data;
	int16 chunkSize = (int16)(_topCatSoundInfo.size / _topCatMaxBackgroundAnimFrames);

	if (decrement)
		_topCatQueuedSoundFrames--;

	int16 idx = _topCatBackgroundAnimFrame + _topCatQueuedSoundFrames;

	while (_topCatQueuedSoundFrames < 10) {
		_topCatQueuedSoundFrames++;

		if (idx >= _topCatMaxBackgroundAnimFrames)
			idx -= _topCatMaxBackgroundAnimFrames;

		_xp->playSound(soundData + chunkSize * idx, chunkSize, 22050);
		idx++;
	}
}

bool BoltEngine::startNextAnim(int16 *playAnswerAnim) {
	TopCatAnim *entry = &_topCatAnimQueue[0];
	bool startResult = false;

	if (_topCatAnimQueueSize == 0)
		return false;

	// Setup next question, if requested...
	if (entry->animType == 3) {
		if (entry->transitionToNextQuestionFlag != 0) {
			if (!setupNextQuestion())
				return false;

			_xp->updateDisplay();
		}

		entry->animIndex = _topCatCurrentAnswerIdx * 60 + _topCatDrawnQuestionId + 7;
	}

	int16 animType = entry->animType;

	if (animType == 6) {
		// Score light animation
		_topCatBlinkSeqPtr = entry->seqPtr;

		*playAnswerAnim = 0;
		startResult = startAnimation(_topCatAvHandle, entry->animIndex);
	} else if (animType == 3) {
		// Question audio
		_topCatCycleStep = 3;

		*playAnswerAnim = 0;
		startResult = startAnimation(_topCatAvHandle, entry->animIndex);
	} else if (animType == 2) {
		// Help audio
		_topCatCycleStep = 0;

		*playAnswerAnim = 0;
		startResult = startAnimation(_rtfHandle, entry->animIndex);
	} else {
		// Correct/wrong answer animations
		*playAnswerAnim = 1;
		setAVBufferSize(0x4B000);
		startResult = prepareAV(_topCatRtfHandle, entry->animIndex,
								_displayWidth, _displayHeight,
								_displayX, _displayY);
	}

	if (startResult) {
		setAnimType(entry->animType);
	} else {
		_topCatCurrentAnimType = 1;
		_topCatAnimStateMachineStep = 0;
		maintainIdleSound(0);
	}

	// Shift queue: copy entries 1..n down to 0..n-1
	for (int16 i = 1; i < _topCatAnimQueueSize; i++) {
		_topCatAnimQueue[i - 1] = _topCatAnimQueue[i];
	}

	_topCatAnimQueueSize--;

	return true;
}

void BoltEngine::setAnimType(int16 newType) {
	if (_topCatCurrentAnimType == newType)
		return;

	// Clean-up the previous state...
	switch (_topCatCurrentAnimType) {
	case 2: // Was playing the help audio
		// Stop blinking entry
		int16 highlight;
		if (_topCatBlinkEntry == _topCatHoveredEntry &&
			_topCatBlinkEntry != 0) {
			if (READ_UINT32(_topCatBlinkEntry) < 3 || READ_UINT32(_topCatBlinkEntry) >= 6) {
				highlight = 1;
			} else {
				highlight = 0;
			}
		} else {
			highlight = 0;
		}

		highlightObject(_topCatBlinkEntry, highlight);

		_topCatBlinkEntry = 0;
		_topCatBlinkTimer = 0;
		highlightObject(_topCatBlinkEntry, 1);
		break;

	case 3: // Was playing the question audio
		if (_topCatCycleStep >= 3) {
			byte *prevEntry = getResolvedPtr(_topCatButtonsPalette, (_topCatCycleStep - 1) * 4);
			highlightObject(prevEntry, 0);
		}

		highlightObject(_topCatBackButton, 0);
		break;

	case 4: // Correct answer anim
		break;

	case 5: // Wrong answer anim
	case 6: // Score lights
		if (_topCatScore != 6)
			_xp->showCursor();

		break;

	default:
		break;
	}

	// Setup the next state
	switch (newType) {
	case 1: // Idle
		maintainIdleSound(0);
		break;

	case 2: // Help audio
		break;

	case 3: // Question audio
		highlightObject(_topCatBackButton, 1);
		break;

	case 4: // Correct answer anim
	case 5: // Wrong answer anim
		_xp->hideCursor();
		break;

	default:
		break;
	}

	_topCatCurrentAnimType = newType;
	_topCatAnimStateMachineStep = 0;
}

void BoltEngine::highlightObject(byte *entry, int16 highlight) {
	if (!entry)
		return;

	if (highlight) {
		_xp->setPalette(READ_UINT16(entry + 0x10), READ_UINT16(entry + 0x0E), entry + 0x12);
		WRITE_UINT16(entry + 0x0C, READ_UINT16(entry + 0x0C) | 1);
	} else {
		_xp->setPalette(READ_UINT16(entry + 0x10), READ_UINT16(entry + 0x0E), entry + 0x2E);
		WRITE_UINT16(entry + 0x0C, READ_UINT16(entry + 0x0C) & ~1);
	}

	_system->updateScreen();
}

bool BoltEngine::setupNextQuestion() {
	_xp->stopCycle();

	int16 attempts = 0;
	while (attempts < 60) {
		_topCatShuffledQuestionsArrayIdx++;
		if (_topCatShuffledQuestionsArrayIdx >= 60)
			_topCatShuffledQuestionsArrayIdx = 0;

		_topCatDrawnQuestionId = (int8)_topCatShuffledQuestions[_topCatShuffledQuestionsArrayIdx];

		if ((_topCatDrawnQuestionId & 0x80) == 0)
			break; // Found a question which hasn't been answered yet...

		attempts++;
	}

	if (attempts >= 60) {
		_topCatShuffledQuestionsArrayIdx = -1;
		shuffleTopCatQuestions();
		shuffleTopCatPermutations();

		for (int16 i = 0; i < 60; i++) {
			_topCatShuffledQuestions[i] &= 0x7F; // Remove "already answered" flag from all questions...
		}

		return setupNextQuestion();
	}

	_topCatShuffledAnswers[_topCatDrawnQuestionId]++;
	if (_topCatShuffledAnswers[_topCatDrawnQuestionId] >= 3)
		_topCatShuffledAnswers[_topCatDrawnQuestionId] = 0;

	int16 qIdx = _topCatDrawnQuestionId;
	int16 permutation = (int8)_topCatShuffledAnswers[_topCatDrawnQuestionId];
	_topCatCurrentAnswerIdx = (int8)_topCatAnswersPermutations[qIdx * 3 + permutation];

	int16 groupId = _topCatDrawnQuestionId * 0x100 + 0x100;
	if (!getBOLTGroup(_topCatBoltLib, groupId, 1))
		return false;

	byte *questionsAnswersGfxTable = memberAddr(_topCatBoltLib, _topCatDrawnQuestionId * 0x100 + 0x104);

	// Display question pic...
	byte *questionPic = getResolvedPtr(questionsAnswersGfxTable, 0);
	displayPic(questionPic, 50, 38, stBack);

	// Display the three answer choices...
	int16 tableOff = 0;
	for (int16 i = 0; i < 3; i++) {
		int8 answerIdx = (int8)_topCatAnswers[_topCatCurrentAnswerIdx * 3 + i];

		int16 x = READ_UINT16(_topCatAnswersScreenPositions + answerIdx * 4);
		int16 y = READ_UINT16(_topCatAnswersScreenPositions + answerIdx * 4 + 2);

		int16 colorShift = (answerIdx - i) * 3;
		byte *answerPic = getResolvedPtr(questionsAnswersGfxTable, tableOff + 4);
		adjustColors(answerPic, colorShift);
		displayPic(answerPic, x, y, stBack);

		tableOff += 4;
	}

	freeBOLTGroup(_topCatBoltLib, groupId, 1);

	return true;
}

void BoltEngine::adjustColors(byte *pic, int8 shift) {
	int16 count = READ_UINT16(pic + 0x0C);
	byte *data = getResolvedPtr(pic, 0x12);

	while (count) {
		byte val = *data;

		if (val & 0x7F) {
			*data += shift;
		}

		data++;

		if (val & 0x80) {
			if (*data == 0)
				count--;
			data++;
		}
	}
}

void BoltEngine::shuffleTopCatQuestions() {
	int16 answerOff = 0;

	for (int16 i = 0; i < 60; i++) {
		int16 randIdx = _xp->getRandom(60);

		byte tmp = _topCatShuffledQuestions[i];
		_topCatShuffledQuestions[i] = _topCatShuffledQuestions[randIdx];
		_topCatShuffledQuestions[randIdx] = tmp;

		for (int16 j = 0; j < 3; j++) {
			int16 randJ = _xp->getRandom(3);

			byte aTmp = _topCatAnswersPermutations[answerOff + j];
			_topCatAnswersPermutations[answerOff + j] = _topCatAnswersPermutations[answerOff + randJ];
			_topCatAnswersPermutations[answerOff + randJ] = aTmp;
		}

		answerOff += 3;
	}
}

void BoltEngine::shuffleTopCatPermutations() {
	// Shuffle first permutation table...
	for (int16 i = 0; i < 3; i++) {
		int16 randIdx = _xp->getRandom(3);

		int16 tmp = _topCatPermTableA[i];
		_topCatPermTableA[i] = _topCatPermTableA[randIdx];
		_topCatPermTableA[randIdx] = tmp;
	}

	// Shuffle second permutation table...
	for (int16 i = 0; i < 3; i++) {
		int16 randIdx = _xp->getRandom(3);

		int16 tmp = _topCatPermTableB[i];
		_topCatPermTableB[i] = _topCatPermTableB[randIdx];
		_topCatPermTableB[randIdx] = tmp;
	}
}

void BoltEngine::getTopCatSoundInfo(BOLTLib *lib, int16 memberId, SoundInfo *soundInfo) {
	soundInfo->data = memberAddr(lib, memberId);
	soundInfo->size = memberSize(lib, memberId);
}

void BoltEngine::setScoreLights(int16 litMask) {
	int16 lightIdx = 0;
	int16 tableOff = 0;

	while (tableOff < 0x18) {
		byte *lightEntry = getResolvedPtr(_topCatLightsPalette, tableOff);

		int16 palStart = READ_UINT16(lightEntry) + 0x80;
		int16 colorCount = READ_UINT16(lightEntry + 4);

		int16 palDataIdx;
		if (litMask & (1 << lightIdx)) {
			palDataIdx = 0;
		} else {
			palDataIdx = colorCount;
		}

		byte *palData = lightEntry + (palDataIdx + 2) * 4;
		_xp->setPalette(colorCount, palStart, palData);

		tableOff += 4;
		lightIdx++;
	}

	_system->updateScreen();
}

void BoltEngine::swapTopCatHelpEntry() {
	byte *data = _boltCurrentMemberEntry->dataPtr;
	uint32 decompSize = _boltCurrentMemberEntry->decompSize;
	int16 offset = 0;

	while (offset < (int32)decompSize) {
		WRITE_UINT32(data + offset + 0x00, READ_BE_UINT32(data + offset + 0x00));
		WRITE_UINT16(data + offset + 0x04, READ_BE_UINT16(data + offset + 0x04));
		WRITE_UINT16(data + offset + 0x06, READ_BE_UINT16(data + offset + 0x06));
		WRITE_UINT16(data + offset + 0x08, READ_BE_UINT16(data + offset + 0x08));
		WRITE_UINT16(data + offset + 0x0A, READ_BE_UINT16(data + offset + 0x0A));
		WRITE_UINT16(data + offset + 0x0C, READ_BE_UINT16(data + offset + 0x0C));
		WRITE_UINT16(data + offset + 0x0E, READ_BE_UINT16(data + offset + 0x0E));
		WRITE_UINT16(data + offset + 0x10, READ_BE_UINT16(data + offset + 0x10));

		offset += 0x4A;
	}
}

} // End of namespace Bolt
