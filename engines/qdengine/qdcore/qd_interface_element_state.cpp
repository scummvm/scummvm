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

#include "qdengine/qdcore/qd_sound.h"
#include "qdengine/qdcore/qd_animation.h"

#include "qdengine/qdcore/qd_interface_element.h"
#include "qdengine/qdcore/qd_interface_dispatcher.h"
#include "qdengine/qdcore/qd_interface_element_state.h"


namespace QDEngine {

qdInterfaceElementState::qdInterfaceElementState() : _state_mode(DEFAULT_MODE), _prev_state_mode(DEFAULT_MODE) {
}

qdInterfaceElementState::qdInterfaceElementState(const qdInterfaceElementState &st) : qdInterfaceObjectBase(st),
	_events(st._events),
	_state_mode(st._state_mode),
	_prev_state_mode(st._prev_state_mode) {
	for (int i = 0; i < NUM_MODES; i++)
		_modes[i] = st._modes[i];

	register_resources();
}

qdInterfaceElementState::~qdInterfaceElementState() {
	unregister_resources();
}

qdInterfaceElementState &qdInterfaceElementState::operator = (const qdInterfaceElementState &st) {
	if (this == &st) return *this;

	unregister_resources();

	this->qdInterfaceObjectBase::operator = (st);

	_events = st._events;

	for (int i = 0; i < NUM_MODES; i++)
		_modes[i] = st._modes[i];

	register_resources();

	_state_mode = st._state_mode;
	_prev_state_mode = st._prev_state_mode;

	return *this;
}

bool qdInterfaceElementState::save_script(Common::WriteStream &fh, int indent) const {
	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}

	fh.writeString("<interface_element_state");

	if (name()) {
		fh.writeString(Common::String::format(" name=\"%s\"", qdscr_XML_string(name())));
	}

	fh.writeString(">\r\n");

	for (uint j = 0; j < _events.size(); j++) {
		for (int i = 0; i <= indent; i++) {
			fh.writeString("\t");
		}
		fh.writeString(Common::String::format("<event type=\"%d\"", int(_events[j].event())));

		if (_events[j].has_data()) {
			fh.writeString(Common::String::format(" event_data=\"%s\"", qdscr_XML_string(_events[j].event_data())));
		}

		if (_events[j].is_before_animation()) {
			fh.writeString(" before_animation=\"1\"");
		}

		if (_events[j].activation() != qdInterfaceEvent::EVENT_ACTIVATION_CLICK) {
			fh.writeString(Common::String::format(" activation_type=\"%d\"", (int)_events[j].activation()));
		}

		fh.writeString("/>\r\n");
	}

	for (int i = 0; i < NUM_MODES; i++) {
		if (has_state_mode(state_mode_t(i))) {
			_modes[i].save_script(fh, i, indent + 1);
		}
	}

	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}
	fh.writeString("</interface_element_state>\r\n");

	return true;
}

bool qdInterfaceElementState::load_script(const xml::tag *p) {
	int num_events = 0;
	for (xml::tag::subtag_iterator it = p->subtags_begin(); it != p->subtags_end(); ++it) {
		switch (it->ID()) {
		case QDSCR_INTERFACE_EVENT:
			num_events++;
			break;
		}
	}

	if (num_events)
		_events.reserve(num_events);

	for (xml::tag::subtag_iterator it = p->subtags_begin(); it != p->subtags_end(); ++it) {
		switch (it->ID()) {
		case QDSCR_NAME:
			set_name(it->data());
			break;
		case QDSCR_INTERFACE_ELEMENT_STATE_MODE:
			if (const xml::tag * tg = it->search_subtag(QDSCR_TYPE))
				_modes[state_mode_t(xml::tag_buffer(*tg).get_int())].load_script(&*it);
			break;
		case QDSCR_INTERFACE_EVENT: {
			qdInterfaceEvent::event_t ev = qdInterfaceEvent::EVENT_NONE;
			qdInterfaceEvent::activation_t act = qdInterfaceEvent::EVENT_ACTIVATION_CLICK;
			const char *ev_data = "";
			bool anm_flag = false;

			if (const xml::tag * tg = it->search_subtag(QDSCR_TYPE))
				ev = qdInterfaceEvent::event_t(xml::tag_buffer(*tg).get_int());
			if (const xml::tag * tg = it->search_subtag(QDSCR_INTERFACE_EVENT_DATA))
				ev_data = tg->data();
			if (const xml::tag * tg = it->search_subtag(QDSCR_INTERFACE_EVENT_BEFORE_ANIMATION)) {
				if (xml::tag_buffer(*tg).get_int())
					anm_flag = true;
			}
			if (const xml::tag * tg = it->search_subtag(QDSCR_INTERFACE_EVENT_ACTIVATION_TYPE))
				act = qdInterfaceEvent::activation_t(xml::tag_buffer(*tg).get_int());

			_events.push_back(qdInterfaceEvent(ev, ev_data, anm_flag, act));
			}
			break;
		default:
			break;
		}
	}

	register_resources();

	return true;
}

