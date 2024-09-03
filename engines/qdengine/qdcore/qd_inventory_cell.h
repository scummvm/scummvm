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

#ifndef QDENGINE_QDCORE_QD_INVENTORY_CELL_H
#define QDENGINE_QDCORE_QD_INVENTORY_CELL_H

#include "qdengine/parser/xml_fwd.h"
#include "qdengine/qdcore/qd_sprite.h"

namespace Common {
class WriteStream;
}

namespace QDEngine {

class qdGameObjectAnimated;

//! Описание ячейки инвентори.
class qdInventoryCellType {
public:
	qdInventoryCellType() : _type(0) { }
	explicit qdInventoryCellType(int tp) : _type(tp) { }
	~qdInventoryCellType() {
		_sprite.free();
	}

	int type() const {
		return _type;
	}
	void set_type(int tp) {
		_type = tp;
	}

	void set_sprite_file(const Common::Path fname) {
		_sprite.set_file(fname);
	}
	const Common::Path sprite_file() const {
		return _sprite.file();
	}

	const qdSprite *sprite() const {
		return &_sprite;
	}

	bool load_resources() const;
	void free_resources() const;

	bool load_script(const xml::tag *p);
	bool save_script(Common::WriteStream &fh, int indent = 0) const;

	const Vect2i &size() const {
		return _sprite.size();
	}

private:
	//! Тип ячейки.
	int _type;
	//! Внешний вид ячейки.
	mutable qdSprite _sprite;
};

inline bool operator == (const qdInventoryCellType &f, const qdInventoryCellType &s) {
	return ((f.type() == s.type()) && (f.sprite() == s.sprite()));
}

inline bool operator == (const qdInventoryCellType &f, int type) {
	return f.type() == type;
}

typedef Std::vector<qdInventoryCellType> qdInventoryCellTypeVector;

//! Ячейка инвентори.
class qdInventoryCell {
public:
	qdInventoryCell();
	qdInventoryCell(const qdInventoryCellType &tp);
	qdInventoryCell(const qdInventoryCell &cl);
	~qdInventoryCell() { }

	qdInventoryCell &operator = (const qdInventoryCell &cl);

	int type() const {
		return _type;
	}
	void set_type(int tp) {
		_type = tp;
	}

	const qdSprite *sprite() const {
		return _sprite;
	}
	void set_sprite(const qdSprite *spr) {
		_sprite = spr;
	}

	qdGameObjectAnimated *object() const {
		return _object;
	}
	void set_object(qdGameObjectAnimated *obj);

	bool is_empty() const {
		if (!_object) return true;
		else return false;
	}

	const Vect2i &size() const {
		if (sprite())
			return sprite()->size();

		static Vect2i z = Vect2i(0, 0);
		return z;
	}

	int size_x() const {
		if (sprite()) return sprite()->size_x();
		else return 0;
	}
	int size_y() const {
		if (sprite()) return sprite()->size_y();
		else return 0;
	}

	void redraw(int x, int y, bool inactive_mode = false) const;

	bool load_resources();
	bool free_resources();

	//! Загрузка данных из сэйва.
	bool load_data(Common::SeekableReadStream &fh, int save_version);
	//! Запись данных в сэйв.
	bool save_data(Common::WriteStream &fh) const;

	static void set_shadow(uint32 color, int alpha) {
		_shadow_color = color;
		_shadow_alpha = alpha;
	}
	static const Vect2i &screen_offset() {
		return _screen_offset;
	}
	static void set_screen_offset(const Vect2i &offset) {
		_screen_offset = offset;
	}

private:
	//! Тип ячейки.
	int _type;

	//! Внешний вид ячейки.
	/**
	Указывает на _sprite из qdInventoryCellType соответствующего типа.
	*/
	const qdSprite *_sprite;

	//! Объект, который лежит в ячейке.
	mutable qdGameObjectAnimated *_object;

