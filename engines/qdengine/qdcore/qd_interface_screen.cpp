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

#include "common/debug.h"

#include "qdengine/qdengine.h"
#include "qdengine/qd_fwd.h"

#include "qdengine/parser/xml_tag_buffer.h"
#include "qdengine/parser/qdscr_parser.h"

#include "qdengine/qdcore/qd_interface_element.h"
#include "qdengine/qdcore/qd_interface_button.h"
#include "qdengine/qdcore/qd_interface_screen.h"
#include "qdengine/qdcore/qd_interface_dispatcher.h"

#include "qdengine/qdcore/qd_game_dispatcher.h"
#include "qdengine/qdcore/qd_game_scene.h"
#include "qdengine/qdcore/qd_game_object_moving.h"


namespace QDEngine {

struct qdInterfaceElementsOrdering {
	bool operator()(const qdInterfaceElement *p0, const qdInterfaceElement *p1) {
		return p0->screen_depth() < p1->screen_depth();
	}
};

qdInterfaceScreen::qdInterfaceScreen() : _is_locked(false),
	_autohide_time(0.0f),
	_autohide_phase(1.0f),
	_autohide_offset(Vect2i(0, 0)),
	_modal_caller(NULL) {
	_sorted_elements.reserve(20);
}

qdInterfaceScreen::~qdInterfaceScreen() {
	_elements.clear();
}

bool qdInterfaceScreen::redraw(int dx, int dy) const {
	for (sorted_element_list_t::const_reverse_iterator it = _sorted_elements.rbegin(); it != _sorted_elements.rend(); ++it)
		(*it)->redraw();

	return true;
}

bool qdInterfaceScreen::pre_redraw(bool force_full_redraw) {
	qdGameDispatcher *dp = qdGameDispatcher::get_dispatcher();
	if (!dp) return false;

	if (force_full_redraw) {
		for (sorted_element_list_t::iterator it = _sorted_elements.begin(); it != _sorted_elements.end(); ++it) {
			if ((*it)->last_screen_region() != (*it)->screen_region())
				dp->add_redraw_region((*it)->last_screen_region());

			dp->add_redraw_region((*it)->screen_region());
		}
	} else {
		for (sorted_element_list_t::iterator it = _sorted_elements.begin(); it != _sorted_elements.end(); ++it) {
			if ((*it)->need_redraw()) {
				if ((*it)->last_screen_region() != (*it)->screen_region())
					dp->add_redraw_region((*it)->last_screen_region());

				dp->add_redraw_region((*it)->screen_region());
			}
		}
	}

	return true;
}

bool qdInterfaceScreen::post_redraw() {
	for (sorted_element_list_t::iterator it = _sorted_elements.begin(); it != _sorted_elements.end(); ++it)
		(*it)->post_redraw();

	return true;
}

bool qdInterfaceScreen::quant(float dt) {
	debugC(9, kDebugQuant, "qdInterfaceScreen::quant(%f)", dt);
	if (_autohide_time > FLT_EPS) {
		float delta = dt / _autohide_time;

		qdInterfaceDispatcher *dp = dynamic_cast<qdInterfaceDispatcher *>(owner());

		if (dp && !dp->is_autohide_enabled())
			_autohide_phase -= delta;
		else
			_autohide_phase += delta;

		if (_autohide_phase < 0.0f)
			_autohide_phase = 0.0f;
		if (_autohide_phase > 1.0f)
			_autohide_phase = 1.0f;

		int x = round(float(_autohide_offset.x) * _autohide_phase);
		int y = round(float(_autohide_offset.y) * _autohide_phase);

		g_engine->set_screen_offset(Vect2i(x, y));
	} else {
		g_engine->set_screen_offset(Vect2i(0, 0));
	}

	for (element_list_t::const_iterator it = element_list().begin(); it != element_list().end(); ++it) {
		if ((*it)->linked_to_option() && qdInterfaceDispatcher::option_value((*it)->option_ID()) != (*it)->option_value())
			qdInterfaceDispatcher::set_option_value((*it)->option_ID(), (*it)->option_value());

		(*it)->quant(dt);
	}

	return true;
}

bool qdInterfaceScreen::save_script(Common::WriteStream &fh, int indent) const {
	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}
	fh.writeString("<interface_screen");

