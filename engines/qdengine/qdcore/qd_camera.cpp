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
#include "common/file.h"

#include "qdengine/qd_fwd.h"
#include "qdengine/parser/qdscr_parser.h"
#include "qdengine/parser/xml_tag_buffer.h"
#include "qdengine/system/graphics/gr_dispatcher.h"

#include "qdengine/qdcore/qd_setup.h"
#include "qdengine/qdcore/qd_camera.h"
#include "qdengine/qdcore/qd_game_object_animated.h"
#include "qdengine/qdcore/qd_game_dispatcher.h"


namespace QDEngine {


struct sPlane4f {
	float A, B, C, D;
	sPlane4f() { A = B = C = D = 0.0; }
	sPlane4f(float a, float b, float c, float d)               {
		A = a, B = b, C = c, D = d;
	}
	sPlane4f(const Vect3f &a, const Vect3f &b, const Vect3f &c) {
		// инициализация плоскости по трем точкам
		A = (b.y - a.y) * (c.z - a.z) - (c.y - a.y) * (b.z - a.z);
		B = (b.z - a.z) * (c.x - a.x) - (c.z - a.z) * (b.x - a.x);
		C = (b.x - a.x) * (c.y - a.y) - (c.x - a.x) * (b.y - a.y);
		GetNormal().normalize();
		D = -A * a.x - B * a.y - C * a.z;
	}
	inline void Set(const Vect3f &a, const Vect3f &b, const Vect3f &c) {
		// инициализация плоскости по трем точкам
		A = (b.y - a.y) * (c.z - a.z) - (c.y - a.y) * (b.z - a.z);
		B = (b.z - a.z) * (c.x - a.x) - (c.z - a.z) * (b.x - a.x);
		C = (b.x - a.x) * (c.y - a.y) - (c.x - a.x) * (b.y - a.y);
		GetNormal().normalize();
		D = -A * a.x - B * a.y - C * a.z;
	}
	inline float GetDistance(const Vect3f &a) {
		// расстояние от точки до плоскости
		float t = A * a.x + B * a.y + C * a.z + D;
		return t;
	}
	inline float GetCross(const Vect3f &a, const Vect3f &b) {
		// поиск пересечения данной плоскости с прямой заданной двумя точками a и b
		Vect3f v = a - b;
		float t = A * v.x + B * v.y + C * v.z;
		if (t == 0) return 0;       // прямая и плоскость параллельны
		t = (A * a.x + B * a.y + C * a.z + D) / t;
		return t;
	}
	inline Vect3f &GetNormal()                           {
		return *(Vect3f *)&A;
	}
	inline void GetReflectionVector(const Vect3f &in, Vect3f &out) {
		// out - поиск отражение вектора от плоскости
		out = in - 2 * dot(GetNormal(), in) * GetNormal();
	}
};

const int32 IMPASSIBLE_CELL_CLR     = 0x00FF0000;
const int32 SELECTED_CELL_CLR       = 0x000000FF;
const int32 NORMAL_CELL_CLR         = 0x00FFFFFF;

const int DASH_LEN = 2;

qdCamera *qdCamera::_current_camera = NULL;
#define CAMERA_WORLD_UP Vect3f(0, 1, 0)
#define CAMERA_AT_POINT Vect3f(0, 0, 0)  // Point we're looking at
const float qdCamera::_NEAR_PLANE = 1;
const float qdCamera::_FAR_PLANE = 10000;

//qdCameraMode qdCamera::_default_mode;

qdCamera::qdCamera() : _m_fR(300.0f), _xAngle(45), _yAngle(0), _zAngle(0),
	_GSX(0), _GSY(0), _grid(NULL),
	_cellSX(32), _cellSY(32), _focus(1000.0f),
	_gridCenter(0, 0, 0),
	_redraw_mode(QDCAM_GRID_ZBUFFER),
	_scrOffset(0, 0),
	_current_mode_work_time(0.0f),
	_current_mode_switch(false),
	_current_object(NULL),
	_default_object(NULL),
	_scale_pow(1.0f),
	_scale_z_offset(0.0f) {
	set_grid_size(50, 50);
	set_scr_size(640, 480);
	set_scr_center(320, 240);
	set_scr_center_initial(Vect2i(320, 240));

	rotate_and_scale(_xAngle, _yAngle, _zAngle, 1, 1, 1);

	_cycle_x = _cycle_y = false;
}

qdCamera::~qdCamera() {
	if (_GSX) {
		delete [] _grid;
	}
}

void qdCamera::set_grid_size(int xs, int ys) {
	if (_GSX == xs && _GSY == ys) return;

	if (_GSX)
		delete [] _grid;

	_grid = new sGridCell[xs * ys];

	_GSX = xs;
	_GSY = ys;
}

void qdCamera::clear_grid() {
	debugC(3, kDebugMovement, "qdCamera::clear_grid()");
	int cnt = 0;
	for (int i = 0; i < _GSY; i++) {
		for (int j = 0; j < _GSX; j++) {
			_grid[cnt++].clear();
		}
	}
}
float qdCamera::get_scale(const Vect3f &glCoord) const {
	if ((_focus < 5000.0f) || (fabs(_scale_pow - 1) > 0.001)) {
		Vect3f cameraCoord = global2camera_coord(glCoord);
		float buf = cameraCoord.z + _scale_z_offset;
		// Если координата отрицательна, то масштабирование происходит по линейному
		// закону. Иначе по общему (степенному) закону.
		if (buf > 0)
			buf = exp(_scale_pow * log(buf));

		float scale = (_focus / (buf + _focus));
		if (scale < 0)
			return 0;
		return scale;
	} else
		return 1.0f;
}

const Vect2s qdCamera::scr2rscr(const Vect2s &v) const {
	Vect2s res;
	res.x = v.x - (_scrCenter.x - _scrOffset.x);
	res.y = (_scrCenter.y - _scrOffset.y) - v.y;
	return res;
}

const Vect2s qdCamera::rscr2scr(const Vect2s &v) const {
	Vect2s res;
	res.x = _scrCenter.x + v.x - _scrOffset.x;
	res.y = _scrCenter.y - v.y - _scrOffset.y;
	return res;
}

const Vect3f qdCamera::camera_coord2global(const Vect3f &v) const {
	return TransformVector(v, MatrixInverse(_m_cam));
}

const Vect3f qdCamera::scr2global(const Vect2s &vScrPoint, float zInCameraCoord) const {
	return rscr2global(scr2rscr(vScrPoint), zInCameraCoord);
}

const Vect3f qdCamera::rscr2global(const Vect2s rScrPoint, const float zInCameraCoord) const {
	//Преобразование экран - координаты в системе камеры
	Vect3f _t = rscr2camera_coord(rScrPoint, zInCameraCoord);
	//Преобразование координаты в системе камеры - в координаты глобальные(в системе основной плоскости)
	return camera_coord2global(_t);
}

const Vect3f qdCamera::global2camera_coord(const Vect3f &glCoord) const {
	return TransformVector(glCoord, _m_cam);
}

const Vect3f qdCamera::rscr2camera_coord(const Vect2s &rScrPoint, float z) const {
	float x = ((float)rScrPoint.x * (z + _focus)) / _focus;
	float y = ((float)rScrPoint.y * (z + _focus)) / _focus;
	return Vect3f(x, y, z);
}

const Vect2s qdCamera::camera_coord2rscr(const Vect3f &coord) const {
	int16 sx = round(coord.x * _focus / (coord.z + _focus));
	int16 sy = round(coord.y * _focus / (coord.z + _focus));
	return Vect2s(sx, sy);
}

const Vect2s qdCamera::camera_coord2scr(const Vect3f &coord) const {
	return rscr2scr(camera_coord2rscr(coord));
}

const Vect2s qdCamera::global2scr(const Vect3f &glCoord) const {
	return camera_coord2scr(global2camera_coord(glCoord));
}

const Vect2s qdCamera::global2rscr(const Vect3f &glCoord) const {
	return camera_coord2rscr(global2camera_coord(glCoord));
}

void qdCamera::set_R(const float r) {
	_m_fR = r;
	rotate_and_scale(_xAngle, _yAngle, _zAngle, 1, 1, 1);
}

bool qdCamera::line_cutting(Vect3f &b, Vect3f &e) const {
	//положение по Z плоскости отсечения
	const float D = -_focus * .9f;
	if (b.z < D) { //первая лежит позади
		if (e.z < D) //обе точки лежат позади
			return false;
		float k = (D - b.z) / (e.z - b.z);
		b.z = D;
		b.y = k * (e.y - b.y) + b.y;
		b.x = k * (e.x - b.x) + b.x;
	} else if (e.z < D) {
		float k = (D - e.z) / (b.z - e.z);
		e.z = D;
		e.y = k * (b.y - e.y) + e.y;
		e.x = k * (b.x - e.x) + e.x;
	}
	return true;
}

void qdCamera::rotate_and_scale(float XA, float YA, float ZA, float kX, float kY, float kZ) {
	_xAngle = XA;
	_yAngle = YA;
	_zAngle = ZA;
	MATRIX3D rot = RotateXMatrix(XA * (M_PI / 180.f));
	rot = MatrixMult(RotateYMatrix(-YA * (M_PI / 180.f)), rot);
	rot = MatrixMult(RotateZMatrix(-ZA * (M_PI / 180.f)), rot);
	//точка, из которой мы сомотрим
	const Vect3f camPos(0, 0, _m_fR);
	//новая позиция камеры после поворота
	Vect3f pos = TransformVector(camPos, rot);

	//вычисляем, как измениться нормальный вектор камеры после поворота
	Vect3f new_up = TransformVector(CAMERA_WORLD_UP, rot);

	_m_cam = ViewMatrix(pos, CAMERA_AT_POINT, CAMERA_WORLD_UP, new_up);
}

const Vect3f qdCamera::rscr2plane_camera_coord(const Vect2s &scrPoint) const {
	const int XSP = _cellSX * _GSX;
	const int YSP = _cellSY * _GSY;
	const float XSP05 = XSP * 0.5f;
	const float YSP05 = YSP * 0.5f;
	Vect3f p0 = global2camera_coord(Vect3f(-XSP05, -YSP05, 0) + _gridCenter);
	Vect3f p1 = global2camera_coord(Vect3f(-XSP05, +YSP05, 0) + _gridCenter);
	Vect3f p2 = global2camera_coord(Vect3f(+XSP05, +YSP05, 0) + _gridCenter);

	sPlane4f plnT(p0, p1, p2);
	Vect3f tlV((float)scrPoint.x, (float)scrPoint.y, _focus);
	Vect3f tlP(0, 0, -_focus);
	float t = -(plnT.A * tlP.x + plnT.B * tlP.y + plnT.C * tlP.z + plnT.D) /
	          (plnT.A * tlV.x + plnT.B * tlV.y + plnT.C * tlV.z);

	float x = tlP.x + tlV.x * t;
	float y = tlP.y + tlV.y * t;
	float z = tlP.z + tlV.z * t;
	return Vect3f(x, y, z);
}

const Vect3f qdCamera::scr2plane_camera_coord(const Vect2s &scrPoint) const {
	return rscr2plane_camera_coord(scr2rscr(scrPoint));
}

const Vect3f qdCamera::scr2plane(const Vect2s &scrPoint) const {
	return camera_coord2global(scr2plane_camera_coord(scrPoint));
}

const Vect3f qdCamera::rscr2plane(const Vect2s &rscrPoint) const {
	return camera_coord2global(rscr2plane_camera_coord(rscrPoint));
}

const Vect2s qdCamera::plane2scr(const Vect3f &plnPoint) const {
	return rscr2scr(plane2rscr(plnPoint));
}

const Vect2s qdCamera::plane2rscr(const Vect3f &plnPoint) const {
	const float SMALL_VALUE = 0.0001f;

	Vect3f res = global2camera_coord(plnPoint);

	if (res.z < (SMALL_VALUE - _focus)) return Vect2s(0, 0);

	int sx0 = round(res.x * _focus / (res.z + _focus));
	int sy0 = round(res.y * _focus / (res.z + _focus));

	return Vect2s(sx0, sy0);
}

const sGridCell *qdCamera::get_cell(float X, float Y) const {
	int x = round(X - _gridCenter.x);
	int y = round(Y - _gridCenter.y);

	const int XSP = _cellSX * _GSX;
	const int YSP = _cellSY * _GSY;
	const int XSP05 = XSP / 2;
	const int YSP05 = YSP / 2;

	x += XSP05;
	y += YSP05;
	if (x < 0 || x >= XSP || y < 0 || y >= YSP) return 0;
	x = x / _cellSX;
	y = y / _cellSY;
	return &_grid[y * _GSX + x];
}

const Vect2s qdCamera::get_cell_index(float X, float Y, bool grid_crop) const {
	int x = round(X - _gridCenter.x);
	int y = round(Y - _gridCenter.y);

	const int XSP = _cellSX * _GSX;
	const int YSP = _cellSY * _GSY;
	const int XSP05 = XSP >> 1;
	const int YSP05 = YSP >> 1;
	x += XSP05;
	y += YSP05;

	if (grid_crop && (x < 0 || x >= XSP || y < 0 || y >= YSP))
		return Vect2s(-1, -1);

	return Vect2s(x / _cellSX, y / _cellSY);
}

const Vect2s qdCamera::get_cell_index(const Vect3f &v, bool grid_crop) const {
	return get_cell_index(v.x, v.y, grid_crop);
}

const Vect3f qdCamera::get_cell_coords(int x_idx, int y_idx) const {
	//float xx = (x_idx - (_GSX>>1)) * _cellSX + (_cellSX>>1) + _gridCenter.x;
	//float yy = (y_idx - (_GSY>>1)) * _cellSY + (_cellSY>>1) + _gridCenter.y;

	float xx = (x_idx - static_cast<float>(_GSX) / 2 + 0.5) * _cellSX + _gridCenter.x;
	float yy = (y_idx - static_cast<float>(_GSY) / 2 + 0.5) * _cellSY + _gridCenter.y;

	return Vect3f(xx, yy, _gridCenter.z);
}

const Vect3f qdCamera::get_cell_coords(const Vect2s &idxs) const {
	return get_cell_coords(idxs.x, idxs.y);
}

void qdCamera::reset_all_select() {
	int cnt = 0;
	for (int i = 0; i < _GSY; i++) {
		for (int j = 0; j < _GSX; j++) {
			_grid[cnt++].deselect();
		}
	}
}

bool qdCamera::select_cell(int x, int y) {
	const int XSP = _cellSX * _GSX;
	const int YSP = _cellSY * _GSY;
	const int XSP05 = XSP >> 1;
	const int YSP05 = YSP >> 1;

	x += XSP05 - _gridCenter.x;
	y += YSP05 - _gridCenter.y;

	if (x < 0 || x >= XSP || y < 0 || y >= YSP) return false;
	x = x / _cellSX;
	y = y / _cellSY;
	_grid[y * _GSX + x].select();
	return true;
}

bool qdCamera::deselect_cell(int x, int y) {
	const int XSP = _cellSX * _GSX;
	const int YSP = _cellSY * _GSY;
	const int XSP05 = XSP >> 1;
	const int YSP05 = YSP >> 1;

	x += XSP05 - _gridCenter.x;
	y += YSP05 - _gridCenter.y;

	if (x < 0 || x >= XSP || y < 0 || y >= YSP) return false;
	x = x / _cellSX;
	y = y / _cellSY;
	_grid[y * _GSX + x].deselect();
	return true;
}

void qdCamera::load_script(const xml::tag *p) {
	int x, y;
	for (xml::tag::subtag_iterator it = p->subtags_begin(); it != p->subtags_end(); ++it) {
		xml::tag_buffer buf(*it);

		switch (it->ID()) {
		case QDSCR_CAMERA_GRID_SIZE:
			buf > x > y;
			set_grid_size(x, y);
			break;
		case QDSCR_CAMERA_CELL_SIZE:
			buf > _cellSX > _cellSY;
			break;
		case QDSCR_CAMERA_SCREEN_SIZE:
			buf > x > y;
			set_scr_size(x, y);
			set_scr_center(x / 2, y / 2);
			set_scr_center_initial(Vect2i(x / 2, y / 2));
			break;
		case QDSCR_CAMERA_SCREEN_OFFSET:
			buf > x > y;
			set_scr_offset(Vect2i(x, y));
			break;
		case QDSCR_CAMERA_SCREEN_CENTER:
			buf > x > y;
			set_scr_center_initial(Vect2i(x, y));
			set_scr_center(x, y);
			break;
		case QDSCR_CAMERA_FOCUS:
			buf > _focus;
			break;
		case QDSCR_CAMERA_ANGLES:
			buf > _xAngle > _yAngle > _zAngle;
			rotate_and_scale(_xAngle, _yAngle, _zAngle, 1, 1, 1);
			break;
		case QDSCR_CAMERA_GRID_CENTER: {
			Vect3f v;
			buf > v.x > v.y > v.z;
			set_grid_center(v);
		}
		break;
		case QDSCR_POS3D:
			buf.get_float();
			buf.get_float();
			set_R(buf.get_float());
			break;
		case QDSCR_CAMERA_SCALE_POW:
			buf > _scale_pow;
			break;
		case QDSCR_CAMERA_SCALE_Z_OFFSET:
			buf > _scale_z_offset;
			break;
		}
	}

	rotate_and_scale(_xAngle, _yAngle, _zAngle, 1, 1, 1);
}

bool qdCamera::save_script(Common::WriteStream &fh, int indent) const {
	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}

