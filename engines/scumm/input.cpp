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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/stdafx.h"

#include "common/config-manager.h"
#include "common/events.h"
#include "common/system.h"

#include "gui/message.h"
#include "gui/newgui.h"

#include "scumm/debugger.h"
#include "scumm/dialogs.h"
#include "scumm/insane/insane.h"
#include "scumm/imuse/imuse.h"
#ifndef DISABLE_HE
#include "scumm/he/intern_he.h"
#include "scumm/he/logic_he.h"
#endif
#include "scumm/scumm.h"
#include "scumm/sound.h"
#include "scumm/intern.h"


#ifdef _WIN32_WCE
#define		KEY_ALL_SKIP	3457
#endif

namespace Scumm {

enum MouseButtonStatus {
	msDown = 1,
	msClicked = 2
};

void ScummEngine::parseEvents() {
	Common::Event event;

	while (_eventMan->pollEvent(event)) {

		switch (event.type) {
		case Common::EVENT_KEYDOWN:
			if (event.kbd.keycode >= '0' && event.kbd.keycode <= '9'
				&& (event.kbd.flags == Common::KBD_ALT ||
					event.kbd.flags == Common::KBD_CTRL)) {
				_saveLoadSlot = event.kbd.keycode - '0';

				//  don't overwrite autosave (slot 0)
				if (_saveLoadSlot == 0)
					_saveLoadSlot = 10;

				sprintf(_saveLoadName, "Quicksave %d", _saveLoadSlot);
				_saveLoadFlag = (event.kbd.flags == Common::KBD_ALT) ? 1 : 2;
				_saveTemporaryState = false;
			} else if (event.kbd.flags == Common::KBD_CTRL) {
				if (event.kbd.keycode == 'f')
					_fastMode ^= 1;
				else if (event.kbd.keycode == 'g')
					_fastMode ^= 2;
				else if (event.kbd.keycode == 'd')
					_debugger->attach();
				else if (event.kbd.keycode == 's')
					_res->resourceStats();
				else
					_keyPressed = event.kbd;	// Normal key press, pass on to the game.
			} else if (event.kbd.flags & Common::KBD_ALT) {
				// The result must be 273 for Alt-W
				// because that's what MI2 looks for in
				// its "instant win" cheat.
				// FIXME: Handle this specific property inside processKeyboard ?
				_keyPressed = event.kbd;
				_keyPressed.ascii = event.kbd.keycode + 154;
			} else if (event.kbd.keycode < Common::KEYCODE_UP || event.kbd.keycode > Common::KEYCODE_LEFT || _game.version >= 7) {
				// FIXME: Handle this specific property inside processKeyboard ?

				// don't let game have arrow keys as we currently steal them
				// for keyboard cursor control
				// this fixes bug with up arrow (273) corresponding to
				// "instant win" cheat in MI2 mentioned above
				//
				// This is not applicable to Full Throttle as it processes keyboard
				// cursor control by itself. Also it fixes derby scene
				_keyPressed = event.kbd;	// Normal key press, pass on to the game.
			}

			if (_game.id == GID_MONKEY && _game.platform == Common::kPlatformSegaCD) {
				if (event.kbd.keycode >= Common::KEYCODE_UP && event.kbd.keycode <= Common::KEYCODE_LEFT) {
					_keyPressed = event.kbd;
					_keyPressed.ascii = event.kbd.ascii - Common::KEYCODE_UP + 54;
				}
			}

			if (_game.heversion >= 80) {
				// Keyboard is controlled via variable
				int _keyState = 0;

				if (event.kbd.keycode == Common::KEYCODE_LEFT) // Left
					_keyState = 1;

				if (event.kbd.keycode == Common::KEYCODE_RIGHT) // Right
					_keyState |= 2;

				if (event.kbd.keycode == Common::KEYCODE_UP) // Up
					_keyState |= 4;

				if (event.kbd.keycode == Common::KEYCODE_DOWN) // Down
					_keyState |= 8;

				if (event.kbd.flags == Common::KBD_SHIFT)
					_keyState |= 16;

				if (event.kbd.flags == Common::KBD_CTRL)
					_keyState |= 32;

				VAR(VAR_KEY_STATE) = _keyState;
			}

			// FIXME: There is a discrepancy between EVENT_KEYDOWN and EVENT_KEYUP here:
			// For EVENT_KEYDOWN, we use _keyPressed.keycode, which has potentially been
			// modified, while for EVENT_KEYUP we use the unfiltered event.kbd.keycode.
			// This could lead problems (like a key becoming 'stuck').
			
			// FIXME #2: We are mixing ascii and keycode values here. We probably should
			// be using keycodes, but at least INSANE checks for "Shift-V" by looking for
			// the 'V' key being pressed. It would be easy to solve that by also storing the
			// the modifier flags. However, since getKeyState() is also called by scripts,
			// we have to be very careful with semantic changes.
			// Nevertheless, it's bad to rely on "ascii" holdoing keycode values for special
			// keys (like the function keys), so this should be fixed.

			if (_keyPressed.ascii >= 512)
				debugC(DEBUG_GENERAL, "_keyPressed > 512 (%d)", _keyPressed.ascii);
			else
				_keyDownMap[_keyPressed.ascii] = true;
			break;

		case Common::EVENT_KEYUP:
			// FIXME: for some reason Common::KBD_ALT is set sometimes
			// possible to a bug in sdl-common.cpp
			if (event.kbd.ascii >= 512)
				debugC(DEBUG_GENERAL, "keyPressed > 512 (%d)", event.kbd.ascii);
			else {
				_keyDownMap[event.kbd.ascii] = false;

				// Due to some weird bug with capslock key pressed
				// generated keydown event is for lower letter but
				// keyup is for upper letter
				// On most (all?) keyboards it is safe to assume that
				// both upper and lower letters are unpressed on keyup event
				//
				// Fixes bug #1709430: "FT: CAPSLOCK + V enables cheating for all fights"
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
				_mouse.x -= (Common::kHercW - _screenWidth * 2) / 2;
				_mouse.x /= 2;
				_mouse.y = _mouse.y * 4 / 7;
			}
			break;
		case Common::EVENT_LBUTTONUP:
			_leftBtnPressed &= ~msDown;
			break;

		case Common::EVENT_RBUTTONUP:
			_rightBtnPressed &= ~msDown;
			break;

		// The following two cases enable dialog choices to be
		// scrolled through in the SegaCD version of MI
		// as nothing else uses the wheel don't bother
		// checking the gameid. Values are taken from script-14.

		case Common::EVENT_WHEELDOWN:
			_keyPressed = Common::KeyState(Common::KEYCODE_7, 55);	// '7'
			break;

		case Common::EVENT_WHEELUP:
			_keyPressed = Common::KeyState(Common::KEYCODE_6, 54);	// '6'
			break;

		case Common::EVENT_QUIT:
			if (ConfMan.getBool("confirm_exit"))
				confirmExitDialog();
			else
				_quit = true;
			break;

		default:
			break;
		}
	}
}

#ifndef DISABLE_HE
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
}

