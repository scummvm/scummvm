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

#include "common/system.h"
#include "common/textconsole.h"
#include "common/translation.h"

#include "engines/util.h"
#include "graphics/blit.h"
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

namespace Mohawk {

ZoombiniGraphics::ZoombiniGraphics(MohawkEngine_Zoombini *vm)
	: GraphicsManager(), _vm(vm), _bmpDecoder(new MohawkBitmap()) {
	initGraphics(_screenRect.width(), _screenRect.height());
	clearPalette();

	_pixelFormat = Graphics::PixelFormat::createFormatCLUT8();

	fillColorAssistPaletteRemapTable(_colorAssistNoseNetRemapTable, kPaletteRemapNoseNet);
	fillColorAssistPaletteRemapTable(_colorAssistMazePurpleRemapTable, kPaletteRemapMazePurple);

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

void ZoombiniGraphics::showStartupLoadingScreen() {
	const Graphics::Font *font = FontMan.getFontByUsage(Graphics::FontManager::kLocalizedFont);
	if (!font)
		return;

	const Common::U32String loadingText = _("Loading game...");

	// Setup a temporary CLUT8 grayscale palette, which will get its antialiased text rendered.
	// 0 is #000000, 254 is #FEFEFE. 255 is set to a key.
	byte loadingPalette[3 * 256] = {};
	for (uint16 palIdx = 0; palIdx < kBlackKey; palIdx++) {
		const byte val = static_cast<byte>(palIdx);
		loadingPalette[palIdx * 3 + 0] = val;
		loadingPalette[palIdx * 3 + 1] = val;
		loadingPalette[palIdx * 3 + 2] = val;
	}
	_vm->_system->getPaletteManager()->setPalette(loadingPalette, 0, ARRAYSIZE(loadingPalette) / 3);

	// Render text with 254: #FEFEFE.
	const byte fgColor = static_cast<byte>(kBlackKey - 1);

	_shapeScreen->fillRect(_screenRect, kTransparentKey);
	const int textX = (_screenRect.width() - font->getStringWidth(loadingText)) / 2;
	const int textY = (_screenRect.height() - font->getFontHeight()) / 2;

#ifdef USE_RGB_COLOR
	// Preserve font hinting with best effort if a 2bpp/4bpp RGB surface is available.
	const Common::Rect bboxRect = font->getBoundingBox(loadingText);
	if (!bboxRect.isEmpty()) {
		// Render only the glyph bounds in true color so FreeType coverage survives.
		const Graphics::PixelFormat textFormat = Graphics::PixelFormat::createFormatBGRA32();
		Graphics::Surface textSurface;
		textSurface.create(bboxRect.width(), bboxRect.height(), textFormat);
		textSurface.fillRect(textSurface.getRect(), 0);

		// Render text to a 4bpp surface with explicit #FEFEFE RGB color, possibly with a hinting.
		const uint32 fgRgbColor = textFormat.ARGBToColor(0xFF, fgColor, fgColor, fgColor);
		font->drawAlphaString(&textSurface, loadingText, -bboxRect.left, -bboxRect.top,
							  textSurface.w, fgRgbColor, Graphics::kTextAlignLeft, 0, false, true);

		// Quantize pixles of 4bpp surface into a CLUT8 grayscale surface.
		const Common::Rect destRect(textX + bboxRect.left, textY + bboxRect.top,
									textX + bboxRect.right, textY + bboxRect.bottom);
		Common::Rect clippedDestRect = destRect;
		clippedDestRect.clip(_screenRect);
		if (!clippedDestRect.isEmpty())
			quantizeHintedTextPixels(&textSurface, _shapeScreen, destRect, clippedDestRect, loadingPalette,
									 ARRAYSIZE(loadingPalette) / 3, fgColor);

		textSurface.free();
	}
#else
	// RGB surface is not available.
	// Draw to a CLUT8 surface directly without hinting.
	font->drawString(_shapeScreen, loadingText, textX, textY, _screenRect.width(), fgColor, Graphics::kTextAlignLeft, 0, false, true);
#endif

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
		if (_isPresentationDraw && _isRenderClipActive) {
			// Position interpolation path
			for (const Common::Rect &clip : _renderClipRects) {
				Common::Rect damage = clipped;
				damage.clip(clip);
				if (!damage.isEmpty()) {
					_isScreenDirty = true;
					mergeRectIntoRegion(_screenDirtyRects, damage);
				}
			}
		} else {
			// Non-interpolating path
			_isScreenDirty = true;
			mergeRectIntoRegion(_screenDirtyRects, clipped);
		}
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
		_nextMouseCursorEyeAnimationFrame = _vm->getAnimationFrameCounter(_vm->getTotalPlayTime());
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
	Common::Rect imageRect = rawSurface->getRect();
	drawImageSectionToScreen(screenKind, imgSurface, imageRect, _screenRect);
}

void ZoombiniGraphics::drawImage(ScreenKind screenKind, int16 image, const Common::Point &destPos) {
	MohawkSurface *imgSurface = findImage(ZmbResource(ZmbResource::kPage, image));
	Graphics::Surface *rawSurface = imgSurface->getSurface();
	Common::Rect srcRect = rawSurface->getRect();
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

Common::Rect ZoombiniGraphics::submitDrawCommand(ScreenKind screenKind, DrawCommand &drawCmd) {
	const bool retain = _presentationCapture && screenKind == kShapeScreen;
	if (retain && drawCmd._image && drawCmd._image->getSurface()) {
		// Freeze the input before drawing in case the source pixels share storage with the destination.
		Graphics::Surface *copy = new Graphics::Surface();
		copy->copyFrom(*drawCmd._image->getSurface());
		drawCmd._imageCopy.reset(new MohawkSurface(copy));
	}
	const Common::Rect drawnRect = executeDrawCommand(screenKind, drawCmd, Common::Point());
	if (retain) {
		if (drawCmd._kind == DrawCommand::Kind::kText)
			drawCmd._coverage = drawnRect;
		if (drawCmd._imageCopy)
			drawCmd._image = drawCmd._imageCopy.get();
		drawCmd._motionKey = _presentationMotionKey;
		drawCmd._motionLeader = _presentationMotionLeader;
		_presentationCapture->push_back(drawCmd);
	}
	return drawnRect;
}

Common::Rect ZoombiniGraphics::executeDrawCommand(ScreenKind screenKind, const DrawCommand &drawCmd, const Common::Point &offset) {
	Common::Rect bounds = drawCmd._bounds;
	bounds.translate(offset.x, offset.y);
	switch (drawCmd._kind) {
	case DrawCommand::Kind::kImage:
		return blitImageSectionToScreen(screenKind, drawCmd._image, drawCmd._source, bounds, drawCmd._clear, drawCmd._paletteRemap);
	case DrawCommand::Kind::kFill:
		fillRectToScreen(screenKind, bounds, drawCmd._color);
		break;
	case DrawCommand::Kind::kText: {
		TextConf tc = drawCmd._textConf;
		if (tc._hasClipRect)
			tc._clipRect.translate(offset.x, offset.y);
		return drawTextToScreen(screenKind, drawCmd._text, bounds, tc);
	}
	case DrawCommand::Kind::kLine:
	case DrawCommand::Kind::kThickLine:
		drawLineToScreen(screenKind, drawCmd, offset);
		break;
	}
	return bounds;
}

void ZoombiniGraphics::replayDrawCommand(const DrawCommand &drawCmd, const Common::Point &offset) {
	_isPresentationDraw = true;
	executeDrawCommand(kShapeScreen, drawCmd, offset);
	_isPresentationDraw = false;
}

void ZoombiniGraphics::drawLineToScreen(ScreenKind screenKind, const DrawCommand &drawCmd, const Common::Point &offset) {
	if (_isRenderClipActive && !_hasRenderClipRect)
		return;
	Common::Rect bounds = drawCmd._bounds;
	bounds.translate(offset.x, offset.y);
	const Common::Point start = drawCmd._start + offset;
	const Common::Point end = drawCmd._end + offset;
	Graphics::Surface *screen = getScreen(screenKind);
	if (!_isPresentationDraw) {
		recordDirtyRect(screenKind, bounds);
		if (drawCmd._kind == DrawCommand::Kind::kLine)
			screen->drawLine(start.x, start.y, end.x, end.y, drawCmd._color);
		else
			screen->drawThickLine(start.x, start.y, end.x, end.y, drawCmd._penX, drawCmd._penY, drawCmd._color);
		return;
	}
	// Replay clips a local line mask so dirty-region boundaries do not change the line's rasterization.
	bounds.clip(_screenRect);
	if (bounds.isEmpty())
		return;
	Graphics::Surface line;
	line.create(bounds.width(), bounds.height(), Graphics::PixelFormat::createFormatCLUT8());
	line.fillRect(line.getRect(), 0);
	const Common::Point localStart = start - Common::Point(bounds.left, bounds.top);
	const Common::Point localEnd = end - Common::Point(bounds.left, bounds.top);
	if (drawCmd._kind == DrawCommand::Kind::kLine)
		line.drawLine(localStart.x, localStart.y, localEnd.x, localEnd.y, 1);
	else
		line.drawThickLine(localStart.x, localStart.y, localEnd.x, localEnd.y, drawCmd._penX, drawCmd._penY, 1);
	for (const Common::Rect &clip : _renderClipRects) {
		Common::Rect rect = bounds;
		rect.clip(clip);
		for (int y = rect.top; y < rect.bottom; y++) {
			for (int x = rect.left; x < rect.right; x++) {
				if (*static_cast<const byte *>(line.getBasePtr(x - bounds.left, y - bounds.top)))
					screen->fillRect(Common::Rect(x, y, x + 1, y + 1), drawCmd._color);
			}
		}
	}
	recordDirtyRect(screenKind, bounds);
	line.free();
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
	if (!_presentationCapture && (destPos.x <= -1 * surface->w || destPos.y <= -1 * surface->h || kScreenWidth <= destPos.x || kScreenHeight <= destPos.y))
		return Common::Rect();

	Common::Rect srcRect = surface->getRect();
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

	Common::Rect srcRect = surface->getRect();

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
	if (!_presentationCapture || screenKind != kShapeScreen)
		return blitImageSectionToScreen(screenKind, mhkSurface, srcRect, dstRect, clearBeforeRender, remapColorAssistPalette);

	DrawCommand drawCmd;
	drawCmd._image = mhkSurface;
	drawCmd._source = srcRect;
	drawCmd._bounds = dstRect;
	drawCmd._clear = clearBeforeRender;
	drawCmd._paletteRemap = remapColorAssistPalette;
	return submitDrawCommand(screenKind, drawCmd);
}

Common::Rect ZoombiniGraphics::blitImageSectionToScreen(ScreenKind screenKind, MohawkSurface *mhkSurface, const Common::Rect &srcRect, const Common::Rect &dstRect, bool clearBeforeRender, PaletteRemapMode remapColorAssistPalette) {
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

	// The shared @ref Graphics::Surface::clip() cannot be used here.
	// It trims both bounds by the same pixel counts, which assumes equal-sized one-to-one bounds.
	// Callers of this function pass unequal bounds.

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
	assert(source->format == Graphics::PixelFormat::createFormatCLUT8());
	assert(screen->format == source->format);

	const uint32 *remapTable = nullptr;
	if (remapMode == kPaletteRemapNoseNet)
		remapTable = _colorAssistNoseNetRemapTable.data();
	else if (remapMode == kPaletteRemapMazePurple)
		remapTable = _colorAssistMazePurpleRemapTable.data();
	else {
		screen->copyRectToSurfaceWithKey(*source, destX, destY, sourceRect, kTransparentKey);
		return;
	}

	Graphics::crossKeyBlitMap(static_cast<byte *>(screen->getBasePtr(destX, destY)), static_cast<const byte *>(source->getBasePtr(sourceRect.left, sourceRect.top)),
							  screen->pitch, source->pitch, sourceRect.width(), sourceRect.height(), 1, remapTable, kTransparentKey);
}

void ZoombiniGraphics::fillColorAssistPaletteRemapTable(Common::Array<uint32> &paletteMap, PaletteRemapMode remapMode) {
	assert(paletteMap.size() == 256);

	for (uint32 i = 0; i < paletteMap.size(); i += 1)
		paletteMap[i] = i;

	switch (remapMode) {
	case kPaletteRemapNoseNet:
		paletteMap[kColor23_DarkOrange] = kColor0C_DarkGray;
		paletteMap[kColor24_Orange] = kColor0A_White;
		paletteMap[kColor25_LightOrange] = kColor0A_White;
		paletteMap[kColor2B_Yellow] = kColor0A_White;
		paletteMap[kColor18_DarkMagenta] = kColor10_DarkCyan;
		paletteMap[kColor19_Magenta] = kColor12_SkyBlue;
		paletteMap[kColor1A_DarkPink] = kColor13_LightBlue;
		paletteMap[kColor1B_Pink] = kColor14_PastelBlue;
		break;
	case kPaletteRemapMazePurple:
		paletteMap[0xA4] = kColor1E_LightAzure;
		paletteMap[0xAA] = kColor1E_LightAzure;
		paletteMap[0xA5] = kColor1D_Azure;
		paletteMap[0xA6] = kColor1D_Azure;
		paletteMap[0xA8] = kColor1D_Azure;
		paletteMap[0xA7] = kColor1C_DarkAzure;
		paletteMap[0xA9] = kColor1C_DarkAzure;
		break;
	case kPaletteRemapNone:
	default:
		break;
	}
}

void ZoombiniGraphics::drawLine(ScreenKind screenKind, const Common::Point &start, const Common::Point &end, uint32 color) {
	if (!_presentationCapture || screenKind != kShapeScreen) {
		if (_isRenderClipActive && !_hasRenderClipRect)
			return;

		Graphics::Surface *screen = getScreen(screenKind);
		Common::Rect dirtyRect(MIN(start.x, end.x), MIN(start.y, end.y), MAX(start.x, end.x) + 1, MAX(start.y, end.y) + 1);
		recordDirtyRect(screenKind, dirtyRect);
		screen->drawLine(start.x, start.y, end.x, end.y, color);
		return;
	}

	DrawCommand drawCmd;
	drawCmd._kind = DrawCommand::Kind::kLine;
	drawCmd._start = start;
	drawCmd._end = end;
	drawCmd._color = color;
	drawCmd._bounds = Common::Rect(MIN(start.x, end.x), MIN(start.y, end.y), MAX(start.x, end.x) + 1, MAX(start.y, end.y) + 1);
	submitDrawCommand(screenKind, drawCmd);
}

void ZoombiniGraphics::drawThickLine(ScreenKind screenKind, const Common::Point &start, const Common::Point &end, int penX, int penY, uint32 color) {
	if (!_presentationCapture || screenKind != kShapeScreen) {
		if (_isRenderClipActive && !_hasRenderClipRect)
			return;

		Graphics::Surface *screen = getScreen(screenKind);
		Common::Rect dirtyRect(MIN(start.x, end.x) - penX, MIN(start.y, end.y) - penY,
							   MAX(start.x, end.x) + penX + 1, MAX(start.y, end.y) + penY + 1);
		recordDirtyRect(screenKind, dirtyRect);
		screen->drawThickLine(start.x, start.y, end.x, end.y, penX, penY, color);
		return;
	}

	DrawCommand drawCmd;
	drawCmd._kind = DrawCommand::Kind::kThickLine;
	drawCmd._start = start;
	drawCmd._end = end;
	drawCmd._color = color;
	drawCmd._penX = penX;
	drawCmd._penY = penY;
	drawCmd._bounds = Common::Rect(MIN(start.x, end.x) - penX, MIN(start.y, end.y) - penY,
								   MAX(start.x, end.x) + penX + 1, MAX(start.y, end.y) + penY + 1);
	submitDrawCommand(screenKind, drawCmd);
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
	if (!_presentationCapture || screenKind != kShapeScreen) {
		fillRectToScreen(screenKind, rect, color);
		return;
	}

	DrawCommand drawCmd;
	drawCmd._kind = DrawCommand::Kind::kFill;
	drawCmd._bounds = rect;
	drawCmd._color = color;
	submitDrawCommand(screenKind, drawCmd);
}

void ZoombiniGraphics::fillRectToScreen(ScreenKind screenKind, const Common::Rect &rect, uint32 color) {
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
	if (!_presentationCapture || screenKind != kShapeScreen) {
		drawTextToScreen(screenKind, text, destRect, tc);
		return;
	}

	DrawCommand drawCmd;
	drawCmd._kind = DrawCommand::Kind::kText;
	drawCmd._bounds = destRect;
	drawCmd._text = text;
	drawCmd._textConf = tc;
	submitDrawCommand(screenKind, drawCmd);
}

Common::Rect ZoombiniGraphics::drawTextToScreen(ScreenKind screenKind, const Common::U32String &text, const Common::Rect &destRect, const TextConf &tc) {
	const Graphics::Font *font = _vm->_text->getFont(tc._fontUsage);
	if (!font) {
		error("gfx: cannot open fontfile of kind %u", static_cast<uint32>(tc._fontUsage));
		return Common::Rect();
	}
	const bool useAA = _vm->_text->fontUsesAntialiasing(tc._fontUsage);
	TextLayoutPolicy policy;
	switch (_vm->getRelInfo().getPlatform()) {
	case Common::kPlatformMacintosh:
		policy = TextLayoutPolicy::kQuickDraw;
		break;
	case Common::kPlatformWindows:
	default:
		policy = TextLayoutPolicy::kGdi;
		break;
	}

	// Lay out against the full caller rectangle before applying the optional pixel clip.
	// Shrinking it to the visible area would change line breaks and alignment during scrolling.
	Common::Array<Common::U32String> lines = prepareTextLines(text, font, tc._wordWrap, destRect.width(), policy, _vm->getLanguage());

	// Align all passes from the unexpanded text bounds.
	// Offset only the four outline passes so the main text does not move.
	const Common::Point boundSize = getTextLinesBounds(font, false, lines);

	// Retain the caller's layout width for wrapped text; the optional clip only limits output after positioning.
	Common::Rect drawRect = destRect;
	if (!tc._wordWrap)
		expandUnwrappedTextRect(drawRect, boundSize.x, tc._hAlign);

	// layoutClip: preserves the layout bounds before applying any adjustments.
	// outputClip: intersects those bounds with the fixed caller clip for the main text, its dirty bounds and the returned bounds.
	const Common::Rect layoutClip = drawRect;
	Common::Rect outputClip = layoutClip;
	if (tc._hasClipRect)
		outputClip.clip(tc._clipRect);

	// Handle background fill
	uint32 fillBgPalette = kTransparentKey;
	if (tc._fillBackground)
		fillBgPalette = tc._fillBackgroundKey;

	drawRect = positionTextLines(lines, font, drawRect, tc._vAlign, policy);

	// Draw the four one-pixel outline passes before the main text.
	// Move each pass's layout clip with its offset; the caller's additional pixel clip stays fixed.
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

			// outlineClip moves layoutClip with this outline pass, then intersects the same fixed caller clip.
			// Translating outputClip instead would also move its already-clipped edges and lose valid outline pixels.
			Common::Rect outlineClip = layoutClip;
			outlineClip.translate(xDelta, yDelta);
			if (tc._hasClipRect)
				outlineClip.clip(tc._clipRect);

			Common::Rect dirtyRect = outlineRect;
			dirtyRect.clip(outlineClip);
			recordDirtyRect(screenKind, dirtyRect);
			drawTextLines(screenKind, font, lines, outlineRect, tc._outlinePalette, tc._hAlign, policy, useAA, fillBgPalette, &outlineClip);
		}
	}

	Common::Rect dirtyRect = drawRect;
	dirtyRect.clip(outputClip);
	recordDirtyRect(screenKind, dirtyRect);
	drawTextLines(screenKind, font, lines, drawRect, tc._textPalette, tc._hAlign, policy, useAA, fillBgPalette, &outputClip);
	if (tc._outlineEffect)
		drawRect.grow(1);
	drawRect.clip(outputClip);
	return drawRect;
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

ZoombiniGraphics::MacLineBreakClass ZoombiniGraphics::getMacLineBreakClass(uint32 chr, Common::Language lang) {
	// 'itl2' is the four-character type of Classic Mac OS international string-manipulation resources.
	// They contain script/region-specific sorting, word-selection and line-break rules.
	// These character classes reproduce their line-break tables, used by StyledLineBreak.
	// Mac OS 9.2.2 resource IDs: English/German 0/3, French/Italian 1/4, Spanish/Portuguese 8/10,
	// Polish (MacCentralEurope) 30762, Japanese 16384, Korean 17408.
	// FIXME: Move Classic MacOS StyledLineBreak helpers to the ScummVM core?
	if (chr == 0 || chr == '\t' || chr == ' ')
		return kMacLineBreakClass02_Blank;
	if (chr == '\r')
		return kMacLineBreakClass03_CarriageReturn;

	switch (lang) {
	case Common::JA_JPN: {
		static const Common::U32String operators(
			U"&*+-/<=>^|±×÷∈∋∝∩∪∽≒≠≡≦≧≪"
			U"≫⊂⊃⊆⊇＆＋－＜＝＞＾");
		static const Common::U32String opening(
			U"([{‘“〈《「『【〔（［｛｢");
		static const Common::U32String closing(
			U"!),.:;?]}，．");
		static const Common::U32String nonstarters(
			U"‐―’”、。〃々〉》」』】〕ぁぃ"
			U"ぅぇぉっゃゅょゎ゛゜ゝゞァィゥェ"
			U"ォッャュョヮ・ーヽヾ仝！）：；？"
			U"］｝｡｣､･ｧｨｩｪｫｬｭｮｯｰ"
			U"ﾞﾟ");
		static const Common::U32String wordSymbols(
			U"№℡ⅠⅡⅢⅣⅤ∑∟∮⊿⑯⑰⑱⑲⑳"
			U"〟㈱㈲㈹㊤㊥㊦㊧㊨㌃㌍㌔㌘㌢㌣㌦"
			U"㌧㌫㌶㌻㍊㍍㍑㍗㍼㍽㍾㎎㎏㎜㎝㎞"
			U"㎡㏄㏍＄￡￥");

		if (chr == 0x3000) // CJK Symbols and Punctuation: IDEOGRAPHIC SPACE.
			return kMacLineBreakClass02_Blank;
		if (operators.contains(chr))
			return kMacLineBreakClass05_Operator;
		if (opening.contains(chr))
			return kMacLineBreakClass06_Opening;
		if (closing.contains(chr))
			return kMacLineBreakClass07_Closing;
		if (nonstarters.contains(chr))
			return kMacLineBreakClass08_JaNonstarter;
		// Fullwidth Latin letters/digits are words; Greek, Cyrillic and ordinary kana/kanji use class 9.
		if (chr <= 0x0080 ||                    // Remaining ASCII and CP932 U+0080; special classes were handled above.
			(0xFF10 <= chr && chr <= 0xFF19) || // Fullwidth digits 0..9.
			(0xFF21 <= chr && chr <= 0xFF3A) || // Fullwidth uppercase Latin letters A..Z.
			(0xFF41 <= chr && chr <= 0xFF5A) || // Fullwidth lowercase Latin letters a..z.
			(0xF8F0 <= chr && chr <= 0xF8F3) || // CP932 private-use mappings for bytes 0xA0 and 0xFD..0xFF.
			wordSymbols.contains(chr))          // Other native word-class symbols listed above.
			return kMacLineBreakClass04_Word;
		return kMacLineBreakClass09_Ideographic;
	}
	case Common::KO_KOR: {
		// The native Korean operator class includes the fullwidth right brace.
		static const Common::U32String operators(
			U"&*+-/<=>^|±×÷–—⁺⁻⁼∈∉∋∌∓∝∧∨∩∪∽≃≅≈≑≒≓≠≡≢≤≥"
			U"≦≧≪≫≮≯≰≱≲≳≶≷≹≺≻⊀⊁⊂⊃⊄⊅⊆⊇⋎⋏⋚⋛⪋⪌⪑⪒⫅⫆⫋⫌〓〜＆＋－"
			U"＜＝＞＾｝");
		static const Common::U32String opening(
			U"([{‘‛“⁽⦅⦗〈《「『【〔〖〘﹙（［｛");
		static const Common::U32String closing(
			U"!),.:;?]}ʼ’”⁾⦆⦘〉》」』】〕〗〙﹚！），．：；？］");
		static const Common::U32String wordSymbols(
			U"£¥©₩＄％￦");

		if (chr == '\n') // Basic Latin: LINE FEED, classified with CR by the original Korean table.
			return kMacLineBreakClass03_CarriageReturn;
		if (chr == 0x3000) // CJK Symbols and Punctuation: IDEOGRAPHIC SPACE.
			return kMacLineBreakClass02_Blank;
		if (operators.contains(chr))
			return kMacLineBreakClass05_Operator;
		if (opening.contains(chr))
			return kMacLineBreakClass06_Opening;
		if (closing.contains(chr))
			return kMacLineBreakClass07_Closing;
		if (chr < 0x0080 ||                     // Remaining Basic Latin (ASCII); special classes were handled above.
			(0x0085 <= chr && chr <= 0x009F) || // Latin-1 Supplement: C1 control codes U+0085..009F.
			chr == 0x00A0 ||                    // Latin-1 Supplement: NO-BREAK SPACE, a word character in the Korean table.
			(0xFF10 <= chr && chr <= 0xFF19) || // Fullwidth digits 0..9.
			(0xFF21 <= chr && chr <= 0xFF3A) || // Fullwidth uppercase Latin letters A..Z.
			(0xFF41 <= chr && chr <= 0xFF5A) || // Fullwidth lowercase Latin letters a..z.
			wordSymbols.contains(chr))          // Other word-class symbols listed above.
			return kMacLineBreakClass04_Word;
		return kMacLineBreakClass08_KoText;
	}
	case Common::EN_ANY:
	case Common::EN_USA:
	case Common::EN_GRB:
	case Common::DE_DEU: {
		static const Common::U32String operators(
			U"&*+-/<=>\\^|±÷–—≈≠≤≥");
		// English and German use the acquired Roman 0/3 line-break classes.
		if (chr == 0x00A0) // Latin-1 Supplement: NO-BREAK SPACE.
			return kMacLineBreakClass01_Nonbreaking;
		if (operators.contains(chr))
			return kMacLineBreakClass05_Operator;
		return kMacLineBreakClass04_Word;
	}
	case Common::ES_ESP:
	case Common::PT_PRT: {
		// Spanish/Portuguese itl2 IDs 8/10 share their own operator set; '+' and '-' remain word characters.
		if (chr == 0x00A0) // Latin-1 Supplement: NO-BREAK SPACE.
			return kMacLineBreakClass01_Nonbreaking;
		static const Common::U32String operators(
			U"#&*/<=>@\\^_|~ß≠≤≥≈«»");
		if (operators.contains(chr))
			return kMacLineBreakClass05_Operator;
		return kMacLineBreakClass04_Word;
	}
	case Common::PL_POL: {
		// Polish itl2 ID 30762 uses MacCentralEurope, whose native operator set omits Roman-only symbols.
		if (chr == 0x00A0) // Latin-1 Supplement: NO-BREAK SPACE.
			return kMacLineBreakClass01_Nonbreaking;
		static const Common::U32String operators(
			U"&*+-/<=>\\^|≠≤≥–—÷");
		if (operators.contains(chr))
			return kMacLineBreakClass05_Operator;
		return kMacLineBreakClass04_Word;
	}
	case Common::FR_FRA:
	case Common::IT_ITA:
		// French/Italian itl2 IDs 1/4 put all mapped characters except blanks, CR and NBSP in class 4 (word).
		// This includes '+', '-' and '/', which have class 5 (operator) in the US/German tables.
		// They therefore create no internal word-break opportunity in an unspaced span such as "AB+CD".
		// styledLineBreak() can still split the span at a character boundary if it cannot fit an empty line.
		if (chr == 0x00A0) // Latin-1 Supplement: NO-BREAK SPACE.
			return kMacLineBreakClass01_Nonbreaking;
		return kMacLineBreakClass04_Word;
	default:
		error("ZoombiniGraphics::getMacLineBreakClass: unsupported language %d", static_cast<int>(lang));
	}
}

uint32 ZoombiniGraphics::getMacCharByteSize(uint32 chr, Common::Language lang) {
	// FIXME: Once Common::kMacJapanese/kMacKorean are merged, replace these per-scalar estimates
	// with encoder-derived native byte offsets. Migrate text decoding and classification together,
	// including CP932-specific mappings. Preserve complete Apple multi-code-point sequences and
	// check encoding errors so replacement bytes are not counted as original character widths.
	switch (lang) {
	case Common::JA_JPN: // MacJapanese
		// These Unicode BMP subsets count as one byte in the legacy Japanese encoding used here.
		if (chr <= 0x0080 ||                    // Basic Latin (U+0000..007F) plus U+0080 in Latin-1 Supplement.
			(0xFF61 <= chr && chr <= 0xFF9F) || // Halfwidth and Fullwidth Forms: halfwidth CJK punctuation, Katakana and voicing marks.
			(0xF8F0 <= chr && chr <= 0xF8F3))   // Private Use Area (U+E000..F8FF): CP932 byte mappings A0 and FD..FF.
			return 1;
		return 2;
	case Common::KO_KOR: // MacKorean
		// MacKorean has single-byte NBSP, copyright and won signs in addition to ASCII and C1 controls.
		if (chr < 0x0080 ||                     // Basic Latin: ASCII characters and controls.
			(0x0085 <= chr && chr <= 0x009F) || // Latin-1 Supplement: C1 control codes U+0085..009F.
			chr == 0x00A0 ||                    // Latin-1 Supplement: NO-BREAK SPACE.
			chr == 0x00A9 ||                    // Latin-1 Supplement: COPYRIGHT SIGN.
			chr == 0x20A9)                      // Currency Symbols: WON SIGN.
			return 1;
		return 2;
	case Common::EN_ANY:
	case Common::EN_USA:
	case Common::EN_GRB:
	case Common::DE_DEU:
	case Common::FR_FRA:
	case Common::ES_ESP:
	case Common::PT_PRT:
	case Common::IT_ITA:
	case Common::PL_POL:
		return 1;
	default:
		error("ZoombiniGraphics::getMacCharByteSize: unsupported language %d", static_cast<int>(lang));
	}
}

void ZoombiniGraphics::findMacWordBreaks(const Common::U32String &text, uint32 lineStart, uint32 fitEnd, Common::Language lang, uint32 &wordStart, uint32 &wordEnd) {
	// Transition tables used by the original Classic Mac OS FindWordBreaks routine.
	// Fixed-width rows replace the original NBreakTable's row-offset header.
	// Each action marks the current character start with bit 7 and selects an even state number in bits 0..6.
	// State zero stops at the last mark; state 2 starts a word. Class zero supplies the end-of-text sentinel.
	static constexpr byte romanBackward[7][6] = {
		{0, 0, 0, 0, 0, 0},
		{136, 138, 132, 132, 134, 134},
		{0, 4, 132, 128, 0, 0},
		{0, 140, 0, 0, 134, 0},
		{0, 140, 0, 0, 0, 0},
		{136, 138, 132, 0, 134, 134},
		{136, 140, 0, 0, 134, 134},
	};
	static constexpr byte jaBackward[8][10] = {
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{136, 130, 132, 132, 134, 134, 138, 140, 140, 142},
		{0, 130, 132, 0, 0, 0, 0, 0, 0, 0},
		{0, 130, 0, 0, 134, 0, 138, 6, 0, 0},
		{0, 130, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 130, 0, 0, 0, 0, 138, 0, 0, 0},
		{0, 130, 0, 0, 134, 134, 138, 140, 140, 142},
		{0, 130, 0, 0, 0, 0, 138, 0, 0, 0},
	};
	static constexpr byte koBackward[8][9] = {
		{0, 0, 0, 0, 0, 0, 0, 0, 0},
		{136, 130, 132, 132, 134, 134, 138, 140, 142},
		{0, 130, 132, 0, 0, 0, 0, 0, 0},
		{0, 130, 0, 0, 134, 0, 138, 6, 0},
		{0, 130, 0, 0, 0, 0, 0, 0, 0},
		{0, 130, 0, 0, 0, 0, 138, 0, 0},
		{0, 130, 0, 0, 134, 134, 138, 140, 142},
		{0, 130, 0, 0, 0, 0, 138, 0, 0},
	};
	static constexpr byte romanTransitions[8][6] = {
		{0, 0, 0, 0, 0, 0},
		{136, 130, 132, 140, 134, 136},
		{128, 142, 132, 140, 128, 128},
		{128, 138, 128, 128, 134, 136},
		{128, 138, 128, 128, 128, 128},
		{136, 138, 128, 128, 134, 136},
		{128, 128, 128, 128, 128, 128},
		{0, 14, 132, 140, 0, 0},
	};
	static constexpr byte jaTransitions[9][10] = {
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{136, 130, 132, 136, 134, 136, 138, 140, 142, 144},
		{128, 130, 132, 136, 128, 128, 128, 128, 128, 128},
		{128, 130, 128, 128, 134, 136, 138, 140, 142, 128},
		{128, 130, 128, 128, 128, 128, 128, 140, 142, 128},
		{128, 130, 128, 128, 134, 136, 138, 140, 142, 144},
		{128, 130, 128, 128, 134, 136, 138, 140, 142, 128},
		{128, 130, 128, 128, 128, 128, 128, 142, 142, 128},
		{128, 130, 128, 128, 128, 136, 128, 142, 142, 128},
	};

	static constexpr byte koTransitions[8][9] = {
		{0, 0, 0, 0, 0, 0, 0, 0, 0},
		{136, 130, 132, 136, 134, 136, 138, 140, 142},
		{128, 130, 132, 136, 128, 128, 128, 128, 128},
		{128, 130, 128, 128, 134, 136, 138, 140, 128},
		{128, 130, 128, 128, 128, 128, 128, 140, 128},
		{128, 130, 128, 128, 134, 136, 138, 140, 142},
		{128, 130, 128, 128, 134, 136, 138, 140, 128},
		{128, 130, 128, 128, 128, 136, 128, 142, 128},
	};

	// The original Mac OS routine uses NBreakTable's doBackup field to enable a backward DFA scan at this encoded-byte offset from lineStart.
	// Below it, scan forward from lineStart; otherwise backtrack from fitEnd to find the forward scan's restart point.
	// It is a trigger threshold, not a backward-scan length limit: 6 bytes for Roman/Polish, 10 for Japanese/Korean.
	uint32 backupThreshold = 6;
	switch (lang) {
	case Common::JA_JPN:
	case Common::KO_KOR:
		backupThreshold = 10;
		break;
	case Common::EN_ANY:
	case Common::EN_USA:
	case Common::EN_GRB:
	case Common::DE_DEU:
	case Common::FR_FRA:
	case Common::ES_ESP:
	case Common::PT_PRT:
	case Common::IT_ITA:
	case Common::PL_POL:
		break;
	default:
		error("ZoombiniGraphics::findMacWordBreaks: unsupported language %d", static_cast<int>(lang));
	}

	// Count the fitting prefix in bytes of the original Mac OS script encoding; our text and span indices remain UTF-32.
	uint32 nativeFitOffset = 0;
	for (uint32 offset = lineStart; offset < fitEnd && nativeFitOffset < backupThreshold; offset++)
		nativeFitOffset += getMacCharByteSize(text[offset], lang);
	wordStart = lineStart;
	if (backupThreshold <= nativeFitOffset) {
		byte state = 2;
		uint32 offset = fitEnd;
		while (lineStart < offset) {
			MacLineBreakClass charClass = getMacLineBreakClass(text[offset], lang);
			byte action;
			switch (lang) {
			case Common::JA_JPN:
				action = jaBackward[state / 2][charClass];
				break;
			case Common::KO_KOR:
				action = koBackward[state / 2][charClass];
				break;
			case Common::EN_ANY:
			case Common::EN_USA:
			case Common::EN_GRB:
			case Common::DE_DEU:
			case Common::FR_FRA:
			case Common::ES_ESP:
			case Common::PT_PRT:
			case Common::IT_ITA:
			case Common::PL_POL:
				action = romanBackward[state / 2][charClass];
				break;
			default:
				error("ZoombiniGraphics::findMacWordBreaks: unsupported language %d", static_cast<int>(lang));
			}
			if (action & 0x80)
				wordStart = offset;
			state = action & 0x7F;
			if (!state)
				break;
			offset -= 1;
		}
		if (offset == lineStart)
			wordStart = lineStart;
	}

	// Scan forward from the backward table's restart point until the selected character is inside a span.
	// Keep both scans: script-specific backward restarts can differ from tokenizing the paragraph from its beginning.
	do {
		wordEnd = wordStart;
		byte state = 2;
		for (uint32 offset = wordStart; offset <= text.size(); offset++) {
			MacLineBreakClass charClass = kMacLineBreakClass00_End;
			if (offset < text.size())
				charClass = getMacLineBreakClass(text[offset], lang);
			byte action;
			switch (lang) {
			case Common::JA_JPN:
				action = jaTransitions[state / 2][charClass];
				break;
			case Common::KO_KOR:
				action = koTransitions[state / 2][charClass];
				break;
			case Common::EN_ANY:
			case Common::EN_USA:
			case Common::EN_GRB:
			case Common::DE_DEU:
			case Common::FR_FRA:
			case Common::ES_ESP:
			case Common::PT_PRT:
			case Common::IT_ITA:
			case Common::PL_POL:
				action = romanTransitions[state / 2][charClass];
				break;
			default:
				error("ZoombiniGraphics::findMacWordBreaks: unsupported language %d", static_cast<int>(lang));
			}
			if (action & 0x80)
				wordEnd = offset;
			state = action & 0x7F;
			if (!state)
				break;
		}
		assert(wordStart < wordEnd);
		if (fitEnd < wordEnd)
			return;
		wordStart = wordEnd;
	} while (wordStart < text.size());
}

uint32 ZoombiniGraphics::styledLineBreak(const Common::U32String &text, uint32 lineStart, const Graphics::Font *font, int targetWidth, Common::Language lang) {
	assert(lineStart < text.size());
	// PixelToChar supplies a fitting whole-character offset to the native routine.
	// Use the selected ScummVM font's advances here; this does not synthesize unavailable QuickDraw font metrics.
	if (targetWidth < 0)
		return lineStart + 1;
	uint32 fitEnd = lineStart;
	uint32 prev = 0;
	int lineWidth = 0;
	while (fitEnd < text.size()) {
		const uint32 chr = text[fitEnd];
		const int charWidth = font->getCharWidth(chr) + font->getKerningOffset(prev, chr);
		if (targetWidth < lineWidth + charWidth)
			break;
		lineWidth += charWidth;
		prev = chr;
		fitEnd += 1;
	}
	if (fitEnd == text.size())
		return fitEnd;

	// FindWordBreaks selects the span containing the fitting edge, with script-specific backward and forward scans.
	uint32 wordStart, wordEnd;
	findMacWordBreaks(text, lineStart, fitEnd, lang, wordStart, wordEnd);

	// StyledLineBreak keeps the entire blank span on the preceding line, even past the fitting width.
	// Otherwise move the whole word to the next line; split characters only when the first word cannot fit.
	if (getMacLineBreakClass(text[wordStart], lang) == kMacLineBreakClass02_Blank)
		return wordEnd;
	if (wordStart == lineStart)
		return MAX(fitEnd, lineStart + 1);
	return wordStart;
}

Common::Array<Common::U32String> ZoombiniGraphics::prepareTextLines(const Common::U32String &text, const Graphics::Font *font, bool wordWrap, int16 targetWidth, TextLayoutPolicy policy, Common::Language lang) {
	Common::U32String normalizedText = text;
	switch (policy) {
	case TextLayoutPolicy::kGdi:
		break;
	case TextLayoutPolicy::kQuickDraw:
		// Normalize TAB char to one Space char, following QuickDraw/TextEdit convention.
		// tokenizeLines() will handle CR/LF/CRLF to CR normalization.
		normalizedText.replace(U'\t', U' ');
		break;
	}

	if (!wordWrap)
		return ZoombiniText::tokenizeLines(normalizedText);

	if (policy == TextLayoutPolicy::kQuickDraw) {
		Common::Array<Common::U32String> lines;
		const Common::Array<Common::U32String> paragraphs = ZoombiniText::tokenizeLines(normalizedText);
		for (const Common::U32String &paragraph : paragraphs) {
			if (paragraph.empty()) {
				lines.push_back(paragraph);
				continue;
			}
			uint32 lineStart = 0;
			while (lineStart < paragraph.size()) {
				const uint32 lineEnd = styledLineBreak(paragraph, lineStart, font, targetWidth, lang);
				lines.push_back(paragraph.substr(lineStart, lineEnd - lineStart));
				lineStart = lineEnd;
			}
		}
		return lines;
	}

	// GDI DrawTextA with DT_WORDBREAK accepts a line whose extent exactly equals the caller rectangle width.
	// @ref Graphics::Font::wordWrapText() treats that equality as overflow because its width test uses >=.
	// Add one pixel only for the line-break decision; drawing still uses the caller's rectangle and clipping.
	const int wordWrapWidth = targetWidth + 1;
	// Wrap before splitting explicit breaks: an overflowing space followed by CR consumes a blank line in GDI.
	// Splitting first would discard that line (for example, the Portuguese RodMap's fourth route name).
	Common::Array<Common::U32String> lines;
	font->wordWrapText(normalizedText, wordWrapWidth, lines);
	return lines;
}

Common::Rect ZoombiniGraphics::positionTextLines(Common::Array<Common::U32String> &lines, const Graphics::Font *font,
												 const Common::Rect &destRect, Graphics::TextAlign vAlign, TextLayoutPolicy policy) {
	// Start from the caller's layout bounds; empty text or an empty rectangle needs no positioning.
	Common::Rect drawRect = destRect;
	if (lines.empty() || destRect.isEmpty())
		return drawRect;

	// Measure the full text block and initially select every line, starting at index zero.
	const int lineHeight = font->getFontHeight();
	assert(0 < lineHeight);
	int textHeight = lineHeight * lines.size();
	int firstLine = 0;

	// [*] Handle visible line counts
	int visibleLines = lines.size();
	if (policy == TextLayoutPolicy::kQuickDraw) {
		// Follow the original Mac engine's calculations.
		visibleLines = MIN<int>(visibleLines, destRect.height() / lineHeight + 1);
		textHeight = visibleLines * lineHeight;
	}

	// [*] Handle vertical alignment.
	// Position the full GDI block or visible Mac subset; Mac center/bottom alignment also chooses firstLine.
	switch (vAlign) {
	case Graphics::kTextAlignStart:
	case Graphics::kTextAlignLeft:
		break;
	case Graphics::kTextAlignCenter:
		switch (policy) {
		case TextLayoutPolicy::kGdi:
			// GDI halves the two heights separately; QuickDraw halves their combined coordinates.
			drawRect.top += destRect.height() / 2 - textHeight / 2;
			break;
		case TextLayoutPolicy::kQuickDraw:
			firstLine = (lines.size() - visibleLines) / 2;
			drawRect.top = (destRect.top + destRect.bottom - textHeight) / 2;
			break;
		}
		break;
	case Graphics::kTextAlignEnd:
	case Graphics::kTextAlignRight:
		drawRect.top = destRect.bottom - textHeight;
		if (policy == TextLayoutPolicy::kQuickDraw)
			firstLine = lines.size() - visibleLines;
		break;
	default:
		error("gfx: invalid vertical alignment %d", vAlign);
	}

	// [*] Calculate top offset of a line, following TECalText convention.
	if (policy == TextLayoutPolicy::kQuickDraw) {
		// Retain [firstLine, firstLine + visibleLines) so drawing begins at the selected line and skips the rest.
		Common::Array<Common::U32String> visibleText;
		for (int i = 0; i < visibleLines; i++)
			visibleText.push_back(lines[firstLine + i]);
		lines = visibleText;

		// Original Mac engine use calculation of: blockTop + lineHeight - descent (when the both metrics are available).
		// FIXME: The ScummVM common TTF renderer exposes neither descent nor leading. Until it does, retain its glyph origin.
		// Exact baseline and native bitmap-strike metrics cannot be reproduced here yet.
		const int descent = font->getFontDescent();
		const int ascent = font->getFontAscent();
		if (0 <= descent && 0 <= ascent)
			drawRect.top += lineHeight - descent - ascent;
	}

	// Keep space below the last baseline for glyph outside its reported line cell;
	// outputClip still bounds the pixels.
	drawRect.bottom = MAX<int>(destRect.bottom, drawRect.top + textHeight);
	return drawRect;
}

int ZoombiniGraphics::getTextLineLeft(const Graphics::Font *font, const Common::U32String &line, const Common::Rect &destRect, Graphics::TextAlign hAlign, TextLayoutPolicy policy) {
	const int textWidth = font->getStringWidth(line);
	switch (hAlign) {
	case Graphics::kTextAlignStart:
	case Graphics::kTextAlignLeft:
		return destRect.left;
	case Graphics::kTextAlignCenter:
		switch (policy) {
		case TextLayoutPolicy::kGdi:
			return destRect.left + (destRect.width() - textWidth) / 2;
		case TextLayoutPolicy::kQuickDraw: {
			int minLeft = destRect.left;
			if (textWidth != destRect.width())
				minLeft += 1;
			return MAX(minLeft, (destRect.left + destRect.right - textWidth) / 2);
		}
		default:
			break;
		}
		break;
	case Graphics::kTextAlignEnd:
	case Graphics::kTextAlignRight:
		return destRect.right - textWidth;
	default:
		error("gfx: invalid horizontal alignment %d", hAlign);
	}
	return destRect.left;
}

Common::Point ZoombiniGraphics::getTextLinesBounds(const Graphics::Font *font, bool outlineEffect, const Common::Array<Common::U32String> &lines) {
	// Retain the font's line advance, including GDI's external leading.
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
	Common::Rect sourceRect = copyRect;
	sourceRect.translate(-destRect.left, -destRect.top);
	screen->copyRectToSurfaceWithKey(*textSurface, copyRect.left, copyRect.top, sourceRect, kTransparentKey);
}

#ifdef USE_RGB_COLOR
void ZoombiniGraphics::quantizeHintedTextPixels(Graphics::Surface *textSurface, Graphics::Surface *screen, const Common::Rect &destRect, const Common::Rect &copyRect, const byte *paletteBytes, uint paletteColorCount, uint32 color) {
	// textSurface has 4bpp pixels of hinted text drawing.
	// Those pixels will be quantized to a CLUT8 target surface: screen.
	assert(0 < paletteColorCount && paletteColorCount <= Graphics::PALETTE_COUNT);
	assert(color < paletteColorCount);
	assert(textSurface->format.bytesPerPixel == 4);
	assert(screen->format.bytesPerPixel == 1);

	// Keep this lookup as a member so its nearest-color cache survives repeated draws with the same palette.
	_textPaletteLookup.setPalette(paletteBytes, paletteColorCount);
	const byte textRed = paletteBytes[color * 3 + 0];
	const byte textGreen = paletteBytes[color * 3 + 1];
	const byte textBlue = paletteBytes[color * 3 + 2];
	const int localLeft = copyRect.left - destRect.left;
	const int localTop = copyRect.top - destRect.top;

	for (int rowIdx = 0; rowIdx < copyRect.height(); rowIdx++) {
		const uint32 *src = static_cast<const uint32 *>(textSurface->getBasePtr(localLeft, localTop + rowIdx));
		byte *dst = static_cast<byte *>(screen->getBasePtr(copyRect.left, copyRect.top + rowIdx));
		for (int colIdx = 0; colIdx < copyRect.width(); colIdx++) {
			const byte alpha = static_cast<byte>((src[colIdx] >> textSurface->format.aShift) & 0xFF);
			// Alpha is glyph coverage: zero preserves the destination, while full coverage uses the exact text palette index.
			if (alpha == 0)
				continue;
			if (alpha == 0xFF) {
				dst[colIdx] = static_cast<byte>(color);
				continue;
			}

			// Composite partial coverage in RGB space, then map the result to the nearest active CLUT8 palette entry.
			const uint32 invAlpha = 0xFF - alpha;
			const byte bgPalette = dst[colIdx];
			assert(bgPalette < paletteColorCount);
			const byte bgRed = paletteBytes[bgPalette * 3 + 0];
			const byte bgGreen = paletteBytes[bgPalette * 3 + 1];
			const byte bgBlue = paletteBytes[bgPalette * 3 + 2];
			const byte quantedRed = static_cast<byte>((textRed * alpha + bgRed * invAlpha + 0x7F) / 0xFF);
			const byte quantedGreen = static_cast<byte>((textGreen * alpha + bgGreen * invAlpha + 0x7F) / 0xFF);
			const byte quantedBlue = static_cast<byte>((textBlue * alpha + bgBlue * invAlpha + 0x7F) / 0xFF);
			dst[colIdx] = _textPaletteLookup.findBestColor(quantedRed, quantedGreen, quantedBlue);
		}
	}
}
#endif

void ZoombiniGraphics::drawTextLines(ScreenKind screenKind, const Graphics::Font *font, const Common::Array<Common::U32String> &lines,
									 const Common::Rect &destRect, uint32 color, Graphics::TextAlign hAlign, TextLayoutPolicy policy, bool useAA,
									 uint32 fillBackgroundColor, const Common::Rect *clipRect) {
	// The destination already contains the layout width and computed vertical position.
	if (destRect.isEmpty())
		return;

	Graphics::Surface *screen = _vm->_gfx->getScreen(screenKind);
	assert(screen->format.bytesPerPixel == 1);
#ifdef USE_RGB_COLOR
	if (useAA)
		assert(color < 256);
#endif

	// Rasterize at the full layout width, preserving horizontal alignment on the local surface.
	// The separate pixel clip is applied when writing to the screen, including partially visible glyphs.
	Graphics::Surface textSurface;
	do {
#ifdef USE_RGB_COLOR
		if (useAA) {
			// Text hinting on CLUT surfaces are achieved by these pipeline:
			// - Render texts on a temporary BGRA32 surface, with hinting.
			// - Quantize a temp BGRA32 surface to a target CLUT surface with its palettes.
			// So, it requires a build with 16bit color support.
			textSurface.create(destRect.width(), destRect.height(), Graphics::PixelFormat::createFormatBGRA32());
			textSurface.fillRect(textSurface.getRect(), 0);
			break;
		}
#endif
		textSurface.create(destRect.width(), destRect.height(), screen->format);
		textSurface.fillRect(textSurface.getRect(), kTransparentKey);
	} while (false);

	// GDI and QuickDraw both has same line advancement policy: advance by the font height when selecting and positioning the lines.
	// - Matches  GDI DrawTextA() with DT_EXTERNALLEADING.
	const int lineHeight = font->getFontHeight();
	Common::Rect drawRect = destRect;

	for (uint32 i = 0; i < lines.size(); i++) {
		const Common::U32String &line = lines[i];

		// Clip: skip lines whose top is below the dest rect bottom, matching GDI IntersectClipRect().
		if (destRect.bottom <= drawRect.top)
			break;

		const int localTop = drawRect.top - destRect.top;
		const int localLeft = getTextLineLeft(font, line, destRect, hAlign, policy) - destRect.left;
		const int localWidth = destRect.width() - localLeft;

		// Background feature is for debug purposes, Zoombini gameplay itself does not use this feature.
		if (fillBackgroundColor != kTransparentKey) {
			const Common::Rect bbox = font->getBoundingBox(line, localLeft, localTop, localWidth, Graphics::kTextAlignLeft);
			do {
#ifdef USE_RGB_COLOR
				// @ref ZoombiniGraphics::quantizeHintedTextPixels treats source alpha as glyph coverage and blends against the screen.
				// Fill the screen first so antialiased edges blend with this background; an opaque surface fill would become the text color.
				if (useAA) {
					Common::Rect screenBBox = bbox;
					screenBBox.translate(destRect.left, destRect.top);
					screenBBox.clip(destRect);
					screenBBox.clip(_screenRect);
					if (clipRect)
						screenBBox.clip(*clipRect);
					fillRectToScreen(screenKind, screenBBox, fillBackgroundColor);
					break;
				}
#endif
				textSurface.fillRect(bbox, fillBackgroundColor);
			} while (false);
		}

		do {
#ifdef USE_RGB_COLOR
			if (useAA) {
				const byte textRed = _paletteBytes[color * 3 + 0];
				const byte textGreen = _paletteBytes[color * 3 + 1];
				const byte textBlue = _paletteBytes[color * 3 + 2];
				const uint32 textColor = textSurface.format.ARGBToColor(0xFF, textRed, textGreen, textBlue);
				font->drawAlphaString(&textSurface, line, localLeft, localTop, localWidth, textColor, Graphics::kTextAlignLeft, 0, false, true);
				break;
			}
#endif
			font->drawString(&textSurface, line, localLeft, localTop, localWidth, color, Graphics::kTextAlignLeft, 0, false, true);
		} while (false);

		if (i + 1 < lines.size()) {
			drawRect.top += lineHeight;
			drawRect.bottom += lineHeight;
		}
	}

	// Trim the output area after layout and rasterization; keep the surface origin and text position unchanged.
	Common::Rect screenRect = destRect;
	screenRect.clip(_screenRect);
	if (clipRect)
		screenRect.clip(*clipRect);
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
					do {
#ifdef USE_RGB_COLOR
						if (useAA) {
							quantizeHintedTextPixels(&textSurface, screen, destRect, clipped, _paletteBytes, ARRAYSIZE(_paletteBytes) / 3, color);
							break;
						}
#endif
						copyTextPixels(&textSurface, screen, destRect, clipped);
					} while (false);
				}
			}
		}
	} else {
		do {
#ifdef USE_RGB_COLOR
			if (useAA) {
				quantizeHintedTextPixels(&textSurface, screen, destRect, screenRect, _paletteBytes, ARRAYSIZE(_paletteBytes) / 3, color);
				break;
			}
#endif
			copyTextPixels(&textSurface, screen, destRect, screenRect);
		} while (false);
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
		currentTime = _vm->getTotalPlayTime();
		fe._startTime = currentTime;
	}
	// Count fade steps with integer arithmetic: one step per 1/60 second, matching the animation clock.
	const uint32 steps = static_cast<uint32>((static_cast<uint64>(fe._duration) * MohawkEngine_Zoombini::kAnimateFrameRate) / MohawkEngine_Zoombini::kAnimationClockTimeBaseMs);
	uint32 elapsedTime = currentTime - fe._startTime;
	if (elapsedTime <= fe._duration) { // Effect in progress
		const uint32 elapsedSteps = static_cast<uint32>((static_cast<uint64>(elapsedTime) * MohawkEngine_Zoombini::kAnimateFrameRate) / MohawkEngine_Zoombini::kAnimationClockTimeBaseMs);
		uint32 stepIdx = MIN<uint32>(elapsedSteps, steps);
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
	return imgSurface->getRect();
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
	if (!readPaletteInternal(static_cast<int16>(id), _unmodifiedPaletteBytes, ARRAYSIZE(_unmodifiedPaletteBytes), MohawkMetaEngine_Zoombini::PaletteFilter::kRawPalette)) {
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
	return readPaletteInternal(id, destBuf, destBufSize, _vm->getPaletteFilter());
}

bool ZoombiniGraphics::readPaletteInternal(int16 id, byte *destBuf, size_t destBufSize, MohawkMetaEngine_Zoombini::PaletteFilter filter) {
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

	if (shplId != id) {
		warning("ZmbGraphics: SHPL %d header has mismatched id %d", id, shplId);
		delete shplStream;
		return false;
	}
	if (shplId < 0) {
		warning("ZmbGraphics: Invalid SHPL %d shape id", id);
		delete shplStream;
		return false;
	}
	if (shapeCount == 0) {
		warning("ZmbGraphics: Invalid SHPL %d shape count", id);
		delete shplStream;
		return false;
	}
	if (static_cast<uint32>(INT16_MAX - shplId) < static_cast<uint32>(shapeCount - 1)) {
		warning("ZmbGraphics: Invalid SHPL %d shape range", id);
		delete shplStream;
		return false;
	}
	if (255 < paletteColorStart) {
		warning("ZmbGraphics: Invalid SHPL %d palette start", id);
		delete shplStream;
		return false;
	}
	if (256 - paletteColorStart < paletteColorCount) {
		warning("ZmbGraphics: Invalid SHPL %d palette range", id);
		delete shplStream;
		return false;
	}

	const uint32 expectedSize = 8 + static_cast<uint32>(paletteColorCount) * 4;
	if (!ZmbResource::hasSize(shplStream, expectedSize, expectedSize)) {
		warning("ZmbGraphics: Invalid SHPL %d payload size", id);
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
	applyPaletteFilter(destBuf, paletteColorStart, paletteColorCount, filter);

	return true;
}

void ZoombiniGraphics::applyPaletteFilter(byte *paletteBytes, uint16 startIdx, uint16 colorCount, MohawkMetaEngine_Zoombini::PaletteFilter filter) {
	if (filter != MohawkMetaEngine_Zoombini::PaletteFilter::kBrightenPalette)
		return;

	const uint16 endEntry = startIdx + colorCount;
	for (uint16 colorIdx = startIdx; colorIdx < endEntry; colorIdx++) {
		for (int ch = 0; ch < 3; ch++) {
			byte &v = paletteBytes[colorIdx * 3 + ch];
			if (v != 0)
				v = v + 31 - (v >> 3);
		}
	}
}

void ZoombiniGraphics::refreshPalette() {
	memcpy(_paletteBytes, _unmodifiedPaletteBytes, sizeof(_paletteBytes));
	applyPaletteFilter(_paletteBytes, 0, ARRAYSIZE(_paletteBytes) / 3, _vm->getPaletteFilter());

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