	fh.writeString("<camera");

	fh.writeString(Common::String::format(" camera_grid_size=\"%d %d\"", _GSX, _GSY));

	fh.writeString(">\r\n");

	for (int i = 0; i <= indent; i++) {
		fh.writeString("\t");
	}
	fh.writeString(Common::String::format("<camera_cell_size>%d %d</camera_cell_size>\r\n", _cellSX, _cellSY));

	for (int i = 0; i <= indent; i++) {
		fh.writeString("\t");
	}
	fh.writeString(Common::String::format("<pos_3d>%ld %ld %f</pos_3d>\r\n", 0L, 0L, get_R()));

	for (int i = 0; i <= indent; i++) {
		fh.writeString("\t");
	}
	fh.writeString(Common::String::format("<camera_focus>%f</camera_focus>\r\n", _focus));

	for (int i = 0; i <= indent; i++) {
		fh.writeString("\t");
	}
	fh.writeString(Common::String::format("<camera_angles>%f %f %f</camera_angles>\r\n", _xAngle, _yAngle, _zAngle));

	for (int i = 0; i <= indent; i++) {
		fh.writeString("\t");
	}
	fh.writeString(Common::String::format("<camera_screen_size>%d %d</camera_screen_size>\r\n", _scrSize.x, _scrSize.y));

