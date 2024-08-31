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
#include "common/savefile.h"

#include "qdengine/qdengine.h"
#include "qdengine/qd_fwd.h"
#include "qdengine/parser/xml_tag_buffer.h"
#include "qdengine/parser/qdscr_parser.h"

#include "qdengine/system/input/mouse_input.h"

#include "qdengine/qdcore/qd_interface_dispatcher.h"
#include "qdengine/qdcore/qd_interface_screen.h"
#include "qdengine/qdcore/qd_interface_element_state.h"
#include "qdengine/qdcore/qd_interface_save.h"
#include "qdengine/qdcore/qd_interface_button.h"
#include "qdengine/qdcore/qd_game_dispatcher.h"
#include "qdengine/qdcore/qd_game_object_moving.h"
#include "qdengine/qdcore/qd_game_scene.h"
#include "qdengine/qdcore/qd_setup.h"
#include "qdengine/qdcore/qd_interface_text_window.h"
#include "qdengine/qdcore/qd_resource.h"


namespace QDEngine {

qdInterfaceDispatcher *qdInterfaceDispatcher::_dispatcher = NULL;

qdInterfaceDispatcher::qdInterfaceDispatcher() : _cur_screen(NULL),
	_next_screen(NULL),
	_is_active(false),
	_is_mouse_hover(false),
	_autohide_disable(false),
	_need_full_redraw(false),
	_need_scene_redraw(false),
	_end_game_mode(false),
	_need_save_screenshot(true),
	_need_show_save_time(false),
	_need_show_save_title(false),
	_save_font_type(QD_FONT_TYPE_NONE),
	_save_font_color(0x00FFFFFF),
	_background_screen(NULL),
	_background_screen_lock(false),
	_modalScreenMode(MODAL_SCREEN_OTHER) {
}

qdInterfaceDispatcher::~qdInterfaceDispatcher() {
	_screens.clear();
}

bool qdInterfaceDispatcher::add_screen(qdInterfaceScreen *scr) {
	return _screens.add_object(scr);
}

bool qdInterfaceDispatcher::rename_screen(qdInterfaceScreen *scr, const char *name) {
	return _screens.rename_object(scr, name);
}

bool qdInterfaceDispatcher::remove_screen(qdInterfaceScreen *scr) {
	return _screens.remove_object(scr);
}

qdInterfaceScreen *qdInterfaceDispatcher::get_screen(const char *screen_name) {
	if (!screen_name) return NULL;

	return _screens.get_object(screen_name);
}

bool qdInterfaceDispatcher::is_screen_in_list(const qdInterfaceScreen *scr) {
	return _screens.is_in_list(scr);
}

bool qdInterfaceDispatcher::select_screen(const char *screen_name, bool lock_resources) {
	qdInterfaceScreen *p = get_screen(screen_name);

	if (p) {
		if (_cur_screen && _cur_screen->is_locked()) {
			debugC(3, kDebugQuant, "qdInterfaceDispatcher::select_screen() Selecting screen: %s", transCyrillic(screen_name));
			for (resource_container_t::resource_list_t::const_iterator it = _resources.resource_list().begin(); it != _resources.resource_list().end(); ++it) {
				if (p->has_references(*it)) {
					if (!(*it)->is_resource_loaded()) {
						debugC(3, kDebugQuant, "qdInterfaceDispatcher::select_screen() Resource is used in both screens %s and %s", transCyrillic(_cur_screen->name()), transCyrillic(p->name()));
						(*it)->load_resource();
					}
				} else {
					if ((*it)->is_resource_loaded() && !_cur_screen->has_references(*it))
						(*it)->free_resource();
				}
			}
		} else {
			for (resource_container_t::resource_list_t::const_iterator it = _resources.resource_list().begin(); it != _resources.resource_list().end(); ++it) {
				if (p->has_references(*it)) {
					if (!(*it)->is_resource_loaded())
						(*it)->load_resource();
				} else {
					if ((*it)->is_resource_loaded())
						(*it)->free_resource();
				}
			}
		}
	} else {
		if (_cur_screen && _cur_screen->is_locked()) {
			for (resource_container_t::resource_list_t::const_iterator it = _resources.resource_list().begin(); it != _resources.resource_list().end(); ++it) {
				if ((*it)->is_resource_loaded() && !_cur_screen->has_references(*it))
					(*it)->free_resource();
			}
		} else {
			for (resource_container_t::resource_list_t::const_iterator it = _resources.resource_list().begin(); it != _resources.resource_list().end(); ++it) {
				if ((*it)->is_resource_loaded())
					(*it)->free_resource();
			}
		}
	}

	if (p && _cur_screen != p)
		p->set_autohide_phase(1.0f);

	_cur_screen = p;

	if (_cur_screen) {
		debugC(3, kDebugQuant, "qdInterfaceDispatcher::select_screen() if(cur_scene_): %s", transCyrillic(screen_name));
		bool is_game_active = false;
		if (qdGameDispatcher * dp = qdGameDispatcher::get_dispatcher()) {
			if (dp->get_active_scene())
				is_game_active = true;

			if (_cur_screen->has_music_track())
				dp->play_music_track(&_cur_screen->music_track(), true);
		}

		_cur_screen->init(is_game_active);
		if (lock_resources)
			_cur_screen->lock_resources();
	}

	_need_full_redraw = true;

	return true;
}

bool qdInterfaceDispatcher::select_background_screen(qdInterfaceScreen *p) {
	if (_background_screen && _background_screen != p) {
		if (!_background_screen_lock)
			_background_screen->unlock_resources();
		_background_screen = 0;
		_need_full_redraw = true;
	}

	if (p) {
		_background_screen = p;
		_background_screen_lock = p->is_locked();
		p->lock_resources();
		_need_full_redraw = true;
	}

	return true;
}

bool qdInterfaceDispatcher::select_ingame_screen(bool inventory_state) {
	if (has_ingame_screen(inventory_state)) {
		debugC(3, kDebugQuant, "qdInterfaceDispatcher::select_ingame_screen(): Selecting ingame screen: %s", transCyrillic(ingame_screen_name(inventory_state)));
		return select_screen(ingame_screen_name(inventory_state), true);
	}

	return select_screen(NULL);
}

qdResource *qdInterfaceDispatcher::add_resource(const Common::Path file_name, const qdInterfaceElementState *owner) {
	return _resources.add_resource(file_name, owner);
}

bool qdInterfaceDispatcher::remove_resource(const Common::Path file_name, const qdInterfaceElementState *owner) {
	return _resources.remove_resource(file_name, owner);
}

bool qdInterfaceDispatcher::redraw(int dx, int dy) const {
	if (_background_screen)
		_background_screen->redraw(dx, dy);

	if (_cur_screen)
		return _cur_screen->redraw(dx, dy);

	return false;
}

bool qdInterfaceDispatcher::pre_redraw() {
	if (_cur_screen)
		_cur_screen->pre_redraw(_need_full_redraw);

	if (_background_screen)
		_background_screen->pre_redraw(_need_full_redraw);

	return false;
}

bool qdInterfaceDispatcher::post_redraw() {
	_need_full_redraw = false;

	if (_cur_screen)
		return _cur_screen->post_redraw();

	if (_background_screen)
		_background_screen->post_redraw();

	return false;
}

bool qdInterfaceDispatcher::quant(float dt) {
	if (_next_screen) {
		select_screen(_next_screen);
		set_next_screen(NULL);
	}

	if (_cur_screen)
		return _cur_screen->quant(dt);

	return false;
}

bool qdInterfaceDispatcher::mouse_handler(int x, int y, mouseDispatcher::mouseEvent ev) {
	debugC(9, kDebugInput, "qdInterfaceDispatcher::mouse_handler() x: %d, y: %d, ev: %d", x, y, ev);
	_is_mouse_hover = false;
	_autohide_disable = false;

	if (_end_game_mode) {
		debugC(3, kDebugInput, "qdInterfaceDispatcher::mouse_handler() _end_game_mode: %d", _end_game_mode);
		if (ev == mouseDispatcher::EV_LEFT_DOWN || ev == mouseDispatcher::EV_RIGHT_DOWN) {
			handle_event(qdInterfaceEvent::EVENT_CHANGE_INTERFACE_SCREEN, main_menu_screen_name());
			_end_game_mode = false;
			return true;
		}
	}

	if (_cur_screen) {
		debugC(9, kDebugInput, "qdInterfaceDispatcher::mouse_handler() _cur_screen");
		return _cur_screen->mouse_handler(x, y, ev);
	}

	return false;
}

bool qdInterfaceDispatcher::keyboard_handler(Common::KeyCode vkey) {
	if (_cur_screen) {
		if (vkey == Common::KEYCODE_ESCAPE && has_main_menu()) {
			if (_cur_screen->name() && !strcmp(_cur_screen->name(), main_menu_screen_name()))
				handle_event(qdInterfaceEvent::EVENT_RESUME_GAME, NULL);
			else {
				if (qdGameDispatcher::get_dispatcher()->is_main_menu_exit_enabled())
					select_screen(main_menu_screen_name());
			}

			return true;
		}

		return _cur_screen->keyboard_handler(vkey);
	}

	return false;
}

bool qdInterfaceDispatcher::char_input_handler(int vkey) {
	if (_cur_screen)
		return _cur_screen->char_input_handler(vkey);

	return false;
}

bool qdInterfaceDispatcher::save_script(Common::WriteStream &fh, int indent) const {
	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}

