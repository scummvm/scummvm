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

#include "ultima8/gumps/container_gump.h"
#include "ultima8/misc/p_dynamic_cast.h"

namespace Ultima8 {

class RenderedText;

class PaperdollGump : public ContainerGump {
public:
	ENABLE_RUNTIME_CLASSTYPE()

	PaperdollGump();
	PaperdollGump(Shape *shape, uint32 framenum, uint16 owner,
	              uint32 _Flags = FLAG_DRAGGABLE, int32 layer = LAYER_NORMAL);
	virtual ~PaperdollGump(void);

	// Init the gump, call after construction
	virtual void InitGump(Gump *newparent, bool take_focus = true);

	// Close the gump
	virtual void Close(bool no_del = false);

	// Paint this Gump
	virtual void PaintThis(RenderSurface *, int32 lerp_factor, bool scaled);

	virtual void ChildNotify(Gump *child, uint32 message);

	// Trace a click, and return ObjId
	virtual uint16 TraceObjId(int mx, int my);

	// Get the location of an item in the gump (coords relative to this).
	// Returns false on failure.
	virtual bool GetLocationOfItem(uint16 itemid, int &gx, int &gy,
	                               int32 lerp_factor = 256);

	virtual bool StartDraggingItem(Item *item, int mx, int my);
	virtual bool DraggingItem(Item *item, int mx, int my);
	virtual void DropItem(Item *item, int mx, int my);

	bool loadData(IDataSource *ids, uint32 version);
protected:
	virtual void saveData(ODataSource *ods);

	//! Paint the stats
	void PaintStats(RenderSurface *, int32 lerp_factor);

	//! Paint a single stat
	void PaintStat(RenderSurface *surf, unsigned int n,
	               std::string text, int val);

	RenderedText *cached_text[14]; // constant!!
	int cached_val[7]; // constant!!

	uint16 statbuttongid;
};

} // End of namespace Ultima8

#endif
