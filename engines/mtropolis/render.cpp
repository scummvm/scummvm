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

#include "mtropolis/render.h"
#include "mtropolis/runtime.h"

#include "graphics/surface.h"
#include "graphics/managed_surface.h"

#include "graphics/cursorman.h"

namespace MTropolis {

template<class TNumber, int TResolution>
struct OrderedDitherGenerator {
	static void generateOrderedDither(TNumber (&pattern)[TResolution][TResolution]);
};

template<class TNumber>
struct OrderedDitherGenerator<TNumber, 1> {
	static void generateOrderedDither(TNumber (&pattern)[1][1]);
};

struct RenderItem {
	VisualElement *element;
};

template<class TNumber, int TResolution>
void OrderedDitherGenerator<TNumber, TResolution>::generateOrderedDither(TNumber (&pattern)[TResolution][TResolution]) {
	const int kHalfResolution = TResolution / 2;
	TNumber halfRes[kHalfResolution][kHalfResolution];

	OrderedDitherGenerator<TNumber, kHalfResolution>::generateOrderedDither(halfRes);

	const int kHalfResNumSteps = kHalfResolution * kHalfResolution;
	for (int y = 0; y < kHalfResolution; y++) {
		for (int x = 0; x < kHalfResolution; x++) {
			pattern[y * 2][x * 2] = halfRes[y][x];
			pattern[y * 2 + 1][x * 2 + 1] = halfRes[y][x] + kHalfResNumSteps * 1;
			pattern[y * 2][x * 2 + 1] = halfRes[y][x] + kHalfResNumSteps * 2;
			pattern[y * 2 + 1][x * 2] = halfRes[y][x] + kHalfResNumSteps * 3;
		}
	}
}

template<class TNumber>
void OrderedDitherGenerator<TNumber, 1>::generateOrderedDither(TNumber (&pattern)[1][1]) {
	pattern[0][0] = 0;
}

inline int quantize8To5Byte(int value, byte orderedDither16x16) {
	return (value * 249 + (orderedDither16x16 << 3)) >> 11;
}

inline int quantize8To5UShort(int value, uint16 orderedDither16x16) {
	return (value * 249 + orderedDither16x16) >> 11;
}

inline int expand5To8(int value) {
	return (value * 33) >> 2;
}

TextStyleFlags::TextStyleFlags() : bold(false), italic(false), underline(false), outline(false), shadow(false), condensed(false), expanded(false) {
}

bool TextStyleFlags::load(uint8 dataStyleFlags) {
	bold = ((dataStyleFlags & 0x01) != 0);
	italic = ((dataStyleFlags & 0x02) != 0);
	underline = ((dataStyleFlags & 0x03) != 0);
	outline = ((dataStyleFlags & 0x04) != 0);
	shadow = ((dataStyleFlags & 0x10) != 0);
	condensed = ((dataStyleFlags & 0x20) != 0);
	expanded = ((dataStyleFlags & 0x40) != 0);
	return true;
}

MacFontFormatting::MacFontFormatting() : fontID(0), fontFlags(0), size(12) {
}

MacFontFormatting::MacFontFormatting(uint16 fontID, uint8 fontFlags, uint16 size) : fontID(fontID), fontFlags(fontFlags), size(size) {
}

WindowParameters::WindowParameters(Runtime *runtime, int32 x, int32 y, int16 width, int16 height, const Graphics::PixelFormat &format)
	: runtime(runtime), x(x), y(y), width(width), height(height), format(format) {
}

Window::Window(const WindowParameters &windowParams)
	: _runtime(windowParams.runtime), _x(windowParams.x), _y(windowParams.y), _strata(0), _isMouseTransparent(false) {
	_surface.reset(new Graphics::ManagedSurface(windowParams.width, windowParams.height, windowParams.format));
}

Window::~Window() {
}

int32 Window::getX() const {
	return _x;
}

int32 Window::getY() const {
	return _y;
}

int32 Window::getWidth() const {
	return _surface->w;
}

int32 Window::getHeight() const {
	return _surface->h;
}

void Window::setPosition(int32 x, int32 y) {
	_x = x;
	_y = y;
}

void Window::resizeWindow(int32 width, int32 height) {
	Graphics::PixelFormat pixFmt = _surface->format;
	_surface.reset();
	_surface.reset(new Graphics::ManagedSurface(width, height, pixFmt));
}

const Common::SharedPtr<Graphics::ManagedSurface> &Window::getSurface() const {
	return _surface;
}

const Graphics::PixelFormat& Window::getPixelFormat() const {
	return _surface->format;
}

const Common::SharedPtr<CursorGraphic> &Window::getCursorGraphic() const {
	return _cursor;
}

void Window::setCursorGraphic(const Common::SharedPtr<CursorGraphic>& cursor) {
	_cursor = cursor;
}

void Window::setStrata(int strata) {
	_strata = strata;
}

int Window::getStrata() const {
	return _strata;
}

// Mouse transparency = ignores mouse events
void Window::setMouseTransparent(bool isTransparent) {
	_isMouseTransparent = isTransparent;
}

bool Window::isMouseTransparent() const {
	return _isMouseTransparent;
}

void Window::close() {
	Runtime *runtime = _runtime;
	_runtime = nullptr;

	if (runtime)
		runtime->removeWindow(this);
}

void Window::detachFromRuntime() {
	_runtime = nullptr;
}

void Window::onMouseDown(int32 x, int32 y, int mouseButton) {
}

void Window::onMouseMove(int32 x, int32 y) {
}

void Window::onMouseUp(int32 x, int32 y, int mouseButton) {
}

void Window::onKeyboardEvent(const Common::EventType evtType, bool repeat, const Common::KeyState &keyEvt) {
}

namespace Render {

uint32 resolveRGB(uint8 r, uint8 g, uint8 b, const Graphics::PixelFormat &fmt) {
	uint32 rPlaced = (static_cast<uint32>(r) >> (8 - fmt.rBits())) << fmt.rShift;
	uint32 gPlaced = (static_cast<uint32>(g) >> (8 - fmt.gBits())) << fmt.gShift;
	uint32 bPlaced = (static_cast<uint32>(b) >> (8 - fmt.bBits())) << fmt.bShift;
	uint32 aPlaced = (static_cast<uint32>(255) >> (8 - fmt.aBits())) << fmt.aShift;

	return rPlaced | gPlaced | bPlaced | aPlaced;
}

static void recursiveCollectDrawElementsAndUpdateOrigins(const Point16 &parentOrigin, Structural *structural, Common::Array<RenderItem> &normalBucket, Common::Array<RenderItem> &directBucket) {
	Point16 elementOrigin = parentOrigin;
	if (structural->isElement()) {
		Element *element = static_cast<Element *>(structural);
		if (element->isVisual()) {
			VisualElement *visualElement = static_cast<VisualElement *>(element);
			const Rect16 &elementRect = visualElement->getRelativeRect();

			elementOrigin.x += elementRect.left;
			elementOrigin.y += elementRect.top;

			visualElement->setCachedAbsoluteOrigin(Point16::create(elementOrigin.x, elementOrigin.y));

			RenderItem item;
			item.element = visualElement;

			if (visualElement->isVisible()) {
				if (visualElement->isDirectToScreen())
					directBucket.push_back(item);
				else
					normalBucket.push_back(item);
			}
		}
	}

	for (Common::Array<Common::SharedPtr<Structural> >::const_iterator it = structural->getChildren().begin(), itEnd = structural->getChildren().end(); it != itEnd; ++it) {
		recursiveCollectDrawElementsAndUpdateOrigins(elementOrigin, it->get(), normalBucket, directBucket);
	}
}

static bool renderItemLess(const RenderItem &a, const RenderItem &b) {
	return a.element->getLayer() < b.element->getLayer();
}

static void renderNormalElement(const RenderItem &item, Window *mainWindow) {
	item.element->render(mainWindow);
	item.element->finalizeRender();
}

static void renderDirectElement(const RenderItem &item, Window *mainWindow) {
	renderNormalElement(item, mainWindow);	// Meh
}

void renderProject(Runtime *runtime, Window *mainWindow) {
	bool sceneChanged = runtime->isSceneGraphDirty();

	Common::Array<Structural *> scenes;
	runtime->getScenesInRenderOrder(scenes);

	Common::Array<RenderItem> normalBucket;
	Common::Array<RenderItem> directBucket;

	for (Common::Array<Structural *>::const_iterator it = scenes.begin(), itEnd = scenes.end(); it != itEnd; ++it) {
		size_t normalStart = normalBucket.size();
		size_t directStart = directBucket.size();

		recursiveCollectDrawElementsAndUpdateOrigins(Point16::create(0, 0), *it, normalBucket, directBucket);

		size_t normalEnd = normalBucket.size();
		size_t directEnd = directBucket.size();

		if (normalEnd - normalStart > 1)
			Common::sort(normalBucket.begin() + normalStart, normalBucket.end(), renderItemLess);
		if (directEnd - directStart > 1)
			Common::sort(directBucket.begin() + directStart, directBucket.end(), renderItemLess);
	}

	if (!sceneChanged) {
		for (Common::Array<RenderItem>::const_iterator it = normalBucket.begin(), itEnd = normalBucket.end(); it != itEnd; ++it) {
			if (it->element->needsRender())
				sceneChanged = true;
		}
	}

	if (!sceneChanged) {
		for (Common::Array<RenderItem>::const_iterator it = directBucket.begin(), itEnd = directBucket.end(); it != itEnd; ++it) {
			if (it->element->needsRender())
				sceneChanged = true;
		}
	}

	if (sceneChanged) {
		for (Common::Array<RenderItem>::const_iterator it = normalBucket.begin(), itEnd = normalBucket.end(); it != itEnd; ++it)
			renderNormalElement(*it, mainWindow);

		for (Common::Array<RenderItem>::const_iterator it = directBucket.begin(), itEnd = directBucket.end(); it != itEnd; ++it)
			renderDirectElement(*it, mainWindow);
	}

	runtime->clearSceneGraphDirty();
}

void convert32To16(Graphics::Surface &destSurface, const Graphics::Surface &srcSurface) {
	const Graphics::PixelFormat srcFmt = srcSurface.format;
	const Graphics::PixelFormat destFmt = destSurface.format;

	assert(srcFmt.bytesPerPixel == 4);
	assert(destFmt.bytesPerPixel == 2);
	assert(destSurface.w == srcSurface.w);
	assert(srcSurface.h == destSurface.h);

	uint16 ditherPattern[16][16];
	OrderedDitherGenerator<uint16, 16>::generateOrderedDither(ditherPattern);

	for (int x = 0; x < 16; x++) {
		for (int y = 0; y < 16; y++)
			ditherPattern[y][x] <<= 3;
	}

	size_t w = srcSurface.w;
	size_t h = srcSurface.h;

	for (size_t y = 0; y < h; y++) {
		const uint16 *ditherRow = ditherPattern[y % 16];
		const uint32 *srcRow = static_cast<const uint32*>(srcSurface.getBasePtr(0, y));
		uint16 *destRow = static_cast<uint16 *>(destSurface.getBasePtr(0, y));

		for (size_t x = 0; x < w; x++) {
			uint16 ditherOffset = ditherRow[x % 16];
			uint32 packed32 = srcRow[x];
			uint8 r = (packed32 >> srcFmt.rShift) & 0xff;
			uint8 g = (packed32 >> srcFmt.gShift) & 0xff;
			uint8 b = (packed32 >> srcFmt.bShift) & 0xff;

			r = quantize8To5UShort(r, ditherOffset);
			g = quantize8To5UShort(g, ditherOffset);
			b = quantize8To5UShort(b, ditherOffset);
			destRow[x] = (r << destFmt.rShift) | (g << destFmt.gShift) | (b << destFmt.bShift);
		}
	}
}

void convert16To32(Graphics::Surface &destSurface, const Graphics::Surface &srcSurface) {
	const Graphics::PixelFormat srcFmt = srcSurface.format;
	const Graphics::PixelFormat destFmt = destSurface.format;

	assert(srcFmt.bytesPerPixel == 2);
	assert(destFmt.bytesPerPixel == 4);
	assert(destSurface.w == srcSurface.w);
	assert(srcSurface.h == destSurface.h);

	size_t w = srcSurface.w;
	size_t h = srcSurface.h;

	for (size_t y = 0; y < h; y++) {
		const uint32 *srcRow = static_cast<const uint32 *>(srcSurface.getBasePtr(0, y));
		uint16 *destRow = static_cast<uint16 *>(destSurface.getBasePtr(0, y));

		for (size_t x = 0; x < w; x++) {
			uint32 packed16 = srcRow[x];
			uint8 r = (packed16 >> srcFmt.rShift) & 0x1f;
			uint8 g = (packed16 >> srcFmt.gShift) & 0x1f;
			uint8 b = (packed16 >> srcFmt.bShift) & 0x1f;

			r = expand5To8(r);
			g = expand5To8(g);
			b = expand5To8(b);
			destRow[x] = (r << destFmt.rShift) | (g << destFmt.gShift) | (b << destFmt.bShift);
		}
	}
}

} // End of namespace Render

} // End of namespace MTropolis