	for (int i = 0; i <= indent; i++) {
		fh.writeString("\t");
	}
	fh.writeString(Common::String::format("<camera_screen_offset>%d %d</camera_screen_offset>\r\n", _scrOffset.x, _scrOffset.y));

	for (int i = 0; i <= indent; i++) {
		fh.writeString("\t");
	}
	fh.writeString(Common::String::format("<camera_screen_center>%d %d</camera_screen_center>\r\n", _scrCenterInitial.x, _scrCenterInitial.y));

	for (int i = 0; i <= indent; i++) {
		fh.writeString("\t");
	}
	fh.writeString(Common::String::format("<camera_grid_center>%f %f %f</camera_grid_center>\r\n", _gridCenter.x, _gridCenter.y, _gridCenter.z));

	for (int i = 0; i <= indent; i++) {
		fh.writeString("\t");
	}
	fh.writeString(Common::String::format("<camera_scale_pow>%f</camera_scale_pow>\r\n", scale_pow()));

	for (int i = 0; i <= indent; i++) {
		fh.writeString("\t");
	}
	fh.writeString(Common::String::format("<camera_scale_z_offset>%f</camera_scale_z_offset>\r\n", scale_z_offset()));

	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}
	fh.writeString("</camera>\r\n");

	return true;
}

const Vect2i qdCamera::screen_center_limit_x() const {
	int x0, x1;
	if (_scrSize.x < g_engine->_screenW) {
		x0 = x1 = g_engine->_screenW / 2;
	} else {
		x0 = -_scrSize.x / 2 + g_engine->_screenW;
		x1 = _scrSize.x / 2;
	}

	if (_cycle_x) {
		x0 -= _scrSize.x;
		x1 += _scrSize.x;
	}

	return Vect2i(x0, x1);
}

const Vect2i qdCamera::screen_center_limit_y() const {
	int y0, y1;
	if (_scrSize.y < g_engine->_screenH) {
		y0 = y1 = g_engine->_screenH / 2;
	} else {
		y0 = -_scrSize.y / 2 + g_engine->_screenH;
		y1 = _scrSize.y / 2;
	}

	if (_cycle_y) {
		y0 -= _scrSize.y;
		y1 += _scrSize.y;
	}

	return Vect2i(y0, y1);
}

void qdCamera::move_scr_center(int dxc, int dyc) {
	_scrCenter.x += dxc;
	_scrCenter.y += dyc;

	clip_center_coords(_scrCenter.x, _scrCenter.y);
}

