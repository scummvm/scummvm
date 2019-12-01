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

#ifndef ULTIMA8_GUMPS_SHAPEVIEWERGUMP_H
#define ULTIMA8_GUMPS_SHAPEVIEWERGUMP_H

#include "modal_gump.h"

#include "ultima8/std/containers.h"
#include "ultima8/misc/p_dynamic_cast.h"

namespace Ultima8 {

class ShapeArchive;


class ShapeViewerGump : public ModalGump {
public:
	ENABLE_RUNTIME_CLASSTYPE();

	ShapeViewerGump();
	ShapeViewerGump(int width, int height,
	                std::vector<std::pair<std::string, ShapeArchive *> > &flexes,
	                uint32 _Flags = 0, int32 layer = LAYER_MODAL);
	virtual ~ShapeViewerGump(void);

	virtual void PaintThis(RenderSurface *, int32 lerp_factor, bool scaled);

	virtual bool OnKeyDown(int key, int mod);
	virtual bool OnTextInput(int unicode);

	static void U8ShapeViewer();
	static void ConCmd_U8ShapeViewer(const Console::ArgvType &argv);

	bool loadData(IDataSource *ids);
protected:
	virtual void saveData(ODataSource *ods);

	std::vector<std::pair<std::string, ShapeArchive *> > flexes;
	unsigned int curflex;
	ShapeArchive *flex;
	uint32 curshape;
	uint32 curframe;

	uint32 background;

	int32 shapew, shapeh, shapex, shapey;
};

} // End of namespace Ultima8

#endif
