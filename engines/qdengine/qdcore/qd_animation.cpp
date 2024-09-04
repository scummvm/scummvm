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

#include "qdengine/qdengine.h"

#include "qdengine/qd_fwd.h"
#include "qdengine/system/graphics/gr_dispatcher.h"
#include "qdengine/system/graphics/gr_tile_animation.h"

#include "qdengine/parser/qdscr_parser.h"
#include "qdengine/parser/xml_tag_buffer.h"

#include "qdengine/qdcore/qd_animation_info.h"
#include "qdengine/qdcore/qd_named_object_reference.h"
#include "qdengine/qdcore/qd_game_dispatcher.h"

#include "qdengine/qdcore/qd_animation.h"
#include "qdengine/qdcore/qd_file_manager.h"


namespace QDEngine {

qdAnimation::qdAnimation() : _parent(NULL) {
	_tileAnimation = 0;
	_num_frames = 0;

	_length = _cur_time = 0.0f;
	_status = QD_ANIMATION_STOPPED;

	_sx = _sy = 0;

	_is_finished = false;

	_playback_speed = 1.0f;

	_frames_ptr = &_frames;
	_scaled_frames_ptr = &_scaled_frames;
}

qdAnimation::qdAnimation(const qdAnimation &anm) : qdNamedObject(anm), qdResource(anm),
	_parent(anm._parent),
	_length(anm._length),
	_cur_time(anm._cur_time),
	_status(anm._status),
	_is_finished(anm._is_finished),
	_sx(anm._sx),
	_sy(anm._sy),
	_num_frames(anm._num_frames),
	_playback_speed(1.0f),
	_tileAnimation(0) {
	copy_frames(anm);

	if (anm._tileAnimation)
		_tileAnimation = new grTileAnimation(*anm._tileAnimation);
}

qdAnimation::~qdAnimation() {
	clear_frames();
	_qda_file.clear();

	delete _tileAnimation;
}

qdAnimation &qdAnimation::operator = (const qdAnimation &anm) {
	if (this == &anm) return *this;

	*static_cast<qdNamedObject *>(this) = anm;
	*static_cast<qdResource *>(this) = anm;

	_parent = anm._parent;

	_length = anm._length;
	_cur_time = anm._cur_time;
	_status = QD_ANIMATION_STOPPED;
	_is_finished = false;

	_playback_speed = anm._playback_speed;

	_sx = anm._sx;
	_sy = anm._sy;

	copy_frames(anm);

	_num_frames = anm._num_frames;

	delete _tileAnimation;
	_tileAnimation = 0;

	if (anm._tileAnimation)
		_tileAnimation = new grTileAnimation(*anm._tileAnimation);

	return *this;
}

void qdAnimation::quant(float dt) {
	if (_status == QD_ANIMATION_PLAYING) {
		if (need_stop()) {
			stop();
			return;
		}

		_cur_time += dt * _playback_speed;
		if (_cur_time >= length()) {
			if (length() > 0.01f) {
				if (!check_flag(QD_ANIMATION_FLAG_LOOP)) {
					_cur_time = length() - 0.01f;
					_status = QD_ANIMATION_END_PLAYING;
				} else {
					_cur_time = fmodf(_cur_time, length());
				}
			} else
				_cur_time = 0.0f;

			_is_finished = true;
		}
	}
}

void qdAnimation::redraw(int x, int y, int z, int mode) const {
	if (check_flag(QD_ANIMATION_FLAG_FLIP_HORIZONTAL))
		mode |= GR_FLIP_HORIZONTAL;

	if (check_flag(QD_ANIMATION_FLAG_FLIP_VERTICAL))
		mode |= GR_FLIP_VERTICAL;

	if (check_flag(QD_ANIMATION_FLAG_BLACK_FON))
		mode |= GR_BLACK_FON;

	if (tileAnimation()) {
		tileAnimation()->drawFrame(Vect2i(x, y), get_cur_frame_number(), mode, -1);
	} else if (const qdAnimationFrame *p = get_cur_frame())
		p->redraw(x, y, z, mode);
}

void qdAnimation::redraw(int x, int y, int z, float scale, int mode) const {
	if (fabs(scale - 1.0f) < 0.01f) {
		redraw(x, y, z, mode);
		return;
	}

	if (check_flag(QD_ANIMATION_FLAG_FLIP_HORIZONTAL))
		mode |= GR_FLIP_HORIZONTAL;

	if (check_flag(QD_ANIMATION_FLAG_FLIP_VERTICAL))
		mode |= GR_FLIP_VERTICAL;

	if (check_flag(QD_ANIMATION_FLAG_BLACK_FON))
		mode |= GR_BLACK_FON;

	if (tileAnimation())
		tileAnimation()->drawFrame_scale(Vect2i(x, y), get_cur_frame_number(), scale, mode);

	if (const qdAnimationFrame *p = get_cur_frame(scale))
		p->redraw(x, y, z, scale, mode);
}

void qdAnimation::redraw_rot(int x, int y, int z, float angle, int mode) const {
	if (check_flag(QD_ANIMATION_FLAG_FLIP_HORIZONTAL))
		mode |= GR_FLIP_HORIZONTAL;

	if (check_flag(QD_ANIMATION_FLAG_FLIP_VERTICAL))
		mode |= GR_FLIP_VERTICAL;

	if (tileAnimation()) {
		tileAnimation()->drawFrame(Vect2i(x, y), get_cur_frame_number(), angle, mode);
	} else if (const qdAnimationFrame *p = get_cur_frame())
		p->redraw_rot(x, y, z, angle, mode);
}

void qdAnimation::redraw_rot(int x, int y, int z, float angle, const Vect2f &scale, int mode) const {
	if (fabs(scale.x - 1.0f) < 0.01f && fabs(scale.y - 1.0f) < 0.01f) {
		redraw_rot(x, y, z, angle, mode);
		return;
	}

	if (check_flag(QD_ANIMATION_FLAG_FLIP_HORIZONTAL))
		mode |= GR_FLIP_HORIZONTAL;

	if (check_flag(QD_ANIMATION_FLAG_FLIP_VERTICAL))
		mode |= GR_FLIP_VERTICAL;

	if (const qdAnimationFrame *p = get_cur_frame())
		p->redraw_rot(x, y, z, angle, scale, mode);
}

void qdAnimation::draw_mask(int x, int y, int z, uint32 mask_color, int mask_alpha, int mode) const {
	if (check_flag(QD_ANIMATION_FLAG_FLIP_HORIZONTAL))
		mode |= GR_FLIP_HORIZONTAL;

	if (check_flag(QD_ANIMATION_FLAG_FLIP_VERTICAL))
		mode |= GR_FLIP_VERTICAL;

	if (check_flag(QD_ANIMATION_FLAG_BLACK_FON))
		mode |= GR_BLACK_FON;

	if (const qdAnimationFrame *p = get_cur_frame())
		p->draw_mask(x, y, z, mask_color, mask_alpha, mode);
}

void qdAnimation::draw_mask(int x, int y, int z, uint32 mask_color, int mask_alpha, float scale, int mode) const {
	if (fabs(scale - 1.0f) < 0.01f) {
		draw_mask(x, y, z, mask_color, mask_alpha, mode);
		return;
	}

	if (check_flag(QD_ANIMATION_FLAG_FLIP_HORIZONTAL))
		mode |= GR_FLIP_HORIZONTAL;

	if (check_flag(QD_ANIMATION_FLAG_FLIP_VERTICAL))
		mode |= GR_FLIP_VERTICAL;

	if (check_flag(QD_ANIMATION_FLAG_BLACK_FON))
		mode |= GR_BLACK_FON;

	if (const qdAnimationFrame *p = get_cur_frame(scale))
		p->draw_mask(x, y, z, mask_color, mask_alpha, scale, mode);
}

void qdAnimation::draw_mask_rot(int x, int y, int z, float angle, uint32 mask_color, int mask_alpha, int mode) const {
	if (check_flag(QD_ANIMATION_FLAG_FLIP_HORIZONTAL))
		mode |= GR_FLIP_HORIZONTAL;

	if (check_flag(QD_ANIMATION_FLAG_FLIP_VERTICAL))
		mode |= GR_FLIP_VERTICAL;

	if (tileAnimation()) {
		tileAnimation()->drawFrame(Vect2i(x, y), get_cur_frame_number(), angle, mode);
	} else if (const qdAnimationFrame *p = get_cur_frame())
		p->draw_mask_rot(x, y, z, angle, mask_color, mask_alpha, mode);
}

void qdAnimation::draw_mask_rot(int x, int y, int z, float angle, uint32 mask_color, int mask_alpha, const Vect2f &scale, int mode) const {
	if (fabs(scale.x - 1.0f) < 0.01f && fabs(scale.y - 1.0f) < 0.01f) {
		draw_mask_rot(x, y, z, angle, mask_color, mask_alpha, mode);
		return;
	}

	if (check_flag(QD_ANIMATION_FLAG_FLIP_HORIZONTAL))
		mode |= GR_FLIP_HORIZONTAL;

	if (check_flag(QD_ANIMATION_FLAG_FLIP_VERTICAL))
		mode |= GR_FLIP_VERTICAL;

	if (const qdAnimationFrame *p = get_cur_frame())
		p->draw_mask_rot(x, y, z, angle, mask_color, mask_alpha, scale, mode);
}

void qdAnimation::draw_contour(int x, int y, uint32 color) const {
	int mode = 0;

	if (check_flag(QD_ANIMATION_FLAG_FLIP_HORIZONTAL))
		mode |= GR_FLIP_HORIZONTAL;

	if (check_flag(QD_ANIMATION_FLAG_FLIP_VERTICAL))
		mode |= GR_FLIP_VERTICAL;

	if (check_flag(QD_ANIMATION_FLAG_BLACK_FON))
		mode |= GR_BLACK_FON;

	const qdAnimationFrame *p = get_cur_frame();
	if (p) p->draw_contour(x, y, color, mode);
}

void qdAnimation::draw_contour(int x, int y, uint32 color, float scale) const {
	int mode = 0;

	if (check_flag(QD_ANIMATION_FLAG_FLIP_HORIZONTAL))
		mode |= GR_FLIP_HORIZONTAL;

	if (check_flag(QD_ANIMATION_FLAG_FLIP_VERTICAL))
		mode |= GR_FLIP_VERTICAL;

	if (check_flag(QD_ANIMATION_FLAG_BLACK_FON))
		mode |= GR_BLACK_FON;

	const qdAnimationFrame *p = get_cur_frame();
	if (p) p->draw_contour(x, y, color, scale, mode);
}

qdAnimationFrame *qdAnimation::get_cur_frame() {
	for (qdAnimationFrameList::const_iterator iaf = _frames_ptr->begin(); iaf != _frames_ptr->end(); ++iaf) {
		if ((*iaf)->end_time() >= cur_time())
			return *iaf;
	}

	return NULL;
}

const qdAnimationFrame *qdAnimation::get_cur_frame() const {
	for (qdAnimationFrameList::const_iterator iaf = _frames_ptr->begin(); iaf != _frames_ptr->end(); ++iaf) {
		if ((*iaf)->end_time() >= cur_time())
			return *iaf;
	}

	return NULL;
}

const qdAnimationFrame *qdAnimation::get_cur_frame(float &scale) const {
	int index = get_scale_index(scale);

	if (index == -1)
		return get_cur_frame();

	return get_scaled_frame(get_cur_frame_number(), index);
}

bool qdAnimation::add_frame(qdAnimationFrame *p, qdAnimationFrame *insert_pos, bool insert_after) {
	debugC(1, kDebugTemp, "qdAnimation::add_frame()");
	if (check_flag(QD_ANIMATION_FLAG_REFERENCE)) return false;

	if (insert_pos) {
		for (auto iaf = _frames.begin(); iaf != _frames.end(); iaf++) {
			if (*iaf == insert_pos) {
				if (insert_after)
					++iaf;
				_frames.insert(iaf, p);
				_num_frames = _frames.size();
				return true;
			}
		}
	} else {
		if (insert_after)
			_frames.push_back(p);
		else
			_frames.insert(_frames.end(), p);

		debugC(1, kDebugTemp, "qdAnimation::add_frame(): inserted, is_empty: %d", is_empty());

		return true;
	}

	return false;
}

void qdAnimation::init_size() {
	_length = 0.0f;
	if (!tileAnimation()) {
		_sx = _sy = 0;

		for (qdAnimationFrameList::const_iterator iaf = _frames_ptr->begin(); iaf != _frames_ptr->end(); ++iaf) {
			qdAnimationFrame *p = *iaf;

			p->set_start_time(_length);

			if (p->size_x() > _sx)
				_sx = p->size_x();

			if (p->size_y() > _sy)
				_sy = p->size_y();

			_length += p->length();
		}
	} else {
		for (qdAnimationFrameList::const_iterator iaf = _frames_ptr->begin(); iaf != _frames_ptr->end(); ++iaf) {
			qdAnimationFrame *p = *iaf;

			p->set_start_time(_length);
			p->set_size(tileAnimation()->frameSize());
			p->set_picture_offset(Vect2i(0, 0));
			p->set_picture_size(tileAnimation()->frameSize());

			_length += p->length();
		}
	}

	if (_cur_time >= _length)
		_cur_time = _length - 0.01f;

	_num_frames = _frames_ptr->size();
}

void qdAnimation::load_script(const xml::tag *p) {
	for (xml::tag::subtag_iterator it = p->subtags_begin(); it != p->subtags_end(); ++it) {
		switch (it->ID()) {
		case QDSCR_NAME:
			set_name(it->data());
			break;
		case QDSCR_ANIMATION_FILE:
			qda_set_file(Common::Path(it->data(), '\\'));
			break;
		case QDSCR_FLAG:
			set_flag(xml::tag_buffer(*it).get_int());
			break;
		}
	}

	init_size();
}

bool qdAnimation::save_script(Common::WriteStream &fh, int indent) const {
	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}

