/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995-1997 Presto Studios, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef PEGASUS_GRAPHICS_H
#define PEGASUS_GRAPHICS_H

#include "common/rect.h"
#include "common/str.h"
#include "common/system.h"
#include "graphics/pict.h"
#include "graphics/surface.h"

#include "pegasus/pegasus.h"
#include "pegasus/util.h"

namespace Pegasus {

enum {
	kImageCacheSize = 10
};

struct ImageCache {
	Common::String filename;
	Graphics::Surface *surface;
	uint32 lastUsed;
};

class DisplayElement;
class PegasusEngine;

class GraphicsManager {
public:
	GraphicsManager(PegasusEngine *vm);
	~GraphicsManager();

	// Older "temporary" API
	void drawPict(Common::String filename, int x, int y, bool updateScreen = true);
	void drawPictTransparent(Common::String filename, int x, int y, uint32 transparency, bool updateScreen = true);
	uint32 getColor(byte r, byte g, byte b);

	// Newer "to-be-used" API
	void addDisplayElement(DisplayElement *element);
	void removeDisplayElement(DisplayElement *element);
	void invalRect(const Common::Rect &rect);
	tDisplayOrder getBackOfActiveLayer() const { return _backLayer; }
	tDisplayOrder getFrontOfActiveLayer() const { return _frontLayer; }
private:		
	PegasusEngine *_vm;

	// Older "temporary" API
	Graphics::PictDecoder *_pictDecoder;
	Graphics::Surface *decodeImage(const Common::String &filename);
	ImageCache _cache[kImageCacheSize];
	int getImageSlot(const Common::String &filename);

	// Newer "to-be-used" API
	Common::Rect _dirtyRect;
	tDisplayOrder _backLayer, _frontLayer;
	DisplayElement *_firstDisplayElement, *_lastDisplayElement;
};

} // End of namespace Pegasus

#endif
