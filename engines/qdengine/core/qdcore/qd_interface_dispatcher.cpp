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

/* ---------------------------- INCLUDE SECTION ----------------------------- */
#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include "qdengine/qdengine.h"
#include "qdengine/core/qd_precomp.h"
#include "qdengine/core/parser/xml_tag_buffer.h"
#include "qdengine/core/parser/qdscr_parser.h"

#include "qdengine/core/qdcore/qd_interface_dispatcher.h"
#include "qdengine/core/qdcore/qd_interface_screen.h"
#include "qdengine/core/qdcore/qd_interface_element_state.h"
#include "qdengine/core/qdcore/qd_interface_save.h"
#include "qdengine/core/qdcore/qd_interface_button.h"
#include "qdengine/core/qdcore/qd_game_dispatcher.h"
#include "qdengine/core/qdcore/qd_game_object_moving.h"
#include "qdengine/core/qdcore/qd_game_scene.h"
#include "qdengine/core/qdcore/qd_setup.h"
#include "qdengine/core/qdcore/qd_interface_text_window.h"
#include "qdengine/core/qdcore/qd_resource.h"


namespace QDEngine {



/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

qdInterfaceDispatcher *qdInterfaceDispatcher::dispatcher_ = NULL;

qdInterfaceDispatcher::qdInterfaceDispatcher() : cur_screen_(NULL),
#ifndef _QUEST_EDITOR
	next_screen_(NULL),
#endif
	is_active_(false),
	is_mouse_hover_(false),
	autohide_disable_(false),
	need_full_redraw_(false),
	need_scene_redraw_(false),
	end_game_mode_(false),
	need_save_screenshot_(true),
	need_show_save_time_(false),
	need_show_save_title_(false),
	save_font_type_(QD_FONT_TYPE_NONE),
	save_font_color_(0x00FFFFFF),
	background_screen_(NULL),
	background_screen_lock_(false),
	modalScreenMode_(MODAL_SCREEN_OTHER) {
}

qdInterfaceDispatcher::~qdInterfaceDispatcher() {
	screens_.clear();
}

bool qdInterfaceDispatcher::add_screen(qdInterfaceScreen *scr) {
	return screens_.add_object(scr);
}

bool qdInterfaceDispatcher::rename_screen(qdInterfaceScreen *scr, const char *name) {
	return screens_.rename_object(scr, name);
}

bool qdInterfaceDispatcher::remove_screen(qdInterfaceScreen *scr) {
	return screens_.remove_object(scr);
}

qdInterfaceScreen *qdInterfaceDispatcher::get_screen(const char *screen_name) {
	if (!screen_name) return NULL;

	return screens_.get_object(screen_name);
}

bool qdInterfaceDispatcher::is_screen_in_list(const qdInterfaceScreen *scr) {
	return screens_.is_in_list(scr);
}

bool qdInterfaceDispatcher::select_screen(const char *screen_name, bool lock_resources) {
	qdInterfaceScreen *p = get_screen(screen_name);

	if (p) {
		if (cur_screen_ && cur_screen_ -> is_locked()) {
			debugC(3, kDebugQuant, "qdInterfaceDispatcher::select_screen() Selecting screen: %s", transCyrillic(screen_name));
			for (resource_container_t::resource_list_t::const_iterator it = resources_.resource_list().begin(); it != resources_.resource_list().end(); ++it) {
				if (p -> has_references(*it)) {
					if (!(*it) -> is_resource_loaded())
						debugC(3, kDebugQuant, "qdInterfaceDispatcher::select_screen() Resource is used in both screens %s and %s", transCyrillic(cur_screen_ -> name()), transCyrillic(p -> name()));
						(*it) -> load_resource();
				} else {
					if ((*it) -> is_resource_loaded() && !cur_screen_ -> has_references(*it))
						(*it) -> free_resource();
				}
			}
		} else {
			for (resource_container_t::resource_list_t::const_iterator it = resources_.resource_list().begin(); it != resources_.resource_list().end(); ++it) {
				if (p -> has_references(*it)) {
					if (!(*it) -> is_resource_loaded())
						(*it) -> load_resource();
				} else {
					if ((*it) -> is_resource_loaded())
						(*it) -> free_resource();
				}
			}
		}
	} else {
		if (cur_screen_ && cur_screen_ -> is_locked()) {
			for (resource_container_t::resource_list_t::const_iterator it = resources_.resource_list().begin(); it != resources_.resource_list().end(); ++it) {
				if ((*it) -> is_resource_loaded() && !cur_screen_ -> has_references(*it))
					(*it) -> free_resource();
			}
		} else {
			for (resource_container_t::resource_list_t::const_iterator it = resources_.resource_list().begin(); it != resources_.resource_list().end(); ++it) {
				if ((*it) -> is_resource_loaded())
					(*it) -> free_resource();
			}
		}
	}

	if (p && cur_screen_ != p)
		p -> set_autohide_phase(1.0f);

	cur_screen_ = p;

	if (cur_screen_) {
		debugC(3, kDebugQuant, "qdInterfaceDispatcher::select_screen() if(cur_scene_): %s", transCyrillic(screen_name));
		bool is_game_active = false;
		if (qdGameDispatcher * dp = qdGameDispatcher::get_dispatcher()) {
			if (dp -> get_active_scene())
				is_game_active = true;

			if (cur_screen_ -> has_music_track())
				dp -> play_music_track(&cur_screen_ -> music_track(), true);
		}

		cur_screen_ -> init(is_game_active);
		if (lock_resources)
			cur_screen_ -> lock_resources();
	}

	need_full_redraw_ = true;

	return true;
}

bool qdInterfaceDispatcher::select_background_screen(qdInterfaceScreen *p) {
	if (background_screen_ && background_screen_ != p) {
		if (!background_screen_lock_)
			background_screen_->unlock_resources();
		background_screen_ = 0;
		need_full_redraw_ = true;
	}

	if (p) {
		background_screen_ = p;
		background_screen_lock_ = p->is_locked();
		p->lock_resources();
		need_full_redraw_ = true;
	}

	return true;
}

bool qdInterfaceDispatcher::select_ingame_screen(bool inventory_state) {
	if (has_ingame_screen(inventory_state))
		debugC(3, kDebugQuant, "qdInterfaceDispatcher::select_ingame_screen(): Selecting ingame screen: %s", transCyrillic(ingame_screen_name(inventory_state)));
		return select_screen(ingame_screen_name(inventory_state), true);

	return select_screen(NULL);
}

qdResource *qdInterfaceDispatcher::add_resource(const char *file_name, const qdInterfaceElementState *owner) {
	return resources_.add_resource(file_name, owner);
}

bool qdInterfaceDispatcher::remove_resource(const char *file_name, const qdInterfaceElementState *owner) {
	return resources_.remove_resource(file_name, owner);
}

bool qdInterfaceDispatcher::redraw(int dx, int dy) const {
	if (background_screen_)
		background_screen_ -> redraw(dx, dy);

	if (cur_screen_)
		return cur_screen_ -> redraw(dx, dy);

	return false;
}

bool qdInterfaceDispatcher::pre_redraw() {
	if (cur_screen_)
		cur_screen_ -> pre_redraw(need_full_redraw_);

	if (background_screen_)
		background_screen_ -> pre_redraw(need_full_redraw_);

	return false;
}

bool qdInterfaceDispatcher::post_redraw() {
	need_full_redraw_ = false;

	if (cur_screen_)
		return cur_screen_ -> post_redraw();

	if (background_screen_)
		background_screen_ -> post_redraw();

	return false;
}

bool qdInterfaceDispatcher::quant(float dt) {
	if (next_screen_) {
		select_screen(next_screen_);
		set_next_screen(NULL);
	}

	if (cur_screen_)
		return cur_screen_ -> quant(dt);

	return false;
}

bool qdInterfaceDispatcher::mouse_handler(int x, int y, mouseDispatcher::mouseEvent ev) {
	is_mouse_hover_ = false;
	autohide_disable_ = false;

	if (end_game_mode_) {
		if (ev == mouseDispatcher::EV_LEFT_DOWN || ev == mouseDispatcher::EV_RIGHT_DOWN) {
			handle_event(qdInterfaceEvent::EVENT_CHANGE_INTERFACE_SCREEN, main_menu_screen_name());
			end_game_mode_ = false;
			return true;
		}
	}

	if (cur_screen_)
		return cur_screen_ -> mouse_handler(x, y, ev);

	return false;
}

bool qdInterfaceDispatcher::keyboard_handler(int vkey) {
	if (cur_screen_) {
		if (vkey == VK_ESCAPE && has_main_menu()) {
			if (cur_screen_ -> name() && !strcmp(cur_screen_ -> name(), main_menu_screen_name()))
				handle_event(qdInterfaceEvent::EVENT_RESUME_GAME, NULL);
			else {
				if (qdGameDispatcher::get_dispatcher()->is_main_menu_exit_enabled())
					select_screen(main_menu_screen_name());
			}

			return true;
		}

		return cur_screen_ -> keyboard_handler(vkey);
	}

	return false;
}

bool qdInterfaceDispatcher::char_input_handler(int vkey) {
	if (cur_screen_)
		return cur_screen_->char_input_handler(vkey);

	return false;
}

bool qdInterfaceDispatcher::save_script(Common::SeekableWriteStream &fh, int indent) const {
	for (int i = 0; i <= indent; i ++) {
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

	if (!save_prompt_screen_name_.empty()) {
		fh.writeString(Common::String::format(" save_prompt_screen=\"%s\"", qdscr_XML_string(save_prompt_screen_name_.c_str())));
	}

	if (!save_title_screen_name_.empty()) {
		fh.writeString(Common::String::format(" save_title_screen=\"%s\"", qdscr_XML_string(save_title_screen_name_.c_str())));
	}

	// Значение по умолчанию - true, значит сохраняем только false
	if (!need_save_screenshot_) {
		fh.writeString(" need_save_screenshot=\"0\"");
	}

	if (need_show_save_time_) {
		fh.writeString(" need_show_save_time=\"1\"");
	}

	if (need_show_save_title_) {
		fh.writeString(" need_show_save_name=\"1\"");
	}

	if (QD_FONT_TYPE_NONE != save_font_type_) {
		fh.writeString(Common::String::format(" save_font_type=\"%d\"", save_font_type_));
	}

	if (0x00FFFFFF != save_font_color_) {
		fh.writeString(Common::String::format(" save_font_color=\"%d\"", save_font_color_));
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

bool qdInterfaceDispatcher::save_script(XStream &fh, int indent) const {
	warning("qdInterfaceDispatcher::save_script(XStream)");
	return true;
}

bool qdInterfaceDispatcher::load_script(const xml::tag *p) {
	bool screen_idx = false;
	for (xml::tag::subtag_iterator it = p -> subtags_begin(); it != p -> subtags_end(); ++it) {
		switch (it -> ID()) {
		case QDSCR_INTERFACE_SCENE_REDRAW:
			set_scene_redraw(xml::tag_buffer(*it).get_int());
			break;
		case QDSCR_INTERFACE_SCREEN: {
			qdInterfaceScreen *scr = new qdInterfaceScreen;
			scr -> set_owner(this);
			scr -> load_script(&*it);
			add_screen(scr);
		}
		break;
		case QDSCR_INTERFACE_MAIN_MENU:
			set_main_menu_screen(it -> data());
			break;
		case QDSCR_INTERFACE_INGAME_SCREEN0:
			ingame_screen_names_[0] = it -> data();
			break;
		case QDSCR_INTERFACE_INGAME_SCREEN1:
			ingame_screen_names_[1] = it -> data();
			break;
		case QDSCR_INTERFACE_SAVE_PROMPT_SCREEN:
			save_prompt_screen_name_ = it -> data();
			break;
		case QDSCR_INTERFACE_SAVE_NAME_SCREEN:
			save_title_screen_name_ = it -> data();
			break;
		case QDSCR_INTERFACE_NEED_SAVE_SCREENSHOT:
			need_save_screenshot_ = (0 != xml::tag_buffer(*it).get_int());
			break;
		case QDSCR_INTERFACE_NEED_SHOW_SAVE_TIME:
			need_show_save_time_ = (0 != xml::tag_buffer(*it).get_int());
			break;
		case QDSCR_INTERFACE_NEED_SHOW_SAVE_NAME:
			need_show_save_title_ = (0 != xml::tag_buffer(*it).get_int());
			break;
		case QDSCR_INTERFACE_SAVE_FONT_TYPE:
			save_font_type_ = xml::tag_buffer(*it).get_int();
			break;
		case QDSCR_INTERFACE_SAVE_FONT_COLOR:
			save_font_color_ = xml::tag_buffer(*it).get_int();
			break;
		}
	}

	return true;
}

void qdInterfaceDispatcher::set_main_menu_screen(const char *name) {
	if (name)
		main_menu_screen_name_ = name;
	else
		main_menu_screen_name_.clear();
}

void qdInterfaceDispatcher::set_ingame_screen(const char *name, bool inventory_state) {
	if (name)
		ingame_screen_names_[inventory_state] = name;
	else
		ingame_screen_names_[inventory_state].clear();
}

bool qdInterfaceDispatcher::handle_event(int event_code, const char *event_data, qdInterfaceObjectBase *sender) {
#ifndef _QUEST_EDITOR
	switch (event_code) {
	case qdInterfaceEvent::EVENT_EXIT:
		if (qdGameDispatcher * p = qd_get_game_dispatcher()) {
			p -> toggle_exit();
			return true;
		}
		break;
	case qdInterfaceEvent::EVENT_CHANGE_INTERFACE_SCREEN:
		if (event_data) {
			if (has_main_menu() && !strcmp(main_menu_screen_name(), event_data)) {
				if (qdGameDispatcher * p = qdGameDispatcher::get_dispatcher()) {
					p -> set_flag(qdGameDispatcher::MAIN_MENU_FLAG);
					return true;
				}
			} else {
				set_next_screen(event_data);
				return true;
			}
		}
		return false;
	case qdInterfaceEvent::EVENT_TMP_HIDE_ELEMENT:
		if (cur_screen_)
			return cur_screen_ -> hide_element(event_data, true);
		break;
	case qdInterfaceEvent::EVENT_HIDE_ELEMENT:
		if (cur_screen_)
			return cur_screen_ -> hide_element(event_data);
		break;
	case qdInterfaceEvent::EVENT_SHOW_ELEMENT:
		if (cur_screen_)
			return cur_screen_ -> show_element(event_data);
		break;
	case qdInterfaceEvent::EVENT_RESUME_GAME:
		if (qdGameDispatcher * p = qd_get_game_dispatcher()) {
			if (p -> get_active_scene())
				return p -> toggle_main_menu(false);
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
			p -> toggle_main_menu(false);
			return p -> restart();
		}
		break;
	case qdInterfaceEvent::EVENT_ACTIVATE_PERSONAGE:
		if (!event_data) return false;
		if (qdGameDispatcher * p = qd_get_game_dispatcher()) {
			if (qdGameScene * sp = p -> get_active_scene()) {
				qdGameObjectMoving *obj = dynamic_cast<qdGameObjectMoving *>(sp -> get_object(event_data));
				if (!obj || obj == sp -> get_active_personage()) return false;
				sp -> set_active_personage(obj);
				return true;
			}
		}
		break;
	case qdInterfaceEvent::EVENT_CHANGE_PERSONAGE:
		if (qdGameDispatcher * p = qd_get_game_dispatcher()) {
			if (qdGameScene * sp = p -> get_active_scene()) {
				sp -> change_active_personage();
				return true;
			}
		}
		break;
	case qdInterfaceEvent::EVENT_PREV_ELEMENT_STATE:
		if (cur_screen_ && event_data) {
			if (qdInterfaceButton * p = dynamic_cast<qdInterfaceButton * >(cur_screen_ -> get_element(event_data)))
				return p -> change_state(false);
		}
		break;
	case qdInterfaceEvent::EVENT_NEXT_ELEMENT_STATE:
		if (cur_screen_ && event_data) {
			if (qdInterfaceButton * p = dynamic_cast<qdInterfaceButton * >(cur_screen_ -> get_element(event_data)))
				return p -> change_state(true);
		}
		break;
	case qdInterfaceEvent::EVENT_MAIN_MENU:
		if (qdGameDispatcher * p = qdGameDispatcher::get_dispatcher()) {
			p -> set_flag(qdGameDispatcher::MAIN_MENU_FLAG);
			return true;
		}
		break;
	case qdInterfaceEvent::EVENT_PLAY_VIDEO:
		if (event_data) {
			if (qdGameDispatcher * p = qdGameDispatcher::get_dispatcher()) {
				if (p -> play_video(event_data)) {
					p -> pause();
					return true;
				}
			}
		}
		break;
	case qdInterfaceEvent::EVENT_BUTTON_STATE:
		if (cur_screen_ && event_data) {
			std::string str = event_data;
			int pos = str.find("::");
			if (pos != std::string::npos) {
				if (qdInterfaceButton * p = dynamic_cast<qdInterfaceButton * >(cur_screen_ -> get_element(str.substr(0, pos).c_str()))) {
					return p->activate_state(str.substr(pos + 2).c_str());
				}
			}
		}
		break;
	case qdInterfaceEvent::EVENT_CLEAR_MOUSE:
		if (qdGameDispatcher * p = qdGameDispatcher::get_dispatcher()) {
			return p -> drop_mouse_object();
		}
		break;
	case qdInterfaceEvent::EVENT_LOAD_SCENE:
		if (event_data) {
			if (qdGameDispatcher * p = qdGameDispatcher::get_dispatcher()) {
				if (qdGameScene * sp = p -> get_scene(event_data)) {
					p -> set_next_scene(sp);
					p -> activate_trigger_links(sp);
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
			if (qdInventory * sp = p -> get_inventory(p->get_active_personage()->inventory_name())) {
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
			select_background_screen(cur_screen_);
			select_screen(event_data);
			if (modalScreenMode_ == MODAL_SCREEN_SAVE_NAME_EDIT) {
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

			if (modalScreenMode_ == MODAL_SCREEN_SAVE_OVERWRITE) {
				if (qdInterfaceEvent::EVENT_MODAL_OK == event_code) {
					if (qdInterfaceSave * save = dynamic_cast<qdInterfaceSave * >(modal_caller_ptr)) {
						save->set_title(save_title_.c_str());
						save->perform_save();
						handle_event(qdInterfaceEvent::EVENT_RESUME_GAME, NULL);
						return true;
					}
				}
			} else if (modalScreenMode_ == MODAL_SCREEN_SAVE_NAME_EDIT) {
				if (qdInterfaceEvent::EVENT_MODAL_OK == event_code) {
					save_title_ = get_save_title();
					if (qdInterfaceSave * save = dynamic_cast<qdInterfaceSave * >(modal_caller_ptr)) {
						if (has_save_prompt_screen() && app_io::is_file_exist(save->save_file())) {
							setModalScreenMode(qdInterfaceDispatcher::MODAL_SCREEN_SAVE_OVERWRITE);
							screen_ptr = dynamic_cast<qdInterfaceScreen *>(modal_caller_ptr->owner());
							if (!screen_ptr)
								screen_ptr = dynamic_cast<qdInterfaceScreen * >(modal_caller_ptr);
							if (screen_ptr)
								select_screen(screen_ptr->name());

							handle_event(qdInterfaceEvent::EVENT_SHOW_INTERFACE_SCREEN_AS_MODAL, save_prompt_screen_name(), save);
							return true;
						} else {
							save->set_title(save_title_.c_str());
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
#endif // _QUEST_EDITOR
	return false;
}

bool qdInterfaceDispatcher::adjust_pack_files(const char *pack_dir, bool can_overwrite) {
	std::list<qdInterfaceElementState *> interf_list;
	resources_.get_owners(interf_list);
	qdFileNameList res_file_list;
	resources_.get_file_list(res_file_list);

	const char *str;
	bool all_ok = true;
	// Правим все найденные файлы ресурсов в qdInterfaceElementState.
	// Заметим, что пути в resources_ правятся автоматически
	for (qdFileNameList::const_iterator it1 = res_file_list.begin();
	        res_file_list.end() != it1;
	        ++it1) {
		for (std::list<qdInterfaceElementState * >::const_iterator it2 = interf_list.begin();
		        interf_list.end() != it2;
		        ++it2)
			if (false == (*it2)->adjust_res_file((*it1).c_str(), pack_dir, str, can_overwrite))
				all_ok = false;
	}

	return all_ok;
};

bool qdInterfaceDispatcher::get_file_list(qdFileNameList &files_to_copy, qdFileNameList &files_to_pack) const {
	for (screen_list_t::const_iterator it = screen_list().begin(); it != screen_list().end(); ++it) {
		if ((*it)->has_music_track())
			files_to_copy.push_back((*it)->music_track().file_name());
	}

	resources_.get_file_list(files_to_pack);

	return true;
}

int qdInterfaceDispatcher::option_value(int option_id, const char *option_data) {
#ifndef _QUEST_EDITOR
	switch (option_id) {
	case qdInterfaceElement::OPTION_SOUND:
		return (int)qdGameConfig::get_config().is_sound_enabled();
	case qdInterfaceElement::OPTION_SOUND_VOLUME:
		return qdGameConfig::get_config().sound_volume();
	case qdInterfaceElement::OPTION_MUSIC:
		return (int)qdGameConfig::get_config().is_music_enabled();
	case qdInterfaceElement::OPTION_MUSIC_VOLUME:
		return qdGameConfig::get_config().music_volume();
	case qdInterfaceElement::OPTION_ACTIVE_PERSONAGE:
		if (option_data) {
			if (qdGameObjectMoving * p = qdGameDispatcher::get_dispatcher() -> get_active_personage()) {
				if (!strcmp(p -> name(), option_data)) return 1;
			}
		}
		return 0;
	}
#endif // _QUEST_EDITOR
	return -1;
}

bool qdInterfaceDispatcher::set_option_value(int option_id, int value, const char *option_data) {
#ifndef _QUEST_EDITOR
	switch (option_id) {
	case qdInterfaceElement::OPTION_SOUND:
		qdGameConfig::get_config().toggle_sound(value > 0);
		qdGameConfig::get_config().update_sound_settings();
		qdGameConfig::get_config().save();
		return true;
	case qdInterfaceElement::OPTION_SOUND_VOLUME:
		qdGameConfig::get_config().set_sound_volume(value);
		qdGameConfig::get_config().update_sound_settings();
		qdGameConfig::get_config().save();
		return true;
	case qdInterfaceElement::OPTION_MUSIC:
		qdGameConfig::get_config().toggle_music(value > 0);
		qdGameConfig::get_config().update_music_settings();
		qdGameConfig::get_config().save();
		return true;
	case qdInterfaceElement::OPTION_MUSIC_VOLUME:
		qdGameConfig::get_config().set_music_volume(value);
		qdGameConfig::get_config().update_music_settings();
		qdGameConfig::get_config().save();
		return true;
	case qdInterfaceElement::OPTION_ACTIVE_PERSONAGE:
		if (option_data) {
			if (qdGameScene * sp = qdGameDispatcher::get_dispatcher() -> get_active_scene()) {
				qdGameObjectMoving *obj = dynamic_cast<qdGameObjectMoving *>(sp -> get_object(option_data));
				if (!obj || obj == sp -> get_active_personage()) return false;
				sp -> set_active_personage(obj);
				return true;
			}
		}
		return false;
	}
#endif // _QUEST_EDITOR
	return false;
}

void qdInterfaceDispatcher::update_personage_buttons() {
	if (cur_screen_)
		cur_screen_ -> update_personage_buttons();
}

#ifdef __QD_DEBUG_ENABLE__
bool qdInterfaceDispatcher::get_resources_info(qdResourceInfoContainer &infos) const {
	for (resource_container_t::resource_list_t::const_iterator it = resources_.resource_list().begin(); it != resources_.resource_list().end(); ++it) {
		if ((*it) -> is_resource_loaded())
			infos.push_back(qdResourceInfo(*it));
	}

	return true;
}
#endif


bool qdInterfaceDispatcher::set_save_title(const char *title) {
	if (!cur_screen_)
		return false;

	const qdInterfaceScreen::element_list_t &lst = cur_screen_->element_list();
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
	if (!cur_screen_)
		return nullptr;

	const qdInterfaceScreen::element_list_t &lst = cur_screen_->element_list();
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