	fh.writeString("<animation name=");

	if (name()) {
		fh.writeString(Common::String::format("\"%s\"", qdscr_XML_string(name())));
	} else {
		fh.writeString("\" \"");
	}

	if (flags()) {
		fh.writeString(Common::String::format(" flags=\"%d\"", flags()));
	}

	if (!qda_file().empty()) {
		fh.writeString(Common::String::format(" animation_file=\"%s\"", qdscr_XML_string(qda_file().toString('\\'))));
	}

	fh.writeString("/>\r\n");
	return true;
}

bool qdAnimation::load_resources() {
	debugC(3, kDebugLoad, "qdAnimation::load_resources(): '%s' name: %s", transCyrillic(qda_file().toString()), transCyrillic(name()));
	if (check_flag(QD_ANIMATION_FLAG_REFERENCE)) return false;

	if (qda_file().empty()) {
		qdAnimationFrameList::iterator iaf;
		for (iaf = _frames.begin(); iaf != _frames.end(); ++iaf) {
			(*iaf)->load_resources();
		}

		init_size();
		return true;
	} else
		return qda_load(qda_file());

	return false;
}

void qdAnimation::free_resources() {
	toggle_resource_status(false);
	if (check_flag(QD_ANIMATION_FLAG_REFERENCE)) return;

	for (qdAnimationFrameList::iterator iaf = _frames.begin(); iaf != _frames.end(); ++iaf)
		(*iaf)->free_resources();

	for (qdAnimationFrameList::iterator iaf = _scaled_frames.begin(); iaf != _scaled_frames.end(); ++iaf)
		(*iaf)->free_resources();
}