	fh.writeString("<interface");

	if (has_main_menu()) {
		fh.writeString(Common::String::format(" main_menu=\"%s\"", qdscr_XML_string(main_menu_screen_name())));
	}

	if (need_scene_redraw()) {
		fh.writeString(" draw_scene=\"1\"");
	}

	if (has_ingame_screen(false)) {
		fh.writeString(Common::String::format(" ingame_screen0=\"%s\"", qdscr_XML_string(ingame_screen_name(false))));
	}

	if (has_ingame_screen(true)) {
		fh.writeString(Common::String::format(" ingame_screen1=\"%s\"", qdscr_XML_string(ingame_screen_name(true))));
	}

	if (!_save_prompt_screen_name.empty()) {
		fh.writeString(Common::String::format(" save_prompt_screen=\"%s\"", qdscr_XML_string(_save_prompt_screen_name.c_str())));
	}

	if (!_save_title_screen_name.empty()) {
		fh.writeString(Common::String::format(" save_title_screen=\"%s\"", qdscr_XML_string(_save_title_screen_name.c_str())));
	}

	// Значение по умолчанию - true, значит сохраняем только false
	if (!_need_save_screenshot) {
		fh.writeString(" need_save_screenshot=\"0\"");
	}

	if (_need_show_save_time) {
		fh.writeString(" need_show_save_time=\"1\"");
	}

