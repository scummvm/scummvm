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

#ifndef QDENGINE_QDCORE_ANIMATION_SET_PREVIEW_H
#define QDENGINE_QDCORE_ANIMATION_SET_PREVIEW_H

#include "qdengine/system/graphics/gr_dispatcher.h"


namespace QDEngine {

class qdCamera;

class qdAnimation;
class qdAnimationSet;

//! Превью походки.
class qdAnimationSetPreview {
public:
	enum preview_mode_t {
		VIEW_WALK_ANIMATIONS,
		VIEW_STATIC_ANIMATIONS,
		VIEW_START_ANIMATIONS,
		VIEW_STOP_ANIMATIONS
	};

	qdAnimationSetPreview(qdAnimationSet *p);
	~qdAnimationSetPreview();

	preview_mode_t preview_mode() const {
		return _preview_mode;
	}
	void set_preview_mode(preview_mode_t md) {
		_preview_mode = md;
	}

	void set_screen(Vect2s offs, Vect2s size);

	uint32 back_color() const {
		return _back_color;
	}
	void set_back_color(uint32 col) {
		_back_color = col;
	}

	uint32 grid_color() const {
		return _grid_color;
	}
	void set_grid_color(uint32 col) {
		_grid_color = col;
	}

	int camera_angle() const {
		return _camera_angle;
	}
	void set_camera_angle(int ang);

	float camera_focus();
	void set_camera_focus(float f);

	int cell_size() const {
		return _cell_size;
	}
	void set_cell_size(int sz);

	float personage_speed() const {
		return _personage_speed;
	}
	void set_personage_speed(float sp) {
		_personage_speed = sp;
	}

	float personage_height() const {
		return _personage_height;
	}
	void set_personage_height(float h) {
		_personage_height = h;
	}

	int direction() const {
		return _direction;
	}
	bool set_direction(int dir);

	bool set_phase(float phase);

	void set_graph_dispatcher(grDispatcher *p);

	void start();
	void quant();
	void quant(float tm);

	void redraw();

	const qdAnimation *cur_animation() const {
		return _animation;
	}

private:
	int _start_time;

	int _direction;
	//float speed_;

	float _personage_speed;
	float _personage_height;

	qdAnimation *_animation;
	qdAnimationSet *_animation_set;

	qdCamera *_camera;
	int _camera_angle;

	grDispatcher *_graph_d;

	uint32 _back_color;
	uint32 _grid_color;

	int _cell_size;
	float _cell_offset;

	Vect2s _screen_offset;
	Vect2s _screen_size;

	preview_mode_t _preview_mode;

	void redraw_grid();
};

} // namespace QDEngine

#endif // QDENGINE_QDCORE_ANIMATION_SET_PREVIEW_H
