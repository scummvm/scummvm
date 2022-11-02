/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995 Presto Studios, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "buried/biochip_right.h"
#include "buried/buried.h"
#include "buried/gameui.h"
#include "buried/graphics.h"
#include "buried/inventory_window.h"
#include "buried/resources.h"
#include "buried/scene_view.h"
#include "buried/sound.h"
#include "buried/environ/scene_base.h"
#include "buried/environ/scene_common.h"

#include "common/system.h"
#include "graphics/font.h"

namespace Buried {

class LairEntry : public SceneBase {
public:
	LairEntry(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int postEnterRoom(Window *viewWindow, const Location &priorLocation) override;
	int preExitRoom(Window *viewWindow, const Location &newLocation) override;
	int timerCallback(Window *viewWindow) override;
	int onCharacter(Window *viewWindow, const Common::KeyState &character) override;

private:
	int _movieIndex;
	uint32 _timerStart;
	Common::String _passwordEntered;
	int _currentSoundID;
	int _passwordIndex;
	uint32 _stepDelay;
	uint32 _rawStepDelay;
};

LairEntry::LairEntry(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_movieIndex = 0;
	_timerStart = 0;
	_currentSoundID = -1;
	_passwordIndex = 0;
	_stepDelay = 0;
	_rawStepDelay = 15000;
}

int LairEntry::postEnterRoom(Window *viewWindow, const Location &priorLocation) {
	const int effectsIndexBase = 2;	// same as kEffectsIndexBase in SoundManager

	// Force enable frame cycling
	((SceneViewWindow *)viewWindow)->forceEnableCycling(true);

	// Disable frame caching
	((SceneViewWindow *)viewWindow)->enableCycleFrameCache(false);

	// Force open the video
	((SceneViewWindow *)viewWindow)->changeCycleFrameMovie(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, SF_CYCLES));

	if (((SceneViewWindow *)viewWindow)->getGlobalFlags().alRestoreSkipAgent3Initial == 1) {
		// Start new secondary ambient
		_vm->_sound->setSecondaryAmbientSound(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 14), 64);

		_staticData.cycleStartFrame = 0;
		_staticData.cycleFrameCount = 54;
		_frameCycleCount = _staticData.cycleStartFrame;

		_passwordIndex = 0;
		_stepDelay = 0;
		_timerStart = g_system->getMillis();

		// Empty the input queue
		_vm->removeMouseMessages(viewWindow);
		_vm->removeKeyboardMessages(viewWindow);

		return SC_TRUE;
	}

	// Empty the input queue
	_vm->removeMouseMessages(viewWindow);
	_vm->removeKeyboardMessages(viewWindow);

	// Make sure we have the proper cycle going on
	_staticData.cycleStartFrame = 54;
	_staticData.cycleFrameCount = 90;
	_frameCycleCount = _staticData.cycleStartFrame;

	// Reset the timer and cursor
	uint32 lastTimerValue = 0;
	Cursor oldCursor = _vm->_gfx->setCursor(kCursorWait);