void qdAnimation::create_reference(qdAnimation *p, const qdAnimationInfo *inf) const {
	p->_frames_ptr = &_frames;
	p->_scaled_frames_ptr = &_scaled_frames;

	p->clear_flags();
	p->set_flag(flags() | QD_ANIMATION_FLAG_REFERENCE);

	p->_length = _length;
	p->_cur_time = 0.0f;

	p->_sx = _sx;
	p->_sy = _sy;

	p->_num_frames = _num_frames;

	debugC(1, kDebugTemp, "num_frames_: %d empty?: %d, is_empty()?: %d", _num_frames, _frames.empty(), is_empty());

	if (inf) {
		if (inf->check_flag(QD_ANIMATION_FLAG_LOOP))
			p->set_flag(QD_ANIMATION_FLAG_LOOP);

		if (inf->check_flag(QD_ANIMATION_FLAG_FLIP_HORIZONTAL))
			p->set_flag(QD_ANIMATION_FLAG_FLIP_HORIZONTAL);

		if (inf->check_flag(QD_ANIMATION_FLAG_FLIP_VERTICAL))
			p->set_flag(QD_ANIMATION_FLAG_FLIP_VERTICAL);

		p->_playback_speed = inf->animation_speed();
	}

	p->_parent = this;
}

bool qdAnimation::hit(int x, int y) const {
	int xx = x;
	int yy = y;

	const qdAnimationFrame *p = get_cur_frame();
	if (p) {
		if (check_flag(QD_ANIMATION_FLAG_FLIP_HORIZONTAL))
			xx = -x;
		if (check_flag(QD_ANIMATION_FLAG_FLIP_VERTICAL))
			yy = -y;

		return p->hit(xx, yy);
	}

	return false;
}

