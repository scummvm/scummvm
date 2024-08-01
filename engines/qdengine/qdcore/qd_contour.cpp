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
#include "qdengine/qdcore/qd_contour.h"

namespace QDEngine {

qdContour::qdContour(qdContourType tp) : contour_type_(tp),
	size_(0, 0),
	mask_pos_(0, 0) {
}

qdContour::qdContour(const qdContour &ct) : contour_type_(ct.contour_type_),
	size_(ct.size_),
	contour_(ct.contour_) {
	mask_pos_ = ct.mask_pos_;
}

qdContour::~qdContour() {
	contour_.clear();
}

qdContour &qdContour::operator = (const qdContour &ct) {
	if (this == &ct) return *this;

	contour_type_ = ct.contour_type_;
	size_ = ct.size_;
	mask_pos_ = ct.mask_pos_;

	contour_ = ct.contour_;

	return *this;
}

void qdContour::add_contour_point(const Vect2s &pt) {
	contour_.push_back(pt);
}

void qdContour::insert_contour_point(const Vect2s &pt, int insert_pos) {
	if (insert_pos < contour_.size()) {
		if (insert_pos < 0) insert_pos = 0;
		contour_.insert(contour_.begin() + insert_pos, pt);
	} else {
		contour_.push_back(pt);
	}
}

bool qdContour::remove_contour_point(int pos) {
	if (pos >= 0 && pos < contour_.size()) {
		contour_.erase(contour_.begin() + pos);
		return true;
	}
	return false;
}

bool qdContour::update_contour_point(const Vect2s &pt, int pos) {
	if (pos >= 0 && pos < contour_.size()) {
		contour_[pos] = pt;
		return true;
	}
	return false;
}

bool qdContour::update_contour() {
	if (contour_type_ != CONTOUR_POLYGON) return false;

	if (contour_.empty())
		return false;

	int x0 = contour_[0].x;
	int x1 = contour_[0].x;
	int y0 = contour_[0].y;
	int y1 = contour_[0].y;

	for (int i = 0; i < contour_.size(); i++) {
		if (contour_[i].x < x0) x0 = contour_[i].x;
		if (contour_[i].x > x1) x1 = contour_[i].x;
		if (contour_[i].y < y0) y0 = contour_[i].y;
		if (contour_[i].y > y1) y1 = contour_[i].y;
	}

	size_ = Vect2s(x1 - x0 + 1, y1 - y0 + 1);
	mask_pos_ = Vect2s(x0 + size_.x / 2, y0 + size_.y / 2);

	return true;
}

bool qdContour::is_inside(const Vect2s &pos) const {
	switch (contour_type_) {
	case CONTOUR_RECTANGLE:
		if (pos.x >= -size_.x / 2 && pos.x < size_.x / 2 && pos.y >= -size_.y / 2 && pos.y < size_.y / 2)
			return true;
		break;
	case CONTOUR_CIRCLE:
		if (pos.x * pos.x + pos.y * pos.y <= size_.x * size_.x)
			return true;
		break;
	case CONTOUR_POLYGON: {
		Vect2s p = pos;
		int intersections_lt0 = 0;
		int intersections_gt0 = 0;
		int intersections_lt1 = 0;
		int intersections_gt1 = 0;
		for (int i = 0; i < contour_.size(); i ++) {
			Vect2s p0 = contour_[i];
			Vect2s p1 = (i < contour_.size() - 1) ? contour_[i + 1] : contour_[0];
			if (p0.y != p1.y) {
				if ((p0.y < p.y && p1.y >= p.y) || (p0.y >= p.y && p1.y < p.y)) {
					if (p0.x < p.x && p1.x < p.x)
						intersections_lt0++;
					else if (p0.x > p.x && p1.x > p.x)
						intersections_gt0++;
					else {
						int x = (p.y - p0.y) * (p1.x - p0.x) / (p1.y - p0.y) + p0.x;

						if (x == p.x)
							return true;
						else if (x > p.x)
							intersections_gt0++;
						else
							intersections_lt0++;
					}
				}
				if ((p0.y <= p.y && p1.y > p.y) || (p0.y > p.y && p1.y <= p.y)) {
					if (p0.x < p.x && p1.x < p.x)
						intersections_lt1++;
					else if (p0.x > p.x && p1.x > p.x)
						intersections_gt1++;
					else {
						int x = (p.y - p0.y) * (p1.x - p0.x) / (p1.y - p0.y) + p0.x;

						if (x == p.x)
							return true;
						else if (x > p.x)
							intersections_gt1++;
						else
							intersections_lt1++;
					}
				}
			}
		}

		return ((intersections_lt0 & 1) && intersections_gt0 != 0) ||
		       ((intersections_lt1 & 1) && intersections_gt1 != 0);
		break;
	}
}
return false;
}

bool qdContour::save_script(Common::WriteStream &fh, int indent) const {
	if (contour_type_ == CONTOUR_POLYGON) {
		for (int i = 0; i < indent; i++) {
			fh.writeString("\t");
		}
		fh.writeString(Common::String::format("<contour_polygon>%d", contour_size() * 2));
		for (int j = 0; j < contour_size(); j++) {
			fh.writeString(Common::String::format(" %d %d", contour_[j].x, contour_[j].y));
		}
		fh.writeString("</contour_polygon>\r\n");
		return true;
	}

	if (contour_type_ == CONTOUR_RECTANGLE) {
		for (int i = 0; i < indent; i++) {
			fh.writeString("\t");
		}
		fh.writeString(Common::String::format("<contour_rect>%d %d</contour_rect>\r\n", size_.x, size_.y));
		return true;
	}

	if (contour_type_ == CONTOUR_CIRCLE) {
		for (int i = 0; i < indent; i++) {
			fh.writeString("\t");
		}
		fh.writeString(Common::String::format("<contour_circle>%d</contour_circle>\r\n", size_.x));
		return true;
	}

	return false;
}

bool qdContour::load_script(const xml::tag *p) {
	int i;

	Vect2s v;
	xml::tag_buffer buf(*p);

	switch (contour_type_) {
	case CONTOUR_RECTANGLE:
		buf > v.x > v.y;
		size_ = Vect2s(v);
		return true;
	case CONTOUR_CIRCLE:
		size_.x = size_.y = buf.get_short();
		return true;
	case CONTOUR_POLYGON:
		contour_.reserve(p->data_size() / 2);
		for (i = 0; i < p->data_size(); i += 2) {
			buf > v.x > v.y;
			add_contour_point(v);
		}
		update_contour();
		return true;
	}

	return false;
}

void qdContour::divide_contour(int shift) {
	std::vector<Vect2s>::iterator _itr = contour_.begin(), _end = contour_.end();
	for (; _itr != _end; ++_itr) {
		Vect2s &v = *_itr;
		v.x >>= shift;
		v.y >>= shift;
	}
}

void qdContour::mult_contour(int shift) {
	std::vector<Vect2s>::iterator _itr = contour_.begin(), _end = contour_.end();
	for (; _itr != _end; ++_itr) {
		Vect2s &v = *_itr;
		v.x <<= shift;
		v.y <<= shift;
	}
}

void qdContour::shift_contour(int dx, int dy) {
	std::vector<Vect2s>::iterator _itr = contour_.begin(), _end = contour_.end();
	for (; _itr != _end; ++_itr) {
		Vect2s &v = *_itr;
		v.x += dx;
		v.y += dy;
	}
}

bool qdContour::can_be_closed() const {
	if (contour_type_ != CONTOUR_POLYGON)
		return false;

	return (contour_.size() > 2);
}

bool qdContour::is_contour_empty() const {
	if (contour_type_ == CONTOUR_POLYGON)
		return (contour_.size() < 3);

	return false;
}

} // namespace QDEngine