	if (_need_show_save_title) {
		fh.writeString(" need_show_save_name=\"1\"");
	}

	if (QD_FONT_TYPE_NONE != _save_font_type) {
		fh.writeString(Common::String::format(" save_font_type=\"%d\"", _save_font_type));
	}

	if (0x00FFFFFF != _save_font_color) {
		fh.writeString(Common::String::format(" save_font_color=\"%d\"", _save_font_color));
	}

	fh.writeString(">\r\n");


	for (auto &it : screen_list()) {
		it->save_script(fh, indent + 1);
	}

	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}

	fh.writeString("</interface>\r\n");

	return true;
}

bool qdInterfaceDispatcher::load_script(const xml::tag *p) {
	for (xml::tag::subtag_iterator it = p->subtags_begin(); it != p->subtags_end(); ++it) {
		switch (it->ID()) {
		case QDSCR_INTERFACE_SCENE_REDRAW:
			set_scene_redraw(xml::tag_buffer(*it).get_int());
			break;
		case QDSCR_INTERFACE_SCREEN: {
			qdInterfaceScreen *scr = new qdInterfaceScreen;
			scr->set_owner(this);
			scr->load_script(&*it);
			add_screen(scr);
		}
		break;
		case QDSCR_INTERFACE_MAIN_MENU:
			set_main_menu_screen(it->data());
			break;
		case QDSCR_INTERFACE_INGAME_SCREEN0:
			_ingame_screen_names[0] = it->data();
			break;
		case QDSCR_INTERFACE_INGAME_SCREEN1:
			_ingame_screen_names[1] = it->data();
			break;
		case QDSCR_INTERFACE_SAVE_PROMPT_SCREEN:
			_save_prompt_screen_name = it->data();
			break;
		case QDSCR_INTERFACE_SAVE_NAME_SCREEN:
			_save_title_screen_name = it->data();
			break;
		case QDSCR_INTERFACE_NEED_SAVE_SCREENSHOT:
			_need_save_screenshot = (0 != xml::tag_buffer(*it).get_int());
			break;
		case QDSCR_INTERFACE_NEED_SHOW_SAVE_TIME:
			_need_show_save_time = (0 != xml::tag_buffer(*it).get_int());
			break;
		case QDSCR_INTERFACE_NEED_SHOW_SAVE_NAME:
			_need_show_save_title = (0 != xml::tag_buffer(*it).get_int());
			break;
		case QDSCR_INTERFACE_SAVE_FONT_TYPE:
			_save_font_type = xml::tag_buffer(*it).get_int();
			break;
		case QDSCR_INTERFACE_SAVE_FONT_COLOR:
			_save_font_color = xml::tag_buffer(*it).get_int();
			break;
		}
	}

	return true;
}