bool qdAnimation::hit(int x, int y, float scale) const {
	int xx = x;
	int yy = y;

	const qdAnimationFrame *p = get_cur_frame();
	if (p) {
		if (check_flag(QD_ANIMATION_FLAG_FLIP_HORIZONTAL))
			xx = -x;
		if (check_flag(QD_ANIMATION_FLAG_FLIP_VERTICAL))
			yy = -y;

		return p->hit(xx, yy, scale);
	}

	return false;
}

bool qdAnimation::qda_load(Common::Path fpath) {
	clear_frames();

	debugC(3, kDebugLoad, "qdAnimation::qda_load(%s)", transCyrillic(fpath.toString()));

	Common::SeekableReadStream *fh;
	if (!qdFileManager::instance().open_file(&fh, fpath.toString().c_str())) {
		return false;
	}

	int32 version = fh->readSint32LE();
	_sx = fh->readSint32LE();
	_sy = fh->readSint32LE();
	_length = fh->readFloatLE();
	int32 fl = fh->readSint32LE();
	int32 num_fr = fh->readSint32LE();

	debugC(3, kDebugLoad, "qdAnimation::qda_load(): vers: %d sx: %d x %d len: %f fl: %d num_fr: %d", version, _sx, _sy, _length, fl, num_fr);

	int num_scales = 0;
	if (version >= 103) {
		num_scales = fh->readSint32LE();
	}

	char tile_flag = 0;
	if (version >= 104) {
		tile_flag = fh->readByte();
	}

	if (!tile_flag) {
		if (num_scales) {
			_scales.resize(num_scales);
			for (int i = 0; i < num_scales; i++)
				_scales[i] = fh->readFloatLE();
		} else
			_scales.clear();

		set_flag(fl & (QD_ANIMATION_FLAG_CROP | QD_ANIMATION_FLAG_COMPRESS));

		for (int i = 0; i < num_fr; i++) {
			qdAnimationFrame *p = new qdAnimationFrame;
			p->qda_load(fh, version);
			add_frame(p);
		}

		for (int i = 0; i < num_fr * num_scales; i++) {
			qdAnimationFrame *p = new qdAnimationFrame;
			p->qda_load(fh, version);
			_scaled_frames.push_back(p);
		}
	} else {
		set_flag(fl);

		_sx = fh->readSint32LE();
		_sy = fh->readSint32LE();

		for (int i = 0; i < num_fr; i++) {
			float start_time, length;
			start_time = fh->readFloatLE();
			length = fh->readFloatLE();

			qdAnimationFrame *p = new qdAnimationFrame;
			p->set_start_time(start_time);
			p->set_length(length);
			add_frame(p);
		}

		debugC(1, kDebugLoad, "qdAnimation::qda_load() tileAnimation %s", transCyrillic(fpath.toString()));
		_tileAnimation = new grTileAnimation;
		_tileAnimation->load(fh, version);

		//_tileAnimation->dumpTiles(fpath, 50);
	}

	init_size();

	return true;
}