	if (name()) {
		fh.writeString(Common::String::format(" name=\"%s\"", qdscr_XML_string(name())));
	}

	if (_autohide_time > FLT_EPS) {
		fh.writeString(Common::String::format(" hide_time=\"%f\"", _autohide_time));
	}

	if (_autohide_offset.x || _autohide_offset.y) {
		fh.writeString(Common::String::format(" hide_offset=\"%d %d\"", _autohide_offset.x, _autohide_offset.y));
	}
	fh.writeString(">\r\n");

	if (has_music_track()) {
		_music_track.save_script(fh, indent + 1);
	}

	for (auto &it : element_list()) {
		it->save_script(fh, indent + 1);
	}


	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}
	fh.writeString("</interface_screen>\r\n");

	return true;
}

bool qdInterfaceScreen::load_script(const xml::tag *p) {
	for (xml::tag::subtag_iterator it = p->subtags_begin(); it != p->subtags_end(); ++it) {
		switch (it->ID()) {
		case QDSCR_NAME:
			set_name(it->data());
			break;
		case QDSCR_INTERFACE_ELEMENT:
			if (const xml::tag *tg = it->search_subtag(QDSCR_TYPE)) {
				if (qdInterfaceElement *el = qdInterfaceElement::create_element(static_cast<qdInterfaceElement::element_type>(xml::tag_buffer(*tg).get_int()))) {
					el->set_owner(this);
					el->load_script(&*it);
					add_element(el);
				}
			}
			break;
		case QDSCR_INTERFACE_SCREEN_HIDE_TIME:
			xml::tag_buffer(*it) > _autohide_time;
			break;
		case QDSCR_INTERFACE_SCREEN_HIDE_OFFSET:
			xml::tag_buffer(*it) > _autohide_offset.x > _autohide_offset.y;
			break;
		case QDSCR_MUSIC_TRACK:
			_music_track.load_script(&*it);
			break;
		}
	}

	return true;
}

bool qdInterfaceScreen::add_element(qdInterfaceElement *p) {
	if (_elements.add_object(p)) {
		_sorted_elements.push_back(p);
		sort_elements();
		return true;
	}

	return false;
}

bool qdInterfaceScreen::rename_element(qdInterfaceElement *p, const char *name) {
	return _elements.rename_object(p, name);
}

bool qdInterfaceScreen::remove_element(qdInterfaceElement *p) {
	sorted_element_list_t::iterator it = Common::find(_sorted_elements.begin(), _sorted_elements.end(), p);
	if (it != _sorted_elements.end())
		_sorted_elements.erase(it);
	return _elements.remove_object(p);
}

qdInterfaceElement *qdInterfaceScreen::get_element(const char *el_name) {
	return _elements.get_object(el_name);
}

bool qdInterfaceScreen::is_element_in_list(const qdInterfaceElement *el) const {
	return _elements.is_in_list(el);
}

bool qdInterfaceScreen::mouse_handler(int x, int y, mouseDispatcher::mouseEvent ev) {
	debugC(9, kDebugInput, "qdInterfaceScreen::mouse_handler(%d, %d, %u)", x, y, _sorted_elements.size());
	if (qdInterfaceDispatcher *dp = dynamic_cast<qdInterfaceDispatcher* >(owner())) {
		for (auto &it : _sorted_elements) {
			if (it->hit_test(x, y)) {
				dp->toggle_mouse_hover();
				if (it->get_element_type() != qdInterfaceElement::EL_TEXT_WINDOW)
					dp->disable_autohide();

				if (it->mouse_handler(x, y, ev) && !it->is_locked())
					return true;
			} else
				it->hover_clear();
		}
	}

	return false;
}

bool qdInterfaceScreen::keyboard_handler(Common::KeyCode vkey) {
	for (element_list_t::const_iterator it = element_list().begin(); it != element_list().end(); ++it) {
		if (!(*it)->is_locked() && (*it)->keyboard_handler(vkey))
			return true;
	}

	return false;
}

bool qdInterfaceScreen::char_input_handler(int vkey) {
	for (element_list_t::const_iterator it = element_list().begin(); it != element_list().end(); ++it) {
		if (!(*it)->is_locked() && (*it)->char_input_handler(vkey))
			return true;
	}

	return false;
}

