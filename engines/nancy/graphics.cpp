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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "image/bmp.h"

#include "engines/util.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/renderobject.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/cursor.h"
#include "engines/nancy/state/scene.h"

namespace Nancy {

GraphicsManager::GraphicsManager() :
	_objects(objectComparator),
	_inputPixelFormat(2, 5, 5, 5, 0, 10, 5, 0, 0),
	_screenPixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0),
	_clut8Format(Graphics::PixelFormat::createFormatCLUT8()),
	_transparentPixelFormat(4, 8, 8, 8, 8, 8, 16, 24, 0),
	_isSuppressed(false) {}

void GraphicsManager::init() {
	auto *bsum = GetEngineData(BSUM);
	assert(bsum);

	// Extract transparent color from the boot summary
	if (g_nancy->getGameType() == kGameTypeVampire) {
		_transColor = bsum->paletteTrans;
	} else {
		_transColor = 	(bsum->rTrans << _inputPixelFormat.rShift) |
						(bsum->gTrans << _inputPixelFormat.gShift) |
						(bsum->bTrans << _inputPixelFormat.bShift);
	}

	initGraphics(640, 480, &_screenPixelFormat);
	_screen.create(640, 480, _screenPixelFormat);
	_screen.setTransparentColor(getTransColor());
	_screen.clear();

	const ImageChunk *ob0 = (const ImageChunk *)g_nancy->getEngineData("OB0");
	assert(ob0);

	g_nancy->_resource->loadImage(ob0->imageName, _object0);
}

void GraphicsManager::draw(bool updateScreen) {
	if (_isSuppressed && updateScreen) {
		_isSuppressed = false;
		return;
	}

	g_nancy->_cursor->applyCursor();

	// Update graphics for all RenderObjects and determine
	// the areas of the screen that need to be redrawn
	for (auto it : _objects) {
		RenderObject &current = *it;

		current.updateGraphics();

		if (current._needsRedraw) {
			if (current._isVisible) {
				if (current.hasMoved() && !current.getPreviousScreenPosition().isEmpty()) {
					// Object moved to a new location on screen, update the previous one
					_dirtyRects.push_back(current.getPreviousScreenPosition());
				}

				// Redraw the current location
				_dirtyRects.push_back(current.getScreenPosition());
			} else if (!current.getPreviousScreenPosition().isEmpty()) {
				// Object just turned invisible, redraw the last location
				_dirtyRects.push_back(current.getPreviousScreenPosition());
			}
		}

		current._needsRedraw = false;
		current._hasMoved = false;
		current._previousScreenPosition = current._screenPosition;
	}

	// Filter out dirty rects that are completely inside others to reduce overdraw
	for (auto outer = _dirtyRects.begin(); outer != _dirtyRects.end(); ++outer) {
		for (auto inner = _dirtyRects.begin(); inner != _dirtyRects.end(); ++inner) {
			if (inner != outer && (*outer).contains(*inner)) {
				_dirtyRects.erase(inner);
				break;
			}
		}
	}

	// Perform the actual drawing. This checks for cases where something would be fully obscured,
	// and skips them (e.g. redrawing the Viewport won't also redraw the background)
	for (Common::Rect rect : _dirtyRects) {
		for (RenderObject **it = _objects.begin(); it < _objects.end(); ++it) {
			RenderObject &current = **it;

			if (!current._isVisible || current.getScreenPosition().isEmpty()) {
				continue;
			}

			bool shouldSkip = false;

			Common::Rect intersection = rect.findIntersectingRect(current.getScreenPosition());
			if (!intersection.isEmpty()) {
				// Found an intersecting RenderObject. Loop through the following
				// RenderObjects, and see if we have another that fully obscures the intersection
				for (auto it2 = it + 1; it2 < _objects.end(); ++it2) {
					RenderObject &other = **it2;

					if (!other._isVisible || other.getScreenPosition().isEmpty()) {
						continue;
					}

					Common::Rect intersection2 = intersection.findIntersectingRect(other.getScreenPosition());
					if (intersection == intersection2) {
						// The entire area that would be drawn is obscured by another RenderObject.
						// If the obscuring RenderObject is not transparent, we skip drawing current

						if (!other._drawSurface.hasTransparentColor() && other._drawSurface.format != _transparentPixelFormat) {
							// No transparency, skip current
							shouldSkip = true;
							break;
						}
					}
				}

				if (shouldSkip) {
					continue;
				}

				blitToScreen(current, rect.findIntersectingRect(current.getScreenPosition()));
			}
		}
	}

	// Draw the screen
	if (updateScreen) {
		_screen.update();
	}

	// Remove all dirty rects for the next frame
	_dirtyRects.clear();
}

void GraphicsManager::loadFonts(Common::SeekableReadStream *chunkStream) {
	auto *bsum = GetEngineData(BSUM);
	assert(bsum);
	assert(chunkStream);

	chunkStream->seek(0);
	_fonts.resize(bsum->numFonts);
	for (uint i = 0; i < _fonts.size(); ++i) {
		_fonts[i].read(*chunkStream);
	}

	delete chunkStream;
}

