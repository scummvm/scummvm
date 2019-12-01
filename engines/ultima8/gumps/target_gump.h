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

#ifndef ULTIMA8_GUMPS_TARGETGUMP_H
#define ULTIMA8_GUMPS_TARGETGUMP_H

#include "ultima8/gumps/modal_gump.h"
#include "ultima8/usecode/intrinsics.h"
#include "ultima8/misc/p_dynamic_cast.h"

namespace Ultima8 {

class UCProcess;

class TargetGump : public ModalGump {
public:
	ENABLE_RUNTIME_CLASSTYPE()

	TargetGump();
	TargetGump(int x, int y);

	virtual ~TargetGump(void);

	virtual bool PointOnGump(int mx, int my);
	virtual void InitGump(Gump *newparent, bool take_focus = true);
	virtual void Close(bool no_del = false);
	virtual void PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled);
	virtual void OnMouseUp(int button, int mx, int my);

	INTRINSIC(I_target);

	bool loadData(IDataSource *ids, uint32 version);
protected:
	virtual void saveData(ODataSource *ods);

private:
	bool target_tracing;
};

} // End of namespace Ultima8

#endif