	// Remove some inventory items
	((GameUIWindow *)viewWindow->getParent())->_bioChipRightWindow->changeCurrentBioChip(kItemBioChipInterface);
	((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->removeItem(kItemBioChipJump);
	((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->removeItem(kItemEnvironCart);

	_vm->_gfx->setCursor(oldCursor);
	_staticData.navFrameIndex = -1;
	((SceneViewWindow *)viewWindow)->playSynchronousAnimation(10);
	_staticData.navFrameIndex = 75;
	oldCursor = _vm->_gfx->setCursor(kCursorWait);
	viewWindow->invalidateWindow(false);
	_vm->_gfx->updateScreen();

	// Play the next Arthur speech
	_currentSoundID = _vm->_sound->playSoundEffect(_vm->getFilePath(IDS_AGENT3_VIRUS_SOUND_BASE), 128, false, true);
	_timerStart = g_system->getMillis();
	lastTimerValue = g_system->getMillis();

	while (!_vm->shouldQuit() && _vm->_sound->isSoundEffectPlaying(_currentSoundID)) {
		if (g_system->getMillis() - lastTimerValue >= 50) {
			timerCallback(viewWindow);
			lastTimerValue = g_system->getMillis();
		}

		_vm->_sound->timerCallback();
		_vm->yield(nullptr, effectsIndexBase + _currentSoundID);
	}

	_vm->_sound->stopSoundEffect(_currentSoundID);

	_vm->_gfx->setCursor(oldCursor);
	((SceneViewWindow *)viewWindow)->playSynchronousAnimation(11);
	oldCursor = _vm->_gfx->setCursor(kCursorWait);

	// Add message here
	Common::String text;
	if (_vm->getVersion() >= MAKEVERSION(1, 0, 4, 0))
		text = _vm->getString(IDS_AGENT3_VIRUS_NETWORK_TEXT);
	else
		text = "Neuroconductor uplink connected. Network on-line.";
	((SceneViewWindow *)viewWindow)->displayLiveText(text, false);

	// Play the next Arthur speech
	_currentSoundID = _vm->_sound->playSoundEffect(_vm->getFilePath(IDS_AGENT3_VIRUS_SOUND_BASE + 1), 128, false, true);
	_timerStart = g_system->getMillis();
	lastTimerValue = g_system->getMillis();

	while (!_vm->shouldQuit() && _vm->_sound->isSoundEffectPlaying(_currentSoundID)) {
		if (g_system->getMillis() - lastTimerValue >= 50) {
			timerCallback(viewWindow);
			lastTimerValue = g_system->getMillis();
		}

		_vm->_sound->timerCallback();
		_vm->yield(nullptr, effectsIndexBase + _currentSoundID);
	}

	_vm->_sound->stopSoundEffect(_currentSoundID);

	_staticData.cycleStartFrame = 0;
	_staticData.cycleFrameCount = 54;
	_frameCycleCount = _staticData.cycleStartFrame;

	_vm->_gfx->setCursor(oldCursor);
	((SceneViewWindow *)viewWindow)->playSynchronousAnimation(12);
	oldCursor = _vm->_gfx->setCursor(kCursorWait);

	_currentSoundID = _vm->_sound->playSoundEffect(_vm->getFilePath(IDS_AGENT3_VIRUS_SOUND_BASE + 6), 128, false, true);

	// Start new secondary ambient
	_vm->_sound->setSecondaryAmbientSound(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 14), 64);

	_passwordIndex = 0;
	_stepDelay = 0;
	_timerStart = g_system->getMillis();

	// Empty the input queue
	_vm->removeMouseMessages(viewWindow);
	_vm->removeKeyboardMessages(viewWindow);

	_vm->_gfx->setCursor(oldCursor);
	return SC_TRUE;
}

int LairEntry::preExitRoom(Window *viewWindow, const Location &newLocation) {
	((SceneViewWindow *)viewWindow)->forceEnableCycling(false);
	return SC_TRUE;
}

int LairEntry::timerCallback(Window *viewWindow) {
	SceneBase::timerCallback(viewWindow);

	if (_currentSoundID >= 0) {
		if (_vm->_sound->isSoundEffectPlaying(_currentSoundID)) {
			return SC_TRUE;
		} else {
			_vm->_sound->stopSoundEffect(_currentSoundID);
			_currentSoundID = -1;
			_timerStart = g_system->getMillis();
		}
	}

	Common::KeyState dummyKey;
	dummyKey.keycode = Common::KEYCODE_INVALID;
	dummyKey.ascii = 0;
	dummyKey.flags = 0;

	if (g_system->getMillis() > _timerStart + _stepDelay) {
		switch (_passwordIndex) {
		case 0:
			_passwordIndex = 1;
			onCharacter(viewWindow, dummyKey);
			_timerStart = g_system->getMillis();
			_stepDelay = _rawStepDelay;
			break;
		case 1:
			_currentSoundID = _vm->_sound->playSoundEffect(_vm->getFilePath(IDS_AGENT3_VIRUS_SOUND_BASE + 2), 128, false, true);
			_passwordIndex = 2;
			onCharacter(viewWindow, dummyKey);
			_timerStart = g_system->getMillis();
			_stepDelay = _rawStepDelay;
			break;
		case 2:
			_currentSoundID = _vm->_sound->playSoundEffect(_vm->getFilePath(IDS_AGENT3_VIRUS_SOUND_BASE + 3), 128, false, true);
			_passwordIndex = 3;
			onCharacter(viewWindow, dummyKey);
			_timerStart = g_system->getMillis();
			_stepDelay = _rawStepDelay;
			break;
		case 3:
			_currentSoundID = _vm->_sound->playSoundEffect(_vm->getFilePath(IDS_AGENT3_VIRUS_SOUND_BASE + 4), 128, false, true);
			_passwordIndex = 4;
			onCharacter(viewWindow, dummyKey);
			_timerStart = g_system->getMillis();
			_stepDelay = _rawStepDelay;
			break;
		case 4:
			// Mind wipe.
			_vm->_sound->setAmbientSound();
			_vm->_sound->setSecondaryAmbientSound();
			((SceneViewWindow *)viewWindow)->playSynchronousAnimation(13);
			((SceneViewWindow *)viewWindow)->forceEnableCycling(false);
			((SceneViewWindow *)viewWindow)->showDeathScene(20);
			break;
		case 5: {
			((SceneViewWindow *)viewWindow)->getGlobalFlags().scoreEliminatedAgent3 = 1;

			// "Kill" off Agent 3 *snicker*
			_passwordIndex = 1;
			onCharacter(viewWindow, dummyKey);
			((SceneViewWindow *)viewWindow)->changeStillFrameMovie(_vm->getFilePath(3, 1, SF_STILLS));
			((SceneViewWindow *)viewWindow)->playSynchronousAnimation(14);
			_frameCycleCount = -1;
			_staticData.cycleStartFrame = -1;
			_staticData.cycleFrameCount = -1;
			_staticData.navFrameIndex = 75;

			DestinationScene newScene;
			newScene.destinationScene = Location(3, 1, 3, 3, 1, 0);
			newScene.transitionType = TRANSITION_VIDEO;
			newScene.transitionData = 15;
			newScene.transitionStartFrame = -1;
			newScene.transitionLength = -1;
			((SceneViewWindow *)viewWindow)->moveToDestination(newScene);
			break;
		}
		}
	}

	return SC_TRUE;
}

int LairEntry::onCharacter(Window *viewWindow, const Common::KeyState &character) {
	const int effectsIndexBase = 2; // same as kEffectsIndexBase in SoundManager

	// Only accept input if we are beyond first voiceover
	if (_passwordIndex <= 0)
		return SC_TRUE;

	if (character.keycode == Common::KEYCODE_DELETE || character.keycode == Common::KEYCODE_BACKSPACE) {
		if (!_passwordEntered.empty())
			_passwordEntered.deleteLastChar();
	} else if ((character.keycode == Common::KEYCODE_SPACE || (character.keycode >= Common::KEYCODE_a && character.keycode <= Common::KEYCODE_z) ||
			(character.keycode >= Common::KEYCODE_0 && character.keycode <= Common::KEYCODE_9)) && _passwordEntered.size() < 15) {

		if (character.keycode == Common::KEYCODE_SPACE)
			_passwordEntered += ' ';
		else if (character.keycode >= Common::KEYCODE_a && character.keycode <= Common::KEYCODE_z)
			_passwordEntered += (char)(character.keycode - Common::KEYCODE_a + 'A');
		else
			_passwordEntered += (char)(character.keycode - Common::KEYCODE_0 + '0');
	}

	Common::String liveText = _vm->getString(IDS_AGENT3_VIRUS_TEXT_A);
	liveText += _passwordEntered;
	liveText += _vm->getString(IDS_AGENT3_VIRUS_CURSOR);

	if (_passwordIndex > 1)
		liveText += "\n" + _vm->getString(IDS_AGENT3_VIRUS_TEXT_B);
	if (_passwordIndex > 2)
		liveText += "\n" + _vm->getString(IDS_AGENT3_VIRUS_TEXT_C);
	if (_passwordIndex > 3)
		liveText += "\n" + _vm->getString(IDS_AGENT3_VIRUS_TEXT_D);

	((SceneViewWindow  *)viewWindow)->displayLiveText(liveText, false);

	if (character.keycode == Common::KEYCODE_RETURN) {
		if (_passwordEntered == _vm->getString(IDS_AGENT3_VIRUS_PASSWORD)) {
			liveText = _vm->getString(IDS_AGENT3_VIRUS_PW_ACCEPTED);

			if (_passwordIndex > 1)
				liveText += "\n" + _vm->getString(IDS_AGENT3_VIRUS_TEXT_B);
			if (_passwordIndex > 2)
				liveText += "\n" + _vm->getString(IDS_AGENT3_VIRUS_TEXT_C);
			if (_passwordIndex > 3)
				liveText += "\n" + _vm->getString(IDS_AGENT3_VIRUS_TEXT_D);

			((SceneViewWindow  *)viewWindow)->displayLiveText(liveText, false);

			if (_currentSoundID >= 0)
				_vm->_sound->stopSoundEffect(_currentSoundID);

			_currentSoundID = _vm->_sound->playSoundEffect(_vm->getFilePath(IDS_AGENT3_VIRUS_SOUND_BASE + 5), 128, false, true);

			_timerStart = g_system->getMillis();
			while (!_vm->shouldQuit() && _vm->_sound->isSoundEffectPlaying(_currentSoundID)) {
				if ((g_system->getMillis() - _timerStart) % 20 < 5)
					timerCallback(viewWindow);

				_vm->_sound->timerCallback();
				_vm->yield(nullptr, effectsIndexBase + _currentSoundID);
			}

			_vm->_sound->stopSoundEffect(_currentSoundID);
			((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->removeItem(kItemBioChipAI);
			((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->addItem(kItemBioChipBlank);
			((GameUIWindow *)viewWindow->getParent())->_bioChipRightWindow->swapAIBioChipIfActive();

			_vm->_sound->setAmbientSound(_vm->getFilePath(3, 2, SF_AMBIENT), false, 64);
			_passwordIndex = 5;
			_timerStart = 0;
			_stepDelay = 0;
			return SC_TRUE;
		}

		// Watch out, some curse words in here
		bool newVersion = (_vm->getVersion() >= MAKEVERSION(1, 0, 4, 0));
		Common::String vulgarLangA = newVersion ? _vm->getString(IDS_AL_VULGAR_LANG_A) : "FUCKER";
		Common::String vulgarLangB = newVersion ? _vm->getString(IDS_AL_VULGAR_LANG_B) : "SHITHEAD";
		Common::String vulgarLangC = newVersion ? _vm->getString(IDS_AL_VULGAR_LANG_C) : "BITCH";
		Common::String vulgarLangD = newVersion ? _vm->getString(IDS_AL_VULGAR_LANG_D) : "CUNT";
		Common::String vulgarLangE = newVersion ? _vm->getString(IDS_AL_VULGAR_LANG_E) : "WHORE";
		Common::String vulgarLangF = newVersion ? _vm->getString(IDS_AL_VULGAR_LANG_F) : "ASSHOLE";
		Common::String vulgarLangG = newVersion ? _vm->getString(IDS_AL_VULGAR_LANG_G) : "TWAT";
		Common::String vulgarLangH = newVersion ? _vm->getString(IDS_AL_VULGAR_LANG_H) : "FUCK";
		Common::String vulgarLangI = newVersion ? _vm->getString(IDS_AL_VULGAR_LANG_I) : "SHIT";

		if (_passwordEntered == vulgarLangA || _passwordEntered == vulgarLangB || _passwordEntered == vulgarLangC ||
				_passwordEntered == vulgarLangD || _passwordEntered == vulgarLangE || _passwordEntered == vulgarLangF ||
				_passwordEntered == vulgarLangG || _passwordEntered == vulgarLangH || _passwordEntered == vulgarLangI) {
			liveText = _vm->getString(IDS_AGENT3_VIRUS_TEXT_A);
			liveText += _passwordEntered;
			liveText += _vm->getString(IDS_AGENT3_VIRUS_CURSOR);
			liveText += newVersion ? _vm->getString(IDS_AL_CASTRATION_TEXT) : "\nVULGAR LANGUAGE UNACCEPTABLE. CASTRATION TOOL ACTIVATED.";
			((SceneViewWindow  *)viewWindow)->displayLiveText(liveText, false);
			_passwordIndex = 4;
			_timerStart = 0;
			_stepDelay = 0;
			return SC_TRUE;
		}

		_passwordEntered.clear();
		liveText = _vm->getString(IDS_AGENT3_VIRUS_TEXT_A);
		liveText += _passwordEntered;
		liveText += _vm->getString(IDS_AGENT3_VIRUS_CURSOR);

		if (_passwordIndex > 1)
			liveText += "\n" + _vm->getString(IDS_AGENT3_VIRUS_TEXT_B);
		if (_passwordIndex > 2)
			liveText += "\n" + _vm->getString(IDS_AGENT3_VIRUS_TEXT_C);
		if (_passwordIndex > 3)
			liveText += "\n" + _vm->getString(IDS_AGENT3_VIRUS_TEXT_D);

		((SceneViewWindow  *)viewWindow)->displayLiveText(liveText, false);
	}

	return SC_TRUE;
}

class ReplicatorInterface : public SceneBase {
public:
	ReplicatorInterface(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int postExitRoom(Window *viewWindow, const Location &newLocation) override;
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation) override;
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation) override;

private:
	int _currentItem;
	Common::Rect _runProgram, _scrollUp, _scrollDown;
};

ReplicatorInterface::ReplicatorInterface(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_currentItem = 0;
	_runProgram = Common::Rect(140, 45, 228, 67);
	_scrollUp = Common::Rect(253, 61, 277, 83);
	_scrollDown = Common::Rect(253, 83, 277, 105);
}

int ReplicatorInterface::postExitRoom(Window *viewWindow, const Location &newLocation) {
	if (newLocation.timeZone == _staticData.location.timeZone && newLocation.environment == _staticData.location.environment)
		_vm->_sound->playSoundEffect("BITDATA/AGENT3/ALNMRCLS.BTA");

	return SC_TRUE;
}

int ReplicatorInterface::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_runProgram.contains(pointLocation)) {
		_vm->_sound->playSynchronousSoundEffect("BITDATA/COMMON/GENB14.BTA");

		switch (_currentItem) {
		case 1:
			((SceneViewWindow *)viewWindow)->playSynchronousAnimation(4);
			break;
		case 2:
			((SceneViewWindow *)viewWindow)->playSynchronousAnimation(3);
			break;
		case 3:
			((SceneViewWindow *)viewWindow)->playSynchronousAnimation(1);
			break;
		case 4:
			((SceneViewWindow *)viewWindow)->playSynchronousAnimation(2);
			break;
		}

		_staticData.navFrameIndex = 76;
		_currentItem = 0;
		viewWindow->invalidateWindow(false);
		return SC_TRUE;
	}

	if (_scrollUp.contains(pointLocation)) {
		_currentItem++;
		if (_currentItem > 4)
			_currentItem = 1;

		_staticData.navFrameIndex = _currentItem + 76;
		viewWindow->invalidateWindow(false);
		_vm->_sound->playSynchronousSoundEffect("BITDATA/COMMON/GENB14.BTA");
		return SC_TRUE;
	}

	if (_scrollDown.contains(pointLocation)) {
		_currentItem--;
		if (_currentItem < 1)
			_currentItem = 4;

		_staticData.navFrameIndex = _currentItem + 76;
		viewWindow->invalidateWindow(false);
		_vm->_sound->playSynchronousSoundEffect("BITDATA/COMMON/GENB14.BTA");
		return SC_TRUE;
	}

	return SC_FALSE;
}

int ReplicatorInterface::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_runProgram.contains(pointLocation) || _scrollUp.contains(pointLocation) || _scrollDown.contains(pointLocation))
		return kCursorFinger;

	return kCursorArrow;
}

class TransporterControls : public SceneBase {
public:
	TransporterControls(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	~TransporterControls();
	void preDestructor() override;
	int postExitRoom(Window *viewWindow, const Location &newLocation) override;
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation) override;
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation) override;
	int onCharacter(Window *viewWindow, const Common::KeyState &character) override;
	int gdiPaint(Window *viewWindow) override;

private:
	Common::Rect _monitor, _retract;
	Common::String _transportCode;
	Common::String _prefixCode;
	int _monitorStatus;
	Graphics::Font *_textFont;
	int _lineHeight;
};

TransporterControls::TransporterControls(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_monitor = Common::Rect(171, 42, 307, 136);
	_retract = Common::Rect(362, 115, 394, 132);
	_monitorStatus = 0;

	_lineHeight = (_vm->getLanguage() == Common::JA_JPN) ? 10 : 12;
	_textFont = _vm->_gfx->createFont(_lineHeight);
}

TransporterControls::~TransporterControls() {
	preDestructor();
}

void TransporterControls::preDestructor() {
	delete _textFont;
	_textFont = nullptr;
}

int TransporterControls::postExitRoom(Window *viewWindow, const Location &newLocation) {
	if (newLocation.timeZone == _staticData.location.timeZone && newLocation.environment == _staticData.location.environment &&
			newLocation.node == _staticData.location.node && newLocation.facing != _staticData.location.facing) {
		_vm->_sound->playSoundEffect("BITDATA/AGENT3/ALNMTRO.BTA");
	}

	return SC_TRUE;
}

int TransporterControls::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_monitor.contains(pointLocation) && (_monitorStatus == 0 || _monitorStatus == 2)) {
		_monitorStatus = 1;
		_staticData.navFrameIndex = 84;
		viewWindow->invalidateWindow();
	} else if (_retract.contains(pointLocation)) {
		DestinationScene newScene;
		newScene.destinationScene = _staticData.location;
		newScene.destinationScene.depth = 0;
		newScene.transitionType = TRANSITION_VIDEO;
		newScene.transitionData = 20;
		newScene.transitionStartFrame = -1;
		newScene.transitionLength = -1;
		((SceneViewWindow *)viewWindow)->moveToDestination(newScene);
		return SC_TRUE;
	}

	return SC_FALSE;
}

