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

#ifndef QDENGINE_QDCORE_QD_ANIMATION_H
#define QDENGINE_QDCORE_QD_ANIMATION_H

#include "qdengine/parser/xml_fwd.h"
#include "qdengine/system/graphics/gr_screen_region.h"
#include "qdengine/system/graphics/gr_tile_animation.h"

#include "qdengine/qdcore/qd_resource.h"
#include "qdengine/qdcore/qd_animation_frame.h"
#include "qdengine/qdcore/qd_named_object.h"


namespace QDEngine {


class qdAnimationInfo;

const int QD_ANIMATION_FLAG_REFERENCE       = 0x01;
const int QD_ANIMATION_FLAG_LOOP            = 0x04;
const int QD_ANIMATION_FLAG_FLIP_HORIZONTAL = 0x08;
const int QD_ANIMATION_FLAG_FLIP_VERTICAL   = 0x10;
const int QD_ANIMATION_FLAG_BLACK_FON       = 0x20;
const int QD_ANIMATION_FLAG_SUPPRESS_ALPHA  = 0x40;
const int QD_ANIMATION_FLAG_CROP            = 0x80;
const int QD_ANIMATION_FLAG_COMPRESS        = 0x100;
const int QD_ANIMATION_FLAG_TILE_COMPRESS   = 0x200;

enum qdAnimationStatus {
	QD_ANIMATION_STOPPED = 0,
	QD_ANIMATION_PLAYING,
	QD_ANIMATION_PAUSED,
	QD_ANIMATION_END_PLAYING
};

//! Анимация.
class qdAnimation : public qdNamedObject, public qdResource {
public:
	qdAnimation();
	qdAnimation(const qdAnimation &anm);
	~qdAnimation();

	qdAnimation &operator = (const qdAnimation &anm);

	int named_object_type() const {
		return QD_NAMED_OBJECT_ANIMATION;
	}

	const qdAnimationFrame *get_cur_frame() const;
	const qdAnimationFrame *get_cur_frame(float &scale) const;
	qdAnimationFrame *get_cur_frame();

	void set_cur_frame(int number);
	int get_cur_frame_number() const;

	qdAnimationFrame *get_frame(int number);
	const qdAnimationFrame *get_scaled_frame(int number, int scale_index) const;

	int num_frames() const {
		return _num_frames;
	}

	float length() const {
		return _length;
	}
	float cur_time() const {
		return _cur_time;
	}

	void set_time(float tm) {
		_cur_time = tm;
	}

	float cur_time_rel() const {
		if (_length > 0.01f)
			return _cur_time / _length;
		return 0.0f;
	}
	void set_time_rel(float tm) {
		if (tm < 0.0f) tm = 0.0f;
		if (tm > 0.99f) tm = 0.99f;
		_cur_time = _length * tm;
	}
	void advance_time(float tm);

	void init_size();
	int size_x() const {
		return _sx;
	}
	int size_y() const {
		return _sy;
	}

	int picture_size_x() const;
	int picture_size_y() const;

	bool is_playing() const {
		return (_status == QD_ANIMATION_PLAYING ||
		        _status == QD_ANIMATION_END_PLAYING);
	}

	int status() const {
		return _status;
	}
	bool is_finished() const {
		return _is_finished;
	}
	bool need_stop() const {
		return _status == QD_ANIMATION_END_PLAYING;
	}

	void start() {
		_status = QD_ANIMATION_PLAYING;
		_is_finished = false;
		_cur_time = 0.0f;
	}
	void stop() {
		_status = QD_ANIMATION_STOPPED;
		_is_finished = true;
	}
	void pause() {
		_status = QD_ANIMATION_PAUSED;
	}
	void resume() {
		_status = QD_ANIMATION_PLAYING;
	}

	void quant(float dt);

	void redraw(int x, int y, int z, int mode = 0) const;
	void redraw(int x, int y, int z, float scale, int mode = 0) const;

	void redraw_rot(int x, int y, int z, float angle, int mode = 0) const;
	void redraw_rot(int x, int y, int z, float angle, const Vect2f &scale, int mode = 0) const;

	void draw_mask(int x, int y, int z, uint32 mask_color, int mask_alpha, int mode = 0) const;
	void draw_mask(int x, int y, int z, uint32 mask_color, int mask_alpha, float scale, int mode = 0) const;

	void draw_mask_rot(int x, int y, int z, float angle, uint32 mask_color, int mask_alpha, int mode = 0) const;
	void draw_mask_rot(int x, int y, int z, float angle, uint32 mask_color, int mask_alpha, const Vect2f &scale, int mode = 0) const;

