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

#ifndef QDENGINE_QDCORE_QD_CAMERA_H
#define QDENGINE_QDCORE_QD_CAMERA_H

#include "qdengine/qdcore/qd_d3dutils.h"
#include "qdengine/qdcore/qd_camera_mode.h"

namespace Common {
class WriteStream;
}

namespace QDEngine {

class sGridCell {
public:
	//! Атрибуты
	enum cell_attribute_t {
		//! ячейка выделена
		CELL_SELECTED       = 0x01,
		//! ячейка непроходима
		CELL_IMPASSABLE     = 0x02,
		//! ячейка занята объектом
		CELL_OCCUPIED       = 0x04,
		//! ячейка занята персонажем
		CELL_PERSONAGE_OCCUPIED = 0x08,
		//! по ячейке проходит путь персонажа
		CELL_PERSONAGE_PATH = 0x10
	};

	sGridCell() {
		_attributes = CELL_IMPASSABLE;
	}
	sGridCell(uint16 atr, int16 h) {
		_attributes = atr;
	}
	~sGridCell() { }

	bool is_walkable() const {
		return check_attribute(CELL_IMPASSABLE);
	}
	void make_impassable() {
		set_attribute(CELL_IMPASSABLE);
	}
	void make_walkable() {
		drop_attribute(CELL_IMPASSABLE);
	}

	void select() {
		set_attribute(CELL_SELECTED);
	}
	void deselect() {
		drop_attribute(CELL_SELECTED);
	}
	void toggle_select() {
		toggle_attribute(CELL_SELECTED);
	}
	bool is_selected() const {
		return check_attribute(CELL_SELECTED);
	}

	int16 height() const {
		return 0;
	}
	void set_height(int16 h) { }

	void set_attribute(uint32 attr) {
		_attributes |= attr;
	}
	void toggle_attribute(uint32 attr) {
		_attributes ^= attr;
	}
	void drop_attribute(uint32 attr) {
		_attributes &= ~attr;
	}
	bool check_attribute(uint32 attr) const {
		if (_attributes & attr) return true;
		else return false;
	}

	uint32 attributes() const {
		return _attributes;
	}
	void set_attributes(uint32 attr) {
		_attributes = attr;
	}

	bool clear() {
		_attributes = 0;
		return true;
	}

private:
	byte _attributes;
};

enum qdCameraRedrawMode {
	QDCAM_GRID_ABOVE,
	QDCAM_GRID_ZBUFFER,
	QDCAM_GRID_NONE
};

class qdCamera {
public:
	qdCamera();
	~qdCamera();

	// Устанавливет параметры клетки с координатами cell_pos
	// по параметрам клетки cell
	bool set_grid_cell(const Vect2s &cell_pos, const sGridCell &cell);
	bool set_grid_cell_attributes(const Vect2s &cell_pos, int attr);

	//! Устанавливает атрибуты для клеток из прямоугольника на сетке с ценром center_pos и размерами size.
	bool set_grid_attributes(const Vect2s &center_pos, const Vect2s &size, int attr);
	//! Очищает атрибуты для клеток из прямоугольника на сетке с ценром center_pos и размерами size.
	bool drop_grid_attributes(const Vect2s &center_pos, const Vect2s &size, int attr);
	//! Возвращает true, если в прямоугольнике на сетке ЕСТЬ ХОТЯ БЫ ОДНА ячейка с атрибутами attr.
	bool check_grid_attributes(const Vect2s &center_pos, const Vect2s &size, int attr) const;
	//! Возвращает количество ячеек в заданной области, имеющих именно аттрибуты attr
	int cells_num_with_exact_attributes(const Vect2s &center_pos, const Vect2s &size, int attr) const;

	//! Устанавливает атрибуты для линии клеток.
	bool set_grid_line_attributes(const Vect2s &start_pos, const Vect2s &end_pos, const Vect2s &size, int attr);
	//! Очищает атрибуты для линии клеток.
	bool drop_grid_line_attributes(const Vect2s &start_pos, const Vect2s &end_pos, const Vect2s &size, int attr);
	//! Возвращает true, если в линии на сетке есть хотя бы одна ячейка с атрибутами attr.
	bool check_grid_line_attributes(const Vect2s &start_pos, const Vect2s &end_pos, const Vect2s &size, int attr) const;

	bool is_walkable(const Vect2s &center_pos, const Vect2s &size, bool ignore_personages = false) const;

	//! Устанавливает атрибуты attr для всех клеток сетки.
	bool set_grid_attributes(int attr);
	//! Очищает атрибуты attr для всех клеток сетки.
	bool drop_grid_attributes(int attr);

	sGridCell *get_cell(const Vect2s &cell_pos);
	const sGridCell *get_cell(const Vect2s &cell_pos) const;

	// Восстанавливает параметры клетки (сейчас - делает ее непроходимой
	// с нулевой высотой)
	bool restore_grid_cell(const Vect2s cell_pos);

	void cycle_coords(int &x, int &y) const;

