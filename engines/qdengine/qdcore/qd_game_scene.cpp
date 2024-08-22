/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/stream.h"

#include "qdengine/qdengine.h"
#include "qdengine/qd_fwd.h"
#include "qdengine/system/graphics/gr_dispatcher.h"
#include "qdengine/parser/xml_tag_buffer.h"
#include "qdengine/system/input/mouse_input.h"
#include "qdengine/qdcore/util/plaympp_api.h"
#include "qdengine/parser/qdscr_parser.h"
#include "qdengine/qdcore/qd_minigame.h"
#include "qdengine/qdcore/qd_grid_zone.h"
#include "qdengine/qdcore/qd_music_track.h"
#include "qdengine/qdcore/qd_game_object_static.h"
#include "qdengine/qdcore/qd_game_object_animated.h"
#include "qdengine/qdcore/qd_game_object_moving.h"
#include "qdengine/qdcore/qd_game_object_mouse.h"
#include "qdengine/qdcore/qd_game_scene.h"
#include "qdengine/qdcore/qd_named_object_reference.h"
#include "qdengine/qdcore/qd_setup.h"
#include "qdengine/qdcore/qd_interface_button.h"
#include "qdengine/qdcore/qd_interface_screen.h"
#include "qdengine/qdcore/qd_game_dispatcher.h"

namespace QDEngine {

struct qdGridZoneOrdering {
	bool operator()(const qdGridZone *z0, const qdGridZone *z1) {
		return z0->update_timer() < z1->update_timer();
	}
};

fpsCounter *g_fpsCounter = nullptr;
grScreenRegion qdGameScene::_fps_region = grScreenRegion_EMPTY;
grScreenRegion qdGameScene::_fps_region_last = grScreenRegion_EMPTY;
char qdGameScene::_fps_string[255];
Std::vector<qdGameObject *> qdGameScene::_visible_objects;

qdGameScene::qdGameScene() : _mouse_click_object(NULL),
	_mouse_right_click_object(NULL),
	_mouse_hover_object(NULL),
	_selected_object(NULL),
	_mouse_click_pos(0, 0),
	_zone_update_count(0),
	_minigame(NULL) {
	set_loading_progress_callback(NULL);

	_restart_minigame_on_load = false;

	_autosave_slot = -1;
}

qdGameScene::~qdGameScene() {
	_grid_zones.clear();
}

fpsCounter *qdGameScene::fps_counter() {
	if (!g_fpsCounter)
		g_fpsCounter = new fpsCounter(1000);

	return g_fpsCounter;
}

void qdGameScene::init_objects_grid() {
	_camera.drop_grid_attributes(sGridCell::CELL_OCCUPIED | sGridCell::CELL_PERSONAGE_OCCUPIED | sGridCell::CELL_SELECTED);

	for (qdGameObjectList::const_iterator io = object_list().begin(); io != object_list().end(); ++io)
		(*io)->save_grid_zone();

	for (qdGameObjectList::const_iterator io = object_list().begin(); io != object_list().end(); ++io) {
		if ((*io)->is_visible() && !(*io)->check_flag(QD_OBJ_SCREEN_COORDS_FLAG))
			(*io)->toggle_grid_zone();
	}
}

void qdGameScene::quant(float dt) {
	debugC(9, kDebugQuant, "qdGameScene::quant(%f)", dt);

	if (_minigame) {
		debugC(3, kDebugQuant, "qdGameScene::quant(%f) minigame", dt);
		_minigame->quant(dt);
	}

	for (qdGameObjectList::const_iterator io = object_list().begin(); io != object_list().end(); ++io)
		(*io)->update_screen_pos();

	conditions_quant(dt);

	personages_quant();

	follow_quant(dt);
	collision_quant();

	if (_camera.quant(dt)) {
		if (qdGameDispatcher *dp = qdGameDispatcher::get_dispatcher()) {
			debugC(3, kDebugQuant, "qdGameScene::quant(%f) _camera", dt);
			dp->toggle_full_redraw();
		}
	}

	qdGameDispatcherBase::quant(dt);

	if (mouseDispatcher::instance()->check_event(mouseDispatcher::EV_LEFT_DOWN) && _selected_object && _selected_object->has_control_type(qdGameObjectMoving::CONTROL_MOUSE)) {
		debugC(5, kDebugMovement, "qdGameScene::quant(%f) mouse movement", dt);
		qdGameDispatcher *dp = qdGameDispatcher::get_dispatcher();
		if (dp && !dp->check_flag(qdGameDispatcher::OBJECT_CLICK_FLAG | qdGameDispatcher::DIALOG_CLICK_FLAG) && _selected_object->can_move()) {
			debugC(5, kDebugMovement, "qdGameScene::quant(%f) can move: %d", dt, _selected_object->can_move());
			Vect3f pos = _camera.get_cell_coords(_camera.get_cell_index(_mouse_click_pos.x, _mouse_click_pos.y, false));

			_selected_object->set_queued_state(NULL);
			_selected_object->move(pos, false);

			// For all the followers, we need to calculate the follow path
			follow_pers_init(qdGameObjectMoving::FOLLOW_UPDATE_PATH);

			if (false == _selected_object->is_moving()) {
				// If the active one was not able to walk, but potentialy could, it goes inot a waiting mode
				if (_selected_object->can_move())
					_selected_object->set_follow_condition(qdGameObjectMoving::FOLLOW_WAIT);
				else
					_selected_object->set_follow_condition(qdGameObjectMoving::FOLLOW_DONE);
			} else
				_selected_object->set_follow_condition(qdGameObjectMoving::FOLLOW_MOVING);

			for (personages_container_t::iterator it = _personages.begin(); it != _personages.end(); ++it)
				if (
				    ((*it) != _selected_object) &&
				    (*it)->has_control_type(qdGameObjectMoving::CONTROL_ACTIVE_CLICK_REACTING)
				)
					(*it)->move(pos, false);
		}
	}

	for (qdGameObjectList::const_iterator io = object_list().begin(); io != object_list().end(); ++io) {
		if (!(*io)->check_flag(QD_OBJ_IS_IN_INVENTORY_FLAG))
			(*io)->quant(dt);
	}

	update_mouse_cursor();

	if (_selected_object && !_selected_object->is_visible()) {
		for (personages_container_t::iterator it = _personages.begin(); it != _personages.end(); ++it) {
			if ((*it)->is_visible() && !(*it)->check_flag(QD_OBJ_NON_PLAYER_PERSONAGE_FLAG)) {
				set_active_personage(*it);
				break;
			}
		}
	}

	if (_selected_object) _selected_object->keyboard_move();

	_mouse_click_object = _mouse_right_click_object = NULL;
	_mouse_hover_object = NULL;
}

void qdGameScene::redraw() {
	if (!object_list().empty()) {
		if (check_flag(CYCLE_X) || check_flag(CYCLE_Y)) {
			const int sx = _camera.get_scr_sx();
			const int sy = _camera.get_scr_sy();

			int offs_x[8] = { -sx, -sx, 0, sx, sx, sx, 0, -sx };
			int offs_y[8] = { 0, -sy, -sy, -sy, 0, sy, sy, sy };

			switch (flags() & (CYCLE_X | CYCLE_Y)) {
			case CYCLE_X:
				for (Std::vector<qdGameObject *>::reverse_iterator it = _visible_objects.rbegin(); it != _visible_objects.rend(); ++it) {
					for (int i = 0; i < 8; i += 4) {
						Vect2i pos = (*it)->screen_pos() + Vect2i(offs_x[i], offs_y[i]);
						Vect2i sz = (*it)->screen_size();

						pos -= sz / 2;

						if (grDispatcher::instance()->is_rectangle_visible(pos.x, pos.y, sz.x, sz.y))
							(*it)->redraw(offs_x[i], offs_y[i]);
					}
					(*it)->redraw();
				}
				break;
			case CYCLE_Y:
				for (Std::vector<qdGameObject *>::reverse_iterator it = _visible_objects.rbegin(); it != _visible_objects.rend(); ++it) {
					for (int i = 2; i < 8; i += 4) {
						Vect2i pos = (*it)->screen_pos() + Vect2i(offs_x[i], offs_y[i]);
						Vect2i sz = (*it)->screen_size();

						pos -= sz / 2;

						if (grDispatcher::instance()->is_rectangle_visible(pos.x, pos.y, sz.x, sz.y))
							(*it)->redraw(offs_x[i], offs_y[i]);
					}
					(*it)->redraw();
				}
				break;
			case CYCLE_X | CYCLE_Y:
				for (Std::vector<qdGameObject *>::reverse_iterator it = _visible_objects.rbegin(); it != _visible_objects.rend(); ++it) {
					for (int i = 0; i < 8; i++) {
						Vect2i pos = (*it)->screen_pos() + Vect2i(offs_x[i], offs_y[i]);
						Vect2i sz = (*it)->screen_size();

						pos -= sz / 2;

						if (grDispatcher::instance()->is_rectangle_visible(pos.x, pos.y, sz.x, sz.y))
							(*it)->redraw(offs_x[i], offs_y[i]);
					}
					(*it)->redraw();
				}
				break;
			}
		} else {
			for (Std::vector<qdGameObject *>::reverse_iterator it = _visible_objects.rbegin(); it != _visible_objects.rend(); ++it)
				(*it)->redraw();
		}
	}
}

bool qdGameScene::mouse_handler(int x, int y, mouseDispatcher::mouseEvent ev) {
	bool result = false;

	_camera.cycle_coords(x, y);
	_mouse_click_pos = _camera.scr2plane(Vect2s(x, y));

	switch (ev) {
	case mouseDispatcher::EV_MOUSE_MOVE:
		if (qdInterfaceDispatcher *dp = qdInterfaceDispatcher::get_dispatcher()) {
			if (dp->is_mouse_hover()) {
				result = false;
				break;
			}
		}
		for (Std::vector<qdGameObject *>::iterator io = _visible_objects.begin(); io != _visible_objects.end(); ++io) {
			if (!(*io)->check_flag(QD_OBJ_DISABLE_MOUSE_FLAG) && (*io)->named_object_type() != QD_NAMED_OBJECT_STATIC_OBJ) {
				if ((*io)->hit(x, y)) {
					_mouse_hover_object = *io;
					break;
				}
			}
		}
		break;
	case mouseDispatcher::EV_LEFT_DOWN:
	case mouseDispatcher::EV_RIGHT_DOWN: {
		if (qdInterfaceDispatcher *dp = qdInterfaceDispatcher::get_dispatcher()) {
			if (dp->is_mouse_hover()) {
				result = false;
				break;
			}
		}
		qdGameObject *pObj = get_hitted_obj(x, y);
		if (NULL != pObj) {
			result = pObj->mouse_handler(x, y, ev);
			if (ev == mouseDispatcher::EV_LEFT_DOWN)
				_mouse_click_object	= pObj;
			else
				_mouse_right_click_object = pObj;
		}
		break;
		}
	default:
		break;
	}

	return result;
}

qdGameObject *qdGameScene::get_hitted_obj(int x, int y) {
	for (Std::vector<qdGameObject *>::iterator io = _visible_objects.begin(); io != _visible_objects.end(); ++io) {
		if (!(*io)->check_flag(QD_OBJ_DISABLE_MOUSE_FLAG) && (*io)->named_object_type() != QD_NAMED_OBJECT_STATIC_OBJ)
			if ((*io)->hit(x, y))
				return (*io);
	}
	return NULL;
}

void qdGameScene::load_script(const xml::tag *p) {
	load_conditions_script(p);
	qdGameDispatcherBase::load_script_body(p);

	qdGameObject *obj;
	qdGridZone *grz;
	qdMusicTrack *trk;

	_personages.clear();
	int personages_count = 0;

	for (xml::tag::subtag_iterator it = p->subtags_begin(); it != p->subtags_end(); ++it) {
		switch (it->ID()) {
		case QDSCR_NAME:
			set_name(it->data());
			break;
		case QDSCR_MINIGAME_GAME_NAME:
			set_minigame_name(it->data());
			break;
		case QDSCR_SCENE_RESTART_MINIGAME:
			toggle_restart_minigame_on_load(xml::tag_buffer(*it).get_int() != 0);
			break;
		case QDSCR_FLAG:
			set_flag(xml::tag_buffer(*it).get_int());
			break;
		case QDSCR_CD:
			set_CD_info(xml::tag_buffer(*it).get_uint());
			break;
		case QDSCR_STATIC_OBJECT:
			obj = new qdGameObjectStatic;
			obj->load_script(&*it);
			add_object(obj);
			break;
		case QDSCR_ANIMATED_OBJECT:
			obj = new qdGameObjectAnimated;
			obj->load_script(&*it);
			add_object(obj);
			break;
		case QDSCR_MOVING_OBJECT:
			obj = new qdGameObjectMoving;
			obj->load_script(&*it);
			add_object(obj);
			personages_count++;
			break;
		case QDSCR_CAMERA:
			_camera.load_script(&*it);
			break;
		case QDSCR_GRID_ZONE:
			grz = new qdGridZone;
			grz->load_script(&*it);
			add_grid_zone(grz);
			break;
		case QDSCR_MUSIC_TRACK:
			trk = new qdMusicTrack;
			trk->load_script(&*it);
			add_music_track(trk);
			break;
		case QDSCR_SCENE_SAVE_SLOT:
			xml::tag_buffer(*it) > _autosave_slot;
			break;
		case QDSCR_INTERFACE_SCREEN:
			if (const xml::tag *name_tag = it->search_subtag(QDSCR_NAME))
				set_interface_screen_name(name_tag->data());
			break;
		}
	}

	_visible_objects.reserve(object_list().size());
	_personages.reserve(personages_count);

	for (qdGameObjectList::const_iterator it = object_list().begin(); it != object_list().end(); ++it) {
		if ((*it)->named_object_type() == QD_NAMED_OBJECT_MOVING_OBJ)
			_personages.push_back(static_cast<qdGameObjectMoving *>(*it));
	}

	_camera.set_cycle(check_flag(CYCLE_X), check_flag(CYCLE_Y));
}

bool qdGameScene::save_script(Common::WriteStream &fh, int indent) const {
	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}
	fh.writeString(Common::String::format("<scene name=\"%s\"", qdscr_XML_string(name())));