float qdCamera::scrolling_phase_x() const {
	if (_scrSize.x <= g_engine->_screenW)
		return 0.0f;
	else
		return float(_scrCenter.x * 2 + _scrSize.x - g_engine->_screenW * 2) / float(_scrSize.x - g_engine->_screenW) - 1.0f;
}

float qdCamera::scrolling_phase_y() const {
	if (_scrSize.y <= g_engine->_screenH)
		return 0.0f;
	else
		return float(_scrCenter.y * 2 + _scrSize.y - g_engine->_screenH * 2) / float(_scrSize.x - g_engine->_screenH) - 1.0f;
}

bool qdCamera::draw_grid() const {
	if (_redraw_mode == QDCAM_GRID_NONE) return true;

	int cnt = 0;

	const int XSP = _cellSX * _GSX;
	const int YSP = _cellSY * _GSY;
	const float XSP05 = XSP / 2.f;
	const float YSP05 = YSP / 2.f;

	for (int i = 0; i < _GSY;++i) {
		for (int j = 0; j < _GSX;++j) {
			if (!_grid[cnt].is_walkable())
				draw_cell(j, i, 0, 1, IMPASSIBLE_CELL_CLR);

			if (_grid[cnt].is_selected() || _grid[cnt].check_attribute(sGridCell::CELL_OCCUPIED | sGridCell::CELL_PERSONAGE_OCCUPIED))
				draw_cell(j, i, 0, 1, SELECTED_CELL_CLR);
			++cnt;
		}
	}

	if (_redraw_mode == QDCAM_GRID_ZBUFFER) {
		for (int i = 0; i <= _GSX; i++) {
			for (int j = 0; j < _GSY; j++) {
				Vect3f begPoint(-XSP05 + i * _cellSX, -YSP05 + j * _cellSY, 0);
				Vect3f endPoint(-XSP05 + i * _cellSX, -YSP05 + (j + 1) *_cellSY, 0);
				begPoint = global2camera_coord(begPoint + _gridCenter);
				endPoint = global2camera_coord(endPoint + _gridCenter);
				if (line_cutting(begPoint, endPoint)) {
					Vect2s b = camera_coord2scr(begPoint);
					Vect2s e = camera_coord2scr(endPoint);

#ifdef _GR_ENABLE_ZBUFFER
					grDispatcher::instance()->Line_z(b.x, b.y, begPoint.z, e.x, e.y, endPoint.z, NORMAL_CELL_CLR, DASH_LEN);
#else
					grDispatcher::instance()->line(b.x, b.y, e.x, e.y, NORMAL_CELL_CLR, DASH_LEN);
#endif
				}
			}
		}
		for (int i = 0; i <= _GSY; i++) {
			for (int j = 0; j < _GSX; j++) {
				Vect3f begPoint(-XSP05 + j * _cellSX, -YSP05 + i * _cellSY, 0);
				Vect3f endPoint(-XSP05 + (j + 1)*_cellSX, -YSP05 + i * _cellSY, 0);
				begPoint = global2camera_coord(begPoint + _gridCenter);
				endPoint = global2camera_coord(endPoint + _gridCenter);
				if (line_cutting(begPoint, endPoint)) {
					Vect2s b = camera_coord2scr(begPoint);
					Vect2s e = camera_coord2scr(endPoint);

#ifdef _GR_ENABLE_ZBUFFER
					grDispatcher::instance()->line_z(b.x, b.y, begPoint.z, e.x, e.y, endPoint.z, NORMAL_CELL_CLR, DASH_LEN);
#else
					grDispatcher::instance()->line(b.x, b.y, e.x, e.y, NORMAL_CELL_CLR, DASH_LEN);
#endif
				}
			}
		}
	} else {
		for (int i = 0; i <= _GSX; i++) {
			Vect3f begPoint(-XSP05 + i * _cellSX, -YSP05, 0);
			Vect3f endPoint(-XSP05 + i * _cellSX, +YSP05, 0);
			begPoint = global2camera_coord(begPoint + _gridCenter);
			endPoint = global2camera_coord(endPoint + _gridCenter);
			if (line_cutting(begPoint, endPoint)) {
				Vect2s b = camera_coord2scr(begPoint);
				Vect2s e = camera_coord2scr(endPoint);

				grDispatcher::instance()->line(b.x, b.y, e.x, e.y, NORMAL_CELL_CLR, DASH_LEN);
			}
		}

		for (int i = 0; i <= _GSY; i++) {
			Vect3f begPoint(-XSP05, -YSP05 + i * _cellSY, 0);
			Vect3f endPoint(+XSP05, -YSP05 + i * _cellSY, 0);
			begPoint = global2camera_coord(begPoint + _gridCenter);
			endPoint = global2camera_coord(endPoint + _gridCenter);
			if (line_cutting(begPoint, endPoint)) {
				Vect2s b = camera_coord2scr(begPoint);
				Vect2s e = camera_coord2scr(endPoint);

				grDispatcher::instance()->line(b.x, b.y, e.x, e.y, NORMAL_CELL_CLR, DASH_LEN);
			}
		}
	}

	cnt = 0;
	uint32 color;
	for (int i = 0; i < _GSY; i++) {
		for (int j = 0; j < _GSX; j++, cnt++) {
			if (_grid[cnt].height() || _grid[cnt].attributes()) {
				color = 0;
				if (_grid[cnt].attributes() & sGridCell::CELL_IMPASSABLE)
					color = grDispatcher::make_rgb565u(0xff, 0x00, 0x00);
				else if (_grid[cnt].attributes() & sGridCell::CELL_PERSONAGE_OCCUPIED)
					color = grDispatcher::make_rgb565u(0xff, 0xff, 0x00);
				else if (_grid[cnt].height())
					color = grDispatcher::make_rgb565u(0xff, 0xff, 0xff);

				if (_grid[cnt].attributes() & sGridCell::CELL_OCCUPIED)
					color = grDispatcher::make_rgb565u(0x00, 0x00, 0xff);
				if (_grid[cnt].attributes() & sGridCell::CELL_PERSONAGE_PATH)
					warning("path");

				if (color)
					draw_cell(j, i, _grid[cnt].height(), 1, color);
			}
		}
	}

	return true;
}