	static Vect2i _screen_offset;
	static uint32 _shadow_color;
	static int _shadow_alpha;
};

typedef Std::vector<qdInventoryCell> qdInventoryCellVector;

//! Группа ячеек инвентори.
class qdInventoryCellSet {
public:
	qdInventoryCellSet();
	qdInventoryCellSet(int x, int y, int sx, int sy, int16 addit_sx, int16 addit_sy, const qdInventoryCellType &tp);
	qdInventoryCellSet(const qdInventoryCellSet &set);
	~qdInventoryCellSet();

	qdInventoryCellSet &operator = (const qdInventoryCellSet &set);

	const Vect2s screen_pos() const {
		return _screen_pos + qdInventoryCell::screen_offset();
	}
	void set_screen_pos(const Vect2s &pos) {
		_screen_pos = pos;
	}

	grScreenRegion screen_region() const;
	const grScreenRegion &last_screen_region() const {
		return _last_screen_region;
	}

	int cell_index(const qdGameObjectAnimated *obj) const;
	Vect2s cell_position(int cell_idx) const;

	const Vect2s &size() const {
		return _size;
	}
	void set_size(const Vect2s &sz) {
		assert(sz.x && sz.y);
		qdInventoryCell t;
		if (_size.x != 0)//предполагаю, что либо оба равны либо оба неравны 0
			t = _cells.front();
		_size = sz;
		_cells.resize((sz.x + _additional_cells.x) * (sz.y + _additional_cells.y));
		Common::fill(_cells.begin(), _cells.end(), t);
	}

	void set_cell_type(const qdInventoryCellType &tp) {
		Common::fill(_cells.begin(), _cells.end(), tp);
	}

	bool hit(const Vect2s &pos) const;
	void set_mouse_hover_object(qdGameObjectAnimated *obj);

	void pre_redraw() const;
	void redraw(int offs_x = 0, int offs_y = 0, bool inactive_mode = false) const;
	void post_redraw();

	bool put_object(qdGameObjectAnimated *p);
	bool put_object(qdGameObjectAnimated *p, const Vect2s &pos);
	bool remove_object(qdGameObjectAnimated *p);
	qdGameObjectAnimated *get_object(const Vect2s &pos) const;
	bool is_object_in_list(const qdGameObjectAnimated *p) const;

	bool load_script(const xml::tag *p);
	bool save_script(Common::WriteStream &fh, int indent = 0) const;

	//! Загрузка данных из сэйва.
	bool load_data(Common::SeekableReadStream &fh, int save_version);
	//! Запись данных в сэйв.
	bool save_data(Common::WriteStream &fh) const;

	bool init(const qdInventoryCellTypeVector &tp);

	const qdInventoryCellVector &cells() const {
		return _cells;
	}
	int num_cells() const {
		return _cells.size();
	}

	bool load_resources();
	bool free_resources();

	void objects_quant(float dt);

	Vect2s additional_cells() const {
		return _additional_cells;
	}
	void set_additional_cells(Vect2s val) {
		_additional_cells = val;
		// Изменили кол-во доп. ячеек - изменяем и всех кол-во массива ячеек
		set_size(size());
	}

	//! Скроллинг
	void scroll_left();
	void scroll_right();
	void scroll_up();
	void scroll_down();

	void debug_log() const;

private:
	// Имеет ли область полного инвентори сета объекты
	bool has_rect_objects(int left, int top, int right, int bottom) const;

	//! Размер группы.
	/**
	В группе _size.x * _size.y ячеек.
	*/
	Vect2s _size;
	//! Дополнительне ячейки по x и y
	Vect2s _additional_cells;
	//! Смещение по x и y (с него выводятся ячекйки в количестве _size)
	Vect2s _cells_shift;
	//! Ячейки.
	qdInventoryCellVector _cells;
	//! Экранные координаты центра первой ячейки группы.
	Vect2s _screen_pos;

	grScreenRegion _last_screen_region;
};

typedef Std::vector<qdInventoryCellSet> qdInventoryCellSetVector;

} // namespace QDEngine

#endif // QDENGINE_QDCORE_QD_INVENTORY_CELL_H
