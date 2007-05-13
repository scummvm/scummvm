/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
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

#include "agos/agos.h"
#include "agos/debugger.h"
#include "agos/intern.h"

#include "common/events.h"
#include "common/system.h"

#include "gui/about.h"

#include "sound/audiocd.h"

namespace AGOS {

void AGOSEngine::addTimeEvent(uint timeout, uint subroutine_id) {
	TimeEvent *te = (TimeEvent *)malloc(sizeof(TimeEvent)), *first, *last = NULL;
	time_t cur_time;

	time(&cur_time);

	te->time = cur_time + timeout - _gameStoppedClock;
	if (getGameType() == GType_FF && _clockStopped)
		te->time -= ((uint32)time(NULL) - _clockStopped);
	te->subroutine_id = subroutine_id;

	first = _firstTimeStruct;
	while (first) {
		if (te->time <= first->time) {
			if (last) {
				last->next = te;
				te->next = first;
				return;
			}
			te->next = _firstTimeStruct;
			_firstTimeStruct = te;
			return;
		}

		last = first;
		first = first->next;
	}

	if (last) {
		last->next = te;
		te->next = NULL;
	} else {
		_firstTimeStruct = te;
		te->next = NULL;
	}
}

void AGOSEngine::delTimeEvent(TimeEvent *te) {
	TimeEvent *cur;

	if (te == _pendingDeleteTimeEvent)
		_pendingDeleteTimeEvent = NULL;

	if (te == _firstTimeStruct) {
		_firstTimeStruct = te->next;
		free(te);
		return;
	}

	cur = _firstTimeStruct;
	if (cur == NULL)
		error("delTimeEvent: none available");

	for (;;) {
		if (cur->next == NULL)
			error("delTimeEvent: no such te");
		if (te == cur->next) {
			cur->next = te->next;
			free(te);
			return;
		}
		cur = cur->next;
	}
}

void AGOSEngine::invokeTimeEvent(TimeEvent *te) {
	Subroutine *sub;

	_scriptVerb = 0;

	if (_runScriptReturn1)
		return;

	sub = getSubroutineByID(te->subroutine_id);
	if (sub != NULL)
		startSubroutineEx(sub);

	_runScriptReturn1 = false;
}

void AGOSEngine::killAllTimers() {
	TimeEvent *cur, *next;

	for (cur = _firstTimeStruct; cur; cur = next) {
		next = cur->next;
		delTimeEvent(cur);
	}
	_clickOnly = 0;
}

bool AGOSEngine::kickoffTimeEvents() {
	time_t cur_time;
	TimeEvent *te;
	bool result = false;

	if (getGameType() == GType_FF && _clockStopped)
		return result;

	time(&cur_time);
	cur_time -= _gameStoppedClock;

	while ((te = _firstTimeStruct) != NULL && te->time <= (uint32)cur_time) {
		result = true;
		_pendingDeleteTimeEvent = te;
		invokeTimeEvent(te);
		if (_pendingDeleteTimeEvent) {
			_pendingDeleteTimeEvent = NULL;
			delTimeEvent(te);
		}
	}

	return result;
}

bool AGOSEngine::isVgaQueueEmpty() {
	VgaTimerEntry *vte = _vgaTimerList;
	bool result = false;

	while (vte->delay) {
		if (vte->cur_vga_file == _variableArray[999] && vte->sprite_id >= 100) {
			result = true;
			break;
		}
		vte++;
	}
	return result;
}

void AGOSEngine::haltAnimation() {
	if (_lockWord & 0x10)
		return;

	_lockWord |= 0x10;

	if (_displayScreen) {
		displayScreen();
		_displayScreen = false;
	}
}

void AGOSEngine::restartAnimation() {
	if (!(_lockWord & 0x10))
		return;

	_window4Flag = 2;

	setMoveRect(0, 0, 224, 127);
	displayScreen();

	_lockWord &= ~0x10;

	// Check picture queue
}

void AGOSEngine::addVgaEvent(uint16 num, const byte *code_ptr, uint16 cur_sprite, uint16 curZoneNum, uint8 type) {
	VgaTimerEntry *vte;

	_lockWord |= 1;

	for (vte = _vgaTimerList; vte->delay; vte++) {
	}

	vte->delay = num;
	vte->script_pointer = code_ptr;
	vte->sprite_id = cur_sprite;
	vte->cur_vga_file = curZoneNum;
	vte->type = type;

	_lockWord &= ~1;
}

void AGOSEngine::deleteVgaEvent(VgaTimerEntry * vte) {
	_lockWord |= 1;

	if (vte + 1 <= _nextVgaTimerToProcess) {
		_nextVgaTimerToProcess--;
	}

	do {
		memcpy(vte, vte + 1, sizeof(VgaTimerEntry));
		vte++;
	} while (vte->delay);

	_lockWord &= ~1;
}

void AGOSEngine::processVgaEvents() {
	VgaTimerEntry *vte = _vgaTimerList;

	_vgaTickCounter++;

	while (vte->delay) {
		vte->delay -= _vgaBaseDelay;
		if (vte->delay <= 0) {
			uint16 curZoneNum = vte->cur_vga_file;
			uint16 cur_sprite = vte->sprite_id;
			const byte *script_ptr = vte->script_pointer;
			uint8 type = vte->type;

			if (type == 2) {
				vte->delay = (getGameType() == GType_SIMON2) ? 5 : _frameRate;

				animateSprites();

				vte++;
			} else if (type == 1) {
				_nextVgaTimerToProcess = vte + 1;
				deleteVgaEvent(vte);

				scrollEvent();

				vte = _nextVgaTimerToProcess;
			} else if (type == 0) {
				_nextVgaTimerToProcess = vte + 1;
				deleteVgaEvent(vte);

				animateEvent(script_ptr, curZoneNum, cur_sprite);

				vte = _nextVgaTimerToProcess;
			}
		} else {
			vte++;
		}
	}
}

void AGOSEngine::animateEvent(const byte *code_ptr, uint16 curZoneNum, uint16 cur_sprite) {
	VgaPointersEntry *vpe;

	_vgaCurSpriteId = cur_sprite;

	_vgaCurZoneNum = curZoneNum;
	_zoneNumber = curZoneNum;
	vpe = &_vgaBufferPointers[curZoneNum];

	_curVgaFile1 = vpe->vgaFile1;
	_curVgaFile2 = vpe->vgaFile2;
	_curSfxFile = vpe->sfxFile;

	_vcPtr = code_ptr;

	runVgaScript();
}

void AGOSEngine::scrollEvent() {
	if (_scrollCount == 0)
		return;

	if (getGameType() == GType_FF) {
		if (_scrollCount < 0) {
			if (_scrollFlag != -8) {
				_scrollFlag = -8;
				_scrollCount += 8;
			}
		} else {
			if (_scrollFlag != 8) {
				_scrollFlag = 8;
				_scrollCount -= 8;
			}
		}
	} else {
		if (_scrollCount < 0) {
			if (_scrollFlag != -1) {
				_scrollFlag = -1;
				if (++_scrollCount == 0)
					return;
			}
		} else {
			if (_scrollFlag != 1) {
				_scrollFlag = 1;
				if (--_scrollCount == 0)
					return;
			}
		}

		addVgaEvent(6, NULL, 0, 0, 1); /* scroll event */
	}
}

void AGOSEngine::delay(uint amount) {
	Common::Event event;

	uint32 start = _system->getMillis();
	uint32 cur = start;
	uint this_delay, vga_period;

	AudioCD.updateCD();

	if (_debugger->isAttached())
		_debugger->onFrame();

	if (_fastMode)
	 	vga_period = 10;
	else if (getGameType() == GType_SIMON2)
		vga_period = 45;
	else
		vga_period = 50;

	_rnd.getRandomNumber(2);

	do {
		while (!_inCallBack && cur >= _lastVgaTick + vga_period && !_pause) {
			_lastVgaTick += vga_period;

			// don't get too many frames behind
			if (cur >= _lastVgaTick + vga_period * 2)
				_lastVgaTick = cur;

			_inCallBack = true;
			timer_callback();
			_inCallBack = false;
		}

		while (_eventMan->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_KEYDOWN:
				if (event.kbd.keycode >= '0' && event.kbd.keycode <='9'
					&& (event.kbd.flags == Common::KBD_ALT ||
						event.kbd.flags == Common::KBD_CTRL)) {
					_saveLoadSlot = event.kbd.keycode - '0';

					// There is no save slot 0
					if (_saveLoadSlot == 0)
						_saveLoadSlot = 10;

					sprintf(_saveLoadName, "Quicksave %d", _saveLoadSlot);
					_saveLoadType = (event.kbd.flags == Common::KBD_ALT) ? 1 : 2;

					// We should only allow a load or save when it was possible in original
					// This stops load/save during copy protection, conversations and cut scenes
					if (!_mouseHideCount && !_showPreposition)
						quickLoadOrSave();
				} else if (event.kbd.flags == Common::KBD_CTRL) {
					if (event.kbd.keycode == 'a') {
						GUI::Dialog *_aboutDialog;
						_aboutDialog = new GUI::AboutDialog();
						_aboutDialog->runModal();
					} else if (event.kbd.keycode == 'f')
						_fastMode ^= 1;
					else if (event.kbd.keycode == 'd')
						_debugger->attach();
				} 

				if (getGameType() == GType_PP) {
					if (event.kbd.flags == Common::KBD_SHIFT)
						_variableArray[41] = 0;
					else
						_variableArray[41] = 1;
				}

				// Make sure backspace works right (this fixes a small issue on OS X)
				if (event.kbd.keycode == 8)
					_keyPressed = 8;
				else
					_keyPressed = (byte)event.kbd.ascii;
				break;
			case Common::EVENT_MOUSEMOVE:
				break;
			case Common::EVENT_LBUTTONDOWN:
				if (getGameType() == GType_FF)
					setBitFlag(89, true);
				_leftButtonDown++;
				_leftButton = 1;
				break;
			case Common::EVENT_LBUTTONUP:
				if (getGameType() == GType_FF)
					setBitFlag(89, false);

				_leftButton = 0;
				_leftButtonCount = 0;
				break;
			case Common::EVENT_RBUTTONDOWN:
				if (getGameType() == GType_FF)
					setBitFlag(92, false);
				_rightButtonDown++;
				break;
			case Common::EVENT_QUIT:
				shutdown();
				return;
			default:
				break;
			}
		}

		if (_leftButton == 1)
			_leftButtonCount++;

		AudioCD.updateCD();

		_system->updateScreen();

		if (amount == 0)
			break;

		this_delay = _fastMode ? 1 : 20;
		if (this_delay > amount)
			this_delay = amount;
		_system->delayMillis(this_delay);

		cur = _system->getMillis();
	} while (cur < start + amount);
}

