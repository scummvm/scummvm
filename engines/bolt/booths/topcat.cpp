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
	g_topCatRtfHandle = openRTF(assetPath(g_displayMode != 0 ? "topcatcr.av" : "topcatog.av"));
	if (!g_topCatRtfHandle)
		return false;

	g_topCatAvHandle = openRTF(assetPath("topcatau.av"));
	if (!g_topCatAvHandle)
		return false;

	if (!openBOLTLib(&g_topCatBoltLib, &g_topCatBoltCallbacks, assetPath("topcat.blt")))
		return false;

	if (!getBOLTGroup(g_topCatBoltLib, 0, 1))
		return false;

	g_topCatBackgroundPalette = memberAddr(g_topCatBoltLib, 0x20);
	g_topCatBackground = memberAddr(g_topCatBoltLib, g_displayMode != 0 ? 0x22 : 0x21);
	g_topCatBackgroundAnimationPalette = memberAddr(g_topCatBoltLib, 0x00);
	g_topCatGraphicsAssets = memberAddr(g_topCatBoltLib, g_displayMode != 0 ? 0x1E : 0x1D);

	g_topCatBackgroundAnimFrame = 0;

	// Count background anim frames...
	g_topCatMaxBackgroundAnimFrames = 0;
	while (getResolvedPtr(g_topCatGraphicsAssets, g_topCatMaxBackgroundAnimFrames * 4) != 0) {
		g_topCatMaxBackgroundAnimFrames++;
	}

	g_topCatCurrentAnimType = 0;
	g_topCatAnimStateMachineStep = 0;
	g_topCatQueuedSoundFrames = 0;
	g_topCatAnimQueueSize = 0;

	g_topCatButtonsPalette = memberAddr(g_topCatBoltLib, 0x3D);
	g_topCatBlinkEntries = memberAddr(g_topCatBoltLib, 0x3E);

	g_topCatLightsPalette = memberAddr(g_topCatBoltLib, 0x31);
	g_topCatBlinkSeqPtr = nullptr;

	g_topCatSelectedChoiceOverlayGfx = memberAddr(g_topCatBoltLib, 0x26);
	g_topCatCycleData = memberAddr(g_topCatBoltLib, 0x27);
	boltCycleToXPCycle(g_topCatCycleData, g_topCatCycleSpecs);

	g_topCatChoiceCycleState->startIndex = 0;
	g_topCatChoiceCycleState->endIndex = 0;
	g_topCatChoiceCycleState->delay = 0;
	g_topCatChoiceCycleState->nextFire = 0;
	g_topCatChoiceCycleState->active = false;

	g_topCatShuffledQuestions = memberAddr(g_topCatBoltLib, 0x32); // [60]
	g_topCatShuffledAnswers = memberAddr(g_topCatBoltLib, 0x33); // [60]
	g_topCatAnswersPermutations = memberAddr(g_topCatBoltLib, 0x34); // [60 * 3]
	g_topCatAnswers = memberAddr(g_topCatBoltLib, 0x35);
	g_topCatAnswersScreenPositions = memberAddr(g_topCatBoltLib, 0x36);

	if (vLoad(&g_topCatSavedScore, g_topCatSaveFileName)) {
		g_topCatScore = g_topCatSavedScore;
	} else {
		g_topCatScore = 0;
	}

	if (vLoad(g_topCatSaveBuffer, g_topCatStaticSaveFileName)) {
		g_topCatShuffledQuestionsArrayIdx = READ_BE_INT16(g_topCatSaveBuffer);

		int offset;
		for (int i = 0; i < ARRAYSIZE(g_topCatSavedShuffledQuestions); i++) {
			offset = sizeof(int16) + i;
			g_topCatSavedShuffledQuestions[i] = g_topCatSaveBuffer[offset];
		}

		for (int i = 0; i < ARRAYSIZE(g_topCatSavedShuffledAnswers); i++) {
			offset = sizeof(g_topCatSavedShuffledQuestions) + sizeof(int16) + i;
			g_topCatSavedShuffledAnswers[i] = g_topCatSaveBuffer[offset];
		}

		for (int i = 0; i < ARRAYSIZE(g_topCatSavedAnswersPermutations); i++) {
			offset = sizeof(g_topCatSavedShuffledAnswers) + sizeof(g_topCatSavedShuffledQuestions) + sizeof(int16) + i;
			g_topCatSavedAnswersPermutations[i] = g_topCatSaveBuffer[offset];
		}

		int16 answerOff = 0;
		for (int16 i = 0; i < 60; i++) {
			g_topCatShuffledQuestions[i] = g_topCatSavedShuffledQuestions[i];
			g_topCatShuffledAnswers[i] = g_topCatSavedShuffledAnswers[i];

			for (int16 j = 0; j < 3; j++) {
				g_topCatAnswersPermutations[answerOff + j] = g_topCatSavedAnswersPermutations[answerOff + j];
			}

			answerOff += 3;
		}
	} else {
		g_topCatShuffledQuestionsArrayIdx = -1;
		shuffleTopCatQuestions();
		shuffleTopCatPermutations();
	}

	g_topCatBlinkTimer = 0;

	getTopCatSoundInfo(g_topCatBoltLib, 0x1F, &g_topCatSoundInfo);

	uint32 dummy;
	while (_xp->getEvent(etTimer, &dummy) != etTimer);

	_xp->stopCycle();
	_xp->setTransparency(false);

	displayColors(g_topCatBackgroundPalette, stFront, 0);
	displayPic(g_topCatBackground, g_displayX, g_displayY, stFront);
	_xp->updateDisplay();

	_xp->setTransparency(true);
	displayColors(g_topCatBackgroundAnimationPalette, stFront, 0);
	displayPic(g_topCatBackground, g_displayX, g_displayY, stBack);

	byte *firstBackgroundAnimFrame = getResolvedPtr(g_topCatGraphicsAssets, 0);
	displayPic(firstBackgroundAnimFrame, g_displayX, g_displayY, stFront);

	displayColors(g_topCatBackgroundPalette, stBack, 0);

	if (!setupNextQuestion())
		return false;

	_xp->updateDisplay();

	g_topCatHoveredEntry = nullptr;

	// Init button palettes...
	int16 off = 0;
	byte *entry;
	while (true) {
		entry = getResolvedPtr(g_topCatButtonsPalette, off);
		if (!entry)
			break;

		if (READ_UINT32(entry) == 1)
			g_topCatHelpButton = entry;

		if (READ_UINT32(entry) == 2)
			g_topCatBackButton = entry;

		_xp->getPalette(READ_UINT16(entry + 0x0E), READ_UINT16(entry + 0x10), entry + 0x2E);
		unpackColors(READ_UINT16(entry + 0x10), entry + 0x12);

		off += 4;
	}

	// Init light palelttes...
	int16 lightOff = 0;
	for (int16 i = 0; i < 6; i++) {
		byte *lightEntry = getResolvedPtr(g_topCatLightsPalette, lightOff);

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
	int16 litMask = (1 << g_topCatScore) - 1;
	setScoreLights(litMask);

	return true;
}

