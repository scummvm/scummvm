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
#include "ultima/nuvie/conf/configuration.h"
#include "ultima/nuvie/misc/u6_misc.h"
#include "ultima/nuvie/gui/gui.h"
#include "ultima/nuvie/views/view_manager.h"
#include "ultima/nuvie/actors/actor.h"
#include "ultima/nuvie/views/actor_view.h"
#include "ultima/nuvie/views/portrait_view.h"
#include "ultima/nuvie/views/inventory_view.h"
#include "ultima/nuvie/views/doll_view_gump.h"
#include "ultima/nuvie/views/container_view_gump.h"
#include "ultima/nuvie/views/portrait_view_gump.h"
#include "ultima/nuvie/views/sign_view_gump.h"
#include "ultima/nuvie/views/scroll_view_gump.h"
#include "ultima/nuvie/views/party_view.h"
#include "ultima/nuvie/views/spell_view.h"
#include "ultima/nuvie/views/spell_view_gump.h"
#include "ultima/nuvie/views/sun_moon_ribbon.h"
#include "ultima/nuvie/gui/widgets/map_window.h"
#include "ultima/nuvie/views/map_editor_view.h"
#include "ultima/nuvie/gui/widgets/msg_scroll.h"
#include "ultima/nuvie/core/party.h"
#include "ultima/nuvie/core/events.h"
#include "ultima/nuvie/portraits/portrait.h"
#include "ultima/nuvie/usecode/usecode.h"
#include "ultima/nuvie/files/nuvie_bmp_file.h"
#include "ultima/nuvie/views/md_sky_strip_widget.h"

