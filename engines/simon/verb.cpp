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

// Verb and hitarea handling
#include "common/stdafx.h"

#include "simon/simon.h"
#include "simon/intern.h"

namespace Simon {

static const char *const russian_verb_names[] = {
	"Ietj _",
	"Qnotrft< pa",
	"Nt_r[t<",
	"Ecjdat<",

	"Q=fst<",
	"C^]t<",
	"Ha_r[t<",
	"Isqom<^ocat<",

	"Docorjt<",
	"Qp]t<",
	"Neft<",
	"Eat<"
};

static const char *const hebrew_verb_names[] = {
	"LJ @L",
	"DQZKL RL",
	"TZG",
	"DFF",

	"@KEL",
	"DXM",
	"QBEX",
	"DYZNY",

	"CAX @L",
	"DQX",
	"LAY",
	"ZO"
};

static const char *const spanish_verb_names[] = {
	"Caminar",
	"Mirar",
	"Abrir",
	"Mover",

	"Consumir",
	"Coger",
	"Cerrar",
	"Usar",

	"Hablar",
	"Quitar",
	"Llevar",
	"Dar"
};

static const char *const italian_verb_names[] = {
	"Vai verso",
	"Osserva",
	"Apri",
	"Sposta",

	"Mangia",
	"Raccogli",
	"Chiudi",
	"Usa",

	"Parla a",
	"Togli",
	"Indossa",
	"Dai"
};

static const char *const french_verb_names[] = {
	"Aller vers",
	"Regarder",
	"Ouvrir",
	"D/placer",

	"Consommer",
	"Prendre",
	"Fermer",
	"Utiliser",

	"Parler ;",
	"Enlever",
	"Mettre",
	"Donner"
};

static const char *const german_verb_names[] = {
	"Gehe zu",
	"Schau an",
	";ffne",
	"Bewege",

	"Verzehre",
	"Nimm",
	"Schlie+e",
	"Benutze",

	"Rede mit",
	"Entferne",
	"Trage",
	"Gib"
};

static const char *const english_verb_names[] = {
	"Walk to",
	"Look at",
	"Open",
	"Move",

	"Consume",
	"Pick up",
	"Close",
	"Use",

	"Talk to",
	"Remove",
	"Wear",
	"Give"
};

static const char *const russian_verb_prep_names[] = {
	"", "", "", "",
	"", "", "", "s yfn?",
	"", "", "", "_onu ?"
};

static const char *const hebrew_verb_prep_names[] = {
	"", "", "", "",
	"", "", "", "RM ND ?",
	"", "", "", "LNI ?"
};

static const char *const spanish_verb_prep_names[] = {
	"", "", "", "",
	"", "", "", "^con qu/?",
	"", "", "", "^a qui/n?"
};

static const char *const italian_verb_prep_names[] = {
	"", "", "", "",
	"", "", "", "con cosa ?",
	"", "", "", "a chi ?"
};

static const char *const french_verb_prep_names[] = {
	"", "", "", "",
	"", "", "", "avec quoi ?",
	"", "", "", "; qui ?"
};

static const char *const german_verb_prep_names[] = {
	"", "", "", "",
	"", "", "", "mit was ?",
	"", "", "", "zu wem ?"
};

static const char *const english_verb_prep_names[] = {
	"", "", "", "",
	"", "", "", "with what ?",
	"", "", "", "to whom ?"
};

void SimonEngine::clearName() {
	HitArea *last;
	HitArea *ha;

	if (getGameType() == GType_FF) {
		stopAnimateSimon2(2, 6);
		_lastNameOn = NULL;
		_animatePointer = 0;
		_mouseAnim = 1;
		return;
	}

	if (getGameType() == GType_SIMON2) {
		if (getBitFlag(79)) {
			sendSync(202);
			_lastNameOn = NULL;
			return;
		}
	}

	last = _currentVerbBox;

	if (last == _lastVerbOn)
		return;

	resetNameWindow();
	_lastVerbOn = last;

	if (last != NULL && (ha = findBox(200)) && (ha->flags & kBFBoxDead) && !(last->flags & kBFBoxDead))
		printVerbOf(last->id);
}

void SimonEngine::printVerbOf(uint hitarea_id) {
	const char *txt;
	const char * const *verb_names;
	const char * const *verb_prep_names;

	hitarea_id -= 101;

	if (_showPreposition) {
		switch (_language) {
		case Common::RU_RUS:
			verb_prep_names = russian_verb_prep_names;
			break;
		case Common::HB_ISR:
			verb_prep_names = hebrew_verb_prep_names;
			break;
		case Common::ES_ESP:
			verb_prep_names = spanish_verb_prep_names;
			break;
		case Common::IT_ITA:
			verb_prep_names = italian_verb_prep_names; 
			break;
		case Common::FR_FRA:
			verb_prep_names = french_verb_prep_names;
			break;
		case Common::DE_DEU:
			verb_prep_names = german_verb_prep_names;
			break;
		default: 
			verb_prep_names = english_verb_prep_names;
			break;
		}
		CHECK_BOUNDS(hitarea_id, english_verb_prep_names);
		txt = verb_prep_names[hitarea_id];
	} else {
		switch (_language) {
		case Common::RU_RUS:
			verb_names = russian_verb_names;
			break;
		case Common::HB_ISR:
			verb_names = hebrew_verb_names;
			break;
		case Common::ES_ESP:
			verb_names = spanish_verb_names;
			break;
		case Common::IT_ITA:
			verb_names = italian_verb_names;
			break;
		case Common::FR_FRA:
			verb_names = french_verb_names;
			break;
		case Common::DE_DEU:
			verb_names = german_verb_names;
			break;
		default: 
			verb_names = english_verb_names;
			break;
		}
		CHECK_BOUNDS(hitarea_id, english_verb_names);
		txt = verb_names[hitarea_id];
	}
	showActionString((const byte *)txt);
}

void SimonEngine::showActionString(const byte *string) {
	WindowBlock *window;
	uint x;

	window = _windowArray[1];
	if (window == NULL || window->text_color == 0)
		return;

	// Arisme : hack for long strings in the French version
	if ((strlen((const char*)string) - 1) <= 53)
		x = (53 - (strlen((const char *)string) - 1)) * 3;
	else
		x = 0;

	window->textColumn = x / 8;
	window->textColumnOffset = x & 7;

	for (; *string; string++)
		windowPutChar(window, *string);
}

void SimonEngine::handleVerbClicked(uint verb) {
	Subroutine *sub;
	int result;

	_objectItem = _hitAreaObjectItem;
	if (_objectItem == _dummyItem2) {
		_objectItem = me();
	}
	if (_objectItem == _dummyItem3) {
		_objectItem = derefItem(me()->parent);
	}

	_subjectItem = _hitAreaSubjectItem;
	if (_subjectItem == _dummyItem2) {
		_subjectItem = me();
	}
	if (_subjectItem == _dummyItem3) {
		_subjectItem = derefItem(me()->parent);
	}

	if (_subjectItem) {
		_scriptNoun1 = _subjectItem->noun;
		_scriptAdj1 = _subjectItem->adjective;
	} else {
		_scriptNoun1 = -1;
		_scriptAdj1 = -1;
	}

	if (_objectItem) {
		_scriptNoun2 = _objectItem->noun;
		_scriptAdj2 = _objectItem->adjective;
	} else {
		_scriptNoun2 = -1;
		_scriptAdj2 = -1;
	}

	_scriptVerb = _verbHitArea;

	sub = getSubroutineByID(0);
	if (sub == NULL)
		return;

	result = startSubroutine(sub);
	if (result == -1)
		showMessageFormat("I don't understand");

	_runScriptReturn1 = false;

	sub = getSubroutineByID(100);
	if (sub)
		startSubroutine(sub);

	if (getGameType() == GType_SIMON2 || getGameType() == GType_FF)
		_runScriptReturn1 = false;

	permitInput();
}

void SimonEngine::resetNameWindow() {
	WindowBlock *window;

	if (getGameType() == GType_SIMON2 && getBitFlag(79))
		return;

	window = _windowArray[1];
	if (window != NULL && window->text_color != 0)
		clearWindow(window);

	_lastNameOn = NULL;
	_lastVerbOn = NULL;
}

HitArea *SimonEngine::findBox(uint hitarea_id) {
	HitArea *ha = _hitAreas;
	uint count = ARRAYSIZE(_hitAreas);

	do {
		if (ha->id == hitarea_id && ha->flags != 0)
			return ha;
	} while (ha++, --count);
	return NULL;
}

HitArea *SimonEngine::findEmptyHitArea() {
	HitArea *ha = _hitAreas;
	uint count = ARRAYSIZE(_hitAreas);

	do {
		if (ha->flags == 0)
			return ha;
	} while (ha++, --count);
	return NULL;
}

void SimonEngine::delete_hitarea_by_index(uint index) {
	CHECK_BOUNDS(index, _hitAreas);
	_hitAreas[index].flags = 0;
}

void SimonEngine::enableBox(uint hitarea) {
	HitArea *ha = findBox(hitarea);
	if (ha != NULL)
		ha->flags &= ~kBFBoxDead;
}

void SimonEngine::disableBox(uint hitarea) {
	HitArea *ha = findBox(hitarea);
	if (ha != NULL) {
		ha->flags |= kBFBoxDead;
		ha->flags &= ~kBFBoxSelected;
		if (hitarea == 102)
			resetVerbs();
	}
}

void SimonEngine::moveBox(uint hitarea, int x, int y) {
	HitArea *ha = findBox(hitarea);
	if (ha != NULL) {
		if (getGameType() == GType_FF) {
			ha->x += x;
			ha->y += y;
		} else {
			ha->x = x;
			ha->y = y;
		}
	}
}

void SimonEngine::undefineBox(uint hitarea) {
	HitArea *ha = findBox(hitarea);
	if (ha != NULL) {
		ha->flags = 0;
		if (ha == _lastNameOn)
			clearName();
		_needHitAreaRecalc++;
	}
}

bool SimonEngine::is_hitarea_0x40_clear(uint hitarea) {
	HitArea *ha = findBox(hitarea);
	if (ha == NULL)
		return false;
	return (ha->flags & kBFBoxDead) == 0;
}

void SimonEngine::defineBox(int id, int x, int y, int width, int height, int flags, int verb, Item *item_ptr) {
	HitArea *ha;
	undefineBox(id);

	ha = findEmptyHitArea();
	ha->x = x;
	ha->y = y;
	ha->width = width;
	ha->height = height;
	ha->flags = flags | kBFBoxInUse;
	ha->id = ha->priority = id;
	ha->verb = verb;
	ha->item_ptr = item_ptr;

	if (getGameType() == GType_FF && (ha->flags & kBFHyperBox)) {
		ha->data = _hyperLink;
		ha->priority = 50;
	}

	_needHitAreaRecalc++;
}

void SimonEngine::resetVerbs() {
	if (getGameType() == GType_FF) {
		_verbHitArea = 300;
		int cursor = 0;
		int animMax = 16;

		if (getBitFlag(203)) {
			cursor = 14;
			animMax = 9;
		} else if (getBitFlag(204)) {
			cursor = 15;
			animMax = 9;
		} else if (getBitFlag(207)) {
			cursor = 26;
			animMax = 2;
		}

		_mouseCursor = cursor;
		_mouseAnimMax = animMax;
		_mouseAnim = 1;
		_needHitAreaRecalc++;

		if (getBitFlag(99)) {
			setVerb(NULL);
		}
	} else {
		uint id;
		HitArea *ha;

		if (getGameType() == GType_SIMON2) {
			id = 2;
			if (!getBitFlag(79))
				id = (_mouseY >= 136) ? 102 : 101;
		} else {
			id = (_mouseY >= 136) ? 102 : 101;
		}

		_defaultVerb = id;

		ha = findBox(id);
		if (ha == NULL)
			return;

		if (ha->flags & kBFBoxDead) {
			_defaultVerb = 999;
			_currentVerbBox = NULL;
		} else {
			_verbHitArea = ha->verb;
			setVerb(ha);
		}
	}
}

void SimonEngine::setVerb(HitArea *ha) {
	if (getGameType() == GType_FF) {
		int cursor = _mouseCursor;
		if (_noRightClick)
			return;

		if (cursor > 13)
			cursor = 0;
		cursor++;
		if (cursor == 5)
			cursor = 1;
		if (cursor == 4) {
			if (getBitFlag(72)) {
				cursor = 1;
			}
		} else if (cursor == 2) {
			if (getBitFlag(99)) {
				cursor = 3;
			}
		}

		_mouseCursor = cursor;
		_mouseAnimMax = (cursor == 4) ? 14: 16;
		_mouseAnim = 1;
		_needHitAreaRecalc++;
		_verbHitArea = cursor + 300;
	} else {
		HitArea *tmp = _currentVerbBox;

		if (ha == tmp)
			return;

		if (getGameType() == GType_SIMON1) {
			if (tmp != NULL) {
				tmp->flags |= kBFInvertTouch;
				video_toggle_colors(tmp, 0xd5, 0xd0, 0xd5, 0xA);
			}

			if (ha->flags & kBFBoxSelected)
				video_toggle_colors(ha, 0xda, 0xd5, 0xd5, 5);
			else
				video_toggle_colors(ha, 0xdf, 0xda, 0xda, 0xA);

			ha->flags &= ~(kBFBoxSelected + kBFInvertTouch);
		} else {
			if (ha->id < 101)
				return;
			_mouseCursor = ha->id - 101;
			_needHitAreaRecalc++;
		}
		_currentVerbBox = ha;
	}
}

void SimonEngine::hitarea_leave(HitArea *ha, bool state) {
	if (getGameType() == GType_FF) {
		invertBox(ha, state);
	} else if (getGameType() == GType_SIMON2) {
		video_toggle_colors(ha, 0xe7, 0xe5, 0xe6, 1);
	} else {
		video_toggle_colors(ha, 0xdf, 0xd5, 0xda, 5);
	}
}

void SimonEngine::leaveHitAreaById(uint hitarea_id) {
	HitArea *ha = findBox(hitarea_id);
	if (ha)
		hitarea_leave(ha);
}

void SimonEngine::checkUp(WindowBlock *window) {
	uint16 j, k;

	if (((_variableArray[31] - _variableArray[30]) == 40) && (_variableArray[31] > 52)) {
		k = (((_variableArray[31] / 52) - 2) % 3);
		j = k * 6;
		if (!is_hitarea_0x40_clear(j + 201)) {
			uint index = getWindowNum(window);
			drawIconArray(index, window->iconPtr->itemRef, 0, window->iconPtr->classMask);
			loadSprite(4, 9, k + 34, 0, 0, 0);	
		}
	}
	if ((_variableArray[31] - _variableArray[30]) == 76) {
		k = ((_variableArray[31] / 52) % 3);
		j = k * 6;
		if (is_hitarea_0x40_clear(j + 201)) {
			loadSprite(4, 9, k + 31, 0, 0, 0);
			undefineBox(j + 201);
			undefineBox(j + 202);
			undefineBox(j + 203);
			undefineBox(j + 204);
			undefineBox(j + 205);
			undefineBox(j + 206);
		}
		_variableArray[31] -= 52;
		_iOverflow = 1;
	}
}

void SimonEngine::checkDown(WindowBlock *window) {
	uint16 j, k;

	if (((_variableArray[31] - _variableArray[30]) == 24) && (_iOverflow == 1)) {
		uint index = getWindowNum(window);
		drawIconArray(index, window->iconPtr->itemRef, 0, window->iconPtr->classMask);
		k = ((_variableArray[31] / 52) % 3);
		loadSprite(4, 9, k + 25, 0, 0, 0);	
		_variableArray[31] += 52;
	}
	if (((_variableArray[31] - _variableArray[30]) == 40) && (_variableArray[30] > 52)) {
		k = (((_variableArray[31] / 52) + 1) % 3);
		j = k * 6;
		if (is_hitarea_0x40_clear(j + 201)) {
			loadSprite(4, 9, k + 28, 0, 0, 0);
			undefineBox(j + 201);
			undefineBox(j + 202);
			undefineBox(j + 203);
			undefineBox(j + 204);
			undefineBox(j + 205);
			undefineBox(j + 206);
		}
	}
}

void SimonEngine::inventoryUp(WindowBlock *window) {
	if (getGameType() == GType_FF) {
		_marks = 0;
		checkUp(window);
		loadSprite(4, 9, 21, 0 ,0, 0);	
		while(1) {
			if (_currentBoxNumber != 0x7FFB || !getBitFlag(89))
				break;
			checkUp(window);
			delay(1);
		}
		waitForMark(2);
		checkUp(window);
		sendSync(922);
		waitForMark(1);
		checkUp(window);
	} else {
		if (window->iconPtr->line == 0)
			return;

		mouseOff();
		uint index = getWindowNum(window);
		drawIconArray(index, window->iconPtr->itemRef, window->iconPtr->line - 1, window->iconPtr->classMask);
		mouseOn();
	}
}

void SimonEngine::inventoryDown(WindowBlock *window) {
	if (getGameType() == GType_FF) {
		_marks = 0;
		checkDown(window);
		loadSprite(4, 9, 23, 0, 0, 0);	
		while(1) {
			if (_currentBoxNumber != 0x7FFC || !getBitFlag(89))
				break;
			checkDown(window);
			delay(1);
		}
		waitForMark(2);
		checkDown(window);
		sendSync(924);
		waitForMark(1);
		checkDown(window);
	} else {
		mouseOff();
		uint index = getWindowNum(window);
		drawIconArray(index, window->iconPtr->itemRef, window->iconPtr->line + 1, window->iconPtr->classMask);
		mouseOn();
	}
}

void SimonEngine::boxController(uint x, uint y, uint mode) {
	HitArea *best_ha;
	HitArea *ha = _hitAreas;
	uint count = ARRAYSIZE(_hitAreas);
	uint16 priority = 0;
	uint16 x_ = x;
	uint16 y_ = y;

	if (getGameType() == GType_FF) {
		x_ += _scrollX;
		y_ += _scrollY;
	}
	if (getGameType() == GType_SIMON2) {
		if (getBitFlag(79) || y < 134) {
			x_ += _scrollX * 8;
		}
	}

	best_ha = NULL;

	do {
		if (ha->flags & kBFBoxInUse) {
			if (!(ha->flags & kBFBoxDead)) {
				if (x_ >= ha->x && y_ >= ha->y &&
						x_ - ha->x < ha->width && y_ - ha->y < ha->height && priority <= ha->priority) {
					priority = ha->priority;
					best_ha = ha;
				} else {
					if (ha->flags & kBFBoxSelected) {
						hitarea_leave(ha , true);
						ha->flags &= ~kBFBoxSelected;
					}
				}
			} else {
				ha->flags &= ~kBFBoxSelected;
			}
		}
	} while (ha++, --count);

	_currentBoxNumber = 0;

	if (best_ha == NULL) {
		clearName();
		return;
	}

	_currentBoxNumber = best_ha->id;

	if (mode != 0 && mode != 3) {
		_lastHitArea = best_ha;
		_variableArray[1] = x;
		_variableArray[2] = y;
	}

	if (best_ha->flags & kBFNoTouchName) {
		clearName();
	} else if (best_ha != _lastNameOn) {
		displayName(best_ha);
	}

	if (best_ha->flags & kBFInvertTouch && !(best_ha->flags & kBFBoxSelected)) {
		hitarea_leave(best_ha, false);
		best_ha->flags |= kBFBoxSelected;
	}

	return;
}

void SimonEngine::displayName(HitArea *ha) {
	bool result;
	int x = 0, y = 0;

	if (getGameType() == GType_FF) {
		if (ha->flags & kBFHyperBox) {
			_lastNameOn = ha;
			return;
		}
		if (findBox(50))
			return;

		if (getBitFlag(99))
			_animatePointer = ((ha->flags & kBFTextBox) == 0);
		else
			_animatePointer = 1;

		if (!getBitFlag(73))
			return;

		y = ha->y;
		if (getBitFlag(99) && y > 288)
			y = 288;
		y -= 17;
		if (y < 0)
			y = 0;
		y += 2;
		x = ha->width / 2 + ha->x;
	} else {
		resetNameWindow();
	}

	if (ha->flags & kBFTextBox) {
		result = printTextOf(ha->flags / 256, x, y);
	} else {
		result = printNameOf(ha->item_ptr, x, y);
	}

	if (result)
		_lastNameOn = ha;
}

void SimonEngine::invertBox(HitArea *ha, bool state) {
	if (getBitFlag(205) || getBitFlag(206)) {
		if (state != 0) {
			_mouseAnimMax = _oldMouseAnimMax;
			_mouseCursor = _oldMouseCursor;
		} else if (_mouseCursor != 18) {
			_oldMouseCursor = _mouseCursor;
			_animatePointer = 0;
			_oldMouseAnimMax = _mouseAnimMax;
			_mouseAnimMax = 2;
			_mouseCursor = 18;
		}
	} else {
		if (getBitFlag(207)) {
			if (state != 0) {
				_noRightClick = 0;
				resetVerbs();
			} else {
				int cursor = ha->id + 9;
				if (cursor >= 23)
					cursor = 21;
				_mouseCursor = cursor;
				_mouseAnimMax = 8;
				_noRightClick = 1;
			}
		} else {
			VgaSprite *vsp = _vgaSprites;

			int id = ha->id - 43;
			while (vsp->id) {
				if (vsp->id == id && vsp->zoneNum == 2) {
					if (state == 0)
						vsp->flags |= kDFShaded;
					else
						vsp->flags &= ~kDFShaded;
					break;
				}
				vsp++;
			}
		}
	}
}

} // End of namespace Simon
