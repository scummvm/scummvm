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

#include "graphics/surface.h"
#include "graphics/managed_surface.h"

#include "graphics/cursorman.h"

#include "mtropolis/render.h"
#include "mtropolis/runtime.h"

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
	size_t sceneStackDepth;
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

MacFontFormatting::MacFontFormatting(uint16 mff_fontID, uint8 mff_fontFlags, uint16 mff_size) : fontID(mff_fontID), fontFlags(mff_fontFlags), size(mff_size) {
}

WindowParameters::WindowParameters(Runtime *wp_runtime, int32 wp_x, int32 wp_y, int16 wp_width, int16 wp_height, const Graphics::PixelFormat &wp_format)
	: runtime(wp_runtime), x(wp_x), y(wp_y), width(wp_width), height(wp_height), format(wp_format) {
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

static void recursiveCollectDrawElementsAndUpdateOrigins(const Common::Point &parentOrigin, Structural *structural, size_t sceneStackDepth, Common::Array<RenderItem> &normalBucket, Common::Array<RenderItem> &directBucket) {
	Common::Point elementOrigin = parentOrigin;
	if (structural->isElement()) {
		Element *element = static_cast<Element *>(structural);
		if (element->isVisual()) {
			VisualElement *visualElement = static_cast<VisualElement *>(element);
			const Common::Rect &elementRect = visualElement->getRelativeRect();

			elementOrigin.x += elementRect.left;
			elementOrigin.y += elementRect.top;

			visualElement->setCachedAbsoluteOrigin(Common::Point(elementOrigin.x, elementOrigin.y));

			RenderItem item;
			item.element = visualElement;
			item.sceneStackDepth = sceneStackDepth;

			if (visualElement->isVisible()) {
				if (visualElement->isDirectToScreen())
					directBucket.push_back(item);
				else
					normalBucket.push_back(item);
			}
		}
	}

	for (Common::Array<Common::SharedPtr<Structural> >::const_iterator it = structural->getChildren().begin(), itEnd = structural->getChildren().end(); it != itEnd; ++it) {
		recursiveCollectDrawElementsAndUpdateOrigins(elementOrigin, it->get(), sceneStackDepth, normalBucket, directBucket);
	}
}

static bool renderItemLess(const RenderItem &a, const RenderItem &b) {
	const uint16 aLayer = a.element->getLayer();
	const uint16 bLayer = b.element->getLayer();

	if (aLayer != bLayer)
		return aLayer < bLayer;

	return a.sceneStackDepth < b.sceneStackDepth;
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

	size_t sceneStackDepth = 0;
	for (Common::Array<Structural *>::const_iterator it = scenes.begin(), itEnd = scenes.end(); it != itEnd; ++it) {
		recursiveCollectDrawElementsAndUpdateOrigins(Common::Point(0, 0), *it, sceneStackDepth, normalBucket, directBucket);
		sceneStackDepth++;
	}

	Common::sort(normalBucket.begin(), normalBucket.end(), renderItemLess);
	Common::sort(directBucket.begin(), directBucket.end(), renderItemLess);

	if (!sceneChanged) {
		for (Common::Array<RenderItem>::const_iterator it = normalBucket.begin(), itEnd = normalBucket.end(); it != itEnd; ++it) {
			if (it->element->needsRender()) {
				sceneChanged = true;
				break;
			}
		}
	}

	if (!sceneChanged) {
		for (Common::Array<RenderItem>::const_iterator it = directBucket.begin(), itEnd = directBucket.end(); it != itEnd; ++it) {
			if (it->element->needsRender()) {
				sceneChanged = true;
				break;
			}
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

class DissolveOrderedDitherPatternGenerator {
public:
	DissolveOrderedDitherPatternGenerator();

	uint8 getNext();
	void nextLine();

private:
	uint8 _ditherPattern[16][16];

	uint16 _x;
	uint16 _y;
};

DissolveOrderedDitherPatternGenerator::DissolveOrderedDitherPatternGenerator() {
	OrderedDitherGenerator<uint8, 16>::generateOrderedDither(_ditherPattern);
}

inline uint8 DissolveOrderedDitherPatternGenerator::getNext() {
	uint8 result = _ditherPattern[_y][_x];

	uint16 newX = _x + 1;
	if (newX == 16)
		newX = 0;
	_x = newX;

	return result;
}

inline void DissolveOrderedDitherPatternGenerator::nextLine() {
	_x = 0;
	uint16 newY = _y + 1;
	if (newY == 16)
		newY = 0;
	_y = newY;
}

class DissolveOrderedDitherRandomGenerator {
public:
	DissolveOrderedDitherRandomGenerator();

	uint8 getNext();
	void nextLine();

private:
	uint32 _lcgState;
};


DissolveOrderedDitherRandomGenerator::DissolveOrderedDitherRandomGenerator() : _lcgState(13) {
}

inline uint8 DissolveOrderedDitherRandomGenerator::getNext() {
	_lcgState = ((_lcgState * 1103515245u) + 12345u) & 0x7fffffffu;
	return (_lcgState >> 16) & 0xff;
}

inline void DissolveOrderedDitherRandomGenerator::nextLine() {
}

template<class TPixel, class TGenerator>
static void runDissolveTransitionWithType(Graphics::ManagedSurface &surface, const Graphics::ManagedSurface &oldFrame, const Graphics::ManagedSurface &newFrame, uint8 breakpoint) {
	TGenerator generator;

	assert(surface.format.bytesPerPixel == oldFrame.format.bytesPerPixel);
	assert(surface.format.bytesPerPixel == newFrame.format.bytesPerPixel);

	uint16 w = surface.w;
	uint16 h = surface.h;

	for (uint y = 0; y < h; y++) {
		TPixel *destRow = static_cast<TPixel *>(surface.getBasePtr(0, y));
		const TPixel *oldRow = static_cast<const TPixel *>(oldFrame.getBasePtr(0, y));
		const TPixel *newRow = static_cast<const TPixel *>(newFrame.getBasePtr(0, y));

		for (uint x = 0; x < w; x++) {
			if (generator.getNext() <= breakpoint)
				destRow[x] = newRow[x];
			else
				destRow[x] = oldRow[x];
		}

		generator.nextLine();
	}
}

template<class TGenerator>
static void runDissolveTransition(Graphics::ManagedSurface &surface, const Graphics::ManagedSurface &oldFrame, const Graphics::ManagedSurface &newFrame, uint8 breakpoint) {
	switch (surface.format.bytesPerPixel) {
	case 1:
		runDissolveTransitionWithType<uint8, TGenerator>(surface, oldFrame, newFrame, breakpoint);
		break;
	case 2:
		runDissolveTransitionWithType<uint16, TGenerator>(surface, oldFrame, newFrame, breakpoint);
		break;
	case 4:
		runDissolveTransitionWithType<uint32, TGenerator>(surface, oldFrame, newFrame, breakpoint);
		break;
	default:
		assert(false);
		break;
	}
}

static void safeCopyRectToSurface(Graphics::ManagedSurface &surface, const Graphics::Surface &srcSurface, int destX, int destY, const Common::Rect subRect) {
	if (subRect.width() == 0 || subRect.height() == 0)
		return;

	surface.copyRectToSurface(srcSurface, destX, destY, subRect);
}

void renderSceneTransition(Runtime *runtime, Window *mainWindow, const SceneTransitionEffect &effect, uint32 startTime, uint32 endTime, uint32 currentTime, const Graphics::ManagedSurface &oldFrame, const Graphics::ManagedSurface &newFrame) {
	Graphics::ManagedSurface &surface = *mainWindow->getSurface();

	assert(endTime > startTime);

	uint32 duration = endTime - startTime;

	uint16 w = surface.w;
	uint16 h = surface.h;

	if (effect._transitionType == SceneTransitionTypes::kSlide || effect._transitionType == SceneTransitionTypes::kWipe)
		safeCopyRectToSurface(surface, oldFrame, 0, 0, Common::Rect(0, 0, w, h));

	switch (effect._transitionType) {
	case SceneTransitionTypes::kPatternDissolve:
		runDissolveTransition<DissolveOrderedDitherPatternGenerator>(surface, oldFrame, newFrame, (currentTime - startTime) * 255 / duration);
		break;
	case SceneTransitionTypes::kRandomDissolve:
		runDissolveTransition<DissolveOrderedDitherRandomGenerator>(surface, oldFrame, newFrame, (currentTime - startTime) * 255 / duration);
		break;
	case SceneTransitionTypes::kFade:
		// Fade transitions fade to black and then fade from black in the new scene
		warning("Fade transitions are not implemented");
		break;
	case SceneTransitionTypes::kSlide:
	case SceneTransitionTypes::kPush: {
			uint32 directionalOffset = 0;
			switch (effect._transitionDirection) {
			case SceneTransitionDirections::kUp:
				directionalOffset = static_cast<uint32>(currentTime - startTime) * static_cast<uint32>(h) / duration;

				if (effect._transitionType == SceneTransitionTypes::kPush)
					safeCopyRectToSurface(surface, oldFrame, 0, 0, Common::Rect(0, directionalOffset, w, h));

				safeCopyRectToSurface(surface, newFrame, 0, h - directionalOffset, Common::Rect(0, 0, w, directionalOffset));
				break;
			case SceneTransitionDirections::kDown:
				directionalOffset = static_cast<uint32>(currentTime - startTime) * static_cast<uint32>(h) / duration;

				if (effect._transitionType == SceneTransitionTypes::kPush)
					safeCopyRectToSurface(surface, oldFrame, 0, directionalOffset, Common::Rect(0, 0, w, h - directionalOffset));

				safeCopyRectToSurface(surface, newFrame, 0, 0, Common::Rect(0, h - directionalOffset, w, h));
				break;
			case SceneTransitionDirections::kLeft:
				directionalOffset = static_cast<uint32>(currentTime - startTime) * static_cast<uint32>(w) / duration;

				if (effect._transitionType == SceneTransitionTypes::kPush)
					safeCopyRectToSurface(surface, oldFrame, 0, 0, Common::Rect(directionalOffset, 0, w, h));

				safeCopyRectToSurface(surface, newFrame, w - directionalOffset, 0, Common::Rect(0, 0, directionalOffset, h));
				break;
			case SceneTransitionDirections::kRight:
				directionalOffset = static_cast<uint32>(currentTime - startTime) * static_cast<uint32>(w) / duration;

				if (effect._transitionType == SceneTransitionTypes::kPush)
					safeCopyRectToSurface(surface, oldFrame, directionalOffset, 0, Common::Rect(0, 0, w - directionalOffset, h));

				safeCopyRectToSurface(surface, newFrame, 0, 0, Common::Rect(w - directionalOffset, 0, w, h));
				break;
			default:
				assert(false);
				break;
			}
		} break;
	case SceneTransitionTypes::kZoom:
		warning("Zoom transitions are not implemented");
		break;
	case SceneTransitionTypes::kWipe:{
			uint32 directionalOffset = 0;
			switch (effect._transitionDirection) {
			case SceneTransitionDirections::kUp:
				directionalOffset = static_cast<uint32>(currentTime - startTime) * static_cast<uint32>(h) / duration;
				safeCopyRectToSurface(surface, newFrame, 0, h - directionalOffset, Common::Rect(0, h - directionalOffset, w, h));
				break;
			case SceneTransitionDirections::kDown:
				directionalOffset = static_cast<uint32>(currentTime - startTime) * static_cast<uint32>(h) / duration;
				safeCopyRectToSurface(surface, newFrame, 0, 0, Common::Rect(0, 0, w, directionalOffset));
				break;
			case SceneTransitionDirections::kLeft:
				directionalOffset = static_cast<uint32>(currentTime - startTime) * static_cast<uint32>(w) / duration;
				safeCopyRectToSurface(surface, newFrame, w - directionalOffset, 0, Common::Rect(w - directionalOffset, 0, w, h));
				break;
			case SceneTransitionDirections::kRight:
				directionalOffset = static_cast<uint32>(currentTime - startTime) * static_cast<uint32>(w) / duration;
				safeCopyRectToSurface(surface, newFrame, 0, 0, Common::Rect(0, 0, directionalOffset, h));
				break;
			default:
				assert(false);
				break;
			}
		} break;
	default:
		assert(false);
		break;
	}
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