bool qdCamera::draw_cell(int x, int y, int z, int penWidth, uint32 color) const {
	const int XSP = get_cell_sx() * get_grid_sx();
	const int YSP = get_cell_sy() * get_grid_sy();
	const float XSP05 = XSP * 0.5f;
	const float YSP05 = YSP * 0.5f;
	const int offset = 2;

	Vect3f point0((float)(x * get_cell_sx() - XSP05 + offset), (float)(y * get_cell_sy() - YSP05 + offset), (float)z);
	Vect3f point1((float)((x + 1)*get_cell_sx() - XSP05 - offset), (float)(y * get_cell_sy() - YSP05 + offset), (float)z);
	Vect3f point2((float)((x + 1)*get_cell_sx() - XSP05 - offset), (float)((y + 1)*get_cell_sy() - YSP05 - offset), (float)z);
	Vect3f point3((float)(x * get_cell_sx() - XSP05 + offset), (float)((y + 1)*get_cell_sy() - YSP05 - offset), (float)z);

	point0 = global2camera_coord(point0 + _gridCenter);
	point1 = global2camera_coord(point1 + _gridCenter);
	point2 = global2camera_coord(point2 + _gridCenter);
	point3 = global2camera_coord(point3 + _gridCenter);

	if (_redraw_mode == QDCAM_GRID_ZBUFFER) {
		if (line_cutting(point0, point1)) {
			Vect2s p0 = camera_coord2scr(point0);
			Vect2s p1 = camera_coord2scr(point1);

#ifdef _GR_ENABLE_ZBUFFER
			grDispatcher::instance()->Line_z(p0.x, p0.y, point0.z, p1.x, p1.y, point1.z, color/*, DASH_LEN*/);
#else
			grDispatcher::instance()->line(p0.x, p0.y, p1.x, p1.y, color/*, DASH_LEN*/);
#endif
		}

		if (line_cutting(point1, point2)) {
			Vect2s p0 = camera_coord2scr(point1);
			Vect2s p1 = camera_coord2scr(point2);

#ifdef _GR_ENABLE_ZBUFFER
			grDispatcher::instance()->Line_z(p0.x, p0.y, point1.z, p1.x, p1.y, point2.z, color/*, DASH_LEN*/);
#else
			grDispatcher::instance()->line(p0.x, p0.y, p1.x, p1.y, color/*, DASH_LEN*/);
#endif
		}

		if (line_cutting(point2, point3)) {
			Vect2s p0 = camera_coord2scr(point2);
			Vect2s p1 = camera_coord2scr(point3);

#ifdef _GR_ENABLE_ZBUFFER
			grDispatcher::instance()->Line_z(p0.x, p0.y, point2.z, p1.x, p1.y, point3.z, color/*, DASH_LEN*/);
#else
			grDispatcher::instance()->line(p0.x, p0.y, p1.x, p1.y, color/*, DASH_LEN*/);
#endif
		}

		if (line_cutting(point3, point0)) {
			Vect2s p0 = camera_coord2scr(point3);
			Vect2s p1 = camera_coord2scr(point0);

#ifdef _GR_ENABLE_ZBUFFER
			grDispatcher::instance()->Line_z(p0.x, p0.y, point3.z, p1.x, p1.y, point0.z, color/*, DASH_LEN*/);
#else
			grDispatcher::instance()->line(p0.x, p0.y, p1.x, p1.y, color/*, DASH_LEN*/);
#endif
		}
	} else {
		if (line_cutting(point0, point1)) {
			Vect2s p0 = camera_coord2scr(point0);
			Vect2s p1 = camera_coord2scr(point1);
			grDispatcher::instance()->line(p0.x, p0.y, p1.x, p1.y, color/*, DASH_LEN*/);
		}

		if (line_cutting(point1, point2)) {
			Vect2s p0 = camera_coord2scr(point1);
			Vect2s p1 = camera_coord2scr(point2);
			grDispatcher::instance()->line(p0.x, p0.y, p1.x, p1.y, color/*, DASH_LEN*/);
		}

		if (line_cutting(point2, point3)) {
			Vect2s p0 = camera_coord2scr(point2);
			Vect2s p1 = camera_coord2scr(point3);
			grDispatcher::instance()->line(p0.x, p0.y, p1.x, p1.y, color/*, DASH_LEN*/);
		}

		if (line_cutting(point3, point0)) {
			Vect2s p0 = camera_coord2scr(point3);
			Vect2s p1 = camera_coord2scr(point0);
			grDispatcher::instance()->line(p0.x, p0.y, p1.x, p1.y, color/*, DASH_LEN*/);
		}
	}
	return true;
}

void qdCamera::scale_grid(int sx, int sy, int csx, int csy) {
	if (_GSX == sx && _GSY == sy) return;

	sGridCell *new_grid = new sGridCell[sx * sy];

	if (_GSX) {
		if (_GSX >= sx && _GSY >= sy) {
			int dx = _GSX / sx;
			int dy = _GSY / sy;

			sGridCell *new_p = new_grid;
			sGridCell *old_p = _grid;

			for (int i = 0; i < sy; i++) {
				for (int j = 0; j < sx; j++) {
					int attr_count = 0;
					int height_sum = 0;

					for (int y = 0; y < dy; y++) {
						for (int x = 0; x < dx; x++) {
							if (!old_p[j * dx + x + y * _GSX].is_walkable())
								attr_count++;

							height_sum += old_p[j * dx + x + y * _GSX].height();
						}
					}
					if (attr_count >= dx * dy / 2)
						new_p->make_impassable();

					new_p->set_height(height_sum / (dx * dy));

					new_p++;
				}
				old_p += _GSX * dy;
			}
		}
		if (_GSX <= sx && _GSY <= sy) {
			int dx = sx / _GSX;
			int dy = sy / _GSY;

			sGridCell *new_p = new_grid;
			sGridCell *old_p = _grid;

			for (int i = 0; i < _GSY; i++) {
				for (int j = 0; j < _GSX; j++) {
					for (int y = 0; y < dy; y++) {
						for (int x = 0; x < dx; x++)
							new_p[j * dx + x + y * sx] = *old_p;
					}
					old_p++;
				}
				new_p += sx * dy;
			}
		}

		delete [] _grid;
	}

	_grid = new_grid;

	_GSX = sx;
	_GSY = sy;

	_cellSX = csx;
	_cellSY = csy;
}

void qdCamera::resize_grid(int sx, int sy) {
	if (_GSX == sx && _GSY == sy) return;

	sGridCell *new_grid = new sGridCell[sx * sy];

	if (_GSX) {
		int x0 = (sx - _GSX) / 2;
		int y0 = (sy - _GSY) / 2;

		for (int y = 0; y < _GSY; y++) {
			for (int x = 0; x < _GSX; x++) {
				if (x + x0 >= 0 && x + x0 < sx && y + y0 >= 0 && y + y0 < sy)
					new_grid[x + x0 + (y + y0) * sx] = _grid[x + y * _GSX];
			}
		}

		delete [] _grid;
	}

	_grid = new_grid;

	_GSX = sx;
	_GSY = sy;
}

bool qdCamera::set_grid_cell(const Vect2s &cell_pos, const sGridCell &cell) {
	if (cell_pos.x >= 0 && cell_pos.x < _GSX && cell_pos.y >= 0 && cell_pos.y < _GSY) {
		_grid[cell_pos.x + cell_pos.y * _GSX] = cell;
		return true;
	}

	return false;
}

bool qdCamera::set_grid_cell_attributes(const Vect2s &cell_pos, int attr) {
	if (cell_pos.x >= 0 && cell_pos.x < _GSX && cell_pos.y >= 0 && cell_pos.y < _GSY) {
		_grid[cell_pos.x + cell_pos.y * _GSX].set_attributes(attr);
		return true;
	}

	return false;
}

bool qdCamera::restore_grid_cell(const Vect2s cell_pos) {
	if (cell_pos.x >= 0 && cell_pos.x < _GSX && cell_pos.y >= 0 && cell_pos.y < _GSY) {
		sGridCell cl;
		cl.make_impassable();
		_grid[cell_pos.x + cell_pos.y * _GSX] = cl;
		return true;
	}

	return false;
}