qdResource *qdInterfaceScreen::add_resource(const Common::Path file_name, const qdInterfaceElementState *res_owner) {
	if (qdInterfaceDispatcher *dp = dynamic_cast<qdInterfaceDispatcher * >(owner())) {
		if (qdResource *p = dp->add_resource(file_name, res_owner)) {
			_resources.register_resource(p, res_owner);
			if (dp->is_screen_active(this) && !p->is_resource_loaded())
				p->load_resource();

			return p;
		}
	}

	return NULL;
}

bool qdInterfaceScreen::remove_resource(const Common::Path file_name, const qdInterfaceElementState *res_owner) {
	if (qdInterfaceDispatcher *dp = dynamic_cast<qdInterfaceDispatcher * >(owner())) {
		if (qdResource *p = dp->get_resource(file_name)) {
			_resources.unregister_resource(p, res_owner);
			return dp->remove_resource(file_name, res_owner);
		}
	}

	return false;
}

bool qdInterfaceScreen::init(bool is_game_active) {
	for (element_list_t::const_iterator it = element_list().begin(); it != element_list().end(); ++it) {
		(*it)->init(is_game_active);
		if ((*it)->linked_to_option())
			(*it)->set_option_value(qdInterfaceDispatcher::option_value((*it)->option_ID()));
	}

	build_visible_elements_list();

	return true;
}

bool qdInterfaceScreen::hide_element(const char *element_name, bool temporary_hide) {
	if (qdInterfaceElement *p = get_element(element_name))
		return hide_element(p, temporary_hide);

	return false;
}

bool qdInterfaceScreen::hide_element(qdInterfaceElement *p, bool temporary_hide) {
	if (!temporary_hide)
		p->hide();

	sorted_element_list_t::iterator it = Common::find(_sorted_elements.begin(), _sorted_elements.end(), p);
	if (it != _sorted_elements.end())
		_sorted_elements.erase(it);

	return true;
}

bool qdInterfaceScreen::show_element(const char *element_name) {
	if (qdInterfaceElement *p = get_element(element_name))
		return show_element(p);

	return false;
}

bool qdInterfaceScreen::show_element(qdInterfaceElement *p) {
	p->show();

	sorted_element_list_t::iterator it = Common::find(_sorted_elements.begin(), _sorted_elements.end(), p);
	if (it == _sorted_elements.end()) {
		_sorted_elements.push_back(p);
		sort_elements();
	}

	return true;
}

bool qdInterfaceScreen::sort_elements() {
	Common::sort(_sorted_elements.begin(), _sorted_elements.end(), qdInterfaceElementsOrdering());
	return true;
}

bool qdInterfaceScreen::build_visible_elements_list() {
	_sorted_elements.clear();

	for (element_list_t::const_iterator it = element_list().begin(); it != element_list().end(); ++it) {
		if ((*it)->is_visible())
			_sorted_elements.push_back(*it);
	}

	sort_elements();

	return true;
}

void qdInterfaceScreen::activate_personage_buttons(const qdNamedObject *p) {
	for (element_list_t::const_iterator it = element_list().begin(); it != element_list().end(); ++it) {
		if (qdInterfaceButton *bt = dynamic_cast<qdInterfaceButton *>(*it)) {
			if (const qdInterfaceEvent *ev = bt->find_event(qdInterfaceEvent::EVENT_ACTIVATE_PERSONAGE)) {
				if (p) {
					if (ev->has_data() && !strcmp(p->name(), ev->event_data()))
						bt->activate_state(1);
					else
						bt->activate_state(0);
				} else
					bt->activate_state(0);
			}
		}
	}
}

void qdInterfaceScreen::update_personage_buttons() {
	qdGameDispatcher *p = qdGameDispatcher::get_dispatcher();
	if (!p) return;

	qdGameScene *sp = p->get_active_scene();
	if (!sp) return;

	for (element_list_t::const_iterator it = element_list().begin(); it != element_list().end(); ++it) {
		if (qdInterfaceButton *bt = dynamic_cast<qdInterfaceButton * >(*it)) {
			if (!sp->set_personage_button(bt)) {
				if (bt->find_event(qdInterfaceEvent::EVENT_ACTIVATE_PERSONAGE))
					hide_element(bt);
			}
		}
	}
}

} // namespace QDEngine