void AGOSEngine::timer_callback() {
	if (_timer5 != 0) {
		_syncFlag2 = true;
		_timer5--;
	} else {
		timer_proc1();
	}
}

void AGOSEngine::timer_proc1() {
	_timer4++;

	if (_lockWord & 0x80E9 || _lockWord & 2)
		return;

	_syncCount++;

	_lockWord |= 2;

	handleMouseMoved();

	if (!(_lockWord & 0x10)) {
		if (getGameType() == GType_PP) {
			_syncFlag2 ^= 1;
			if (!_syncFlag2) {
				processVgaEvents();
			} else {
				if (_scrollCount == 0) {
					_lockWord &= ~2;
					return;
				}
			}
		} else if (getGameType() == GType_FF) {
			_syncFlag2 ^= 1;
			if (!_syncFlag2) {
				processVgaEvents();
			} else {
				// Double speed on Oracle
				if (getBitFlag(99)) {
					processVgaEvents();
				} else if (_scrollCount == 0) {
					_lockWord &= ~2;
					return;
				}
			}
		} else {
			processVgaEvents();
			processVgaEvents();
			_syncFlag2 ^= 1;
			_cepeFlag ^= 1;
			if (!_cepeFlag)
				processVgaEvents();

			if (_mouseHideCount != 0 && _syncFlag2) {
				_lockWord &= ~2;
				return;
			}
		}
	} 

	if (getGameType() == GType_FF) {
		_moviePlay->nextFrame();
		animateSprites();
	}

	if (_copyPartialMode == 2) {
		fillFrontFromBack(0, 0, _screenWidth, _screenHeight);
		_copyPartialMode = 0;
	}

	if (_updateScreen) {
		_system->copyRectToScreen(getFrontBuf(), _screenWidth, 0, 0, _screenWidth, _screenHeight);
		_system->updateScreen();

		_updateScreen = false;
	}

	if (_displayScreen) {
		if (getGameType() == GType_FF) {
			if (!getBitFlag(78)) {
				oracleLogo();
			}
			if (getBitFlag(76)) {
				swapCharacterLogo();
			}
		}
		displayScreen();
		_displayScreen = false;
	}

	_lockWord &= ~2;
}

} // End of namespace AGOS
