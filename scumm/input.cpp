/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2004 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"

#include "common/config-manager.h"

#include "scumm/debugger.h"
#include "scumm/imuse.h"
#include "scumm/insane/insane.h"
#include "scumm/scumm.h"
#include "scumm/sound.h"

namespace Scumm {

enum MouseButtonStatus {
	msDown = 1,
	msClicked = 2
};

void ScummEngine::parseEvents() {
	OSystem::Event event;

	while (_system->pollEvent(event)) {

		switch (event.event_code) {
		case OSystem::EVENT_KEYDOWN:
			if (event.kbd.keycode >= '0' && event.kbd.keycode <= '9'
				&& (event.kbd.flags == OSystem::KBD_ALT ||
					event.kbd.flags == OSystem::KBD_CTRL)) {
				_saveLoadSlot = event.kbd.keycode - '0';

				//  don't overwrite autosave (slot 0)
				if (_saveLoadSlot == 0)
					_saveLoadSlot = 10;

				sprintf(_saveLoadName, "Quicksave %d", _saveLoadSlot);
				_saveLoadFlag = (event.kbd.flags == OSystem::KBD_ALT) ? 1 : 2;
				_saveTemporaryState = false;
			} else if (event.kbd.flags == OSystem::KBD_CTRL) {
				if (event.kbd.keycode == 'f')
					_fastMode ^= 1;
				else if (event.kbd.keycode == 'g')
					_fastMode ^= 2;
				else if (event.kbd.keycode == 'd')
					_debugger->attach();
				else if (event.kbd.keycode == 's')
					resourceStats();
				else
					_keyPressed = event.kbd.ascii;	// Normal key press, pass on to the game.
			} else if (event.kbd.flags & OSystem::KBD_ALT) {
				// The result must be 273 for Alt-W
				// because that's what MI2 looks for in
				// its "instant win" cheat.
				_keyPressed = event.kbd.keycode + 154;
			} else if (event.kbd.ascii == 315 && (_gameId == GID_CMI && !(_features & GF_DEMO))) {
				// FIXME: support in-game menu screen. For now, this remaps F1 to F5 in COMI
				_keyPressed = 319;
			} else if (_gameId == GID_INDY4 && event.kbd.ascii >= '0' && event.kbd.ascii <= '9') {
				// To support keyboard fighting in FOA, we need to remap the number keys.
				// FOA apparently expects PC scancode values (see script 46 if you want
				// to know where I got these numbers from).
				static const int numpad[10] = {
						'0',
						335, 336, 337,
						331, 332, 333,
						327, 328, 329
					};
				_keyPressed = numpad[event.kbd.ascii - '0'];
			} else if (event.kbd.ascii < 273 || event.kbd.ascii > 276 || _version >= 7) {
				// don't let game have arrow keys as we currently steal them
				// for keyboard cursor control
				// this fixes bug with up arrow (273) corresponding to
				// "instant win" cheat in MI2 mentioned above
				//
				// This is not applicable to Full Throttle as it processes keyboard
				// cursor control by itself. Also it fixes derby scene
				_keyPressed = event.kbd.ascii;	// Normal key press, pass on to the game.
			}

			if (_heversion >= 80) {
				// Keyboard is controlled via variable
				int _keyState = 0;

				if (event.kbd.ascii == 276) // Left
					_keyState = 1;

				if (event.kbd.ascii == 275) // Right
					_keyState |= 2;

				if (event.kbd.ascii == 273) // Up
					_keyState |= 4;

				if (event.kbd.ascii == 274) // Down
					_keyState |= 8;

				if (event.kbd.flags == OSystem::KBD_SHIFT)
					_keyState |= 16;

				if (event.kbd.flags == OSystem::KBD_CTRL)
					_keyState |= 32;

				VAR(VAR_KEY_STATE) = _keyState;
			}

			if (_keyPressed >= 512)
				debugC(DEBUG_GENERAL, "_keyPressed > 512 (%d)", _keyPressed);
			else
				_keyDownMap[_keyPressed] = true;
			break;

		case OSystem::EVENT_KEYUP:
			// FIXME: for some reason OSystem::KBD_ALT is set sometimes
			// possible to a bug in sdl-common.cpp
			if (event.kbd.ascii >= 512)
				debugC(DEBUG_GENERAL, "keyPressed > 512 (%d)", event.kbd.ascii);
			else
				_keyDownMap[event.kbd.ascii] = false;
			break;

		case OSystem::EVENT_MOUSEMOVE:
			_mouse.x = event.mouse.x;
			_mouse.y = event.mouse.y;
			break;

		case OSystem::EVENT_LBUTTONDOWN:
			_leftBtnPressed |= msClicked|msDown;
#if defined(_WIN32_WCE) || defined(__PALM_OS__)
			_mouse.x = event.mouse.x;
			_mouse.y = event.mouse.y;
#endif
			break;

		case OSystem::EVENT_RBUTTONDOWN:
			_rightBtnPressed |= msClicked|msDown;
#if defined(_WIN32_WCE) || defined(__PALM_OS__)
			_mouse.x = event.mouse.x;
			_mouse.y = event.mouse.y;
#endif
			break;

		case OSystem::EVENT_LBUTTONUP:
			_leftBtnPressed &= ~msDown;
			break;

		case OSystem::EVENT_RBUTTONUP:
			_rightBtnPressed &= ~msDown;
			break;
		
		// The following two cases enable dialog choices to be
		// scrolled through in the SegaCD version of MI
		// as nothing else uses the wheel don't bother
		// checking the gameid
			
		case OSystem::EVENT_WHEELDOWN:
			_keyPressed = 55;
			break;

		case OSystem::EVENT_WHEELUP:
			_keyPressed = 54;
			break;
	
		case OSystem::EVENT_QUIT:
			if (_confirmExit)
				confirmexitDialog();
			else
				_quit = true;
			break;
	
		default:
			break;
		}
	}
}

void ScummEngine::clearClickedStatus() {
	_keyPressed = 0;
	_mouseButStat = 0;
	_leftBtnPressed &= ~msClicked;
	_rightBtnPressed &= ~msClicked;
}

void ScummEngine::processKbd(bool smushMode) {
	int saveloadkey;

	_lastKeyHit = _keyPressed;
	_keyPressed = 0;
	if (((_version <= 2) || (_features & GF_FMTOWNS && _version == 3)) && 315 <= _lastKeyHit && _lastKeyHit < 315+12) {
		// Convert F-Keys for V1/V2 games (they start at 1 instead of at 315)
		_lastKeyHit -= 314;
	}
	
	
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
	if (_features & GF_NEW_CAMERA)
		_virtualMouse.y += _screenTop;

	if (_virtualMouse.y < 0)
		_virtualMouse.y = -1;
	if (_virtualMouse.y >= virtscr[0].h)
		_virtualMouse.y = -1;

	//
	// Determine the mouse button state.
	//
	_mouseButStat = 0;

	// Interpret 'return' as left click and 'tab' as right click
	if (_lastKeyHit && _cursor.state > 0) {
		if (_lastKeyHit == 9) {
			_mouseButStat = MBS_RIGHT_CLICK;
			_lastKeyHit = 0;
		} else if (_lastKeyHit == 13) {
			_mouseButStat = MBS_LEFT_CLICK;
			_lastKeyHit = 0;
		}
	}

	if (_leftBtnPressed & msClicked && _rightBtnPressed & msClicked && _version > 3) {
		// Pressing both mouse buttons is treated as if you pressed
		// the cutscene exit key (i.e. ESC in most games). That mimicks
		// the behaviour of the original engine where pressing both
		// mouse buttons also skips the current cutscene.
		_mouseButStat = 0;
		_lastKeyHit = (uint)VAR(VAR_CUTSCENEEXIT_KEY);
	} else if (_rightBtnPressed & msClicked && (_version < 4 && _gameId != GID_LOOM)) {
		// Pressing right mouse button is treated as if you pressed
		// the cutscene exit key (i.e. ESC in most games). That mimicks
		// the behaviour of the original engine where pressing right
		// mouse button also skips the current cutscene.
		_mouseButStat = 0;
		_lastKeyHit = (uint)VAR(VAR_CUTSCENEEXIT_KEY);
	} else if (_leftBtnPressed & msClicked) {
		_mouseButStat = MBS_LEFT_CLICK;
	} else if (_rightBtnPressed & msClicked) {
		_mouseButStat = MBS_RIGHT_CLICK;
	}

	if (_version == 8) {
		VAR(VAR_MOUSE_BUTTONS) = 0;
		VAR(VAR_MOUSE_HOLD) = 0;
		VAR(VAR_RIGHTBTN_HOLD) = 0;

		if (_leftBtnPressed & msClicked)
			VAR(VAR_MOUSE_BUTTONS) += 1;

		if (_rightBtnPressed & msClicked)
			VAR(VAR_MOUSE_BUTTONS) += 2;

		if (_leftBtnPressed & msDown)
			VAR(VAR_MOUSE_HOLD) += 1;

		if (_rightBtnPressed & msDown) {
			VAR(VAR_RIGHTBTN_HOLD) = 1;
			VAR(VAR_MOUSE_HOLD) += 2;
		}
	} else if (_version >= 6) {
		VAR(VAR_LEFTBTN_HOLD) = (_leftBtnPressed & msDown) != 0;
		VAR(VAR_RIGHTBTN_HOLD) = (_rightBtnPressed & msDown) != 0;

		if (_version == 7) {
			VAR(VAR_LEFTBTN_DOWN) = (_leftBtnPressed & msClicked) != 0;
			VAR(VAR_RIGHTBTN_DOWN) = (_rightBtnPressed & msClicked) != 0;
		}
	}

	_leftBtnPressed &= ~msClicked;
	_rightBtnPressed &= ~msClicked;

	if (!_lastKeyHit)
		return;

	// If a key script was specified (a V8 feature), and it's trigger
	// key was pressed, run it.
	if (_keyScriptNo && (_keyScriptKey == _lastKeyHit)) {
		runScript(_keyScriptNo, 0, 0, 0);
		return;
	}

#ifdef _WIN32_WCE
	if (_lastKeyHit == KEY_ALL_SKIP) {
		// Skip cutscene
		if (smushMode) {
			_lastKeyHit = (uint)VAR(VAR_CUTSCENEEXIT_KEY);
		}
		else
		if (vm.cutScenePtr[vm.cutSceneStackPointer])
			_lastKeyHit = (uint)VAR(VAR_CUTSCENEEXIT_KEY);
		else 
		// Skip talk 
		if (_talkDelay > 0) 
			_lastKeyHit = (uint)VAR(VAR_TALKSTOP_KEY);
		else
		// Escape
			_lastKeyHit = 27;
	}
#endif

	if (VAR_RESTART_KEY != 0xFF && _lastKeyHit == VAR(VAR_RESTART_KEY) ||
	   (((_version <= 2) || (_features & GF_FMTOWNS && _version == 3)) && _lastKeyHit == 8)) {
		confirmrestartDialog();
		return;
	}

	if ((VAR_PAUSE_KEY != 0xFF && _lastKeyHit == VAR(VAR_PAUSE_KEY)) ||
		(VAR_PAUSE_KEY == 0xFF && _lastKeyHit == ' ')) {
		pauseGame();
		return;
	}

	// COMI version string is hard coded
	// Dig/FT version strings are partly hard coded too
	if (_version == 7 && _lastKeyHit == VAR(VAR_VERSION_KEY)) {
		versionDialog();
		return;
	}

	if ((_version <= 2) || (_features & GF_FMTOWNS && _version == 3))
		saveloadkey = 5;	// F5
	else if ((_version <= 3) || (_gameId == GID_SAMNMAX) || (_gameId == GID_CMI) || (_heversion >= 72))
		saveloadkey = 319;	// F5
	else
		saveloadkey = VAR(VAR_MAINMENU_KEY);

	if (_lastKeyHit == VAR(VAR_CUTSCENEEXIT_KEY) ||
		(VAR(VAR_CUTSCENEEXIT_KEY) == 4 && _lastKeyHit == 27)) {
		// Skip cutscene (or active SMUSH video). For the V2 games, which
		// normally use F4 for this, we add in a hack that makes escape work,
		// too (just for convenience).
		if (smushMode) {
			if (_gameId == GID_FT)
				_insane->escapeKeyHandler();
			else
				_smushVideoShouldFinish = true;
		}
		if (!smushMode || _smushVideoShouldFinish)
			abortCutscene();
		if (_version <= 2) {
			// Ensure that the input script also sees the key press.
			// This is necessary so you can abort the airplane travel
			// in Zak.
			VAR(VAR_KEYPRESS) = VAR(VAR_CUTSCENEEXIT_KEY);
		}
	} else if (_lastKeyHit == saveloadkey) {
		if (VAR_SAVELOAD_SCRIPT != 0xFF && _currentRoom != 0)
			runScript(VAR(VAR_SAVELOAD_SCRIPT), 0, 0, 0);

		mainMenuDialog();		// Display NewGui

		if (VAR_SAVELOAD_SCRIPT != 0xFF && _currentRoom != 0)
			runScript(VAR(VAR_SAVELOAD_SCRIPT2), 0, 0, 0);
		return;
	} else if (VAR_TALKSTOP_KEY != 0xFF && _lastKeyHit == VAR(VAR_TALKSTOP_KEY)) {
		_talkDelay = 0;
		if (_sound->_sfxMode & 2)
			stopTalk();
		return;
	} else if (_lastKeyHit == '[') { // [ Music volume down
		int vol = ConfMan.getInt("music_volume");
		if (!(vol & 0xF) && vol)
			vol -= 16;
		vol = vol & 0xF0;
		ConfMan.set("music_volume", vol);
		if (_imuse)
			_imuse->set_music_volume (vol);
	} else if (_lastKeyHit == ']') { // ] Music volume up
		int vol = ConfMan.getInt("music_volume");
		vol = (vol + 16) & 0xFF0;
		if (vol > 255) vol = 255;
		ConfMan.set("music_volume", vol);
		if (_imuse)
			_imuse->set_music_volume (vol);
	} else if (_lastKeyHit == '-') { // - text speed down
		if (_defaultTalkDelay < 9)
			_defaultTalkDelay++;
		if (VAR_CHARINC != 0xFF)
			VAR(VAR_CHARINC) = _defaultTalkDelay;
	} else if (_lastKeyHit == '+') { // + text speed up
		if (_defaultTalkDelay > 0)
			_defaultTalkDelay--;
		if (VAR_CHARINC != 0xFF)
			VAR(VAR_CHARINC) = _defaultTalkDelay;
	} else if (_lastKeyHit == '~' || _lastKeyHit == '#') { // Debug console
		_debugger->attach();
	} else if (_version <= 2) {
		// Store the input type. So far we can't distinguish
		// between 1, 3 and 5.
		// 1) Verb	2) Scene	3) Inv.		4) Key
		// 5) Sentence Bar

		if (_lastKeyHit) {		// Key Input
			VAR(VAR_KEYPRESS) = _lastKeyHit;
		}
	}

	_mouseButStat = _lastKeyHit;
}

} // End of namespace Scumm