void qdAnimation::qda_set_file(Common::Path fname) {
	_qda_file = fname;
}

bool qdAnimation::crop() {
	for (qdAnimationFrameList::iterator it = _frames.begin(); it != _frames.end(); ++it)
		(*it)->crop();
	for (qdAnimationFrameList::iterator it = _scaled_frames.begin(); it != _scaled_frames.end(); ++it)
		(*it)->crop();

	return true;
}

bool qdAnimation::undo_crop() {
	for (qdAnimationFrameList::iterator it = _frames.begin(); it != _frames.end(); ++it)
		(*it)->undo_crop();
	for (qdAnimationFrameList::iterator it = _scaled_frames.begin(); it != _scaled_frames.end(); ++it)
		(*it)->undo_crop();

	return true;
}

bool qdAnimation::compress() {
	if (check_flag(QD_ANIMATION_FLAG_COMPRESS)) return false;

	bool result = true;
	for (qdAnimationFrameList::iterator it = _frames.begin(); it != _frames.end(); ++it) {
		if (!(*it)->compress()) result = false;
	}
	for (qdAnimationFrameList::iterator it = _scaled_frames.begin(); it != _scaled_frames.end(); ++it) {
		if (!(*it)->compress()) result = false;
	}

	set_flag(QD_ANIMATION_FLAG_COMPRESS);
	return result;
}

