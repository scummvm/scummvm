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
#include "qdengine/parser/qdscr_parser.h"
#include "qdengine/parser/xml_tag.h"
#include "qdengine/qdcore/qd_game_dispatcher.h"
#include "qdengine/qdcore/qd_interface_button.h"

namespace QDEngine {

qdInterfaceButton::qdInterfaceButton() : _cur_state(-1) {
}

qdInterfaceButton::qdInterfaceButton(const qdInterfaceButton &bt) : qdInterfaceElement(bt),
#ifndef _QD_INTERFACE_BUTTON_PTR_CONTAINER
	_states(bt._states),
#endif
	_cur_state(-1) {

#ifdef _QD_INTERFACE_BUTTON_PTR_CONTAINER
	for (int i = 0; i < bt.num_states(); i++)
		_states.push_back(new qdInterfaceElementState(*bt.get_state(i)));
#endif

	for (int i = 0; i < num_states(); i++) {
		get_state(i)->set_owner(this);
		get_state(i)->register_resources();
	}
}

qdInterfaceButton::~qdInterfaceButton() {
	_states.clear();
}

qdInterfaceButton &qdInterfaceButton::operator = (const qdInterfaceButton &bt) {
	if (this == &bt) return *this;

	*static_cast<qdInterfaceElement *>(this) = bt;

#ifdef _QD_INTERFACE_BUTTON_PTR_CONTAINER
	for (state_container_t::iterator it = _states.begin(); it != _states.end(); ++it)
		delete *it;

	_states.clear();

	for (int i = 0; i < bt.num_states(); i++)
		_states.push_back(new qdInterfaceElementState(*bt.get_state(i)));
#else
	_states.clear();
	_states = bt._states;
#endif

	for (int i = 0; i < num_states(); i++) {
		get_state(i)->set_owner(this);
		get_state(i)->register_resources();
	}

	_cur_state = -1;

	return *this;
}

bool qdInterfaceButton::activate_state(int state_num) {
	if (state_num >= (int)_states.size())
		state_num = _states.size() - 1;

	if (!_states.size() || _cur_state == state_num) return false;

	if (state_num < 0)
		state_num = 0;

	_cur_state = state_num;

	if (state_num < (int)_states.size())
		return set_state(get_state(state_num));

	return true;
}

bool qdInterfaceButton::activate_state(const char *state_name) {
	for (int i = 0; i < num_states(); i++) {
		qdInterfaceElementState *p = get_state(i);
		if (!strcmp(p->name(), state_name))
			return activate_state(i);
	}

	return false;
}

bool qdInterfaceButton::set_option_value(int value) {
	value = CLIP<int>(value, 0, _states.size() - 1);

	activate_state(value);
	return true;
}

bool qdInterfaceButton::add_state(const qdInterfaceElementState &st) {
#ifdef _QD_INTERFACE_BUTTON_PTR_CONTAINER
	_states.push_back(new qdInterfaceElementState(st));
#else
	_states.push_back(st);
#endif

	get_state(_states.size() - 1)->set_owner(this);
	get_state(_states.size() - 1)->register_resources();

	return true;
}

bool qdInterfaceButton::insert_state(int insert_before, const qdInterfaceElementState &st) {
	assert(insert_before >= 0 && insert_before < (int)_states.size());

#ifdef _QD_INTERFACE_BUTTON_PTR_CONTAINER
	_states.insert(_states.begin() + insert_before, new qdInterfaceElementState(st));
#else
	_states.insert(_states.begin() + insert_before, st);
#endif

	get_state(insert_before)->set_owner(this);
	get_state(insert_before)->register_resources();

	return true;
}

bool qdInterfaceButton::erase_state(int state_num) {
	assert(state_num >= 0 && state_num < (int)_states.size());

#ifdef _QD_INTERFACE_BUTTON_PTR_CONTAINER
//	delete *(_states.begin() + state_num);
#endif

	_states.erase(_states.begin() + state_num);

	if (_cur_state == state_num)
		activate_state(--_cur_state);

	return true;
}

bool qdInterfaceButton::mouse_handler(int x, int y, mouseDispatcher::mouseEvent ev) {
	if (_cur_state == -1) return false;

	return get_state(_cur_state)->mouse_handler(x, y, ev);
}

bool qdInterfaceButton::keyboard_handler(Common::KeyCode vkey) {
	return false;
}

bool qdInterfaceButton::init(bool is_game_active) {
	set_lock(false);

	if (!is_game_active) {
		for (int i = 0; i < num_states(); i++) {
			if (get_state(i)->need_active_game()) {
				set_lock(true);
				break;
			}
		}
	}

	for (int i = 0; i < num_states(); i++) {
		qdInterfaceElementState *p = get_state(i);
		p->set_state_mode(qdInterfaceElementState::DEFAULT_MODE);
	}

	if (_cur_state != -1)
		return set_state(get_state(_cur_state));

	return true;
}

bool qdInterfaceButton::save_script_body(Common::WriteStream &fh, int indent) const {
	for (int i = 0; i < num_states(); i++) {
		get_state(i)->save_script(fh, indent + 1);
	}

	return true;
}

bool qdInterfaceButton::load_script_body(const xml::tag *p) {
	int num_states = 0;
	for (xml::tag::subtag_iterator it = p->subtags_begin(); it != p->subtags_end(); ++it) {
		switch (it->ID()) {
		case QDSCR_INTERFACE_ELEMENT_STATE:
			num_states++;
			break;
		}
	}

	if (num_states) {
		_states.reserve(num_states);
		_cur_state = 0;
	}

	for (xml::tag::subtag_iterator it = p->subtags_begin(); it != p->subtags_end(); ++it) {
		switch (it->ID()) {
		case QDSCR_INTERFACE_ELEMENT_STATE: {
			qdInterfaceElementState st;
			st.load_script(&*it);
			add_state(st);
		}
		break;
		}
	}

	return true;
}

bool qdInterfaceButton::quant(float dt) {
	debugC(9, kDebugQuant, "qdInterfaceButton::quant()");
	qdInterfaceElement::quant(dt);

	if (find_event(qdInterfaceEvent::EVENT_CLEAR_MOUSE)) {
		if (qdGameDispatcher * dp = qdGameDispatcher::get_dispatcher()) {
			debugC(3, kDebugQuant, "qdInterfaceButton::quant()");
			if (dp->is_on_mouse(NULL))
				activate_state(1);
			else
				activate_state(0);
		}
	}

	if (_cur_state != -1) {
		get_state(_cur_state)->quant(dt);
	}

	return true;
}

bool qdInterfaceButton::hit_test(int x, int y) const {
	if (_cur_state != -1) {
		if (get_state(_cur_state)->has_contour(get_state(_cur_state)->state_mode()))
			return get_state(_cur_state)->hit_test(x - r().x, y - r().y, get_state(_cur_state)->state_mode());
	}

	return qdInterfaceElement::hit_test(x, y);
}

bool qdInterfaceButton::change_state(bool direction) {
	if (num_states()) {
		if (_cur_state != -1) {
			_cur_state += (direction) ? 1 : -1;
			if (_cur_state < 0) _cur_state = num_states() - 1;
			if (_cur_state >= num_states()) _cur_state = 0;
		} else
			_cur_state = 0;

		return activate_state(_cur_state);
	}

	return false;
}

const qdInterfaceEvent *qdInterfaceButton::find_event(qdInterfaceEvent::event_t type) const {
	for (int i = 0; i < num_states(); i++) {
		const qdInterfaceElementState *p = get_state(i);
		if (const qdInterfaceEvent * ev = p->find_event(type))
			return ev;
	}

	return NULL;
}

bool qdInterfaceButton::has_event(qdInterfaceEvent::event_t type, const char *ev_data) const {
	for (int i = 0; i < num_states(); i++) {
		const qdInterfaceElementState *p = get_state(i);
		if (p->has_event(type, ev_data))
			return true;
	}

	return false;
}
} // namespace QDEngine