void GraphicsManager::addObject(RenderObject *object) {
	for (auto &r : _objects) {
		if (r == object) {
			// Erase and re-add objects already in the array to make sure
			// any changes in the z depth are reflected correctly
			_objects.erase(&r);
		}
	}

	_objects.insert(object);
}

void GraphicsManager::removeObject(RenderObject *object) {
	for (auto &r : _objects) {
		if (r == object) {
			// Make sure the object gets properly cleared
			_dirtyRects.push_back(r->getPreviousScreenPosition());
			_objects.erase(&r);
			break;
		}
	}
}

void GraphicsManager::clearObjects() {
	_objects.clear();
}

void GraphicsManager::redrawAll() {
	for (auto &obj : _objects) {
		obj->_needsRedraw = true;
	}
}

void GraphicsManager::suppressNextDraw() {
	_isSuppressed = true;
}

void GraphicsManager::loadSurfacePalette(Graphics::ManagedSurface &inSurf, const Common::Path &paletteFilename, uint paletteStart, uint paletteSize) {
	Common::File f;
	if (f.open(paletteFilename.append(".bmp"))) {
		Image::BitmapDecoder dec;
		if (dec.loadStream(f)) {
			inSurf.setPalette(dec.getPalette(), paletteStart, paletteSize);
		}
	}
}

void GraphicsManager::copyToManaged(const Graphics::Surface &src, Graphics::ManagedSurface &dst, bool verticalFlip, bool doubleSize) {
	if (dst.w != (doubleSize ? src.w * 2 : src.w) || dst.h != (doubleSize ? src.h * 2 : src.h)) {
		uint8 palette[256 * 3];
		bool hasPalette = dst.hasPalette();
		bool hasTransColor = dst.hasTransparentColor();

		if (hasPalette && g_nancy->getGameType() == kGameTypeVampire) {
			dst.grabPalette(palette, 0, 256);
		}

		dst.create(doubleSize ? src.w * 2 : src.w, doubleSize ? src.h * 2 : src.h, src.format);

		if (hasPalette && g_nancy->getGameType() == kGameTypeVampire) {
			dst.setPalette(palette, 0, 256);
		}

		if (hasTransColor) {
			// Do the same trick with the transparent color
			dst.setTransparentColor(dst.getTransparentColor());
		}
	}

	if (!verticalFlip && !doubleSize) {
		dst.copyRectToSurface(src, 0, 0, Common::Rect(0, 0, src.w, src.h));
		return;
	}

	for (int y = 0; y < src.h; ++y) {
		if (!doubleSize) {
			// Copy single line bottom to top
			memcpy(dst.getBasePtr(0, y), src.getBasePtr(0, src.h - y - 1), src.w * src.format.bytesPerPixel);
		} else {
			// Make four copies of each source pixel
			for (int x = 0; x < src.w; ++x) {
				switch (src.format.bytesPerPixel) {
				case 1: {
					const byte *srcP = (const byte *)src.getBasePtr(x, y);
					uint dstX = x * 2;
					uint dstY = verticalFlip ? (src.h - y - 1) * 2 : src.h - y - 1;
					byte *dstP = ((byte *)dst.getBasePtr(dstX, dstY));
					*dstP = *srcP;
					*(dstP + 1) = *srcP;
					dstP += dst.w;
					*dstP = *srcP;
					*(dstP + 1) = *srcP;
					break;
				}
				case 2: {
					const uint16 *srcP = (const uint16 *)src.getBasePtr(x, y);
					uint dstX = x * 2;
					uint dstY = verticalFlip ? (src.h - y - 1) * 2 : src.h - y - 1;
					uint16 *dstP = ((uint16 *)dst.getBasePtr(dstX, dstY));
					*dstP = *srcP;
					*(dstP + 1) = *srcP;
					dstP += dst.w;
					*dstP = *srcP;
					*(dstP + 1) = *srcP;
					break;
				}
				case 4: {
					const uint32 *srcP = (const uint32 *)src.getBasePtr(x, y);
					uint dstX = x * 2;
					uint dstY = verticalFlip ? (src.h - y - 1) * 2 : src.h - y - 1;
					uint32 *dstP = ((uint32 *)dst.getBasePtr(dstX, dstY));
					*dstP = *srcP;
					*(dstP + 1) = *srcP;
					dstP += dst.w;
					*dstP = *srcP;
					*(dstP + 1) = *srcP;
					break;
				}
				default:
					return;
				}
			}
		}
	}
}

void GraphicsManager::copyToManaged(void *src, Graphics::ManagedSurface &dst, uint srcW, uint srcH, const Graphics::PixelFormat &format, bool verticalFlip, bool doubleSize) {
	// Do things the lazy way and simply create a Surface and pass it to the other overload
	// We do NOT free the surface since it's a temporary object and does not own the pixels
	Graphics::Surface surf;
	surf.w = srcW;
	surf.h = srcH;
	surf.format = format;
	surf.pitch = srcW * format.bytesPerPixel;
	surf.setPixels(src);

	copyToManaged(surf, dst, verticalFlip, doubleSize);
}

