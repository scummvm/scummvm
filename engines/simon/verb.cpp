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
		o_kill_sprite_simon2(2, 6);
		_lastNameOn = NULL;
		//_animatePointer = 0;
		_mouseAnim = 1;
		return;
	}

	if (getGameType() == GType_SIMON2) {
		if (getBitFlag(79)) {
			o_sync(202);
			_lastNameOn = NULL;
			return;
		}
	}

	last = _currentVerbBox;

	if (last == _hitAreaPtr7)
		return;

	hitareaChangedHelper();
	_hitAreaPtr7 = last;

	if (last != NULL && (ha = findHitAreaByID(200)) && (ha->flags & 0x40) && !(last->flags & 0x40))
		focusVerb(last->id);
}

void SimonEngine::focusVerb(uint hitarea_id) {
	uint x;
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
	x = (53 - strlen(txt)) * 3;
	showActionString(x, (const byte *)txt);
}

void SimonEngine::showActionString(uint x, const byte *string) {
	WindowBlock *window;

	window = _windowArray[1];
	if (window == NULL || window->text_color == 0)
		return;

	window->textColumn = x >> 3;
	window->textColumnOffset = x & 7;

	for (; *string; string++)
		video_putchar(window, *string);
}

void SimonEngine::hitareaChangedHelper() {
	WindowBlock *window;

	if (getGameType() == GType_SIMON2) {
		if (getBitFlag(79))
			return;
	}

	window = _windowArray[1];
	if (window != NULL && window->text_color != 0)
		clearWindow(window);

	_lastNameOn = NULL;
	_hitAreaPtr7 = NULL;
}

HitArea *SimonEngine::findHitAreaByID(uint hitarea_id) {
	HitArea *ha = _hitAreas;
	uint count = ARRAYSIZE(_hitAreas);

	do {
		if (ha->id == hitarea_id)
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

void SimonEngine::clear_hitarea_bit_0x40(uint hitarea) {
	HitArea *ha = findHitAreaByID(hitarea);
	if (ha != NULL)
		ha->flags &= ~0x40;
}

void SimonEngine::set_hitarea_bit_0x40(uint hitarea) {
	HitArea *ha = findHitAreaByID(hitarea);
	if (ha != NULL) {
		ha->flags |= 0x40;
		ha->flags &= ~2;
		if (hitarea == 102)
			resetVerbs();
	}
}

void SimonEngine::moveBox(uint hitarea, int x, int y) {
	HitArea *ha = findHitAreaByID(hitarea);
	if (ha != NULL) {
		ha->x = x;
		ha->y = y;
	}
}

void SimonEngine::delete_hitarea(uint hitarea) {
	HitArea *ha = findHitAreaByID(hitarea);
	if (ha != NULL) {
		ha->flags = 0;
		if (ha == _lastNameOn)
			clearName();
		_needHitAreaRecalc++;
	}
}

bool SimonEngine::is_hitarea_0x40_clear(uint hitarea) {
	HitArea *ha = findHitAreaByID(hitarea);
	if (ha == NULL)
		return false;
	return (ha->flags & 0x40) == 0;
}

void SimonEngine::addNewHitArea(int id, int x, int y, int width, int height, int flags, int verb, Item *item_ptr) {
	HitArea *ha;
	delete_hitarea(id);

	ha = findEmptyHitArea();
	ha->x = x;
	ha->y = y;
	ha->width = width;
	ha->height = height;
	ha->flags = flags | 0x20;
	ha->id = ha->priority = id;
	ha->verb = verb;
	ha->item_ptr = item_ptr;

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

		ha = findHitAreaByID(id);
		if (ha == NULL)
			return;

		if (ha->flags & 0x40) {
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
				tmp->flags |= 8;
				video_toggle_colors(tmp, 0xd5, 0xd0, 0xd5, 0xA);
			}

			if (ha->flags & 2)
				video_toggle_colors(ha, 0xda, 0xd5, 0xd5, 5);
			else
				video_toggle_colors(ha, 0xdf, 0xda, 0xda, 0xA);

			ha->flags &= ~(2 + 8);
		} else {
			if (ha->id < 101)
				return;
			_mouseCursor = ha->id - 101;
			_needHitAreaRecalc++;
		}
		_currentVerbBox = ha;
	}
}

void SimonEngine::hitarea_leave(HitArea *ha) {
	if (!(getGameType() == GType_SIMON2)) {
		video_toggle_colors(ha, 0xdf, 0xd5, 0xda, 5);
	} else {
		video_toggle_colors(ha, 0xe7, 0xe5, 0xe6, 1);
	}
}

void SimonEngine::leaveHitAreaById(uint hitarea_id) {
	HitArea *ha = findHitAreaByID(hitarea_id);
	if (ha)
		hitarea_leave(ha);
}

