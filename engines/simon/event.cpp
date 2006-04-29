/* ScummVM - Scumm Interpreter
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

#include "simon/simon.h"
#include "simon/intern.h"

namespace Simon {

void SimonEngine::addTimeEvent(uint timeout, uint subroutine_id) {
	TimeEvent *te = (TimeEvent *)malloc(sizeof(TimeEvent)), *first, *last = NULL;
	time_t cur_time;

	time(&cur_time);

	te->time = cur_time + timeout - _gameStoppedClock;
	if (_clockStopped)
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

void SimonEngine::delTimeEvent(TimeEvent *te) {
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

void SimonEngine::invokeTimeEvent(TimeEvent *te) {
	Subroutine *sub;

	_scriptVerb = 0;

	if (_runScriptReturn1)
		return;

	sub = getSubroutineByID(te->subroutine_id);
	if (sub != NULL)
		startSubroutineEx(sub);

	_runScriptReturn1 = false;
}

void SimonEngine::killAllTimers() {
	TimeEvent *cur, *next;

	for (cur = _firstTimeStruct; cur; cur = next) {
		next = cur->next;
		delTimeEvent(cur);
	}
}

bool SimonEngine::kickoffTimeEvents() {
	time_t cur_time;
	TimeEvent *te;
	bool result = false;

	if (_clockStopped)
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

void SimonEngine::addVgaEvent(uint16 num, const byte *code_ptr, uint16 cur_sprite, uint16 curZoneNum, int32 param) {
	VgaTimerEntry *vte;

	// When Simon talks to the Golum about stew in French version of
	// Simon the Sorcerer 1 the code_ptr is at wrong location for
	// sprite 200. This  was a bug in the original game, which
	// caused several glitches in this scene.
	// We work around the problem by correcting the code_ptr for sprite
	// 200 in this scene, if it is wrong.
	if (getGameType() == GType_SIMON1 && _language == Common::FR_FRA &&
		(code_ptr - _vgaBufferPointers[curZoneNum].vgaFile1 == 4) && (cur_sprite == 200) && (curZoneNum == 2))
		code_ptr += 0x66;

	_lockWord |= 1;

	for (vte = _vgaTimerList; vte->delay; vte++) {
	}

	vte->delay = num;
	vte->script_pointer = code_ptr;
	vte->sprite_id = cur_sprite;
	vte->cur_vga_file = curZoneNum;
	vte->param = param;

	_lockWord &= ~1;
}

void SimonEngine::deleteVgaEvent(VgaTimerEntry * vte) {
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

void SimonEngine::processVgaEvents() {
	VgaTimerEntry *vte = _vgaTimerList;
	uint timer = (getGameType() == GType_FF) ? 5 : 1;

	_vgaTickCounter++;

	while (vte->delay) {
		vte->delay -= timer;
		if (vte->delay <= 0) {
			uint16 curZoneNum = vte->cur_vga_file;
			uint16 cur_sprite = vte->sprite_id;
			const byte *script_ptr = vte->script_pointer;
			int32 param = vte->param;

			_nextVgaTimerToProcess = vte + 1;
			deleteVgaEvent(vte);

			if (getGameType() == GType_FF && script_ptr == NULL) {
				panEvent(curZoneNum, cur_sprite, param);
			} else if (getGameType() == GType_SIMON2 && script_ptr == NULL) {
				scrollEvent();
			} else {
				animateEvent(script_ptr, curZoneNum, cur_sprite);
			}
			vte = _nextVgaTimerToProcess;
		} else {
			vte++;
		}
	}
}

void SimonEngine::animateEvent(const byte *code_ptr, uint16 curZoneNum, uint16 cur_sprite) {
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

void SimonEngine::panEvent(uint16 curZoneNum, uint16 cur_sprite, int32 param) {
	_vgaCurSpriteId = cur_sprite;
	_vgaCurZoneNum = curZoneNum;

	VgaSprite *vsp = findCurSprite();

	param &= 0x10;

	int32 pan = (vsp->x - _scrollX + param) * 8 - 2560;
	if (pan < -10000)
		pan = -10000;
	if (pan > 10000)
		pan = 10000;

	//setSfxPan(param, pan);

	if (pan != 0)
		addVgaEvent(10, NULL, _vgaCurSpriteId, _vgaCurZoneNum); /* pan event */
	debug(0, "panEvent: param %d pan %d", param, pan);
}

void SimonEngine::scrollEvent() {
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

		addVgaEvent(6, NULL, 0, 0); /* scroll event */
	}
}

} // End of namespace Simon
