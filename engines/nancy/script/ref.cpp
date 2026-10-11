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

#include "graphics/managed_surface.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/resource.h"

#include "engines/nancy/script/api.h"
#include "engines/nancy/script/luabind.h"
#include "engines/nancy/script/types.h"

namespace Nancy {
namespace Script {

// A "Ref" is an image scripts look things up in instead of showing it: the
// colors of its pixels say what is where. The map the player walks around on
// has one for every part of it, painted in the colors of where one can go.

static const char *const kRefType = "ND.Ref";

// What to do about a place that is outside of the image
enum RefEdgeMode {
	kRefClamp	= 0,	// take the nearest pixel
	kRefWrap	= 1,	// start over at the other side
	kRefDismiss	= 2		// there is nothing there
};

struct RefImage {
	Graphics::ManagedSurface image;
	RefEdgeMode edgeMode = kRefClamp;

	// Returns false when there is no pixel at that place
	bool getPixel(int x, int y, uint32 &argb) const;
};

bool RefImage::getPixel(int x, int y, uint32 &argb) const {
	if (image.w == 0 || image.h == 0) {
		return false;
	}

	switch (edgeMode) {
	case kRefClamp:
		x = CLIP<int>(x, 0, image.w - 1);
		y = CLIP<int>(y, 0, image.h - 1);
		break;
	case kRefWrap:
		x = ((x % image.w) + image.w) % image.w;
		y = ((y % image.h) + image.h) % image.h;
		break;
	case kRefDismiss:
		if (x < 0 || y < 0 || x >= image.w || y >= image.h) {
			return false;
		}

		break;
	}

	byte a, r, g, b;
	image.format.colorToARGB(image.getPixel(x, y), a, r, g, b);
	argb = ((uint32)a << 24) | ((uint32)r << 16) | ((uint32)g << 8) | b;
	return true;
}

static RefImage *checkRef(lua_State *L, int index) {
	return *(RefImage **)checkUserData(L, index, kRefType);
}

static ScriptPoint checkPlace(lua_State *L, int index) {
	ScriptPoint point;
	if (!readPoint(L, index, point)) {
		luaL_argerror(L, index, "Single parameter must be a Point, a Vector or a table with x and y fields");
	}

	return point;
}

// Ref:New(image, edgeMode)
static int refNew(lua_State *L) {
	const char *imageName = luaL_checkstring(L, 2);
	const Common::String edgeMode = luaL_optstring(L, 3, "clamp");

	RefImage *ref = new RefImage();
	if (edgeMode == "wrap") {
		ref->edgeMode = kRefWrap;
	} else if (edgeMode == "dismiss") {
		ref->edgeMode = kRefDismiss;
	}

	g_nancy->_resource->loadImage(Common::Path(imageName), ref->image);

	RefImage **object = (RefImage **)pushUserData(L, sizeof(RefImage *), kRefType);
	*object = ref;
	return 1;
}

static int refCollect(lua_State *L) {
	RefImage **object = (RefImage **)checkUserData(L, 1, kRefType);
	delete *object;
	*object = nullptr;
	return 0;
}

// ref:IsAt(place, color)
static int refIsAt(lua_State *L) {
	const RefImage *ref = checkRef(L, 1);
	const ScriptPoint place = checkPlace(L, 2);
	const uint32 color = checkColor(L, 3);

	uint32 pixel;
	lua_pushboolean(L, ref->getPixel(place.x, place.y, pixel) && pixel == color);
	return 1;
}

// ref:IsOpaqueAt(place)
static int refIsOpaqueAt(lua_State *L) {
	const RefImage *ref = checkRef(L, 1);
	const ScriptPoint place = checkPlace(L, 2);

	uint32 pixel;
	lua_pushboolean(L, ref->getPixel(place.x, place.y, pixel) && (pixel >> 24) != 0);
	return 1;
}

// How much of the square of the given size around a place passes a test, from 0 to 1.
// The places that have no pixel count as failing it.
static double getAreaShare(const RefImage *ref, const ScriptPoint &place, int size, bool testColor, uint32 color) {
	if (size <= 0) {
		return 0.0;
	}

	const int left = place.x - size / 2;
	const int top = place.y - size / 2;
	int numMatches = 0;

	for (int y = top; y < top + size; ++y) {
		for (int x = left; x < left + size; ++x) {
			uint32 pixel;
			if (!ref->getPixel(x, y, pixel)) {
				continue;
			}

			if (testColor ? pixel == color : (pixel >> 24) != 0) {
				++numMatches;
			}
		}
	}

	return (double)numMatches / (size * size);
}

// ref:IsInArea(place, color, size)
static int refIsInArea(lua_State *L) {
	const RefImage *ref = checkRef(L, 1);
	const ScriptPoint place = checkPlace(L, 2);
	const uint32 color = checkColor(L, 3);
	const int size = luaL_checkint(L, 4);

	lua_pushnumber(L, getAreaShare(ref, place, size, true, color));
	return 1;
}

// ref:IsOpaqueInArea(place, size)
static int refIsOpaqueInArea(lua_State *L) {
	const RefImage *ref = checkRef(L, 1);
	const ScriptPoint place = checkPlace(L, 2);
	const int size = luaL_checkint(L, 3);

	lua_pushnumber(L, getAreaShare(ref, place, size, false, 0));
	return 1;
}

void registerRefAPI(lua_State *L) {
	static const luaL_Reg kRefFunctions[] = {
		{ "__index", indexFunctions },
		{ "__gc", refCollect },
		{ "IsAt", refIsAt },
		{ "IsOpaqueAt", refIsOpaqueAt },
		{ "IsInArea", refIsInArea },
		{ "IsOpaqueInArea", refIsOpaqueInArea },
		{ nullptr, nullptr }
	};

	static const luaL_Reg kRefHelperFunctions[] = {
		{ "__index", indexFunctions },
		{ "New", refNew },
		{ nullptr, nullptr }
	};

	registerType(L, kRefType, kRefFunctions);
	registerGlobal(L, "Ref", "ND.RefHelper", kRefHelperFunctions);
}

} // End of namespace Script
} // End of namespace Nancy
