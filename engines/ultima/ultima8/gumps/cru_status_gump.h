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

#ifndef ULTIMA8_GUMPS_CRUSTATUSGUMP_H
#define ULTIMA8_GUMPS_CRUSTATUSGUMP_H

#include "ultima/ultima8/gumps/gump.h"
#include "ultima/ultima8/usecode/intrinsics.h"
#include "ultima/ultima8/misc/p_dynamic_cast.h"

namespace Ultima {
namespace Ultima8 {

/**
 * Represents the collection of status boxes along the bottom of the screen.
 * Each of the individual items is a class of CruStatGump.
 */
class CruStatusGump : public Gump {
public:
	ENABLE_RUNTIME_CLASSTYPE()

	// default constructor for save game loading
	CruStatusGump();

	// need a parameter to differentiate the non-default constructor..
	CruStatusGump(bool unused);

	~CruStatusGump() override;

	// Init the gump, call after construction
	void InitGump(Gump *newparent, bool take_focus = true) override;

	// Paint this Gump
	void PaintThis(RenderSurface *, int32 lerp_factor, bool scaled) override;

	bool loadData(Common::ReadStream *rs, uint32 version);
	void saveData(Common::WriteStream *ws) override;

	void createStatusItems();

	static CruStatusGump *get_instance() {
		return _instance;
	}

	INTRINSIC(I_hideStatusGump);
	INTRINSIC(I_showStatusGump);

private:
	static CruStatusGump *_instance;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