	if (CD_info()) {
		fh.writeString(Common::String::format(" cd=\"%d\"", CD_info()));
	}

	if (flags()) {
		fh.writeString(Common::String::format(" flags=\"%d\"", flags()));
	}

	if (_autosave_slot != -1) {
		fh.writeString(Common::String::format(" save_slot=\"%d\"", _autosave_slot));
	}

	if (_restart_minigame_on_load) {
		fh.writeString(" restart_minigame=\"1\"");
	}

	if (has_minigame()) {
		fh.writeString(Common::String::format(" game_name=\"%s\"", qdscr_XML_string(minigame_name())));
	}

	fh.writeString(">\r\n");

	if (has_interface_screen()) {
		for (int i = 0; i < indent; i++) {
			fh.writeString("\t");
		}

		fh.writeString(Common::String::format("<interface_screen_name=\"%s\"/>\r\n", qdscr_XML_string(interface_screen_name())));
	}

	qdGameDispatcherBase::save_script_body(fh, indent);

	_camera.save_script(fh, indent + 1);

	for (auto &it : object_list()) {
		it->save_script(fh, indent + 1);
	}

	for (auto &it : grid_zone_list()) {
		it->save_script(fh, indent + 1);
	}

	for (auto &it : music_track_list()) {
		it->save_script(fh, indent + 1);
	}

	save_conditions_script(fh, indent);

	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}
	fh.writeString("</scene>\r\n");

	return true;
}

int qdGameScene::load_resources() {
	debug("[%d] Loading scene \"%s\"", g_system->getMillis(), transCyrillic(name()));

	int total_size = get_resources_size();
	set_resources_size(total_size);

	qdGameDispatcher *dsp = qd_get_game_dispatcher();
	if (dsp)
		set_loading_progress_callback(dsp->get_scene_loading_progress_callback(), dsp->get_scene_loading_progress_data());

	int size = qdGameDispatcherBase::load_resources();

	for (auto &io : object_list()) {
		io->load_resources();
		show_loading_progress(1);
		size ++;
	}

	set_resources_size(0);

	fps_counter()->reset();

	debugC(3, kDebugLoad, "qdGameScene::load_resources(): Loaded %d resources", size);

	return size;
}

void qdGameScene::free_resources() {
	if (qdGameDispatcher *dp = qd_get_game_dispatcher()) {
		if (dp->current_music() && !dp->current_music()->check_flag(QD_MUSIC_TRACK_DISABLE_SWITCH_OFF))
			dp->stop_music();
	}

	for (auto &io : object_list()) {
		io->free_resources();
	}

	qdGameDispatcherBase::free_resources();
}