bool qdAnimation::tileCompress(grTileCompressionMethod method, int tolerance) {
	if (!_num_frames || check_flag(QD_ANIMATION_FLAG_TILE_COMPRESS)) return false;

	uncompress();
	undo_crop();

	grTileSprite::setComprasionTolerance(tolerance);

	_tileAnimation = new grTileAnimation;
	_tileAnimation->init(_num_frames, Vect2i(_sx, _sy), _frames.front()->check_flag(qdSprite::ALPHA_FLAG));

	for (int i = 0; i < _num_frames; i++)
		_tileAnimation->addFrame((const uint32 *)get_frame(i)->data());

	if (method != TILE_UNCOMPRESSED)
		_tileAnimation->compress(method);

	_tileAnimation->compact();

	set_flag(QD_ANIMATION_FLAG_TILE_COMPRESS);

	return true;
}

bool qdAnimation::uncompress() {
	if (!check_flag(QD_ANIMATION_FLAG_COMPRESS) || check_flag(QD_ANIMATION_FLAG_TILE_COMPRESS)) return false;

	bool result = true;
	qdAnimationFrameList::iterator iaf;
	for (qdAnimationFrameList::iterator it = _frames.begin(); it != _frames.end(); ++it) {
		if (!(*it)->uncompress()) result = false;
	}
	for (qdAnimationFrameList::iterator it = _scaled_frames.begin(); it != _scaled_frames.end(); ++it) {
		if (!(*it)->uncompress()) result = false;
	}

	drop_flag(QD_ANIMATION_FLAG_COMPRESS);
	return result;
}

int qdAnimation::get_cur_frame_number() const {
	int num = 0;
	for (qdAnimationFrameList::const_iterator iaf = _frames_ptr->begin(); iaf != _frames_ptr->end(); ++iaf) {
		if ((*iaf)->end_time() >= cur_time()) {
			return num;
		}
		num++;
	}

	return -1;
}

void qdAnimation::set_cur_frame(int number) {
	int num = 0;
	for (qdAnimationFrameList::const_iterator iaf = _frames_ptr->begin(); iaf != _frames_ptr->end(); ++iaf) {
		if (num++ == number) {
			set_time((*iaf)->start_time() + (*iaf)->length() / 2.0f);
			return;
		}
	}
}

bool qdAnimation::remove_frame(int number) {
	int num = 0;
	for (qdAnimationFrameList::iterator iaf = _frames.begin(); iaf != _frames.end(); ++iaf) {
		if (num++ == number) {
			delete *iaf;
			_frames.erase(iaf);
			init_size();
			return true;
		}
	}

	return false;
}

bool qdAnimation::remove_frame_range(int number0, int number1) {
	int num = 0;
	qdAnimationFrameList::iterator iaf, iaf0, iaf1;

	iaf0 = iaf1 = _frames.end();
	for (iaf = _frames.begin(); iaf != _frames.end(); ++iaf) {
		if (num == number0)
			iaf0 = iaf;
		if (num == number1) {
			iaf1 = iaf;
			break;
		}
		num++;
	}

	if (iaf0 != _frames.end() && iaf1 != _frames.end()) {
		for (iaf = iaf0; iaf != iaf1; ++iaf)
			delete *iaf;

		_frames.erase(iaf0, iaf1);
		init_size();

		return true;
	}

	return false;
}

bool qdAnimation::reverse_frame_range(int number0, int number1) {
	int num = 0;
	qdAnimationFrameList::iterator iaf0 = _frames.end();
	qdAnimationFrameList::iterator iaf1 = _frames.end();

	for (qdAnimationFrameList::iterator iaf = _frames.begin(); iaf != _frames.end(); ++iaf) {
		if (num == number0)
			iaf0 = iaf;
		if (num == number1) {
			iaf1 = iaf;
			break;
		}
		num++;
	}

	if (iaf0 != _frames.end() && iaf1 != _frames.end()) {
		iaf1++;

		Common::reverse(iaf0, iaf1);
		init_size();
		return true;
	}

	return false;
}

qdAnimationFrame *qdAnimation::get_frame(int number) {
	int num = 0;
	for (qdAnimationFrameList::const_iterator iaf = _frames_ptr->begin(); iaf != _frames_ptr->end(); ++iaf) {
		if (num == number)
			return *iaf;

		num++;
	}

	return 0;
}

bool qdAnimation::load_resource() {
	toggle_resource_status();
	return load_resources();
}


bool qdAnimation::free_resource() {
	toggle_resource_status(false);

	free_resources();
	return true;
}

