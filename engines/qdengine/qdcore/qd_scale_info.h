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

#ifndef QDENGINE_QDCORE_QD_SCALE_INFO_H
#define QDENGINE_QDCORE_QD_SCALE_INFO_H

#include "qdengine/parser/xml_fwd.h"
#include "qdengine/qdcore/qd_named_object.h"


namespace QDEngine {

class qdScaleInfo : public qdNamedObject {
public:
	qdScaleInfo() : _scale(1.0f) { }
	qdScaleInfo(const qdScaleInfo &sc);
	~qdScaleInfo() { }

	int named_object_type() const {
		return QD_NAMED_OBJECT_SCALE_INFO;
	}

	float scale() const {
		return _scale;
	}
	void set_scale(float sc) {
		_scale = sc;
	}

	void load_script(const xml::tag *p);
	bool save_script(Common::WriteStream &fh, int indent = 0) const;

private:

	float _scale;
};

typedef Std::list<qdScaleInfo *> qdScaleInfoList;

} // namespace QDEngine

#endif // QDENGINE_QDCORE_QD_SCALE_INFO_H