void ScummEngine_v0::processInput() {
	// F1 - F3
	if (_keyPressed.keycode >= Common::KEYCODE_F1 && _keyPressed.keycode <= Common::KEYCODE_F3) {
		switchActor(_keyPressed.keycode - Common::KEYCODE_F1);
	}

	ScummEngine::processInput();
}

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

	_virtualMouse.x = _mouse.x + virtscr[0].xstart;
	_virtualMouse.y = _mouse.y - virtscr[0].topline;
	if (_game.version >= 7)
		_virtualMouse.y += _screenTop;

	if (_virtualMouse.y < 0)
		_virtualMouse.y = -1;
	if (_virtualMouse.y >= virtscr[0].h)
		_virtualMouse.y = -1;

	//
	// Determine the mouse button state.
	//
	_mouseAndKeyboardStat = 0;

	// Interpret 'return' as left click and 'tab' as right click
	if (lastKeyHit.keycode && _cursor.state > 0) {
		if (lastKeyHit.keycode == Common::KEYCODE_TAB) {
			_mouseAndKeyboardStat = MBS_RIGHT_CLICK;
			lastKeyHit.reset();
		} else if (lastKeyHit.keycode == Common::KEYCODE_RETURN) {
			_mouseAndKeyboardStat = MBS_LEFT_CLICK;
			lastKeyHit.reset();
		}
	}

	if ((_leftBtnPressed & msClicked) && (_rightBtnPressed & msClicked) && _game.version >= 4) {
		// Pressing both mouse buttons is treated as if you pressed
		// the cutscene exit key (ESC) in V4+ games. That mimicks
		// the behaviour of the original engine where pressing both
		// mouse buttons also skips the current cutscene.
		_mouseAndKeyboardStat = 0;
		lastKeyHit = Common::KeyState(Common::KEYCODE_ESCAPE);
	} else if ((_rightBtnPressed & msClicked) && (_game.version <= 3 && _game.id != GID_LOOM)) {
		// Pressing right mouse button is treated as if you pressed
		// the cutscene exit key (ESC) in V0-V3 games. That mimicks
		// the behaviour of the original engine where pressing right
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

		if (_game.version >= 7) {
			VAR(VAR_LEFTBTN_DOWN) = (_leftBtnPressed & msClicked) != 0;
			VAR(VAR_RIGHTBTN_DOWN) = (_rightBtnPressed & msClicked) != 0;
		}
	}

	_leftBtnPressed &= ~msClicked;
	_rightBtnPressed &= ~msClicked;