int TransporterControls::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if ((_monitor.contains(pointLocation) && (_monitorStatus == 0 || _monitorStatus == 2)) || _retract.contains(pointLocation))
		return kCursorFinger;

	return kCursorArrow;
}

int TransporterControls::onCharacter(Window *viewWindow, const Common::KeyState &character) {
	if (_monitorStatus == 1) {
		if (character.keycode == Common::KEYCODE_BACKSPACE || character.keycode == Common::KEYCODE_DELETE) {
			if (!_transportCode.empty()) {
				_transportCode.deleteLastChar();
				_vm->_sound->playSoundEffect("BITDATA/COMMON/GENB14.BTA");
			}
		} else if (character.keycode >= Common::KEYCODE_0 && character.keycode <= Common::KEYCODE_9) {
			_transportCode += (char)(character.keycode - Common::KEYCODE_0 + '0');
			_vm->_sound->playSoundEffect("BITDATA/COMMON/GENB14.BTA");
		}

		viewWindow->invalidateWindow(false);

		if (_transportCode.size() >= 12) {
			if (_transportCode == "657255190235") {
				if (((SceneViewWindow *)viewWindow)->getGlobalFlags().generalWalkthroughMode == 1) {
					// Bypass the prefix code puzzle in walkthrough mode
					_monitorStatus = 4;
					_staticData.navFrameIndex = 87;
					viewWindow->invalidateWindow(false);

					// Wait two seconds
					uint32 startTime = g_system->getMillis();
					while (!_vm->shouldQuit() && startTime + 2000 > g_system->getMillis())
						_vm->yield(nullptr, -1);

					// Move to a different depth to enter the transporter
					DestinationScene newScene;
					newScene.destinationScene = _staticData.location;
					newScene.destinationScene.depth = 2;
					newScene.transitionType = TRANSITION_VIDEO;
					newScene.transitionData = 7;
					newScene.transitionStartFrame = -1;
					newScene.transitionLength = -1;
					((SceneViewWindow *)viewWindow)->moveToDestination(newScene);
				} else {
					_monitorStatus = 3;
					_staticData.navFrameIndex = 85;
					viewWindow->invalidateWindow(false);
				}
			} else {
				_monitorStatus = 2;
				_staticData.navFrameIndex = 86;
				_transportCode.clear();
				viewWindow->invalidateWindow(false);
			}
		}

		return SC_TRUE;
	} else if (_monitorStatus == 3) {
		// Original allows any character to be printed and doesn't include backspace here
		// That sucks; I'm using the same code as for the transporter code.
		if (character.keycode == Common::KEYCODE_BACKSPACE || character.keycode == Common::KEYCODE_DELETE) {
			if (!_prefixCode.empty()) {
				_prefixCode.deleteLastChar();
				_vm->_sound->playSoundEffect("BITDATA/COMMON/GENB14.BTA");
			}
		} else if (character.keycode >= Common::KEYCODE_0 && character.keycode <= Common::KEYCODE_9) {
			_prefixCode += (char)(character.keycode - Common::KEYCODE_0 + '0');
			_vm->_sound->playSoundEffect("BITDATA/COMMON/GENB14.BTA");
		}

		viewWindow->invalidateWindow(false);

		if (_prefixCode.size() >= 3) {
			// Flag a wrong prefix for later
			((SceneViewWindow *)viewWindow)->getGlobalFlags().alNMWrongAlienPrefixCode = (_prefixCode != "272");

			_prefixCode.clear();
			_monitorStatus = 4;
			_staticData.navFrameIndex = 87;
			viewWindow->invalidateWindow(false);

			// Wait two seconds
			uint32 startTime = g_system->getMillis();
			while (!_vm->shouldQuit() && startTime + 2000 > g_system->getMillis())
				_vm->yield(nullptr, -1);

			// Move to a different depth to enter the transporter
			DestinationScene newScene;
			newScene.destinationScene = _staticData.location;
			newScene.destinationScene.depth = 2;
			newScene.transitionType = TRANSITION_VIDEO;
			newScene.transitionData = 7;
			newScene.transitionStartFrame = -1;
			newScene.transitionLength = -1;
			((SceneViewWindow *)viewWindow)->moveToDestination(newScene);
		}

		return SC_TRUE;
	}

	return SC_FALSE;
}

