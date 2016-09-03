/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef PRINCE_MOB_H
#define PRINCE_MOB_H

#include "common/scummsys.h"
#include "common/rect.h"
#include "common/str.h"
#include "common/stream.h"

#include "prince/common.h"

namespace Prince {

class Mob {
public:

	Mob() : _name(""), _examText(""), _visible(false), _type(0), _mask(0), _examDirection(kDirL), _useDirection(kDirL) {}

	bool loadFromStream(Common::SeekableReadStream &stream);

	// Used instead of offset in setData and getData
	enum AttrId {
		kMobVisible = 0,
		kMobExamX = 14,
		kMobExamY = 16,
		kMobExamDir = 18
	};

	void setData(AttrId dataId, uint16 value);
	uint16 getData(AttrId dataId);

	bool _visible;
	uint16 _type;
	uint16 _mask;
	Common::Rect _rect;

	Common::Point _examPosition;
	Direction _examDirection;

	Common::Point _usePosition;
	Direction _useDirection;

	Common::String _name;
	Common::String _examText;
};

} // End of namespace Prince

#endif
