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

#include "common/file.h"

#include "agos/intern.h"
#include "agos/agos.h"
#include "agos/vga.h"

namespace AGOS {

void AGOSEngine::setup_cond_c_helper() {
	HitArea *last;
	uint id;

	_noRightClick = 1;

	if (getGameType() == GType_WW)
		clearMenuStrip();

	if (getGameType() == GType_FF) {
		int cursor = 5;
		int animMax = 16;

		if (getBitFlag(200)) {
			cursor = 11;
			animMax = 5;
		} else if (getBitFlag(201)) {
			cursor = 12;
			animMax = 5;
		} else if (getBitFlag(202)) {
			cursor = 13;
			animMax = 5;
		} else if (getBitFlag(203)) {
			cursor = 14;
			animMax = 9;
		} else if (getBitFlag(205)) {
			cursor = 17;
			animMax = 11;
		} else if (getBitFlag(206)) {
			cursor = 16;
			animMax = 2;
		} else if (getBitFlag(208)) {
			cursor = 26;
			animMax = 2;
		} else if (getBitFlag(209)) {
			cursor = 27;
			animMax = 9;
		} else if (getBitFlag(210)) {
			cursor = 28;
			animMax = 9;
		}

		_animatePointer = 0;
		_mouseCursor = cursor;
		_mouseAnimMax = animMax;
		_mouseAnim = 1;
		_needHitAreaRecalc++;
	}

	if (getGameType() == GType_SIMON2) {
		_mouseCursor = 0;
		if (_defaultVerb != 999) {
			_mouseCursor = 9;
			_needHitAreaRecalc++;
			_defaultVerb = 0;
		}
	}

	_lastHitArea = 0;
	_hitAreaObjectItem = NULL;
	_nameLocked = 0;

	last = _lastNameOn;
	clearName();
	_lastNameOn = last;

	for (;;) {
		_lastHitArea = NULL;
		_lastHitArea3 = 0;
		_leftButtonDown = 0;

		do {
			if (_exitCutscene && getBitFlag(9)) {
				endCutscene();
				goto out_of_here;
			}

			if (getGameType() == GType_FF) {
				if (_variableArray[254] == 63) {
					hitarea_stuff_helper_2();
				} else if (_variableArray[254] == 75) {
					hitarea_stuff_helper_2();
					_variableArray[60] = 9999;
					goto out_of_here;
				}
			}

			delay(100);
		} while (_lastHitArea3 == (HitArea *) -1 || _lastHitArea3 == 0);

		if (_lastHitArea == NULL) {
		} else if (_lastHitArea->id == 0x7FFB) {
			inventoryUp(_lastHitArea->window);
		} else if (_lastHitArea->id == 0x7FFC) {
			inventoryDown(_lastHitArea->window);
		} else if (_lastHitArea->item_ptr != NULL) {
			_hitAreaObjectItem = _lastHitArea->item_ptr;
			id = 0xFFFF;
			if (_lastHitArea->flags & kBFTextBox) {
				if (getGameType() == GType_PP)
					id = _lastHitArea->id;
				else if (getGameType() == GType_FF && (_lastHitArea->flags & kBFHyperBox))
					id = _lastHitArea->data;
				else
					id = _lastHitArea->flags / 256;
			}
			if (getGameType() == GType_PP)
				_variableArray[199] = id;
			else if (getGameType() == GType_WW)
				_variableArray[10] = id;
			else
				_variableArray[60] = id;
			break;
		}
	}

out_of_here:
	_lastHitArea3 = 0;
	_lastHitArea = 0;
	_lastNameOn = NULL;
	_mouseCursor = 0;
	_noRightClick = 0;
}

void AGOSEngine::waitForInput() {
	HitArea *ha;
	uint id;

	_leftButtonDown = 0;
	_lastHitArea = 0;
	_verbHitArea = 0;
	_hitAreaSubjectItem = NULL;
	_hitAreaObjectItem = NULL;
	_nameLocked = 0;

	if (getGameType() == GType_WW) {
		_mouseCursor = 0;
		_needHitAreaRecalc++;
		clearMenuStrip();
	} else {
		resetVerbs();
	}

startOver:
	for (;;) {
		_lastHitArea = NULL;
		_lastHitArea3 = NULL;

		for (;;) {
			if (getGameType() != GType_FF && getGameType() != GType_PP && _keyPressed == 35)
				displayBoxStars();
			if (processSpecialKeys() != 0) {
				goto out_of_here;
			}
			if (_lastHitArea3 == (HitArea *) -1)
				goto startOver;
			if (_lastHitArea3 != 0)
				break;
			hitarea_stuff_helper();
			delay(100);
		}

		ha = _lastHitArea;

		if (ha == NULL) {
		} else if (ha->id == 0x7FFB) {
			inventoryUp(ha->window);
		} else if (ha->id == 0x7FFC) {
			inventoryDown(ha->window);
		} else if ((getGameType() == GType_SIMON1 || getGameType() == GType_SIMON2) &&
			(ha->id >= 101 && ha->id < 113)) {
			_verbHitArea = ha->verb;
			setVerb(ha);
			_defaultVerb = 0;
		} else {
			if (getGameType() == GType_WW) {
				if (_mouseCursor == 3)
					_verbHitArea = 236;

				if (ha->id == 98) {
					animate(2, 0, 110, 0, 0, 0);
					waitForSync(34);
				} else if (ha->id == 108) {
					animate(2, 0, 106, 0, 0, 0);
					waitForSync(34);
				} else if (ha->id == 109) {
					animate(2, 0, 107, 0, 0, 0);
					waitForSync(34);
				} else if (ha->id == 115) {
					animate(2, 0, 109, 0, 0, 0);
					waitForSync(34);
				} else if (ha->id == 116) {
					animate(2, 0, 113, 0, 0, 0);
					waitForSync(34);
				} else if (ha->id == 117) {
					animate(2, 0, 112, 0, 0, 0);
					waitForSync(34);
				} else if (ha->id == 118) {
					animate(2, 0, 108, 0, 0, 0);
					waitForSync(34);
				} else if (ha->id == 119) {
					animate(2, 0, 111, 0, 0, 0);
					waitForSync(34);
				}
			}
			if ((_verbHitArea != 0 || _hitAreaSubjectItem != ha->item_ptr && ha->flags & kBFBoxItem) &&
					ha->item_ptr) {
			if_1:;
				_hitAreaSubjectItem = ha->item_ptr;
				id = 0xFFFF;
				if (ha->flags & kBFTextBox) {
					if (getGameType() == GType_PP)
						id = _lastHitArea->id;
					else if (getGameType() == GType_FF && (ha->flags & kBFHyperBox))
						id = ha->data;
					else
						id = ha->flags / 256;
				}
				if (getGameType() == GType_PP)
					_variableArray[199] = id;
				else if (getGameType() == GType_WW)
					_variableArray[10] = id;
				else
					_variableArray[60] = id;

				_nameLocked = 2;
				displayName(ha);
				_nameLocked = 1;

				if (_verbHitArea != 0) {
					break;
				}

				if (getGameType() == GType_ELVIRA2)
					doMenuStrip(menuFor_e2(ha->item_ptr, id));
				else if (getGameType() == GType_WW)
					doMenuStrip(menuFor_ww(ha->item_ptr, id));
			} else {
				// else 1
				if (ha->verb == 0) {
					if (ha->item_ptr)
						goto if_1;
				} else {
					if (getGameType() == GType_WW && _mouseCursor != 0 && _mouseCursor < 4) {
						_hitAreaSubjectItem = ha->item_ptr;
						break;
					}

					_verbHitArea = ha->verb & 0xBFFF;
					if (ha->verb & 0x4000) {
						_hitAreaSubjectItem = ha->item_ptr;
						break;
					}
					if (_hitAreaSubjectItem != NULL)
						break;

					if (getGameType() == GType_WW) {
						if (ha->id == 109) {
							_mouseCursor = 2;
							_needHitAreaRecalc++;
						} else if (ha->id == 117) {
							_mouseCursor = 3;
							_needHitAreaRecalc++;
						}
					}
				}
			}
		}
	}

out_of_here:
	if (getGameType() == GType_ELVIRA2 || getGameType() == GType_WW)
		clearMenuStrip();

	_nameLocked = 0;
	_needHitAreaRecalc++;
}

void AGOSEngine::hitarea_stuff_helper() {
	time_t cur_time;

	if (getGameType() == GType_SIMON2 || getGameType() == GType_FF ||
		getGameType() == GType_PP) {
		if (_variableArray[254] || _variableArray[249]) {
			hitarea_stuff_helper_2();
		}
	} else if (getGameType() == GType_ELVIRA2 || getGameType() == GType_WW || 
		getGameType() == GType_SIMON1) {
		uint subr_id = (uint16)_variableArray[254];
		if (subr_id != 0) {
			Subroutine *sub = getSubroutineByID(subr_id);
			if (sub != NULL) {
				startSubroutineEx(sub);
				permitInput();
			}
			_variableArray[254] = 0;
			_runScriptReturn1 = false;
		}
	}

	time(&cur_time);
	if ((uint) cur_time != _lastTime) {
		_lastTime = cur_time;
		if (kickoffTimeEvents())
			permitInput();
	}
}

void AGOSEngine::hitarea_stuff_helper_2() {
	uint subr_id;
	Subroutine *sub;

	subr_id = (uint16)_variableArray[249];
	if (subr_id != 0) {
		sub = getSubroutineByID(subr_id);
		if (sub != NULL) {
			_variableArray[249] = 0;
			startSubroutineEx(sub);
			permitInput();
		}
		_variableArray[249] = 0;
	}

	subr_id = (uint16)_variableArray[254];
	if (subr_id != 0) {
		sub = getSubroutineByID(subr_id);
		if (sub != NULL) {
			_variableArray[254] = 0;
			startSubroutineEx(sub);
			permitInput();
		}
		_variableArray[254] = 0;
	}

	_runScriptReturn1 = false;
}

void AGOSEngine::permitInput() {
	if (!_mortalFlag) {
		_mortalFlag = true;
		showmessage_print_char(0);
		_curWindow = 0;
		if (_windowArray[0] != 0) {
			_textWindow = _windowArray[0];
			if (getGameType() == GType_FF || getGameType() == GType_PP)
				showmessage_helper_3(_textWindow->textColumn, _textWindow->width);
			else
				showmessage_helper_3(_textWindow->textLength, _textWindow->textMaxLength);
		}
		_mortalFlag = false;
	}
}

bool AGOSEngine::processSpecialKeys() {
	switch (_keyPressed) {
	case 17: // Up
		if (getGameType() == GType_PP)
			_verbHitArea = 302;
		else if (getGameType() == GType_WW)
			_verbHitArea = 239;
		break;
	case 18: // Down
		if (getGameType() == GType_PP)
			_verbHitArea = 304;
		else if (getGameType() == GType_WW)
			_verbHitArea = 241;
		break;
	case 19: // Right
		if (getGameType() == GType_PP)
			_verbHitArea = 303;
		else if (getGameType() == GType_WW)
			_verbHitArea = 240;
		break;
	case 20: // Left
		if (getGameType() == GType_PP)
			_verbHitArea = 301;
		else if (getGameType() == GType_WW)
			_verbHitArea = 242;
		break;
	case 27: // escape
		_exitCutscene = true;
		break;
	case 59: // F1
		if (getGameType() == GType_SIMON1) {
			vcWriteVar(5, 40);
		} else {
			vcWriteVar(5, 50);
		}
		vcWriteVar(86, 0);
		break;
	case 60: // F2
		if (getGameType() == GType_SIMON1) {
			vcWriteVar(5, 60);
		} else {
			vcWriteVar(5, 75);
		}
		vcWriteVar(86, 1);
		break;
	case 61: // F3
		if (getGameType() == GType_SIMON1) {
			vcWriteVar(5, 100);
		} else {
			vcWriteVar(5, 125);
		}
		vcWriteVar(86, 2);
		break;
	case 63: // F5
		if (getGameType() == GType_SIMON2 || getGameType() == GType_FF)
			_exitCutscene = true;
		break;
	case 65: // F7
		if (getGameType() == GType_FF && getBitFlag(76))
			_variableArray[254] = 70;
		break;
	case 67: // F9
		if (getGameType() == GType_FF)
			setBitFlag(73, !getBitFlag(73));
		break;
	case 'p':
		pause();
		break;
	case 't':
		if (getGameType() == GType_FF || (getGameType() == GType_SIMON2 && (getFeatures() & GF_TALKIE)) ||
			((getFeatures() & GF_TALKIE) && _language != Common::EN_ANY && _language != Common::DE_DEU)) {
			if (_speech)
				_subtitles ^= 1;
		}
		break;
	case 'v':
		if (getGameType() == GType_FF || (getGameType() == GType_SIMON2 && (getFeatures() & GF_TALKIE))) {
			if (_subtitles)
				_speech ^= 1;
		}
	case '+':
		midi.set_volume(midi.get_volume() + 16);
		break;
	case '-':
		midi.set_volume(midi.get_volume() - 16);
		break;
	case 'm':
		midi.pause(_musicPaused ^= 1);
		break;
	case 's':
		if (getGameId() == GID_SIMON1DOS)
			midi._enable_sfx ^= 1;
		else
			_sound->effectsPause(_effectsPaused ^= 1);
		break;
	case 'b':
		_sound->ambientPause(_ambientPaused ^= 1);
		break;
	case 'r':
		if (_debugMode)
			_startMainScript ^= 1;
		break;
	case 'o':
		if (_debugMode)
			_continousMainScript ^= 1;
		break;
	case 'a':
		if (_debugMode)
			_startVgaScript ^= 1;
		break;
	case 'g':
		if (_debugMode)
			_continousVgaScript ^= 1;
		break;
	case 'i':
		if (_debugMode)
			_drawImagesDebug ^= 1;
		break;
	case 'd':
		if (_debugMode)
			_dumpImages ^=1;
		break;
	}

	bool result = (_keyPressed != 0);
	_keyPressed = 0;
	return result;
}

} // End of namespace AGOS