int TransporterControls::gdiPaint(Window *viewWindow) {
	if (_monitorStatus == 1 || _monitorStatus == 3) {
		uint32 color = _vm->_gfx->getColor(80, 216, 144);
		Common::Rect absoluteRect = viewWindow->getAbsoluteRect();

		if (_monitorStatus == 1) {
			Common::Rect textRegion(190, 78, 280, 128);
			textRegion.translate(absoluteRect.left, absoluteRect.top);
			_vm->_gfx->renderText(_vm->_gfx->getScreen(), _textFont, _transportCode, textRegion.left, textRegion.top, textRegion.width(), textRegion.height(), color, _lineHeight);
		} else {
			Common::Rect textRegion(190, 120, 253, 133);
			textRegion.translate(absoluteRect.left, absoluteRect.top);
			_vm->_gfx->renderText(_vm->_gfx->getScreen(), _textFont, _prefixCode, textRegion.left, textRegion.top, textRegion.width(), textRegion.height(), color, _lineHeight);
		}
	}

	return SC_TRUE;
}

class GeneratorCoreZoom : public SceneBase {
public:
	GeneratorCoreZoom(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation) override;
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation) override;

private:
	Common::Rect _clickableArea;
};

GeneratorCoreZoom::GeneratorCoreZoom(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	if (((SceneViewWindow *)viewWindow)->getGlobalFlags().alRDTakenDeadCore == 1)
		_staticData.navFrameIndex = 82;

	_clickableArea = Common::Rect(42, 34, 132, 116);
}

