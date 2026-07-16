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

/*************************************
 *
 * USED IN:
 * Night Light
 *
 *************************************/
/*
-- HitMap XObject
ISIII   mNew, path, xOffset, yOffset, scale -- path, xOffset, yOffset, scale
X       mDispose
III     mWhere, x, y                        -- Return GWorld Index

-- Hitmap Scale Factor 27.JUL.94
hitmap
S       mName                                               --Returns the XObject name (hitmap)
S       mGetSys                                             --Returns Windows System Directory Path(hitmap)
ISIII   mNew, hitBitMapfile, hitmaposx, hitmaposy, factor   --Creates new instance of XObject
III     mWhere, xpos, ypos                                  --coordinate position
X       mDispose                                            --Dispose of memory allocation
*/

#include "common/file.h"
#include "common/system.h"
#include "graphics/surface.h"
#include "image/bmp.h"

#include "director/director.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-utils.h"
#include "director/lingo/lingo.h"
#include "director/lingo/xlibs/h/hitmap.h"

namespace Director {

const char *const HitMapXObj::xlibName = "HitMap";
const XlibFileDesc HitMapXObj::fileNames[] = {
	{"HitMap", nullptr},
	{"maskXobj", nullptr},
	{nullptr, nullptr},
};

static const MethodProto xlibMethods[] = {
	{"new",		HitMapXObj::m_new,	   4, 4, 400},  // D4
	{"where",	HitMapXObj::m_where,   2, 2, 400},  // D4
	{"name",	HitMapXObj::m_name,	   0, 0, 400},  // D4
	{"getSys",  HitMapXObj::m_getSys,  0, 0, 400},  // D4
	{"dispose", HitMapXObj::m_dispose, 0, 0, 400},  // D4
	{nullptr,	nullptr,			   0, 0, 0  }
};

void HitMapXObj::open(ObjectType type, const Common::Path &path) {
	if (type == kXObj) {
		HitMapXObject::initMethods(xlibMethods);
		HitMapXObject *xobj = new HitMapXObject(kXObj);
		g_lingo->exposeXObject(xlibName, xobj);
	}
}

void HitMapXObj::close(ObjectType type) {
	if (type == kXObj) {
		HitMapXObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
	}
}

HitMapXObject::HitMapXObject(ObjectType objType) : Object<HitMapXObject>("HitMap") {
	_objType = objType;
	_width = 0;
	_height = 0;
	_xOffset = 0;
	_yOffset = 0;
	_scale = 1;
}

HitMapXObject::~HitMapXObject() {
	_bitmapData.clear();
}

void HitMapXObj::m_new(int nargs) {
	HitMapXObject *me = static_cast<HitMapXObject *>(g_lingo->_state->me.u.obj);

	// Pop arguments in reverse order (factor, yOffset, xOffset, filename)
	int scale = g_lingo->pop().asInt();
	int yOffset = g_lingo->pop().asInt();
	int xOffset = g_lingo->pop().asInt();
	Common::String fileName = g_lingo->pop().asString();

	debugC(5, kDebugLingoExec, "HitMapXObj::m_new: file='%s', x=%d, y=%d, scale=%d",
		   fileName.c_str(), xOffset, yOffset, scale);

	// Store parameters
	me->_xOffset = xOffset;
	me->_yOffset = yOffset;
	me->_scale = scale > 0 ? scale : 1;

	// Try to open the file
	Common::File bmpFile;
	if (!bmpFile.open(findPath(fileName))) {
		warning("HitMapXObj::m_new: Could not open file '%s'", fileName.c_str());
		g_lingo->push(g_lingo->_state->me);
		return;
	}

	// Load bitmap using BMP decoder
	Image::BitmapDecoder bmpDecoder;
	if (!bmpDecoder.loadStream(bmpFile)) {
		warning("HitMapXObj::m_new: Could not decode bitmap '%s'", fileName.c_str());
		bmpFile.close();
		g_lingo->push(g_lingo->_state->me);
		return;
	}

	// Get the decoded surface
	const Graphics::Surface *surface = bmpDecoder.getSurface();
	if (!surface) {
		warning("HitMapXObj::m_new: No surface data in bitmap '%s'", fileName.c_str());
		bmpFile.close();
		g_lingo->push(g_lingo->_state->me);
		return;
	}

	// Store dimensions
	me->_width = surface->w;
	me->_height = surface->h;

	debugC(5, kDebugLingoExec, "HitMapXObj::m_new: Loaded bitmap %dx%d", me->_width, me->_height);

	// Allocate hitmap data buffer
	uint32 dataSize = me->_width * me->_height;
	me->_bitmapData.resize(dataSize);

	// Convert surface to byte array for hitmap lookup
	const byte *srcPixels = (const byte *)surface->getPixels();

	if (surface->format.bytesPerPixel == 1) {
		// Direct copy for 8-bit indexed color
		memcpy(me->_bitmapData.data(), srcPixels, dataSize);
	} else {
		// Extract first byte for higher bit depths
		for (uint32 i = 0; i < dataSize; i++) {
			me->_bitmapData[i] = srcPixels[i * surface->format.bytesPerPixel];
		}
	}

	bmpFile.close();

	// Return the object instance
	g_lingo->push(g_lingo->_state->me);
}

void HitMapXObj::m_where(int nargs) {
	HitMapXObject *me = static_cast<HitMapXObject *>(g_lingo->_state->me.u.obj);

	// Pop arguments (y, then x due to stack order)
	int screenY = g_lingo->pop().asInt();
	int screenX = g_lingo->pop().asInt();

	// Check if hitmap was loaded
	if (me->_bitmapData.empty() || me->_width == 0 || me->_height == 0) {
		warning("HitMapXObj::m_where: No hitmap data loaded");
		g_lingo->push(Datum(-1));
		return;
	}

	// Calculate scaled dimensions for bounds checking
	int scaledWidth = me->_width * me->_scale;
	int scaledHeight = me->_height * me->_scale;

	// Check if coordinates are out of bounds
	if (screenX < 0 || screenX >= scaledWidth ||
		screenY < 0 || screenY >= scaledHeight) {
		debugC(5, kDebugLingoExec, "HitMapXObj::m_where: Out of bounds (%d, %d)", screenX, screenY);
		g_lingo->push(Datum(-1));
		return;
	}

	// Adjust coordinates by the object's position offset
	int adjustedX = screenX - me->_xOffset;
	int adjustedY = screenY - me->_yOffset;

	// Convert from screen coordinates to bitmap coordinates
	int bitmapX = adjustedX / me->_scale;
	int bitmapY = adjustedY / me->_scale;

	// Additional bounds check after adjustment
	if (bitmapX < 0 || bitmapX >= (int)me->_width ||
		bitmapY < 0 || bitmapY >= (int)me->_height) {
		debugC(5, kDebugLingoExec, "HitMapXObj::m_where: Adjusted coords out of bounds (%d, %d)", bitmapX, bitmapY);
		g_lingo->push(Datum(-1));
		return;
	}

	// Calculate offset into bitmap data
	// BMP format is bottom-up, so invert Y coordinate
	// Formula: x * width + (width - y - 1)
	uint32 offset = bitmapX * me->_width + (me->_width - bitmapY - 1);

	// Final safety check
	if (offset >= me->_bitmapData.size()) {
		warning("HitMapXObj::m_where: Calculated offset %d exceeds data size %d", offset, me->_bitmapData.size());
		g_lingo->push(Datum(-1));
		return;
	}

	// Get and return the hitmap value
	byte value = me->_bitmapData[offset];
	debugC(5, kDebugLingoExec, "HitMapXObj::m_where: (%d, %d) -> bitmap(%d, %d) = %d",
		   screenX, screenY, bitmapX, bitmapY, value);

	g_lingo->push(Datum((int)value));
}

void HitMapXObj::m_name(int nargs) {
	g_lingo->push(Datum("hitmap_name"));
}

void HitMapXObj::m_getSys(int nargs) {
	// Get current working directory
	Common::String sysDir = g_director->getCurrentPath();

	if (sysDir.empty()) {
		// Original Windows implementation returned "Aw_Shishi" on failure
		g_lingo->push(Datum("Aw_Shishi"));
	} else {
		g_lingo->push(Datum(sysDir));
	}
}

void HitMapXObj::m_dispose(int nargs) {
	HitMapXObject *me = static_cast<HitMapXObject *>(g_lingo->_state->me.u.obj);

	// Clear bitmap data
	me->_bitmapData.clear();
	me->_width = 0;
	me->_height = 0;

	debugC(5, kDebugLingoExec, "HitMapXObj::m_dispose: Freed hitmap data");
}

} // End of namespace Director
