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

#include "ultima/nuvie/keybinding/key_actions.h"
#include "ultima/nuvie/keybinding/keys.h"
#include "ultima/nuvie/core/events.h"
#include "ultima/nuvie/core/game.h"
#include "ultima/nuvie/core/party.h"
#include "ultima/nuvie/core/player.h"
#include "ultima/nuvie/views/view_manager.h"
#include "ultima/nuvie/gui/widgets/msg_scroll.h"
#include "ultima/nuvie/views/inventory_view.h"
#include "ultima/nuvie/gui/widgets/command_bar.h"
#include "ultima/nuvie/views/actor_view.h"
#include "ultima/nuvie/gui/widgets/map_window.h"
#include "ultima/nuvie/core/effect.h"
#include "ultima/nuvie/core/egg_manager.h"
#include "ultima/nuvie/screen/screen.h"
#include "ultima/nuvie/gui/gui.h"
#include "ultima/nuvie/sound/sound_manager.h"
#include "ultima/nuvie/gui/widgets/background.h"
#include "ultima/nuvie/conf/configuration.h"
#include "ultima/nuvie/misc/u6_misc.h"
#include "ultima/nuvie/nuvie.h"

namespace Ultima {
namespace Nuvie {

#define GAME Game::get_game()
#define EVENT Game::get_game()->get_event()
#define PARTY Game::get_game()->get_party()
#define PLAYER Game::get_game()->get_player()
#define VIEW_MANAGER Game::get_game()->get_view_manager()
#define INVENTORY_VIEW Game::get_game()->get_view_manager()->get_inventory_view()
#define ACTOR_VIEW Game::get_game()->get_view_manager()->get_actor_view()
#define MAP_WINDOW Game::get_game()->get_map_window()

void ActionWalkWest(int const *params) {
	EVENT->move(-1, 0);
}

void ActionWalkEast(int const *params) {
	EVENT->move(1, 0);
}

void ActionWalkNorth(int const *params) {
	EVENT->move(0, -1);
}

void ActionWalkSouth(int const *params) {
	EVENT->move(0, 1);
}

void ActionWalkNorthEast(int const *params) {
	EVENT->move(1, -1);
}

void ActionWalkSouthEast(int const *params) {
	EVENT->move(1, 1);
}

void ActionWalkNorthWest(int const *params) {
	EVENT->move(-1, -1);
}

void ActionWalkSouthWest(int const *params) {
	EVENT->move(-1, 1);
}

void ActionCast(int const *params) {
	if (GAME->get_game_type() != NUVIE_GAME_U6) {
		GAME->get_keybinder()->handle_wrong_key_pressed();
		return;
	} else if (PLAYER->is_in_vehicle())
		EVENT->display_not_aboard_vehicle();
	else
		EVENT->newAction(CAST_MODE);
}

void ActionLook(int const *params) {
	EVENT->newAction(LOOK_MODE);
}

void ActionTalk(int const *params) {
	EVENT->newAction(TALK_MODE);
}

void ActionUse(int const *params) {
	EVENT->newAction(USE_MODE);
}

void ActionGet(int const *params) {
	EVENT->newAction(GET_MODE);
}

void ActionMove(int const *params) {
	EVENT->newAction(PUSH_MODE);
}

void ActionDrop(int const *params) {
	EVENT->set_drop_from_key(true);
	EVENT->newAction(DROP_MODE);
}

void ActionToggleCombat(int const *params) {
	EVENT->newAction(COMBAT_MODE);
}

void ActionAttack(int const *params) {
	EVENT->newAction(ATTACK_MODE);
}

void ActionRest(int const *params) {
	EVENT->newAction(REST_MODE);
}

void ActionMultiUse(int const *params) {
	if (EVENT->get_mode() == ATTACK_MODE)
		EVENT->doAction();
	else
		EVENT->newAction(MULTIUSE_MODE);
}

static const sint8 SE_command_tbl[] = {6, -1, 4, 5, 1, 2, 0, 3, 7, 8}; // convert U6 indexes
static const sint8 MD_command_tbl[] = {0, -1, 1, 2, 3, 4, 5, 6, -1, 7};

void ActionSelectCommandBar(int const *params) {
	CommandBar *cb = GAME->get_command_bar();
	if (params[0] < 0 || params[0] > 9) // deactivate
		cb->select_action(-1);
	else if (GAME->get_game_type() == NUVIE_GAME_U6)
		cb->select_action(params[0]);
	else if (GAME->get_game_type() == NUVIE_GAME_SE)
		cb->select_action(SE_command_tbl[params[0]]);
	else // MD
		cb->select_action(MD_command_tbl[params[0]]);
}

void ActionSelectNewCommandBar(int const *params) {
	CommandBar *cb = GAME->get_new_command_bar();
	if (!cb)
		return;

	cb->grab_focus();
	cb->Show();
	GAME->get_keybinder()->set_enable_joy_repeat(false);
}

void ActionDollGump(int const *params) {
	if (EVENT->is_looking_at_spellbook()) {
		EVENT->cancelAction();
		return;
	}
	if (params[0] > 0) {
		Actor *party_member = PARTY->get_actor(params[0] - 1);
		if (party_member)
			VIEW_MANAGER->open_doll_view(party_member);
	} else
		VIEW_MANAGER->open_doll_view(NULL);
}

void ActionShowStats(int const *params) {
	if (EVENT->using_control_cheat())
		return;
	Actor *party_member = PARTY->get_actor(params[0] - 1);
	if (party_member == NULL)
		return;
	if (!GAME->is_new_style()) {
		ACTOR_VIEW->set_party_member(params[0] - 1);
		VIEW_MANAGER->set_actor_mode();
	} else
		VIEW_MANAGER->open_portrait_gump(party_member);
}

void ActionInventory(int const *params) {
	if (EVENT->is_looking_at_spellbook()) {
		EVENT->cancelAction();
		return;
	}
	if (EVENT->using_control_cheat() || params[0] == 0)
		return;
	if (PARTY->get_party_size() >= params[0]) {
		if (!GAME->is_new_style()) {
			VIEW_MANAGER->set_inventory_mode();
			INVENTORY_VIEW->set_party_member(params[0] - 1);
		} else {
			VIEW_MANAGER->open_container_view(PARTY->get_actor(params[0] - 1));
		}
	}
}

void ActionPartyView(int const *params) {
	if (!EVENT->using_control_cheat())
		VIEW_MANAGER->set_party_mode();
}

void ActionNextPartyMember(int const *params) {
	if (EVENT->using_control_cheat())
		return;
	if (!GAME->is_new_style()) {
		if (VIEW_MANAGER->get_current_view() == ACTOR_VIEW) {
			uint8 party_num = ACTOR_VIEW->get_party_member_num();
			if (PARTY->get_party_size() >= party_num + 2)
				ACTOR_VIEW->set_party_member(party_num + 1);
		} else if (!INVENTORY_VIEW->is_picking_pocket()) {
			uint8 party_num = INVENTORY_VIEW->get_party_member_num();
			if (PARTY->get_party_size() >= party_num + 2
			        && INVENTORY_VIEW->set_party_member(party_num + 1))
				VIEW_MANAGER->set_inventory_mode();
		}
	}
}

void ActionPreviousPartyMember(int const *params) {
	if (EVENT->using_control_cheat())
		return;
	if (!GAME->is_new_style()) {
		if (VIEW_MANAGER->get_current_view() == ACTOR_VIEW) {
			uint8 party_num = ACTOR_VIEW->get_party_member_num();
			if (party_num >= 1)
				ACTOR_VIEW->set_party_member(party_num - 1);
		} else if (!INVENTORY_VIEW->is_picking_pocket()) {
			uint8 party_num = INVENTORY_VIEW->get_party_member_num();
			if (party_num >= 1 && INVENTORY_VIEW->set_party_member(party_num - 1))
				VIEW_MANAGER->set_inventory_mode();
		}
	}
}

void ActionHome(int const *params) {
	if (EVENT->using_control_cheat())
		return;
	if (!GAME->is_new_style()) {
		if (VIEW_MANAGER->get_current_view() == ACTOR_VIEW)
			ACTOR_VIEW->set_party_member(0);
		else if (!INVENTORY_VIEW->is_picking_pocket() && INVENTORY_VIEW->set_party_member(0))
			VIEW_MANAGER->set_inventory_mode();
	}
}

void ActionEnd(int const *params) {
	if (EVENT->using_control_cheat())
		return;
	if (!GAME->is_new_style()) {
		uint8 mem_num = PARTY->get_party_size() - 1;
		if (VIEW_MANAGER->get_current_view() == ACTOR_VIEW)
			ACTOR_VIEW->set_party_member(mem_num);
		else if (!INVENTORY_VIEW->is_picking_pocket()) {
			if (VIEW_MANAGER->get_current_view() != INVENTORY_VIEW)
				VIEW_MANAGER->set_inventory_mode();
			if (INVENTORY_VIEW->set_party_member(mem_num))
				VIEW_MANAGER->set_inventory_mode();
		}
	}
}

void ActionToggleView(int const *params) {
	if (!GAME->is_new_style()) {
		if (VIEW_MANAGER->get_current_view() == ACTOR_VIEW)
			VIEW_MANAGER->set_inventory_mode();
		else if (VIEW_MANAGER->get_current_view() == INVENTORY_VIEW && !INVENTORY_VIEW->is_picking_pocket())
			VIEW_MANAGER->set_actor_mode();
	}
}

void ActionSoloMode(int const *params) {
	if (params[0] == 0) {
		if (PLAYER->in_party_mode())
			EVENT->solo_mode(0);
		else {
			uint8 party_size = PARTY->get_party_size() - 1;
			sint8 new_mem_num = PARTY->get_member_num(PLAYER->get_actor()) + 1;
			if (new_mem_num > party_size) {
				if (!EVENT->party_mode())
					EVENT->solo_mode(0); // failed so start over again
			} else
				EVENT->solo_mode((uint32)new_mem_num);
		}
		return;
	}
	if (EVENT->get_mode() == INPUT_MODE)
		EVENT->select_party_member(params[0] - 1);
	else if (PLAYER->is_in_vehicle())
		EVENT->display_not_aboard_vehicle();
	else
		EVENT->solo_mode(params[0] - 1);
}

void ActionPartyMode(int const *params) {
	if (EVENT->get_mode() == MOVE_MODE)
		EVENT->party_mode();
	else
		EVENT->cancelAction();
}

void ActionSaveDialog(int const *params) {
	g_engine->saveGameDialog();
}

void ActionLoadLatestSave(int const *params) {
	EVENT->close_gumps();
	GAME->get_scroll()->display_string("Load game!\n");
	g_engine->loadLatestSave();
}

void ActionQuickSave(int const *params) {
	g_engine->quickSave(params[0], false);
}

void ActionQuickLoad(int const *params) {
	g_engine->quickSave(params[0], true);
}

void ActionQuitDialog(int const *params) {
	if (!EVENT) { // intro or used view ending command line
	} // FIXME need way to quit
	else
		EVENT->quitDialog();
}

void ActionQuitNODialog(int const *params) {
	GAME->quit();
}

void ActionGameMenuDialog(int const *params) {
	EVENT->gameMenuDialog();
}

void ActionToggleFullscreen(int const *params) {
	if (!GAME->get_screen()->toggle_fullscreen())
		new TextEffect("Couldn't toggle fullscreen");
	else
		GAME->get_gui()->force_full_redraw();
}

void ActionToggleCursor(int const *params) {
	if (!GAME->is_new_style()) {
		if (EVENT->get_input()->select_from_inventory == false)
			EVENT->moveCursorToInventory();
		else // cursor is on inventory
			EVENT->moveCursorToMapWindow(true);
	} else {
		Actor *actor;
		if (PLAYER->is_in_vehicle())
			actor = PARTY->get_actor(0);
		else
			actor = PLAYER->get_actor();
		VIEW_MANAGER->open_container_view(actor);
	}
}

void ActionToggleCombatStrategy(int const *params) {
	if (!GAME->is_new_style() && VIEW_MANAGER->get_current_view() == INVENTORY_VIEW)
		INVENTORY_VIEW->simulate_CB_callback();
}

void ActionToggleFps(int const *params) {
	if (EVENT)
		EVENT->toggleFpsDisplay();
}

void ActionToggleAudio(int const *params) {
	bool audio = !GAME->get_sound_manager()->is_audio_enabled();
	GAME->get_sound_manager()->set_audio_enabled(audio);
	new TextEffect(audio ? "Audio enabled" : "Audio disabled");
}

void ActionToggleMusic(int const *params) {
	bool music = !GAME->get_sound_manager()->is_music_enabled();
	GAME->get_sound_manager()->set_music_enabled(music);
	new TextEffect(music ? "Music enabled" : "Music disabled");
}

void ActionToggleSFX(int const *params) {
	bool sfx = !GAME->get_sound_manager()->is_sfx_enabled();
	GAME->get_sound_manager()->set_sfx_enabled(sfx);
	new TextEffect(sfx ? "Sfx enabled" : "Sfx disabled");
}

void ActionToggleOriginalStyleCommandBar(int const *params) {
	if (Game::get_game()->is_orig_style())
		return;
	CommandBar *cb = GAME->get_command_bar();
	Configuration *config = GAME->get_config();
	bool hide = cb->Status() == WIDGET_VISIBLE;
	if (hide) {
		cb->Hide();
		GAME->get_screen()->clear(cb->X(), cb->Y(), cb->W(), cb->H(), NULL); // can be over null background so need to not leave corruption
		GAME->get_screen()->update(cb->X(), cb->Y(), cb->W(), cb->H());
	} else {
		cb->Show();
	}
	config->set(config_get_game_key(config) + "/show_orig_style_cb", !hide);
	config->write();
}

void ActionDoAction(int const *params) {
	EVENT->doAction();
}

void ActionCancelAction(int const *params) {
	EVENT->cancelAction();
}

void ActionMsgScrollUP(int const *params) {
	if (!GAME->is_new_style())
		GAME->get_scroll()->page_up();
	else
		GAME->get_scroll()->move_scroll_up();
}

void ActionMsgScrollDown(int const *params) {
	if (!GAME->is_new_style())
		GAME->get_scroll()->page_down();
	else
		GAME->get_scroll()->move_scroll_down();
}

void ActionShowKeys(int const *params) {
	GAME->get_keybinder()->ShowKeys();
}

void ActionDecreaseDebug(int const *params) {
	DEBUG(0, LEVEL_EMERGENCY, "!!decrease!!\n");
}
void ActionIncreaseDebug(int const *params) {
	DEBUG(0, LEVEL_EMERGENCY, "!!increase!!\n");
}

void ActionCloseGumps(int const *params) {
	EVENT->close_gumps();
}

void ActionUseItem(int const *params) {
	if (EVENT->get_mode() != MOVE_MODE && EVENT->get_mode() != EQUIP_MODE)
		return;
	uint16 obj_n = params[0] > 0 ? params[0] : 0;
#if 0 // need to increase c_maxparams to 5 to use this many parameters
	uint8 qual = params[1] > 0 ? params[1] : 0;
	bool match_qual = params[2] == 1 ? true : false;
	uint8 frame_n = params[3] > 0 ? params[3] : 0;
	bool match_frame_n = params[4] == 1 ? true : false;
#else
	uint8 qual = 0;
	bool match_qual = false;
	uint8 frame_n = 0;
	bool match_frame_n = false;
#endif

	// try player first
	Obj *obj = PLAYER->get_actor()->inventory_get_object(obj_n, qual, match_qual, frame_n, match_frame_n);
	if (!obj && !EVENT->using_control_cheat())
		obj =  PARTY->get_obj(obj_n, qual, match_qual, frame_n, match_frame_n);
	if (obj) {
		GAME->get_scroll()->display_string("Use-", MSGSCROLL_NO_MAP_DISPLAY);
		EVENT->set_mode(USE_MODE);
		EVENT->use(obj);
	}
	// printf("ActionUseItem obj_n = %d, qual = %d, match_qual = %s, frame_n = %d, match_frame_n = %s\n", obj_n, qual, match_qual ? "true": "false", frame_n, match_frame_n ? "true": "false");
}

void ActionShowEggs(int const *params) {
	bool show_eggs = !GAME->get_obj_manager()->is_showing_eggs();
	GAME->get_obj_manager()->set_show_eggs(show_eggs);
	GAME->get_egg_manager()->set_egg_visibility(show_eggs);
	new TextEffect(show_eggs ? "Showing eggs" : "Eggs invisible");
}

void ActionToggleHackmove(int const *params) {
	bool hackmove = !GAME->using_hackmove();
	GAME->set_hackmove(hackmove);
	new TextEffect(hackmove ? "Hack move enabled" : "Hack move disabled");
}

void ActionToggleEggSpawn(int const *params) {
	EggManager *egg_manager = GAME->get_obj_manager()->get_egg_manager();
	bool spawning = !egg_manager->is_spawning_actors();
	egg_manager->set_spawning_actors(spawning);
	new TextEffect(spawning ? "Will spawn actors" : "Won't spawn actors");
}

void ActionToggleUnlimitedCasting(int const *params) {
	bool unlimited = !GAME->has_unlimited_casting();
	GAME->set_unlimited_casting(unlimited);
	new TextEffect(unlimited ? "Unlimited casting" : "Normal casting");
}

void ActionToggleNoDarkness(int const *params) {
	bool no_darkness = GAME->get_screen()->toggle_darkness_cheat();
	new TextEffect(no_darkness ? "No more darkness" : "Normal lighting");
}

void ActionTogglePickpocket(int const *params) {
	bool pickpocket = (EVENT->using_pickpocket_cheat = !EVENT->using_pickpocket_cheat);
	new TextEffect(pickpocket ? "Pickpocket mode" : "Pickpocket disabled");
}

void ActionToggleGodMode(int const *params) {
	bool god_mode = GAME->toggle_god_mode();
	new TextEffect(god_mode ? "God mode enabled" : "God mode disabled");
}

void ActionToggleEthereal(int const *params) {
	bool ethereal = !GAME->is_ethereal();
	GAME->set_ethereal(ethereal);
	PARTY->set_ethereal(ethereal);
	new TextEffect(ethereal ? "Ethereal movement" : "Normal movement");
}

void ActionToggleX_Ray(int const *params) {
	bool x_ray = MAP_WINDOW->get_x_ray_view() <= X_RAY_OFF;
	MAP_WINDOW->set_x_ray_view(x_ray ? X_RAY_CHEAT_ON : X_RAY_OFF, true);
	new TextEffect(x_ray ? "X-ray mode" : "X-ray mode off");
}

void ActionHealParty(int const *params) {
	PARTY->heal();
	PARTY->cure();
	new TextEffect("Party healed");
}

void ActionTeleportToCursor(int const *params) {
	MAP_WINDOW->teleport_to_cursor();
}

void ActionToggleCheats(int const *params) {
	bool cheats = !GAME->are_cheats_enabled();
	GAME->set_cheats_enabled(cheats);
	new TextEffect(cheats ? "Cheats enabled" : "Cheats disabled");

	if (GAME->is_ethereal()) // doesn't change the bool's value
		PARTY->set_ethereal(cheats);
	if (GAME->get_obj_manager()->is_showing_eggs())
		GAME->get_egg_manager()->set_egg_visibility(cheats);

	X_RayType xray = MAP_WINDOW->get_x_ray_view();
	if (xray == X_RAY_CHEAT_OFF)
		MAP_WINDOW->set_x_ray_view(X_RAY_CHEAT_ON);
	else if (xray == X_RAY_CHEAT_ON)
		MAP_WINDOW->set_x_ray_view(X_RAY_CHEAT_OFF);
}

void ActionDoNothing(int const *params) {
}

} // End of namespace Nuvie
} // End of namespace Ultima
