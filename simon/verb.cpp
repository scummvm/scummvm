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
 * $Header$
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

void SimonEngine::defocusHitarea() {
	HitArea *last;
	HitArea *ha;

	if (_game & GF_SIMON2) {
		if (_bitArray[4] & 0x8000) {
			o_unk_120(202);
			_lastHitArea2Ptr = NULL;
			return;
		}
	}

	last = _hitAreaPtr5;

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
		case 21: verb_prep_names = russian_verb_prep_names; break;
		case 20: verb_prep_names = hebrew_verb_prep_names; break;
		case  5: verb_prep_names = spanish_verb_prep_names; break;
		case  3: verb_prep_names = italian_verb_prep_names; break;
		case  2: verb_prep_names = french_verb_prep_names; break;
		case  1: verb_prep_names = german_verb_prep_names; break;
		default: verb_prep_names = english_verb_prep_names; break;
		}
		CHECK_BOUNDS(hitarea_id, english_verb_prep_names);
		txt = verb_prep_names[hitarea_id];
	} else {
		switch (_language) {
		case 21: verb_names = russian_verb_names; break;
		case 20: verb_names = hebrew_verb_names; break;
		case  5: verb_names = spanish_verb_names; break;
		case  3: verb_names = italian_verb_names; break;
		case  2: verb_names = french_verb_names; break;
		case  1: verb_names = german_verb_names; break;
		default: verb_names = english_verb_names; break;
		}
		CHECK_BOUNDS(hitarea_id, english_verb_names);
		txt = verb_names[hitarea_id];
	}
	x = (53 - strlen(txt)) * 3;
	showActionString(x, (const byte *)txt);
}

void SimonEngine::showActionString(uint x, const byte *string) {
	FillOrCopyStruct *fcs;

	fcs = _fcsPtrArray3[1];
	if (fcs == NULL || fcs->text_color == 0)
		return;

	fcs->textColumn = x >> 3;
	fcs->textColumnOffset = x & 7;

	for (; *string; string++)
		video_putchar(fcs, *string);
}

void SimonEngine::hitareaChangedHelper() {
	FillOrCopyStruct *fcs;

	if (_game & GF_SIMON2) {
		if (_bitArray[4] & 0x8000)
			return;
	}

	fcs = _fcsPtrArray3[1];
	if (fcs != NULL && fcs->text_color != 0)
		video_fill_or_copy_from_3_to_2(fcs);

	_lastHitArea2Ptr = NULL;
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
			hitarea_proc_1();
	}
}

void SimonEngine::set_hitarea_x_y(uint hitarea, int x, int y) {
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
		if (ha == _lastHitArea2Ptr)
			defocusHitarea();
		_needHitAreaRecalc++;
	}
}

bool SimonEngine::is_hitarea_0x40_clear(uint hitarea) {
	HitArea *ha = findHitAreaByID(hitarea);
	if (ha == NULL)
		return false;
	return (ha->flags & 0x40) == 0;
}

void SimonEngine::addNewHitArea(int id, int x, int y, int width, int height, int flags, int unk3, Item *item_ptr) {
	HitArea *ha;
	delete_hitarea(id);

	ha = findEmptyHitArea();
	ha->x = x;
	ha->y = y;
	ha->width = width;
	ha->height = height;
	ha->flags = flags | 0x20;
	ha->id = ha->layer = id;
	ha->unk3 = unk3;
	ha->item_ptr = item_ptr;

	_needHitAreaRecalc++;
}

void SimonEngine::hitarea_proc_1() {
	uint id;
	HitArea *ha;

	if (_game & GF_SIMON2) {
		id = 2;
		if (!(_bitArray[4] & 0x8000))
			id = (_mouseY >= 136) ? 102 : 101;
	} else {
		id = (_mouseY >= 136) ? 102 : 101;
	}

	_hitAreaUnk4 = id;

	ha = findHitAreaByID(id);
	if (ha == NULL)
		return;

	if (ha->flags & 0x40) {
		_hitAreaUnk4 = 999;
		_hitAreaPtr5 = NULL;
	} else {
		_verbHitArea = ha->unk3;
		handle_verb_hitarea(ha);
	}
}

void SimonEngine::handle_verb_hitarea(HitArea *ha) {
	HitArea *tmp = _hitAreaPtr5;

	if (ha == tmp)
		return;

	if (!(_game & GF_SIMON2)) {
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
	_hitAreaPtr5 = ha;
}

void SimonEngine::hitarea_leave(HitArea *ha) {
	if (!(_game & GF_SIMON2)) {
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

void SimonEngine::handle_uparrow_hitarea(FillOrCopyStruct *fcs) {
	uint index;

	index = get_fcs_ptr_3_index(fcs);

	if (fcs->fcs_data->unk1 == 0)
		return;

	lock();
	fcs_unk_proc_1(index, fcs->fcs_data->item_ptr, fcs->fcs_data->unk1 - 1, fcs->fcs_data->unk2);
	unlock();
}

void SimonEngine::handle_downarrow_hitarea(FillOrCopyStruct *fcs) {
	uint index;

	index = get_fcs_ptr_3_index(fcs);

	lock();
	fcs_unk_proc_1(index, fcs->fcs_data->item_ptr, fcs->fcs_data->unk1 + 1, fcs->fcs_data->unk2);
	unlock();
}

void SimonEngine::setup_hitarea_from_pos(uint x, uint y, uint mode) {
	HitArea *best_ha;
	HitArea *ha = _hitAreas;
	uint count = ARRAYSIZE(_hitAreas);
	uint16 layer = 0;
	uint16 x_ = x;
	const uint16 y_ = y;

	if (_game & GF_SIMON2) {
		if (_bitArray[4] & 0x8000 || y < 134) {
			x_ += _scrollX * 8;
		}
	}

	best_ha = NULL;

	do {
		if (ha->flags & 0x20) {
			if (!(ha->flags & 0x40)) {
				if (x_ >= ha->x && y_ >= ha->y &&
						x_ - ha->x < ha->width && y_ - ha->y < ha->height && layer <= ha->layer) {
					layer = ha->layer;
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

	if (best_ha == NULL) {
		defocusHitarea();
		return;
	}

	if (mode != 0 && mode != 3) {
		_lastHitArea = best_ha;
		_variableArray[1] = x;
		_variableArray[2] = y;
	}

	if (best_ha->flags & 4) {
		defocusHitarea();
	} else if (best_ha != _lastHitArea2Ptr) {
		new_current_hitarea(best_ha);
	}

	if (best_ha->flags & 8 && !(best_ha->flags & 2)) {
		hitarea_leave(best_ha);
		best_ha->flags |= 2;
	}

	return;
}

void SimonEngine::new_current_hitarea(HitArea *ha) {
	bool result;

	hitareaChangedHelper();
	if (ha->flags & 1) {
		result = hitarea_proc_2(ha->flags >> 8);
	} else {
		result = hitarea_proc_3(ha->item_ptr);
	}

	if (result)
		_lastHitArea2Ptr = ha;
}

bool SimonEngine::hitarea_proc_2(uint a) {
	uint x;
	const byte *string_ptr;

	if (_game & GF_SIMON2) {
		if (_bitArray[4] & 0x8000) {
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

bool SimonEngine::hitarea_proc_3(Item *item) {
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
