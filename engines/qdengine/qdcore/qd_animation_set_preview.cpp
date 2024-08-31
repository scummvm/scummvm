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

#include "common/system.h"

#include "qdengine/qd_fwd.h"
#include "qdengine/qdcore/qd_animation_set.h"
#include "qdengine/qdcore/qd_animation_set_preview.h"
#include "qdengine/qdcore/qd_camera.h"


namespace QDEngine {

qdAnimationSetPreview::qdAnimationSetPreview(qdAnimationSet *p) :
	_graph_d(0),
	_animation_set(p),
	_preview_mode(VIEW_WALK_ANIMATIONS) {
	_animation = new qdAnimation;

	_camera = new qdCamera;
	_camera->set_focus(2000.0f);
	_camera_angle = 45;

	_start_time = 0;
	_back_color = 0x000000FF;
	_grid_color = 0x00FF00FF;

	set_direction(0);

	_cell_size = 100;

	_personage_height = float(_animation->size_y());

	p->load_animations(NULL);
}

qdAnimationSetPreview::~qdAnimationSetPreview() {
	_animation_set->free_animations(NULL);

	delete _animation;
	delete _camera;
}

void qdAnimationSetPreview::set_graph_dispatcher(grDispatcher *p) {
	_graph_d = p;
	set_screen(Vect2s(0, 0), Vect2s(p->get_SizeX(), p->get_SizeY()));
}

void qdAnimationSetPreview::start() {
	_start_time = g_system->getMillis();

	_animation->start();
	_cell_offset = 0.0f;
}

void qdAnimationSetPreview::quant() {
	int time = g_system->getMillis();
	float tm = float(time - _start_time) / 1000.0f;
	_start_time = time;

	if (tm >= 0.3f) return;

	quant(tm);
}

void qdAnimationSetPreview::quant(float tm) {
	_animation->quant(tm);

	_cell_offset -= _personage_speed * tm;
	while (_cell_offset <= -float(_cell_size)) _cell_offset += float(_cell_size);
}

void qdAnimationSetPreview::redraw() {
	grDispatcher *gp = grDispatcher::set_instance(_graph_d);

	grDispatcher::instance()->fill(_back_color);

	redraw_grid();

	Vect2s v = _camera->global2scr(Vect3f(0.0f, 0.0f, _personage_height / 2.0f));
	float scale = _camera->get_scale(Vect3f(0.0f, 0.0f, _personage_height / 2.0f));
	_animation->redraw(v.x, v.y, scale);

	Vect2s v0 = _camera->global2scr(Vect3f(0.0f, 0.0f, _personage_height));
	Vect2s v1 = _camera->global2scr(Vect3f(0.0f, 0.0f, 0.0f));

	const int rect_sz = 4;
	grDispatcher::instance()->rectangle(v.x - rect_sz / 2, v.y - rect_sz / 2, rect_sz, rect_sz, _grid_color, _grid_color, GR_FILLED);

	const int line_sz = 10;
	grDispatcher::instance()->line(v0.x - line_sz, v0.y, v0.x + line_sz, v0.y, _grid_color);
	grDispatcher::instance()->line(v1.x - line_sz, v1.y, v1.x + line_sz, v1.y, _grid_color);
	grDispatcher::instance()->line(v0.x, v0.y, v1.x, v1.y, _grid_color);
	grDispatcher::instance()->rectangle(_screen_offset.x, _screen_offset.y, _screen_size.x, _screen_size.y, _grid_color, 0, GR_OUTLINED);
	grDispatcher::instance()->flush();

	grDispatcher::set_instance(gp);
}

bool qdAnimationSetPreview::set_direction(int dir) {
	bool result = false;

	_animation->clear();
	_personage_speed = 0.0f;
	_direction = dir;
	_cell_offset = 0.0f;

	float angle = _animation_set->get_index_angle(_direction) * 180.0f / M_PI;
	_camera->rotate_and_scale(_camera_angle, 0, angle, 1.0f, 1.0f, 1.0f);

	qdAnimationInfo *p = NULL;
	if (_preview_mode == VIEW_WALK_ANIMATIONS)
		p = _animation_set->get_animation_info(dir);
	else if (_preview_mode == VIEW_STATIC_ANIMATIONS)
		p = _animation_set->get_static_animation_info(dir);
	else if (_preview_mode == VIEW_START_ANIMATIONS)
		p = _animation_set->get_start_animation_info(dir);
	else if (_preview_mode == VIEW_STOP_ANIMATIONS)
		p = _animation_set->get_stop_animation_info(dir);

	if (p) {
		qdAnimation *ap = p->animation();
		if (ap) {
			ap->create_reference(_animation, p);
			result = true;
		}
		_personage_speed = p->speed();
	}

	_animation->start();

	return result;
}

void qdAnimationSetPreview::set_cell_size(int sz) {
	_cell_size = sz;

	_animation->start();
	_cell_offset = 0.0f;
}

void qdAnimationSetPreview::set_screen(Vect2s offs, Vect2s size) {
	if (!_graph_d) return;

	_screen_offset = offs;
	_screen_size = size;

	_camera->set_scr_size(size.x, size.y);
	_camera->set_scr_center(offs.x + size.x / 2, offs.y + size.y * 3 / 4);

	_graph_d->setClip(offs.x, offs.y, offs.x + size.x, offs.y + size.y);
}

void qdAnimationSetPreview::redraw_grid() {
	float size = 0;
	Vect2f p = _camera->scr2plane(_screen_offset);
	if (fabs(p.x) > size) size = fabs(p.x);
	if (fabs(p.y) > size) size = fabs(p.y);
	p = _camera->scr2plane(_screen_offset + _screen_size);
	if (fabs(p.x) > size) size = fabs(p.x);
	if (fabs(p.y) > size) size = fabs(p.y);
	p = _camera->scr2plane(Vect2s(_screen_offset.x + _screen_size.x, _screen_offset.y));
	if (fabs(p.x) > size) size = fabs(p.x);
	if (fabs(p.y) > size) size = fabs(p.y);
	p = _camera->scr2plane(Vect2s(_screen_offset.x, _screen_offset.y + _screen_size.y));
	if (fabs(p.x) > size) size = fabs(p.x);
	if (fabs(p.y) > size) size = fabs(p.y);

	int sz = round(size) + _cell_size;
	sz -= sz % _cell_size;
	for (int i = -sz; i <= sz; i += _cell_size) {
		int dx = round(_cell_offset);

		Vect3f v00 = _camera->global2camera_coord(Vect3f(i + dx, -sz, 0));
		Vect3f v10 = _camera->global2camera_coord(Vect3f(i + dx, sz, 0));
		if (_camera->line_cutting(v00, v10)) {
			Vect2s v0 = _camera->camera_coord2scr(v00);
			Vect2s v1 = _camera->camera_coord2scr(v10);
			grDispatcher::instance()->line(v0.x, v0.y, v1.x, v1.y, _grid_color, 0);
		}

		v00 = _camera->global2camera_coord(Vect3f(-sz + dx, i, 0));
		v10 = _camera->global2camera_coord(Vect3f(sz + dx, i, 0));
		if (_camera->line_cutting(v00, v10)) {
			Vect2s v0 = _camera->camera_coord2scr(v00);
			Vect2s v1 = _camera->camera_coord2scr(v10);
			grDispatcher::instance()->line(v0.x, v0.y, v1.x, v1.y, _grid_color, 0);
		}
		/*
		        v0 = _camera->global2scr(Vect3f(-sz + dx,i,0));
		        v1 = _camera->global2scr(Vect3f(sz + dx,i,0));
		        grDispatcher::instance()->line(v0.x,v0.y,v1.x,v1.y,_grid_color,0);
		*/
	}
}

void qdAnimationSetPreview::set_camera_angle(int ang) {
	_camera_angle = ang;

	float angle = _animation_set->get_index_angle(_direction) * 180.0f / M_PI;
	_camera->rotate_and_scale(_camera_angle, 0, angle, 1.0f, 1.0f, 1.0f);
}

float qdAnimationSetPreview::camera_focus() {
	return _camera->get_focus();
}

void qdAnimationSetPreview::set_camera_focus(float f) {
	_camera->set_focus(f);
}

bool qdAnimationSetPreview::set_phase(float phase) {
	if (!_animation->is_empty()) {
		if (!_animation->is_playing())
			_animation->start();

		_cell_offset = 0.0f;
		_animation->set_time(0.0f);
		quant(_animation->length() * phase);
		return true;
	}

	return false;
}
} // namespace QDEngine
