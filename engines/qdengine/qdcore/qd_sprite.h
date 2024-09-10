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

#ifndef QDENGINE_QDCORE_QD_SPRITE_H
#define QDENGINE_QDCORE_QD_SPRITE_H

#include "common/str.h"

namespace Common {
class SeekableReadStream;
}

#include "qdengine/system/graphics/gr_screen_region.h"
#include "qdengine/qdcore/qd_resource.h"
#include "qdengine/xmath.h"


namespace QDEngine {

//! Спрайт.
class qdSprite : public qdResource {
public:
	enum { // flags
		ALPHA_FLAG      = 0x01
	};

	qdSprite();
	qdSprite(int wid, int hei, int format);
	qdSprite(const qdSprite &spr);

	~qdSprite();

	qdSprite &operator = (const qdSprite &spr);

	const Vect2i &size() const {
		return _size;
	}
	void set_size(const Vect2i &size) {
		_size = size;
	}

	int size_x() const {
		return _size.x;
	}
	int size_y() const {
		return _size.y;
	}

	int picture_x() const {
		return _picture_offset.x;
	}
	int picture_y() const {
		return _picture_offset.y;
	}
	void set_picture_offset(const Vect2i &offs) {
		_picture_offset = offs;
	}

	int picture_size_x() const {
		return _picture_size.x;
	}
	int picture_size_y() const {
		return _picture_size.y;
	}
	void set_picture_size(const Vect2i &size) {
		_picture_size = size;
	}

	int format() const {
		return _format;
	}
	const byte *data() const {
		return _data;
	}
	uint32 data_size() const;

	void set_file(const Common::Path fname) {
		_file = fname;
	}
	const Common::Path file() const {
		return _file;
	}
	bool has_file() const {
		return !_file.empty();
	}

	bool load(const Common::Path fname);
	bool load();
	void save(const Common::Path fname);
	void free();

	virtual void qda_load(Common::SeekableReadStream *fh, int version = 100);

	void redraw(int x, int y, int z, int mode = 0) const;
	void redraw_rot(int x, int y, int z, float angle, int mode = 0) const;
	void redraw_rot(int x, int y, int z, float angle, const Vect2f &scale, int mode = 0) const;

	void redraw(int x, int y, int z, float scale, int mode = 0) const;
	void draw_mask(int x, int y, int z, uint32 mask_color, int mask_alpha, int mode = 0) const;
	void draw_mask(int x, int y, int z, uint32 mask_color, int mask_alpha, float scale, int mode = 0) const;

	void draw_mask_rot(int x, int y, int z, float angle, uint32 mask_color, int mask_alpha, int mode = 0) const;
	void draw_mask_rot(int x, int y, int z, float angle, uint32 mask_color, int mask_alpha, const Vect2f &scale, int mode = 0) const;

	void draw_contour(int x, int y, uint32 color, int mode = 0) const;
	void draw_contour(int x, int y, uint32 color, float scale, int mode = 0) const;

	bool hit(int x, int y) const;
	bool hit(int x, int y, float scale) const;

	bool put_pixel(int x, int y, byte r, byte g, byte b);

	bool crop();
	bool crop(int left, int top, int right, int bottom, bool store_offsets = true);
	bool undo_crop();

	bool get_edges_width(int &left, int &top, int &right, int &bottom);
	Vect2i remove_edges();

	bool compress();
	bool uncompress();
	bool is_compressed() const {
		if (_rle_data) return true;
		else return false;
	}

	bool scale(float coeff_x, float coeff_y);

	void set_flag(int fl) {
		_flags |= fl;
	}
	void drop_flag(int fl) {
		_flags &= ~fl;
	}
	bool check_flag(int fl) const {
		if (_flags & fl) {
			return true;
		}
		return false;
	}
	void clear_flags() {
		_flags = 0;
	}
	int flags() const {
		return _flags;
	}

	//! Загружает в память данные ресурса.
	bool load_resource() {
		return load();
	}
	//! Выгружает из памяти данные ресурса.
	bool free_resource() {
		free();
		return true;
	}

	//! Устанавливает имя файла, в котором хранятся данные ресурса.
	void set_resource_file(const Common::Path filename) {
		set_file(filename);
	}
	//! Возвращает имя файла, в котором хранятся данные ресурса.
	/**
	Если оно не задано, должна возвращаеть NULL.
	*/
	const Common::Path resource_file() const {
		if (has_file()) return file();
		return NULL;
	}
#ifdef __QD_DEBUG_ENABLE__
	uint32 resource_data_size() const {
		return data_size();
	}
#endif

	//! Возвращает область экрана, занимаемую спрайтом.
	/**
	Координаты области - смещение от центра спрайта.
	В mode задаются повороты спрайта по горизонтали и вертикали
	(GR_FLIP_HORIZONTAL, GR_FLIP_VERTICAL)
	*/
	grScreenRegion screen_region(int mode = 0, float scale = 1.0f) const;

private:
	int _format;
	int _flags;

	Vect2i _size;

	Vect2i _picture_size;
	Vect2i _picture_offset;

	byte *_data;
	class RLEBuffer *_rle_data;

	Common::Path _file;

	friend bool operator == (const qdSprite &sp1, const qdSprite &sp2);
};

} // namespace QDEngine

#endif // QDENGINE_QDCORE_QD_SPRITE_H