namespace Ultima {
namespace Nuvie {

ViewManager::ViewManager(Configuration *cfg) {
	config = cfg;
	config->value("config/GameType", game_type);
	current_view = NULL;
	gui = NULL;
	font = NULL;
	tile_manager = NULL;
	obj_manager = NULL;
	party = NULL;
	portrait = NULL;
	actor_view = NULL;
	inventory_view = NULL;
	portrait_view = NULL;
	party_view = NULL;
	spell_view = NULL;
	doll_next_party_member = 0;
	ribbon = NULL;
	mdSkyWidget = NULL;
}

ViewManager::~ViewManager() {
// only delete the views that are not currently active
	if (current_view != actor_view)     delete actor_view;
	if (current_view != inventory_view) delete inventory_view;
	if (current_view != party_view)     delete party_view;
	if (current_view != portrait_view)  delete portrait_view;
	if (current_view != spell_view)  delete spell_view;

}

bool ViewManager::init(GUI *g, Font *f, Party *p, Player *player, TileManager *tm, ObjManager *om, Portrait *por) {
	gui = g;
	font = f;
	party = p;
	tile_manager = tm;
	obj_manager = om;
	portrait = por;

	uint16 x_off = Game::get_game()->get_game_x_offset();
	uint16 y_off = Game::get_game()->get_game_y_offset();
	if (Game::get_game()->is_original_plus())
		x_off += Game::get_game()->get_game_width() - 320;

	inventory_view = new InventoryView(config);
	inventory_view->init(gui->get_screen(), this, 176 + x_off, 8 + y_off, font, party, tile_manager, obj_manager);

	portrait_view = new PortraitView(config);
	portrait_view->init(176 + x_off, 8 + y_off, font, party, player, tile_manager, obj_manager, portrait);

	if (!Game::get_game()->is_new_style()) {
		//inventory_view = new InventoryView(config);
		//inventory_view->init(gui->get_screen(), this, 176+x_off,8+y_off, font, party, tile_manager, obj_manager);
		actor_view = new ActorView(config);
		actor_view->init(gui->get_screen(), this, 176 + x_off, 8 + y_off, font, party, tile_manager, obj_manager, portrait);

		party_view = new PartyView(config);
		if (game_type == NUVIE_GAME_U6) {
			party_view->init(this, 168 + x_off, 6 + y_off, font, party, player, tile_manager, obj_manager);
			spell_view = new SpellView(config);
		} else {
			party_view->init(this, 176 + x_off, 6 + y_off, font, party, player, tile_manager, obj_manager);
		}
		if (game_type == NUVIE_GAME_MD) {
			if (Game::get_game()->is_new_style() == false) {
				mdSkyWidget = new MDSkyStripWidget(config, Game::get_game()->get_clock(), player);
				mdSkyWidget->init(32 + x_off, 2 + y_off);
				gui->AddWidget(mdSkyWidget);
				if (Game::get_game()->is_original_plus())
					mdSkyWidget->Hide();
			}
		}
	} else {
		//inventory_view = new InventoryViewGump(config);
		//inventory_view->init(gui->get_screen(), this, 176+x_off,8+y_off, font, party, tile_manager, obj_manager);
		if (game_type == NUVIE_GAME_U6) {
			spell_view = new SpellViewGump(config);
			ribbon = new SunMoonRibbon(player, Game::get_game()->get_weather(), tile_manager);
			ribbon->init(gui->get_screen());
			gui->AddWidget(ribbon);
			ribbon->Hide(); //will be shown on first call to update()
		}
	}

	uint16 spell_x_offset = 168 + x_off;
	if (Game::get_game()->is_new_style()) {
		spell_x_offset = Game::get_game()->get_game_width() - SPELLVIEWGUMP_WIDTH + x_off;
	}

	if (spell_view) {
		spell_view->init(gui->get_screen(), this, spell_x_offset, 6 + y_off, font, party, tile_manager, obj_manager);
	}
//set_current_view((View *)party_view);

	return true;
}

void ViewManager::reload() {
	if (!Game::get_game()->is_new_style())
		actor_view->set_party_member(0);
	inventory_view->lock_to_actor(false);
	inventory_view->set_party_member(0);

	set_party_mode();
	update();
}

bool ViewManager::set_current_view(View *view) {
	uint8 cur_party_member;

//actor_view->set_party_member(cur_party_member);
	if (view == NULL) // || game_type != NUVIE_GAME_U6) //HACK! remove this when views support MD and SE
		return false;

	if (current_view == view) // nothing to do if view is already the current_view.
		return false;

	if (current_view != NULL) {
		gui->removeWidget((GUI_Widget *)current_view);//remove current widget from gui

		cur_party_member = current_view->get_party_member_num();
		view->set_party_member(cur_party_member);
	}

	current_view = view;
	view->Show();
	gui->AddWidget((GUI_Widget *)view);
	view->Redraw();
	gui->Display();

	if (actor_view) {
		if (view != actor_view) {
			actor_view->set_show_cursor(false);
			actor_view->release_focus();
		}
	}

	if (inventory_view) {
		if (view != inventory_view) {
			inventory_view->set_show_cursor(false);
			inventory_view->release_focus();
		}
	}

	return true;
}

void ViewManager::close_current_view() {
	if (current_view == NULL)
		return;

	gui->removeWidget((GUI_Widget *)current_view);//remove current widget from gui
	current_view = NULL;
}

void ViewManager::update() {
	if (current_view)
		current_view->Redraw();

	if (ribbon && ribbon->Status() == WIDGET_HIDDEN) {
		ribbon->Show();
	}

	if (mdSkyWidget) {
		mdSkyWidget->Redraw();
	}

	return;
}

// We only change to portrait mode if the actor has a portrait.
void ViewManager::set_portrait_mode(Actor *actor, const char *name) {
	if (portrait_view->set_portrait(actor, name) == true) {
		set_current_view((View *)portrait_view);
	}
}

void ViewManager::set_inventory_mode() {
	set_current_view((View *)inventory_view);
	Events *event = Game::get_game()->get_event();
	if (event->get_mode() == EQUIP_MODE || event->get_mode() == INPUT_MODE
	        || event->get_mode() == ATTACK_MODE)
		inventory_view->set_show_cursor(true);
}

void ViewManager::set_party_mode() {
	Events *event = Game::get_game()->get_event();
	if (event->get_mode() == EQUIP_MODE)
		event->cancelAction();
	else if (event->get_mode() == INPUT_MODE || event->get_mode() == ATTACK_MODE)
		event->moveCursorToMapWindow();

	if (!Game::get_game()->is_new_style())
		set_current_view((View *)party_view);
	return;
}

void ViewManager::set_actor_mode() {
	set_current_view((View *)actor_view);
	Events *event = Game::get_game()->get_event();
	if (event->get_mode() == EQUIP_MODE || event->get_mode() == INPUT_MODE
	        || event->get_mode() == ATTACK_MODE) {
		actor_view->set_show_cursor(true);
		actor_view->moveCursorToButton(2);
	}
}

void ViewManager::set_spell_mode(Actor *caster, Obj *spell_container, bool eventMode) {
	if (spell_view != NULL) {
		spell_view->set_spell_caster(caster, spell_container, eventMode);
		set_current_view((View *)spell_view);
	}
	return;
}

void ViewManager::close_spell_mode() {
	if (spell_view) {
		//FIXME this should set previous view. Don't default to inventory view.
		spell_view->release_focus();
		if (!Game::get_game()->is_new_style())
			set_inventory_mode();
		else
			close_current_view();
	}
}

void ViewManager::open_doll_view(Actor *actor) {
	Screen *screen = Game::get_game()->get_screen();

	if (Game::get_game()->is_new_style()) {
		if (actor == NULL) {
			actor = doll_view_get_next_party_member();
		}
		DollViewGump *doll = get_doll_view(actor);
		if (doll == NULL) {
			uint16 x_off = Game::get_game()->get_game_x_offset();
			uint16 y_off = Game::get_game()->get_game_y_offset();
			uint8 num_doll_gumps = doll_gumps.size();
			doll = new DollViewGump(config);
			uint16 x = 12 * num_doll_gumps;
			uint16 y = 12 * num_doll_gumps;

			if (y + DOLLVIEWGUMP_HEIGHT > screen->get_height())
				y = screen->get_height() - DOLLVIEWGUMP_HEIGHT;

			doll->init(Game::get_game()->get_screen(), this, x + x_off, y + y_off, actor, font, party, tile_manager, obj_manager);

			add_view((View *)doll);
			add_gump(doll);
			doll_gumps.push_back(doll);
		} else {
			move_gump_to_top(doll);
		}
	}
}

Actor *ViewManager::doll_view_get_next_party_member() {
	if (doll_gumps.empty()) {
		doll_next_party_member = 0; //reset to first party member when there are no doll gumps on screen.
	}
	Actor *a = party->get_actor(doll_next_party_member);
	doll_next_party_member = (doll_next_party_member + 1) % party->get_party_size();

	return a;
}

DollViewGump *ViewManager::get_doll_view(Actor *actor) {
	Std::list<DraggableView *>::iterator iter;
	for (iter = doll_gumps.begin(); iter != doll_gumps.end(); iter++) {
		DollViewGump *view = (DollViewGump *)*iter;
		if (view->get_actor() == actor) {
			return view;
		}
	}

	return NULL;
}

ContainerViewGump *ViewManager::get_container_view(Actor *actor, Obj *obj) {
	Std::list<DraggableView *>::iterator iter;
	for (iter = container_gumps.begin(); iter != container_gumps.end(); iter++) {
		ContainerViewGump *view = (ContainerViewGump *)*iter;
		if (actor) {
			if (view->is_actor_container() && view->get_actor() == actor) {
				return view;
			}
		} else if (obj) {
			if (!view->is_actor_container() && view->get_container_obj() == obj) {
				return view;
			}
		}
	}

	return NULL;
}

void ViewManager::open_container_view(Actor *actor, Obj *obj) {
	ContainerViewGump *view = get_container_view(actor, obj);

	if (view == NULL) {
		uint16 x_off = Game::get_game()->get_game_x_offset();
		uint16 y_off = Game::get_game()->get_game_y_offset();
		uint16 container_x, container_y;
		if (!Game::get_game()->is_new_style()) {
			container_x = x_off;
			container_y = y_off;
		} else {
			container_x = Game::get_game()->get_game_width() - 120 + x_off;
			container_y = 20 + y_off;
		}

		view = new ContainerViewGump(config);
		view->init(Game::get_game()->get_screen(), this, container_x, container_y, font, party, tile_manager, obj_manager, obj);
		if (actor)
			view->set_actor(actor);
		else
			view->set_container_obj(obj);

		container_gumps.push_back(view);
		add_gump(view);
		add_view((View *)view);
	} else {
		move_gump_to_top(view);
	}
}

void ViewManager::close_container_view(Actor *actor) {
	ContainerViewGump *view = get_container_view(actor, NULL);

	if (view) {
		close_gump(view);
	}
}

void ViewManager::open_mapeditor_view() {
	if (Game::get_game()->is_new_style() && Game::get_game()->is_roof_mode()) {
		uint16 x_off = Game::get_game()->get_game_x_offset();
		uint16 y_off = Game::get_game()->get_game_y_offset();
		x_off += Game::get_game()->get_game_width() - 90;
		MapEditorView *view = new MapEditorView(config);
		view->init(Game::get_game()->get_screen(), this, x_off , y_off, font, party, tile_manager, obj_manager);
		add_view((View *)view);
		view->grab_focus();
	}
}

void ViewManager::open_portrait_gump(Actor *a) {
	if (Game::get_game()->is_new_style()) {
		uint16 x_off = Game::get_game()->get_game_x_offset();
		uint16 y_off = Game::get_game()->get_game_y_offset();
		PortraitViewGump *view = new PortraitViewGump(config);
		view->init(Game::get_game()->get_screen(), this, 62 + x_off, y_off, font, party, tile_manager, obj_manager, portrait, a);
		add_view((View *)view);
		add_gump(view);
		view->grab_focus();
	}
}

void ViewManager::open_sign_gump(const char *sign_text, uint16 length) {
	if (Game::get_game()->is_using_text_gumps()) { // check should be useless
		SignViewGump *view = new SignViewGump(config);
		view->init(Game::get_game()->get_screen(), this, font, party, tile_manager, obj_manager, sign_text, length);
		add_view((View *)view);
		add_gump(view);
		view->grab_focus();
	}
}

void ViewManager::open_scroll_gump(const char *text, uint16 length) {
	if (Game::get_game()->is_using_text_gumps()) { // check should be useless
		ScrollViewGump *view = new ScrollViewGump(config);
		view->init(Game::get_game()->get_screen(), this, font, party, tile_manager, obj_manager, string(text, length));
		add_view((View *)view);
		add_gump(view);
		view->grab_focus();
	}
}

void ViewManager::add_view(View *view) {
	view->Show();
	gui->AddWidget((GUI_Widget *)view);
	if (Game::get_game()->is_new_style()) {
		Game::get_game()->get_scroll()->moveToFront();
	}
	view->Redraw();
	gui->Display();
}

void ViewManager::add_gump(DraggableView *gump) {
	gumps.push_back(gump);
	Game::get_game()->get_map_window()->set_walking(false);
	if (ribbon) {
		ribbon->extend();
	}
}

void ViewManager::close_gump(DraggableView *gump) {
	gumps.remove(gump);
	container_gumps.remove(gump);
	doll_gumps.remove(gump);

	gump->close_view();
	gump->Delete();
	//gui->removeWidget((GUI_Widget *)gump);

	if (gumps.empty() && ribbon != NULL) {
		ribbon->retract();
	}
}

void ViewManager::close_all_gumps() {
	Std::list<DraggableView *>::iterator iter;
	for (iter = gumps.begin(); iter != gumps.end();) {
		DraggableView *gump = *iter;
		iter++;

		close_gump(gump);
	}
	//TODO make sure all gump objects have been deleted by GUI.
}

void ViewManager::move_gump_to_top(DraggableView *gump) {
	gump->moveToFront();
	Game::get_game()->get_scroll()->moveToFront();
}

// callbacks for switching views

GUI_status partyViewButtonCallback(void *data) {
	ViewManager *view_manager;

	view_manager = (ViewManager *)data;

	view_manager->set_party_mode();

	return GUI_YUM;
}

GUI_status actorViewButtonCallback(void *data) {
	ViewManager *view_manager;

	view_manager = (ViewManager *)data;

	view_manager->set_actor_mode();

	return GUI_YUM;
}

GUI_status inventoryViewButtonCallback(void *data) {
	ViewManager *view_manager;

	view_manager = (ViewManager *)data;

	view_manager->set_inventory_mode();

	return GUI_YUM;
}

void ViewManager::double_click_obj(Obj *obj) {
	Events *event = Game::get_game()->get_event();
	if (Game::get_game()->get_usecode()->is_readable(obj)) { // look at a scroll or book
		event->set_mode(LOOK_MODE);
		event->look(obj);
		event->endAction(false); // FIXME: should be in look()
	} else if (event->newAction(USE_MODE))
		event->select_obj(obj);
}

unsigned int ViewManager::get_display_weight(float weight) {
	if (weight > 1)
		return static_cast<unsigned int>(roundf(weight));
	else if (weight > 0)
		return 1;
	else // weight == 0 (or somehow negative)
		return 0;
}

// beginning of custom doll functions shared between DollWidget and DollViewGump
Std::string ViewManager::getDollDataDirString() {
	if (!DollDataDirString.empty())
		return DollDataDirString;
	DollDataDirString = GUI::get_gui()->get_data_dir();
	Std::string path;
	build_path(DollDataDirString, "images", path);
	DollDataDirString = path;
	build_path(DollDataDirString, "gumps", path);
	DollDataDirString = path;
	build_path(DollDataDirString, "doll", path);
	DollDataDirString = path;

	return DollDataDirString;
}

Graphics::ManagedSurface *ViewManager::loadAvatarDollImage(Graphics::ManagedSurface *avatar_doll, bool orig) {
	char filename[17]; //avatar_nn_nn.bmp\0
	Std::string imagefile;
	uint8 portrait_num = Game::get_game()->get_portrait()->get_avatar_portrait_num();

	sprintf(filename, "avatar_%s_%02d.bmp", get_game_tag(Game::get_game()->get_game_type()), portrait_num);
	if (orig) {
		build_path(getDollDataDirString(), "orig_style", imagefile);
		build_path(imagefile, filename, imagefile);
	} else {
		build_path(getDollDataDirString(), filename, imagefile);
	}
	if (avatar_doll != NULL)
		SDL_FreeSurface(avatar_doll);
	NuvieBmpFile bmp;
	avatar_doll = bmp.getSdlSurface32(imagefile);
	if (avatar_doll == NULL)
		avatar_doll = loadGenericDollImage(orig);
	return avatar_doll;
}

Graphics::ManagedSurface *ViewManager::loadCustomActorDollImage(Graphics::ManagedSurface *actor_doll, uint8 actor_num, bool orig) {
	char filename[17]; //actor_nn_nnn.bmp\0
	Std::string imagefile;

	if (actor_doll != NULL)
		SDL_FreeSurface(actor_doll);

	sprintf(filename, "actor_%s_%03d.bmp", get_game_tag(Game::get_game()->get_game_type()), actor_num);
	if (orig) {
		build_path(getDollDataDirString(), "orig_style", imagefile);
		build_path(imagefile, filename, imagefile);
	} else {
		build_path(getDollDataDirString(), filename, imagefile);
	}
	NuvieBmpFile bmp;
	actor_doll = bmp.getSdlSurface32(imagefile);

	if (actor_doll == NULL)
		actor_doll = loadGenericDollImage(orig);
	return actor_doll;
}

Graphics::ManagedSurface *ViewManager::loadGenericDollImage(bool orig) {
	char filename[14]; //avatar_nn.bmp\0
	Std::string imagefile;

	sprintf(filename, "actor_%s.bmp", get_game_tag(Game::get_game()->get_game_type()));
	if (orig) {
		build_path(getDollDataDirString(), "orig_style", imagefile);
		build_path(imagefile, filename, imagefile);
	} else {
		build_path(getDollDataDirString(), filename, imagefile);
	}
	NuvieBmpFile bmp;
	return bmp.getSdlSurface32(imagefile);
}

} // End of namespace Nuvie
} // End of namespace Ultima
