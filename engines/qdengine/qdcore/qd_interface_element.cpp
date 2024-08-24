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

#include "qdengine/qd_fwd.h"
#include "qdengine/parser/qdscr_parser.h"
#include "qdengine/parser/xml_tag_buffer.h"
#include "qdengine/system/sound/snd_dispatcher.h"

#include "qdengine/qdcore/qd_sound.h"
#include "qdengine/qdcore/qd_interface_screen.h"
#include "qdengine/qdcore/qd_interface_element.h"
#include "qdengine/qdcore/qd_interface_element_state.h"
#include "qdengine/qdcore/qd_interface_background.h"
#include "qdengine/qdcore/qd_interface_counter.h"
#include "qdengine/qdcore/qd_interface_button.h"
#include "qdengine/qdcore/qd_interface_save.h"
#include "qdengine/qdcore/qd_interface_slider.h"
#include "qdengine/qdcore/qd_interface_text_window.h"


namespace QDEngine {

Vect2i qdInterfaceElement::_screen_offset = Vect2i(0, 0);

qdInterfaceElement::qdInterfaceElement() : _r(0, 0),
	_option_ID(OPTION_NONE),
	_screen_depth(0),
	_is_visible(true),
	_is_locked(false),
	_last_animation_frame(NULL) {
}

qdInterfaceElement::qdInterfaceElement(const qdInterfaceElement &el) : qdInterfaceObjectBase(el),
	_r(el._r),
	_option_ID(el._option_ID),
	_option_data(el._option_data),
	_screen_depth(el._screen_depth),
	_is_visible(el._is_visible),
	_is_locked(el._is_locked),
	_last_animation_frame(NULL) {
}

qdInterfaceElement::~qdInterfaceElement() {
}

qdInterfaceElement &qdInterfaceElement::operator = (const qdInterfaceElement &el) {
	if (this == &el) return *this;

	this->qdInterfaceObjectBase::operator = (el);

	_r = el._r;
	_option_ID = el._option_ID;
	_option_data = el._option_data;
	_screen_depth = el._screen_depth;
	_is_visible = el._is_visible;

	return *this;
}

bool qdInterfaceElement::set_animation(const qdAnimation *anm, int anm_flags) {
	if (anm) {
		anm->create_reference(&_animation);

		if (anm_flags & QD_ANIMATION_FLAG_LOOP)
			_animation.set_flag(QD_ANIMATION_FLAG_LOOP);

		if (anm_flags & QD_ANIMATION_FLAG_FLIP_HORIZONTAL)
			_animation.set_flag(QD_ANIMATION_FLAG_FLIP_HORIZONTAL);

		if (anm_flags & QD_ANIMATION_FLAG_FLIP_VERTICAL)
			_animation.set_flag(QD_ANIMATION_FLAG_FLIP_VERTICAL);

		_animation.start();
	} else
		_animation.clear();

	return true;
}

bool qdInterfaceElement::set_state(const qdInterfaceElementState *p) {
	qdInterfaceElementState::state_mode_t mode = p->state_mode();

	set_animation(p->animation(mode), p->animation_flags(mode));

	if (p->sound(mode)) {
		if (sndDispatcher * dp = sndDispatcher::get_dispatcher())
			dp->stop_sound(&_sound_handle);

		p->sound(mode)->play(&_sound_handle);
	}

	return true;
}

bool qdInterfaceElement::redraw() const {
	_animation.redraw(r().x, r().y, 0);
	return true;
}

bool qdInterfaceElement::need_redraw() const {
	if (_last_animation_frame != _animation.get_cur_frame())
		return true;

	if (_last_screen_region != screen_region())
		return true;

	return false;
}

bool qdInterfaceElement::post_redraw() {
	_last_screen_region = screen_region();
	_last_animation_frame = _animation.get_cur_frame();

	return true;
}

bool qdInterfaceElement::quant(float dt) {
	_animation.quant(dt);
	return true;
}

grScreenRegion qdInterfaceElement::screen_region() const {
	if (!_animation.is_empty()) {
		grScreenRegion reg = _animation.screen_region();
		reg.move(r().x, r().y);

		return reg;
	} else
		return grScreenRegion_EMPTY;
}

bool qdInterfaceElement::save_script(Common::WriteStream &fh, int indent) const {
	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}