int GeneratorCoreZoom::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_clickableArea.contains(pointLocation)) {
		DestinationScene newScene;
		newScene.destinationScene = _staticData.location;
		newScene.destinationScene.depth = 1;
		newScene.transitionType = TRANSITION_VIDEO;
		newScene.transitionStartFrame = -1;
		newScene.transitionLength = -1;

		if (((SceneViewWindow *)viewWindow)->getGlobalFlags().alRDTakenDeadCore == 1)
			newScene.transitionData = 7;
		else
			newScene.transitionData = 6;

		((SceneViewWindow *)viewWindow)->moveToDestination(newScene);

		return SC_TRUE;
	}

	return SC_FALSE;
}

int GeneratorCoreZoom::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_clickableArea.contains(pointLocation))
		return kCursorMagnifyingGlass;

	return kCursorArrow;
}

class GeneratorCoreAcquire : public SceneBase {
public:
	GeneratorCoreAcquire(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int mouseDown(Window *viewWindow, const Common::Point &pointLocation) override;
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation) override;
	int draggingItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) override;
	int droppedItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) override;
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation) override;

private:
	int _currentStatus;
	Common::Rect _deadCore;
	Common::Rect _closedEmpty;
};

GeneratorCoreAcquire::GeneratorCoreAcquire(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	if (((SceneViewWindow *)viewWindow)->getGlobalFlags().alRDTakenDeadCore == 1) {
		_staticData.navFrameIndex = 80;
		_currentStatus = 2;
	} else {
		_currentStatus = 0;
	}

	_deadCore = Common::Rect(167, 0, 257, 138);
	_closedEmpty = Common::Rect(181, 76, 257, 132);
}