void qdGameScene::debug_redraw() {
	if (ConfMan.getBool("show_fps"))
		grDispatcher::instance()->drawText(10, 10, grDispatcher::instance()->make_rgb888(255, 255, 255), _fps_string);

	if (g_engine->_debugDraw) {
		if (_selected_object) {
			static char buffer[256];
			snprintf(buffer, 256, "%.1f %.1f %.1f, %.1f", _selected_object->R().x, _selected_object->R().y, _selected_object->R().z, R2G(_selected_object->direction_angle()));
			grDispatcher::instance()->drawText(10, 30, grDispatcher::instance()->make_rgb888(255, 255, 255), buffer);
			float z = _camera.global2camera_coord(_selected_object->R()).z;
			snprintf(buffer, 256, "D: %.2f", z);
			grDispatcher::instance()->drawText(10, 50, grDispatcher::instance()->make_rgb888(255, 255, 255), buffer);

			if (_selected_object->get_cur_state() && _selected_object->get_cur_state()->name())
				grDispatcher::instance()->drawText(10, 70, grDispatcher::instance()->make_rgb888(255, 255, 255), _selected_object->get_cur_state()->name());

			snprintf(buffer, 256, "%d %d", _camera.get_scr_center_x(), _camera.get_scr_center_y());
			grDispatcher::instance()->drawText(10, 90, grDispatcher::instance()->make_rgb888(255, 255, 255), buffer);
			/*
			            sprintf(buffer,"%d %d",mouseDispatcher::instance()->mouse_x(),mouseDispatcher::instance()->mouse_y());
			            grDispatcher::instance()->drawText(10,130,grDispatcher::instance()->make_rgb888(255,255,255),buffer);
			*/
		}

		if (qdGameDispatcher *dp = qdGameDispatcher::get_dispatcher()) {
			if (dp->current_music()) {
				grDispatcher::instance()->drawText(10, 130, grDispatcher::instance()->make_rgb888(255, 255, 255), dp->current_music()->file_name());
			}
		}

		// Draw criss-cross
		const uint32 cl = 0x202020;

		Vect2s v0,v1;
		v0 = qdCamera::current_camera()->global2scr(Vect3f(-300,-300,0));
		v1 = qdCamera::current_camera()->global2scr(Vect3f(300,300,0));
		grDispatcher::instance()->line(v0.x,v0.y,v1.x,v1.y,cl,2);

		v0 = qdCamera::current_camera()->global2scr(Vect3f(-300,300,0));
		v1 = qdCamera::current_camera()->global2scr(Vect3f(300,-300,0));
		grDispatcher::instance()->line(v0.x,v0.y,v1.x,v1.y,cl,2);

		for (Std::vector<qdGameObject *>::reverse_iterator it = _visible_objects.rbegin(); it != _visible_objects.rend(); ++it)
			(*it)->debug_redraw();
	}

	if (g_engine->_debugDrawGrid)
		_camera.draw_grid();

}

int qdGameScene::get_resources_size() {
	return object_list().size() + qdGameDispatcherBase::get_resources_size();
}

bool qdGameScene::activate() {
	debugC(3, kDebugLog, "Activation of the scene, %s", transCyrillic(name()));
	_camera.quant(0.0f);

	// During scene activation all the followers are moved to the normal state
	follow_pers_init(qdGameObjectMoving::FOLLOW_DONE);

	for (qdGameObjectList::const_iterator it = object_list().begin(); it != object_list().end(); ++it) {
		(*it)->init_grid_zone();
		if (qdGameObjectAnimated * p = dynamic_cast<qdGameObjectAnimated *>(*it))
			p->set_states_owner();
	}

	qdGameDispatcher *dp = qdGameDispatcher::get_dispatcher();

	for (personages_container_t::iterator it = _personages.begin(); it != _personages.end(); ++it) {
		if (qdGameObjectState *p = dp->get_walk_state((*it)->name()))
			(*it)->set_last_walk_state(p);
	}

	if (!get_active_personage()) {
		for (qdGameObjectList::const_iterator io = object_list().begin(); io != object_list().end(); ++io) {
			if ((*io)->named_object_type() == QD_NAMED_OBJECT_MOVING_OBJ && !(*io)->check_flag(QD_OBJ_NON_PLAYER_PERSONAGE_FLAG)) {
				set_active_personage(static_cast<qdGameObjectMoving *>(*io));
				break;
			}
		}
	}

	for (qdGridZoneList::const_iterator iz = grid_zone_list().begin(); iz != grid_zone_list().end(); ++iz)
		(*iz)->set_state((*iz)->state());

	init_visible_objects_list();

	return true;
}

bool qdGameScene::deactivate() {
	if (_minigame)
		_minigame->end();

	return true;
}

bool qdGameScene::merge_global_objects(qdGameObject *obj) {
	qdGameDispatcher *gd = qd_get_game_dispatcher();
	if (!gd) return false;

	if (!obj) {
		for (auto &it : object_list()) {
			if (it->named_object_type() == QD_NAMED_OBJECT_MOVING_OBJ) {
				qdGameObject *p = gd->get_global_object(it->name());
				if (p && p->named_object_type() == QD_NAMED_OBJECT_MOVING_OBJ) {
					static_cast<qdGameObjectMoving *>(it)->merge(static_cast<qdGameObjectMoving *>(p));
				}
			}
		}
	} else {
		for (auto &it : object_list()) {
			if (it->named_object_type() == QD_NAMED_OBJECT_MOVING_OBJ) {
				if (it->name() && obj->name() && !strcmp(it->name(), obj->name())) {
					static_cast<qdGameObjectMoving *>(it)->merge(static_cast<qdGameObjectMoving *>(obj));
				}
			}
		}
	}

	return true;
}

bool qdGameScene::split_global_objects(qdGameObject *obj) {
	qdGameDispatcher *gd = qd_get_game_dispatcher();
	if (!gd) return false;

	if (!obj) {
		for (auto &it : object_list()) {
			if (it->named_object_type() == QD_NAMED_OBJECT_MOVING_OBJ) {
				qdGameObject *p = gd->get_global_object(it->name());
				if (p && p->named_object_type() == QD_NAMED_OBJECT_MOVING_OBJ) {
					static_cast<qdGameObjectMoving *>(it)->split(static_cast<qdGameObjectMoving *>(p));
				}
			}
		}
	} else {
		for (auto &it : object_list()) {
			if (it->named_object_type() == QD_NAMED_OBJECT_MOVING_OBJ) {
				if (it->name() && obj->name() && !strcmp(it->name(), obj->name())) {
					static_cast<qdGameObjectMoving *>(it)->split(static_cast<qdGameObjectMoving *>(obj));
				}
			}
		}
	}

	return true;
}

struct qdObjectOrdering {
	bool operator()(const qdGameObject *p0, const qdGameObject *p1) {
		if (p0->screen_depth() == p1->screen_depth())
			return p0->tempPosInList() < p1->tempPosInList();
		return p0->screen_depth() < p1->screen_depth();
	}
};

bool qdGameScene::init_visible_objects_list() {
	_visible_objects.clear();

	for (auto &it : object_list()) {
		it->update_screen_pos();
		if (it->is_visible() && !it->check_flag(QD_OBJ_SCREEN_COORDS_FLAG)) {
			// The original depended on stable sort, so we imitate it here
			// since Common::sort() is unstable
			it->setTempPosInList(_visible_objects.size());
			_visible_objects.push_back(it);
		}
	}
	Common::sort(_visible_objects.begin(), _visible_objects.end(), qdObjectOrdering());

	return true;
}

bool qdGameScene::add_object(qdGameObject *p) {
	if (_objects.add_object(p)) {
		p->set_owner(this);
		return true;
	}
	return false;
}

bool qdGameScene::rename_object(qdGameObject *p, const char *name) {
	return _objects.rename_object(p, name);
}

bool qdGameScene::remove_object(const char *name) {

	if (_objects.remove_object(name)) {
		return true;
	}
	return false;
}

bool qdGameScene::remove_object(qdGameObject *p) {
	if (_objects.remove_object(p)) {
		return true;
	}
	return false;
}

qdGameObject *qdGameScene::get_object(const char *name) {
	return _objects.get_object(name);
}

bool qdGameScene::is_object_in_list(const char *name) {
	return _objects.is_in_list(name);
}

bool qdGameScene::is_object_in_list(qdGameObject *p) {
	return _objects.is_in_list(p);
}

bool qdGameScene::add_grid_zone(qdGridZone *p) {
	if (_grid_zones.add_object(p)) {
		p->set_owner(this);
		return true;
	}

	return false;
}

bool qdGameScene::rename_grid_zone(qdGridZone *p, const char *name) {
	return _grid_zones.rename_object(p, name);
}