void BoltEngine::cleanUpTopCat() {
	_xp->stopSound();
	_xp->stopCycle();

	if (g_topCatScore < 6) {
		byte questionIdx = g_topCatShuffledQuestions[g_topCatShuffledQuestionsArrayIdx];
		g_topCatShuffledAnswers[questionIdx]--;
		g_topCatShuffledQuestionsArrayIdx--;
	} else {
		g_topCatScore = 0;
	}

	g_topCatSavedScore = g_topCatScore;
	g_topCatSaveHistory = g_topCatShuffledQuestionsArrayIdx;

	int16 answerOff = 0;
	for (int16 i = 0; i < 60; i++) {
		g_topCatSavedShuffledQuestions[i] = g_topCatShuffledQuestions[i];
		g_topCatSavedShuffledAnswers[i] = g_topCatShuffledAnswers[i];

		for (int16 j = 0; j < 3; j++) {
			g_topCatSavedAnswersPermutations[answerOff + j] = g_topCatAnswersPermutations[answerOff + j];
		}

		answerOff += 3;
	}

	memset(g_topCatSaveBuffer, 0, sizeof(g_topCatSaveBuffer));

	WRITE_BE_INT16(g_topCatSaveBuffer, g_topCatSaveHistory);

	int offset;
	for (int i = 0; i < ARRAYSIZE(g_topCatSavedShuffledQuestions); i++) {
		offset = sizeof(int16) + i;
		g_topCatSaveBuffer[offset] = g_topCatSavedShuffledQuestions[i];
	}

	for (int i = 0; i < ARRAYSIZE(g_topCatSavedShuffledAnswers); i++) {
		offset = sizeof(g_topCatSavedShuffledQuestions) + sizeof(int16) + i;
		g_topCatSaveBuffer[offset] = g_topCatSavedShuffledAnswers[i];
	}

	for (int i = 0; i < ARRAYSIZE(g_topCatSavedAnswersPermutations); i++) {
		offset = sizeof(g_topCatSavedShuffledAnswers) + sizeof(g_topCatSavedShuffledQuestions) + sizeof(int16) + i;
		g_topCatSaveBuffer[offset] = g_topCatSavedAnswersPermutations[i];
	}

	vSave(&g_topCatSavedScore, sizeof(g_topCatSavedScore), g_topCatSaveFileName);
	vSave(g_topCatSaveBuffer, sizeof(g_topCatSaveBuffer), g_topCatStaticSaveFileName);

	freeBOLTGroup(g_topCatBoltLib, 0, 1);
	closeBOLTLib(&g_topCatBoltLib);

	if (g_topCatRtfHandle) {
		closeRTF(g_topCatRtfHandle);
		g_topCatRtfHandle = nullptr;
	}

	if (g_topCatAvHandle) {
		closeRTF(g_topCatAvHandle);
		g_topCatAvHandle = nullptr;
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
			if (eventData != g_topCatBlinkTimer)
				break;

			if (!g_topCatBlinkEntry)
				break;

			g_topCatBlinkTimer = _xp->startTimer(500);

			// Toggle highlight...
			int16 highlight;
			if (READ_UINT16(g_topCatBlinkEntry + 0x0C) & 1) {
				highlight = 0;
			} else {
				highlight = 1;
			}

			highlightObject(g_topCatBlinkEntry, highlight);
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
				entry = getResolvedPtr(g_topCatButtonsPalette, off);
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
			if (g_topCatHoveredEntry != entry &&
				g_topCatHoveredEntry != nullptr &&
				g_topCatHoveredEntry != g_topCatBlinkEntry &&
				g_topCatHoveredEntry != g_topCatBackButton) {

				if (g_topCatHoveredEntry == g_topCatHelpButton) {
					if (g_topCatCurrentAnimType != 2)
						highlightObject(g_topCatHoveredEntry, 0);
				} else {
					byte *hovered = g_topCatHoveredEntry;
					if (READ_UINT32(hovered) < 3 || READ_UINT32(hovered) >= 6)
						highlightObject(g_topCatHoveredEntry, 0);
				}
			}

			g_topCatHoveredEntry = entry;

			// Highlight new entry if applicable...
			if (!entry)
				break;

			if (entry == g_topCatBlinkEntry)
				break;

			if (entry == g_topCatBackButton)
				break;

			if (READ_UINT32(entry) >= 3 && READ_UINT32(entry) < 6)
				break;

			highlightObject(g_topCatHoveredEntry, 1);
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
			if (soundDataPtr >= g_topCatSoundInfo.data && soundDataPtr < g_topCatSoundInfo.data + g_topCatSoundInfo.size) {
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
			if (g_topCatBlinkEntry == g_topCatHoveredEntry &&
				g_topCatBlinkEntry != 0) {
				if (READ_UINT32(g_topCatBlinkEntry) < 3 || READ_UINT32(g_topCatBlinkEntry) >= 6) {
					highlight = 1;
				} else {
					highlight = 0;
				}
			} else {
				highlight = 0;
			}

			highlightObject(g_topCatBlinkEntry, highlight);

			// Set help button as new blinking entry...
			g_topCatBlinkEntry = g_topCatHelpButton;

			if (g_topCatHelpButton != 0) {
				if (READ_UINT32(g_topCatBlinkEntry) < 3 || READ_UINT32(g_topCatBlinkEntry) >= 6) {
					g_topCatBlinkTimer = _xp->startTimer(500);
				} else {
					g_topCatBlinkTimer = 0;
				}
			} else {
				g_topCatBlinkTimer = 0;
			}

			highlightObject(g_topCatBlinkEntry, 1);
			break;
		}

		case etTrigger: { // Palette cycle event
			if (g_topCatCurrentAnimType == 2) {
				// Toggle blinking highlight...
				int16 highlight;
				if (g_topCatBlinkEntry == g_topCatHoveredEntry &&
					g_topCatBlinkEntry != 0) {
					if (READ_UINT32(g_topCatBlinkEntry) < 3 || READ_UINT32(g_topCatBlinkEntry) >= 6) {
						highlight = 1;
					} else {
						highlight = 0;
					}
				} else {
					highlight = 0;
				}

				highlightObject(g_topCatBlinkEntry, highlight);

				// Advance to next blinking entry...
				if (g_topCatCycleStep & 1) {
					g_topCatBlinkEntry = nullptr;
				} else {
					g_topCatBlinkEntry = getResolvedPtr(g_topCatBlinkEntries, (g_topCatCycleStep >> 1) * 4);
				}

				if (g_topCatBlinkEntry != 0) {
					if (READ_UINT32(g_topCatBlinkEntry) < 3 || READ_UINT32(g_topCatBlinkEntry) >= 6) {
						g_topCatBlinkTimer = _xp->startTimer(500);
					} else {
						g_topCatBlinkTimer = 0;
					}
				} else {
					g_topCatBlinkTimer = 0;
				}

				highlightObject(g_topCatBlinkEntry, 1);
				g_topCatCycleStep++;

			} else if (g_topCatCurrentAnimType == 3) {
				// Unhighlight previous...
				if (g_topCatCycleStep > 3) {
					byte *prevEntry = getResolvedPtr(g_topCatButtonsPalette, (g_topCatCycleStep - 1) * 4);
					highlightObject(prevEntry, 0);
				} else {
					highlightObject(g_topCatBackButton, 0);
				}

				// Highlight next...
				byte *nextEntry = getResolvedPtr(g_topCatButtonsPalette, g_topCatCycleStep * 4);
				highlightObject(nextEntry, 1);
				g_topCatCycleStep++;
			} else if (g_topCatCurrentAnimType == 6) {
				// Light blinking animation...
				int16 mask = READ_UINT16(g_topCatBlinkSeqPtr);
				g_topCatBlinkSeqPtr++;

				setScoreLights(mask);
			}

			break;
		}

		default:
			break;
		}

		if (exitFlag && g_topCatAnimQueueSize == 0 && g_topCatCurrentAnimType == 1) {
			if (g_topCatScore != 6)
				_xp->hideCursor();

			return result;
		}
	}
}

