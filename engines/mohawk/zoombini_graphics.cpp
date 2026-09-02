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
#include "mohawk/zoombini_resource.h"

#include "common/substream.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/translation.h"

#include "engines/util.h"
#include "graphics/fontman.h"
#include "graphics/fonts/ttf.h"
#include "graphics/paletteman.h"

#include "mohawk/cursors.h"
#include "mohawk/resource.h"
#include "mohawk/zoombini.h"
#include "mohawk/zoombini_graphics.h"
#include "mohawk/zoombini_metaengine.h"
#include "mohawk/zoombini_page.h"
#include "mohawk/zoombini_scripts.h"
#include "mohawk/zoombini_text.h"
#include "zoombini_graphics.h"

namespace Mohawk {

ZoombiniGraphics::ZoombiniGraphics(MohawkEngine_Zoombini *vm) : GraphicsManager(), _vm(vm),
																_bmpDecoder(new MohawkBitmap()),
																_screenRect(Common::Rect(kScreenWidth, kScreenHeight)) {
	initGraphics(_screenRect.width(), _screenRect.height());
	clearPalette();

	_pixelFormat = Graphics::PixelFormat::createFormatCLUT8();

	// Initialize the drawing surfaces.
	_backScreen = new Graphics::Surface();
	_backScreen->create(kScreenWidth, kScreenHeight, _pixelFormat);
	_shapeScreen = new Graphics::Surface();
	_shapeScreen->create(kScreenWidth, kScreenHeight, _pixelFormat);
	clearScreens();
}

ZoombiniGraphics::~ZoombiniGraphics() {
	clearCommonCache();

	delete _bmpDecoder;

	_shapeScreen->free();
	delete _shapeScreen;
	_backScreen->free();
	delete _backScreen;
}

Graphics::Surface *ZoombiniGraphics::getScreen(ScreenKind screenKind) {
	switch (screenKind) {
	case kBackScreen:
		return _backScreen;
	case kShapeScreen:
		return _shapeScreen;
	default:
		error("gfx: invalid ScreenKind %d", screenKind);
		return nullptr;
	}
}

void ZoombiniGraphics::createScreen(Graphics::Surface &screen) {
	screen.create(kScreenWidth, kScreenHeight, _pixelFormat);
}

void ZoombiniGraphics::captureScreen(ScreenKind srcScreenKind, Graphics::Surface *destScreen) {
	assert(destScreen != nullptr);

	Graphics::Surface *srcScreen = _vm->_gfx->getScreen(srcScreenKind);
	destScreen->copyFrom(*srcScreen);
}

void ZoombiniGraphics::copyToScreen(ScreenKind destScreenKind, const Graphics::Surface &srcScreen) {
	Graphics::Surface *destScreen = _vm->_gfx->getScreen(destScreenKind);
	destScreen->copyFrom(srcScreen);
	recordDirtyRect(destScreenKind, destScreen->getRect());
}

void ZoombiniGraphics::captureComposedScreen(ScreenKind destScreenKind) {
	Graphics::Surface *destScreen = _vm->_gfx->getScreen(destScreenKind);

	Graphics::Surface *systemScreen = _vm->_system->lockScreen();
	destScreen->copyFrom(*systemScreen);
	_vm->_system->unlockScreen();
	recordDirtyRect(destScreenKind, destScreen->getRect());
}

void ZoombiniGraphics::captureComposedScreen(Graphics::Surface *destScreen) {
	assert(destScreen != nullptr);

	Graphics::Surface *systemScreen = _vm->_system->lockScreen();
	destScreen->copyFrom(*systemScreen);
	_vm->_system->unlockScreen();
}

// [*] Screen updates
void ZoombiniGraphics::setDirty() {
	_isScreenDirty = true;
	mergeRectIntoRegion(_screenDirtyRects, _screenRect);
}

bool ZoombiniGraphics::flushScreens() {
	if (!_isScreenDirty)
		return false;

	assert(!_screenDirtyRects.empty());
	for (const Common::Rect &dirtyRect : _screenDirtyRects) {
		_vm->_system->copyRectToScreen(_shapeScreen->getBasePtr(dirtyRect.left, dirtyRect.top), _shapeScreen->pitch,
									   dirtyRect.left, dirtyRect.top, dirtyRect.width(), dirtyRect.height());
	}

	_screenDirtyRects.clear();
	_isScreenDirty = false;
	return true;
}

void ZoombiniGraphics::clearScreens() {
	uint32 blackColor = kTransparentKey;
	_backScreen->fillRect(_screenRect, blackColor);
	_shapeScreen->fillRect(_screenRect, blackColor);

	_vm->_system->fillScreen(blackColor);
	_screenDirtyRects.clear();
	_isScreenDirty = false;
}

void ZoombiniGraphics::showDemoStartupLoadingScreen() {
	const Graphics::Font *font = FontMan.getFontByUsage(Graphics::FontManager::kLocalizedFont);
	if (!font)
		return;

	const Common::U32String loadingText = _("Loading game...");
	const byte maxTextColor = static_cast<byte>(kBlackKey - 1);
	// Reserve index 0 for the black background and preserve the black-key index.
	byte loadingPalette[3 * 256] = {};
	for (uint16 paletteIndex = 0; paletteIndex < kBlackKey; paletteIndex++) {
		const byte component = static_cast<byte>(paletteIndex);
		loadingPalette[paletteIndex * 3 + 0] = component;
		loadingPalette[paletteIndex * 3 + 1] = component;
		loadingPalette[paletteIndex * 3 + 2] = component;
	}
	_vm->_system->getPaletteManager()->setPalette(loadingPalette, 0, ARRAYSIZE(loadingPalette) / 3);

	_shapeScreen->fillRect(_screenRect, kTransparentKey);
	const int textX = (_screenRect.width() - font->getStringWidth(loadingText)) / 2;
	const int textY = (_screenRect.height() - font->getFontHeight()) / 2;
	const Common::Rect textBounds = font->getBoundingBox(loadingText);
	if (!textBounds.isEmpty()) {
		// Render only the glyph bounds in true color so FreeType coverage survives.
		const Graphics::PixelFormat textFormat = Graphics::PixelFormat::createFormatARGB32();
		Graphics::Surface textSurface;
		textSurface.create(textBounds.width(), textBounds.height(), textFormat);
		textSurface.fillRect(textSurface.getRect(), 0);

		const uint32 textColor = textFormat.ARGBToColor(0xFF, maxTextColor, maxTextColor, maxTextColor);
		font->drawAlphaString(&textSurface, loadingText, -textBounds.left, -textBounds.top,
							  textSurface.w, textColor, Graphics::kTextAlignLeft, 0, false, true);

		const Common::Rect destRect(textX + textBounds.left, textY + textBounds.top,
									textX + textBounds.right, textY + textBounds.bottom);
		Common::Rect clippedDestRect = destRect;
		clippedDestRect.clip(_screenRect);
		const int sourceX = clippedDestRect.left - destRect.left;
		const int sourceY = clippedDestRect.top - destRect.top;

		// Quantize alpha into the startup grayscale palette while copying to CLUT8.
		for (int rowIdx = 0; rowIdx < clippedDestRect.height(); rowIdx++) {
			const uint32 *sourceRow = static_cast<const uint32 *>(textSurface.getBasePtr(sourceX, sourceY + rowIdx));
			byte *destRow = static_cast<byte *>(_shapeScreen->getBasePtr(clippedDestRect.left, clippedDestRect.top + rowIdx));
			for (int columnIdx = 0; columnIdx < clippedDestRect.width(); columnIdx++) {
				const byte alpha = static_cast<byte>((sourceRow[columnIdx] >> textFormat.aShift) & 0xFF);
				destRow[columnIdx] = static_cast<byte>((static_cast<uint32>(alpha) * maxTextColor + 0x7F) / 0xFF);
			}
		}

		textSurface.free();
	}

	setDirty();
	flushScreens();
	_vm->_system->updateScreen();
}

void ZoombiniGraphics::copyBackToShapeScreen() {
	// Copy the background into the composite screen before drawing shapes.
	_shapeScreen->copyRectToSurface(*_backScreen, 0, 0, _screenRect);
	recordDirtyRect(kShapeScreen, _screenRect);
}

void ZoombiniGraphics::copyBackToShapeScreen(const Common::Rect &clipRect) {
	// Restore the background only inside the requested dirty rectangle.
	Common::Rect rect = clipRect;
	rect.clip(_screenRect);
	if (rect.isEmpty())
		return;
	_shapeScreen->copyRectToSurface(*_backScreen, rect.left, rect.top, rect);
	recordDirtyRect(kShapeScreen, rect);
}

static bool areRegionRectsAdjacent(const Common::Rect &a, const Common::Rect &b) {
	const bool horizontalJoin = a.top == b.top && a.bottom == b.bottom &&
								(a.right == b.left || b.right == a.left);
	const bool verticalJoin = a.left == b.left && a.right == b.right &&
							  (a.bottom == b.top || b.bottom == a.top);
	return horizontalJoin || verticalJoin;
}

static void appendCoalescedRegionRect(Common::Array<Common::Rect> &region, Common::Rect rect) {
	uint32 index = 0;
	while (index < region.size()) {
		if (!areRegionRectsAdjacent(region[index], rect)) {
			index += 1;
			continue;
		}

		rect.extend(region[index]);
		region[index] = region.back();
		region.pop_back();
		index = 0;
	}

	region.push_back(rect);
}

bool ZoombiniGraphics::mergeRectIntoRegion(Common::Array<Common::Rect> &region, const Common::Rect &rect) {
	if (rect.isEmpty())
		return false;

	// A containing rectangle is the common path when clipped draw calls repeat
	// coverage already established by the frame's background restoration.
	for (const Common::Rect &existing : region) {
		if (existing.contains(rect))
			return false;
	}

	// Replace covered entries with the new rectangle instead of fragmenting it
	// around coverage that it already contains.
	uint32 existingIndex = 0;
	while (existingIndex < region.size()) {
		if (rect.contains(region[existingIndex])) {
			region[existingIndex] = region.back();
			region.pop_back();
			continue;
		}

		existingIndex += 1;
	}

	// Keep only portions of the new rectangle that are not already represented.
	// Existing partial overlaps remain unchanged.
	Common::Array<Common::Rect> pending;
	pending.reserve(8);
	pending.push_back(rect);

	for (const Common::Rect &existing : region) {
		uint32 pieceIndex = 0;
		while (pieceIndex < pending.size()) {
			const Common::Rect piece = pending[pieceIndex];
			if (!piece.intersects(existing)) {
				pieceIndex += 1;
				continue;
			}

			Common::Rect overlap = piece;
			overlap.clip(existing);
			pending[pieceIndex] = pending.back();
			pending.pop_back();

			if (piece.top < overlap.top)
				pending.push_back(Common::Rect(piece.left, piece.top, piece.right, overlap.top));
			if (overlap.bottom < piece.bottom)
				pending.push_back(Common::Rect(piece.left, overlap.bottom, piece.right, piece.bottom));
			if (piece.left < overlap.left)
				pending.push_back(Common::Rect(piece.left, overlap.top, overlap.left, overlap.bottom));
			if (overlap.right < piece.right)
				pending.push_back(Common::Rect(overlap.right, overlap.top, piece.right, overlap.bottom));
		}

		if (pending.empty())
			return false;
	}

	// Only the newly exposed pieces can create new adjacency. Coalesce outward
	// from those pieces instead of repeatedly comparing every pair in the region.
	for (const Common::Rect &piece : pending)
		appendCoalescedRegionRect(region, piece);

	return true;
}

void ZoombiniGraphics::setRenderClipRects(const Common::Array<Common::Rect> &rects) {
	_renderClipRects.clear();
	for (const Common::Rect &rect : rects) {
		Common::Rect clipped = rect;
		clipped.clip(_screenRect);
		if (!clipped.isEmpty())
			mergeRectIntoRegion(_renderClipRects, clipped);
	}
	_isRenderClipActive = true;
	if (_renderClipRects.empty()) {
		_hasRenderClipRect = false;
		return;
	}
	_renderClipBounds = _renderClipRects[0];
	for (uint32 i = 1; i < _renderClipRects.size(); i++)
		_renderClipBounds.extend(_renderClipRects[i]);
	_renderClipBounds.clip(_screenRect);
	_hasRenderClipRect = !_renderClipBounds.isEmpty();
}

void ZoombiniGraphics::addRenderClipRect(const Common::Rect &rect) {
	Common::Rect clipped = rect;
	clipped.clip(_screenRect);
	if (clipped.isEmpty())
		return;
	_isRenderClipActive = true;
	if (!mergeRectIntoRegion(_renderClipRects, clipped))
		return;
	if (_hasRenderClipRect) {
		_renderClipBounds.extend(clipped);
	} else {
		_renderClipBounds = clipped;
		_hasRenderClipRect = true;
	}
}

void ZoombiniGraphics::clearRenderClipRect() {
	_renderClipRects.clear();
	_isRenderClipActive = false;
	_hasRenderClipRect = false;
}

void ZoombiniGraphics::beginDirtyRectTracking(bool expandRenderClip) {
	_isDirtyRectTracking = true;
	_hasTrackedDirtyBounds = false;
	_expandTrackedDirtyClip = expandRenderClip;
	_trackedDirtyBounds = Common::Rect();
}

Common::Rect ZoombiniGraphics::endDirtyRectTracking() {
	_isDirtyRectTracking = false;
	_expandTrackedDirtyClip = false;
	return _hasTrackedDirtyBounds ? _trackedDirtyBounds : Common::Rect();
}

void ZoombiniGraphics::recordDirtyRect(ScreenKind screenKind, const Common::Rect &rect) {
	if (rect.isEmpty())
		return;

	Graphics::Surface *screen = getScreen(screenKind);

	Common::Rect clipped = rect;
	clipped.clip(screen->w, screen->h);
	if (clipped.isEmpty())
		return;

	if (_isDirtyRectTracking) {
		if (_hasTrackedDirtyBounds) {
			_trackedDirtyBounds.extend(clipped);
		} else {
			_trackedDirtyBounds = clipped;
			_hasTrackedDirtyBounds = true;
		}

		if (_expandTrackedDirtyClip && screenKind == kShapeScreen)
			addRenderClipRect(clipped);
	}

	if (screenKind == kShapeScreen) {
		_isScreenDirty = true;
		mergeRectIntoRegion(_screenDirtyRects, clipped);
	}

	// A back-port write changes the pixels which must be restored into the persistent shape port on the next render.
	// Queue its exact rectangle in the page's external dirty RMap equivalent.
	// This is distinct from shape output damage, which @ref ZoombiniGraphics::flushScreens() copies to the host screen.
	if (screenKind == kBackScreen && _vm->getCurrentPage())
		_vm->getCurrentPage()->invalidateBackgroundRect(clipped);
}

void ZoombiniGraphics::clearScreen(ScreenKind screenKind) {
	uint32 blackColor = kTransparentKey;
	Graphics::Surface *screen = _vm->_gfx->getScreen(screenKind);
	screen->fillRect(_screenRect, blackColor);
	recordDirtyRect(screenKind, _screenRect);
}

void ZoombiniGraphics::reinitGraphics(bool trueColor) {
	// Enable true color support only when playing Bink videos; otherwise, use CLUT8 mode.
	bool isTrueColor = 1 < _pixelFormat.bytesPerPixel;
	if (trueColor == isTrueColor)
		return;

	clearCache();
	clearCommonCache();

	_backScreen->free();
	_shapeScreen->free();

	if (trueColor) {
		initGraphics(kScreenWidth, kScreenHeight, nullptr);
		_pixelFormat = _vm->_system->getScreenFormat();
	} else {
		initGraphics(kScreenWidth, kScreenHeight);
		_pixelFormat = Graphics::PixelFormat::createFormatCLUT8();
		clearPalette();
	}

	_backScreen->create(kScreenWidth, kScreenHeight, _pixelFormat);
	_shapeScreen->create(kScreenWidth, kScreenHeight, _pixelFormat);
	_screenDirtyRects.clear();
	_isScreenDirty = false;
}

// [*] Handle Cursor
void ZoombiniGraphics::setMouseCursor(MouseCursorResourceId cursorId) {
	if (cursorId == _activeCursorId)
		return;
	const bool leavingWatchPose = _activeCursorId == kResCursor01_Watch && cursorId != kResCursor01_Watch;

	switch (cursorId) {
	case kResCursor00_Default:
		_vm->_cursor->setDefaultCursor();
		break;
	case kResCursor01_Watch:
	case kResCursor02_EyeMiddle:
	case kResCursor03_EyeRight:
	case kResCursor04_EyeLeft:
	case kResCursor05_EyeBlink:
		_vm->_cursor->setCursor(static_cast<uint16>(cursorId));
		break;
	default:
		error("gfx: invalid CursorType %d", cursorId);
		break;
	}
	_activeCursorId = cursorId;
	if (leavingWatchPose) {
		// The initial watch pose does not own a full animation interval. Once it
		// leaves, make the following eye pose eligible on the next presentation.
		_nextMouseCursorEyeAnimationFrame = _vm->getAnimationFrameCounter(_vm->_system->getMillis());
	}
}

void ZoombiniGraphics::startMouseCursorEyeAnimation() {
	if (getMouseCursor() == ZoombiniGraphics::kResCursor01_Watch)
		return;

	setMouseCursor(ZoombiniGraphics::kResCursor01_Watch);
}

void ZoombiniGraphics::stopMouseCursorEyeAnimation() {
	if (!isMouseCursorEyeAnimationActive())
		return;

	setMouseCursor(ZoombiniGraphics::kResCursor00_Default);
}

void ZoombiniGraphics::runMouseCursorEyeAnimationFrame(uint32 currentFrame) {
	if (currentFrame < _nextMouseCursorEyeAnimationFrame)
		return;
	_nextMouseCursorEyeAnimationFrame = currentFrame + MohawkEngine_Zoombini::kMouseCursorEyeFrameTicks;

	assert(_mouseCursorEyeAnimationFrameIdx < ARRAYSIZE(_mouseCursorEyeAnimationFrames));

	setMouseCursor(_mouseCursorEyeAnimationFrames[_mouseCursorEyeAnimationFrameIdx]);
	_mouseCursorEyeAnimationFrameIdx = (_mouseCursorEyeAnimationFrameIdx + 1) % ARRAYSIZE(_mouseCursorEyeAnimationFrames);
}

bool ZoombiniGraphics::isMouseCursorEyeAnimationActive() const {
	return getMouseCursor() != ZoombiniGraphics::kResCursor00_Default;
}

// [*] Handle Bitmap
void ZoombiniGraphics::drawBackground(int16 image) {
	drawBackground(kBackScreen, image);
}

void ZoombiniGraphics::drawBackground(ScreenKind screenKind, int16 image) {
	MohawkSurface *imgSurface = findImage(ZmbResource(ZmbResource::kPage, image));
	Graphics::Surface *rawSurface = findImage(ZmbResource(ZmbResource::kPage, image))->getSurface();
	Common::Rect imageRect(0, 0, rawSurface->w, rawSurface->h);
	drawImageSectionToScreen(screenKind, imgSurface, imageRect, _screenRect);
}

void ZoombiniGraphics::drawImage(ScreenKind screenKind, int16 image, const Common::Point &destPos) {
	MohawkSurface *imgSurface = findImage(ZmbResource(ZmbResource::kPage, image));
	Graphics::Surface *rawSurface = imgSurface->getSurface();
	Common::Rect srcRect(0, 0, rawSurface->w, rawSurface->h);
	Common::Rect dstRect(destPos, rawSurface->w, rawSurface->h);
	drawImageSectionToScreen(screenKind, imgSurface, srcRect, dstRect);
}

Common::Rect ZoombiniGraphics::drawShape(ScreenKind screenKind, ZmbResource imgResource, uint16 shapeIdx, const Common::Point &destPos, bool clearBeforeRender, PaletteRemapMode remapColorAssistPalette) {
	const bool shouldRemapColorAssistPalette = remapColorAssistPalette != kPaletteRemapNone &&
											   _vm->isColorBlindModeEnabled();
	return drawSubImage(screenKind, imgResource, shapeIdx - 1, destPos, clearBeforeRender,
						shouldRemapColorAssistPalette ? remapColorAssistPalette : kPaletteRemapNone);
}

Common::Rect ZoombiniGraphics::drawShape(ScreenKind screenKind, ZmbResource imgResource, uint16 shapeIdx, const Common::Rect &destRect, bool clearBeforeRender, PaletteRemapMode remapColorAssistPalette) {
	const bool shouldRemapColorAssistPalette = remapColorAssistPalette != kPaletteRemapNone &&
											   _vm->isColorBlindModeEnabled();
	return drawSubImage(screenKind, imgResource, shapeIdx - 1, destRect, clearBeforeRender,
						shouldRemapColorAssistPalette ? remapColorAssistPalette : kPaletteRemapNone);
}

Common::Rect ZoombiniGraphics::drawShape(ScreenKind screenKind, ZmbResource imgResource, const ZmbHotspot *hotspot, bool clearBeforeRender, PaletteRemapMode remapColorAssistPalette) {
	// (ScummVM addition) Color assist mode for colorblind players
	// Remap palette for the snoid nose, dirt of net, and maze.
	const bool colorBlindModeEnabled = _vm->isColorBlindModeEnabled();
	bool remapColorAssistNose = false;
	if (colorBlindModeEnabled) {
		if (imgResource == ZmbResource(ZmbResource::kSystem, 3000)) {
			// Subimages 342-381 are exposed as renderer shape IDs 343-382.
			const uint16 subImage = hotspot->getSubImageId();
			if (342 <= subImage && subImage <= 381)
				remapColorAssistNose = true;
		} else if (imgResource == ZmbResource(ZmbResource::kSystem, 3100)) {
			// Subimages 396-575 are exposed as renderer shape IDs 397-576.
			const uint16 subImage = hotspot->getSubImageId();
			if (396 <= subImage && subImage <= 575)
				remapColorAssistNose = true;
		} else if (hotspot->_hsId == 3 &&
				   imgResource == ZmbResource(ZmbResource::kPage, 4300)) {
			remapColorAssistNose = true;
		} else if (imgResource == ZmbResource(ZmbResource::kPage, 4400)) {
			if ((10 <= hotspot->_hsId && hotspot->_hsId <= 14) ||
				(30 <= hotspot->_hsId && hotspot->_hsId <= 34))
				remapColorAssistNose = true;
		}
	}
	const bool shouldRemapColorAssistPalette =
		remapColorAssistPalette != kPaletteRemapNone && colorBlindModeEnabled;
	PaletteRemapMode effectiveRemap = kPaletteRemapNone;
	if (remapColorAssistNose)
		effectiveRemap = kPaletteRemapNoseNet;
	else if (shouldRemapColorAssistPalette)
		effectiveRemap = remapColorAssistPalette;
	return drawSubImage(screenKind, imgResource, hotspot->getSubImageId(), hotspot->getPos(), clearBeforeRender, effectiveRemap);
}

Common::Rect ZoombiniGraphics::drawSubImage(ScreenKind screenKind, ZmbResource imgResource, uint16 subImage, const Common::Point &destPos, bool clearBeforeRender, PaletteRemapMode remapColorAssistPalette) {
	if (subImage == UINT16_MAX) { // -1 check
		error("gfx: sub-image index cannot be -1 for %s", imgResource.toString().c_str());
		return Common::Rect();
	}
	MohawkSurface *rawSurface = findSubImage(imgResource, subImage);
	if (!rawSurface) {
		error("gfx: cannot decode sub-image %u from %s", subImage, imgResource.toString().c_str());
		return Common::Rect();
	}
	Graphics::Surface *surface = rawSurface->getSurface();
	if (!surface) {
		error("gfx: decoded sub-image %u from %s has no surface", subImage, imgResource.toString().c_str());
		return Common::Rect();
	}

	// Bail out early if the sprite is entirely off-screen.
	if (destPos.x <= -1 * surface->w || destPos.y <= -1 * surface->h || kScreenWidth <= destPos.x || kScreenHeight <= destPos.y)
		return Common::Rect();

	Common::Rect srcRect(0, 0, surface->w, surface->h);
	Common::Rect dstRect(destPos, surface->w, surface->h);
	return drawImageSectionToScreen(screenKind, rawSurface, srcRect, dstRect, clearBeforeRender, remapColorAssistPalette);
}

Common::Rect ZoombiniGraphics::drawSubImage(ScreenKind screenKind, ZmbResource imgResource, uint16 subImage, const Common::Rect &destRect, bool clearBeforeRender, PaletteRemapMode remapColorAssistPalette) {
	if (subImage == UINT16_MAX) { // -1 check
		error("gfx: sub-image index cannot be -1 for %s", imgResource.toString().c_str());
		return Common::Rect();
	}
	MohawkSurface *rawSurface = findSubImage(imgResource, subImage);
	if (!rawSurface) {
		error("gfx: cannot decode sub-image %u from %s", subImage, imgResource.toString().c_str());
		return Common::Rect();
	}
	Graphics::Surface *surface = rawSurface->getSurface();
	if (!surface) {
		error("gfx: decoded sub-image %u from %s has no surface", subImage, imgResource.toString().c_str());
		return Common::Rect();
	}

	Common::Rect srcRect(0, 0, surface->w, surface->h);

	// If the destRect is larger than shape's actual size, align to the center.
	Common::Point startPos(destRect.left, destRect.top);
	if (surface->w < destRect.width())
		startPos.x += (destRect.width() - surface->w) / 2;
	if (surface->h < destRect.height())
		startPos.y += (destRect.height() - surface->h) / 2;
	Common::Rect dstRect(startPos, surface->w, surface->h);
	return drawImageSectionToScreen(screenKind, rawSurface, srcRect, dstRect, clearBeforeRender, remapColorAssistPalette);
}

Common::Rect ZoombiniGraphics::drawImageSectionToScreen(ScreenKind screenKind, MohawkSurface *mhkSurface, const Common::Rect &srcRect, const Common::Rect &dstRect, bool clearBeforeRender, PaletteRemapMode remapColorAssistPalette) {
	if (!mhkSurface) {
		error("gfx: source bitmap is missing");
		return Common::Rect();
	}
	Graphics::Surface *srcSurface = mhkSurface->getSurface();
	Graphics::Surface *screen = getScreen(screenKind);

	if (!srcSurface) {
		error("gfx: source bitmap has no decoded surface");
		return Common::Rect();
	}
	if (!screen) {
		error("gfx: screen kind %d has no surface", static_cast<int>(screenKind));
		return Common::Rect();
	}
	if (!srcRect.isValidRect() || srcRect.left < 0 || srcRect.top < 0) {
		error("gfx: invalid source rectangle (%d, %d)-(%d, %d)", srcRect.left, srcRect.top, srcRect.right, srcRect.bottom);
		return Common::Rect();
	}
	if (!dstRect.isValidRect()) {
		error("gfx: invalid destination rectangle (%d, %d)-(%d, %d)", dstRect.left, dstRect.top, dstRect.right, dstRect.bottom);
		return Common::Rect();
	}

	Common::Rect clipSrcRect = srcRect;
	Common::Rect clipDstRect = dstRect;
	clipSrcRect.clip(srcSurface->w, srcSurface->h);

	// Bail out early if the sprite is entirely off-screen.
	if (clipDstRect.right <= 0 || clipDstRect.bottom <= 0 || screen->w <= clipDstRect.left || screen->h <= clipDstRect.top)
		return Common::Rect();

	// Left/top clipping: when dstRect extends beyond the left or top screen edge,
	// advance srcRect by the same amount so we skip the off-screen source pixels.
	// Without this, sprites at negative coordinates (e.g. walk-in snoids at x=-50)
	// have their FULL source drawn starting at x=0, appearing fully on-screen
	// instead of being properly clipped.
	if (clipDstRect.left < 0) {
		clipSrcRect.left += -1 * clipDstRect.left;
		clipDstRect.left = 0;
	}
	if (clipDstRect.top < 0) {
		clipSrcRect.top += -1 * clipDstRect.top;
		clipDstRect.top = 0;
	}

	clipDstRect.clip(screen->w, screen->h);

	if (clipSrcRect.isEmpty() || clipDstRect.isEmpty())
		return Common::Rect(0, 0, 0, 0);

	// Trim the source rectangle when the destination extends beyond the right or bottom screen edge.
	if (screen->w < clipDstRect.left + clipSrcRect.width())
		clipSrcRect.right -= (clipDstRect.left + clipSrcRect.width() - screen->w);
	if (screen->h < clipDstRect.top + clipSrcRect.height())
		clipSrcRect.bottom -= (clipDstRect.top + clipSrcRect.height() - screen->h);

	// The logical rectangle is the screen-clipped destination before the dirty-region clip is applied.
	// @ref ZoombiniPage::blitShapes() uses it for Z-sorting and hit testing,
	// so both cover the complete current shape even when only part of it needs to be redrawn.
	Common::Rect logicalRect(clipDstRect.left, clipDstRect.top,
							 clipDstRect.left + clipSrcRect.width(), clipDstRect.top + clipSrcRect.height());
	recordDirtyRect(screenKind, logicalRect);

	// Draw only the intersection of the sprite with each dirty rectangle.
	// This confines drawing to the exact union of those rectangles.
	// Non-dirty pixels persist on the shape-screen from previous frames.
	if (_isRenderClipActive) {
		if (!_hasRenderClipRect)
			return logicalRect;

		// Quick bounding-box reject: if the sprite doesn't touch the dirty
		// region's bounding box at all, skip drawing entirely.
		if (!clipDstRect.intersects(_renderClipBounds))
			return logicalRect;

		// Draw the sprite once for each intersecting dirty rect.
		for (const Common::Rect &dirtyRect : _renderClipRects) {
			Common::Rect subDst = clipDstRect;
			subDst.clip(dirtyRect);
			if (subDst.isEmpty())
				continue;

			Common::Rect subSrc = clipSrcRect;
			subSrc.left += subDst.left - clipDstRect.left;
			subSrc.top += subDst.top - clipDstRect.top;
			subSrc.right -= clipDstRect.right - subDst.right;
			subSrc.bottom -= clipDstRect.bottom - subDst.bottom;

			if (clearBeforeRender)
				screen->fillRect(subDst, kTransparentKey);
			if (remapColorAssistPalette)
				copyRectToSurfaceWithColorAssistPaletteRemap(screen, srcSurface, subDst.left, subDst.top, subSrc, remapColorAssistPalette);
			else
				screen->copyRectToSurfaceWithKey(*srcSurface, subDst.left, subDst.top, subSrc, kTransparentKey);
		}
		return logicalRect;
	}

	if (clearBeforeRender)
		screen->fillRect(clipDstRect, kTransparentKey);

	if (remapColorAssistPalette)
		copyRectToSurfaceWithColorAssistPaletteRemap(screen, srcSurface, clipDstRect.left, clipDstRect.top, clipSrcRect, remapColorAssistPalette);
	else
		screen->copyRectToSurfaceWithKey(*srcSurface, clipDstRect.left, clipDstRect.top, clipSrcRect, kTransparentKey);

	return logicalRect;
}

void ZoombiniGraphics::copyRectToSurfaceWithColorAssistPaletteRemap(Graphics::Surface *screen, Graphics::Surface *source, int destX, int destY, const Common::Rect &sourceRect, PaletteRemapMode remapMode) {
	for (int rowIdx = 0; rowIdx < sourceRect.height(); rowIdx++) {
		const byte *sourceRow = static_cast<const byte *>(source->getBasePtr(sourceRect.left, sourceRect.top + rowIdx));
		byte *destRow = static_cast<byte *>(screen->getBasePtr(destX, destY + rowIdx));

		for (int columnIdx = 0; columnIdx < sourceRect.width(); columnIdx++) {
			const byte paletteIndex = sourceRow[columnIdx];
			if (paletteIndex != static_cast<byte>(kTransparentKey))
				destRow[columnIdx] = remapColorAssistPaletteIndex(paletteIndex, remapMode);
		}
	}
}

byte ZoombiniGraphics::remapColorAssistPaletteIndex(byte paletteIndex, PaletteRemapMode remapMode) const {
	if (remapMode == kPaletteRemapNoseNet) {
		switch (paletteIndex) {
		case kColor23_DarkOrange:
			return static_cast<byte>(kColor0C_DarkGray);
		case kColor24_Orange:
		case kColor25_LightOrange:
		case kColor2B_Yellow:
			return static_cast<byte>(kColor0A_White);
		case kColor18_DarkMagenta:
			return static_cast<byte>(kColor10_DarkCyan);
		case kColor19_Magenta:
			return static_cast<byte>(kColor12_SkyBlue);
		case kColor1A_DarkPink:
			return static_cast<byte>(kColor13_LightBlue);
		case kColor1B_Pink:
			return static_cast<byte>(kColor14_PastelBlue);
		default:
			return paletteIndex;
		}
	} else if (remapMode == kPaletteRemapMazePurple) {
		switch (paletteIndex) {
		case 0xA4:
		case 0xAA:
			return static_cast<byte>(kColor1E_LightAzure);
		case 0xA5:
		case 0xA6:
		case 0xA8:
			return static_cast<byte>(kColor1D_Azure);
		case 0xA7:
		case 0xA9:
			return static_cast<byte>(kColor1C_DarkAzure);
		default:
			return paletteIndex;
		}
	}

	return paletteIndex;
}

void ZoombiniGraphics::drawLine(ScreenKind screenKind, const Common::Point &start, const Common::Point &end, uint32 color) {
	if (_isRenderClipActive && !_hasRenderClipRect)
		return;

	Graphics::Surface *screen = getScreen(screenKind);
	Common::Rect dirtyRect(MIN(start.x, end.x), MIN(start.y, end.y),
						   MAX(start.x, end.x) + 1, MAX(start.y, end.y) + 1);
	recordDirtyRect(screenKind, dirtyRect);
	screen->drawLine(start.x, start.y, end.x, end.y, color);
}

void ZoombiniGraphics::drawThickLine(ScreenKind screenKind, const Common::Point &start, const Common::Point &end, int penX, int penY, uint32 color) {
	if (_isRenderClipActive && !_hasRenderClipRect)
		return;

	Graphics::Surface *screen = getScreen(screenKind);
	Common::Rect dirtyRect(MIN(start.x, end.x) - penX, MIN(start.y, end.y) - penY,
						   MAX(start.x, end.x) + penX + 1, MAX(start.y, end.y) + penY + 1);
	recordDirtyRect(screenKind, dirtyRect);
	screen->drawThickLine(start.x, start.y, end.x, end.y, penX, penY, color);
}

void ZoombiniGraphics::clearArea(ScreenKind screenKind, ZmbDrawRecord *record) {
	fillArea(screenKind, record->_drawnRect, kTransparentKey);
}

void ZoombiniGraphics::clearArea(ScreenKind screenKind, ZmbResource imgResource, const ZmbHotspot *hotspot) {
	fillArea(screenKind, imgResource, hotspot, kTransparentKey);
}

void ZoombiniGraphics::clearArea(ScreenKind screenKind, const Common::Rect &rect) {
	fillArea(screenKind, rect, kTransparentKey);
}

void ZoombiniGraphics::fillArea(ScreenKind screenKind, ZmbDrawRecord *record, uint32 color) {
	fillArea(screenKind, record->_drawnRect, color);
}

void ZoombiniGraphics::fillArea(ScreenKind screenKind, ZmbResource imgResource, const ZmbHotspot *hotspot, uint32 color) {
	MohawkSurface *rawSurface = findShape(imgResource, hotspot->getSubImageId());
	Graphics::Surface *surface = rawSurface->getSurface();

	Common::Rect dstRect(hotspot->getPos(), surface->w, surface->h);
	fillArea(screenKind, dstRect, color);
}

void ZoombiniGraphics::fillArea(ScreenKind screenKind, const Common::Rect &rect, uint32 color) {
	Graphics::Surface *screen = getScreen(screenKind);

	recordDirtyRect(screenKind, rect);
	if (_isRenderClipActive) {
		if (!_hasRenderClipRect)
			return;

		for (const Common::Rect &dirtyRect : _renderClipRects) {
			Common::Rect clipped = rect;
			clipped.clip(dirtyRect);
			if (!clipped.isEmpty())
				screen->fillRect(clipped, color);
		}
		return;
	}
	if (!rect.isEmpty())
		screen->fillRect(rect, color);
}

void ZoombiniGraphics::fillArea(ScreenKind screenKind, uint32 color) {
	fillArea(screenKind, _screenRect, color);
}

void ZoombiniGraphics::drawText(ScreenKind screenKind, uint32 textKey, const Common::Rect &destRect) {
	drawText(screenKind, textKey, destRect, TextConf());
}

void ZoombiniGraphics::drawText(ScreenKind screenKind, uint32 textKey, const Common::Rect &destRect, const TextConf &tc) {
	const Common::U32String &text = _vm->_text->getLocalizedString(textKey);
	drawText(screenKind, text, destRect, tc);
}

void ZoombiniGraphics::drawText(ScreenKind screenKind, const Common::U32String &text, const Common::Rect &destRect) {
	return drawText(screenKind, text, destRect, TextConf());
}

void ZoombiniGraphics::drawText(ScreenKind screenKind, const Common::U32String &text, const Common::Rect &destRect, const TextConf &tc) {
	const Graphics::Font *font = _vm->_text->getFont(tc._fontUsage);
	if (!font) {
		error("gfx: cannot open fontfile of kind %u", static_cast<uint32>(tc._fontUsage));
		return;
	}
	const bool useAntialiasing = _vm->_text->fontUsesAntialiasing(tc._fontUsage);

	const Common::Array<Common::U32String> &lines = prepareTextLines(text, font, tc._wordWrap, destRect.width());

	// Align all passes from the unexpanded text bounds.
	// Offset only the four outline passes so the main text does not move.
	const Common::Point boundSize = getTextLinesBounds(font, false, lines);

	// Keep the caller rectangle unchanged for normal wrapped text so alignment and clipping share the same bounds.
	Common::Rect drawRect = destRect;
	if (!tc._wordWrap)
		expandUnwrappedTextRect(drawRect, boundSize.x, tc._hAlign);

	// Handle background fill
	uint32 fillBackgroundPalette = kTransparentKey;
	if (tc._fillBackground)
		fillBackgroundPalette = tc._fillBackgroundKey;

	// Virtualize Vertical Aligment
	if (0 < text.size()) {
		switch (tc._vAlign) {
		case Graphics::kTextAlignStart:
		case Graphics::kTextAlignLeft:
			// Do nothing
			break;
		case Graphics::kTextAlignCenter:
			drawRect.top = (drawRect.top + drawRect.bottom - boundSize.y) / 2;
			drawRect.bottom = drawRect.top + boundSize.y;
			break;
		case Graphics::kTextAlignEnd:
		case Graphics::kTextAlignRight:
			drawRect.top = drawRect.bottom - boundSize.y;
			break;
		default:
			error("gfx: invalid vertical alignment %d", tc._vAlign);
			break;
		}
	}

	// Draw the four one-pixel outline passes before the main text.
	if (tc._outlineEffect) {
		for (uint32 i = 0; i < 4; i++) {
			Common::Rect outlineRect = drawRect;
			int16 xDelta = 0;
			int16 yDelta = 0;
			switch (i) {
			case 0:
				xDelta -= 1;
				break;
			case 1:
				yDelta -= 1;
				break;
			case 2:
				xDelta += 1;
				break;
			case 3:
				yDelta += 1;
				break;
			}
			outlineRect.left += xDelta;
			outlineRect.right += xDelta;
			outlineRect.top += yDelta;
			outlineRect.bottom += yDelta;

			recordDirtyRect(screenKind, outlineRect);
			drawTextLines(screenKind, font, lines, outlineRect, tc._outlinePalette, tc._hAlign, useAntialiasing, fillBackgroundPalette);
		}
	}

	recordDirtyRect(screenKind, drawRect);
	drawTextLines(screenKind, font, lines, drawRect, tc._textPalette, tc._hAlign, useAntialiasing, fillBackgroundPalette);
}

int ZoombiniGraphics::getTextWidth(const Common::U32String &text, const TextConf &tc) {
	const Graphics::Font *font = _vm->_text->getFont(tc._fontUsage);
	if (!font)
		error("gfx: cannot open fontfile of kind %u", static_cast<uint32>(tc._fontUsage));

	return font->getStringWidth(text);
}

int16 ZoombiniGraphics::getFontHeight(const TextConf &tc) {
	const Graphics::Font *font = _vm->_text->getFont(tc._fontUsage);
	if (!font)
		error("gfx: cannot open fontfile of kind %u", static_cast<uint32>(tc._fontUsage));

	return font->getFontHeight();
}

void ZoombiniGraphics::expandUnwrappedTextRect(Common::Rect &rect, int textWidth, Graphics::TextAlign hAlign) {
	const int16 extraWidth = textWidth - rect.width();
	if (extraWidth <= 0)
		return;

	switch (hAlign) {
	case Graphics::kTextAlignCenter: {
		// Odd pixel counts cannot preserve an integer rectangle center exactly.
		// Put the extra pixel on the right to minimize the center displacement.
		const int16 leftExpansion = extraWidth / 2;
		rect.left -= leftExpansion;
		rect.right += extraWidth - leftExpansion;
		break;
	}
	case Graphics::kTextAlignLeft:
	case Graphics::kTextAlignStart:
		rect.right += extraWidth;
		break;
	case Graphics::kTextAlignRight:
	case Graphics::kTextAlignEnd:
		rect.left -= extraWidth;
		break;
	default:
		error("gfx: invalid horizontal alignment %d", hAlign);
		break;
	}
}

Common::Array<Common::U32String> ZoombiniGraphics::prepareTextLines(const Common::U32String &text, const Graphics::Font *font, bool wordWrap, int16 targetWidth) {
	// Tokenize strings with CR, LF or CRLF
	Common::Array<Common::U32String> lines = ZoombiniText::tokenizeLines(text);
	if (!wordWrap)
		return lines;

	// GDI DrawTextA with DT_WORDBREAK accepts a line whose extent exactly equals the caller rectangle width.
	// @ref Graphics::Font::wordWrapText() treats that equality as overflow because its width test uses >=.
	// Add one pixel only for the line-break decision; drawing still uses the caller's rectangle and clipping.
	const int wordWrapWidth = targetWidth + 1;
	Common::Array<Common::U32String> newLines;
	for (const Common::U32String &line : lines) {
		// An explicit empty source line still advances the text by one line.
		// Font::wordWrapText() returns no entries for it, so preserve it here.
		if (line.empty()) {
			newLines.push_back(line);
			continue;
		}

		// Splice rawLine into a word-wrapped lines
		Common::Array<Common::U32String> wrapLines;
		font->wordWrapText(line, wordWrapWidth, wrapLines);
		newLines.push_back(wrapLines);
	}
	return newLines;
}

Common::Point ZoombiniGraphics::getTextLinesBounds(const Graphics::Font *font, bool outlineEffect, const Common::Array<Common::U32String> &lines) {
	// Use @ref Graphics::Font::getFontHeight() for line height to match GDI DrawTextA with DT_EXTERNALLEADING.
	// The returned height includes the ascender, descender, and line gap.
	const int16 lineHeight = font->getFontHeight();
	int16 drawTotalHeight = 0;
	Common::Point boundSize;
	for (const Common::U32String &line : lines) {
		Common::Rect bbox = font->getBoundingBox(line, 0, 0, _screenRect.width(), Graphics::kTextAlignLeft);

		drawTotalHeight += lineHeight;
		boundSize.x = MAX(boundSize.x, static_cast<int16>(bbox.width()));
		boundSize.y = MAX(boundSize.y, drawTotalHeight);
	}

	if (outlineEffect) {
		// Outline adds 1 pixel in each direction (L/U/R/D)
		boundSize.x += 2;
		boundSize.y += 2;
	}

	return boundSize;
}

void ZoombiniGraphics::copyTextPixels(Graphics::Surface *textSurface, Graphics::Surface *screen, const Common::Rect &destRect, const Common::Rect &copyRect) {
	const int localLeft = copyRect.left - destRect.left;
	const int localTop = copyRect.top - destRect.top;
	for (int rowIdx = 0; rowIdx < copyRect.height(); rowIdx++) {
		const byte *src = reinterpret_cast<const byte *>(textSurface->getBasePtr(localLeft, localTop + rowIdx));
		byte *dst = reinterpret_cast<byte *>(screen->getBasePtr(copyRect.left, copyRect.top + rowIdx));
		for (int columnIdx = 0; columnIdx < copyRect.width(); columnIdx++) {
			if (src[columnIdx] != kTransparentKey)
				dst[columnIdx] = src[columnIdx];
		}
	}
}

void ZoombiniGraphics::blendTextPixels(Graphics::Surface *textSurface, Graphics::Surface *screen, const Common::Rect &destRect, const Common::Rect &copyRect, uint32 palette) {
	assert(palette < 256);
	assert(textSurface->format.bytesPerPixel == 4);
	assert(screen->format.bytesPerPixel == 1);

	_textPaletteLookup.setPalette(_paletteBytes, ARRAYSIZE(_paletteBytes) / 3);
	const byte textRed = _paletteBytes[palette * 3 + 0];
	const byte textGreen = _paletteBytes[palette * 3 + 1];
	const byte textBlue = _paletteBytes[palette * 3 + 2];
	const int localLeft = copyRect.left - destRect.left;
	const int localTop = copyRect.top - destRect.top;

	for (int rowIdx = 0; rowIdx < copyRect.height(); rowIdx++) {
		const uint32 *src = static_cast<const uint32 *>(textSurface->getBasePtr(localLeft, localTop + rowIdx));
		byte *dst = static_cast<byte *>(screen->getBasePtr(copyRect.left, copyRect.top + rowIdx));
		for (int columnIdx = 0; columnIdx < copyRect.width(); columnIdx++) {
			const byte alpha = static_cast<byte>((src[columnIdx] >> textSurface->format.aShift) & 0xFF);
			if (alpha == 0)
				continue;
			if (alpha == 0xFF) {
				dst[columnIdx] = static_cast<byte>(palette);
				continue;
			}

			const uint32 inverseAlpha = 0xFF - alpha;
			const byte backgroundPalette = dst[columnIdx];
			const byte backgroundRed = _paletteBytes[backgroundPalette * 3 + 0];
			const byte backgroundGreen = _paletteBytes[backgroundPalette * 3 + 1];
			const byte backgroundBlue = _paletteBytes[backgroundPalette * 3 + 2];
			const byte blendedRed = static_cast<byte>((static_cast<uint32>(textRed) * alpha +
													   static_cast<uint32>(backgroundRed) * inverseAlpha + 0x7F) /
													  0xFF);
			const byte blendedGreen = static_cast<byte>((static_cast<uint32>(textGreen) * alpha +
														 static_cast<uint32>(backgroundGreen) * inverseAlpha + 0x7F) /
														0xFF);
			const byte blendedBlue = static_cast<byte>((static_cast<uint32>(textBlue) * alpha +
														static_cast<uint32>(backgroundBlue) * inverseAlpha + 0x7F) /
													   0xFF);
			dst[columnIdx] = _textPaletteLookup.findBestColor(blendedRed, blendedGreen, blendedBlue);
		}
	}
}

void ZoombiniGraphics::drawTextLines(ScreenKind screenKind, const Graphics::Font *font, const Common::Array<Common::U32String> &lines, const Common::Rect &destRect, uint32 palette, Graphics::TextAlign hAlign, bool useAntialiasing, uint32 fillBackgroundColor) {
	// Clip text to the caller's destination rectangle.
	if (destRect.isEmpty())
		return;

	Graphics::Surface *screen = _vm->_gfx->getScreen(screenKind);
	assert(screen->format.bytesPerPixel == 1);
	if (useAntialiasing)
		assert(palette < 256);

	// Render to a transparent local surface first. This gives every text path
	// the same exact caller-rectangle clipping that GDI applies, including
	// partially visible glyphs at the top, bottom, left and right edges.
	Graphics::Surface textSurface;
	if (useAntialiasing) {
		textSurface.create(destRect.width(), destRect.height(), Graphics::PixelFormat::createFormatARGB32());
		textSurface.fillRect(textSurface.getRect(), 0);
	} else {
		textSurface.create(destRect.width(), destRect.height(), screen->format);
		textSurface.fillRect(textSurface.getRect(), kTransparentKey);
	}

	// Use @ref Graphics::Font::getFontHeight() for line advancement to match GDI DrawTextA with DT_EXTERNALLEADING.
	const int lineHeight = font->getFontHeight();
	Common::Rect drawRect = destRect;

	for (uint32 i = 0; i < lines.size(); i++) {
		const Common::U32String &line = lines[i];

		// Clip: skip lines whose top is below the dest rect bottom (matching GDI IntersectClipRect)
		if (destRect.bottom <= drawRect.top)
			break;

		const int localTop = drawRect.top - destRect.top;

		// Background is for debug purposes, Zoombini game itself does not use this feature.
		if (fillBackgroundColor != kTransparentKey) {
			const Common::Rect bbox = font->getBoundingBox(line, 0, localTop, destRect.width(), hAlign);
			if (useAntialiasing) {
				Common::Rect screenBbox = bbox;
				screenBbox.translate(destRect.left, destRect.top);
				screenBbox.clip(destRect);
				screenBbox.clip(_screenRect);
				fillArea(screenKind, screenBbox, fillBackgroundColor);
			} else {
				textSurface.fillRect(bbox, fillBackgroundColor);
			}
		}

		if (useAntialiasing) {
			const byte textRed = _paletteBytes[palette * 3 + 0];
			const byte textGreen = _paletteBytes[palette * 3 + 1];
			const byte textBlue = _paletteBytes[palette * 3 + 2];
			const uint32 textColor = textSurface.format.ARGBToColor(0xFF, textRed, textGreen, textBlue);
			font->drawAlphaString(&textSurface, line, 0, localTop, destRect.width(), textColor, hAlign);
		} else {
			font->drawString(&textSurface, line, 0, localTop, destRect.width(), palette, hAlign);
		}

		if (i + 1 < lines.size()) {
			drawRect.top += lineHeight;
			drawRect.bottom += lineHeight;
		}
	}

	Common::Rect screenRect = destRect;
	screenRect.clip(_screenRect);
	if (screenRect.isEmpty()) {
		textSurface.free();
		return;
	}

	// Preserve the active render region in addition to the caller's destination rectangle.
	if (_isRenderClipActive) {
		if (_hasRenderClipRect) {
			for (const Common::Rect &dirtyRect : _renderClipRects) {
				Common::Rect clipped = screenRect;
				clipped.clip(dirtyRect);
				if (!clipped.isEmpty()) {
					if (useAntialiasing)
						blendTextPixels(&textSurface, screen, destRect, clipped, palette);
					else
						copyTextPixels(&textSurface, screen, destRect, clipped);
				}
			}
		}
	} else {
		if (useAntialiasing)
			blendTextPixels(&textSurface, screen, destRect, screenRect, palette);
		else
			copyTextPixels(&textSurface, screen, destRect, screenRect);
	}

	textSurface.free();
}

// [*] Transitions and effects
void ZoombiniGraphics::queueFadeEffect(FadeType type, uint32 duration) {
	_fadeQueue.push(FadeEffect(type, duration));
}

void ZoombiniGraphics::queuePaletteScaleEffect(uint16 startEntry, uint16 count, uint8 scalePercent, uint32 duration) {
	assert(startEntry <= 256);
	assert(0 < duration);

	FadeEffect fadeEffect(kFadePartialPalette, duration);
	const uint16 endEntry = static_cast<uint16>(MIN<uint32>(static_cast<uint32>(startEntry) + count, 256));
	fadeEffect._startEntry = startEntry;
	fadeEffect._entryCount = endEntry - startEntry;
	fadeEffect._scalePercent = scalePercent;
	memcpy(fadeEffect._sourcePalette, _paletteBytes, sizeof(fadeEffect._sourcePalette));

	for (uint16 entryIdx = startEntry; entryIdx < endEntry; entryIdx++) {
		for (uint16 channel = 0; channel < 3; channel++) {
			const uint16 paletteOffset = entryIdx * 3 + channel;
			_paletteBytes[paletteOffset] = static_cast<byte>(static_cast<uint16>(_paletteBytes[paletteOffset]) * scalePercent / 100);
		}
	}

	_fadeQueue.push(fadeEffect);
}

bool ZoombiniGraphics::applyFadeEffect(uint32 currentTime) {
	if (_fadeQueue.empty())
		return false;

	FadeEffect &fe = _fadeQueue.front();
	if (!fe._isFading) {
		fe._isFading = true;
		// A modal dialog can run a nested frame loop after the caller captured
		// currentTime. Start a newly dequeued effect at its actual first-apply
		// time so that stale outer-frame time cannot skip part or all of it.
		currentTime = _vm->_system->getMillis();
		fe._startTime = currentTime;
	}
	uint32 steps = fe._duration / MohawkEngine_Zoombini::kTargetFrameTimeMs;
	uint32 elapsedTime = currentTime - fe._startTime;
	if (elapsedTime <= fe._duration) { // Effect in progress
		uint32 stepIdx = MIN<uint32>(elapsedTime / MohawkEngine_Zoombini::kTargetFrameTimeMs, steps);
		switch (fe._type) {
		case kFadeIn:
			dimPalette(stepIdx, steps);
			break;
		case kFadeOut:
			dimPalette(steps - stepIdx, steps);
			break;
		case kFadePartialPalette: {
			uint16 blendIndex = static_cast<uint16>(elapsedTime * 256 / fe._duration);
			blendIndex &= ~3;
			applyPartialPaletteFade(fe, blendIndex);
			break;
		}
		default:
			error("gfx: invalid fade effect type: %d", fe._type);
			break;
		}
		return true;
	} else { // Effect completed
		switch (fe._type) {
		case kFadeIn:
			dimPalette(steps, steps);
			break;
		case kFadeOut:
			dimPalette(0, steps);
			break;
		case kFadePartialPalette:
			applyPartialPaletteFade(fe, 256);
			break;
		default:
			error("gfx: invalid fade effect type: %d", fe._type);
			break;
		}

		_fadeQueue.pop();
		// Keep the current page alive until every effect already queued for it has completed.
		return !_fadeQueue.empty();
	}
}

bool ZoombiniGraphics::isFading() const {
	// A queued effect owns input before its first application frame.
	return !_fadeQueue.empty();
}

void ZoombiniGraphics::dimPalette(uint16 fadeStepIdx, uint16 steps) {
	assert(fadeStepIdx <= steps);
	assert(0 <= fadeStepIdx);

	const size_t bufSize = ARRAYSIZE(_paletteBytes);
	byte fadePalette[3 * 256];

	for (uint16 bufferIdx = 0; bufferIdx < bufSize; bufferIdx++)
		fadePalette[bufferIdx] = static_cast<byte>(static_cast<uint32>(_paletteBytes[bufferIdx]) * fadeStepIdx / steps);

	_vm->_system->getPaletteManager()->setPalette(fadePalette, 0, bufSize / 3);
}

void ZoombiniGraphics::applyPartialPaletteFade(const FadeEffect &fadeEffect, uint16 blendIndex) {
	assert(blendIndex <= 256);

	byte transitionPalette[3 * 256];
	const uint16 endEntry = fadeEffect._startEntry + fadeEffect._entryCount;
	for (uint16 entryIdx = fadeEffect._startEntry; entryIdx < endEntry; entryIdx++) {
		for (uint16 channel = 0; channel < 3; channel++) {
			const uint16 paletteOffset = entryIdx * 3 + channel;
			const byte source = fadeEffect._sourcePalette[paletteOffset];
			const byte target = static_cast<byte>(static_cast<uint16>(source) * fadeEffect._scalePercent / 100);
			const uint16 delta = static_cast<uint16>(static_cast<int32>(blendIndex) *
													 (static_cast<int32>(target) - source));
			transitionPalette[paletteOffset] = static_cast<byte>(source + (delta >> 8));
		}
	}

	_vm->_system->getPaletteManager()->setPalette(
		&transitionPalette[fadeEffect._startEntry * 3], fadeEffect._startEntry, fadeEffect._entryCount);
}

MohawkSurface *ZoombiniGraphics::findImage(ZmbResource imgResource) {
	if (imgResource._id < 0) {
		error("gfx: invalid image resource id %d", imgResource._id);
		return nullptr;
	}

	switch (imgResource._archiveKind) {
	case ZmbResource::kSystem:
		if (!_sysImageCache.contains(imgResource._id))
			_sysImageCache[imgResource._id] = decodeImage(imgResource);
		return _sysImageCache[imgResource._id];
	case ZmbResource::kPage:
		return GraphicsManager::findImage(static_cast<uint16>(imgResource._id));
	default:
		error("gfx: invalid archive kind(%d)", static_cast<int>(imgResource._archiveKind));
		break;
	}
	return nullptr;
}

void ZoombiniGraphics::preloadImage(ZmbResource imgResource) {
	// This path accepts only a single-image tBMP.
	// Compound resources must use @ref ZoombiniGraphics::preloadShapes().
	findImage(imgResource);
}

MohawkSurface *ZoombiniGraphics::findShape(ZmbResource imgResource, uint16 shapeIdx) {
	return findSubImage(imgResource, shapeIdx - 1);
}

MohawkSurface *ZoombiniGraphics::findSubImage(ZmbResource imgResource, uint16 subImage) {
	if (imgResource._id < 0) {
		error("gfx: invalid sub-image resource id %d", imgResource._id);
		return nullptr;
	}

	switch (imgResource._archiveKind) {
	case ZmbResource::kSystem: {
		if (!_sysSubImageCache.contains(imgResource._id))
			_sysSubImageCache[imgResource._id] = decodeImages(imgResource);
		Common::Array<MohawkSurface *> &sysImages = _sysSubImageCache[imgResource._id];
		if (sysImages.size() <= subImage) {
			error("gfx: sub-image %u is out of bounds (size %u) for system resource %d", subImage, sysImages.size(), imgResource._id);
			return nullptr;
		}
		return sysImages[subImage];
	}
	case ZmbResource::kPage:
		return GraphicsManager::findSubImage(static_cast<uint16>(imgResource._id), subImage);
	default:
		error("gfx: invalid archive kind(%d)", static_cast<int>(imgResource._archiveKind));
		break;
	}
	return nullptr;
}

void ZoombiniGraphics::preloadShapes(ZmbResource imgResource) {
	// Compound sub-images use a separate cache from @ref ZoombiniGraphics::preloadImage().
	getShapeCount(imgResource);
}

Common::Rect ZoombiniGraphics::getShapeSize(ZmbResource imgResource, uint16 shapeIdx) {
	return getSubImageSize(imgResource, shapeIdx - 1);
}

Common::Rect ZoombiniGraphics::getSubImageSize(ZmbResource imgResource, uint16 subImage) {
	const MohawkSurface *mhkSurface = findSubImage(imgResource, subImage);
	if (!mhkSurface) {
		error("gfx: cannot find shape-idx(%u) in image(%d)", subImage, imgResource._id);
		return Common::Rect();
	}
	const Graphics::Surface *imgSurface = mhkSurface->getSurface();
	if (!imgSurface) {
		error("gfx: cannot get image surface from sub-image(%u) in image(%d)", subImage, imgResource._id);
		return Common::Rect();
	}
	return Common::Rect(imgSurface->w, imgSurface->h);
}

uint32 ZoombiniGraphics::getShapeCount(ZmbResource imgResource) {
	if (imgResource._id < 0) {
		error("gfx: invalid shape-count resource id %d", imgResource._id);
		return 0;
	}

	switch (imgResource._archiveKind) {
	case ZmbResource::kSystem:
		if (!_sysSubImageCache.contains(imgResource._id))
			_sysSubImageCache[imgResource._id] = decodeImages(imgResource);
		return _sysSubImageCache[imgResource._id].size();
	case ZmbResource::kPage:
		return GraphicsManager::getSubImageCount(static_cast<uint16>(imgResource._id));
	default:
		error("gfx: invalid archive kind: %d", static_cast<int>(imgResource._archiveKind));
		break;
	}
	return 0;
}

void ZoombiniGraphics::clearCommonCache() {
	for (Common::HashMap<int16, MohawkSurface *>::iterator it = _sysImageCache.begin(); it != _sysImageCache.end(); it++)
		delete it->_value;

	for (Common::HashMap<int16, Common::Array<MohawkSurface *>>::iterator it = _sysSubImageCache.begin(); it != _sysSubImageCache.end(); it++) {
		Common::Array<MohawkSurface *> &array = it->_value;
		for (MohawkSurface *surface : array)
			delete surface;
	}

	_sysImageCache.clear();
	_sysSubImageCache.clear();
}

// [*] 256color Palette
void ZoombiniGraphics::setPalette(uint16 id) {
	if (INT16_MAX < id) {
		error("gfx: palette resource id %u is outside the authored int16 range", id);
		return;
	}
	if (!readPaletteInternal(static_cast<int16>(id), _unmodifiedPaletteBytes, ARRAYSIZE(_unmodifiedPaletteBytes), false)) {
		error("gfx: required palette SHPL p:%04u is malformed", id);
		return;
	}

	refreshPalette();
}

void ZoombiniGraphics::rotatePaletteRight(uint16 startEntry, uint16 count) {
	if (count < 2 || 256 <= startEntry)
		return;

	uint16 endEntry = static_cast<uint16>(MIN<uint32>(static_cast<uint32>(startEntry) + count, 256));
	if (endEntry - startEntry < 2)
		return;

	byte saved[3];
	const uint16 lastEntry = endEntry - 1;
	memcpy(saved, &_unmodifiedPaletteBytes[lastEntry * 3], sizeof(saved));
	memmove(&_unmodifiedPaletteBytes[(startEntry + 1) * 3], &_unmodifiedPaletteBytes[startEntry * 3], (endEntry - startEntry - 1) * 3);
	memcpy(&_unmodifiedPaletteBytes[startEntry * 3], saved, sizeof(saved));

	refreshPalette();
}

bool ZoombiniGraphics::readPalette(int16 id, byte *destBuf, size_t destBufSize) {
	return readPaletteInternal(id, destBuf, destBufSize, _vm->useBrightenPalette());
}

bool ZoombiniGraphics::readPaletteInternal(int16 id, byte *destBuf, size_t destBufSize, bool applyBrightness) {
	if (!destBuf || destBufSize == 0)
		return false;

	// Do not clear @p destBuf because entries that are not overwritten must retain their previous values.
	Common::SeekableReadStream *shplStream = _vm->getResource(ID_SHPL, ZmbResource(ZmbResource::kPage, id));
	if (!ZmbResource::hasBytes(shplStream, 8)) {
		warning("ZmbGraphics: Missing or truncated SHPL %d header", id);
		delete shplStream;
		return false;
	}
	const int16 shplId = shplStream->readSint16BE();
	const uint16 shapeCount = shplStream->readUint16BE();
	uint16 paletteColorStart = shplStream->readUint16BE();
	uint16 paletteColorCount = shplStream->readUint16BE();
	const uint32 expectedSize = 8 + static_cast<uint32>(paletteColorCount) * 4;
	const bool shapeRangeFits = 0 <= shplId && shapeCount != 0 && static_cast<uint32>(shapeCount - 1) <= static_cast<uint32>(INT16_MAX - shplId);
	if (shplId != id || !shapeRangeFits ||
		255 < paletteColorStart || 256 - paletteColorStart < paletteColorCount ||
		!ZmbResource::hasSize(shplStream, expectedSize, expectedSize)) {
		warning("ZmbGraphics: Invalid SHPL %d header, range, or payload", id);
		delete shplStream;
		return false;
	}
	const uint16 paletteColorEnd = paletteColorStart + paletteColorCount;

	// Is the destination buffer large enough?
	if (destBufSize < 3 * paletteColorEnd) {
		delete shplStream;
		return false;
	}

	for (uint16 i = paletteColorStart; i < paletteColorEnd; i++) {
		destBuf[i * 3 + 0] = shplStream->readByte();
		destBuf[i * 3 + 1] = shplStream->readByte();
		destBuf[i * 3 + 2] = shplStream->readByte();
		shplStream->readByte(); // Skip flags byte
	}

	delete shplStream;

	// Apply brightness adjustment when enabled.
	if (applyBrightness) {
		for (uint16 entryIdx = paletteColorStart; entryIdx < paletteColorStart + paletteColorCount; entryIdx++) {
			for (int ch = 0; ch < 3; ch++) {
				byte &v = destBuf[entryIdx * 3 + ch];
				if (v != 0)
					v = v + 31 - (v >> 3);
			}
		}
	}

	return true;
}

void ZoombiniGraphics::refreshPalette() {
	memcpy(_paletteBytes, _unmodifiedPaletteBytes, sizeof(_paletteBytes));

	if (_vm->useBrightenPalette()) {
		for (uint16 colorIdx = 0; colorIdx < 256; colorIdx++) {
			for (int ch = 0; ch < 3; ch++) {
				byte &v = _paletteBytes[colorIdx * 3 + ch];
				if (v != 0)
					v = v + 31 - (v >> 3);
			}
		}
	}

	_vm->_system->getPaletteManager()->setPalette(_paletteBytes, 0, ARRAYSIZE(_paletteBytes) / 3);
}

void ZoombiniGraphics::clearPalette() {
	// Set the palette to all black
	memset(_unmodifiedPaletteBytes, 0, sizeof(_unmodifiedPaletteBytes));
	memset(_paletteBytes, 0, sizeof(_paletteBytes));
	_vm->_system->getPaletteManager()->setPalette(_paletteBytes, 0, ARRAYSIZE(_paletteBytes) / 3);
}

MohawkSurface *ZoombiniGraphics::decodeImage(uint16 id) {
	if (0x7FFF < id)
		error("gfx: image resource id %u is outside the authored int16 range", id);
	return _bmpDecoder->decodeImage(_vm->getResource(ID_TBMP, ZmbResource(ZmbResource::kPage, static_cast<int16>(id))));
}

MohawkSurface *ZoombiniGraphics::decodeImage(ZmbResource imgResource) {
	return _bmpDecoder->decodeImage(_vm->getResource(ID_TBMP, imgResource));
}

Common::Array<MohawkSurface *> ZoombiniGraphics::decodeImages(uint16 id) {
	if (0x7FFF < id)
		error("gfx: image resource id %u is outside the authored int16 range", id);
	return _bmpDecoder->decodeImages(_vm->getResource(ID_TBMP, ZmbResource(ZmbResource::kPage, static_cast<int16>(id))));
}

Common::Array<MohawkSurface *> ZoombiniGraphics::decodeImages(ZmbResource imgResource) {
	return _bmpDecoder->decodeImages(_vm->getResource(ID_TBMP, imgResource));
}

} // End of namespace Mohawk