bool qdGameScene::remove_grid_zone(const char *name) {
	return _grid_zones.remove_object(name);
}

bool qdGameScene::remove_grid_zone(qdGridZone *p) {
	return _grid_zones.remove_object(p);
}

qdGridZone *qdGameScene::get_grid_zone(const char *name) {
	return _grid_zones.get_object(name);
}

bool qdGameScene::is_grid_zone_in_list(const char *name) {
	return _grid_zones.is_in_list(name);
}

bool qdGameScene::is_grid_zone_in_list(qdGridZone *p) {
	return _grid_zones.is_in_list(p);
}

bool qdGameScene::add_music_track(qdMusicTrack *p) {
	if (_music_tracks.add_object(p)) {
		p->set_owner(this);
		return true;
	}

	return false;
}

bool qdGameScene::rename_music_track(qdMusicTrack *p, const char *name) {
	return _music_tracks.rename_object(p, name);
}

bool qdGameScene::remove_music_track(const char *name) {
	return _music_tracks.remove_object(name);
}

bool qdGameScene::remove_music_track(qdMusicTrack *p) {
	return _music_tracks.remove_object(p);
}

qdMusicTrack *qdGameScene::get_music_track(const char *name) {
	return _music_tracks.get_object(name);
}

bool qdGameScene::is_music_track_in_list(const char *name) const {
	return _music_tracks.is_in_list(name);
}

bool qdGameScene::is_music_track_in_list(qdMusicTrack *p) const {
	return _music_tracks.is_in_list(p);
}

bool qdGameScene::load_data(Common::SeekableReadStream &fh, int save_version) {
	debugC(3, kDebugSave, "  qdGameScene::load_data before: %ld", fh.pos());
	if (!qdConditionalObject::load_data(fh, save_version)) {
		return false;
	}

	if (!_camera.load_data(fh, save_version))
		return false;

	debugC(3, kDebugSave, "  qdGameScene::load_data(%u): Loading _objects %ld", object_list().size(), fh.pos());
	for (auto &it : object_list()) {
		if (!it->load_data(fh, save_version)) {
			return false;
		}
	}

	size_t sz = grid_zone_list().size();
	if (sz) {
		for (auto &it : _grid_zones.get_list()) {
			if (!it->load_data(fh, save_version))
				return false;
		}

		Std::vector<qdGridZone *> zone_order;
		zone_order.reserve(sz);

		for (auto &it1 : grid_zone_list())
			zone_order.push_back(it1);

		Common::sort(zone_order.begin(), zone_order.end(), qdGridZoneOrdering());

		_zone_update_count = 0;

		for (Std::vector<qdGridZone *>::iterator it = zone_order.begin(); it != zone_order.end(); ++it)
			(*it)->set_state((*it)->state());
	}

	int fl = fh.readUint32LE();
	if (fl) {
		qdNamedObjectReference ref;
		if (!ref.load_data(fh, save_version))
			return false;

		if (qdGameDispatcher *p = qd_get_game_dispatcher())
			_selected_object = static_cast<qdGameObjectMoving *>(p->get_named_object(&ref));

		if (!_selected_object) return false;

		_selected_object->toggle_selection(true);

		if (qdGameDispatcher *dp = qdGameDispatcher::get_dispatcher())
			dp->toggle_inventory(true);
	} else
		_selected_object = NULL;

	if (save_version >= 107) {
		const int save_buf_sz = 64 * 1024;
		char save_buf[save_buf_sz];

		fl = fh.readSint32LE();
		debugC(3, kDebugLog, "qdGameScene::load_data(%d): minigame", fl);
		if (fl) {
			debugC(3, kDebugLog, "qdGameScene::load_data(%d): minigame", fl);
			fh.read(save_buf, fl);
		}
		if (_minigame)
			_minigame->load_game(save_buf, fl, this);
	}

	debugC(3, kDebugSave, "  qdGameScene::load_data after: %ld", fh.pos());

	return true;
}

bool qdGameScene::save_data(Common::WriteStream &fh) const {
	debugC(3, kDebugSave, "  qdGameScene::save_data before: %ld", fh.pos());
	if (!qdConditionalObject::save_data(fh)) {
		return false;
	}

	if (!_camera.save_data(fh)) {
		return false;
	}

	debugC(3, kDebugSave, "  qdGameSceen::save_data(%u): Saving _objects %ld", object_list().size(), fh.pos());
	for (auto &it : object_list()) {
		if (!it->save_data(fh)) {
			return false;
		}
	}

	for (auto &it : _grid_zones.get_list()) {
		if (!it->save_data(fh)) {
			return false;
		}
	}

	if (_selected_object) {
		fh.writeUint32LE(1);
		qdNamedObjectReference ref(_selected_object);
		if (!ref.save_data(fh)) {
			return false;
		}
	} else {
		fh.writeUint32LE(0);
	}

	if (_minigame) {
		const int save_buf_sz = 64 * 1024;
		char save_buf[save_buf_sz];
		int size = _minigame->save_game(save_buf, save_buf_sz, const_cast<qdGameScene *>(this));
		fh.writeSint32LE(size);
		if (size) {
			fh.write(save_buf, size);
		}
	} else {
		fh.writeUint32LE(0);
	}

	debugC(3, kDebugSave, "  qdGameScene::save_data after: %ld", fh.pos());
	return true;
}

void qdGameScene::set_active_object(qdGameObjectAnimated *p) {
	if (p->named_object_type() == QD_NAMED_OBJECT_MOVING_OBJ)
		set_active_personage(static_cast<qdGameObjectMoving *>(p));
}

void qdGameScene::set_active_personage(qdGameObjectMoving *p) {
	if (p && !p->is_visible())
		return;

	if (_selected_object)
		_selected_object->toggle_selection(false);

	_selected_object = p;
	if (_selected_object)
		_selected_object->toggle_selection(true);

	_camera.set_default_object(p);

	if (p && p->has_camera_mode()) {
		_camera.set_mode(p->camera_mode(), p);
		_camera.set_default_mode(p->camera_mode());
	}

	if (qdGameDispatcher *dp = qdGameDispatcher::get_dispatcher()) {
		dp->toggle_inventory(true);
	}

	follow_pers_init(qdGameObjectMoving::FOLLOW_DONE); // When the active actor is changed, we stop the followers
	for (personages_container_t::iterator it = _personages.begin(); it != _personages.end(); ++it) {
		if ((*it) != p && !(*it)->check_flag(QD_OBJ_NON_PLAYER_PERSONAGE_FLAG)) {
			if ((*it)->check_flag(QD_OBJ_MOVING_FLAG)) {
				(*it)->set_queued_state(NULL);
				(*it)->stop_movement();
			}
		}
	}
}

bool qdGameScene::init() {
	if (!qdGameDispatcherBase::init()) return false;

	_zone_update_count = 0;
	_camera.init();

	_selected_object = NULL;

	for (auto it : object_list())
		it->init();

	for (auto it : grid_zone_list())
		it->init();

	for (auto it : music_track_list())
		it->init();

	if (has_minigame() && !_minigame) {
		if (qdGameDispatcher *dp = qdGameDispatcher::get_dispatcher())
			_minigame = dp->get_minigame(minigame_name());

		if (_minigame)
			create_minigame_objects();
	}

	return true;
}

bool qdGameScene::change_active_personage(void) {
	if (_selected_object) {
		personages_container_t::iterator it = Common::find(_personages.begin(), _personages.end(), _selected_object);
		if (it == _personages.end()) return false;

		do {
			if (++it == _personages.end())
				it = _personages.begin();

			if (!(*it)->check_flag(QD_OBJ_NON_PLAYER_PERSONAGE_FLAG)) {
				if (*it != _selected_object)
					set_active_personage(*it);
				return true;
			}

		} while (*it != _selected_object);
	} else {
		for (personages_container_t::iterator it = _personages.begin(); it != _personages.end(); ++it) {
			if (!(*it)->check_flag(QD_OBJ_NON_PLAYER_PERSONAGE_FLAG)) {
				set_active_personage(*it);
				return true;
			}
		}
	}

	return false;
}