bool qdInterfaceElementState::quant(float dt) {
	debugC(9, kDebugQuant, "qdInterfaceElementState::quant(%f)", dt);
	if (qdInterfaceElement * ep = dynamic_cast<qdInterfaceElement * >(owner())) {
		if (_prev_state_mode == MOUSE_HOVER_MODE && state_mode() == DEFAULT_MODE)
			handle_events(qdInterfaceEvent::EVENT_ACTIVATION_HOVER, false);

		_prev_state_mode = state_mode();

		switch (ep->state_status(this)) {
		case qdInterfaceElement::STATE_INACTIVE:
			ep->set_state(this);
			break;
		case qdInterfaceElement::STATE_DONE:
			if (state_mode() == EVENT_MODE) {
				debugC(3, kDebugQuant, "qdInterfaceElementState::quant(%f) - EVENT_MODE", dt);
				handle_events(qdInterfaceEvent::EVENT_ACTIVATION_CLICK, false);
			}


			set_state_mode(DEFAULT_MODE);
			break;
		default:
			break;
		}

		return true;
	}

	return false;
}

void qdInterfaceElementState::set_sound_file(const Common::Path str, state_mode_t snd_id) {
	if (has_sound(snd_id)) {
		if (qdInterfaceElement * p = dynamic_cast<qdInterfaceElement * >(owner()))
			p->remove_resource(sound_file(snd_id), this);

		_modes[snd_id].set_sound(NULL);
	}

	_modes[snd_id].set_sound_file(str);
	if (has_sound(snd_id)) {
		if (qdInterfaceElement * p = dynamic_cast<qdInterfaceElement * >(owner()))
			_modes[snd_id].set_sound(dynamic_cast<const qdSound * >(p->add_resource(sound_file(snd_id), this)));
	}
}

void qdInterfaceElementState::set_animation_file(const Common::Path name, state_mode_t anm_id) {
	if (has_animation(anm_id)) {
		if (qdInterfaceElement * p = dynamic_cast<qdInterfaceElement * >(owner()))
			p->remove_resource(animation_file(anm_id), this);

		_modes[anm_id].set_animation(NULL);
	}

	_modes[anm_id].set_animation_file(name);
	if (has_animation(anm_id)) {
		if (qdInterfaceElement * p = dynamic_cast<qdInterfaceElement * >(owner()))
			_modes[anm_id].set_animation(dynamic_cast<const qdAnimation * >(p->add_resource(animation_file(anm_id), this)));
	}
}

bool qdInterfaceElementState::unregister_resources() {
	bool res = true;

	for (int i = 0; i < NUM_MODES; i++) {
		if (has_animation(state_mode_t(i))) {
			if (qdInterfaceElement * p = dynamic_cast<qdInterfaceElement * >(owner())) {
				if (!p->remove_resource(animation_file(state_mode_t(i)), this))
					res = false;

				_modes[i].set_animation(NULL);
			}
		}

		if (has_sound(state_mode_t(i))) {
			if (qdInterfaceElement * p = dynamic_cast<qdInterfaceElement * >(owner())) {
				if (!p->remove_resource(sound_file(state_mode_t(i)), this))
					res = false;

				_modes[i].set_sound(NULL);
			}
		}
	}

	return res;
}

bool qdInterfaceElementState::register_resources() {
	bool res = true;

	for (int i = 0; i < NUM_MODES; i++) {
		if (has_animation(state_mode_t(i))) {
			if (qdInterfaceElement * p = dynamic_cast<qdInterfaceElement * >(owner()))
				_modes[i].set_animation(dynamic_cast<const qdAnimation * >(p->add_resource(animation_file(state_mode_t(i)), this)));

			if (!_modes[i].animation()) res = false;
		}

		if (has_sound(state_mode_t(i))) {
			if (qdInterfaceElement * p = dynamic_cast<qdInterfaceElement * >(owner()))
				_modes[i].set_sound(dynamic_cast<const qdSound * >(p->add_resource(sound_file(state_mode_t(i)), this)));

			if (!_modes[i].sound()) res = false;
		}
	}

	return res;
}