int16 BoltEngine::handleActionButton(int16 *result) {
	int16 prevState = g_topCatCurrentAnimType;

	switch (g_topCatCurrentAnimType) {
	case 2:
	case 3:
		stopAnimation();
		_xp->stopSound();
		g_topCatQueuedSoundFrames = 0;
		setAnimType(1);
		break;

	case 4:
	case 5:
	case 6:
		return 0;
	default:
		break;
	}

	if (g_topCatAnimQueueSize != 0 && g_topCatHoveredEntry != 0) {
		if (READ_UINT32(g_topCatHoveredEntry) != 0)
			return 0;
	}

	if (!g_topCatHoveredEntry)
		return 0;

	_xp->setInactivityTimer(30);

	// If blinking entry is the help button, stop blinking...
	if (g_topCatBlinkEntry == g_topCatHelpButton) {
		int16 highlight;
		if (g_topCatBlinkEntry == g_topCatHoveredEntry &&
			g_topCatBlinkEntry != 0) {
			if (READ_UINT32(g_topCatBlinkEntry) < 3 || READ_UINT32(g_topCatBlinkEntry) >= 6) {
				highlight = 1;
			} else {
				highlight = 0;
			}
		} else {
			highlight = 0;
		}

		highlightObject(g_topCatBlinkEntry, highlight);
		g_topCatBlinkEntry = 0;
		g_topCatBlinkTimer = 0;
		highlightObject(g_topCatBlinkEntry, 1);
	}

	int32 entryType = READ_UINT32(g_topCatHoveredEntry);

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
	int16 selectedChoiceOverlayX = READ_UINT16(g_topCatAnswersScreenPositions + answerSlot * 4);
	int16 selectedChoiceOverlayY = READ_UINT16(g_topCatAnswersScreenPositions + answerSlot * 4 + 2);
	byte *selectedChoiceOverlayPic = getResolvedPtr(g_topCatSelectedChoiceOverlayGfx, answerSlot * 4);
	displayPic(selectedChoiceOverlayPic, selectedChoiceOverlayX, selectedChoiceOverlayY, stBack);

	_xp->updateDisplay();

	// Start palette cycle on the answer slot overlay...
	g_topCatChoiceCycleState[0].startIndex = g_topCatCycleSpecs[answerSlot].startIndex;
	g_topCatChoiceCycleState[0].endIndex = g_topCatCycleSpecs[answerSlot].endIndex;
	g_topCatChoiceCycleState[0].delay = g_topCatCycleSpecs[answerSlot].delay;
	g_topCatChoiceCycleState[0].nextFire = g_topCatCycleSpecs[answerSlot].nextFire;
	g_topCatChoiceCycleState[0].active = g_topCatCycleSpecs[answerSlot].active;

	_xp->startCycle(g_topCatChoiceCycleState);

	// Check answer...
	int8 correctAnswer = (int8)g_topCatAnswers[g_topCatCurrentAnswerIdx * 3];

	if (correctAnswer == answerSlot) {
		// Correct! :-)
		queueAnim(4, 0); // Correct answer animation
		queueAnim(6, 0); // Blink the lights

		g_topCatShuffledQuestions[g_topCatShuffledQuestionsArrayIdx] |= 0x80; // Answered!
		g_topCatScore++;

		if (g_topCatScore == 6) {
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
	int16 slot = g_topCatAnimQueueSize;
	g_topCatAnimQueueSize++;

	TopCatAnim *entry = &g_topCatAnimQueue[slot];

	if (g_topCatAnimQueueSize > 3) {
		g_topCatAnimQueueSize--;
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
		entry->animIndex = g_topCatCorrectAnimIdx;
		g_topCatCorrectAnimIdx++;
		g_topCatCorrectAnimIdx %= 3;
		break;

	case 5: // Wrong answer
		entry->animIndex = g_topCatWrongAnimIdx + 3;
		g_topCatWrongAnimIdx++;
		g_topCatWrongAnimIdx %= 3;
		break;

	case 6: // Score light
		if (g_topCatScore < 6)
			entry->animIndex = g_topCatScore + 1;
		else
			entry->animIndex = 0;

		// Store blink sequence pointer from table
		entry->seqPtr = g_topCatBlinkSeqTable[g_topCatScore];
		break;
	}
}

bool BoltEngine::maintainAnim(int16 soundEvent) {
	int16 animDone = 0;

	switch (g_topCatAnimStateMachineStep) {
	case 0: // Waiting to start next anim
		if (g_topCatAnimQueueSize == 0)
			break;

		if (!startNextAnim(&g_topCatShouldPlayAnswerAnim))
			return false;

		g_topCatAnimStateMachineStep = 1;
		break;
	case 1: // Filling RTF buffer
		if (fillRTFBuffer()) {
			if (fillRTFBuffer())
				break;
		}

		g_topCatAnimStateMachineStep = 2;
		break;
	case 2: // Waiting for sound sync
		if (!soundEvent)
			break;

		if (g_topCatMaxBackgroundAnimFrames - g_topCatQueuedSoundFrames != g_topCatBackgroundAnimFrame)
			break;

		flushRTFSoundQueue();
		g_topCatAnimStateMachineStep = 3;
		break;
	case 3: // Waiting for AV end
	case 4: // Playing AV
		if (g_topCatAnimStateMachineStep == 3) {
			if (soundEvent != 2)
				break;

			g_topCatAnimStateMachineStep = 4;
			g_topCatMaintainSoundFlag = 1;
		}

		int16 avResult;
		if (g_topCatShouldPlayAnswerAnim != 0)
			avResult = maintainAV(soundEvent == 2 ? 1 : 0);
		else
			avResult = maintainAudioPlay(soundEvent == 2 ? 1 : 0);

		if (!avResult) { // AV finished
			if (g_topCatShouldPlayAnswerAnim != 0) {
				// Display background anim frame...
				byte *animFrame = getResolvedPtr(g_topCatGraphicsAssets, g_topCatBackgroundAnimFrame * 4);
				displayPic(animFrame, g_displayX, g_displayY, stFront);
			}

			animDone = 1;
		} else {
			// Check if idle sound needs maintaining...
			if (g_topCatMaintainSoundFlag != 0) {
				if (!isRTFPlaying()) {
					int16 savedFrame = g_topCatBackgroundAnimFrame;
					g_topCatBackgroundAnimFrame = 0;
					maintainIdleSound(0);
					g_topCatBackgroundAnimFrame = savedFrame;
					g_topCatMaintainSoundFlag = 0;
				}
			}
		}

		break;
	}

	// Advance audio and the background animation together...
	if (soundEvent == 1 || (soundEvent == 2 && g_topCatShouldPlayAnswerAnim == 0)) {
		g_topCatBackgroundAnimFrame++;
		if (g_topCatBackgroundAnimFrame >= g_topCatMaxBackgroundAnimFrames)
			g_topCatBackgroundAnimFrame = 0;

		if (soundEvent == 1) {
			if (g_topCatAnimStateMachineStep == 3 || g_topCatAnimStateMachineStep == 4) {
				g_topCatQueuedSoundFrames--;
			} else {
				maintainIdleSound(1);
			}
		}

		// If no pending sound event, display the next background anim frame...
		uint32 peekData;
		if (_xp->peekEvent(etSound, &peekData) != etSound) {
			byte *backgroundFrame = getResolvedPtr(g_topCatGraphicsAssets, g_topCatBackgroundAnimFrame * 4);
			displayPic(backgroundFrame, g_displayX, g_displayY, stFront);
			_xp->updateDisplay();
		}
	}

	if (animDone)
		setAnimType(1);

	return true;
}

void BoltEngine::maintainIdleSound(int16 decrement) {
	byte *soundData = g_topCatSoundInfo.data;
	int16 chunkSize = (int16)(g_topCatSoundInfo.size / g_topCatMaxBackgroundAnimFrames);

	if (decrement)
		g_topCatQueuedSoundFrames--;

	int16 idx = g_topCatBackgroundAnimFrame + g_topCatQueuedSoundFrames;

	while (g_topCatQueuedSoundFrames < 10) {
		g_topCatQueuedSoundFrames++;

		if (idx >= g_topCatMaxBackgroundAnimFrames)
			idx -= g_topCatMaxBackgroundAnimFrames;

		_xp->playSound(soundData + chunkSize * idx, chunkSize, 22050);
		idx++;
	}
}

bool BoltEngine::startNextAnim(int16 *playAnswerAnim) {
	TopCatAnim *entry = &g_topCatAnimQueue[0];
	bool startResult = false;

	if (g_topCatAnimQueueSize == 0)
		return false;

	// Setup next question, if requested...
	if (entry->animType == 3) {
		if (entry->transitionToNextQuestionFlag != 0) {
			if (!setupNextQuestion())
				return false;

			_xp->updateDisplay();
		}

		entry->animIndex = g_topCatCurrentAnswerIdx * 60 + g_topCatDrawnQuestionId + 7;
	}

	int16 animType = entry->animType;

	if (animType == 6) {
		// Score light animation
		g_topCatBlinkSeqPtr = entry->seqPtr;

		*playAnswerAnim = 0;
		startResult = startAnimation(g_topCatAvHandle, entry->animIndex);
	} else if (animType == 3) {
		// Question audio
		g_topCatCycleStep = 3;

		*playAnswerAnim = 0;
		startResult = startAnimation(g_topCatAvHandle, entry->animIndex);
	} else if (animType == 2) {
		// Help audio
		g_topCatCycleStep = 0;

		*playAnswerAnim = 0;
		startResult = startAnimation(g_rtfHandle, entry->animIndex);
	} else {
		// Correct/wrong answer animations
		*playAnswerAnim = 1;
		setAVBufferSize(0x4B000);
		startResult = prepareAV(g_topCatRtfHandle, entry->animIndex,
								g_displayWidth, g_displayHeight,
								g_displayX, g_displayY);
	}

	if (startResult) {
		setAnimType(entry->animType);
	} else {
		g_topCatCurrentAnimType = 1;
		g_topCatAnimStateMachineStep = 0;
		maintainIdleSound(0);
	}

	// Shift queue: copy entries 1..n down to 0..n-1
	for (int16 i = 1; i < g_topCatAnimQueueSize; i++) {
		g_topCatAnimQueue[i - 1] = g_topCatAnimQueue[i];
	}

	g_topCatAnimQueueSize--;

	return true;
}

void BoltEngine::setAnimType(int16 newType) {
	if (g_topCatCurrentAnimType == newType)
		return;

	// Clean-up the previous state...
	switch (g_topCatCurrentAnimType) {
	case 2: // Was playing the help audio
		// Stop blinking entry
		int16 highlight;
		if (g_topCatBlinkEntry == g_topCatHoveredEntry &&
			g_topCatBlinkEntry != 0) {
			if (READ_UINT32(g_topCatBlinkEntry) < 3 || READ_UINT32(g_topCatBlinkEntry) >= 6) {
				highlight = 1;
			} else {
				highlight = 0;
			}
		} else {
			highlight = 0;
		}

		highlightObject(g_topCatBlinkEntry, highlight);

		g_topCatBlinkEntry = 0;
		g_topCatBlinkTimer = 0;
		highlightObject(g_topCatBlinkEntry, 1);
		break;

	case 3: // Was playing the question audio
		if (g_topCatCycleStep >= 3) {
			byte *prevEntry = getResolvedPtr(g_topCatButtonsPalette, (g_topCatCycleStep - 1) * 4);
			highlightObject(prevEntry, 0);
		}

		highlightObject(g_topCatBackButton, 0);
		break;

	case 4: // Correct answer anim
		break;

	case 5: // Wrong answer anim
	case 6: // Score lights
		if (g_topCatScore != 6)
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
		highlightObject(g_topCatBackButton, 1);
		break;

	case 4: // Correct answer anim
	case 5: // Wrong answer anim
		_xp->hideCursor();
		break;

	default:
		break;
	}

	g_topCatCurrentAnimType = newType;
	g_topCatAnimStateMachineStep = 0;
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
		g_topCatShuffledQuestionsArrayIdx++;
		if (g_topCatShuffledQuestionsArrayIdx >= 60)
			g_topCatShuffledQuestionsArrayIdx = 0;

		g_topCatDrawnQuestionId = (int8)g_topCatShuffledQuestions[g_topCatShuffledQuestionsArrayIdx];

		if ((g_topCatDrawnQuestionId & 0x80) == 0)
			break; // Found a question which hasn't been answered yet...

		attempts++;
	}

	if (attempts >= 60) {
		g_topCatShuffledQuestionsArrayIdx = -1;
		shuffleTopCatQuestions();
		shuffleTopCatPermutations();

		for (int16 i = 0; i < 60; i++) {
			g_topCatShuffledQuestions[i] &= 0x7F; // Remove "already answered" flag from all questions...
		}

		return setupNextQuestion();
	}

	g_topCatShuffledAnswers[g_topCatDrawnQuestionId]++;
	if (g_topCatShuffledAnswers[g_topCatDrawnQuestionId] >= 3)
		g_topCatShuffledAnswers[g_topCatDrawnQuestionId] = 0;

	int16 qIdx = g_topCatDrawnQuestionId;
	int16 permutation = (int8)g_topCatShuffledAnswers[g_topCatDrawnQuestionId];
	g_topCatCurrentAnswerIdx = (int8)g_topCatAnswersPermutations[qIdx * 3 + permutation];

	int16 groupId = g_topCatDrawnQuestionId * 0x100 + 0x100;
	if (!getBOLTGroup(g_topCatBoltLib, groupId, 1))
		return false;

	byte *questionsAnswersGfxTable = memberAddr(g_topCatBoltLib, g_topCatDrawnQuestionId * 0x100 + 0x104);

	// Display question pic...
	byte *questionPic = getResolvedPtr(questionsAnswersGfxTable, 0);
	displayPic(questionPic, 50, 38, stBack);

	// Display the three answer choices...
	int16 tableOff = 0;
	for (int16 i = 0; i < 3; i++) {
		int8 answerIdx = (int8)g_topCatAnswers[g_topCatCurrentAnswerIdx * 3 + i];

		int16 x = READ_UINT16(g_topCatAnswersScreenPositions + answerIdx * 4);
		int16 y = READ_UINT16(g_topCatAnswersScreenPositions + answerIdx * 4 + 2);

		int16 colorShift = (answerIdx - i) * 3;
		byte *answerPic = getResolvedPtr(questionsAnswersGfxTable, tableOff + 4);
		adjustColors(answerPic, colorShift);
		displayPic(answerPic, x, y, stBack);

		tableOff += 4;
	}

	freeBOLTGroup(g_topCatBoltLib, groupId, 1);

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

		byte tmp = g_topCatShuffledQuestions[i];
		g_topCatShuffledQuestions[i] = g_topCatShuffledQuestions[randIdx];
		g_topCatShuffledQuestions[randIdx] = tmp;

		for (int16 j = 0; j < 3; j++) {
			int16 randJ = _xp->getRandom(3);

			byte aTmp = g_topCatAnswersPermutations[answerOff + j];
			g_topCatAnswersPermutations[answerOff + j] = g_topCatAnswersPermutations[answerOff + randJ];
			g_topCatAnswersPermutations[answerOff + randJ] = aTmp;
		}

		answerOff += 3;
	}
}