void qdInterfaceDispatcher::set_main_menu_screen(const char *name) {
	if (name)
		_main_menu_screen_name = name;
	else
		_main_menu_screen_name.clear();
}

void qdInterfaceDispatcher::set_ingame_screen(const char *name, bool inventory_state) {
	if (name)
		_ingame_screen_names[inventory_state] = name;
	else
		_ingame_screen_names[inventory_state].clear();
}

bool qdInterfaceDispatcher::handle_event(int event_code, const char *event_data, qdInterfaceObjectBase *sender) {
	switch (event_code) {
	case qdInterfaceEvent::EVENT_EXIT:
		if (qdGameDispatcher * p = qd_get_game_dispatcher()) {
			p->toggle_exit();
			return true;
		}
		break;
	case qdInterfaceEvent::EVENT_CHANGE_INTERFACE_SCREEN:
		if (event_data) {
			if (has_main_menu() && !strcmp(main_menu_screen_name(), event_data)) {
				if (qdGameDispatcher * p = qdGameDispatcher::get_dispatcher()) {
					p->set_flag(qdGameDispatcher::MAIN_MENU_FLAG);
					return true;
				}
			} else {
				set_next_screen(event_data);
				return true;
			}
		}
		return false;
	case qdInterfaceEvent::EVENT_TMP_HIDE_ELEMENT:
		if (_cur_screen)
			return _cur_screen->hide_element(event_data, true);
		break;
	case qdInterfaceEvent::EVENT_HIDE_ELEMENT:
		if (_cur_screen)
			return _cur_screen->hide_element(event_data);
		break;
	case qdInterfaceEvent::EVENT_SHOW_ELEMENT:
		if (_cur_screen)
			return _cur_screen->show_element(event_data);
		break;
	case qdInterfaceEvent::EVENT_RESUME_GAME:
		if (qdGameDispatcher * p = qd_get_game_dispatcher()) {
			if (p->get_active_scene())
				return p->toggle_main_menu(false);
		}
		break;
	case qdInterfaceEvent::EVENT_SET_SAVE_MODE:
		qdInterfaceSave::set_save_mode(true);
		return true;
	case qdInterfaceEvent::EVENT_SET_LOAD_MODE:
		qdInterfaceSave::set_save_mode(false);
		return true;
	case qdInterfaceEvent::EVENT_NEW_GAME:
		if (qdGameDispatcher * p = qd_get_game_dispatcher()) {
			p->toggle_main_menu(false);
			return p->restart();
		}
		break;
	case qdInterfaceEvent::EVENT_ACTIVATE_PERSONAGE:
		if (!event_data) return false;
		if (qdGameDispatcher * p = qd_get_game_dispatcher()) {
			if (qdGameScene * sp = p->get_active_scene()) {
				qdGameObjectMoving *obj = dynamic_cast<qdGameObjectMoving *>(sp->get_object(event_data));
				if (!obj || obj == sp->get_active_personage()) return false;
				sp->set_active_personage(obj);
				return true;
			}
		}
		break;
	case qdInterfaceEvent::EVENT_CHANGE_PERSONAGE:
		if (qdGameDispatcher * p = qd_get_game_dispatcher()) {
			if (qdGameScene * sp = p->get_active_scene()) {
				sp->change_active_personage();
				return true;
			}
		}
		break;
	case qdInterfaceEvent::EVENT_PREV_ELEMENT_STATE:
		if (_cur_screen && event_data) {
			if (qdInterfaceButton * p = dynamic_cast<qdInterfaceButton * >(_cur_screen->get_element(event_data)))
				return p->change_state(false);
		}
		break;
	case qdInterfaceEvent::EVENT_NEXT_ELEMENT_STATE:
		if (_cur_screen && event_data) {
			if (qdInterfaceButton * p = dynamic_cast<qdInterfaceButton * >(_cur_screen->get_element(event_data)))
				return p->change_state(true);
		}
		break;
	case qdInterfaceEvent::EVENT_MAIN_MENU:
		if (qdGameDispatcher * p = qdGameDispatcher::get_dispatcher()) {
			p->set_flag(qdGameDispatcher::MAIN_MENU_FLAG);
			return true;
		}
		break;
	case qdInterfaceEvent::EVENT_PLAY_VIDEO:
		if (event_data) {
			if (qdGameDispatcher * p = qdGameDispatcher::get_dispatcher()) {
				if (p->play_video(event_data)) {
					p->pause();
					return true;
				}
			}
		}
		break;
	case qdInterfaceEvent::EVENT_BUTTON_STATE:
		if (_cur_screen && event_data) {
			Common::String str = event_data;
			size_t pos = str.find("::");
			if (pos != Common::String::npos) {
				if (qdInterfaceButton * p = dynamic_cast<qdInterfaceButton * >(_cur_screen->get_element(str.substr(0, pos).c_str()))) {
					return p->activate_state(str.substr(pos + 2).c_str());
				}
			}
		}
		break;
	case qdInterfaceEvent::EVENT_CLEAR_MOUSE:
		if (qdGameDispatcher * p = qdGameDispatcher::get_dispatcher()) {
			return p->drop_mouse_object();
		}
		break;
	case qdInterfaceEvent::EVENT_LOAD_SCENE:
		if (event_data) {
			if (qdGameDispatcher * p = qdGameDispatcher::get_dispatcher()) {
				if (qdGameScene * sp = p->get_scene(event_data)) {
					p->set_next_scene(sp);
					p->activate_trigger_links(sp);
					return true;
				}
			}
		}
		break;
	case qdInterfaceEvent::EVENT_SCROLL_LEFT:
	case qdInterfaceEvent::EVENT_SCROLL_RIGHT:
	case qdInterfaceEvent::EVENT_SCROLL_UP:
	case qdInterfaceEvent::EVENT_SCROLL_DOWN:
		if (qdGameDispatcher * p = qdGameDispatcher::get_dispatcher()) {
			if (!p->get_active_personage())
				break;
			if (qdInventory * sp = p->get_inventory(p->get_active_personage()->inventory_name())) {
				if (qdInterfaceEvent::EVENT_SCROLL_LEFT == event_code)
					sp->scroll_left();
				else if (qdInterfaceEvent::EVENT_SCROLL_RIGHT == event_code)
					sp->scroll_right();
				else if (qdInterfaceEvent::EVENT_SCROLL_UP == event_code)
					sp->scroll_up();
				else sp->scroll_down();
				return true;
			}
		}
		break;
	case qdInterfaceEvent::EVENT_SHOW_INTERFACE_SCREEN_AS_MODAL:
		if (event_data) {
			if (qdInterfaceScreen * screen = get_screen(event_data))
				screen->set_modal_caller(sender);
			select_background_screen(_cur_screen);
			select_screen(event_data);
			if (_modalScreenMode == MODAL_SCREEN_SAVE_NAME_EDIT) {
				if (qdInterfaceSave * save = dynamic_cast<qdInterfaceSave * >(sender))
					set_save_title(save->title());
			}
			return true;
		}
		break;
	case qdInterfaceEvent::EVENT_MODAL_OK:
	case qdInterfaceEvent::EVENT_MODAL_CANCEL:
		if (sender) {
			select_background_screen(0);
			// Определяем экран, от которого пришло событие
			qdInterfaceScreen *screen_ptr = NULL;
			if (sender->owner())
				screen_ptr = dynamic_cast<qdInterfaceScreen * >(sender->owner());
			if (!screen_ptr)
				screen_ptr = dynamic_cast<qdInterfaceScreen * >(sender);
			if (!screen_ptr) {
				assert(0 && "Не найден экран, от которого пришло событие модального типа");
				return true; // Сообщение все же обработано
			}
			// Обрабатываем положительную реакцию пользователя на запрос
			qdInterfaceObjectBase *modal_caller_ptr = screen_ptr->modal_caller();
			if (!modal_caller_ptr) return false;

			if (_modalScreenMode == MODAL_SCREEN_SAVE_OVERWRITE) {
				if (qdInterfaceEvent::EVENT_MODAL_OK == event_code) {
					if (qdInterfaceSave * save = dynamic_cast<qdInterfaceSave * >(modal_caller_ptr)) {
						save->set_title(_save_title.c_str());
						save->perform_save();
						handle_event(qdInterfaceEvent::EVENT_RESUME_GAME, NULL);
						return true;
					}
				}
			} else if (_modalScreenMode == MODAL_SCREEN_SAVE_NAME_EDIT) {
				if (qdInterfaceEvent::EVENT_MODAL_OK == event_code) {
					_save_title = get_save_title();
					if (qdInterfaceSave *save = dynamic_cast<qdInterfaceSave * >(modal_caller_ptr)) {
						if (has_save_prompt_screen() && g_engine->getSaveFileManager()->exists(g_engine->getSaveStateName(save->save_ID()))) {
							setModalScreenMode(qdInterfaceDispatcher::MODAL_SCREEN_SAVE_OVERWRITE);
							screen_ptr = dynamic_cast<qdInterfaceScreen *>(modal_caller_ptr->owner());
							if (!screen_ptr)
								screen_ptr = dynamic_cast<qdInterfaceScreen * >(modal_caller_ptr);
							if (screen_ptr)
								select_screen(screen_ptr->name());

							handle_event(qdInterfaceEvent::EVENT_SHOW_INTERFACE_SCREEN_AS_MODAL, save_prompt_screen_name(), save);
							return true;
						} else {
							save->set_title(_save_title.c_str());
							save->perform_save();
							handle_event(qdInterfaceEvent::EVENT_RESUME_GAME, NULL);
							return true;
						}
					}
				}
			}

			screen_ptr = dynamic_cast<qdInterfaceScreen *>(modal_caller_ptr->owner());
			if (!screen_ptr)
				screen_ptr = dynamic_cast<qdInterfaceScreen * >(modal_caller_ptr);
			if (!screen_ptr) {
				assert(0 && "Не найден экран, от которого пришло событие модального типа");
				return true;
			}
			select_screen(screen_ptr->name());
			return true;
		}
		break;
	}
	return false;
}