sGridCell *qdCamera::get_cell(const Vect2s &cell_pos) {
	if (cell_pos.x >= 0 && cell_pos.x < _GSX && cell_pos.y >= 0 && cell_pos.y < _GSY) {
		return &_grid[cell_pos.x + cell_pos.y * _GSX];
	}
	return NULL;
}

const sGridCell *qdCamera::get_cell(const Vect2s &cell_pos) const {
	if (cell_pos.x >= 0 && cell_pos.x < _GSX && cell_pos.y >= 0 && cell_pos.y < _GSY) {
		return &_grid[cell_pos.x + cell_pos.y * _GSX];
	}
	return NULL;
}

bool qdCamera::load_data(Common::SeekableReadStream &fh, int save_version) {
	debugC(3, kDebugSave, "  qdCamera::load_data(): before: %d", (int)fh.pos());

	int x, y;
	char flag;
	_scrCenter.x = fh.readSint32LE();
	_scrCenter.y = fh.readSint32LE();
	x = fh.readSint32LE();
	y = fh.readSint32LE();
	_current_mode_work_time = fh.readFloatLE();
	flag = fh.readByte();
	_current_mode_switch = bool(flag);

	if (x != _GSX || y != _GSY) return false;

	if (!_current_mode.load_data(fh, save_version))
		return false;
	if (!_default_mode.load_data(fh, save_version))
		return false;

	flag = fh.readByte();
	if (flag) {
		qdNamedObjectReference ref;
		if (!ref.load_data(fh, save_version))
			return false;
		_current_object = dynamic_cast<qdGameObjectAnimated *>(qdGameDispatcher::get_dispatcher()->get_named_object(&ref));
	}

	flag = fh.readByte();
	if (flag) {
		qdNamedObjectReference ref;
		if (!ref.load_data(fh, save_version))
			return false;
		_default_object = dynamic_cast<qdGameObjectAnimated *>(qdGameDispatcher::get_dispatcher()->get_named_object(&ref));
	}

	debugC(3, kDebugSave, "  qdCamera::load_data(): after: %d", (int)fh.pos());
	return true;
}

bool qdCamera::save_data(Common::WriteStream &fh) const {
	debugC(3, kDebugSave, "  qdCamera::save_data(): before: %d", (int)fh.pos());
	fh.writeSint32LE(_scrCenter.x);
	fh.writeSint32LE(_scrCenter.y);
	fh.writeSint32LE(_GSX);
	fh.writeSint32LE(_GSY);
	fh.writeFloatLE(_current_mode_work_time);
	fh.writeByte(char(_current_mode_switch));

	_current_mode.save_data(fh);
	_default_mode.save_data(fh);

	if (_current_object) {
		fh.writeByte(char(1));
		qdNamedObjectReference ref(_current_object);
		ref.save_data(fh);
	} else {
		fh.writeByte(char(0));
	}

	if (_default_object) {
		fh.writeByte(char(1));
		qdNamedObjectReference ref(_default_object);
		ref.save_data(fh);
	} else {
		fh.writeByte(char(0));
	}

	debugC(3, kDebugSave, "  qdCamera::save_data(): after: %d", (int)fh.pos());
	return true;
}

bool qdCamera::set_mode(const qdCameraMode &mode, qdGameObjectAnimated *object) {
	_current_mode = mode;
	_current_object = object;

	_current_mode_work_time = 0.0f;
	_current_mode_switch = _current_mode.smooth_switch();

	return true;
}

bool qdCamera::quant(float dt) {
	Vect2i last_pos = _scrCenter;

	qdGameObjectAnimated *p = _current_object;
	if (!p) p = _default_object;

	if (p)
		p->qdGameObject::update_screen_pos();

	switch (_current_mode.camera_mode()) {
	case qdCameraMode::MODE_CENTER_OBJECT:
		if (p) {
			Vect2i r = p->screen_pos() + _current_mode.center_offset();

			int cx = _scrCenter.x + g_engine->_screenW / 2 - r.x;
			int cy = _scrCenter.y + g_engine->_screenH / 2 - r.y;

			clip_center_coords(cx, cy);

			int dx = cx - _scrCenter.x;
			int dy = cy - _scrCenter.y;

			if (_current_mode_switch) {
				Vect2f dr(dx, dy);

				float dr0 = _current_mode.scrolling_speed() * dt;

				if (dr.norm2() > dr0 * dr0)
					dr.normalize(dr0);
				else
					_current_mode_switch = false;

				move_scr_center(dr.xi(), dr.yi());
			} else
				move_scr_center(dx, dy);
		}
		break;
	case qdCameraMode::MODE_OBJECT_ON_SCREEN:
		if (p) {
			Vect2s r = p->screen_pos();
			float sz = p->radius();

			int dx = 0;
			int dy = 0;
			if (r.x + sz + _current_mode.scrolling_distance() >= g_engine->_screenW) {
				dx = g_engine->_screenW - (r.x + sz + _current_mode.scrolling_distance());
			} else {
				if (r.x - sz - _current_mode.scrolling_distance() < 0)
					dx = -r.x + sz + _current_mode.scrolling_distance();
			}

			if (r.y + sz + _current_mode.scrolling_distance() >= g_engine->_screenH) {
				dy = g_engine->_screenH - (r.y + sz + _current_mode.scrolling_distance());
			} else {
				if (r.y - sz - _current_mode.scrolling_distance() < 0)
					dy = -r.y + sz + _current_mode.scrolling_distance();
			}

			if (_current_mode_switch) {
				int cx = _scrCenter.x + dx;
				int cy = _scrCenter.y + dy;

				clip_center_coords(cx, cy);

				dx = cx - _scrCenter.x;
				dy = cy - _scrCenter.y;

				Vect2f dr(dx, dy);

				float dr0 = _current_mode.scrolling_speed() * dt;

				if (dr.norm2() > dr0 * dr0)
					dr.normalize(dr0);
				else
					_current_mode_switch = false;

				move_scr_center(dr.xi(), dr.yi());
			} else
				move_scr_center(dx, dy);
		}
		break;
	case qdCameraMode::MODE_FOLLOW_OBJECT:
		if (p) {
			Vect2s r = p->screen_pos() + _current_mode.center_offset();
			int dx = -r.x + g_engine->_screenW / 2;
			int dy = -r.y + g_engine->_screenH / 2;

			if (dx || dy) {
				Vect2f dr(dx, dy);

				float dr0 = _current_mode.scrolling_speed() * dt;

				if (dr.norm2() > dr0 * dr0)
					dr.normalize(dr0);

				move_scr_center(dr.xi(), dr.yi());
			}
			_current_mode_switch = false;
		}
		break;
	case qdCameraMode::MODE_CENTER_OBJECT_WHEN_LEAVING:
		if (p) {
			Vect2s r = p->screen_pos() + _current_mode.center_offset();
			float sz = p->radius();

			int dx = 0;
			int dy = 0;
			if (r.x + sz + _current_mode.scrolling_distance() >= g_engine->_screenW) {
				dx = g_engine->_screenW - (r.x + sz + _current_mode.scrolling_distance());
			} else {
				if (r.x - sz - _current_mode.scrolling_distance() < 0)
					dx = -r.x + sz + _current_mode.scrolling_distance();
			}

			if (r.y + sz + _current_mode.scrolling_distance() >= g_engine->_screenH) {
				dy = g_engine->_screenH - (r.y + sz + _current_mode.scrolling_distance());
			} else {
				if (r.y - sz - _current_mode.scrolling_distance() < 0)
					dy = -r.y + sz + _current_mode.scrolling_distance();
			}

			if (dx || dy) {
				int dx1 = -r.x + g_engine->_screenW / 2;
				int dy1 = -r.y + g_engine->_screenH / 2;

				Vect2f dr(dx1, dy1);

				float dr0 = _current_mode.scrolling_speed() * dt;

				if (dr.norm2() > dr0 * dr0)
					dr.normalize(dr0);

				move_scr_center(dr.xi(), dr.yi());
			}

			_current_mode_switch = false;
		}
		break;
	default:
		break;
	}

	if (p)
		p->update_screen_pos();

	clip_center_coords(_scrCenter.x, _scrCenter.y);

	if (!_current_mode_switch)
		_current_mode_work_time += dt;

	if (_current_mode.has_work_time() && _current_mode_work_time > _current_mode.work_time())
		set_mode(_default_mode, _default_object);

	if (last_pos.x != _scrCenter.x || last_pos.y != _scrCenter.y)
		return true;

	return false;
}

