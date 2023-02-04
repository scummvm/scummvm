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

#ifndef ULTIMA8_GUMPS_READABLEGUMP_H
#define ULTIMA8_GUMPS_READABLEGUMP_H

#include "ultima/shared/std/string.h"
#include "ultima/ultima8/gumps/modal_gump.h"
#include "ultima/ultima8/usecode/intrinsics.h"
#include "ultima/ultima8/misc/classtype.h"

namespace Ultima {
namespace Ultima8 {

/**
 * The gump for popping up text in the game you can read (plaques and gravestones)
 */
class ReadableGump : public ModalGump {
	uint16 _shapeNum;
	int _fontNum;
	Std::string _text;
public:
	ENABLE_RUNTIME_CLASSTYPE()

	ReadableGump();
	ReadableGump(ObjId owner, uint16 shape, int font, const Std::string &msg);
	~ReadableGump() override;

	// Close on mouse click on key press
	Gump *onMouseDown(int button, int32 mx, int32 my) override;
	bool OnKeyDown(int key, int mod) override;

	// Init the gump, call after construction
	void InitGump(Gump *newparent, bool take_focus = true) override;

	INTRINSIC(I_readGrave);
	INTRINSIC(I_readPlaque);

	bool loadData(Common::ReadStream *rs, uint32 version);
	void saveData(Common::WriteStream *ws) override;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
