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

#ifndef ULTIMA8_GUMPS_ASKGUMP_H
#define ULTIMA8_GUMPS_ASKGUMP_H

#include "ultima/ultima8/gumps/item_relative_gump.h"
#include "ultima/ultima8/misc/p_dynamic_cast.h"

namespace Ultima {
namespace Ultima8 {

class UCList;

/**
 * Represents asking a question while talking to an NPC
 */
class AskGump : public ItemRelativeGump {
	UCList *_answers;
public:
	// p_dynamic_class stuff
	ENABLE_RUNTIME_CLASSTYPE()

	AskGump();
	AskGump(uint16 owner, UCList *answers);
	~AskGump() override;

	// Init the gump, call after construction
	void        InitGump(Gump *newparent, bool take_focus = true) override;

	void        ChildNotify(Gump *child, uint32 message) override;

	bool loadData(Common::ReadStream *rs, uint32 version);
	void saveData(Common::WriteStream *ws) override;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