void BoltEngine::shuffleTopCatPermutations() {
	// Shuffle first permutation table...
	for (int16 i = 0; i < 3; i++) {
		int16 randIdx = _xp->getRandom(3);

		int16 tmp = g_topCatPermTableA[i];
		g_topCatPermTableA[i] = g_topCatPermTableA[randIdx];
		g_topCatPermTableA[randIdx] = tmp;
	}

	// Shuffle second permutation table...
	for (int16 i = 0; i < 3; i++) {
		int16 randIdx = _xp->getRandom(3);

		int16 tmp = g_topCatPermTableB[i];
		g_topCatPermTableB[i] = g_topCatPermTableB[randIdx];
		g_topCatPermTableB[randIdx] = tmp;
	}
}

void BoltEngine::getTopCatSoundInfo(BOLTLib *lib, int16 memberId, FredSoundInfo *soundInfo) {
	soundInfo->data = memberAddr(lib, memberId);
	soundInfo->size = memberSize(lib, memberId);
}

void BoltEngine::setScoreLights(int16 litMask) {
	int16 lightIdx = 0;
	int16 tableOff = 0;

	while (tableOff < 0x18) {
		byte *lightEntry = getResolvedPtr(g_topCatLightsPalette, tableOff);

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
	byte *data = g_boltCurrentMemberEntry->dataPtr;
	uint32 decompSize = g_boltCurrentMemberEntry->decompSize;
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
