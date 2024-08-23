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

qdContour::qdContour(qdContourType tp) : _contour_type(tp),
	_size(0, 0),
	_mask_pos(0, 0) {
}

qdContour::qdContour(const qdContour &ct) : _contour_type(ct._contour_type),
	_size(ct._size),
	_contour(ct._contour) {
	_mask_pos = ct._mask_pos;
}

qdContour::~qdContour() {
	_contour.clear();
}

qdContour &qdContour::operator = (const qdContour &ct) {
	if (this == &ct) return *this;

	_contour_type = ct._contour_type;
	_size = ct._size;
	_mask_pos = ct._mask_pos;

	_contour = ct._contour;

	return *this;
}

void qdContour::add_contour_point(const Vect2s &pt) {
	_contour.push_back(pt);
}

void qdContour::insert_contour_point(const Vect2s &pt, int insert_pos) {
	if (insert_pos < (int)_contour.size()) {
		if (insert_pos < 0) insert_pos = 0;
		_contour.insert(_contour.begin() + insert_pos, pt);
	} else {
		_contour.push_back(pt);
	}
}

bool qdContour::remove_contour_point(int pos) {
	if (pos >= 0 && pos < (int)_contour.size()) {
		_contour.erase(_contour.begin() + pos);
		return true;
	}
	return false;
}

bool qdContour::update_contour_point(const Vect2s &pt, int pos) {
	if (pos >= 0 && pos < (int)_contour.size()) {
		_contour[pos] = pt;
		return true;
	}
	return false;
}

bool qdContour::update_contour() {
	if (_contour_type != CONTOUR_POLYGON) return false;

	if (_contour.empty())
		return false;

	int x0 = _contour[0].x;
	int x1 = _contour[0].x;
	int y0 = _contour[0].y;
	int y1 = _contour[0].y;

	for (uint i = 0; i < _contour.size(); i++) {
		if (_contour[i].x < x0) x0 = _contour[i].x;
		if (_contour[i].x > x1) x1 = _contour[i].x;
		if (_contour[i].y < y0) y0 = _contour[i].y;
		if (_contour[i].y > y1) y1 = _contour[i].y;
	}

	_size = Vect2s(x1 - x0 + 1, y1 - y0 + 1);
	_mask_pos = Vect2s(x0 + _size.x / 2, y0 + _size.y / 2);

	return true;
}

bool qdContour::is_inside(const Vect2s &pos) const {
	switch (_contour_type) {
	case CONTOUR_RECTANGLE:
		if (pos.x >= -_size.x / 2 && pos.x < _size.x / 2 && pos.y >= -_size.y / 2 && pos.y < _size.y / 2)
			return true;
		break;
	case CONTOUR_CIRCLE:
		if (pos.x * pos.x + pos.y * pos.y <= _size.x * _size.x)
			return true;
		break;
	case CONTOUR_POLYGON: {
		Vect2s p = pos;
		int intersections_lt0 = 0;
		int intersections_gt0 = 0;
		int intersections_lt1 = 0;
		int intersections_gt1 = 0;
		for (uint i = 0; i < _contour.size(); i ++) {
			Vect2s p0 = _contour[i];
			Vect2s p1 = (i < _contour.size() - 1) ? _contour[i + 1] : _contour[0];
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
	if (_contour_type == CONTOUR_POLYGON) {
		for (int i = 0; i < indent; i++) {
			fh.writeString("\t");
		}
		fh.writeString(Common::String::format("<contour_polygon>%d", contour_size() * 2));
		for (int j = 0; j < contour_size(); j++) {
			fh.writeString(Common::String::format(" %d %d", _contour[j].x, _contour[j].y));
		}
		fh.writeString("</contour_polygon>\r\n");
		return true;
	}

	if (_contour_type == CONTOUR_RECTANGLE) {
		for (int i = 0; i < indent; i++) {
			fh.writeString("\t");
		}
		fh.writeString(Common::String::format("<contour_rect>%d %d</contour_rect>\r\n", _size.x, _size.y));
		return true;
	}

	if (_contour_type == CONTOUR_CIRCLE) {
		for (int i = 0; i < indent; i++) {
			fh.writeString("\t");
		}
		fh.writeString(Common::String::format("<contour_circle>%d</contour_circle>\r\n", _size.x));
		return true;
	}

	return false;
}

bool qdContour::load_script(const xml::tag *p) {
	int i;

	Vect2s v;
	xml::tag_buffer buf(*p);

	switch (_contour_type) {
	case CONTOUR_RECTANGLE:
		buf > v.x > v.y;
		_size = Vect2s(v);
		return true;
	case CONTOUR_CIRCLE:
		_size.x = _size.y = buf.get_short();
		return true;
	case CONTOUR_POLYGON:
		_contour.reserve(p->data_size() / 2);
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
	Std::vector<Vect2s>::iterator _itr = _contour.begin(), _end = _contour.end();
	for (; _itr != _end; ++_itr) {
		Vect2s &v = *_itr;
		v.x >>= shift;
		v.y >>= shift;
	}
}

void qdContour::mult_contour(int shift) {
	Std::vector<Vect2s>::iterator _itr = _contour.begin(), _end = _contour.end();
	for (; _itr != _end; ++_itr) {
		Vect2s &v = *_itr;
		v.x <<= shift;
		v.y <<= shift;
	}
}

void qdContour::shift_contour(int dx, int dy) {
	Std::vector<Vect2s>::iterator _itr = _contour.begin(), _end = _contour.end();
	for (; _itr != _end; ++_itr) {
		Vect2s &v = *_itr;
		v.x += dx;
		v.y += dy;
	}
}

bool qdContour::can_be_closed() const {
	if (_contour_type != CONTOUR_POLYGON)
		return false;

	return (_contour.size() > 2);
}

bool qdContour::is_contour_empty() const {
	if (_contour_type == CONTOUR_POLYGON)
		return (_contour.size() < 3);

	return false;
}

} // namespace QDEngine
