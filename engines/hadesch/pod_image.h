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
 * Copyright 2020 Google
 *
 */
#ifndef HADESCH_POD_IMAGE_H
#define HADESCH_POD_IMAGE_H

#include "hadesch/pod_file.h"
#include "common/ptr.h"
#include "common/rect.h"
#include "common/hashmap.h"
#include "hadesch/gfx_context.h"
#include "graphics/cursor.h"

namespace Hadesch {

class PodImage : public Graphics::Cursor {
public:
	PodImage();
	bool loadImage(const PodFile &col, int index);
	void render(Common::SharedPtr<GfxContext>, Common::Point offset,
		    int colourScale = 0x100, int scale = 100) const;
	bool isValid() const;
	void setHotspot(Common::Point pnt);
	Common::Point getOffset() const;

	uint16 getWidth() const override;
	uint16 getHeight() const override;
	uint16 getHotspotX() const override;
	uint16 getHotspotY() const override;
	byte getKeyColor() const override;
	const byte *getSurface() const override;
	const byte *getPalette() const override;
	byte getPaletteStartIndex() const override;
	uint16 getPaletteCount() const override;

	~PodImage();
private:
	struct ScaledVersion {
		Common::SharedPtr<byte> _pixels;
		int _w, _h;
	};
	void makeScale(int scale) const;

	mutable Common::HashMap<int, ScaledVersion> _scales;
	int _w, _h;
	Common::Point _pos, _hotspot;
	int _ncolors;
	Common::SharedPtr<byte> _pixels;
	Common::SharedPtr<byte> _palette;
	Common::SharedPtr<byte> _paletteCursor;
};
}

#endif
