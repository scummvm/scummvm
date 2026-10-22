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

#include "engines/nancy/nancy.h"
#include "engines/nancy/resource.h"

#include "engines/nancy/script/api.h"
#include "engines/nancy/script/luabind.h"
#include "engines/nancy/script/ref.h"
#include "engines/nancy/script/types.h"

namespace Nancy {
namespace Script {

static const char *const kRefType = "ND.Ref";

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

// What a Ref object of the scripts holds. In Nancy19 that is one image; Nancy20
// lays any number of images side by side, and has a color for the places none
// of them covers.
struct ScriptRef {
	struct Layer {
		RefImage image;
		ScriptRect rect;
	};

	~ScriptRef();

	bool getPixel(int x, int y, uint32 &argb) const;

	Common::Array<Layer *> layers;
	RefEdgeMode edgeMode = kRefClamp;
	bool hasBorder = false;	// Nancy20
	uint32 borderColor = 0x7FFFFFFF;
};

ScriptRef::~ScriptRef() {
	for (Layer *layer : layers) {
		delete layer;
	}
}

bool ScriptRef::getPixel(int x, int y, uint32 &argb) const {
	if (!hasBorder) {
		return !layers.empty() && layers[0]->image.getPixel(x, y, argb);
	}

	for (const Layer *layer : layers) {
		if (layer->rect.contains(x, y)) {
			return layer->image.getPixel(x - layer->rect.left, y - layer->rect.top, argb);
		}
	}

	argb = borderColor;
	return true;
}

static ScriptRef *checkRef(lua_State *L, int index) {
	return *(ScriptRef **)checkUserData(L, index, kRefType);
}

// Puts an image into a Nancy20 Ref, unless it would overlap one that is in it
// already. An empty rectangle stands for the size of the image.
static bool addRefLayer(ScriptRef &ref, const Common::String &imageName, const ScriptRect &rect) {
	ScriptRef::Layer *layer = new ScriptRef::Layer();
	layer->image.edgeMode = ref.edgeMode;
	if (!g_nancy->_resource->loadImage(Common::Path(imageName), layer->image.image)) {
		warning("Could not load Ref %s", imageName.c_str());
	}

	layer->rect = rect;
	if (rect.isEmpty()) {
		layer->rect = ScriptRect(0, 0, layer->image.image.w, layer->image.image.h);
	}

	for (const ScriptRef::Layer *other : ref.layers) {
		const ScriptRect &a = other->rect;
		const ScriptRect &b = layer->rect;
		if (MAX(a.left, b.left) < MIN(a.right, b.right) && MAX(a.top, b.top) < MIN(a.bottom, b.bottom)) {
			delete layer;
			return false;
		}
	}

	ref.layers.push_back(layer);
	return true;
}

static ScriptPoint checkPlace(lua_State *L, int index) {
	ScriptPoint point;
	if (!readPoint(L, index, point)) {
		luaL_argerror(L, index, "Single parameter must be a Point, a Vector or a table with x and y fields");
	}

	return point;
}

// Ref:New(image, edgeMode) in Nancy19. Nancy20 takes a table instead:
// Ref:New({ mode = "clamp" or "wrap", borderColor = color, ref = image })
static int refNew(lua_State *L) {
	ScriptRef *ref = new ScriptRef();

	if (lua_istable(L, 2)) {
		ref->hasBorder = true;

		Common::String edgeMode = "clamp";
		readField(L, 2, "mode", edgeMode);
		if (edgeMode == "wrap") {
			ref->edgeMode = kRefWrap;
		} else if (edgeMode != "clamp") {
			delete ref;
			return luaL_argerror(L, 2, "Ref mode unrecognized");
		}

		if (pushField(L, 2, "borderColor")) {
			ref->borderColor = checkColor(L, -1);
			lua_pop(L, 1);
		}

		Common::String imageName;
		if (readField(L, 2, "ref", imageName)) {
			addRefLayer(*ref, imageName, ScriptRect());
		}
	} else {
		const char *imageName = luaL_checkstring(L, 2);
		const Common::String edgeMode = luaL_optstring(L, 3, "clamp");

		if (edgeMode == "wrap") {
			ref->edgeMode = kRefWrap;
		} else if (edgeMode == "dismiss") {
			ref->edgeMode = kRefDismiss;
		}

		ScriptRef::Layer *layer = new ScriptRef::Layer();
		layer->image.edgeMode = ref->edgeMode;
		g_nancy->_resource->loadImage(Common::Path(imageName), layer->image.image);
		ref->layers.push_back(layer);
	}

	ScriptRef **object = (ScriptRef **)pushUserData(L, sizeof(ScriptRef *), kRefType);
	*object = ref;
	return 1;
}

static int refCollect(lua_State *L) {
	ScriptRef **object = (ScriptRef **)checkUserData(L, 1, kRefType);
	delete *object;
	*object = nullptr;
	return 0;
}

// ref:Add(image, rect), Nancy20. Returns false when the image would overlap
// one that is in the Ref already.
static int refAdd(lua_State *L) {
	ScriptRef *ref = checkRef(L, 1);
	const char *imageName = luaL_checkstring(L, 2);
	const ScriptRect &rect = checkRect(L, 3);

	lua_pushboolean(L, addRefLayer(*ref, imageName, rect));
	return 1;
}

// ref:Remove(rect), Nancy20, which takes out the image put at that rectangle
static int refRemove(lua_State *L) {
	ScriptRef *ref = checkRef(L, 1);
	const ScriptRect &rect = checkRect(L, 2);

	for (uint i = 0; i < ref->layers.size(); ++i) {
		if (ref->layers[i]->rect == rect) {
			delete ref->layers[i];
			ref->layers.remove_at(i);
			lua_pushboolean(L, true);
			return 1;
		}
	}

	lua_pushboolean(L, false);
	return 1;
}

// ref:At(place), Nancy20, the color there
static int refAt(lua_State *L) {
	const ScriptRef *ref = checkRef(L, 1);
	const ScriptPoint place = checkPlace(L, 2);

	uint32 pixel = 0;
	ref->getPixel(place.x, place.y, pixel);
	pushColor(L, pixel);
	return 1;
}

// ref:IsAt(place, color)
static int refIsAt(lua_State *L) {
	const ScriptRef *ref = checkRef(L, 1);
	const ScriptPoint place = checkPlace(L, 2);
	const uint32 color = checkColor(L, 3);

	uint32 pixel;
	lua_pushboolean(L, ref->getPixel(place.x, place.y, pixel) && pixel == color);
	return 1;
}

// ref:IsOpaqueAt(place)
static int refIsOpaqueAt(lua_State *L) {
	const ScriptRef *ref = checkRef(L, 1);
	const ScriptPoint place = checkPlace(L, 2);

	uint32 pixel;
	lua_pushboolean(L, ref->getPixel(place.x, place.y, pixel) && (pixel >> 24) != 0);
	return 1;
}

// How much of the square of the given size around a place passes a test, from 0 to 1.
// The places that have no pixel count as failing it.
static double getAreaShare(const ScriptRef *ref, const ScriptPoint &place, int size, bool testColor, uint32 color) {
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
	const ScriptRef *ref = checkRef(L, 1);
	const ScriptPoint place = checkPlace(L, 2);
	const uint32 color = checkColor(L, 3);
	const int size = luaL_checkint(L, 4);

	lua_pushnumber(L, getAreaShare(ref, place, size, true, color));
	return 1;
}

// ref:IsOpaqueInArea(place, size)
static int refIsOpaqueInArea(lua_State *L) {
	const ScriptRef *ref = checkRef(L, 1);
	const ScriptPoint place = checkPlace(L, 2);
	const int size = luaL_checkint(L, 3);

	lua_pushnumber(L, getAreaShare(ref, place, size, false, 0));
	return 1;
}

void registerRefAPI(lua_State *L) {
	static const luaL_Reg kRefFunctions[] = {
		{ "__index", indexFunctions },
		{ "__gc", refCollect },
		{ "Add", refAdd },
		{ "Remove", refRemove },
		{ "At", refAt },
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