bool qdCamera::set_grid_attributes(const Vect2s &center_pos, const Vect2s &size, int attr) {
	int x0 = center_pos.x - size.x / 2;
	int y0 = center_pos.y - size.y / 2;

	int x1 = x0 + size.x;
	int y1 = y0 + size.y;

	if (x0 < 0) x0 = 0;
	if (x1 > _GSX - 1) x1 = _GSX - 1;
	if (y0 < 0) y0 = 0;
	if (y1 > _GSY - 1) y1 = _GSY - 1;

	sGridCell *cells = _grid + x0 + y0 * _GSX;

	debugC(4, kDebugMovement, "qdCamera::set_grid_attributes() attr: %d at [%d, %d]", attr, x0, y0);
	for (int y = y0; y < y1; y++) {
		sGridCell *p = cells;
		for (int x = x0; x < x1; x++, p++)
			p->set_attribute(attr);

		cells += _GSX;
	}

	return true;
}

bool qdCamera::drop_grid_attributes(const Vect2s &center_pos, const Vect2s &size, int attr) {
	int x0 = center_pos.x - size.x / 2;
	int y0 = center_pos.y - size.y / 2;

	int x1 = x0 + size.x;
	int y1 = y0 + size.y;

	if (x0 < 0) x0 = 0;
	if (x1 > _GSX - 1) x1 = _GSX - 1;
	if (y0 < 0) y0 = 0;
	if (y1 > _GSY - 1) y1 = _GSY - 1;

	sGridCell *cells = _grid + x0 + y0 * _GSX;

	for (int y = y0; y < y1; y++) {
		sGridCell *p = cells;
		for (int x = x0; x < x1; x++, p++)
			p->drop_attribute(attr);

		cells += _GSX;
	}

	return true;
}

bool qdCamera::set_grid_attributes(int attr) {
	sGridCell *p = _grid;
	for (int i = 0; i < _GSX * _GSY; i++, p++)
		p->set_attribute(attr);

	return true;
}

bool qdCamera::drop_grid_attributes(int attr) {
	sGridCell *p = _grid;
	for (int i = 0; i < _GSX * _GSY; i++, p++)
		p->drop_attribute(attr);

	return true;
}

bool qdCamera::check_grid_attributes(const Vect2s &center_pos, const Vect2s &size, int attr) const {
	int x0 = center_pos.x - size.x / 2;
	int y0 = center_pos.y - size.y / 2;

	int x1 = x0 + size.x;
	int y1 = y0 + size.y;

	if (x0 < 0) x0 = 0;
	if (x1 > _GSX - 1) x1 = _GSX - 1;
	if (y0 < 0) y0 = 0;
	if (y1 > _GSY - 1) y1 = _GSY - 1;

	const sGridCell *cells = _grid + x0 + y0 * _GSX;

	for (int y = y0; y < y1; y++) {
		const sGridCell *p = cells;
		for (int x = x0; x < x1; x++, p++) {
			if (p->check_attribute(attr))
				return true;
		}

		cells += _GSX;
	}

	return false;
}

int qdCamera::cells_num_with_exact_attributes(const Vect2s &center_pos, const Vect2s &size, int attr) const {
	int x0 = center_pos.x - size.x / 2;
	int y0 = center_pos.y - size.y / 2;

	int x1 = x0 + size.x;
	int y1 = y0 + size.y;

	if (x0 < 0) x0 = 0;
	if (x1 > _GSX - 1) x1 = _GSX - 1;
	if (y0 < 0) y0 = 0;
	if (y1 > _GSY - 1) y1 = _GSY - 1;

	const sGridCell *cells = _grid + x0 + y0 * _GSX;

	int ret = 0;
	for (int y = y0; y < y1; y++) {
		const sGridCell *p = cells;
		for (int x = x0; x < x1; x++, p++) {
			if (p->attributes() == (uint)attr)
				ret++;
		}

		cells += _GSX;
	}

	return ret;
}

bool qdCamera::is_walkable(const Vect2s &center_pos, const Vect2s &size, bool ignore_personages) const {
	int x0 = center_pos.x - size.x / 2;
	int y0 = center_pos.y - size.y / 2;

	int x1 = x0 + size.x;
	int y1 = y0 + size.y;

	if (x0 < 0) x0 = 0;
	if (x1 > _GSX - 1) x1 = _GSX - 1;
	if (y0 < 0) y0 = 0;
	if (y1 > _GSY - 1) y1 = _GSY - 1;

	const sGridCell *cells = _grid + x0 + y0 * _GSX;
	debugC(3, kDebugMovement, "qdCamera::is_walkable(): attr: %d [%d, %d] size: [%d, %d], ignore_personages: %d", cells->attributes(), x0, y0, size.x, size.y, ignore_personages);

	int attr = sGridCell::CELL_IMPASSABLE | sGridCell::CELL_OCCUPIED;
	if (!ignore_personages) {
		attr |= sGridCell::CELL_PERSONAGE_OCCUPIED;
	 }

	for (int y = y0; y < y1; y++) {
		const sGridCell *p = cells;

		for (int x = x0; x < x1; x++, p++) {
			debugC(3, kDebugMovement, "qdCamera::is_walkable(): attr %d at [%d, %d]", p->attributes(), x, y);
			if (p->check_attribute(attr) && !p->check_attribute(sGridCell::CELL_SELECTED)) {
				return false;
			}
		}
		cells += _GSX;
	}

	return true;
}