bool qdInterfaceElementState::has_state_mode(state_mode_t mode) const {
	switch (mode) {
	case DEFAULT_MODE:
		return true;
	case MOUSE_HOVER_MODE:
		if (has_animation(MOUSE_HOVER_MODE) || has_sound(MOUSE_HOVER_MODE) || has_contour(MOUSE_HOVER_MODE)) return true;
		return false;
	case EVENT_MODE:
		if (_events.size()) return true;
		if (has_animation(EVENT_MODE) || has_sound(EVENT_MODE) || has_contour(EVENT_MODE)) return true;
		return false;
	}

	return false;
}

bool qdInterfaceElementState::mouse_handler(int x, int y, mouseDispatcher::mouseEvent ev) {
	switch (ev) {
	case mouseDispatcher::EV_MOUSE_MOVE:
		if (state_mode() != EVENT_MODE) {
			if (has_state_mode(MOUSE_HOVER_MODE)) {
				set_state_mode(MOUSE_HOVER_MODE);
				if (_prev_state_mode != MOUSE_HOVER_MODE)
					handle_events(qdInterfaceEvent::EVENT_ACTIVATION_HOVER, true);
				return true;
			}
		}
		break;
	case mouseDispatcher::EV_LEFT_DOWN:
	case mouseDispatcher::EV_RIGHT_DOWN:
		if (has_state_mode(EVENT_MODE)) {
			set_state_mode(EVENT_MODE);
			handle_events(qdInterfaceEvent::EVENT_ACTIVATION_CLICK, true);
			return true;
		}
		break;
	default:
		break;
	}

	return false;
}

bool qdInterfaceElementState::keyboard_handler(Common::KeyCode vkey) {
	return false;
}

bool qdInterfaceElementState::handle_events(qdInterfaceEvent::activation_t activation_type, bool before_animation) {
	if (qdInterfaceDispatcher * dp = qdInterfaceDispatcher::get_dispatcher()) {
		for (uint i = 0; i < _events.size(); i++) {
			if (_events[i].activation() == activation_type && _events[i].is_before_animation() == before_animation) {
				dp->handle_event(_events[i].event(), _events[i].event_data(), owner());
			}
		}

		return true;
	}

	return false;
}

bool qdInterfaceElementState::get_contour(state_mode_t mode, qdContour &cnt) const {
	_modes[mode].get_contour(cnt);
	return true;
}

bool qdInterfaceElementState::set_contour(state_mode_t mode, const qdContour &cnt) {
	_modes[mode].set_contour(cnt);
	return true;
}

bool qdInterfaceElementState::need_active_game() const {
	for (uint i = 0; i < _events.size(); i++) {
		if (_events[i].event() == qdInterfaceEvent::EVENT_SAVE_GAME)
			return true;
		if (_events[i].event() == qdInterfaceEvent::EVENT_CHANGE_PERSONAGE)
			return true;
		if (_events[i].event() == qdInterfaceEvent::EVENT_RESUME_GAME)
			return true;
		if (_events[i].event() == qdInterfaceEvent::EVENT_ACTIVATE_PERSONAGE)
			return true;
		if (_events[i].event() == qdInterfaceEvent::EVENT_SET_SAVE_MODE)
			return true;
	}

	return false;
}

const qdInterfaceEvent *qdInterfaceElementState::find_event(qdInterfaceEvent::event_t type) const {
	events_container_t::const_iterator it = Common::find(_events.begin(), _events.end(), type);
	if (it != _events.end())
		return &*it;

	return NULL;
}

bool qdInterfaceElementState::has_event(qdInterfaceEvent::event_t type, const char *ev_data) const {
	for (events_container_t::const_iterator it = _events.begin(); it != _events.end(); ++it) {
		if (it->event() == type) {
			if ((!ev_data && !it->event_data()) || (it->event_data() && !strcmp(ev_data, it->event_data())))
				return true;
		}
	}

	return false;
}
} // namespace QDEngine