	void set_redraw_mode(int md) const {
		_redraw_mode = md;
	}
	int get_redraw_mode() const {
		return _redraw_mode;
	}

	void set_grid_size(int xs, int ys);

	void scale_grid(int sx, int sy, int csx, int csy);
	void resize_grid(int sx, int sy);

	int get_grid_sx() const {
		return _GSX;
	}
	int get_grid_sy() const {
		return _GSY;
	}

	const sGridCell *get_grid() const {
		return _grid;
	}

	int get_cell_sx() const {
		return _cellSX;
	}
	int get_cell_sy() const {
		return _cellSY;
	}

	void set_cell_size(int csx, int csy) {
		_cellSX = csx;
		_cellSY = csy;
	}

	void clear_grid();

	// rotateAndScaling
	void rotate_and_scale(float XA, float YA, float ZA, float kX, float kY, float kZ);

	float get_focus() const {
		return _focus;
	}
	void set_focus(float focus) {
		_focus = focus;
	}

	void set_R(const float r);
	float get_R() const {
		return _m_fR;
	}

	float get_x_angle() const {
		return _xAngle;
	}
	float get_y_angle() const {
		return _yAngle;
	}
	float get_z_angle() const {
		return _zAngle;
	}

	inline void set_scr_size(int xs, int ys) {
		_scrSize.x = xs;
		_scrSize.y = ys;
	}

	const Vect2i &get_scr_size() const {
		return _scrSize;
	}

	// getScrSizeX
	int get_scr_sx() const {
		return _scrSize.x;
	}
	// getScrSizeY
	int get_scr_sy() const {
		return _scrSize.y;
	}

	void set_scr_center(int xc, int yc) {
		_scrCenter.x = xc;
		_scrCenter.y = yc;
	}

	const Vect2i &get_scr_center() const {
		return _scrCenter;
	}

	int get_scr_center_x() const {
		return _scrCenter.x;
	}
	int get_scr_center_y() const {
		return _scrCenter.y;
	}

	const Vect2i screen_center_limit_x() const;
	const Vect2i screen_center_limit_y() const;

	const Vect2i &get_scr_offset() const {
		return _scrOffset;
	}
	void set_scr_offset(const Vect2i &offs) {
		_scrOffset = offs;
	}

	const Vect2i &get_scr_center_initial() const {
		return _scrCenterInitial;
	}
	void set_scr_center_initial(const Vect2i &v) {
		_scrCenterInitial = v;
	}

	void move_scr_center(int dxc, int dyc);

	float scrolling_phase_x() const;
	float scrolling_phase_y() const;

	const Vect3f scr2plane(const Vect2s &scrPoint) const;
	const Vect3f rscr2plane(const Vect2s &rscrPoint) const;

	const Vect3f scr2plane_camera_coord(const Vect2s &scrPoint) const;
	const Vect3f rscr2plane_camera_coord(const Vect2s &scrPoint) const;
	const Vect2s plane2scr(const Vect3f &plnPoint) const;
	const Vect2s plane2rscr(const Vect3f &plnPoint) const;

	const Vect3f rscr2camera_coord(const Vect2s &rScrPoint, float z) const;
	const Vect2s camera_coord2rscr(const Vect3f &coord) const;

	const Vect2s rscr2scr(const Vect2s &v) const;
	const Vect2s scr2rscr(const Vect2s &v) const;

	const Vect2s camera_coord2scr(const Vect3f &coord) const;

	const Vect3f scr2global(const Vect2s &vScrPoint, float zInCameraCoord) const;
	const Vect3f rscr2global(const Vect2s rScrPoint, const float zInCameraCoord) const;
	const Vect3f global2camera_coord(const Vect3f &glCoord) const;
	const Vect3f camera_coord2global(const Vect3f &v) const;

	const Vect2s global2scr(const Vect3f &glCoord) const;
	const Vect2s global2rscr(const Vect3f &glCoord) const;

	//обрезание отрезка по плоскости отсечения камеры
	//возвращает false, если отрезок вообеще невидим
	bool line_cutting(Vect3f &b, Vect3f &e) const;
	//! Обрезка линиии, так чтобы она за сетку не выходила.
	/**
	Все координаты - сеточные, т.е. индексы ячеек сетки.
	*/
	bool clip_grid_line(Vect2s &v0, Vect2s &v1) const;

	// getKScale
	float get_scale(const Vect3f &glCoord) const;

	const Vect3f &get_grid_center() const {
		return _gridCenter;
	}
	void set_grid_center(const Vect3f &v) {
		_gridCenter = v;
	}

	const sGridCell *get_cell(float X, float Y) const;
	const Vect2s get_cell_index(float X, float Y, bool grid_crop = true) const;
	const Vect2s get_cell_index(const Vect3f &v, bool grid_crop = true) const;
	const Vect3f get_cell_coords(int x_idx, int y_idx) const;
	const Vect3f get_cell_coords(const Vect2s &idxs) const;

