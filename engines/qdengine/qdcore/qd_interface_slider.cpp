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
#include "qdengine/qdcore/qd_interface_slider.h"


namespace QDEngine {

qdInterfaceSlider::qdInterfaceSlider() : _active_rectangle(0, 0),
	_phase(0.5f),
	_orientation(SL_HORIZONTAL),
	_track_mouse(false),
	_background_offset(Vect2i(0, 0)) {
	_inverse_direction = false;
	_background.set_owner(this);
	_slider.set_owner(this);
}

qdInterfaceSlider::qdInterfaceSlider(const qdInterfaceSlider &sl) : qdInterfaceElement(sl),
	_active_rectangle(sl._active_rectangle),
	_phase(sl._phase),
	_orientation(sl._orientation),
	_inverse_direction(sl._inverse_direction),
	_track_mouse(false) {
	_background.set_owner(this);
	_slider.set_owner(this);

	_background = sl._background;
	_background_offset = sl._background_offset;
	_slider = sl._slider;
}

qdInterfaceSlider::~qdInterfaceSlider() {
	_background.unregister_resources();
	_slider.unregister_resources();
}

qdInterfaceSlider &qdInterfaceSlider::operator = (const qdInterfaceSlider &sl) {
	if (this == &sl) return *this;

	*static_cast<qdInterfaceElement *>(this) = sl;

	_background = sl._background;
	_background_offset = sl._background_offset;
	_slider = sl._slider;

	_active_rectangle = sl._active_rectangle;

	_phase = sl._phase;
	_orientation = sl._orientation;
	_inverse_direction = sl._inverse_direction;

	_track_mouse = false;

	return *this;
}

bool qdInterfaceSlider::mouse_handler(int x, int y, mouseDispatcher::mouseEvent ev) {
	x -= r().x;
	y -= r().y;

	Vect2i rect(_active_rectangle);
	if (!_slider_animation.is_empty()) {
		if (rect.x < _slider_animation.size_x())
			rect.x = _slider_animation.size_x();
		if (rect.y < _slider_animation.size_y())
			rect.y = _slider_animation.size_y();
	}

	switch (ev) {
	case mouseDispatcher::EV_LEFT_DOWN:
		if (x >= -rect.x / 2 && x < rect.x / 2 && y >= -rect.y / 2 && y < rect.y / 2) {
			set_phase(offset2phase(Vect2i(x, y)));
			_track_mouse = true;
			return true;
		}
		break;
	case mouseDispatcher::EV_MOUSE_MOVE:
		if (_track_mouse) {
			if (mouseDispatcher::instance()->is_pressed(mouseDispatcher::ID_BUTTON_LEFT)) {
				set_phase(offset2phase(Vect2i(x, y)));
				return true;
			} else
				_track_mouse = false;
		}
		break;
	default:
		break;
	}

	return false;
}

bool qdInterfaceSlider::keyboard_handler(Common::KeyCode vkey) {
	return false;
}

int qdInterfaceSlider::option_value() const {
	return round(_phase * 255.0f);
}

bool qdInterfaceSlider::set_option_value(int value) {
	set_phase(float(value) / 255.0f);
	return true;
}

bool qdInterfaceSlider::init(bool is_game_active) {
	set_state(&_background);
	set_slider_animation(_slider.animation());

	_track_mouse = false;

	return true;
}

bool qdInterfaceSlider::save_script_body(Common::WriteStream &fh, int indent) const {
	if (!_background.save_script(fh, indent)) {
		return false;
	}

	if (!_slider.save_script(fh, indent)) {
		return false;
	}

	if (_active_rectangle.x || _active_rectangle.y) {
		for (int i = 0; i <= indent; i++) {
			fh.writeString("\t");
		}
		fh.writeString(Common::String::format("<slider_rect>%d %d</slider_rect>\r\n", _active_rectangle.x, _active_rectangle.y));
	}

	if (_background_offset.x || _background_offset.y) {
		for (int i = 0; i <= indent; i++) {
			fh.writeString("\t");
		}
		fh.writeString(Common::String::format("<background_offset>%d %d</background_offset>\r\n", _background_offset.x, _background_offset.y));
	}

	if (_orientation != SL_HORIZONTAL) {
		for (int i = 0; i <= indent; i++) {
			fh.writeString("\t");
		}
		fh.writeString(Common::String::format("<slider_orientation>%d</slider_orientation>\r\n", int(_orientation)));
	}

	if (_inverse_direction) {
		for (int i = 0; i <= indent; i++) {
			fh.writeString("\t");
		}
		fh.writeString("<inverse_direction>1</inverse_direction>\r\n");
	}

	return true;
}

bool qdInterfaceSlider::load_script_body(const xml::tag *p) {
	bool background_flag = false;
	for (xml::tag::subtag_iterator it = p->subtags_begin(); it != p->subtags_end(); ++it) {
		switch (it->ID()) {
		case QDSCR_INTERFACE_ELEMENT_STATE:
			if (!background_flag) {
				if (!_background.load_script(&*it)) return false;
				background_flag = true;
			} else {
				if (!_slider.load_script(&*it)) return false;
			}
			break;
		case QDSCR_INTERFACE_SLIDER_RECTANGLE:
			xml::tag_buffer(*it) > _active_rectangle.x > _active_rectangle.y;
			break;
		case QDSCR_INTERFACE_SLIDER_ORIENTATION: {
			int v;
			xml::tag_buffer(*it) > v;
			_orientation = orientation_t(v);
		}
		break;
		case QDSCR_INVERSE_DIRECTION:
			_inverse_direction = xml::tag_buffer(*it).get_int() != 0;
			break;
		case QDSCR_INTERFACE_BACKGROUND_OFFSET:
			xml::tag_buffer(*it) > _background_offset.x > _background_offset.y;
			break;
		}
	}

	return true;
}

bool qdInterfaceSlider::redraw() const {
	Vect2i rr = r() + _background_offset;
	animation().redraw(rr.x, rr.y, 0);

	if (!_slider_animation.is_empty()) {
		rr = r() + phase2offset(_phase);
		_slider_animation.redraw(rr.x, rr.y, 0);
	}

	return true;
}

int qdInterfaceSlider::size_x() const {
	int x = _active_rectangle.x;

	if (!_slider_animation.is_empty()) {
		if (x < _slider_animation.size_x())
			x = _slider_animation.size_x();
	}

	return x;
}

int qdInterfaceSlider::size_y() const {
	int y = _active_rectangle.y;

	if (!_slider_animation.is_empty()) {
		if (y < _slider_animation.size_y())
			y = _slider_animation.size_y();
	}

	return y;
}

grScreenRegion qdInterfaceSlider::screen_region() const {
	if (!_slider_animation.is_empty()) {
		grScreenRegion reg = qdInterfaceElement::screen_region();
		reg.move(_background_offset.x, _background_offset.y);

		Vect2i rr = r() + phase2offset(_phase);
		grScreenRegion reg1 = _slider_animation.screen_region();
		reg1.move(rr.x, rr.y);

		reg += reg1;

		return reg;
	} else
		return qdInterfaceElement::screen_region();
}

bool qdInterfaceSlider::set_slider_animation(const qdAnimation *anm, int anm_flags) {
	if (anm) {
		anm->create_reference(&_slider_animation);

		if (anm_flags & QD_ANIMATION_FLAG_LOOP)
			_slider_animation.set_flag(QD_ANIMATION_FLAG_LOOP);

		if (anm_flags & QD_ANIMATION_FLAG_FLIP_HORIZONTAL)
			_slider_animation.set_flag(QD_ANIMATION_FLAG_FLIP_HORIZONTAL);

		if (anm_flags & QD_ANIMATION_FLAG_FLIP_VERTICAL)
			_slider_animation.set_flag(QD_ANIMATION_FLAG_FLIP_VERTICAL);

		_slider_animation.start();
	} else
		_slider_animation.clear();

	return true;
}

Vect2i qdInterfaceSlider::phase2offset(float ph) const {
	if (ph < 0.0f) ph = 0.0f;
	if (ph > 1.0f) ph = 1.0f;

	Vect2i offs(0, 0);

	if (_inverse_direction)
		ph = 1.0f - ph;

	switch (_orientation) {
	case SL_HORIZONTAL:
		offs.x = int(ph * _active_rectangle.x) - _active_rectangle.x / 2;
		break;
	case SL_VERTICAL:
		offs.y = _active_rectangle.y / 2 - int(ph * _active_rectangle.y);
		break;
	}

	return offs;
}

float qdInterfaceSlider::offset2phase(const Vect2i &offs) const {
	float ph = 0.0f;

	switch (_orientation) {
	case SL_HORIZONTAL:
		if (!_active_rectangle.x) return 0.0f;
		ph = float(offs.x + _active_rectangle.x / 2) / float(_active_rectangle.x);
		break;
	case SL_VERTICAL:
		if (!_active_rectangle.y) return 0.0f;
		ph = float(-offs.y + _active_rectangle.y / 2) / float(_active_rectangle.y);
		break;
	}

	if (ph < 0.0f) ph = 0.0f;
	if (ph > 1.0f) ph = 1.0f;

	if (_inverse_direction)
		ph = 1.0f - ph;

	return ph;
}

bool qdInterfaceSlider::hit_test(int x, int y) const {
	x -= r().x;
	y -= r().y;

	Vect2i rect(_active_rectangle);
	if (!_slider_animation.is_empty()) {
		if (rect.x < _slider_animation.size_x())
			rect.x = _slider_animation.size_x();
		if (rect.y < _slider_animation.size_y())
			rect.y = _slider_animation.size_y();
	}

	if (x >= -rect.x / 2 && x < rect.x / 2 && y >= -rect.y / 2 && y < rect.y / 2)
		return true;

	return false;
}
} // namespace QDEngine