#ifdef _WIN32_WCE
	if (lastKeyHit.ascii == KEY_ALL_SKIP) {
		// Skip talk
		if (VAR_TALKSTOP_KEY != 0xFF && _talkDelay > 0) {
			lastKeyHit = Common::KeyState(Common::KEYCODE_PERIOD, '.');
		} else {
			lastKeyHit = Common::KeyState(Common::KEYCODE_ESCAPE);
		}
	}
#endif

	if (!lastKeyHit.ascii)
		return;
	
	processKeyboard(lastKeyHit);
}

#ifndef DISABLE_SCUMM_7_8
void ScummEngine_v8::processKeyboard(Common::KeyState lastKeyHit) {

	if (!(_game.features & GF_DEMO)) {
		// F1 (the trigger for the original save/load dialog) is mapped to F5
		if (lastKeyHit.keycode == Common::KEYCODE_F1 && lastKeyHit.flags == 0) {
			lastKeyHit = Common::KeyState(Common::KEYCODE_F5, Common::ASCII_F5);
		}
	
		// Alt-F5 brings up the original save/load dialog
		if (lastKeyHit.keycode == Common::KEYCODE_F5 && lastKeyHit.flags == Common::KBD_ALT) {
			lastKeyHit = Common::KeyState(Common::KEYCODE_F1, Common::ASCII_F1);
		}
	}

	// If a key script was specified (a V8 feature), and it's trigger
	// key was pressed, run it.
	if (_keyScriptNo && (_keyScriptKey == lastKeyHit.ascii)) {
		runScript(_keyScriptNo, 0, 0, 0);
		return;
	}

	// Fall back to V7 behavior
	ScummEngine_v7::processKeyboard(lastKeyHit);
}

void ScummEngine_v7::processKeyboard(Common::KeyState lastKeyHit) {

	// VAR_VERSION_KEY (usually ctrl-v) is used in COMI, Dig and FT to trigger
	// a version dialog, unless VAR_VERSION_KEY is set to 0. However, the COMI
	// version string is hard coded in the engine, hence we don't invoke
	// versionDialog for it. Dig/FT version strings are partly hard coded, too.
	if (_game.id != GID_CMI && 0 != VAR(VAR_VERSION_KEY) &&
	    lastKeyHit.keycode == Common::KEYCODE_v && lastKeyHit.flags == Common::KBD_CTRL) {
		versionDialog();
		return;
	}

	const bool cutsceneExitKeyEnabled = (VAR_CUTSCENEEXIT_KEY != 0xFF && VAR(VAR_CUTSCENEEXIT_KEY) != 0);

#ifndef _WIN32_WCE
	if (cutsceneExitKeyEnabled && lastKeyHit.keycode == Common::KEYCODE_ESCAPE) {
		// Skip cutscene (or active SMUSH video).
		if (_smushActive) {
			if (_game.id == GID_FT)
				_insane->escapeKeyHandler();
			else
				_smushVideoShouldFinish = true;
		}
		if (!_smushActive || _smushVideoShouldFinish)
			abortCutscene();

		_mouseAndKeyboardStat = Common::ASCII_ESCAPE;
		return;
	}
#else
	// On WinCE we've also got one special for skipping cutscenes or dialog, whatever is appropriate
	// Since _smushActive is not a member of the base case class ScummEngine::, we detect here if we're
	// playing a cutscene and skip it; else we forward the keystroke through to ScummEngine::processInput.
	if (cutsceneExitKeyEnabled && lastKeyHit.keycode == Common::KEYCODE_ESCAPE) {
// FIXME: I do not quite understand why this code behaves differently on WinCE ?!?
		int bail = 1;
		if (_smushActive) {
			if (_game.id == GID_FT) {
				_insane->escapeKeyHandler();
				bail = 0;
			} else
				_smushVideoShouldFinish = true;
		}
		if ((!_smushActive && vm.cutScenePtr[vm.cutSceneStackPointer]) || _smushVideoShouldFinish) {
			abortCutscene();
			bail = 0;
		}
		if (!bail) {
			_mouseAndKeyboardStat = Common::ASCII_ESCAPE;
			return;
		}
		
	}
#endif

	// Fall back to V6 behavior
	ScummEngine_v6::processKeyboard(lastKeyHit);
}
#endif