void qdGameScene::pre_redraw() {
	qdGameDispatcher *dp = qdGameDispatcher::get_dispatcher();
	if (!dp) return;

	init_visible_objects_list();

	if (!dp->need_full_redraw()) {
		if (ConfMan.getBool("show_fps")) {
			const int sx = 80;
			const int sy = 20;
			dp->add_redraw_region(grScreenRegion(10 + sx / 2, 10 + sy / 2, sx, sy));
		}

		for (qdGameObjectList::const_iterator io = object_list().begin(); io != object_list().end(); ++io) {
			if (!(*io)->check_flag(QD_OBJ_IS_IN_INVENTORY_FLAG) && (*io)->need_redraw()) {
				add_redraw_region((*io)->last_screen_region());
				add_redraw_region((*io)->screen_region());
			}
		}

		if (!_fps_region.is_empty())
			dp->add_redraw_region(_fps_region);
		if (!_fps_region_last.is_empty())
			dp->add_redraw_region(_fps_region_last);
	}

	if (ConfMan.getBool("show_fps")) {
		if (fps_counter()->fps_value() > 0.0f)
			snprintf(_fps_string, 255, "%.1f fps", fps_counter()->fps_value());
		else
			snprintf(_fps_string, 255, "--");

		int sx = grDispatcher::instance()->textWidth(_fps_string);
		int sy = grDispatcher::instance()->textHeight(_fps_string);
		_fps_region = grScreenRegion(10 + sx / 2, 10 + sy / 2, sx, sy);
	} else
		_fps_region.clear();

	fps_counter()->quant();
}

void qdGameScene::post_redraw() {
	for (qdGameObjectList::const_iterator io = object_list().begin(); io != object_list().end(); ++io)
		(*io)->post_redraw();

	_fps_region_last = _fps_region;
}

qdConditionalObject::trigger_start_mode qdGameScene::trigger_start() {
	if (qdGameDispatcher *dp = qd_get_game_dispatcher()) {
		dp->set_next_scene(this);

		if (check_flag(RESET_TRIGGERS_ON_LOAD))
			dp->deactivate_scene_triggers(this);

		debug("[%d] Scene queued->%s", g_system->getMillis(), transCyrillic(name()));

		return qdConditionalObject::TRIGGER_START_ACTIVATE;
	}

	return qdConditionalObject::TRIGGER_START_FAILED;
}

bool qdGameScene::is_active() const {
	if (qdGameDispatcher *dp = qd_get_game_dispatcher())
		return (dp->get_active_scene() == this);

	return false;
}

void qdGameScene::update_mouse_cursor() {
	qdGameDispatcher *dp = qdGameDispatcher::get_dispatcher();
	if (!dp) return;

	if (dp->mouse_object()->object()) {
		if (const qdGameObjectState *p = dp->mouse_object()->object()->get_cur_state()) {
			if (p->has_mouse_cursor_ID()) {
				dp->mouse_object()->set_cursor_state(p->mouse_cursor_ID());
				return;
			}
		}
	}

	if (_mouse_hover_object) {
		int cursor = qdGameObjectState::CURSOR_UNASSIGNED;
		qdGameObjectMouse::cursor_ID_t obj_cursor = qdGameObjectMouse::OBJECT_CURSOR;

		if (const qdGameObjectAnimated *obj = dynamic_cast<const qdGameObjectAnimated *>(_mouse_hover_object)) {
			if (!obj->get_cur_state()->check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_INVENTORY))
				cursor = obj->mouse_cursor_ID();
			else
				obj_cursor = qdGameObjectMouse::INVENTORY_OBJECT_CURSOR;
		}

		if (cursor == qdGameObjectState::CURSOR_UNASSIGNED)
			dp->mouse_object()->set_cursor(obj_cursor);
		else
			dp->mouse_object()->set_cursor_state(cursor);
	} else {
		qdGameObjectMouse::cursor_ID_t mouse_cursor = qdGameObjectMouse::DEFAULT_CURSOR;

		qdInterfaceDispatcher *ip = qdInterfaceDispatcher::get_dispatcher();
		if (ip && ip->is_mouse_hover()) {
			if (qdGameDispatcher *dsp = qdGameDispatcher::get_dispatcher())
				mouse_cursor = qdGameObjectMouse::INGAME_INTERFACE_CURSOR;
		} else {
			Vect2f r = _camera.scr2plane(Vect2s(dp->mouse_cursor_pos().x, dp->mouse_cursor_pos().y));
			for (qdGridZoneList::const_iterator it = grid_zone_list().begin(); it != grid_zone_list().end(); ++it) {
				if ((*it)->check_flag(qdGridZone::ZONE_EXIT_FLAG) && (*it)->is_point_in_zone(r)) {
					mouse_cursor = qdGameObjectMouse::ZONE_CURSOR;
					break;
				}
			}
		}

		dp->mouse_object()->set_cursor(mouse_cursor);
	}
}

// Intersection of rectanfles with centers in c* and c and with sizes gr*
bool inters3f(const Vect3f &c1, const Vect3f &sz1, const Vect3f &c2, const Vect3f &sz2) {
	Vect3f a1, b1, a2, b2;
	a1 = c1 - sz1 / 2;
	b1 = a1 + sz1;
	a2 = c2 - sz2 / 2;
	b2 = a2 + sz2;

	if ((MAX(a1.x, a2.x) <= MIN(b1.x, b2.x)) &&
	        (MAX(a1.y, a2.y) <= MIN(b1.y, b2.y)) &&
	        (MAX(a1.z, a2.z) <= MIN(b1.z, b2.z)))
		return true;
	else return false;
}

bool inters2s(Vect2s c1, Vect2s sz1, Vect2s c2, Vect2s sz2) {
	Vect2s a1, b1, a2, b2;

	a1 = c1 - sz1 / 2;
	b1 = a1 + sz1;
	a2 = c2 - sz2 / 2;
	b2 = a2 + sz2;
	b1.x--;
	b1.y--;
	b2.x--;
	b2.y--;

	if ((MAX(a1.x, a2.x) <= MIN(b1.x, b2.x)) &&
	        (MAX(a1.y, a2.y) <= MIN(b1.y, b2.y)))
		return true;
	else return false;
}

void qdGameScene::follow_pers_init(int follow_cond) {
	for (personages_container_t::iterator it = _personages.begin(); it != _personages.end(); ++it) {
		(*it)->ref_circuit_objs().clear();
		(*it)->set_follow_condition(follow_cond);
		if (qdGameObjectMoving::FOLLOW_DONE == follow_cond)
			(*it)->set_last_move_order((*it)->R());
	}
}

bool qdGameScene::follow_path_seek(qdGameObjectMoving *pObj, bool lock_target) {
	if (qdGameObjectMoving::FOLLOW_UPDATE_PATH == pObj->follow_condition())
		_selected_object->set_grid_zone_attributes(sGridCell::CELL_SELECTED);

	return pObj->move(_selected_object->last_move_order(), lock_target);

	if (qdGameObjectMoving::FOLLOW_UPDATE_PATH == pObj->follow_condition())
		_selected_object->drop_grid_zone_attributes(sGridCell::CELL_SELECTED);
}