	void reset_all_select();
	//принимает глобальные координаты
	bool select_cell(int x, int y);

	//принимает глобальные координаты
	bool deselect_cell(int x, int y);

	void load_script(const xml::tag *p);
	bool save_script(Common::WriteStream &fh, int indent = 0) const;

	//! Загрузка данных из сэйва.
	bool load_data(Common::SeekableReadStream &fh, int save_version);
	//! Запись данных в сэйв.
	bool save_data(Common::WriteStream &fh) const;

	//! Инициализация данных, вызывается при старте и перезапуске игры.
	bool init();

	bool draw_grid() const;
	bool draw_cell(int x, int y, int z, int penWidth, uint32 color) const;

	//! Установка текущего режима камеры.
	/**
	Если объект нулевой, то берется объект по-умолчанию.
	*/
	bool set_mode(const qdCameraMode &mode, qdGameObjectAnimated *object = NULL);
	//! Возвращает true, если в данный момент можно менять режим камеры.
	bool can_change_mode() const;

	//! Установка объекта, за которым камера следит по умолчанию.
	void set_default_object(qdGameObjectAnimated *p) {
		_default_object = p;
	}

	//! Обсчет логики (движения камеры и т.д), параметр - время в секундах.
	bool quant(float dt);

	//! Возвращает false, если вся рабочая область камеры находится за пределами экрана.
	/**
	Параметр - дополнительное смещение центра рабочей области камеры.
	*/
	bool is_visible(const Vect2i &center_offs = Vect2i(0, 0)) const;

	void set_cycle(bool cx, bool cy) {
		_cycle_x = cx;
		_cycle_y = cy;
	}

	void dump_grid(const char *file_name) const;

	//! Параметры функции масштабирования
	float scale_pow() const {
		return _scale_pow;
	}
	void set_scale_pow(float sp) {
		_scale_pow = sp;
	}
	float scale_z_offset() const {
		return _scale_z_offset;
	};
	void set_scale_z_offset(float szo) {
		_scale_z_offset = szo;
	};

	bool need_perspective_correction() const {
		return (fabs(_scale_pow - 1) > 0.001 || fabs(_scale_z_offset) > 0.001);
	}

	//! Возвращает режим работы камеры по умолчанию.
	const qdCameraMode &default_mode() const {
		return _default_mode;
	}
	//! Установка режима работы камеры по умолчанию.
	void set_default_mode(const qdCameraMode &mode) {
		_default_mode = mode;
	}

	static qdCamera *current_camera() {
		return _current_camera;
	}
	static void set_current_camera(qdCamera *p) {
		_current_camera = p;
	}

	MATRIX3D const &get_view_matrix() const {
		return _m_cam;
	}

private:

	MATRIX3D _m_cam;
	float _m_fR;
	float _xAngle, _yAngle, _zAngle;

	int _GSX, _GSY;
	sGridCell *_grid;

	bool _cycle_x;
	bool _cycle_y;

	int _cellSX, _cellSY;
	float _focus;//! расстояние до ближней плоскости отсечения
	// расстояние от центра рабочей области до
	// верхнего левого угла окна
	Vect2i _scrCenter;

	// начальное расстояние от центра рабочей области до
	// верхнего левого угла окна
	Vect2i _scrCenterInitial;

	// размер рабочей области
	Vect2i _scrSize;
	// смещение рабочей области
	Vect2i _scrOffset;

	// Координаты центра сетки
	Vect3f _gridCenter;

	mutable int _redraw_mode;

	//! Текущий режим работы камеры.
	qdCameraMode _current_mode;

	//! Время в секуднах с установки текущего режима.
	float _current_mode_work_time;
	bool _current_mode_switch;

	//! Объект, за которым следит камера.
	qdGameObjectAnimated *_current_object;

	//! Объект, за которым камера следит по умолчанию.
	qdGameObjectAnimated *_default_object;
	//! Режим работы камеры по умолчанию.
	qdCameraMode _default_mode;

	//! Параметры функции ускоренного масштабирования из get_scale()
	float _scale_pow;
	float _scale_z_offset;

	static qdCamera *_current_camera;

	static const float _NEAR_PLANE; //ближная плоскость отсечения
	static const float _FAR_PLANE;  //дальняя

	enum {
		clLEFT   = 1,
		clRIGHT  = 2,
		clBOTTOM = 4,
		clTOP    = 8
	};

	inline int clip_out_code(const Vect2s &v) const {
		int code = 0;
		if (v.y >= _GSY)
			code |= clTOP;
		else if (v.y < 0)
			code |= clBOTTOM;
		if (v.x >= _GSX)
			code |= clRIGHT;
		else if (v.x < 0)
			code |= clLEFT;

		return code;
	}

	void clip_center_coords(int &x, int &y) const;
};

inline Vect3f To3D(const Vect2f &v) {
	return Vect3f(v.x, v.y, 0);
}

} // namespace QDEngine

#endif // QDENGINE_QDCORE_QD_CAMERA_H