bool qdCamera::clip_grid_line(Vect2s &v0, Vect2s &v1) const {
	int x = 0, y = 0;
	bool accept = false, done = false;

	int outcodeOut;
	int outcode0 = clip_out_code(v0);
	int outcode1 = clip_out_code(v1);

	do {
		if (outcode0 == 0 && outcode1 == 0) {
			accept = true;
			done = true;
		} else {
			if ((outcode0 & outcode1) != 0)
				done = true;
			else {
				if (outcode0)
					outcodeOut = outcode0;
				else
					outcodeOut = outcode1;

				if (clTOP & outcodeOut) {
					x = v0.x + (v1.x - v0.x) * (_GSY - v0.y - 1) / (v1.y - v0.y);
					y = _GSY - 1;
				} else if (clBOTTOM & outcodeOut) {
					x = v0.x + (v1.x - v0.x) * (-v0.y) / (v1.y - v0.y);
					y = 0;
				}
				if (clRIGHT & outcodeOut) {
					y = v0.y + (v1.y - v0.y) * (_GSX - v0.x - 1) / (v1.x - v0.x);
					x = _GSX - 1;
				} else if (clLEFT & outcodeOut) {
					y = v0.y + (v1.y - v0.y) * (-v0.x) / (v1.x - v0.x);
					x = 0;
				}

				if (outcodeOut == outcode0) {
					v0.x = x;
					v0.y = y;

					outcode0 = clip_out_code(Vect2s(x, y));
				} else {
					v1.x = x;
					v1.y = y;

					outcode1 = clip_out_code(Vect2s(x, y));
				}
			}
		}
	} while (!done);

	return accept;
}

bool qdCamera::init() {
	_default_object = NULL;
	_current_object = NULL;

	_scrCenter = _scrCenterInitial;

	set_mode(_default_mode);

	return true;
}

void qdCamera::clip_center_coords(int &x, int &y) const {
	Vect2i lim = screen_center_limit_x();

	if (x < lim.x)
		x = lim.x;
	else if (x > lim.y)
		x = lim.y;

	lim = screen_center_limit_y();

	if (y < lim.x)
		y = lim.x;
	else if (y > lim.y)
		y = lim.y;
}

bool qdCamera::is_visible(const Vect2i &center_offs) const {
	int sx = g_engine->_screenW / 2;
	int sy = g_engine->_screenH / 2;

	Vect2s pos = scr2rscr(Vect2s(sx, sy));

	pos.x -= center_offs.x;
	pos.y += center_offs.y;

	if (pos.x < -_scrSize.x / 2 - sx || pos.x > _scrSize.x / 2 + sx || pos.y < -_scrSize.y / 2 - sy || pos.y > _scrSize.y / 2 + sy)
		return false;

	return true;
}

void qdCamera::cycle_coords(int &x, int &y) const {
	Vect2s pos = scr2rscr(Vect2s(x, y));

	if (_cycle_x) {
		if (pos.x < -_scrSize.x / 2 + _scrOffset.x)
			pos.x += _scrSize.x;
		else if (pos.x > _scrSize.x / 2 + _scrOffset.x)
			pos.x -= _scrSize.x;
	}
	if (_cycle_y) {
		if (pos.y < -_scrSize.y / 2 + _scrOffset.y)
			pos.y += _scrSize.y;
		else if (pos.y > _scrSize.y / 2 + _scrOffset.y)
			pos.y -= _scrSize.y;
	}

	pos = rscr2scr(pos);

	x = pos.x;
	y = pos.y;
}

bool qdCamera::set_grid_line_attributes(const Vect2s &start_pos, const Vect2s &end_pos, const Vect2s &size, int attr) {
	if (start_pos.x == end_pos.x && start_pos.y == end_pos.y) {
		set_grid_attributes(start_pos, size, attr);
		return true;
	}

	Vect2f r(start_pos.x, start_pos.y);

	int dx = end_pos.x - start_pos.x;
	int dy = end_pos.y - start_pos.y;

	Vect2f dr(dx, dy);
	float d = (float)_cellSX / 3.0;
	if (d < 0.5f) d = 0.5f;
	dr.normalize(d);

	if (abs(dx) > abs(dy)) {
		int i = round(float(dx) / dr.x);
		do {
			set_grid_attributes(Vect2s(r.xi(), r.yi()), size, attr);
			r += dr;
		} while (--i >= 0);
	} else {
		int i = round(float(dy) / dr.y);
		do {
			set_grid_attributes(Vect2s(r.xi(), r.yi()), size, attr);
			r += dr;
		} while (--i >= 0);
	}

	return true;
}

bool qdCamera::drop_grid_line_attributes(const Vect2s &start_pos, const Vect2s &end_pos, const Vect2s &size, int attr) {
	if (start_pos.x == end_pos.x && start_pos.y == end_pos.y) {
		drop_grid_attributes(start_pos, size, attr);
		return true;
	}

	Vect2f r(start_pos.x, start_pos.y);

	int dx = end_pos.x - start_pos.x;
	int dy = end_pos.y - start_pos.y;

	Vect2f dr(dx, dy);
	float d = (float)_cellSX / 3.0;
	if (d < 0.5f) d = 0.5f;
	dr.normalize(d);

	if (abs(dx) > abs(dy)) {
		int i = round(float(dx) / dr.x);
		do {
			drop_grid_attributes(Vect2s(r.xi(), r.yi()), size, attr);
			r += dr;
		} while (--i >= 0);
	} else {
		int i = round(float(dy) / dr.y);
		do {
			drop_grid_attributes(Vect2s(r.xi(), r.yi()), size, attr);
			r += dr;
		} while (--i >= 0);
	}

	return true;
}

bool qdCamera::check_grid_line_attributes(const Vect2s &start_pos, const Vect2s &end_pos, const Vect2s &size, int attr) const {
	if (start_pos.x == end_pos.x && start_pos.y == end_pos.y)
		return check_grid_attributes(start_pos, size, attr);

	Vect2f r(start_pos.x, start_pos.y);

	int dx = end_pos.x - start_pos.x;
	int dy = end_pos.y - start_pos.y;

	Vect2f dr(dx, dy);
	float d = (float)_cellSX / 3.0;
	if (d < 0.5f) d = 0.5f;
	dr.normalize(d);

	if (abs(dx) > abs(dy)) {
		int i = round(float(dx) / dr.x);
		do {
			if (check_grid_attributes(Vect2s(r.xi(), r.yi()), size, attr))
				return true;
			r += dr;
		} while (--i >= 0);
	} else {
		int i = round(float(dy) / dr.y);
		do {
			if (check_grid_attributes(Vect2s(r.xi(), r.yi()), size, attr))
				return true;
			r += dr;
		} while (--i >= 0);
	}

	return false;
}

void qdCamera::dump_grid(const char *file_name) const {
	Common::DumpFile fh;
	fh.open(Common::Path(file_name));

	fh.writeString(Common::String::format("%d x %d\n", _GSX, _GSY));

	for (int i = 0; i < _GSY; i++) {
		for (int j = 0; j < _GSX; j++) {
			if (_grid[j + i * _GSX].attributes() < 10)
				fh.writeString(" ");
			fh.writeString(Common::String::format("%u ", _grid[j + i * _GSX].attributes()));
		}
		fh.writeString("\n");
	}

	fh.close();
}

bool qdCamera::can_change_mode() const {
	if (_current_mode.has_work_time()) return false;

	return true;
}
} // namespace QDEngine