/*
bool qdGameScene::follow_path_seek(qdGameObjectMoving* pObj, bool lock_target)
{
    Vect3f dest_pnt = _selected_object->last_move_order();
    if (pObj->has_control_type(
          qdGameObjectMoving::CONTROL_ATTACHMENT_TO_ACTIVE_WITH_MOVING))
    {
        dest_pnt.x += pObj->attach_shift().x;
        dest_pnt.y += pObj->attach_shift().y;
    };

    Vect3f dist_vec = dest_pnt - pObj->R();
    // Если точка, к которой нужно приблизиться достаточно близка, то идти к ней не нужно, но все путь найден удачно
    // (пустой путь)
    if (dist_vec.norm() <= pObj->follow_max_radius())
        return true;
    else
    {
        // Ищем точку, к которой сможем пойти (min)
        int lin  = pObj->follow_min_radius();
        int diag = round(lin * (0.7071067811)); // 0.7071067811 = sqrt(2)/2
        Vect2s test_pnt;
        Vect2s min;
        double min_dist = 1e100;
        Vect2s center;
        center.x = static_cast<int16>(dest_pnt.x);
        center.y = static_cast<int16>(dest_pnt.y);
        Std::vector<Vect2s> pts_vec;
        for (int i = -1; i <= 1; i++)
            for (int j = -1; j <= 1; j++)
            {
                if ((0 == i) && (0 == j)) continue;
                if (min_dist < 1e100) break;

                test_pnt = center;
                // Для диагональных сумма кратна 2
                if (0 == ((i + j) % 2))
                {
                    test_pnt.x += i*diag;
                    test_pnt.y += j*diag;
                }
                else
                {
                    test_pnt.x += i*lin;
                    test_pnt.y += j*lin;
                };

                // Кладем все допустимые ячейки для подхода в массив (в порядки возростания длины)
                // Допустимой считаем ячейку, которая проходима. Выделенные и прочие
                // ячейки не учитываем, потому как они не постоянны - мы считаем, что к ним
                // можно бежать
                if (!qdCamera::current_camera()->check_grid_attributes(
                      test_pnt,
                      Vect2s(1,1),
                      sGridCell::CELL_IMPASSABLE)
                   )
                {
                    Vect3f buf = qdCamera::current_camera()->get_cell_coords(test_pnt);
                    Vect3f ins_pnt_dist = buf - pObj->R();
                    bool is_ins = false;
                    for (Std::vector<Vect2s>::iterator ins_it = pts_vec.begin();
                         ins_it != pts_vec.end();
                         ++ins_it)
                    {
                        Vect3f cur_pnt_dist;
                        cur_pnt_dist = qdCamera::current_camera()->get_cell_coords(*ins_it) - pObj->R();
                        // Добавляем перед первой точкой, превосходящей расстоянием
                        if (ins_pnt_dist.norm2() < cur_pnt_dist.norm2())
                        {
                            is_ins = true;
                            pts_vec.insert(ins_it, test_pnt);
                            break;
                        }
                    }
                    if (false == is_ins) pts_vec.push_back(test_pnt);
                }
            };
        // Добавляем точку непосредственного подхода
        if (0 != pObj->follow_max_radius())
        {
            Vect3f dist = dest_pnt - pObj->R();
            float mul = 1 - pObj->follow_max_radius()/dist.norm();
            test_pnt.x = (dest_pnt.x - pObj->R().x)*mul + pObj->R().x;
            test_pnt.y = (dest_pnt.y - pObj->R().y)*mul + pObj->R().y;
            // В начало, как, очевидно, самую близкую
            pts_vec.insert(pts_vec.begin(), test_pnt);
        }
        // Пытаемся подойти ко всем точкам. При первой удачной выходим
        for (Std::vector<Vect2s>::const_iterator it = pts_vec.begin();
             it != pts_vec.end();
             ++it)
        {
            if (true == pObj->move(Vect3f((*it).x, (*it).y, 0), lock_target))
                return true;
        }
    };

    return false; // Не удалось найти путь
}
*/

void qdGameScene::follow_implement_update_path() {
	for (personages_container_t::iterator it = _personages.begin(); it != _personages.end(); ++it) {
		// Следование за активным персонажем, а так же следов. за точкой привязки к активному.
		// Ищем новую точку для следования, только если персонаж не пропускает сейчас активного
		if ((
		            (*it)->has_control_type(qdGameObjectMoving::CONTROL_FOLLOW_ACTIVE_PERSONAGE) ||
		            (*it)->has_control_type(
		                qdGameObjectMoving::CONTROL_ATTACHMENT_TO_ACTIVE_WITH_MOVING)
		        ) &&
		        (NULL != _selected_object) &&
		        (*it != _selected_object) &&
		        (qdGameObjectMoving::FOLLOW_UPDATE_PATH == (*it)->follow_condition()) &&
		        (*it)->can_move()
		   ) {
			Vect3f dist = _selected_object->R() - (*it)->R();
			// Если активный близко и движется, то никуда не идем (ждем пока отойдет)
			if ((_selected_object->is_moving()) &&
			        (dist.norm() < (*it)->follow_min_radius())) continue;
			// Пытаемся найти путь, который будет идти непосредственно к цели (lock_target = true)
			// иначе будет плохо следовать, довольствясь подоходом к краю препятствия

			if (follow_path_seek((*it), true))
				(*it)->set_follow_condition(qdGameObjectMoving::FOLLOW_MOVING);
			else
				(*it)->set_follow_condition(qdGameObjectMoving::FOLLOW_WAIT);
		}
	};
}

void qdGameScene::follow_wakening() {
	// Возобновление следования если движущиеся и участвующие в следовании достаточно
	// удалились
	for (personages_container_t::iterator it1 = _personages.begin(); it1 != _personages.end(); ++it1)
		if ((
		            (_selected_object == (*it1)) ||
		            (*it1)->has_control_type(qdGameObjectMoving::CONTROL_FOLLOW_ACTIVE_PERSONAGE) ||
		            (*it1)->has_control_type(
		                qdGameObjectMoving::CONTROL_ATTACHMENT_TO_ACTIVE_WITH_MOVING)
		        ) &&
		        (qdGameObjectMoving::FOLLOW_WAIT == (*it1)->follow_condition())) {
			Vect3f dist_vec;
			bool all_follow_moving_far = true;
			for (personages_container_t::iterator it2 = _personages.begin(); it2 != _personages.end(); ++it2)
				if ((
				            (_selected_object == (*it2)) ||
				            (*it2)->has_control_type(
				                qdGameObjectMoving::CONTROL_FOLLOW_ACTIVE_PERSONAGE) ||
				            (*it2)->has_control_type(
				                qdGameObjectMoving::CONTROL_ATTACHMENT_TO_ACTIVE_WITH_MOVING)
				        ) &&
				        (it1 != it2)) {
					dist_vec = (*it1)->R() - (*it2)->R();
					// Считаем далеким, если расстояние между персонажами в два раза
					// превышает сумму их collision_radius.
					if (dist_vec.norm() < 2 * ((*it1)->collision_radius() + (*it2)->collision_radius())) {
						all_follow_moving_far = false;
						break;
					}
				}

			if (true == all_follow_moving_far) {
				bool any_move = false;
				if (((*it1) == _selected_object) && ((*it1)->move((*it1)->last_move_order(), false)))
					any_move = true;

				if ((*it1) != _selected_object) {
					dist_vec = _selected_object->R() - (*it1)->R();
					// Если достаточно далеко от следующего, то не учитываем занятое активным при поиске пути
					if (dist_vec.norm2() > sqr(_selected_object->collision_radius() + 10 + (*it1)->collision_radius()))
						_selected_object->set_grid_zone_attributes(sGridCell::CELL_SELECTED);

					follow_path_seek(*it1, false);

					_selected_object->drop_grid_zone_attributes(sGridCell::CELL_SELECTED);
					any_move = true;
				}

				if (any_move)
					(*it1)->set_follow_condition(qdGameObjectMoving::FOLLOW_MOVING);
				else
					(*it1)->set_follow_condition(qdGameObjectMoving::FOLLOW_FULL_STOP_WAIT);
			}
		}

	// Смотрим, все ли участники следования остановились
	bool is_all_stay = true;
	for (personages_container_t::iterator it = _personages.begin(); it != _personages.end(); ++it)
		// Ждем пока остановятся ВСЕ персонажи
		if (/*
            (
              (_selected_object == (*it)) ||
              (*it)->has_control_type(qdGameObjectMoving::CONTROL_FOLLOW_ACTIVE_PERSONAGE) ||
              (*it)->has_control_type(
                qdGameObjectMoving::CONTROL_ATTACHMENT_TO_ACTIVE_WITH_MOVING)
            ) &&
            */
		    (*it)->is_moving()) {
			is_all_stay = false;
			break;
		}

	// Если все стоят, то одному из ждущих можно попытаться пройти к точке следования
	if (is_all_stay) {
		for (personages_container_t::iterator it = _personages.begin(); it != _personages.end(); ++it)
			if ((
			            (_selected_object == (*it)) ||
			            (*it)->has_control_type(qdGameObjectMoving::CONTROL_FOLLOW_ACTIVE_PERSONAGE) ||
			            (*it)->has_control_type(
			                qdGameObjectMoving::CONTROL_ATTACHMENT_TO_ACTIVE_WITH_MOVING)
			        ) &&
			        (
			            (qdGameObjectMoving::FOLLOW_WAIT == (*it)->follow_condition()) ||
			            (qdGameObjectMoving::FOLLOW_FULL_STOP_WAIT == (*it)->follow_condition())
			        ) &&
			        (*it)->can_move()) {
				bool any_move = false;
				if (((*it) == _selected_object) && ((*it)->move((*it)->last_move_order(), false)))
					any_move = true;

				if ((*it) != _selected_object) {
					Vect3f dist_vec = _selected_object->R() - (*it)->R();
					// Если достаточно далеко от следующего, то не учитываем занятое активным при поиске пути
					if (dist_vec.norm2() > sqr(_selected_object->collision_radius() + 10 + (*it)->collision_radius()))
						_selected_object->set_grid_zone_attributes(sGridCell::CELL_SELECTED);

					follow_path_seek(*it, false);

					_selected_object->drop_grid_zone_attributes(sGridCell::CELL_SELECTED);
					any_move = true;
				}

				if (any_move) {
					// Если получилось идти, то сразу прерываем попытки, чтобы остальные ждущие остались на месте
					(*it)->set_follow_condition(qdGameObjectMoving::FOLLOW_MOVING);
					return;
				}
			}
		// Дошли досюда. Значит никого двинуть не удалось. Значит безнадежно - все персонажи перестают ждать.
		follow_pers_init(qdGameObjectMoving::FOLLOW_DONE);
	}
}