void SimonEngine::checkUp(WindowBlock *window) {
	uint16 j, k;

	if (((_variableArray[31] - _variableArray[30]) == 40) && (_variableArray[31] > 52)) {
		k = (((_variableArray[31] / 52) - 2) % 3);
		j = k * 6;
		if (!is_hitarea_0x40_clear(j + 201)) {
			uint index = get_fcs_ptr_3_index(window);
			drawIconArray(index, window->iconPtr->itemRef, 0, window->iconPtr->classMask);
			loadSprite(4, 9, k + 34, 0, 0, 0);	
		}
	}
	if ((_variableArray[31] - _variableArray[30]) == 76) {
		k = ((_variableArray[31] / 52) % 3);
		j = k * 6;
		if (is_hitarea_0x40_clear(j + 201)) {
			loadSprite(4, 9, k + 31, 0, 0, 0);
			delete_hitarea(j + 201);
			delete_hitarea(j + 202);
			delete_hitarea(j + 203);
			delete_hitarea(j + 204);
			delete_hitarea(j + 205);
			delete_hitarea(j + 206);
		}
		_variableArray[31] -= 52;
		_iOverflow = 1;
	}
}

void SimonEngine::checkDown(WindowBlock *window) {
	uint16 j, k;

	if (((_variableArray[31] - _variableArray[30]) == 24) && (_iOverflow == 1)) {
		uint index = get_fcs_ptr_3_index(window);
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
			delete_hitarea(j + 201);
			delete_hitarea(j + 202);
			delete_hitarea(j + 203);
			delete_hitarea(j + 204);
			delete_hitarea(j + 205);
			delete_hitarea(j + 206);
		}
	}
}

void SimonEngine::inventoryUp(WindowBlock *window) {
	if (getGameType() == GType_FF) {
		_marks = 0;
		checkUp(window);
		loadSprite(4, 9 ,21 ,0 ,0, 0);	
		while(1) {
			if (_currentBoxNumber != 0x7FFB || !_leftButtonDown)
				break;
			checkUp(window);
		}
		o_waitForMark(2);
		checkUp(window);
		o_sync(922);
		o_waitForMark(1);
		checkUp(window);
	} else {
		if (window->iconPtr->line == 0)
			return;

		mouseOff();
		uint index = get_fcs_ptr_3_index(window);
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
			if (_currentBoxNumber != 0x7FFC || !_leftButtonDown)
				break;
			checkDown(window);
		}
		o_waitForMark(2);
		checkDown(window);
		o_sync(924);
		o_waitForMark(1);
		checkDown(window);
	} else {
		mouseOff();
		uint index = get_fcs_ptr_3_index(window);
		drawIconArray(index, window->iconPtr->itemRef, window->iconPtr->line + 1, window->iconPtr->classMask);
		mouseOn();
	}
}

void SimonEngine::setup_hitarea_from_pos(uint x, uint y, uint mode) {
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
		if (ha->flags & 0x20) {
			if (!(ha->flags & 0x40)) {
				if (x_ >= ha->x && y_ >= ha->y &&
						x_ - ha->x < ha->width && y_ - ha->y < ha->height && priority <= ha->priority) {
					priority = ha->priority;
					best_ha = ha;
				} else {
					if (ha->flags & 2) {
						hitarea_leave(ha);
						ha->flags &= ~2;
					}
				}
			} else {
				ha->flags &= ~2;
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

	if (best_ha->flags & 4) {
		clearName();
	} else if (best_ha != _lastNameOn) {
		displayName(best_ha);
	}

	if (best_ha->flags & 8 && !(best_ha->flags & 2)) {
		hitarea_leave(best_ha);
		best_ha->flags |= 2;
	}

	return;
}

void SimonEngine::displayName(HitArea *ha) {
	bool result;

	hitareaChangedHelper();
	if (ha->flags & 1) {
		result = printTextOf(ha->flags >> 8);
	} else {
		result = printNameOf(ha->item_ptr);
	}

	if (result)
		_lastNameOn = ha;
}

bool SimonEngine::printTextOf(uint a) {
	uint x;
	const byte *string_ptr;

	if (getGameType() == GType_SIMON2) {
		if (getBitFlag(79)) {
			Subroutine *sub;
			_variableArray[84] = a;
			sub = getSubroutineByID(5003);
			if (sub != NULL)
				startSubroutineEx(sub);
			return true;
		}
	}

	if (a >= 20)
		return false;

	string_ptr = getStringPtrByID(_stringIdArray2[a]);
	// Arisme : hack for long strings in the French version
	if ((strlen((const char*)string_ptr) - 1) <= 53)
		x = (53 - (strlen((const char *)string_ptr) - 1)) * 3;
	else
		x = 0;
	showActionString(x, string_ptr);

	return true;
}

bool SimonEngine::printNameOf(Item *item) {
	Child2 *child2;
	uint x;
	const byte *string_ptr;

	if (item == 0 || item == _dummyItem2 || item == _dummyItem3)
		return false;

	child2 = (Child2 *)findChildOfType(item, 2);
	if (child2 == NULL)
		return false;

	string_ptr = getStringPtrByID(child2->string_id);
	// Arisme : hack for long strings in the French version
	if ((strlen((const char*)string_ptr) - 1) <= 53)
		x = (53 - (strlen((const char *)string_ptr) - 1)) * 3;
	else
		x = 0;
	showActionString(x, string_ptr);

	return true;
}

} // End of namespace Simon