void ScummEngine_v6::processKeyboard(Common::KeyState lastKeyHit) {
	if (lastKeyHit.keycode == Common::KEYCODE_t && lastKeyHit.flags == Common::KBD_CTRL) {
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
		}

		if (VAR_VOICE_MODE != 0xFF)
			VAR(VAR_VOICE_MODE) = _voiceMode;

		return;
	}

	// Fall back to default behavior
	ScummEngine::processKeyboard(lastKeyHit);
}

void ScummEngine_v2::processKeyboard(Common::KeyState lastKeyHit) {
	if (lastKeyHit.ascii == ' ') {		// space
		pauseGame();
	} else if (lastKeyHit.keycode == Common::KEYCODE_F5 && lastKeyHit.flags == 0) {
		mainMenuDialog();
	} else if (lastKeyHit.keycode == Common::KEYCODE_F8 && lastKeyHit.flags == 0) {
		confirmRestartDialog();
	} else {

		const bool cutsceneExitKeyEnabled = (_game.version == 0) ||
						((VAR_CUTSCENEEXIT_KEY != 0xFF) ? (VAR(VAR_CUTSCENEEXIT_KEY) != 0) : false);

		if (cutsceneExitKeyEnabled && (lastKeyHit.keycode == Common::KEYCODE_ESCAPE || lastKeyHit.keycode == Common::KEYCODE_F4)) {
			abortCutscene();
		} else {
			// Fall back to default behavior
			ScummEngine::processKeyboard(lastKeyHit);
		}

		// Alt-F5 brings up the original save/load dialog
		if (lastKeyHit.keycode == Common::KEYCODE_F5 && lastKeyHit.flags == Common::KBD_ALT) {
			lastKeyHit = Common::KeyState(Common::KEYCODE_F5, Common::ASCII_F5);
		}
	
		// Store the input type. So far we can't distinguish
		// between 1, 3 and 5.
		// 1) Verb	2) Scene	3) Inv.		4) Key
		// 5) Sentence Bar
	
		if (VAR_KEYPRESS != 0xFF && lastKeyHit.keycode) {		// Key Input
			if (Common::KEYCODE_F1 <= lastKeyHit.keycode && lastKeyHit.keycode <= Common::KEYCODE_F12) {
				// Convert F-Keys for V1/V2 games (they start at 1)
				VAR(VAR_KEYPRESS) = lastKeyHit.keycode - Common::KEYCODE_F1 + 1;
			} else {
				VAR(VAR_KEYPRESS) = lastKeyHit.ascii;
			}
		}
	}
}

void ScummEngine_v3::processKeyboard(Common::KeyState lastKeyHit) {
	const bool restartKeyEnabled = (_game.platform == Common::kPlatformFMTowns);

	// F8 in FM-TOWNS games always triggers restart
	if (restartKeyEnabled && (lastKeyHit.keycode == Common::KEYCODE_F8 && lastKeyHit.flags == 0)) {
		confirmRestartDialog();
	} else {
		// Fall back to default behavior
		ScummEngine::processKeyboard(lastKeyHit);
	}

	// i brings up an IQ dialog in Indy3

	if (lastKeyHit.ascii == 'i' && _game.id == GID_INDY3) {
		// SCUMM var 244 is the episode score
		// and var 245 is the series score
		char text[50];

		// FIXME: Currently, the series score does not work properly
		// This workaround just sets it equal to the episode score
		// However, at the end of the game, it does show the episode
		// score by itself
		int a = _scummVars[245];
		if (!a)
			a = _scummVars[244];

		sprintf(text, "IQ Points: Episode = %d, Series = %d", _scummVars[244], a);
		Indy3IQPointsDialog indy3IQPointsDialog(this, text);
		runDialog(indy3IQPointsDialog);
	}
}

