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

#ifndef ULTIMA8_GUMPS_DESKTOPGUMP_H
#define ULTIMA8_GUMPS_DESKTOPGUMP_H

#include "ultima8/gumps/gump.h"
#include "ultima8/misc/p_dynamic_cast.h"

namespace Ultima8 {

class DesktopGump : public Gump {
	static bool faded_modal;
public:
	ENABLE_RUNTIME_CLASSTYPE();

	DesktopGump();
	DesktopGump(int32 x, int32 y, int32 width, int32 height);
	virtual ~DesktopGump(void);

	virtual void PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled);
	virtual void PaintChildren(RenderSurface *surf, int32 lerp_factor, bool scaled);

	virtual bool StartDraggingChild(Gump *gump, int mx, int my);
	virtual void DraggingChild(Gump *gump, int mx, int my);
	virtual void StopDraggingChild(Gump *gump);

	bool loadData(IDataSource *ids, uint32 version);

	void RenderSurfaceChanged(RenderSurface *surf);

	static void SetFadedModal(bool set) {
		faded_modal = set;
	}

protected:
	virtual void saveData(ODataSource *ods);
	virtual void RenderSurfaceChanged();
};

} // End of namespace Ultima8

#endif