int qdInterfaceDispatcher::option_value(int option_id, const char *option_data) {
	switch (option_id) {
	case qdInterfaceElement::OPTION_SOUND:
		return ConfMan.getBool("enable_sound");
	case qdInterfaceElement::OPTION_SOUND_VOLUME:
		return ConfMan.getInt("sound_volume");
	case qdInterfaceElement::OPTION_MUSIC:
		return ConfMan.getBool("enable_music");
	case qdInterfaceElement::OPTION_MUSIC_VOLUME:
		return ConfMan.getInt("music_volume");
	case qdInterfaceElement::OPTION_ACTIVE_PERSONAGE:
		if (option_data) {
			if (qdGameObjectMoving * p = qdGameDispatcher::get_dispatcher()->get_active_personage()) {
				if (!strcmp(p->name(), option_data)) return 1;
			}
		}
		return 0;
	}
	return -1;
}

bool qdInterfaceDispatcher::set_option_value(int option_id, int value, const char *option_data) {
	switch (option_id) {
	case qdInterfaceElement::OPTION_SOUND:
		ConfMan.setBool("enable_sound", value > 0);
		g_engine->syncSoundSettings();
		return true;
	case qdInterfaceElement::OPTION_SOUND_VOLUME:
		ConfMan.setInt("sound_volume", value);
		g_engine->syncSoundSettings();
		return true;
	case qdInterfaceElement::OPTION_MUSIC:
		ConfMan.setBool("enable_music", value > 0);
		g_engine->syncSoundSettings();
		return true;
	case qdInterfaceElement::OPTION_MUSIC_VOLUME:
		ConfMan.setInt("music_volume", value);
		g_engine->syncSoundSettings();
		return true;
	case qdInterfaceElement::OPTION_ACTIVE_PERSONAGE:
		if (option_data) {
			if (qdGameScene * sp = qdGameDispatcher::get_dispatcher()->get_active_scene()) {
				qdGameObjectMoving *obj = dynamic_cast<qdGameObjectMoving *>(sp->get_object(option_data));
				if (!obj || obj == sp->get_active_personage()) return false;
				sp->set_active_personage(obj);
				return true;
			}
		}
		return false;
	}
	return false;
}

