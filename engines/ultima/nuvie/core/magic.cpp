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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/misc/u6_misc.h"
#include "ultima/nuvie/core/party.h"
#include "ultima/nuvie/gui/widgets/msg_scroll.h"
#include "ultima/nuvie/conf/configuration.h"
#include "ultima/nuvie/files/nuvie_io_file.h"
#include "ultima/nuvie/usecode/usecode.h"
#include "ultima/nuvie/gui/gui.h"
#include "ultima/nuvie/gui/gui_yes_no_dialog.h"
#include "ultima/nuvie/gui/widgets/console.h"
#include "ultima/nuvie/actors/actor.h"
#include "ultima/nuvie/actors/actor_manager.h"
#include "ultima/nuvie/core/obj_manager.h"
#include "ultima/nuvie/views/view_manager.h"
#include "ultima/nuvie/views/spell_view.h"
#include "ultima/nuvie/sound/sound_manager.h"
#include "ultima/nuvie/core/u6_objects.h"
#include "ultima/nuvie/core/magic.h"
#include "ultima/nuvie/core/game.h"
#include "ultima/nuvie/core/game_clock.h"
#include "ultima/nuvie/misc/u6_llist.h"
#include "ultima/nuvie/core/effect.h"
#include "ultima/nuvie/core/weather.h"
#include "ultima/nuvie/script/script.h"

