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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

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

	Mob() : _name(""), _examText("") {}

	bool loadFromStream(Common::SeekableReadStream &stream);

	// Used instead of offset in setData and getData
	enum AttrId {
		Visible = 0,
		Type = 2,
		X1 = 4,
		Y1 = 6,
		X2 = 8,
		Y2 = 10,
		Mask = 12,
		ExamX = 14,
		ExamY = 16,
		ExamDir = 18,
		UseX = 20,
		UseY = 21,
		UseDir = 22,
		Name = 24,
		ExamText = 28 
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