// Custom rotation code since Surface::rotoscale() produces incorrect results
// Only works on 16 bit surfaces and ignores transparency
// Rotation is a value between 0 and 3, corresponding to 0, 90, 180, or 270 degrees clockwise
void GraphicsManager::rotateBlit(const Graphics::ManagedSurface &src, Graphics::ManagedSurface &dest, byte rotation) {
	assert(!src.empty() && !dest.empty());
	assert(rotation <= 3);
	assert(src.format.bytesPerPixel == 2 && dest.format.bytesPerPixel == 2);

	uint srcW = src.w;
	uint srcH = src.h;
	const uint16 *s, *e;

	if (rotation % 2) {
		if (src.h != dest.w || src.w != dest.h) {
			// Dest surface is wrong size, destroy it and create an appropriate one
			dest.create(src.h, src.w, src.format);
		}
	} else {
		if (src.w != dest.w || src.h != dest.h) {
			// Dest surface is wrong size, destroy it and create an appropriate one
			dest.create(src.w, src.h, src.format);
		}
	}

	switch (rotation) {
	case 0 :
		// No rotation, just blit
		dest.rawBlitFrom(src, src.getBounds(), Common::Point());
		return;
	case 2 : {
		// 180 degrees
		uint16 *d;
		for (uint y = 0; y < srcH; ++y) {
			s = (const uint16 *)src.getBasePtr(0, y);
			e = (const uint16 *)src.getBasePtr(srcW, y);
			d = (uint16 *)dest.getBasePtr(srcW - 1, srcH - y - 1);
			for (; s < e; ++s, --d) {
				*d = *s;
			}
		}

		break;
	}
	case 1 :
		// 90 degrees
		for (uint y = 0; y < srcH; ++y) {
			s = (const uint16 *)src.getBasePtr(0, y);
			for (uint x = 0; x < srcW; ++x, ++s) {
				*((uint16 *)dest.getBasePtr(srcH - y - 1, x)) = *s;
			}
		}

		break;
	case 3 :
		// 270 degrees
		for (uint y = 0; y < srcH; ++y) {
			s = (const uint16 *)src.getBasePtr(0, y);
			for (uint x = 0; x < srcW; ++x, ++s) {
				*((uint16 *)dest.getBasePtr(y, srcW - x - 1)) = *s;
			}
		}

		break;
	}
}

void GraphicsManager::crossDissolve(const Graphics::ManagedSurface &from, const Graphics::ManagedSurface &to, byte alpha, const Common::Rect rect, Graphics::ManagedSurface &inResult) {
	assert(from.getBounds() == to.getBounds());
	inResult.blitFrom(from, rect, Common::Point());
	inResult.transBlitFrom(to, rect, Common::Point(), (uint32)-1, false, alpha);
}

void GraphicsManager::debugDrawToScreen(const Graphics::ManagedSurface &surf) {
	_screen.blitFrom(surf, Common::Point());
	_screen.update();
}

const Graphics::PixelFormat &GraphicsManager::getInputPixelFormat() {
	if (g_nancy->getGameType() == kGameTypeVampire) {
		return _clut8Format;
	} else {
		return _inputPixelFormat;
	}
}

const Graphics::PixelFormat &GraphicsManager::getScreenPixelFormat() {
	return _screenPixelFormat;
}

const Graphics::PixelFormat &GraphicsManager::getTransparentPixelFormat() {
	return _transparentPixelFormat;
}

void GraphicsManager::grabViewportObjects(Common::Array<RenderObject *> &inArray) {
	// Add the viewport
	inArray.push_back(&(RenderObject &)NancySceneState.getViewport());

	// Add all viewport-relative (non-UI) objects
	for (RenderObject *obj : _objects) {
		if (obj->isViewportRelative()) {
			inArray.push_back(obj);
		}
	}
}

void GraphicsManager::screenshotScreen(Graphics::ManagedSurface &inSurf) {
	draw(false);
	inSurf.free();
	inSurf.copyFrom(_screen);
}

// Draw a given screen-space rectangle to the screen
void GraphicsManager::blitToScreen(const RenderObject &src, Common::Rect screenRect) {
	_screen.blitFrom(src._drawSurface, src._drawSurface.getBounds().findIntersectingRect(src.convertToLocal(screenRect)), screenRect);
}

int GraphicsManager::objectComparator(const void *a, const void *b) {
	if (((const RenderObject*)a)->getZOrder() < ((const RenderObject*)b)->getZOrder()) {
		return -1;
	} else if (((const RenderObject*)a)->getZOrder() > ((const RenderObject*)b)->getZOrder()) {
		return 1;
	} else {
		return 0;
	}
}

} // End of namespace Nancy