void qdGameScene::follow_circuit(float dt) {
	for (personages_container_t::iterator it1 = _personages.begin(); it1 != _personages.end(); ++it1) {
		bool is_it1_follow = (_selected_object == (*it1)) ||
		                     (*it1)->has_control_type(qdGameObjectMoving::CONTROL_FOLLOW_ACTIVE_PERSONAGE) ||
		                     (*it1)->has_control_type(qdGameObjectMoving::CONTROL_ATTACHMENT_TO_ACTIVE_WITH_MOVING);
		// не следующих и стоящих не обрабатываем как
		if ((false == is_it1_follow) || (false == (*it1)->is_moving()) ||
		        (qdGameObjectMoving::FOLLOW_MOVING != (*it1)->follow_condition()))
			continue;

		Vect2s it1_cur_pos, it1_cur_grid, it1_next_pos, it1_next_grid;
		(*it1)->calc_cur_and_future_walk_grid(dt, it1_cur_pos, it1_cur_grid, it1_next_pos, it1_next_grid);

		for (personages_container_t::iterator it2 = _personages.begin(); it2 != _personages.end(); ++it2) {
			if ((*it1) == (*it2))
				continue;

			Vect2s it2_cur_pos, it2_cur_grid, it2_next_pos, it2_next_grid;
			(*it2)->calc_cur_and_future_walk_grid(dt, it2_cur_pos, it2_cur_grid, it2_next_pos, it2_next_grid);
			bool is_it2_follow = (_selected_object == (*it2)) ||
			                     (*it2)->has_control_type(qdGameObjectMoving::CONTROL_FOLLOW_ACTIVE_PERSONAGE) ||
			                     (*it2)->has_control_type(qdGameObjectMoving::CONTROL_ATTACHMENT_TO_ACTIVE_WITH_MOVING);

			// Если на следующем шаге не пересекаются, то все в порядке
			if (!inters2s(it1_next_pos, it1_next_grid, it2_next_pos, it2_next_grid)) {
				// Если it2 с которым НЕ пересекся текущий, которого обходим
				// то удаляем его из списка обходимых текущим
				for (Std::vector<const qdGameObjectMoving *>::iterator cir_it = (*it1)->ref_circuit_objs().begin();
				        cir_it != (*it1)->ref_circuit_objs().end(); ++cir_it)
					if ((*cir_it) == (*it2)) {
						(*it1)->ref_circuit_objs().erase(cir_it);
						break;
					}
				continue;
			}

			// Сначала пытаемся стопить второго для решения проблемы
			if (is_it2_follow && (*it2)->is_moving() &&
			        (false == inters2s(it1_next_pos, it1_next_grid, it2_cur_pos, it2_cur_grid))) {
				(*it2)->set_follow_condition(qdGameObjectMoving::FOLLOW_WAIT);
				(*it2)->stop_movement();
				continue;
			}

			// Если it1 пытается обойти it2, то не учитываем их пересечение
			bool it2_is_circuit = false;
			for (Std::vector<const qdGameObjectMoving *>::iterator cir_it = (*it1)->ref_circuit_objs().begin();
			        cir_it != (*it1)->ref_circuit_objs().end(); ++cir_it)
				if ((*cir_it) == (*it2)) {
					it2_is_circuit = true;
					break;
				}
			if (it2_is_circuit) continue;

			// Пытаемся обойти второго, если он участвует в следовании или стоит.
			// Стопим второго при удача, а сами продолжаем обход.
			if ((*it1)->can_move() && (is_it2_follow || !(*it2)->is_moving())) {
				if (true == (*it1)->move((*it1)->last_move_order(), false)) {
					(*it1)->ref_circuit_objs().push_back((*it2));
					(*it2)->set_follow_condition(qdGameObjectMoving::FOLLOW_WAIT);
					(*it2)->stop_movement();
					continue;
				}
			}

			// Ничего не помогло => стопим первого до поры, когда все движущиеся
			// остановяться
			(*it1)->set_follow_condition(qdGameObjectMoving::FOLLOW_FULL_STOP_WAIT);
			(*it1)->stop_movement();
		} // 2-for...
	} // 1-for...
}

void qdGameScene::follow_end_moving() {
	if (NULL == _selected_object) return;
	for (personages_container_t::iterator it = _personages.begin(); it != _personages.end(); ++it)
		if ((
		            (*it)->has_control_type(qdGameObjectMoving::CONTROL_FOLLOW_ACTIVE_PERSONAGE) ||
		            (*it)->has_control_type(qdGameObjectMoving::CONTROL_ATTACHMENT_TO_ACTIVE_WITH_MOVING)
		        ) &&
		        (qdGameObjectMoving::FOLLOW_MOVING == (*it)->follow_condition()) &&
		        (_selected_object != (*it))) {
			Vect3f dist = _selected_object->last_move_order() - (*it)->R();
			if (qdGameObjectMoving::FOLLOW_DONE == _selected_object->follow_condition())
				dist = _selected_object->R() - (*it)->R();

			if (dist.norm() <= (*it)->follow_min_radius()) {
				(*it)->stop_movement();
				(*it)->set_follow_condition(qdGameObjectMoving::FOLLOW_DONE);
			}
		}
}

void qdGameScene::follow_quant(float dt) {
	follow_implement_update_path();
	follow_wakening();
	follow_circuit(dt);
	follow_end_moving();
}

void qdGameScene::collision_quant() {
	for (personages_container_t::iterator it = _personages.begin(); it != _personages.end(); ++it) {
		// Жесткая привязка с учетом направления привязывающего и без
		if (((*it)->has_control_type(qdGameObjectMoving::CONTROL_ATTACHMENT_WITH_DIR_REL) ||
		        (*it)->has_control_type(qdGameObjectMoving::CONTROL_ATTACHMENT_WITHOUT_DIR_REL))
		        &&
		        (NULL != (*it)->attacher())) {
			Vect2s shift = (*it)->attach_shift();
			// Учитываем направление, если нужно
			if ((*it)->has_control_type(qdGameObjectMoving::CONTROL_ATTACHMENT_WITH_DIR_REL)) {
				Vect2s buf = shift;
				double ang = (*it)->attacher()->direction_angle();
				shift.x = round(buf.x * cos(ang) - buf.y * sin(ang));
				shift.y = round(buf.x * sin(ang) + buf.y * cos(ang));
			}

			Vect3f vec = (*it)->attacher()->R();
			vec.x += shift.x;
			vec.y += shift.y;

			(*it)->set_pos(vec);
		};
	}; // for(...)

	if ((NULL == _selected_object) || !_selected_object->is_moving()) return;

	bool pflag = false;

	for (personages_container_t::iterator it = _personages.begin(); it != _personages.end(); ++it) {
		if (*it != _selected_object && !(*it)->has_control_type(qdGameObjectMoving::CONTROL_ATTACHMENT_WITHOUT_DIR_REL) && !(*it)->has_control_type(qdGameObjectMoving::CONTROL_ATTACHMENT_WITH_DIR_REL) && (*it)->can_move()) {
			Vect3f dr = _selected_object->R() - (*it)->R();

			float dist = _selected_object->collision_radius() + (*it)->collision_radius();
			float angle = _selected_object->calc_direction_angle((*it)->R());

			if (dr.norm() < dist) {
				if (fabs(getDeltaAngle(angle, _selected_object->direction_angle())) < M_PI / 2.0f) {
					if ((*it)->has_control_type(qdGameObjectMoving::CONTROL_COLLISION))
						(*it)->set_movement_impulse(_selected_object->direction_angle());
				}
			}

			if (dr.norm() < dist) {
				if (fabs(getDeltaAngle(angle, _selected_object->direction_angle())) < M_PI / 2.0f) {
					if ((*it)->has_control_type(qdGameObjectMoving::CONTROL_AVOID_COLLISION))
						(*it)->avoid_collision(_selected_object);
				}
			}

			if (!(*it)->is_moving())
				pflag = true;
		}
	}

	if (pflag && _selected_object->has_control_type(qdGameObjectMoving::CONTROL_CLEAR_PATH)) {
		_selected_object->set_path_attributes(sGridCell::CELL_PERSONAGE_PATH);
		for (personages_container_t::iterator it = _personages.begin(); it != _personages.end(); ++it) {
			if (!(*it)->has_control_type(qdGameObjectMoving::CONTROL_ATTACHMENT_WITHOUT_DIR_REL) && !(*it)->has_control_type(qdGameObjectMoving::CONTROL_ATTACHMENT_WITH_DIR_REL)) {
				if (*it != _selected_object && (*it)->can_move() && !(*it)->is_moving() && (*it)->check_grid_zone_attributes(sGridCell::CELL_PERSONAGE_PATH)) {
					(*it)->move_from_personage_path();
				}
			}
		}
		_selected_object->clear_path_attributes(sGridCell::CELL_PERSONAGE_PATH);
	}


	// Повторение движений активного персонажа
	for (personages_container_t::iterator it = _personages.begin(); it != _personages.end(); ++it)
		if ((*it != _selected_object) &&
		        (*it)->has_control_type(qdGameObjectMoving::CONTROL_REPEAT_ACTIVE_PERSONAGE_MOVEMENT) &&
		        (*it)->can_move()) {
			(*it)->set_movement_impulse(_selected_object->direction_angle());
		};
}