int GeneratorCoreAcquire::mouseDown(Window *viewWindow, const Common::Point &pointLocation) {
	if (_currentStatus == 1 && _deadCore.contains(pointLocation)) {
		_staticData.navFrameIndex = 80;
		_currentStatus = 2;
		viewWindow->invalidateWindow(false);

		// Taken the core
		((SceneViewWindow *)viewWindow)->getGlobalFlags().alRDTakenDeadCore = 1;

		Common::Point ptInventoryWindow = viewWindow->convertPointToWindow(pointLocation, ((GameUIWindow *)viewWindow->getParent())->_inventoryWindow);
		((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->startDraggingNewItem(kItemBurnedOutCore, ptInventoryWindow);
		return SC_TRUE;
	}

	return SC_FALSE;
}

int GeneratorCoreAcquire::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_currentStatus == 0 && _closedEmpty.contains(pointLocation)) {
		((SceneViewWindow *)viewWindow)->playSynchronousAnimation(9);
		_staticData.navFrameIndex = 77;
		_currentStatus = 1;
		return SC_TRUE;
	}

	DestinationScene newScene;
	newScene.destinationScene = _staticData.location;
	newScene.destinationScene.depth = 0;
	newScene.transitionType = TRANSITION_NONE;
	newScene.transitionData = -1;
	newScene.transitionStartFrame = -1;
	newScene.transitionLength = -1;
	((SceneViewWindow *)viewWindow)->moveToDestination(newScene);
	return SC_TRUE;
}