void qdAnimation::advance_time(float tm) {
	if (_length <= 0.01f) return;

	tm *= _playback_speed;

	if (_cur_time + tm >= length()) {
		if (check_flag(QD_ANIMATION_FLAG_LOOP)) {
			tm -= length() - _cur_time;
			while (tm >= length())
				tm -= length();

			_cur_time = tm;
		} else
			_cur_time = length() - 0.01f;
	} else
		_cur_time += tm;
}

int qdAnimation::picture_size_x() const {
	int i = 0;
	int sx = 0;
	for (qdAnimationFrameList::const_iterator iaf = _frames_ptr->begin(); iaf != _frames_ptr->end(); ++iaf, i++) {
		sx += (*iaf)->picture_size_x();
	}

	if (i) return sx / i;
	return 0;
}

int qdAnimation::picture_size_y() const {
	int i = 0;
	int sy = 0;
	for (qdAnimationFrameList::const_iterator iaf = _frames_ptr->begin(); iaf != _frames_ptr->end(); ++iaf, i++) {
		sy += (*iaf)->picture_size_y();
	}

	if (i) return sy / i;
	return 0;
}

bool qdAnimation::scale(float coeff_x, float coeff_y) {
	bool res = true;

	qdAnimationFrameList::iterator iaf;
	for (iaf = _frames.begin(); iaf != _frames.end(); ++iaf) {
		if (!(*iaf)->scale(coeff_x, coeff_y)) res = false;
	}

	init_size();

	return res;
}

Vect2i qdAnimation::remove_edges() {
	if (_frames.empty()) return Vect2i(0, 0);

	bool crop_flag = false;
	bool compress_flag = false;

	if (check_flag(QD_ANIMATION_FLAG_COMPRESS)) {
		uncompress();
		compress_flag = true;
	}

	if (check_flag(QD_ANIMATION_FLAG_CROP)) {
		undo_crop();
		crop_flag = true;
	}

	int left, top, right, bottom;
	if (!(*_frames.begin())->get_edges_width(left, top, right, bottom)) return Vect2i(0, 0);

	qdAnimationFrameList::iterator iaf;
	for (iaf = _frames.begin(); iaf != _frames.end(); ++iaf) {
		int l, t, r, b;
		if ((*iaf)->get_edges_width(l, t, r, b)) {
			if (l < left) left = l;
			if (t < top) top = t;
			if (r < right) right = r;
			if (b < bottom) bottom = b;
		} else
			return Vect2i(0, 0);
	}

	for (iaf = _frames.begin(); iaf != _frames.end(); ++iaf) {
		if (!(*iaf)->crop(left, top, right, bottom, false)) return Vect2i(0, 0);
	}

	_sx -= left + right;
	_sy -= top + bottom;

	if (crop_flag)
		crop();

	if (compress_flag)
		compress();

	return Vect2i(left, top);
}

bool qdAnimation::load_data(Common::SeekableReadStream &fh, int save_version) {
	debugC(3, kDebugSave, "  qdAnimation::load_data(): before %ld", fh.pos());

	if (!qdNamedObject::load_data(fh, save_version))
		return false;

	char fl;
	fl = fh.readByte();

	if (fl) {
		qdNamedObjectReference ref;

		if (!ref.load_data(fh, save_version))
			return false;

		if (qdGameDispatcher *p = qd_get_game_dispatcher()) {
			if (qdNamedObject *obj = p->get_named_object(&ref)) {
				if (obj->named_object_type() != QD_NAMED_OBJECT_ANIMATION) return false;
				int fl1 = flags();

				static_cast<qdAnimation *>(obj)->create_reference(this);

				clear_flags();
				set_flag(fl1);
			}
		}
	} else
		clear();

	char st, finished;
	st = fh.readByte();
	finished = fh.readByte();
	_cur_time = fh.readFloatLE();
	_length = fh.readFloatLE();

	_status = st;
	_is_finished = (finished) ? true : false;

	debugC(2, kDebugSave, "  qdAnimation::load_data(): after %ld", fh.pos());
	return true;
}

