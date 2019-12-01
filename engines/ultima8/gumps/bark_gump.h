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

#include "ultima8/gumps/item_relative_gump.h"
#include "ultima8/misc/p_dynamic_cast.h"

namespace Ultima8 {

class BarkGump : public ItemRelativeGump {
protected:
	std::string barked;
	int32 counter;
	ObjId textwidget;
	uint32 speechshapenum;
	uint32 speechlength;
	uint32 totaltextheight;

public:
	ENABLE_RUNTIME_CLASSTYPE();

	BarkGump();
	BarkGump(uint16 owner, std::string msg, uint32 speechshapenum = 0);
	virtual ~BarkGump(void);

	// Run the gump (decrement the counter)
	virtual void        run();

	// Got to the next page on mouse click
	virtual Gump       *OnMouseDown(int button, int mx, int my);

	// Init the gump, call after construction
	virtual void        InitGump(Gump *newparent, bool take_focus = true);

protected:
	//! show next text.
	//! returns false if no more text available
	bool NextText();

	int textdelay;

public:
	bool loadData(IDataSource *ids, uint32 version);
protected:
	virtual void saveData(ODataSource *ods);
};

} // End of namespace Ultima8

#endif