int GeneratorCoreAcquire::draggingItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) {
	if (_currentStatus == 2 && (itemID == kItemGeneratorCore || itemID == kItemBurnedOutCore) && _closedEmpty.contains(pointLocation))
		return 1;

	return 0; // Original had a nice bug here returning the put down cursor
}

int GeneratorCoreAcquire::droppedItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) {
	if (pointLocation.x == -1 && pointLocation.y == -1)
		return SIC_REJECT;

	if (_currentStatus == 2 && (itemID == kItemGeneratorCore || itemID == kItemBurnedOutCore) && _closedEmpty.contains(pointLocation)) {
		if (itemID == kItemBurnedOutCore) {
			// Change background and status
			_staticData.navFrameIndex = 77;
			_currentStatus = 1;
			((SceneViewWindow *)viewWindow)->getGlobalFlags().alRDTakenDeadCore = 0;
			viewWindow->invalidateWindow(false);
		} else if (itemID == kItemGeneratorCore) {
			_staticData.navFrameIndex = 79;

			// Move to the new scene
			DestinationScene newScene;
			newScene.destinationScene = _staticData.location;
			newScene.destinationScene.depth = 0;
			newScene.destinationScene.environment = 2;
			newScene.transitionType = TRANSITION_VIDEO;
			newScene.transitionData = 8;
			newScene.transitionStartFrame = -1;
			newScene.transitionLength = -1;
			((SceneViewWindow *)viewWindow)->moveToDestination(newScene);
		}

		return SIC_ACCEPT;
	}

	return SIC_REJECT;
}

int GeneratorCoreAcquire::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_currentStatus == 0 && _closedEmpty.contains(pointLocation))
		return kCursorFinger;

	if (_currentStatus == 1 && _deadCore.contains(pointLocation))
		return kCursorOpenHand;

	return kCursorPutDown;
}

class ZoomInPostItAndINN : public SceneBase {
public:
	ZoomInPostItAndINN(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation) override;
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation) override;

private:
	Common::Rect _postItNote;
	Common::Rect _innScreen;
};

ZoomInPostItAndINN::ZoomInPostItAndINN(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_postItNote = Common::Rect(8, 150, 57, 189);
	_innScreen = Common::Rect(64, 97, 208, 160);
}

int ZoomInPostItAndINN::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_postItNote.contains(pointLocation)) {
		DestinationScene newScene;
		newScene.destinationScene = _staticData.location;
		newScene.destinationScene.depth = 1;
		newScene.transitionType = TRANSITION_VIDEO;
		newScene.transitionData = 5;
		newScene.transitionStartFrame = -1;
		newScene.transitionLength = 1;
		((SceneViewWindow *)viewWindow)->moveToDestination(newScene);
		return SC_TRUE;
	}

	if (_innScreen.contains(pointLocation)) {
		DestinationScene newScene;
		newScene.destinationScene = _staticData.location;
		newScene.destinationScene.depth = 2;
		newScene.transitionType = TRANSITION_VIDEO;
		newScene.transitionData = 16;
		newScene.transitionStartFrame = -1;
		newScene.transitionLength = 1;
		((SceneViewWindow *)viewWindow)->moveToDestination(newScene);
		return SC_TRUE;
	}

	return SC_FALSE;
}

int ZoomInPostItAndINN::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_postItNote.contains(pointLocation) || _innScreen.contains(pointLocation))
		return kCursorMagnifyingGlass;

	return kCursorArrow;
}