bool qdAnimation::save_data(Common::WriteStream &fh) const {
	debugC(3, kDebugSave, "  qdAnimation::save_data(): before %ld", fh.pos());

	if (!qdNamedObject::save_data(fh)) return false;

	if (check_flag(QD_ANIMATION_FLAG_REFERENCE) && _parent) {
		fh.writeByte(1);
		qdNamedObjectReference ref(_parent);

		if (!ref.save_data(fh))
			return false;
	} else
		fh.writeByte(0);

	fh.writeByte(_status);
	fh.writeByte(_is_finished);
	fh.writeFloatLE(_cur_time);
	fh.writeFloatLE(_length);

	debugC(3, kDebugSave, "  qdAnimation::save_data(): after %ld", fh.pos());
	return true;
}

grScreenRegion qdAnimation::screen_region(int mode, float scale) const {
	if (const qdAnimationFrame *p = get_cur_frame()) {
		if (check_flag(QD_ANIMATION_FLAG_FLIP_HORIZONTAL))
			mode |= GR_FLIP_HORIZONTAL;

		if (check_flag(QD_ANIMATION_FLAG_FLIP_VERTICAL))
			mode |= GR_FLIP_VERTICAL;

		return p->screen_region(mode, scale);
	} else
		return grScreenRegion_EMPTY;
}

bool qdAnimation::copy_frames(const qdAnimation &anm) {
	if (!check_flag(QD_ANIMATION_FLAG_REFERENCE)) {
		clear_frames();

		_frames_ptr = &_frames;

		for (auto &it : anm._frames) {
			_frames.push_back(it->clone());
		}

		_scaled_frames_ptr = &_scaled_frames;

		for (auto &it : anm._scaled_frames) {
			_scaled_frames.push_back(it->clone());
		}
	} else {
		_frames_ptr = anm._frames_ptr;
		_scaled_frames_ptr = anm._scaled_frames_ptr;
	}

	return true;
}

void qdAnimation::clear_frames() {
	for (qdAnimationFrameList::iterator it = _frames.begin(); it != _frames.end(); ++it)
		delete *it;
	for (qdAnimationFrameList::iterator it = _scaled_frames.begin(); it != _scaled_frames.end(); ++it)
		delete *it;

	_frames.clear();
	_scaled_frames.clear();
}

bool qdAnimation::add_scale(float value) {
	if (fabs(value - 1.0f) <= 0.01f || value <= 0.01f) return false;

	Std::vector<float>::const_iterator it = Common::find(_scales.begin(), _scales.end(), value);
	if (it != _scales.end()) return false;

	_scales.push_back(value);
	Common::sort(_scales.begin(), _scales.end());
	return true;
}

bool qdAnimation::create_scaled_frames() {
	if (check_flag(QD_ANIMATION_FLAG_REFERENCE)) return false;

	for (qdAnimationFrameList::iterator it = _scaled_frames.begin(); it != _scaled_frames.end(); ++it)
		delete *it;
	_scaled_frames.clear();

	for (uint i = 0; i < _scales.size(); i++) {
		for (qdAnimationFrameList::iterator it = _frames.begin(); it != _frames.end(); ++it) {
			_scaled_frames.push_back((*it)->clone());
			_scaled_frames.back()->scale(_scales[i], _scales[i]);
		}
	}

	return true;
}

int qdAnimation::get_scale_index(float &scale_value) const {
	int index = -1;
	float scl = 1.0f;

	const Std::vector<float> &scales_vect = (check_flag(QD_ANIMATION_FLAG_REFERENCE) && _parent) ? _parent->_scales : _scales;

	for (uint i = 0; i < scales_vect.size(); i++) {
		if (fabs(scale_value - scl) > fabs(scale_value - scales_vect[i])) {
			scl = scales_vect[i];
			index = i;
		}
	}

	if (index != -1)
		scale_value /= scl;

	return index;
}

const qdAnimationFrame *qdAnimation::get_scaled_frame(int number, int scale_index) const {
	int num = 0;
	number += scale_index * _num_frames;
	for (qdAnimationFrameList::const_iterator it = _scaled_frames_ptr->begin(); it != _scaled_frames_ptr->end(); ++it) {
		if (num++ == number)
			return *it;
	}

	return NULL;
}

#ifdef __QD_DEBUG_ENABLE__
uint32 qdAnimation::resource_data_size() const {
	uint32 size = 0;

	for (qdAnimationFrameList::const_iterator it = _frames.begin(); it != _frames.end(); ++it)
		size += (*it)->resource_data_size();

	for (qdAnimationFrameList::const_iterator it = _scaled_frames.begin(); it != _scaled_frames.end(); ++it)
		size += (*it)->resource_data_size();

	return size;
}
#endif
} // namespace QDEngine
