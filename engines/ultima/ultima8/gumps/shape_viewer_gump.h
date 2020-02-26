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

#include "ultima/ultima8/gumps/modal_gump.h"

#include "ultima/shared/std/containers.h"
#include "ultima/ultima8/misc/p_dynamic_cast.h"

namespace Ultima {
namespace Ultima8 {

class ShapeArchive;


class ShapeViewerGump : public ModalGump {
public:
	ENABLE_RUNTIME_CLASSTYPE()

	ShapeViewerGump();
	ShapeViewerGump(int width, int height,
	                Std::vector<Std::pair<Std::string, ShapeArchive *> > &flexes,
	                uint32 flags = 0, int32 layer = LAYER_MODAL);
	~ShapeViewerGump() override;

	void PaintThis(RenderSurface *, int32 lerp_factor, bool scaled) override;

	bool OnKeyDown(int key, int mod) override;
	bool OnTextInput(int unicode) override;

	static void U8ShapeViewer();

	bool loadData(IDataSource *ids);
protected:
	void saveData(ODataSource *ods) override;

	Std::vector<Std::pair<Std::string, ShapeArchive *> > _flexes;
	unsigned int _curFlex;
	ShapeArchive *_flex;
	uint32 _curShape;
	uint32 _curFrame;

	uint32 _background;

	int32 _shapeW, _shapeH, _shapeX, _shapeY;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