	fh.writeString("<interface_element");
	fh.writeString(Common::String::format(" type=\"%d\"", static_cast<int>(get_element_type())));

	if (name()) {
		fh.writeString(Common::String::format(" name=\"%s\"", qdscr_XML_string(name())));
	}
	fh.writeString(Common::String::format(" pos=\"%d %d %d\"", _r.x, _r.y, _screen_depth));

	if (_option_ID != OPTION_NONE) {
		fh.writeString(Common::String::format(" option_id=\"%d\"", (int)_option_ID));
	}

	fh.writeString(">\r\n");

	if (!save_script_body(fh, indent)) {
		return false;
	}

	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}

	fh.writeString("</interface_element>\r\n");
	return true;

}

bool qdInterfaceElement::load_script(const xml::tag *p) {
	for (xml::tag::subtag_iterator it = p->subtags_begin(); it != p->subtags_end(); ++it) {
		switch (it->ID()) {
		case QDSCR_NAME:
			set_name(it->data());
			break;
		case QDSCR_INTERFACE_ELEMENT_POS:
			xml::tag_buffer(*it) > _r.x > _r.y > _screen_depth;
			break;
		case QDSCR_INTERFACE_OPTION_ID:
			_option_ID = option_ID_t(xml::tag_buffer(*it).get_int());
			break;
		}
	}

	return load_script_body(p);
}

qdResource *qdInterfaceElement::add_resource(const Common::Path file_name, const qdInterfaceElementState *res_owner) {
	if (qdInterfaceScreen * p = dynamic_cast<qdInterfaceScreen * >(owner()))
		return p->add_resource(file_name, res_owner);

	return NULL;
}

bool qdInterfaceElement::remove_resource(const Common::Path file_name, const qdInterfaceElementState *res_owner) {
	if (qdInterfaceScreen * p = dynamic_cast<qdInterfaceScreen * >(owner()))
		return p->remove_resource(file_name, res_owner);

	return false;
}

qdInterfaceElement *qdInterfaceElement::create_element(element_type tp) {
	switch (tp) {
	case EL_BACKGROUND:
		return new qdInterfaceBackground;
	case EL_BUTTON:
		return new qdInterfaceButton;
	case EL_SLIDER:
		return new qdInterfaceSlider;
	case EL_SAVE:
		return new qdInterfaceSave;
	case EL_TEXT_WINDOW:
		return new qdInterfaceTextWindow;
	case EL_COUNTER:
		return new qdInterfaceCounter;
	}

	return NULL;
}

void qdInterfaceElement::destroy_element(qdInterfaceElement *p) {
	delete p;
}


bool qdInterfaceElement::hit_test(int x, int y) const {
	if (!_animation.is_empty()) {
		x -= r().x;
		y -= r().y;

		return _animation.hit(x, y);
	}

	return false;
}

qdInterfaceElement::state_status_t qdInterfaceElement::state_status(const qdInterfaceElementState *p) const {
	qdInterfaceElementState::state_mode_t mode = p->state_mode();

	if (p->animation(mode)) {
		if (p->animation(mode)->is_reference(&_animation)) {
			if (!_animation.is_finished())
				return STATE_ACTIVE;
		} else
			return STATE_INACTIVE;
	} else {
		if (!_animation.is_empty())
			return STATE_INACTIVE;
	}

	if (p->sound(mode)) {
		if (!p->sound(mode)->is_stopped(&_sound_handle))
			return STATE_ACTIVE;
	}

	return STATE_DONE;
}
} // namespace QDEngine