void qdInterfaceDispatcher::update_personage_buttons() {
	if (_cur_screen)
		_cur_screen->update_personage_buttons();
}

#ifdef __QD_DEBUG_ENABLE__
bool qdInterfaceDispatcher::get_resources_info(qdResourceInfoContainer &infos) const {
	for (resource_container_t::resource_list_t::const_iterator it = _resources.resource_list().begin(); it != _resources.resource_list().end(); ++it) {
		if ((*it)->is_resource_loaded())
			infos.push_back(qdResourceInfo(*it));
	}

	return true;
}
#endif


bool qdInterfaceDispatcher::set_save_title(const char *title) {
	if (!_cur_screen)
		return false;

	const qdInterfaceScreen::element_list_t &lst = _cur_screen->element_list();
	for (qdInterfaceScreen::element_list_t::const_iterator it = lst.begin(); it != lst.end(); ++it) {
		if ((*it)->get_element_type() == qdInterfaceElement::EL_TEXT_WINDOW) {
			qdInterfaceTextWindow *wnd = static_cast<qdInterfaceTextWindow *>(*it);
			if (wnd->windowType() == qdInterfaceTextWindow::WINDOW_EDIT) {
				wnd->set_input_string(title);
				wnd->edit_start();
				return true;
			}
		}
	}

	return false;
}

const char *qdInterfaceDispatcher::get_save_title() const {
	if (!_cur_screen)
		return nullptr;

	const qdInterfaceScreen::element_list_t &lst = _cur_screen->element_list();
	for (qdInterfaceScreen::element_list_t::const_iterator it = lst.begin(); it != lst.end(); ++it) {
		if ((*it)->get_element_type() == qdInterfaceElement::EL_TEXT_WINDOW) {
			qdInterfaceTextWindow *wnd = static_cast<qdInterfaceTextWindow *>(*it);
			if (wnd->windowType() == qdInterfaceTextWindow::WINDOW_EDIT)
				return wnd->input_string();
		}
	}

	return nullptr;
}

} // namespace QDEngine