void ScummEngine::processKeyboard(Common::KeyState lastKeyHit) {
	const bool restartKeyEnabled = (VAR_RESTART_KEY != 0xFF && VAR(VAR_RESTART_KEY) != 0);
	const bool pauseKeyEnabled = (VAR_PAUSE_KEY != 0xFF && VAR(VAR_PAUSE_KEY) != 0);
	const bool talkstopKeyEnabled = (VAR_TALKSTOP_KEY != 0xFF && VAR(VAR_TALKSTOP_KEY) != 0);
	const bool cutsceneExitKeyEnabled = (VAR_CUTSCENEEXIT_KEY != 0xFF && VAR(VAR_CUTSCENEEXIT_KEY) != 0);

	bool mainmenuKeyEnabled = true;
	
	// For games which use VAR_MAINMENU_KEY, disable the mainmenu key if
	// requested by the scripts. We make an exception for COMI (i.e.
	// forcefully always enable it there), as that always disables it.
	if (VAR_MAINMENU_KEY != 0xFF && (_game.id != GID_CMI))
		mainmenuKeyEnabled = (VAR(VAR_MAINMENU_KEY) != 0);

/*
	FIXME: We also used to force-enable F5 in Sam&Max and HE >= 72 games -- why?
	if ((_game.version <= 3) || (_game.id == GID_SAMNMAX) || (_game.id == GID_CMI) || (_game.heversion >= 72))
		mainmenuKeyEnabled = true;
*/

	// Alt-F5 should bring up the original save/load dialog, if any.
	// Hence remap it to F5
	if ((_game.version > 2 && _game.version < 8) && (lastKeyHit.keycode == Common::KEYCODE_F5 && lastKeyHit.flags == Common::KBD_ALT)) {
		_mouseAndKeyboardStat = 319;	// SCUMM encoding for F5

	} else if (mainmenuKeyEnabled && (lastKeyHit.keycode == Common::KEYCODE_F5 && lastKeyHit.flags == 0)) {
		if (VAR_SAVELOAD_SCRIPT != 0xFF && _currentRoom != 0)
			runScript(VAR(VAR_SAVELOAD_SCRIPT), 0, 0, 0);

		mainMenuDialog();		// Display NewGui

		if (VAR_SAVELOAD_SCRIPT != 0xFF && _currentRoom != 0)
			runScript(VAR(VAR_SAVELOAD_SCRIPT2), 0, 0, 0);

	} else if (restartKeyEnabled && (lastKeyHit.keycode == Common::KEYCODE_F8 && lastKeyHit.flags == 0)) {
		confirmRestartDialog();

	} else if (pauseKeyEnabled && (lastKeyHit.keycode == Common::KEYCODE_SPACE && lastKeyHit.flags == 0)) {
		pauseGame();

	} else if (talkstopKeyEnabled && (lastKeyHit.keycode == Common::KEYCODE_PERIOD && lastKeyHit.flags == 0)) {
		_talkDelay = 0;
		if (_sound->_sfxMode & 2)
			stopTalk();

	} else if (cutsceneExitKeyEnabled && (lastKeyHit.keycode == Common::KEYCODE_ESCAPE && lastKeyHit.flags == 0)) {
		abortCutscene();
		_mouseAndKeyboardStat = VAR(VAR_CUTSCENEEXIT_KEY);

	} else {

		if (lastKeyHit.ascii == '[' || lastKeyHit.ascii == ']') { // Change music volume
			int vol = ConfMan.getInt("music_volume") / 16;
			if (lastKeyHit.ascii == ']' && vol < 16)
				vol++;
			else if (lastKeyHit.ascii == '[' && vol > 0)
				vol--;
	
			// Display the music volume
			ValueDisplayDialog dlg("Music volume: ", 0, 16, vol, ']', '[');
			vol = runDialog(dlg);
	
			vol *= 16;
			if (vol > Audio::Mixer::kMaxMixerVolume)
				vol = Audio::Mixer::kMaxMixerVolume;
	
			ConfMan.setInt("music_volume", vol);
			updateSoundSettings();
		} else if (lastKeyHit.ascii == '-' || lastKeyHit.ascii == '+') { // Change text speed
			if (lastKeyHit.ascii == '+' && _defaultTalkDelay > 0)
				_defaultTalkDelay--;
			else if (lastKeyHit.ascii == '-' && _defaultTalkDelay < 9)
				_defaultTalkDelay++;
	
			// Display the talk speed
			ValueDisplayDialog dlg("Subtitle speed: ", 0, 9, 9 - _defaultTalkDelay, '+', '-');
			_defaultTalkDelay = 9 - runDialog(dlg);
			
			// Save the new talkspeed value to ConfMan
			setTalkspeed(_defaultTalkDelay);
	
			if (VAR_CHARINC != 0xFF)
				VAR(VAR_CHARINC) = _defaultTalkDelay;
		} else if (lastKeyHit.ascii == '~' || lastKeyHit.ascii == '#') { // Debug console
			_debugger->attach();
		}

		_mouseAndKeyboardStat = lastKeyHit.ascii;
	}
}

} // End of namespace Scumm