	void draw_contour(int x, int y, uint32 color) const;
	void draw_contour(int x, int y, uint32 color, float scale) const;

	bool hit(int x, int y) const;
	bool hit(int x, int y, float scale) const;

	bool add_frame(qdAnimationFrame *p, qdAnimationFrame *insert_pos = 0, bool insert_after = true);
	bool remove_frame(int number);
	bool remove_frame_range(int number0, int number1);
	bool reverse_frame_range(int number0, int number1);

	void load_script(const xml::tag *p);
	bool save_script(Common::WriteStream &fh, int indent = 0) const;

	const Common::Path qda_file() const {
		return _qda_file;
	}
	void qda_set_file(const Common::Path fname);

	bool qda_load(const Common::Path fname);

	bool load_resources();
	void free_resources();

	bool scale(float coeff_x, float coeff_y);

	bool crop();
	bool undo_crop();

	Vect2i remove_edges();

	bool compress();
	bool uncompress();
	bool tileCompress(grTileCompressionMethod method = TILE_UNCOMPRESSED, int tolerance = 0);

	qdAnimationFrameList &frames_list() {
		return _frames;
	};

	void create_reference(qdAnimation *p, const qdAnimationInfo *inf = NULL) const;
	bool is_reference(const qdAnimation *p) const {
		if (p->check_flag(QD_ANIMATION_FLAG_REFERENCE) && p->_parent == this) return true;
		return false;
	}

	void clear() {
		stop();
		_frames_ptr = &_frames;
		_parent = NULL;
	}

	bool is_empty() const {
		return (_frames_ptr->empty());
	}

	//! Возвращает область экрана, занимаемую анимацией.
	/**
	Координаты области - смещение от центра анимации.
	В mode задаются повороты анимации по горизонтали и вертикали
	(QD_ANIMATION_FLAG_FLIP_HORIZONTAL, QD_ANIMATION_FLAG_FLIP_VERTICAL)
	*/
	grScreenRegion screen_region(int mode = 0, float scale = 1.0f) const;

	const qdAnimation *parent() const {
		return _parent;
	}

	// qdResource
	bool load_resource();
	bool free_resource();
	//! Устанавливает имя файла, в котором хранятся данные ресурса.
	void set_resource_file(const Common::Path file_name) {
		qda_set_file(file_name);
	}
	//! Возвращает имя файла, в котором хранится анимация.
	const Common::Path resource_file() const {
		if (qda_file().empty()) {
			if (!check_flag(QD_ANIMATION_FLAG_REFERENCE) && !_frames.empty()) {
				if (_frames.front()->has_file())
					return _frames.front()->file();
				else
					return NULL;
			} else
				return NULL;
		} else
			return qda_file();
	}
#ifdef __QD_DEBUG_ENABLE__
	uint32 resource_data_size() const;
#endif

	//! Загрузка данных из сэйва.
	bool load_data(Common::SeekableReadStream &fh, int save_version);
	//! Запись данных в сэйв.
	bool save_data(Common::WriteStream &fh) const;

	bool add_scale(float value);
	bool create_scaled_frames();

	const Std::vector<float> &scales() const {
		if (check_flag(QD_ANIMATION_FLAG_REFERENCE) && _parent) return _parent->_scales;
		else return _scales;
	}
	void clear_scales() {
		_scales.clear();
	}

	const grTileAnimation *tileAnimation() const {
		if (check_flag(QD_ANIMATION_FLAG_REFERENCE) && _parent)
			return _parent->_tileAnimation;
		else
			return _tileAnimation;
	}

	static Common::String flag2str(int fl, bool truncate = false, bool icon = false);
	static Common::String status2str(int fl, bool truncate = false);

private:
	int _sx;
	int _sy;

	enum {
		qda_version = 104
	};

	float _length;
	float _cur_time;

	float _playback_speed;

	int _num_frames;

	const qdAnimationFrameList *_frames_ptr;
	qdAnimationFrameList _frames;

	const qdAnimationFrameList *_scaled_frames_ptr;
	qdAnimationFrameList _scaled_frames;
	Std::vector<float> _scales;

	grTileAnimation *_tileAnimation;

	int _status;
	bool _is_finished;

	Common::Path _qda_file;

	const qdAnimation *_parent;

	int get_scale_index(float &scale_value) const;

	bool copy_frames(const qdAnimation &anm);
	void clear_frames();
};

} // namespace QDEngine

#endif // QDENGINE_QDCORE_QD_ANIMATION_H
