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

#include "common/config-manager.h"
#include "common/events.h"
#include "common/system.h"
#include "common/translation.h"
#include "audio/mixer.h"

#include "scumm/debugger.h"
#include "scumm/dialogs.h"
#include "scumm/insane/insane.h"
#include "scumm/imuse/imuse.h"
#include "scumm/imuse_digi/dimuse_engine.h"
#ifdef ENABLE_HE
#include "scumm/he/intern_he.h"
#include "scumm/he/logic_he.h"
#endif
#include "scumm/resource.h"
#include "scumm/scumm_v0.h"
#include "scumm/scumm_v6.h"
#include "scumm/scumm_v8.h"
#include "scumm/sound.h"
#include "scumm/dialogs.h"

#include "graphics/cursorman.h"

namespace Scumm {

enum MouseButtonStatus {
	msDown = 1,
	msClicked = 2
};

#ifdef ENABLE_HE
void ScummEngine_v80he::parseEvent(Common::Event event) {
	ScummEngine::parseEvent(event);

	// Keyboard is controlled via variable
	switch (event.type) {
	case Common::EVENT_KEYDOWN:
		if (event.kbd.keycode == Common::KEYCODE_LEFT)
			VAR(VAR_KEY_STATE) |= 1;

		if (event.kbd.keycode == Common::KEYCODE_RIGHT)
			VAR(VAR_KEY_STATE) |= 2;

		if (event.kbd.keycode == Common::KEYCODE_UP)
			VAR(VAR_KEY_STATE) |= 4;

		if (event.kbd.keycode == Common::KEYCODE_DOWN)
			VAR(VAR_KEY_STATE) |= 8;

		if (event.kbd.keycode == Common::KEYCODE_LSHIFT || event.kbd.keycode == Common::KEYCODE_RSHIFT)
			VAR(VAR_KEY_STATE) |= 16;

		if (event.kbd.keycode == Common::KEYCODE_LCTRL || event.kbd.keycode == Common::KEYCODE_RCTRL)
			VAR(VAR_KEY_STATE) |= 32;
		break;

	case Common::EVENT_KEYUP:
		if (event.kbd.keycode == Common::KEYCODE_LEFT)
			VAR(VAR_KEY_STATE) &= ~1;

		if (event.kbd.keycode == Common::KEYCODE_RIGHT)
			VAR(VAR_KEY_STATE) &= ~2;

		if (event.kbd.keycode == Common::KEYCODE_UP)
			VAR(VAR_KEY_STATE) &= ~4;

		if (event.kbd.keycode == Common::KEYCODE_DOWN)
			VAR(VAR_KEY_STATE) &= ~8;

		if (event.kbd.keycode == Common::KEYCODE_LSHIFT || event.kbd.keycode == Common::KEYCODE_RSHIFT)
			VAR(VAR_KEY_STATE) &= ~16;

		if (event.kbd.keycode == Common::KEYCODE_LCTRL || event.kbd.keycode == Common::KEYCODE_RCTRL)
			VAR(VAR_KEY_STATE) &= ~32;
		break;

	default:
		break;
	}
}
#endif

void ScummEngine::parseEvent(Common::Event event) {
	switch (event.type) {
	case Common::EVENT_KEYDOWN:
		if (event.kbd.keycode >= Common::KEYCODE_0 && event.kbd.keycode <= Common::KEYCODE_9 &&
			((event.kbd.hasFlags(Common::KBD_ALT) && canSaveGameStateCurrently()) ||
			(event.kbd.hasFlags(Common::KBD_CTRL) && canLoadGameStateCurrently()))) {
			_saveLoadSlot = event.kbd.keycode - Common::KEYCODE_0;

			//  don't overwrite autosave (slot 0)
			if (_saveLoadSlot == 0)
				_saveLoadSlot = 10;

			_saveLoadDescription = Common::String::format("Quicksave %d", _saveLoadSlot);
			_saveLoadFlag = (event.kbd.hasFlags(Common::KBD_ALT)) ? 1 : 2;
			_saveTemporaryState = false;
		} else if (event.kbd.hasFlags(Common::KBD_CTRL) && event.kbd.keycode == Common::KEYCODE_f) {
			_fastMode ^= 1;
		} else if (event.kbd.hasFlags(Common::KBD_CTRL) && event.kbd.keycode == Common::KEYCODE_g) {
			_fastMode ^= 2;
		} else if (event.kbd.hasFlags(Common::KBD_CTRL) && event.kbd.keycode == Common::KEYCODE_s) {
			_res->resourceStats();
		} else if (event.kbd.hasFlags(Common::KBD_ALT) && event.kbd.keycode == Common::KEYCODE_x) {
			if (_game.version < 8) {
				if (isUsingOriginalGUI()) {
					_keyPressed = event.kbd;
				} else {
					quitGame();
				}
			}
		} else {
			// Normal key press, pass on to the game.
			_keyPressed = event.kbd;
		}

		// HACK: Because we use ASCII values here, it's necessary to
		// remap keypad keys to always have a corresponding ASCII value.
		// Normally, keypad keys would only have an ASCII value when
		// NumLock is enabled. This fixes fighting in Indy 3 (Trac #11227)
		if (_keyPressed.keycode >= Common::KEYCODE_KP0 && _keyPressed.keycode <= Common::KEYCODE_KP9) {
			_keyPressed.ascii = (_keyPressed.keycode - Common::KEYCODE_KP0) + '0';
		}

		// FIXME: We are using ASCII values to index the _keyDownMap here,
		// yet later one code which checks _keyDownMap will use KEYCODEs
		// to do so. That is, we are mixing ascii and keycode values here,
		// which is bad. We probably should be only using keycodes, but at
		// least INSANE checks for "Shift-V" by looking for the 'V' key
		// being pressed. It would be easy to solve that by also storing
		// the modifier flags. However, since getKeyState() is also called
		// by scripts, we have to be careful with semantic changes.
		if (_keyPressed.ascii >= 512)
			debugC(DEBUG_GENERAL, "_keyPressed > 512 (%d)", _keyPressed.ascii);
		else
			_keyDownMap[_keyPressed.ascii] = true;
		break;

	case Common::EVENT_KEYUP:
		// HACK: Because we use ASCII values here, it's necessary to
		// remap keypad keys to always have a corresponding ASCII value.
		// Normally, keypad keys would only have an ASCII value when
		// NumLock is enabled. This fixes fighting in Indy 3 (Trac #11227)
		if (_keyPressed.keycode >= Common::KEYCODE_KP0 && _keyPressed.keycode <= Common::KEYCODE_KP9) {
			_keyPressed.ascii = (_keyPressed.keycode - Common::KEYCODE_KP0) + '0';
		}

		if (event.kbd.ascii >= 512) {
			debugC(DEBUG_GENERAL, "keyPressed > 512 (%d)", event.kbd.ascii);
		} else {
			_keyDownMap[event.kbd.ascii] = false;

			// Due to some weird bug with capslock key pressed
			// generated keydown event is for lower letter but
			// keyup is for upper letter
			// On most (all?) keyboards it is safe to assume that
			// both upper and lower letters are unpressed on keyup event
			//
			// Fixes bug #3173: "FT: CAPSLOCK + V enables cheating for all fights"
			//
			// Fingolfin remarks: This wouldn't be a problem if we used keycodes.
			_keyDownMap[toupper(event.kbd.ascii)] = false;
		}
		break;


	// We update the mouse position whenever the mouse moves or a click occurs.
	// The latter is done to accomodate systems with a touchpad / pen controller.
	case Common::EVENT_LBUTTONDOWN:
	case Common::EVENT_RBUTTONDOWN:
	case Common::EVENT_MOUSEMOVE:
		if (event.type == Common::EVENT_LBUTTONDOWN)
			_leftBtnPressed |= msClicked|msDown;
		else if (event.type == Common::EVENT_RBUTTONDOWN)
			_rightBtnPressed |= msClicked|msDown;
		_mouse.x = event.mouse.x;
		_mouse.y = event.mouse.y;

		if (_renderMode == Common::kRenderHercA || _renderMode == Common::kRenderHercG) {
			_mouse.x -= (kHercWidth - _screenWidth * 2) / 2;
			_mouse.x >>= 1;
			if (_game.version < 3) {
				// MM/ZAK v1/v2
				if (_mouse.y >= _virtscr[kMainVirtScreen].topline)
					_mouse.y = _mouse.y / 2 + _virtscr[kMainVirtScreen].topline / 2;
				if (_mouse.y > _virtscr[kVerbVirtScreen].topline)
					_mouse.y += (_mouse.y - _virtscr[kVerbVirtScreen].topline);
			} else {
				// MI1
				_mouse.y = _mouse.y * 4 / 7;
			}

		} else if (_macScreen || (_useCJKMode && _textSurfaceMultiplier == 2) || _renderMode == Common::kRenderCGA_BW || _enableEGADithering) {
			_mouse.x >>= 1;
			_mouse.y >>= 1;
		}
		break;
	case Common::EVENT_LBUTTONUP:
		_leftBtnPressed &= ~msDown;
		break;

	case Common::EVENT_RBUTTONUP:
		_rightBtnPressed &= ~msDown;
		break;

	// The following two cases enable dialog choices to be scrolled
	// through in the SegaCD version of MI. Values are taken from script-14.
	// See bug report #2013 for details.
	case Common::EVENT_WHEELDOWN:
		if (_game.id == GID_MONKEY && _game.platform == Common::kPlatformSegaCD)
			_keyPressed = Common::KeyState(Common::KEYCODE_7, 55);	// '7'
		if (_mainMenuIsActive)
			_mouseWheelFlag = Common::EVENT_WHEELDOWN;
		break;

	case Common::EVENT_WHEELUP:
		if (_game.id == GID_MONKEY && _game.platform == Common::kPlatformSegaCD)
			_keyPressed = Common::KeyState(Common::KEYCODE_6, 54);	// '6'
		if (_mainMenuIsActive)
			_mouseWheelFlag = Common::EVENT_WHEELUP;
		break;

	case Common::EVENT_MAINMENU:
		// Let the user open the GMM when the menu is active.
		if (isUsingOriginalGUI() && _mainMenuIsActive)
			openMainMenuDialog();
		break;
	case Common::EVENT_RETURN_TO_LAUNCHER:
	case Common::EVENT_QUIT:
		if (isUsingOriginalGUI()) {
			if (!_quitByGUIPrompt && !_mainMenuIsActive) {
				bool exitType = (event.type == Common::EVENT_RETURN_TO_LAUNCHER);
				// If another message banner is currently on the screen, close it
				// and then execute the quit prompt. Otherwise, prompt the user.
				getEventManager()->resetQuit();
				getEventManager()->resetReturnToLauncher();
				if (!_messageBannerActive) {
					queryQuit(exitType);
					_closeBannerAndQueryQuitFlag = false;
				} else {
					_closeBannerAndQueryQuitFlag = true;
				}
			} else if (_quitByGUIPrompt) {
				if (!getEventManager()->shouldReturnToLauncher())
					quitGame();
			}
		}
		break;
	default:
		break;
	}
}

void ScummEngine::parseEvents() {
	Common::Event event;

	while (_eventMan->pollEvent(event)) {
		parseEvent(event);
	}
}

#ifdef ENABLE_HE
void ScummEngine_v90he::clearClickedStatus() {
	ScummEngine::clearClickedStatus();
	if (_game.heversion >= 98) {
		_logicHE->processKeyStroke(_keyPressed.ascii);
	}
}

void ScummEngine_v90he::processInput() {
	if (_game.heversion >= 98) {
		_logicHE->processKeyStroke(_keyPressed.ascii);
	}
	ScummEngine::processInput();
}
#endif

void ScummEngine::clearClickedStatus() {
	_keyPressed.reset();

	_mouseAndKeyboardStat = 0;
	_leftBtnPressed &= ~msClicked;
	_rightBtnPressed &= ~msClicked;
	_mouseWheelFlag = 0;
}

void ScummEngine_v0::processInput() {
	// F1 - F3
	if (_keyPressed.keycode >= Common::KEYCODE_F1 && _keyPressed.keycode <= Common::KEYCODE_F3) {
		switchActor(_keyPressed.keycode - Common::KEYCODE_F1);
	}

	ScummEngine::processInput();
}

#ifdef ENABLE_SCUMM_7_8
void ScummEngine_v7::processInput() {
	ScummEngine::processInput();

	if (_skipVideo && !_smushActive) {
		abortCutscene();
		_mouseAndKeyboardStat = Common::ASCII_ESCAPE;
		_skipVideo = false;
	}
}
#endif

void ScummEngine::processInput() {
	Common::KeyState lastKeyHit = _keyPressed;
	_keyPressed.reset();

	//
	// Clip the mouse coordinates, and compute _virtualMouse.x (and clip it, too)
	//
	if (_mouse.x < 0)
		_mouse.x = 0;
	if (_mouse.x > _screenWidth-1)
		_mouse.x = _screenWidth-1;
	if (_mouse.y < 0)
		_mouse.y = 0;
	if (_mouse.y > _screenHeight-1)
		_mouse.y = _screenHeight-1;

	VirtScreen *vs = &_virtscr[kMainVirtScreen];
	_virtualMouse.x = _mouse.x + vs->xstart;
	_virtualMouse.y = _mouse.y - vs->topline;
	if (_game.version >= 7)
		_virtualMouse.y += _screenTop;

	if (_virtualMouse.y < 0)
		_virtualMouse.y = -1;
	if (_virtualMouse.y >= vs->h)
		_virtualMouse.y = -1;

	//
	// Determine the mouse button state.
	//
	_mouseAndKeyboardStat = 0;

	if ((_leftBtnPressed & msClicked) && (_rightBtnPressed & msClicked) && _game.version >= 4) {
		// Pressing both mouse buttons is treated as if you pressed
		// the cutscene exit key (ESC) in V4+ games. That mimicks
		// the behavior of the original engine where pressing both
		// mouse buttons also skips the current cutscene.
		_mouseAndKeyboardStat = 0;
		lastKeyHit = Common::KeyState(Common::KEYCODE_ESCAPE);
	} else if ((_rightBtnPressed & msClicked) && (_game.version <= 3 && _game.id != GID_LOOM)) {
		// Pressing right mouse button is treated as if you pressed
		// the cutscene exit key (ESC) in V0-V3 games. That mimicks
		// the behavior of the original engine where pressing right
		// mouse button also skips the current cutscene.
		_mouseAndKeyboardStat = 0;
		lastKeyHit = Common::KeyState(Common::KEYCODE_ESCAPE);
	} else if (_leftBtnPressed & msClicked) {
		_mouseAndKeyboardStat = MBS_LEFT_CLICK;
	} else if (_rightBtnPressed & msClicked) {
		_mouseAndKeyboardStat = MBS_RIGHT_CLICK;
	}

	if (_game.version >= 6) {
		VAR(VAR_LEFTBTN_HOLD) = (_leftBtnPressed & msDown) != 0;
		VAR(VAR_RIGHTBTN_HOLD) = (_rightBtnPressed & msDown) != 0;

		if (_game.heversion >= 72) {
			// HE72 introduced a flag for whether or not this is a click
			// or the player is continuing to hold the button down.
			// 0x80 signifies that the button is continuing to be held down
			// Backyard Soccer needs this in order to function
			if (VAR(VAR_LEFTBTN_HOLD) && !(_leftBtnPressed & msClicked))
				VAR(VAR_LEFTBTN_HOLD) |= 0x80;

			if (VAR(VAR_RIGHTBTN_HOLD) && !(_rightBtnPressed & msClicked))
				VAR(VAR_RIGHTBTN_HOLD) |= 0x80;
		} else if (_game.version >= 7) {
			VAR(VAR_LEFTBTN_DOWN) = (_leftBtnPressed & msClicked) != 0;
			VAR(VAR_RIGHTBTN_DOWN) = (_rightBtnPressed & msClicked) != 0;

			// WORKAROUND: In COMI main menu, sometimes clicks are not registered
			// correctly; in particular there usually a situation in which both
			// states for a mouse button (msClicked and msDown) are being captured.
			// Apparently this is not what the script expects: it asks for the
			// mouse button being held down (why?) but not clicked (again, why?).
			//
			// Instead of mangling our very sturdy input system just for this use
			// case, we just insert this little hack to make sure that the menu
			// works as intended. After all, this all just might be caused by the
			// difference between the (very slow...) mouse polling rate of the original
			// and ours.
			if (isUsingOriginalGUI() && _game.id == GID_CMI && _currentRoom == 92) {
				VAR(VAR_LEFTBTN_HOLD) = (_leftBtnPressed & msDown) != 0;
				VAR(VAR_RIGHTBTN_HOLD) = (_rightBtnPressed & msDown) != 0;
				VAR(VAR_LEFTBTN_DOWN) = 0;
				VAR(VAR_RIGHTBTN_DOWN) = 0;
			}
		}
	}

	_leftBtnPressed &= ~msClicked;
	_rightBtnPressed &= ~msClicked;

	if (!lastKeyHit.ascii)
		return;

	processKeyboard(lastKeyHit);
}

#ifdef ENABLE_SCUMM_7_8
void ScummEngine_v8::processKeyboard(Common::KeyState lastKeyHit) {
	if (isUsingOriginalGUI()) {
		if (lastKeyHit.keycode == Common::KEYCODE_INVALID)
			return;

		// Main menu (allow both F1 and F5) mapping; unlike in
		// the past we choose not to enable it without the
		// original GUI selected as an option
		if (!(_game.features & GF_DEMO) &&
			(lastKeyHit.keycode == Common::KEYCODE_F1 || lastKeyHit.keycode == Common::KEYCODE_F5) &&
			lastKeyHit.hasFlags(0)) {
			lastKeyHit = Common::KeyState(Common::KEYCODE_F1, 315);
		}

		// Actually show the main menu screen...
		if (!(_game.features & GF_DEMO) && _keyScriptNo && (_keyScriptKey == lastKeyHit.ascii)) {
			if (!isSmushActive())
				runScript(_keyScriptNo, 0, 0, 0);
			return;
		}
	}

	// F1 (the trigger for the original save/load dialog) is mapped to F5
	if (!(_game.features & GF_DEMO) && lastKeyHit.keycode == Common::KEYCODE_F1 && lastKeyHit.hasFlags(0)) {
		lastKeyHit = Common::KeyState(Common::KEYCODE_F5, 319);
	}

	// Fall back to V7 behavior...
	ScummEngine_v7::processKeyboard(lastKeyHit);

}

void ScummEngine_v7::processKeyboard(Common::KeyState lastKeyHit) {
	if (isUsingOriginalGUI()) {
		if (lastKeyHit.keycode == Common::KEYCODE_b && lastKeyHit.hasFlags(Common::KBD_CTRL)) {
			int curBufferCount = _imuseDigital->roundRobinSetBufferCount();
			// "iMuse buffer count changed to %d"
			showBannerAndPause(0, 90, getGUIString(gsIMuseBuffer), curBufferCount);
			return;
		}
	}

	const bool cutsceneExitKeyEnabled = (VAR_CUTSCENEEXIT_KEY == 0xFF || VAR(VAR_CUTSCENEEXIT_KEY) != 0);

	// VAR_VERSION_KEY (usually ctrl-v) is used in COMI, Dig and FT to trigger
	// a version dialog, unless VAR_VERSION_KEY is set to 0. However, the COMI
	// version string is hard coded in the engine, hence we don't invoke
	// versionDialog for it. Dig/FT version strings are partly hard coded, too.
	if (!isUsingOriginalGUI() && _game.id != GID_CMI && 0 != VAR(VAR_VERSION_KEY) &&
	    lastKeyHit.keycode == Common::KEYCODE_v && lastKeyHit.hasFlags(Common::KBD_CTRL)) {
		versionDialog();

	} else if (cutsceneExitKeyEnabled && lastKeyHit.keycode == Common::KEYCODE_ESCAPE) {
		// Skip cutscene (or active SMUSH video).
		if (_smushActive) {
			if (_game.id == GID_FT)
				_insane->escapeKeyHandler();
			else
				_smushVideoShouldFinish = true;

			// WORKAROUND bug #12022: For some reason, skipping the cutscene in which Ben fires up
			// his bike (after retrieving the keys from the bartender), will outright skip the first
			// bike fight sequence. Because of this, the script which handles playing ambient and wind SFX
			// outside the bar is never stopped, so those SFX are unintentionally played throughout the
			// rest of the game.
			// This fix produces the intended behaviour from the original interpreter.
			if (_game.id == GID_FT && _currentRoom == 6
				&& (vm.slot[_currentScript].number == 65 || vm.slot[_currentScript].number == 64)) {
				_skipVideo = false;
			} else {
				_skipVideo = true;
			}
		} else {
			abortCutscene();
		}

		_mouseAndKeyboardStat = Common::ASCII_ESCAPE;

	} else {
		// Fall back to V6 behavior
		ScummEngine_v6::processKeyboard(lastKeyHit);
	}
}
#endif


void ScummEngine::waitForBannerInput(int32 waitTime, Common::KeyState &ks, bool &leftBtnClicked, bool &rightBtnClicked, bool handeleMouseWheel) {
	bool validKey = false;

	if (waitTime && waitTime != -1) {
		uint32 millis = _system->getMillis();
		while (((_system->getMillis() - millis) * (_timerFrequency / 4) / 1000) < waitTime) {
			waitForTimer(1); // Allow the engine to update the screen and fetch new inputs...

			if (_game.version < 7 && (_guiCursorAnimCounter++ & 16)) {
				_guiCursorAnimCounter = 0;
				animateCursor();
			}

			ks = _keyPressed;
			leftBtnClicked = (_leftBtnPressed & msClicked) != 0;
			rightBtnClicked = (_rightBtnPressed & msClicked) != 0;

			validKey = ks.keycode != Common::KEYCODE_INVALID &&
					   ks.keycode != Common::KEYCODE_LALT    &&
					   ks.keycode != Common::KEYCODE_RALT    &&
					   !(ks.keycode == Common::KEYCODE_s && ks.hasFlags(Common::KBD_ALT));

			if (validKey || leftBtnClicked || rightBtnClicked || (handeleMouseWheel && _mouseWheelFlag))
				return;

			if (shouldQuit())
				return;

			if (_closeBannerAndQueryQuitFlag) {
				_closeBannerAndQueryQuitFlag = false;
				Common::Event event;
				event.type = Common::EVENT_QUIT;
				getEventManager()->pushEvent(event);
				return;
			}
		}
	} else {
		while (!validKey && !leftBtnClicked && !rightBtnClicked && !(handeleMouseWheel && _mouseWheelFlag)) {
			waitForTimer(1); // Allow the engine to update the screen and fetch new inputs...

			if (_game.version < 7 && (_guiCursorAnimCounter++ & 16)) {
				_guiCursorAnimCounter = 0;
				animateCursor();
			}

			ks = _keyPressed;
			leftBtnClicked = (_leftBtnPressed & msClicked) != 0;
			rightBtnClicked = (_rightBtnPressed & msClicked) != 0;

			if (shouldQuit())
				return;

			if (_closeBannerAndQueryQuitFlag && !_comiQuitMenuIsOpen) {
				_closeBannerAndQueryQuitFlag = false;
				Common::Event event;
				event.type = Common::EVENT_QUIT;
				getEventManager()->pushEvent(event);
				return;
			}

 			validKey = ks.keycode != Common::KEYCODE_INVALID &&
					   ks.keycode != Common::KEYCODE_LALT    &&
					   ks.keycode != Common::KEYCODE_RALT    &&
					   ks.keycode != Common::KEYCODE_LCTRL   &&
					   ks.keycode != Common::KEYCODE_RCTRL   &&
					   ks.keycode != Common::KEYCODE_LSHIFT  &&
					   ks.keycode != Common::KEYCODE_RSHIFT  &&
					   ks.keycode != Common::KEYCODE_UP      &&
					   ks.keycode != Common::KEYCODE_DOWN    &&
					   ks.keycode != Common::KEYCODE_LEFT    &&
					   ks.keycode != Common::KEYCODE_RIGHT   &&
					   !(ks.keycode == Common::KEYCODE_s && ks.hasFlags(Common::KBD_ALT));
		}
	}
}

void ScummEngine_v6::processKeyboard(Common::KeyState lastKeyHit) {
	if (isUsingOriginalGUI()) {
		char sliderString[256];
		PauseToken pt;

		if (_game.version != 8 || (_game.version == 8 && (_game.features & GF_DEMO))) {
			// "Music Volume  Low  =========  High"
			if (lastKeyHit.hasFlags(Common::KBD_SHIFT) &&
				(lastKeyHit.keycode == Common::KEYCODE_o || lastKeyHit.keycode == Common::KEYCODE_p)) {
				Common::KeyState ks = lastKeyHit;

				pt = pauseEngine();

				int volume = (_game.version > 6) ? _imuseDigital->diMUSEGetMusicGroupVol() : getMusicVolume();
				do {
					if (ks.keycode == Common::KEYCODE_o) {
						volume -= 16;
						if (volume < 0)
							volume = 0;
					} else {
						volume += 16;
						if (volume > 127)
							volume = 127;
					}

					getSliderString(gsMusicVolumeSlider, volume, sliderString, sizeof(sliderString));
					showBannerAndPause(0, 0, sliderString);
					ks = Common::KEYCODE_INVALID;
					bool leftBtnPressed = false, rightBtnPressed = false;
					waitForBannerInput(60, ks, leftBtnPressed, rightBtnPressed);
				} while (ks.keycode == Common::KEYCODE_o || ks.keycode == Common::KEYCODE_p);
				clearBanner();

				setMusicVolume(volume);

				pt.clear();

				return;
			}

			// "Voice Volume  Low  =========  High"
			if (lastKeyHit.hasFlags(Common::KBD_SHIFT) &&
				(lastKeyHit.keycode == Common::KEYCODE_k || lastKeyHit.keycode == Common::KEYCODE_l)) {
				Common::KeyState ks = lastKeyHit;

				pt = pauseEngine();

				int volume = (_game.version > 6) ? _imuseDigital->diMUSEGetVoiceGroupVol() : getSpeechVolume();
				do {
					if (ks.keycode == Common::KEYCODE_k) {
						volume -= 16;
						if (volume < 0)
							volume = 0;
					} else {
						volume += 16;
						if (volume > 127)
							volume = 127;
					}

					getSliderString(gsVoiceVolumeSlider, volume, sliderString, sizeof(sliderString));
					showBannerAndPause(0, 0, sliderString);
					ks = Common::KEYCODE_INVALID;
					bool leftBtnPressed = false, rightBtnPressed = false;
					waitForBannerInput(60, ks, leftBtnPressed, rightBtnPressed);
				} while (ks.keycode == Common::KEYCODE_k || ks.keycode == Common::KEYCODE_l);
				clearBanner();

				setSpeechVolume(volume);

				pt.clear();

				return;
			}

			// "Sfx Volume  Low  =========  High"
			if (lastKeyHit.hasFlags(Common::KBD_SHIFT) &&
				(lastKeyHit.keycode == Common::KEYCODE_n || lastKeyHit.keycode == Common::KEYCODE_m)) {
				Common::KeyState ks = lastKeyHit;

				pt = pauseEngine();

				int volume = (_game.version > 6) ? _imuseDigital->diMUSEGetSFXGroupVol() : getSFXVolume();
				do {
					if (ks.keycode == Common::KEYCODE_n) {
						volume -= 16;
						if (volume < 0)
							volume = 0;
					} else {
						volume += 16;
						if (volume > 127)
							volume = 127;
					}

					getSliderString(gsSfxVolumeSlider, volume, sliderString, sizeof(sliderString));
					showBannerAndPause(0, 0, sliderString);
					ks = Common::KEYCODE_INVALID;
					bool leftBtnPressed = false, rightBtnPressed = false;
					waitForBannerInput(60, ks, leftBtnPressed, rightBtnPressed);
				} while (ks.keycode == Common::KEYCODE_n || ks.keycode == Common::KEYCODE_m);
				clearBanner();

				setSFXVolume(volume);

				pt.clear();

				return;
			}
		}

		if (VAR_VERSION_KEY != 0xFF && VAR(VAR_VERSION_KEY) != 0 &&
			lastKeyHit.keycode == Common::KEYCODE_v && lastKeyHit.hasFlags(Common::KBD_CTRL)) {
			if (_game.version == 8) {
				showBannerAndPause(0, -1, _dataFileVersionString);
				// This is not the string being used by the interpreter, which is instead hardcoded
				// in the executable. The one used here is found in the data files.
				showBannerAndPause(0, -1, _engineVersionString);

				if (_game.features & GF_DEMO)
					showBannerAndPause(0, -1, "iMuse(tm) initialized");
				return;
			} else if (_game.version == 7) {
				showBannerAndPause(0, -1, getGUIString(gsVersion));
				showBannerAndPause(0, -1, "Scripts compiled %s", _dataFileVersionString);
				showBannerAndPause(0, -1, "SPU(tm) version %s", _engineVersionString);
				showBannerAndPause(0, -1, "iMuse(tm) initialized");
				return;
			}
			// Older versions show this banner via scripts, so just let it fallback...
		}
	} else {
		if (lastKeyHit.keycode == Common::KEYCODE_t && lastKeyHit.hasFlags(Common::KBD_CTRL)) {
			SubtitleSettingsDialog dialog(this, _voiceMode);
			_voiceMode = runDialog(dialog);

			switch (_voiceMode) {
			case 0:
				ConfMan.setBool("speech_mute", false);
				ConfMan.setBool("subtitles", false);
				break;
			case 1:
				ConfMan.setBool("speech_mute", false);
				ConfMan.setBool("subtitles", true);
				break;
			case 2:
				ConfMan.setBool("speech_mute", true);
				ConfMan.setBool("subtitles", true);
				break;
			default:
				break;
			}

			// We need to sync the current sound settings here to make sure that
			// we actually update the mute state of speech properly.
			syncSoundSettings();

			return;
		}
	}

	// Fall back to default behavior
	ScummEngine::processKeyboard(lastKeyHit);
}

void ScummEngine_v2::processKeyboard(Common::KeyState lastKeyHit) {
	// RETURN is used to skip cutscenes in the Commodore 64 version of Zak McKracken
	if (_game.id == GID_ZAK &&_game.platform == Common::kPlatformC64 && lastKeyHit.keycode == Common::KEYCODE_RETURN && lastKeyHit.hasFlags(0)) {
		lastKeyHit = Common::KeyState(Common::KEYCODE_ESCAPE);
	// F7 is used to skip cutscenes in the Commodore 64 version of Maniac Mansion
	} else if (_game.id == GID_MANIAC &&_game.platform == Common::kPlatformC64) {
		// Demo always F7 to be pressed to restart
		if (_game.features & GF_DEMO) {
			if (_roomResource != 0x2D && lastKeyHit.keycode == Common::KEYCODE_F7 && lastKeyHit.hasFlags(0)) {
				restart();
				return;
			}
		} else {
			if (lastKeyHit.keycode == Common::KEYCODE_F7 && lastKeyHit.hasFlags(0))
				lastKeyHit = Common::KeyState(Common::KEYCODE_ESCAPE);
		}
	// 'B' is used to skip cutscenes in the NES version of Maniac Mansion
	} else if (_game.id == GID_MANIAC &&_game.platform == Common::kPlatformNES) {
		if (lastKeyHit.keycode == Common::KEYCODE_b && lastKeyHit.hasFlags(Common::KBD_SHIFT))
			lastKeyHit = Common::KeyState(Common::KEYCODE_ESCAPE);
	// 'F4' is used to skip cutscenes in the other versions of Maniac Mansion
	} else if (_game.id == GID_MANIAC) {
		if (lastKeyHit.keycode == Common::KEYCODE_F4 && lastKeyHit.hasFlags(0))
			lastKeyHit = Common::KeyState(Common::KEYCODE_ESCAPE);
	}

	// Fall back to default behavior
	ScummEngine::processKeyboard(lastKeyHit);

	// On Alt-F5 prepare savegame for the original save/load dialog.
	if (lastKeyHit.keycode == Common::KEYCODE_F5 && lastKeyHit.hasFlags(Common::KBD_ALT)) {
		prepareSavegame();
		if (_game.id == GID_MANIAC && _game.version == 0) {
			runScript(2, 0, 0, nullptr);
		}
		if (_game.id == GID_MANIAC &&_game.platform == Common::kPlatformNES) {
			runScript(163, 0, 0, nullptr);
		}
	}

	if (VAR_KEYPRESS != 0xFF && _mouseAndKeyboardStat) {		// Key Input
		if (315 <= _mouseAndKeyboardStat && _mouseAndKeyboardStat <= 323) {
			// Convert F-Keys for V1/V2 games (they start at 1)
			VAR(VAR_KEYPRESS) = _mouseAndKeyboardStat - 314;
		} else {
			VAR(VAR_KEYPRESS) = _mouseAndKeyboardStat;
		}
	}
}

void ScummEngine_v3::processKeyboard(Common::KeyState lastKeyHit) {
	// Fall back to default behavior
	ScummEngine::processKeyboard(lastKeyHit);

	// On Alt-F5 prepare savegame for the original save/load dialog.
	if (lastKeyHit.keycode == Common::KEYCODE_F5 && lastKeyHit.hasFlags(Common::KBD_ALT)) {
		prepareSavegame();
	}

	// 'i' brings up an IQ dialog in Indy3 (disabled in save/load dialog for input)
	if (lastKeyHit.ascii == 'i' && _game.id == GID_INDY3 && _currentRoom != 14) {
		// SCUMM var 244 is the episode score
		// and var 245 is the series score
		char text[50];

		updateIQPoints();

		sprintf(text, "IQ Points: Episode = %d, Series = %d", _scummVars[244], _scummVars[245]);
		Indy3IQPointsDialog indy3IQPointsDialog(this, text);
		runDialog(indy3IQPointsDialog);
	}
}

void ScummEngine::processKeyboard(Common::KeyState lastKeyHit) {
	// Enable the following special keys conditionally:
	bool restartKeyEnabled = (VAR_RESTART_KEY == 0xFF || VAR(VAR_RESTART_KEY) != 0);
	bool pauseKeyEnabled = (VAR_PAUSE_KEY == 0xFF || VAR(VAR_PAUSE_KEY) != 0);
	bool talkstopKeyEnabled = (VAR_TALKSTOP_KEY == 0xFF || VAR(VAR_TALKSTOP_KEY) != 0);
	bool cutsceneExitKeyEnabled = (VAR_CUTSCENEEXIT_KEY == 0xFF || VAR(VAR_CUTSCENEEXIT_KEY) != 0);
	bool mainmenuKeyEnabled = (VAR_MAINMENU_KEY == 0xFF || VAR(VAR_MAINMENU_KEY) != 0);
	bool snapScrollKeyEnabled = (_game.version >= 2 && _game.version <= 4);
	bool optionKeysEnabled = !isUsingOriginalGUI();

	// In FM-TOWNS games F8 / restart is always enabled
	if (_game.platform == Common::kPlatformFMTowns)
		restartKeyEnabled = true;

	if (isUsingOriginalGUI()) {
		char sliderString[256];
		PauseToken pt;

		if ((VAR_PAUSE_KEY != 0xFF && (lastKeyHit.ascii == VAR(VAR_PAUSE_KEY))) ||
			(lastKeyHit.keycode == Common::KEYCODE_SPACE && _game.features & GF_DEMO)) {
			// Force the cursor OFF...
			int8 oldCursorState = _cursor.state;
			_cursor.state = 0;
			CursorMan.showMouse(_cursor.state > 0);
			// "Game Paused.  Press SPACE to Continue."
			if (_game.version > 4)
				showBannerAndPause(0, -1, getGUIString(gsPause));
			else
				showOldStyleBannerAndPause(getGUIString(gsPause), 12, -1);

			_cursor.state = oldCursorState;
			return;
		}

		if ((VAR_RESTART_KEY != 0xFF && (lastKeyHit.ascii == VAR(VAR_RESTART_KEY))) ||
			((_game.id == GID_CMI && !(_game.features & GF_DEMO))
				&& lastKeyHit.keycode == Common::KEYCODE_F8 && lastKeyHit.hasFlags(0))) {
			queryRestart();
			return;
		}

		// Generally allow voice mode settings only for v6, 7 and 8.
		// Also allow it for Indy4 Talkie, which does its own thing.
		if (((VAR_VOICE_MODE != 0xFF) || (_game.variant && (!strcmp(_game.variant, "SE Talkie") ||
			(_game.id == GID_INDY4 && strcmp(_game.variant, "Floppy") && strcmp(_game.variant, "Amiga"))))) &&
			(lastKeyHit.keycode == Common::KEYCODE_t && lastKeyHit.hasFlags(Common::KBD_CTRL))) {
			int voiceMode = (_game.version == 5) ? _v5VoiceMode : VAR(VAR_VOICE_MODE);

			voiceMode++;
			if (voiceMode >= 3) {
				voiceMode = 0;
			}

			if (_game.version == 5)
				_v5VoiceMode = voiceMode;

			switch (voiceMode) {
			case 0: // "Voice Only"
				showBannerAndPause(0, 120, getGUIString(gsVoiceOnly));
				break;
			case 1: // "Voice and Text"
				showBannerAndPause(0, 120, getGUIString(gsVoiceAndText));
				break;
			default: // "Text Display Only"
				showBannerAndPause(0, 120, getGUIString(gsTextDisplayOnly));
			}

			ConfMan.setInt("original_gui_text_status", voiceMode);
			switch (voiceMode) {
			case 0:
				ConfMan.setBool("speech_mute", false);
				ConfMan.setBool("subtitles", false);
				break;
			case 1:
				ConfMan.setBool("speech_mute", false);
				ConfMan.setBool("subtitles", true);
				break;
			case 2:
				ConfMan.setBool("speech_mute", true);
				ConfMan.setBool("subtitles", true);
				break;
			default:
				break;
			}

			ConfMan.flushToDisk();
			syncSoundSettings();
			return;
		}

		// "Text Speed  Slow  ==========  Fast"
		if (_game.version > 3 && (lastKeyHit.ascii == '+' || lastKeyHit.ascii == '-')) {
			if (VAR_CHARINC == 0xFF)
				return;

			Common::KeyState ks = lastKeyHit;

			pt = pauseEngine();

			do {
				if (ks.ascii == '+') {
					VAR(VAR_CHARINC) -= 1;
					if (VAR(VAR_CHARINC) < 0)
						VAR(VAR_CHARINC) = 0;
				} else {
					VAR(VAR_CHARINC) += 1;
					if (VAR(VAR_CHARINC) > 9)
						VAR(VAR_CHARINC) = 9;
				}

				_defaultTextSpeed = 9 - VAR(VAR_CHARINC);
				ConfMan.setInt("original_gui_text_speed", _defaultTextSpeed);
				setTalkSpeed(_defaultTextSpeed);

				getSliderString(gsTextSpeedSlider, VAR(VAR_CHARINC), sliderString, sizeof(sliderString));
				if (_game.version > 4)
					showBannerAndPause(0, 0, sliderString);
				else
					showOldStyleBannerAndPause(sliderString, 9, 0);

				ks = Common::KEYCODE_INVALID;
				bool leftBtnPressed = false, rightBtnPressed = false;
				waitForBannerInput(60, ks, leftBtnPressed, rightBtnPressed);
			} while (ks.ascii == '+' || ks.ascii == '-');
			clearBanner();

			pt.clear();

			return;
		}


		if (_game.version > 4 && lastKeyHit.keycode == Common::KEYCODE_k && lastKeyHit.hasFlags(Common::KBD_CTRL)) {
			showBannerAndPause(0, 120, getGUIString(gsHeap), _res->getHeapSize() / 1024);
			return;
		}

		if ((lastKeyHit.keycode == Common::KEYCODE_c && lastKeyHit.hasFlags(Common::KBD_CTRL)) ||
			(lastKeyHit.keycode == Common::KEYCODE_x && lastKeyHit.hasFlags(Common::KBD_ALT))) {
			Common::Event event;
			event.type = Common::EVENT_QUIT;
			getEventManager()->pushEvent(event);
			return;
		}

		if (VAR_MAINMENU_KEY != 0xFF && (lastKeyHit.ascii == VAR(VAR_MAINMENU_KEY) && lastKeyHit.hasFlags(0))
			&& _game.platform != Common::kPlatformFMTowns) {
			showMainMenu();
			return;
		}

		if (_game.version == 4) {
			if (lastKeyHit.keycode == Common::KEYCODE_r && lastKeyHit.hasFlags(Common::KBD_CTRL)) {
				_snapScroll ^= 1;
				if (_snapScroll) {
					showOldStyleBannerAndPause("Horizontal Screen Snap", 9, 90);
				} else {
					showOldStyleBannerAndPause("Horizontal Screen Scroll", 9, 90);
				}

				if (VAR_CAMERA_FAST_X != 0xFF)
					VAR(VAR_CAMERA_FAST_X) = _snapScroll;

				return;
			}

			// The following ones serve no purpose whatsoever, but just for the sake of completeness...
			// Also, these were originally mapped with the CTRL flag, but they would clash with other
			// internal ScummVM commands, so they are instead available with the SHIFT flag.
			if (lastKeyHit.keycode == Common::KEYCODE_j && lastKeyHit.hasFlags(Common::KBD_SHIFT)) {
				showOldStyleBannerAndPause("Recalibrating Joystick", 2, 90);
				return;
			}

			if (lastKeyHit.keycode == Common::KEYCODE_m && lastKeyHit.hasFlags(Common::KBD_SHIFT)) {
				showOldStyleBannerAndPause("Mouse Mode", 2, 90);
				return;
			}

			if (lastKeyHit.keycode == Common::KEYCODE_s && lastKeyHit.hasFlags(Common::KBD_SHIFT)) {
				_internalSpeakerSoundsAreOn ^= 1;
				if (_internalSpeakerSoundsAreOn) {
					showOldStyleBannerAndPause("Sounds On", 9, 90);
				} else {
					showOldStyleBannerAndPause("Sounds Off", 9, 90);
				}

				return;
			}

		}
	}

	// For games which use VAR_MAINMENU_KEY, disable the mainmenu key if
	// requested by the scripts. We make an exception for COMI (i.e.
	// forcefully always enable it there), as that always disables it.
	if (_game.id == GID_CMI)
		mainmenuKeyEnabled = true;

	if (mainmenuKeyEnabled && (lastKeyHit.keycode == Common::KEYCODE_F5 && lastKeyHit.hasFlags(0))) {
		if (VAR_SAVELOAD_SCRIPT != 0xFF && _currentRoom != 0)
			runScript(VAR(VAR_SAVELOAD_SCRIPT), 0, 0, nullptr);

		openMainMenuDialog();		// Display global main menu

		// reload options
		_enableAudioOverride = ConfMan.getBool("audio_override");

		if (VAR_SAVELOAD_SCRIPT2 != 0xFF && _currentRoom != 0)
			runScript(VAR(VAR_SAVELOAD_SCRIPT2), 0, 0, nullptr);

	} else if (restartKeyEnabled && (lastKeyHit.keycode == Common::KEYCODE_F8 && lastKeyHit.hasFlags(0))) {
		confirmRestartDialog();

	} else if (pauseKeyEnabled && (lastKeyHit.keycode == Common::KEYCODE_SPACE && lastKeyHit.hasFlags(0))) {
		pauseGame();

	} else if (talkstopKeyEnabled && lastKeyHit.ascii == '.') {
		_talkDelay = 0;
		if (_sound->_sfxMode & 2)
			stopTalk();

	} else if (cutsceneExitKeyEnabled && (lastKeyHit.keycode == Common::KEYCODE_ESCAPE && lastKeyHit.hasFlags(0))) {
		abortCutscene();

		// VAR_CUTSCENEEXIT_KEY doesn't exist in SCUMM0
		if (VAR_CUTSCENEEXIT_KEY != 0xFF)
			_mouseAndKeyboardStat = VAR(VAR_CUTSCENEEXIT_KEY);
	} else if (snapScrollKeyEnabled && lastKeyHit.keycode == Common::KEYCODE_r &&
		lastKeyHit.hasFlags(Common::KBD_CTRL)) {
		_snapScroll ^= 1;
		if (_snapScroll) {
			messageDialog(_("Snap scroll on"));
		} else {
			messageDialog(_("Snap scroll off"));
		}

		if (VAR_CAMERA_FAST_X != 0xFF)
			VAR(VAR_CAMERA_FAST_X) = _snapScroll;
	} else if (optionKeysEnabled && (lastKeyHit.ascii == '[' || lastKeyHit.ascii == ']')) { // Change music volume
		int vol = ConfMan.getInt("music_volume") / 16;
		if (lastKeyHit.ascii == ']' && vol < 16)
			vol++;
		else if (lastKeyHit.ascii == '[' && vol > 0)
			vol--;

		// Display the music volume
		ValueDisplayDialog dlg(_("Music volume: "), 0, 16, vol, ']', '[');
		vol = runDialog(dlg);

		vol *= 16;
		if (vol > Audio::Mixer::kMaxMixerVolume)
			vol = Audio::Mixer::kMaxMixerVolume;

		ConfMan.setInt("music_volume", vol);
		syncSoundSettings();

	} else if (optionKeysEnabled && (lastKeyHit.ascii == '-' || lastKeyHit.ascii == '+')) { // Change text speed
		if (lastKeyHit.ascii == '-' && _defaultTextSpeed > 0)
			_defaultTextSpeed--;
		else if (lastKeyHit.ascii == '+' && _defaultTextSpeed < 9)
			_defaultTextSpeed++;

		// Display the talk speed
		ValueDisplayDialog dlg(_("Subtitle speed: "), 0, 9, _defaultTextSpeed, '+', '-');
		_defaultTextSpeed = runDialog(dlg);

		// Save the new talkspeed value to ConfMan
		setTalkSpeed(_defaultTextSpeed);

		if (VAR_CHARINC != 0xFF)
			VAR(VAR_CHARINC) = 9 - _defaultTextSpeed;

	} else {

		if (lastKeyHit.keycode >= Common::KEYCODE_F1 &&
		    lastKeyHit.keycode <= Common::KEYCODE_F9) {
			_mouseAndKeyboardStat = lastKeyHit.keycode - Common::KEYCODE_F1 + 315;

		} else if (lastKeyHit.flags & Common::KBD_CTRL && _game.version >= 3 && _game.version <= 7 &&
				   (lastKeyHit.keycode >= Common::KEYCODE_a && lastKeyHit.keycode <= Common::KEYCODE_z)) {
			// Some games (at least their DOS variants)
			// expect Ctrl+A, B, C, etc. to generate codes 1, 2, 3, etc.
			//
			// This is used for several settings in the "ultimate talkie" versions of
			// Monkey Island 1 and 2. Monkey Island 1 also uses it for Ctrl+W to immediately
			// win the game. On other games, Ctrl+I shows the inventory, Ctrl+V shows version
			// information. On The Dig Ctrl+B makes Boston display his muscles.
			_mouseAndKeyboardStat = lastKeyHit.keycode & 0x1f;
		} else if (_game.id == GID_MONKEY2 && (lastKeyHit.flags & Common::KBD_ALT)) {
			// Handle KBD_ALT combos in MI2. We know that the result must be 273 for Alt-W
			// because that's what MI2 looks for in its "instant win" cheat.
			_mouseAndKeyboardStat = lastKeyHit.keycode + 154;

		} else if (lastKeyHit.keycode >= Common::KEYCODE_UP &&
		          lastKeyHit.keycode <= Common::KEYCODE_LEFT) {
			if (_game.id == GID_MONKEY && _game.platform == Common::kPlatformSegaCD) {
				// Map arrow keys to number keys in the SEGA version of MI to support
				// scrolling to conversation choices. See bug report #2013 for details.
				_mouseAndKeyboardStat = lastKeyHit.keycode - Common::KEYCODE_UP + 54;
			} else if (isUsingOriginalGUI() || (_game.id == GID_LOOM && _game.platform == Common::kPlatformPCEngine)) {
				// Map arrow keys to number keys in games which use the original menu screen.
				switch (lastKeyHit.keycode) {
				case Common::KEYCODE_UP:
					_mouseAndKeyboardStat = 328;
					break;
				case Common::KEYCODE_DOWN:
					_mouseAndKeyboardStat = 336;
					break;
				case Common::KEYCODE_LEFT:
					_mouseAndKeyboardStat = 331;
					break;
				case Common::KEYCODE_RIGHT:
					_mouseAndKeyboardStat = 333;
					break;
				default:
					break;
				}
			} else if (_game.version >= 7) {
				// Don't let pre-V7 game see arrow keys. This fixes bug with up arrow (273)
				// corresponding to the "instant win" cheat in MI2 mentioned above.
				//
				// This is not applicable to V7+ games, which need to see the arrow keys,
				// too, else certain things (derby scene, asterorid lander) won't work.
				_mouseAndKeyboardStat = lastKeyHit.ascii;
			}

		} else {
			// Map the DEL key when using the original GUI; used when writing the savegame name.
			if (isUsingOriginalGUI() && lastKeyHit.keycode == Common::KEYCODE_DELETE)
				_mouseAndKeyboardStat = 339;
			else
				_mouseAndKeyboardStat = lastKeyHit.ascii;
		}
	}

	// The original interpreters allowed the usage of the Enter key as a substitute for left mouse click,
	// and the Tab key as a substitute for right click; while v7-8 games handle this substitution via
	// scripts, we have to do this manually for the other games.
	if (_mouseAndKeyboardStat == Common::KEYCODE_RETURN && _cursor.state > 0 && _game.version <= 6) {
		_mouseAndKeyboardStat = MBS_LEFT_CLICK;
	} else if (_mouseAndKeyboardStat == Common::KEYCODE_TAB && _cursor.state > 0 && _game.version >= 4 && _game.version <= 6) {
		_mouseAndKeyboardStat = MBS_RIGHT_CLICK;
	}
}

} // End of namespace Scumm
