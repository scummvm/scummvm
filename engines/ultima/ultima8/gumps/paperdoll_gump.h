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

#ifndef ULTIMA8_GUMPS_PAPERDOLLGUMP_H
#define ULTIMA8_GUMPS_PAPERDOLLGUMP_H

#include "ultima/ultima8/gumps/container_gump.h"
#include "ultima/ultima8/misc/p_dynamic_cast.h"

namespace Ultima {
namespace Ultima8 {

class RenderedText;

/**
 * The gump activated by 'Z', which shows the avatar with current armor, and stats
 */
class PaperdollGump : public ContainerGump {
public:
	ENABLE_RUNTIME_CLASSTYPE()

	PaperdollGump();
	PaperdollGump(const Shape *shape, uint32 frameNum, uint16 owner,
	              uint32 flags = FLAG_DRAGGABLE, int32 layer = LAYER_NORMAL);
	~PaperdollGump() override;

	// Init the gump, call after construction
	void InitGump(Gump *newparent, bool take_focus = true) override;

	// Close the gump
	void Close(bool no_del = false) override;

	// Paint this Gump
	void PaintThis(RenderSurface *, int32 lerp_factor, bool scaled) override;

	void ChildNotify(Gump *child, uint32 message) override;

	// Trace a click, and return ObjId
	uint16 TraceObjId(int32 mx, int32 my) override;

	// Get the location of an item in the gump (coords relative to this).
	// Returns false on failure.
	bool GetLocationOfItem(uint16 itemid, int32 &gx, int32 &gy,
	                               int32 lerp_factor = 256) override;

	bool StartDraggingItem(Item *item, int mx, int my) override;
	bool DraggingItem(Item *item, int mx, int my) override;
	void DropItem(Item *item, int mx, int my) override;

	bool loadData(Common::ReadStream *rs, uint32 version);
	void saveData(Common::WriteStream *ws) override;

protected:
	//! Paint the stats
	void PaintStats(RenderSurface *, int32 lerp_factor);

	//! Paint a single stat
	void PaintStat(RenderSurface *surf, unsigned int n,
	               Std::string text, int val);

	RenderedText *_cachedText[14]; // constant!!
	int _cachedVal[7]; // constant!!

	uint16 _statButtonId;
private:
	const Rect _backpackRect;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
