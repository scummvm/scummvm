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

#ifndef ULTIMA8_GUMPS_BARKGUMP_H
#define ULTIMA8_GUMPS_BARKGUMP_H

#include "ultima/ultima8/gumps/item_relative_gump.h"
#include "ultima/ultima8/misc/p_dynamic_cast.h"

namespace Ultima {
namespace Ultima8 {

/**
* Represents text which appears on the screen to show the name of an item, etc
*/
class BarkGump : public ItemRelativeGump {
protected:
	Std::string _barked;
	int32 _counter;
	ObjId _textWidget;
	uint32 _speechShapeNum;
	uint32 _speechLength;
	uint32 _totalTextHeight;

public:
	ENABLE_RUNTIME_CLASSTYPE()

	BarkGump();
	BarkGump(uint16 owner, const Std::string &msg, uint32 speechShapeNum = 0);
	~BarkGump() override;

	// Run the gump (decrement the counter)
	void        run() override;

	// Got to the next page on mouse click
	Gump       *onMouseDown(int button, int32 mx, int32 my) override;

	// Init the gump, call after construction
	void        InitGump(Gump *newparent, bool take_focus = true) override;

	/// Get the font that should be used from dialog from this actor
	static int dialogFontForActor(uint16 actor);

protected:
	//! show next text.
	//! returns false if no more text available
	bool NextText();

	int _textDelay;

public:
	bool loadData(Common::ReadStream *rs, uint32 version);
	void saveData(Common::WriteStream *ws) override;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
