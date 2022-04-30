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

template<class TNumber, int TResolution>
void OrderedDitherGenerator<TNumber, TResolution>::generateOrderedDither(TNumber (&pattern)[TResolution][TResolution]) {
	const int kHalfResolution = TResolution / 2;
	byte halfRes[kHalfResolution][kHalfResolution];

	OrderedDitherGenerator<kHalfResolution>::generateOrderedDither(halfRes);

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

inline int quantize8To5(int value, byte orderedDither16x16) {
	return (value * 249 + (orderedDither16x16 << 3)) >> 11;
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


namespace Render {

uint32 resolveRGB(uint8 r, uint8 g, uint8 b, const Graphics::PixelFormat &fmt) {
	uint32 rPlaced = (static_cast<uint32>(r) >> (8 - fmt.rBits())) << fmt.rShift;
	uint32 gPlaced = (static_cast<uint32>(g) >> (8 - fmt.gBits())) << fmt.gShift;
	uint32 bPlaced = (static_cast<uint32>(b) >> (8 - fmt.bBits())) << fmt.bShift;
	uint32 aPlaced = (static_cast<uint32>(255) >> (8 - fmt.aBits())) << fmt.aShift;

	return rPlaced | gPlaced | bPlaced | aPlaced;
}

static void recursiveCollectDrawElements(Structural *structural, Common::Array<VisualElement *> &normalBucket, Common::Array<VisualElement *> &directBucket) {
	if (structural->isElement()) {
		Element *element = static_cast<Element *>(structural);
		if (element->isVisual()) {
			VisualElement *visualElement = static_cast<VisualElement *>(element);
			if (visualElement->isVisible()) {
				if (visualElement->isDirectToScreen())
					directBucket.push_back(visualElement);
				else
					normalBucket.push_back(visualElement);
			}
		}
	}

	for (Common::Array<Common::SharedPtr<Structural> >::const_iterator it = structural->getChildren().begin(), itEnd = structural->getChildren().end(); it != itEnd; ++it) {
		recursiveCollectDrawElements(it->get(), normalBucket, directBucket);
	}
}

static bool visualElementLayerLess(VisualElement *a, VisualElement *b) {
	return a->getLayer() < b->getLayer();
}

static void renderNormalElement(VisualElement *element, Window *mainWindow) {
	element->render(mainWindow);
}

static void renderDirectElement(VisualElement *element, Window *mainWindow) {
	renderNormalElement(element, mainWindow);	// Meh
}

void renderProject(Runtime *runtime, Window *mainWindow) {
	Common::Array<Structural *> scenes;
	runtime->getScenesInRenderOrder(scenes);

	Common::Array<VisualElement *> normalBucket;
	Common::Array<VisualElement *> directBucket;

	for (Common::Array<Structural *>::const_iterator it = scenes.begin(), itEnd = scenes.end(); it != itEnd; ++it) {
		size_t normalStart = normalBucket.size();
		size_t directStart = directBucket.size();

		recursiveCollectDrawElements(*it, normalBucket, directBucket);

		size_t normalEnd = normalBucket.size();
		size_t directEnd = directBucket.size();

		if (normalEnd - normalStart > 1)
			Common::sort(normalBucket.begin() + normalStart, normalBucket.end(), visualElementLayerLess);
		if (directEnd - directStart > 1)
			Common::sort(directBucket.begin() + directStart, directBucket.end(), visualElementLayerLess);
	}

	for (Common::Array<VisualElement *>::const_iterator it = normalBucket.begin(), itEnd = normalBucket.end(); it != itEnd; ++it)
		renderNormalElement(*it, mainWindow);

	for (Common::Array<VisualElement *>::const_iterator it = directBucket.begin(), itEnd = directBucket.end(); it != itEnd; ++it)
		renderDirectElement(*it, mainWindow);
}

} // End of namespace Render

} // End of namespace MTropolis