namespace Ultima {
namespace Nuvie {

/* Syllable            Meaning     Syllable            Meaning
 * An .......... Negate/Dispel     Nox ................ Poison
 * Bet ................. Small     Ort ................. Magic
 * Corp ................ Death     Por ......... Move/Movement
 * Des ............ Lower/Down     Quas ............. Illusion
 * Ex ................ Freedom     Rel ................ Change
 * Flam ................ Flame     Sanct .. Protect/Protection
 * Grav ......... Energy/Field     Tym .................. Time
 * Hur .................. Wind     Uus .............. Raise/Up
 * In ...... Make/Create/Cause     Vas ................. Great
 * Jux ...... Danger/Trap/Harm     Wis ........ Know/Knowledge
 * Kal ......... Summon/Invoke     Xen .............. Creature
 * Lor ................. Light     Ylem ............... Matter
 * Mani ......... Life/Healing     Zu .................. Sleep
 */

const char *syllable[26] = {"An ", "Bet ", "Corp ", "Des ", "Ex ", "Flam ", "Grav ", "Hur ", "In ", "Jux ", "Kal ", "Lor ", "Mani ", "Nox ", "Ort ", "Por ", "Quas ", "Rel ", "Sanct ", "Tym ", "Uus ", "Vas ", "Wis ", "Xen ", "Ylem ", "Zu "};
const char *reagent[8] = {"mandrake root", "nightshade", "black pearl", "blood moss", "spider silk", "garlic", "ginseng", "sulfurous ash"}; // check names
const int obj_n_reagent[8] = {OBJ_U6_MANDRAKE_ROOT, OBJ_U6_NIGHTSHADE, OBJ_U6_BLACK_PEARL, OBJ_U6_BLOOD_MOSS, OBJ_U6_SPIDER_SILK, OBJ_U6_GARLIC, OBJ_U6_GINSENG, OBJ_U6_SULFUROUS_ASH};


Magic::Magic() {
	event = NULL;
	target_object = NULL;
	magic_script = NULL;
	spellbook_obj = NULL;
	state = 0;

	for (uint16 index = 0; index < 256; index++) spell[index] = NULL;
	clear_cast_buffer();
}

Magic::~Magic() {
	for (uint16 index = 0; index < 256; index++) delete(spell[index]);
}

bool Magic::init(Events *evt) {
	event = evt;
	return read_spell_list();
}

bool Magic::read_spell_list() {
	return Game::get_game()->get_script()->call_magic_get_spell_list(spell);
}

Obj *Magic::book_equipped() {
	// book(s) equipped? Maybe should check all locations?
	Obj *obj = NULL;
	Actor *caster = event->player->get_actor();

	obj = caster->inventory_get_readied_object(ACTOR_ARM);
	if (obj && obj->obj_n == OBJ_U6_SPELLBOOK)
		return obj;

	obj = caster->inventory_get_readied_object(ACTOR_ARM_2);

	if (obj && obj->obj_n == OBJ_U6_SPELLBOOK)
		return obj;

	return NULL;
}

bool Magic::start_new_spell() {
	spellbook_obj = book_equipped();

	if (Game::get_game()->get_clock()->get_timer(GAMECLOCK_TIMER_U6_STORM) > 0 && !Game::get_game()->has_unlimited_casting()) {
		event->scroll->display_string("No magic at this time!\n\n");
	} else if (spellbook_obj != NULL) {
		state = MAGIC_STATE_SELECT_SPELL;
		clear_cast_buffer();
		event->close_gumps();
		Game::get_game()->get_view_manager()->set_spell_mode(event->player->get_actor(), spellbook_obj);
		Game::get_game()->get_view_manager()->get_spell_view()->grab_focus();
		return true;
	} else
		event->scroll->display_string("\nNo spellbook is readied.\n\n");

	state = MAGIC_STATE_READY;
	return false;
}

bool Magic::cast() {
	if (magic_script != NULL)
		return false;

	Game::get_game()->get_view_manager()->close_spell_mode();


	cast_buffer_str[cast_buffer_len] = '\0';
	DEBUG(0, LEVEL_DEBUGGING, "Trying to cast '%s'\n", cast_buffer_str);
	/* decode the invocation */
	// FIXME? original allows random order of syllables, do we want that?
	// easy enough to sort invocations, but would somewhat limit custom spells
	uint16 index;

	if (cast_buffer_len != 0) {
		for (index = 0; index < 256; index++) {
			if (spell[index] == NULL) {
				continue;
			}
			if (!strcmp(spell[index]->invocation, cast_buffer_str)) {
				break;
			}
		}
	} else {
		sint16 view_spell = Game::get_game()->get_view_manager()->get_spell_view()->get_selected_spell();
		if (view_spell < 256 && view_spell != -1)
			index = (uint16)view_spell;
		else
			index = 256;
	}

	if (index >= 256) {
		DEBUG(0, LEVEL_DEBUGGING, "didn't find spell in spell list\n");
		event->scroll->display_string("\nThat spell is not in thy spellbook!\n");
		return false;
	}
//20110701 Pieter Luteijn: add an assert(spell[index]) to be sure it's not NULL?
	if (cast_buffer_len != 0) {
		event->scroll->display_string("\n(");
		event->scroll->display_string(spell[index]->name);
		event->scroll->display_string(")\n");
	} else {
		event->scroll->display_string(spell[index]->name);
		event->scroll->display_string("\n\"");
		display_spell_incantation(index);
		event->scroll->display_string("\"\n");
	}

	if (Game::get_game()->has_unlimited_casting()) {
		cast_spell_directly(index);
		return true;
	}

	/* debug block */
	DEBUG(0, LEVEL_DEBUGGING, "matched spell #%d\n", index);
	DEBUG(0, LEVEL_DEBUGGING, "name: %s\n", spell[index]->name);
	DEBUG(0, LEVEL_DEBUGGING, "reagents: ");
	const char *comma = "";
	for (uint8 shift = 0; shift < 8; shift++) {
		if (1 << shift & spell[index]->reagents) {
			DEBUG(1, LEVEL_DEBUGGING, "%s%s", comma, reagent[shift]);
			comma = ", ";
		}
	}
	DEBUG(1, LEVEL_DEBUGGING, "\n");
	//DEBUG(0,LEVEL_DEBUGGING,"script: %s\n",spell[index]->script);
	/* end debug block */


	if (Game::get_game()->user_paused()) //event->mode == WAIT_MODE)
		return false;

	// book(s) equipped? Maybe should check all locations?
	Actor *caster = event->player->get_actor();
	Obj *right = caster->inventory_get_readied_object(ACTOR_ARM);
	Obj *left = caster->inventory_get_readied_object(ACTOR_ARM_2);
	uint8 books = 0;
	if (right != NULL && right->obj_n == OBJ_U6_SPELLBOOK) {
		books += 1;
	};
	if (left != NULL && left->obj_n == OBJ_U6_SPELLBOOK) {
		books += 2;
	};

	if (right && right->obj_n != OBJ_U6_SPELLBOOK)
		right = NULL;
	if (left && left->obj_n !=  OBJ_U6_SPELLBOOK)
		left = NULL;

	if (right == NULL && left == NULL) {
		event->scroll->display_string("\nNo spellbook is readied.\n");
		return false;
	}

	// any spells available?
	uint32 spells = 0;
	if ((books & 1) && right->container) { // hmm, relying on shortcut logic here.
		spells = right->container->count();
	}
	if ((books & 2) && left->container) {
		spells += left->container->count();
	}
	if (!spells) {
		event->scroll->display_string("\nNo spells in the spellbook.\n");
		return false;
	}

	// spell (or catch all spell 255) in (one of the) book(s)?
	if (spellbook_has_spell(right, index) == false && spellbook_has_spell(left, index) == false) {
		event->scroll->display_string("\nThat spell is not in thy spellbook!\n");
		return false;
	}

	// level of caster sufficient
	uint8 spell_level = MIN(8, (index / 16) + 1);
	if (caster->get_level() < spell_level) {
		event->scroll->display_string("\nYour level is not high enough.\n");
		return false;
	}

	// enough Magic Points available
	if (caster->get_magic() < spell_level) {
		event->scroll->display_string("\nNot enough magic points.\n");
		return false;
	}


	// reagents available
	for (uint8 shift = 0; shift < 8; shift++) {
		if (1 << shift & spell[index]->reagents) {
			if (!caster->inventory_has_object(obj_n_reagent[shift], 0, false)) {
				DEBUG(0, LEVEL_DEBUGGING, "Didn't have %s\n", reagent[shift]);
				event->scroll->display_string("\nNo Reagents.\n");
				Game::get_game()->get_sound_manager()->playSfx(NUVIE_SFX_FAILURE);
				return false;
			}
			DEBUG(0, LEVEL_DEBUGGING, "Ok, has %s\n", reagent[shift]);
		}
	}

	/* TODO check all pre-requisites before continue */
	// 'spell failed' because of bad luck
	// anything else?

	// consume the reagents and magic points; we checked so they must be there.
	caster->set_magic(caster->get_magic() - spell_level); // add a MAX (0, here?

	for (uint8 shift = 0; shift < 8; shift++) {
		if (1 << shift & spell[index]->reagents) {
			// FIXME Although we just checked, maybe something is messed up, so we
			// should probably check that we're not passing NULL to delete_obj
			caster->inventory_del_object(obj_n_reagent[shift], 1, 0);
		}
	}

	cast_spell_directly(index);

	event->player->subtract_movement_points(spell_level * 3 + 10);

	return true;
}

void Magic::display_spell_incantation(uint8 index) {
	string incantation_str;
	for (uint8 i = 0; spell[index]->invocation[i] != '\0'; i++)
		incantation_str += syllable[spell[index]->invocation[i] - Common::KEYCODE_a];

	incantation_str.erase(incantation_str.size() - 1); // get rid of extra space at the end
	event->scroll->display_string(incantation_str);
}

void Magic::show_spell_description(uint8 index) {
	event->scroll->display_string(spell[index]->name);
	event->scroll->display_string("-");
	display_spell_incantation(index);
	display_ingredients(index);
}

void Magic::display_ingredients(uint8 index) {
	event->scroll->display_string("\nIngredients:\n");
	if (spell[index]->reagents == 0) {
		event->scroll->display_string("None\n\n");
		return;
	}
	string list;
	for (uint8 shift = 0; shift < 8; shift++) {
		if (1 << shift & spell[index]->reagents) {
			list += " ";
			list += reagent[shift];
			list += "\n";
		}
	}
	list += "\n";
	event->scroll->set_discard_whitespace(false);
	event->scroll->display_string(list);
	event->scroll->set_discard_whitespace(true);
}

void Magic::cast_spell_directly(uint8 spell_num) {
	string lua = "run_magic_script(\"";
	lua += spell[spell_num]->invocation;
	lua += "\")";

	magic_script = Game::get_game()->get_script()->new_thread_from_string(lua.c_str());

	if (magic_script)
		process_script_return(magic_script->start());
}

bool Magic::resume(MapCoord location) {
	if (magic_script) {
		process_script_return(magic_script->resume_with_location(location));
	}

	return true;
}

bool Magic::resume(uint8 dir) {
	if (magic_script) {
		process_script_return(magic_script->resume_with_direction(dir));
	}

	return true;
}

bool Magic::resume_with_spell_num(uint8 spell_num) {
	if (magic_script) {
		process_script_return(magic_script->resume_with_spell_num(spell_num));
	}

	return true;
}

bool Magic::resume(Obj *obj) {
	if (magic_script) {
		process_script_return(magic_script->resume_with_obj(obj));
	}

	return true;
}

bool Magic::resume() {
	if (magic_script) {
		process_script_return(magic_script->resume_with_nil());
	}

	return true;
}

bool Magic::spellbook_has_spell(Obj *book, uint8 spell_index) {
	if (!book)
		return false;

	if (book->find_in_container(OBJ_U6_SPELL, MAGIC_ALL_SPELLS, OBJ_MATCH_QUALITY) ||
	        book->find_in_container(OBJ_U6_SPELL, spell_index, OBJ_MATCH_QUALITY)) {
		return true;
	}

	return false;
}

bool Magic::process_script_return(uint8 ret) {
	Game::get_game()->get_view_manager()->close_all_gumps();
	if (ret == NUVIE_SCRIPT_ERROR) {
		delete magic_script;
		magic_script = NULL;
		return false;
	}

	uint32 nturns;
	uint8 *cb_msgid;
	switch (ret) {
	case NUVIE_SCRIPT_FINISHED :
		delete magic_script;
		magic_script = NULL;
		state = MAGIC_STATE_READY;
		break;
	case NUVIE_SCRIPT_GET_TARGET :
		state = MAGIC_STATE_ACQUIRE_TARGET;
		break;
	case NUVIE_SCRIPT_GET_DIRECTION :
		state = MAGIC_STATE_ACQUIRE_DIRECTION;
		break;
	case NUVIE_SCRIPT_GET_INV_OBJ :
		state = MAGIC_STATE_ACQUIRE_INV_OBJ;
		break;
	case NUVIE_SCRIPT_GET_OBJ :
		state = MAGIC_STATE_ACQUIRE_OBJ;
		break;
	case NUVIE_SCRIPT_GET_SPELL :
		state = MAGIC_STATE_ACQUIRE_SPELL;
		break;

	case NUVIE_SCRIPT_ADVANCE_GAME_TIME :
		nturns = magic_script->get_data();
		DEBUG(0, LEVEL_DEBUGGING, "Magic: Advance %d turns\n", nturns);
		cb_msgid = new uint8;
		*cb_msgid = NUVIE_SCRIPT_CB_ADV_GAME_TIME;
		new GameTimedCallback((CallBack *)this, cb_msgid, nturns);
		break;

	case NUVIE_SCRIPT_TALK_TO_ACTOR :
		state = MAGIC_STATE_TALK_TO_ACTOR;
		break;

	default :
		DEBUG(0, LEVEL_WARNING, "Unknown ScriptThread return code!\n");
		break;
	}

	return true;
}

Actor *Magic::get_actor_from_script() {
	if (magic_script && (state == MAGIC_STATE_ACQUIRE_INV_OBJ || state == MAGIC_STATE_TALK_TO_ACTOR))
		return Game::get_game()->get_actor_manager()->get_actor((uint8)magic_script->get_data());

	return NULL;
}

uint16 Magic::callback(uint16 msg, CallBack *caller, void *data) {
	if (msg == CB_DATA_READY) {
		if (event->input.type != EVENTINPUT_KEY)
			return 0;
		Common::KeyCode sym = event->input.key;

		if (state == MAGIC_STATE_SELECT_SPELL) {
			if (sym >= Common::KEYCODE_a && sym <= Common::KEYCODE_z) {
				if (cast_buffer_len < 4) {
					cast_buffer_str[cast_buffer_len++] = sym;
					event->scroll->display_string(syllable[sym - Common::KEYCODE_a]);
					return 1; // handled the event
				}
				return 1; // handled the event
			} else if (sym == Common::KEYCODE_BACKSPACE) {
				if (cast_buffer_len > 0) {
					cast_buffer_len--; // back up a syllable FIXME, doesn't handle automatically inserted newlines, so we need to keep track more. (THAT SHOULD BE DONE BY MSGSCROLL)
					size_t len = strlen(syllable[cast_buffer_str[cast_buffer_len] - Common::KEYCODE_a]);
					while (len--) event->scroll->remove_char();
					event->scroll->Display(true);
					return 1; // handled the event
				}
				return 1; // handled the event
			}
		} // MAGIC_STATE_SELECT_SPELL
		if (state == MAGIC_STATE_ACQUIRE_TARGET) {
			if (sym >= Common::KEYCODE_1 && sym <= Common::KEYCODE_9) {
				cast();//event->player->get_party()->get_actor(sym - 48-1));
				event->cancel_key_redirect();
				return 1; // handled the event
			}
		}

		// We must handle all keys even those we may not want or else
		// we'll lose input focus, except for these three which end
		// Casting. (besides, not handling all keys means they go back
		// to global which could start another action)
		if (event->input.action_key_type != DO_ACTION_KEY && event->input.action_key_type != CANCEL_ACTION_KEY)
			return 1;

		return 0;
	} else if (magic_script) {
		switch (msg) {
		case NUVIE_SCRIPT_GET_TARGET :
			process_script_return(magic_script->resume_with_location(MapCoord(200, 200, 0))); //FIXME need to get real loc.
			break;
		case NUVIE_SCRIPT_GET_DIRECTION :
			process_script_return(magic_script->resume_with_direction(NUVIE_DIR_N)); //FIXME need to get real dir.
			break;
		}
	}

	return 1;
}

} // End of namespace Nuvie
} // End of namespace Ultima