class CompleteTransport : public SceneBase {
public:
	CompleteTransport(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int timerCallback(Window *viewWindow) override;
};

CompleteTransport::CompleteTransport(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
}

int CompleteTransport::timerCallback(Window *viewWindow) {
	((SceneViewWindow *)viewWindow)->playSynchronousAnimation(19);

	if (((SceneViewWindow *)viewWindow)->getGlobalFlags().alNMWrongAlienPrefixCode == 1) {
		((SceneViewWindow *)viewWindow)->showDeathScene(21);
	} else {
		if (((SceneViewWindow *)viewWindow)->getGlobalFlags().lensFilterActivated == 1) {
			DestinationScene newScene;
			newScene.destinationScene = Location(7, 1, 5, 3, 1, 0);
			newScene.transitionType = TRANSITION_VIDEO;
			newScene.transitionData = 18;
			newScene.transitionStartFrame = -1;
			newScene.transitionLength = -1;
			((SceneViewWindow *)viewWindow)->moveToDestination(newScene);
		} else {
			DestinationScene newScene;
			newScene.destinationScene = Location(7, 1, 5, 3, 1, 1);
			newScene.transitionType = TRANSITION_NONE;
			newScene.transitionData = -1;
			newScene.transitionStartFrame = -1;
			newScene.transitionLength = -1;
			((SceneViewWindow *)viewWindow)->jumpToScene(newScene.destinationScene);
		}
	}

	return SC_TRUE;
}

class ClickChangeScenePostIt : public ClickChangeScene {
public:
	ClickChangeScenePostIt(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
			int left = -1, int top = -1, int right = -1, int bottom = -1, int cursorID = 0,
			int timeZone = -1, int environment = -1, int node = -1, int facing = -1, int orientation = -1, int depth = -1,
			int transitionType = -1, int transitionData = -1, int transitionStartFrame = -1, int transitionLength = -1);
};

ClickChangeScenePostIt::ClickChangeScenePostIt(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
			int left, int top, int right, int bottom, int cursorID,
			int timeZone, int environment, int node, int facing, int orientation, int depth,
			int transitionType, int transitionData, int transitionStartFrame, int transitionLength) :
			ClickChangeScene(vm, viewWindow, sceneStaticData, priorLocation, left, top, right, bottom, cursorID,
					timeZone, environment, node, facing, orientation, depth, transitionType, transitionData,
					transitionStartFrame, transitionLength) {
	((SceneViewWindow *)viewWindow)->getGlobalFlags().scoreResearchAgent3Note = 1;
}

class PlayTransporterClosing : public SceneBase {
public:
	PlayTransporterClosing(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int postExitRoom(Window *viewWindow, const Location &newLocation) override;
};

PlayTransporterClosing::PlayTransporterClosing(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
}

int PlayTransporterClosing::postExitRoom(Window *viewWindow, const Location &newLocation) {
	if (_staticData.location.node == newLocation.node && _staticData.location.timeZone == newLocation.timeZone)
		_vm->_sound->playSoundEffect("BITDATA/AGENT3/ALNMTCLS.BTA");

	return SC_TRUE;
}

bool SceneViewWindow::initializeAgent3LairTimeZoneAndEnvironment(Window *viewWindow, int environment) {
	if (environment == -1)
		((SceneViewWindow *)viewWindow)->getGlobalFlags().alNMWrongAlienPrefixCode = 0;

	return true;
}

bool SceneViewWindow::startAgent3LairAmbient(int oldTimeZone, int oldEnvironment, int environment, bool fade) {
	_vm->_sound->setAmbientSound(_vm->getFilePath(3, environment, SF_AMBIENT), fade, 64);
	return true;
}

SceneBase *SceneViewWindow::constructAgent3LairSceneObject(Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) {
	SceneViewWindow *sceneView = ((SceneViewWindow *)viewWindow);
	GlobalFlags &globalFlags = sceneView->getGlobalFlags();

	switch (sceneStaticData.classID) {
	case 0:
		// Default scene
		break;
	case 1:
		return new GenericItemAcquire(_vm, viewWindow, sceneStaticData, priorLocation, 177, 96, 231, 184, kItemGeneratorCore, 15, globalFlags.alRDTakenLiveCore);
	case 2:
		return new GeneratorCoreZoom(_vm, viewWindow, sceneStaticData, priorLocation);
	case 3:
		return new GeneratorCoreAcquire(_vm, viewWindow, sceneStaticData, priorLocation);
	case 10:
		return new LairEntry(_vm, viewWindow, sceneStaticData, priorLocation);
	case 20:
		return new ClickChangeScene(_vm, viewWindow, sceneStaticData, priorLocation, 36, 15, 396, 189, kCursorFinger, 3, 2, 0, 1, 1, 1, TRANSITION_VIDEO, 0, -1, -1);
	case 21:
		return new ReplicatorInterface(_vm, viewWindow, sceneStaticData, priorLocation);
	case 25:
		return new ClickChangeScene(_vm, viewWindow, sceneStaticData, priorLocation, 150, 24, 280, 124, kCursorFinger, 3, 2, 4, 0, 1, 1, TRANSITION_VIDEO, 6, -1, -1);
	case 26:
		return new TransporterControls(_vm, viewWindow, sceneStaticData, priorLocation);
	case 27:
		return new ZoomInPostItAndINN(_vm, viewWindow, sceneStaticData, priorLocation);
	case 28:
		return new ClickChangeScenePostIt(_vm, viewWindow, sceneStaticData, priorLocation, 109, 0, 322, 189, kCursorPutDown, 3, 2, 0, 2, 1, 0, TRANSITION_VIDEO, 9, -1, -1);
	case 29:
		return new InteractiveNewsNetwork(_vm, viewWindow, sceneStaticData, priorLocation, -1, 3, 2, 0, 2, 1, 0, TRANSITION_VIDEO, 17, -1, -1);
	case 30:
		return new CompleteTransport(_vm, viewWindow, sceneStaticData, priorLocation);
	case 31:
		return new PlayTransporterClosing(_vm, viewWindow, sceneStaticData, priorLocation);
	default:
		warning("Unknown Agent 3 lair scene object %d", sceneStaticData.classID);
		break;
	}

	return new SceneBase(_vm, viewWindow, sceneStaticData, priorLocation);
}

} // End of namespace Buried