void qdGameScene::add_redraw_region(const grScreenRegion &reg) const {
	qdGameDispatcher *dp = qdGameDispatcher::get_dispatcher();

	dp->add_redraw_region(reg);

	if (check_flag(CYCLE_X)) {
		grScreenRegion reg1 = reg;

		reg1.move(-_camera.get_scr_sx(), 0);
		dp->add_redraw_region(reg1);

		reg1.move(_camera.get_scr_sx() * 2, 0);
		dp->add_redraw_region(reg1);
	}

	if (check_flag(CYCLE_Y)) {
		grScreenRegion reg1 = reg;

		reg1.move(0, -_camera.get_scr_sy());
		dp->add_redraw_region(reg1);

		reg1.move(0, _camera.get_scr_sy() * 2);
		dp->add_redraw_region(reg1);
	}

	if (check_flag(CYCLE_X) && check_flag(CYCLE_Y)) {
		grScreenRegion reg1 = reg;

		reg1.move(-_camera.get_scr_sx(), -_camera.get_scr_sy());
		dp->add_redraw_region(reg1);

		reg1.move(_camera.get_scr_sx() * 2, 0);
		dp->add_redraw_region(reg1);

		reg1.move(0, _camera.get_scr_sy() * 2);
		dp->add_redraw_region(reg1);

		reg1.move(-_camera.get_scr_sx() * 2, 0);
		dp->add_redraw_region(reg1);
	}
}

bool qdGameScene::is_any_personage_in_zone(const qdGridZone *p) const {
	for (personages_container_t::const_iterator it = _personages.begin(); it != _personages.end(); ++it) {
		if (p->is_object_in_zone(*it))
			return true;
	}
	return false;
}

bool qdGameScene::set_personage_button(qdInterfaceButton *p) {
	bool ret = false;
	for (personages_container_t::const_iterator it = _personages.begin(); it != _personages.end(); ++it) {
		if (p->has_event(qdInterfaceEvent::EVENT_ACTIVATE_PERSONAGE, (*it)->name())) {
			(*it)->set_button(p);
			ret = true;
		}
	}

	return ret;
}

bool qdGameScene::get_files_list(qdFileNameList &files_to_copy, qdFileNameList &files_to_pack) const {
	for (qdMusicTrackList::const_iterator it = music_track_list().begin(); it != music_track_list().end(); ++it)
		files_to_copy.push_back((*it)->file_name());

	for (qdSoundList::const_iterator it = sound_list().begin(); it != sound_list().end(); ++it)
		files_to_pack.push_back((*it)->file_name());

	for (qdAnimationList::const_iterator it = animation_list().begin(); it != animation_list().end(); ++it)
		files_to_pack.push_back((*it)->qda_file());

	for (qdGameObjectList::const_iterator it = object_list().begin(); it != object_list().end(); ++it) {
		if ((*it)->named_object_type() == QD_NAMED_OBJECT_STATIC_OBJ) {
			qdGameObjectStatic *obj = static_cast<qdGameObjectStatic *>(*it);
			if (obj->get_sprite()->file())
				files_to_pack.push_back(obj->get_sprite()->file());
		}
	}

	return true;
}


void qdGameScene::personages_quant() {
	for (personages_container_t::const_iterator it = _personages.begin(); it != _personages.end(); ++it) {
		if ((*it)->button()) {
			if ((*it)->is_visible()) {
				if (!(*it)->button()->is_visible()) {
					(*it)->button()->show();
					if (qdInterfaceScreen *sp = dynamic_cast<qdInterfaceScreen *>((*it)->button()->owner()))
						sp->build_visible_elements_list();
				}

				if (get_active_personage() == *it)
					(*it)->button()->activate_state(1);
				else
					(*it)->button()->activate_state(0);
			} else
				(*it)->button()->hide();
		}
	}

	if (check_flag(CYCLE_X | CYCLE_Y)) {
		for (personages_container_t::iterator it = _personages.begin(); it != _personages.end(); ++it) {
			Vect3f r = (*it)->R();
			r.z = 0.0f;

			Vect2s scr_r = _camera.plane2rscr(r);
			Vect2s scr_r0 = scr_r;

			if (check_flag(CYCLE_X)) {
				if (scr_r.x > _camera.get_scr_sx() / 2 + _camera.get_scr_offset().x)
					scr_r.x -= _camera.get_scr_sx();
				else if (scr_r.x < -_camera.get_scr_sx() / 2 + _camera.get_scr_offset().x)
					scr_r.x += _camera.get_scr_sx();
			}
			if (check_flag(CYCLE_Y)) {
				if (scr_r.y > _camera.get_scr_sy() / 2 + _camera.get_scr_offset().y)
					scr_r.y -= _camera.get_scr_sy();
				else if (scr_r.y < -_camera.get_scr_sy() / 2 + _camera.get_scr_offset().y)
					scr_r.y += _camera.get_scr_sy();
			}

			if (!(scr_r0 == scr_r)) {
				r = _camera.rscr2plane(scr_r);
				(*it)->set_pos(r);
				(*it)->adjust_z();
			}
		}
	}

	for (personages_container_t::iterator itp = _personages.begin(); itp != _personages.end(); ++itp)
		(*itp)->clear_shadow();

	for (qdGridZoneList::const_iterator it = grid_zone_list().begin(); it != grid_zone_list().end(); ++it) {
		if ((*it)->has_shadow()) {
			for (personages_container_t::iterator itp = _personages.begin(); itp != _personages.end(); ++itp) {
				if ((*it)->is_object_in_zone(*itp))
					(*itp)->set_shadow((*it)->shadow_color(), (*it)->shadow_alpha());
			}
		}
	}
}

bool qdGameScene::need_to_redraw_inventory(const char *inventory_name) const {
	for (personages_container_t::const_iterator it = _personages.begin(); it != _personages.end(); ++it) {
		if (!strcmp((*it)->inventory_name(), inventory_name))
			return true;
	}
	return false;
}

void qdGameScene::create_minigame_objects() {
	Common::String name_buf;
	for (qdMiniGame::config_container_t::const_iterator it = _minigame->config().begin(); it != _minigame->config().end(); ++it) {
		if (it->data_type() == qdMinigameConfigParameter::PRM_DATA_OBJECT) {
			if (const qdGameObject *obj = get_object(it->data_string())) {
				for (int i = 0; i < it->data_count(); i++) {
					name_buf = Common::String::format("%s%03d", obj->name(), i);

					qdGameObject *new_obj = NULL;
					switch (obj->named_object_type()) {
					case QD_NAMED_OBJECT_ANIMATED_OBJ:
						new_obj = new qdGameObjectAnimated(*static_cast<const qdGameObjectAnimated *>(obj));
						break;
					case QD_NAMED_OBJECT_MOVING_OBJ:
						new_obj = new qdGameObjectMoving(*static_cast<const qdGameObjectMoving *>(obj));
						break;
					}

					if (new_obj) {
						new_obj->set_name(name_buf.c_str());
						add_object(new_obj);
					}
				}
			}
		}
	}
}

bool qdGameScene::set_camera_mode(const qdCameraMode &mode, qdGameObjectAnimated *object) {
	if (!_camera.can_change_mode())
		return false;

	if (object && object->named_object_type() == QD_NAMED_OBJECT_MOVING_OBJ && object != _selected_object)
		return false;

	_camera.set_mode(mode, object);
	return true;
}

void qdGameScene::start_minigame() {
	if (_minigame)
		_minigame->start();
}

} // namespace QDEngine
