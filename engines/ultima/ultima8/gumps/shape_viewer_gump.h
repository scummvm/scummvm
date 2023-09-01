/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ULTIMA8_GUMPS_SHAPEVIEWERGUMP_H
#define ULTIMA8_GUMPS_SHAPEVIEWERGUMP_H

#include "ultima/ultima8/gumps/modal_gump.h"

#include "ultima/shared/std/containers.h"
#include "ultima/ultima8/misc/classtype.h"

namespace Ultima {
namespace Ultima8 {

class ShapeArchive;

/**
 * A tool for viewing the shapes in the game, for debugging purposes.
 */
class ShapeViewerGump : public ModalGump {
public:
	struct ShapeArchiveEntry {
		Common::String _name;
		ShapeArchive *_archive;
		DisposeAfterUse::Flag _disposeAfterUse;

		ShapeArchiveEntry(const char *name, ShapeArchive *archive, DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::NO)
			: _name(name), _archive(archive), _disposeAfterUse(disposeAfterUse) {}
	};

	ENABLE_RUNTIME_CLASSTYPE()

	ShapeViewerGump();
	ShapeViewerGump(int x, int y, int width, int height,
					Common::Array<ShapeArchiveEntry> &archives,
					uint32 flags = FLAG_PREVENT_SAVE, int32 layer = LAYER_MODAL);
	~ShapeViewerGump() override;

	void PaintThis(RenderSurface *, int32 lerp_factor, bool scaled) override;

	bool OnKeyDown(int key, int mod) override;

	// Init the gump, call after construction
	void InitGump(Gump *newparent, bool take_focus = true) override;

	static void U8ShapeViewer();

	bool loadData(Common::ReadStream *rs);
	void saveData(Common::WriteStream *ws) override;

protected:
	Common::Array<ShapeArchiveEntry> _archives;
	unsigned int _curArchive;
	uint32 _curShape;
	uint32 _curFrame;

	uint32 _background;

	//! The font used in the shape viewer
	uint32 _fontNo;

	bool _showGrid;
	bool _mirrored;

	int32 _shapeW, _shapeH, _shapeX, _shapeY;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
