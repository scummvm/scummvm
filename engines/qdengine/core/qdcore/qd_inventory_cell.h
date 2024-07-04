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

#ifndef QDENGINE_CORE_QDCORE_QD_INVENTORY_CELL_H
#define QDENGINE_CORE_QDCORE_QD_INVENTORY_CELL_H

#include <vector>
#include <algorithm>
#include "qdengine/core/parser/xml_fwd.h"
#include "qdengine/core/qdcore/qd_sprite.h"

namespace Common {
class SeekableWriteStream;
}

namespace QDEngine {

class qdGameObjectAnimated;

//! Описание ячейки инвентори.
class qdInventoryCellType {
public:
	qdInventoryCellType() : type_(0) { }
	explicit qdInventoryCellType(int tp) : type_(tp) { }
	~qdInventoryCellType() {
		sprite_.free();
	}

	int type() const {
		return type_;
	}
	void set_type(int tp) {
		type_ = tp;
	}

	void set_sprite_file(const char *fname) {
		sprite_.set_file(fname);
	}
	const char *sprite_file() const {
		return sprite_.file();
	}

	const qdSprite *sprite() const {
		return &sprite_;
	}

	bool load_resources() const;
	void free_resources() const;

	bool load_script(const xml::tag *p);
	bool save_script(Common::SeekableWriteStream &fh, int indent = 0) const;

	const Vect2i &size() const {
		return sprite_.size();
	}

private:
	//! Тип ячейки.
	int type_;
	//! Внешний вид ячейки.
	mutable qdSprite sprite_;
};

inline bool operator == (const qdInventoryCellType &f, const qdInventoryCellType &s) {
	return ((f.type() == s.type()) && (f.sprite() == s.sprite()));
}

inline bool operator == (const qdInventoryCellType &f, int type) {
	return f.type() == type;
}

typedef std::vector<qdInventoryCellType> qdInventoryCellTypeVector;

//! Ячейка инвентори.
class qdInventoryCell {
public:
	qdInventoryCell();
	qdInventoryCell(const qdInventoryCellType &tp);
	qdInventoryCell(const qdInventoryCell &cl);
	~qdInventoryCell() { }

	qdInventoryCell &operator = (const qdInventoryCell &cl);

	int type() const {
		return type_;
	}
	void set_type(int tp) {
		type_ = tp;
	}

#ifndef _QUEST_EDITOR
	const qdSprite *sprite() const {
		return sprite_;
	}
	void set_sprite(const qdSprite *spr) {
		sprite_ = spr;
	}
#else
	const qdSprite *sprite() const;
#endif

	qdGameObjectAnimated *object() const {
		return object_;
	}
	void set_object(qdGameObjectAnimated *obj);

	bool is_empty() const {
		if (!object_) return true;
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
	bool load_data(qdSaveStream &fh, int save_version);
	bool load_data(Common::SeekableReadStream &fh, int save_version);
	//! Запись данных в сэйв.
	bool save_data(qdSaveStream &fh) const;
	bool save_data(Common::SeekableWriteStream &fh) const;

	static void set_shadow(unsigned color, int alpha) {
		shadow_color_ = color;
		shadow_alpha_ = alpha;
	}
	static const Vect2i &screen_offset() {
		return screen_offset_;
	}
	static void set_screen_offset(const Vect2i &offset) {
		screen_offset_ = offset;
	}

private:
	//! Тип ячейки.
	int type_;

#ifndef _QUEST_EDITOR
	//! Внешний вид ячейки.
	/**
	Указывает на sprite_ из qdInventoryCellType соответствующего типа.
	*/
	const qdSprite *sprite_;
#endif

	//! Объект, который лежит в ячейке.
	mutable qdGameObjectAnimated *object_;

	static Vect2i screen_offset_;
	static unsigned shadow_color_;
	static int shadow_alpha_;
};

typedef std::vector<qdInventoryCell> qdInventoryCellVector;

//! Группа ячеек инвентори.
class qdInventoryCellSet {
public:
	qdInventoryCellSet();
	qdInventoryCellSet(int x, int y, int sx, int sy, short addit_sx, short addit_sy, const qdInventoryCellType &tp);
	qdInventoryCellSet(const qdInventoryCellSet &set);
	~qdInventoryCellSet();

	qdInventoryCellSet &operator = (const qdInventoryCellSet &set);

	const Vect2s screen_pos() const {
		return screen_pos_ + qdInventoryCell::screen_offset();
	}
	void set_screen_pos(const Vect2s &pos) {
		screen_pos_ = pos;
	}

#ifdef _QUEST_EDITOR
	const Vect2i screen_size() const;
	const Vect2i cell_size() const;
#endif // _QUEST_EDITOR

	grScreenRegion screen_region() const;
	const grScreenRegion &last_screen_region() const {
		return last_screen_region_;
	}

	int cell_index(const qdGameObjectAnimated *obj) const;
	Vect2s cell_position(int cell_idx) const;

	const Vect2s &size() const {
		return size_;
	}
	void set_size(const Vect2s &sz) {
		assert(sz.x && sz.y);
		qdInventoryCell __t;
		if (size_.x != 0)//предполагаю, что либо оба равны либо оба неравны 0
			__t = cells_.front();
		size_ = sz;
		cells_.resize((sz.x + additional_cells_.x) * (sz.y + additional_cells_.y));
		std::fill(cells_.begin(), cells_.end(), __t);
	}

	void set_cell_type(const qdInventoryCellType &tp) {
		std::fill(cells_.begin(), cells_.end(), tp);
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
	bool save_script(Common::SeekableWriteStream &fh, int indent = 0) const;

	//! Загрузка данных из сэйва.
	bool load_data(qdSaveStream &fh, int save_version);
	//! Запись данных в сэйв.
	bool save_data(qdSaveStream &fh) const;
	bool save_data(Common::SeekableWriteStream &fh) const;

	bool init(const qdInventoryCellTypeVector &tp);

	const qdInventoryCellVector &cells() const {
		return cells_;
	}
	int num_cells() const {
		return cells_.size();
	}

	bool load_resources();
	bool free_resources();

	void objects_quant(float dt);

	Vect2s additional_cells() const {
		return additional_cells_;
	}
	void set_additional_cells(Vect2s val) {
		additional_cells_ = val;
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
	В группе size_.x * size_.y ячеек.
	*/
	Vect2s size_;
	//! Дополнительне ячейки по x и y
	Vect2s additional_cells_;
	//! Смещение по x и y (с него выводятся ячекйки в количестве size_)
	Vect2s cells_shift_;
	//! Ячейки.
	qdInventoryCellVector cells_;
	//! Экранные координаты центра первой ячейки группы.
	Vect2s screen_pos_;

	grScreenRegion last_screen_region_;
};

typedef std::vector<qdInventoryCellSet> qdInventoryCellSetVector;

} // namespace QDEngine

#endif // QDENGINE_CORE_QDCORE_QD_INVENTORY_CELL_H
