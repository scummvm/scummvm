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

#include "common/hashmap.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/resource/resource.h"

#include "engines/nancy/script/api.h"
#include "engines/nancy/script/luabind.h"
#include "engines/nancy/script/records.h"
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
static bool addRefLayer(ScriptRef &ref, const Common::String &imageName, const Common::String &treeName, const ScriptRect &rect) {
	ScriptRef::Layer *layer = new ScriptRef::Layer();
	layer->image.edgeMode = ref.edgeMode;
	if (!loadTreeImage(imageName, treeName, layer->image.image)) {
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
			addRefLayer(*ref, imageName, Common::String(), ScriptRect());
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
		layer->rect = ScriptRect(0, 0, layer->image.image.w, layer->image.image.h);
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
// one that is in the Ref already. Nancy23 names the tree of the image too,
// ref:Add(image, tree, rect), which lets the calligraphy puzzle look at a
// picture the camera took of what the player drew.
static int refAdd(lua_State *L) {
	ScriptRef *ref = checkRef(L, 1);
	const char *imageName = luaL_checkstring(L, 2);

	Common::String treeName;
	int rectIndex = 3;
	if (g_nancy->getGameType() >= kGameTypeNancy23) {
		treeName = luaL_checkstring(L, 3);
		rectIndex = 4;
	}

	const ScriptRect &rect = checkRect(L, rectIndex);
	lua_pushboolean(L, addRefLayer(*ref, imageName, treeName, rect));
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

// ref:SampleArea(rect): how many pixels of every color there are in the
// rectangle, as a table of counts with Colors for keys. The places none of the
// images cover count as the border color. The strings puzzle of Nancy23 finds
// out which areas a cut joins with it.
static int refSampleArea(lua_State *L) {
	const ScriptRef *ref = checkRef(L, 1);
	const ScriptRect area = checkRect(L, 2);

	Common::HashMap<uint32, int> counts;
	int numUncovered = area.width() * area.height();

	for (const ScriptRef::Layer *layer : ref->layers) {
		const ScriptRect &rect = layer->rect;
		const int left = MAX(rect.left, area.left);
		const int top = MAX(rect.top, area.top);
		const int right = MIN(rect.right, area.right);
		const int bottom = MIN(rect.bottom, area.bottom);
		if (left >= right || top >= bottom) {
			continue;
		}

		for (int y = top; y < bottom; ++y) {
			for (int x = left; x < right; ++x) {
				uint32 pixel;
				if (layer->image.getPixel(x - rect.left, y - rect.top, pixel)) {
					++counts[pixel];
				}
			}
		}

		numUncovered -= (right - left) * (bottom - top);
	}

	if (numUncovered > 0) {
		counts[ref->borderColor] += numUncovered;
	}

	lua_createtable(L, 0, counts.size());
	for (const auto &count : counts) {
		pushColor(L, count._key);
		lua_pushinteger(L, count._value);
		lua_settable(L, -3);
	}

	return 1;
}

// The one image of a Ref that the rectangle is in, for Difference
static const RefImage *findDifferenceImage(const ScriptRef &ref, const ScriptRect &rect) {
	if (ref.layers.size() != 1) {
		return nullptr;
	}

	const ScriptRect &layerRect = ref.layers[0]->rect;
	if (rect.left < layerRect.left || rect.top < layerRect.top || rect.right > layerRect.right || rect.bottom > layerRect.bottom) {
		return nullptr;
	}

	return &ref.layers[0]->image;
}

// The pixel of an image at a place, taking the nearest one for those outside it
static uint32 getClampedPixel(const RefImage &image, int x, int y) {
	uint32 pixel = 0;
	const Graphics::ManagedSurface &surface = image.image;
	if (surface.w > 0 && surface.h > 0) {
		byte a, r, g, b;
		surface.format.colorToARGB(surface.getPixel(CLIP<int>(x, 0, surface.w - 1), CLIP<int>(y, 0, surface.h - 1)), a, r, g, b);
		pixel = ((uint32)a << 24) | ((uint32)r << 16) | ((uint32)g << 8) | b;
	}

	return pixel;
}

// ref:Difference({ source, before, beforeRect, beforeColor, after, afterRect,
// afterColor, tolerance }), Nancy23, which grades a stroke of the calligraphy
// puzzle. This Ref is the solution: where it has the before color, the "before"
// image (the empty paper) and the "after" one (a picture of what the player
// drew) have to match; where it has the after color, they have to differ. Two
// pixels differ when three times the difference of their blue is over the
// tolerance. Gives back { beforePass, beforeFail, afterPass, afterFail }, the
// counts of the pixels that did and didn't.
static int refDifference(lua_State *L) {
	const ScriptRef *ref = checkRef(L, 1);
	luaL_checktype(L, 2, LUA_TTABLE);

	ScriptRect rects[3];
	static const char *const kRectNames[3] = { "beforeRect", "afterRect", "source" };
	for (int i = 0; i < 3; ++i) {
		const ScriptRect *rect = pushField(L, 2, kRectNames[i]) ? testRect(L, -1) : nullptr;
		if (!rect) {
			return luaL_error(L, "Table must have an '%s' field that is a Rect", kRectNames[i]);
		}

		rects[i] = *rect;
		lua_pop(L, 1);
	}

	const ScriptRef *refs[2];
	static const char *const kRefNames[2] = { "before", "after" };
	for (int i = 0; i < 2; ++i) {
		ScriptRef **object = pushField(L, 2, kRefNames[i]) ? (ScriptRef **)testUserData(L, -1, kRefType) : nullptr;
		if (!object) {
			return luaL_error(L, "Table must have a '%s' field that is a Ref", kRefNames[i]);
		}

		refs[i] = *object;
		lua_pop(L, 1);
	}

	uint32 colors[2];
	static const char *const kColorNames[2] = { "beforeColor", "afterColor" };
	for (int i = 0; i < 2; ++i) {
		const uint32 *color = pushField(L, 2, kColorNames[i]) ? testColor(L, -1) : nullptr;
		if (!color) {
			return luaL_error(L, "Table must have a '%s' field that is a Color", kColorNames[i]);
		}

		colors[i] = *color & 0xFFFFFF;
		lua_pop(L, 1);
	}

	if (!pushField(L, 2, "tolerance") || !lua_isnumber(L, -1)) {
		return luaL_argerror(L, 2, "Table must have a 'tolerance' field that is a number");
	}

	const double tolerance = lua_tonumber(L, -1);
	lua_pop(L, 1);

	const ScriptRect &beforeRect = rects[0];
	const ScriptRect &afterRect = rects[1];
	const ScriptRect &source = rects[2];
	if (source.width() != beforeRect.width() || source.height() != beforeRect.height() ||
			source.width() != afterRect.width() || source.height() != afterRect.height()) {
		return luaL_argerror(L, 2, "source, beforeRect and afterRect rects must have same width/height");
	}

	const RefImage *solution = findDifferenceImage(*ref, source);
	if (!solution) {
		return luaL_argerror(L, 2, "Ref object must have one tile that contains the source rect");
	}

	const RefImage *before = findDifferenceImage(*refs[0], beforeRect);
	if (!before) {
		return luaL_argerror(L, 2, "'before' ref object must have one tile that contains the beforeRect rect");
	}

	const RefImage *after = findDifferenceImage(*refs[1], afterRect);
	if (!after) {
		return luaL_argerror(L, 2, "'after' ref object must have one tile that contains the afterRect rect");
	}

	// The places are in the pixels of the images themselves
	int beforePass = 0, beforeFail = 0, afterPass = 0, afterFail = 0;
	for (int y = 0; y < source.height(); ++y) {
		for (int x = 0; x < source.width(); ++x) {
			const uint32 expected = getClampedPixel(*solution, source.left + x, source.top + y) & 0xFFFFFF;
			const int blueBefore = getClampedPixel(*before, beforeRect.left + x, beforeRect.top + y) & 0xFF;
			const int blueAfter = getClampedPixel(*after, afterRect.left + x, afterRect.top + y) & 0xFF;
			const bool differs = ABS(blueBefore - blueAfter) * 3 > tolerance;

			if (expected == colors[0]) {
				if (differs) {
					++beforeFail;
				} else {
					++beforePass;
				}
			} else if (expected == colors[1]) {
				if (differs) {
					++afterPass;
				} else {
					++afterFail;
				}
			}
		}
	}

	lua_createtable(L, 0, 4);
	lua_pushinteger(L, beforePass);
	lua_setfield(L, -2, "beforePass");
	lua_pushinteger(L, beforeFail);
	lua_setfield(L, -2, "beforeFail");
	lua_pushinteger(L, afterPass);
	lua_setfield(L, -2, "afterPass");
	lua_pushinteger(L, afterFail);
	lua_setfield(L, -2, "afterFail");
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
		{ "SampleArea", refSampleArea },
		{ "Difference", refDifference },
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
