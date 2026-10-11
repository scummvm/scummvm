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
#include "engines/nancy/iff.h"
#include "engines/nancy/renderobject.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/cursor.h"
#include "engines/nancy/state/scene.h"

namespace Nancy {

// The ARGB colors in the game data are 32-bit ARGB pixels
const Graphics::PixelFormat kARGBFormat(4, 8, 8, 8, 8, 16, 8, 0, 24);

uint32 argbToColor(const Graphics::PixelFormat &format, uint32 argb) {
	byte a, r, g, b;
	kARGBFormat.colorToARGB(argb, a, r, g, b);
	return format.ARGBToColor(a, r, g, b);
}

void blendPixel(Graphics::ManagedSurface &surface, int x, int y, byte a, byte r, byte g, byte b) {
	if (x < 0 || y < 0 || x >= surface.w || y >= surface.h) {
		return;
	}

	byte destA, destR, destG, destB;
	surface.format.colorToARGB(surface.getPixel(x, y), destA, destR, destG, destB);

	uint destWeight = destA * (255 - a) / 255;
	uint outA = a + destWeight;
	if (outA == 0) {
		return;
	}

	surface.setPixel(x, y, surface.format.ARGBToColor(outA,
		(r * a + destR * destWeight) / outA,
		(g * a + destG * destWeight) / outA,
		(b * a + destB * destWeight) / outA));
}

byte colorChannelToByte(float channel) {
	return (byte)(CLIP(channel, 0.0f, 1.0f) * 255 + 0.5f);
}

void argbToChannels(uint32 argb, float *channels) {
	byte a, r, g, b;
	kARGBFormat.colorToARGB(argb, a, r, g, b);
	channels[0] = a / 255.0f;
	channels[1] = r / 255.0f;
	channels[2] = g / 255.0f;
	channels[3] = b / 255.0f;
}

uint32 channelsToArgb(const float *channels) {
	return kARGBFormat.ARGBToColor(colorChannelToByte(channels[0]), colorChannelToByte(channels[1]),
		colorChannelToByte(channels[2]), colorChannelToByte(channels[3]));
}

float getBlendAmount(float seconds, float rate) {
	return 1.0f - powf(rate, 30.0f * seconds);
}

bool blendColor(float *current, uint32 target, float amount) {
	float targetChannels[4];
	argbToChannels(target, targetChannels);

	bool changed = false;
	for (uint i = 0; i < 4; ++i) {
		byte before = colorChannelToByte(current[i]);
		current[i] += (targetChannels[i] - current[i]) * amount;
		changed |= colorChannelToByte(current[i]) != before;
	}

	return changed;
}

GraphicsManager::GraphicsManager() :
	_objects(objectComparator),
	_inputPixelFormat16(2, 5, 5, 5, 0, 10, 5, 0, 0),
	_inputPixelFormat24(Graphics::PixelFormat::createFormatBGR24()),
	_inputPixelFormat32(Graphics::PixelFormat::createFormatBGRA32()),
	_screenPixelFormat16(2, 5, 6, 5, 0, 11, 5, 0, 0),
	_screenPixelFormat32(Graphics::PixelFormat::createFormatBGRA32()),
	_clut8Format(Graphics::PixelFormat::createFormatCLUT8()),
	_transparentPixelFormat(Graphics::PixelFormat::createFormatBGRA32()),
	_screen(getScreenBounds().width(), getScreenBounds().height(), getScreenPixelFormat()),
	_isSuppressed(false){}
void GraphicsManager::init() {
	auto *bsum = GetEngineData(BSUM);
	assert(bsum);

	// Extract transparent color from the boot summary
	if (g_nancy->getGameType() == kGameTypeVampire) {
		_transColor = bsum->paletteTrans;
	} else {
		const Graphics::PixelFormat &format = getInputPixelFormat();
		_transColor = (bsum->rTrans << format.rShift) |
					  (bsum->gTrans << format.gShift) |
					  (bsum->bTrans << format.bShift);
	}

	initGraphics(_screen.w, _screen.h, &getScreenPixelFormat());
	_screen.setTransparentColor(getTransColor());
	_screen.clear();

	// Nancy16 runs at a higher resolution than the game was made for. The
	// viewport's chunk holds the resolution the game gets drawn at.
	if (g_nancy->getGameType() >= kGameTypeNancy16) {
		auto *viewportData = GetEngineData(VIEW);
		assert(viewportData);

		_gameSurface.create(viewportData->bounds.width(), viewportData->bounds.height(), getScreenPixelFormat());
		_gameSurface.setTransparentColor(getTransColor());
		_gameSurface.clear();
	}

	// OB0 has been dropped in Nancy16+
	const ImageChunk *ob0 = (const ImageChunk *)g_nancy->getEngineData("OB0");
	if (ob0)
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

		if (current.needsRedraw()) {
			if (current.isVisible()) {
				if (current.hasMoved() && !current.getPreviousScreenPosition().isEmpty()) {
					// Object moved to a new location on screen, update the previous one
					addDirtyRect(current, current.getPreviousScreenPosition());
				}

				// Redraw the current location
				addDirtyRect(current, current.getScreenPosition());
			} else if (!current.getPreviousScreenPosition().isEmpty()) {
				// Object just turned invisible, redraw the last location
				addDirtyRect(current, current.getPreviousScreenPosition());
			}
		}

		current.setNeedsRedraw(false);
		current.setHasMoved(false);
		current.updatePreviousScreenPosition();
	}

	filterDirtyRects(_dirtyRects);

	if (!isGameScaled()) {
		drawObjects(_screen, _dirtyRects, kLayerAll);
	} else {
		// The game gets drawn at its own resolution, and the changed parts get scaled
		// onto the screen, where the objects at the screen's resolution go over them
		drawObjects(_gameSurface, _dirtyRects, kLayerGame);

		for (const Common::Rect &rect : _dirtyRects) {
			// Filtering blends each screen pixel with its game pixel's neighbors
			Common::Rect screenRect = gameToScreen(rect);
			screenRect.grow(2);
			screenRect.clip(Common::Rect(_screen.w, _screen.h));
			_nativeDirtyRects.push_back(screenRect);
		}

		filterDirtyRects(_nativeDirtyRects);

		for (const Common::Rect &rect : _nativeDirtyRects) {
			blitGameToScreen(rect);
		}

		drawObjects(_screen, _nativeDirtyRects, kLayerNative);
	}

	// Draw the screen
	if (updateScreen) {
		_screen.update();
	}

	// Remove all dirty rects for the next frame
	_dirtyRects.clear();
	_nativeDirtyRects.clear();
}

void GraphicsManager::addDirtyRect(const RenderObject &object, const Common::Rect &rect) {
	if (isGameScaled() && object.isNativeResolution()) {
		_nativeDirtyRects.push_back(rect);
	} else {
		_dirtyRects.push_back(rect);
	}
}

bool GraphicsManager::isInLayer(const RenderObject &object, Layer layer) const {
	return layer == kLayerAll || object.isNativeResolution() == (layer == kLayerNative);
}

// Filter out dirty rects that are completely inside others to reduce overdraw
void GraphicsManager::filterDirtyRects(Common::List<Common::Rect> &rects) {
	// Rects that overlap are merged into one. An area that gets drawn twice would
	// have the objects with an alpha channel blended onto it twice.
	bool hasMerged;
	do {
		hasMerged = false;

		for (auto outer = rects.begin(); outer != rects.end() && !hasMerged; ++outer) {
			for (auto inner = rects.begin(); inner != rects.end(); ++inner) {
				if (inner != outer && outer->intersects(*inner)) {
					outer->extend(*inner);
					rects.erase(inner);
					hasMerged = true;
					break;
				}
			}
		}
	} while (hasMerged);
}

// Perform the actual drawing. This checks for cases where something would be fully obscured,
// and skips them (e.g. redrawing the Viewport won't also redraw the background)
void GraphicsManager::drawObjects(Graphics::ManagedSurface &target, const Common::List<Common::Rect> &rects, Layer layer) {
	for (const Common::Rect &rect : rects) {
		for (RenderObject **it = _objects.begin(); it < _objects.end(); ++it) {
			RenderObject &current = **it;

			if (!isInLayer(current, layer) || !current.isVisible() || current.getScreenPosition().isEmpty()) {
				continue;
			}

			bool shouldSkip = false;

			Common::Rect intersection = rect.findIntersectingRect(current.getScreenPosition());
			if (!intersection.isEmpty()) {
				// Found an intersecting RenderObject. Loop through the following
				// RenderObjects, and see if we have another that fully obscures the intersection
				for (auto it2 = it + 1; it2 < _objects.end(); ++it2) {
					RenderObject &other = **it2;

					if (!isInLayer(other, layer) || !other.isVisible() || other.getScreenPosition().isEmpty()) {
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

				blitObject(target, current, rect.findIntersectingRect(current.getScreenPosition()));
			}
		}
	}
}

// The game pixel a screen pixel's center falls on, in 1/256ths of a pixel, as texture
// sampling puts it; the fraction is how much of the next game pixel gets blended in
static int getBilinearPosition(int screenPos, int screenSize, int gameSize) {
	int pos = (2 * screenPos + 1) * gameSize * 128 / screenSize - 128;
	return CLIP<int>(pos, 0, (gameSize - 1) * 256);
}

// The game gets scaled onto the screen with bilinear filtering, like the original's
// Direct3D texture sampling does
void GraphicsManager::blitGameToScreen(Common::Rect screenRect) {
	screenRect.clip(Common::Rect(_screen.w, _screen.h));
	if (screenRect.isEmpty()) {
		return;
	}

	assert(_screen.format.bytesPerPixel == 4 && _gameSurface.format == _screen.format);

	// Where each column samples the game, and how much of the next game pixel it takes
	Common::Array<int> srcColumns(screenRect.width());
	Common::Array<int> columnWeights(screenRect.width());
	for (int x = screenRect.left; x < screenRect.right; ++x) {
		int pos = getBilinearPosition(x, _screen.w, _gameSurface.w);
		srcColumns[x - screenRect.left] = pos >> 8;
		columnWeights[x - screenRect.left] = pos & 0xFF;
	}

	for (int y = screenRect.top; y < screenRect.bottom; ++y) {
		int pos = getBilinearPosition(y, _screen.h, _gameSurface.h);
		int srcY = pos >> 8;
		int rowWeight = pos & 0xFF;

		const byte *row0 = (const byte *)_gameSurface.getBasePtr(0, srcY);
		const byte *row1 = (const byte *)_gameSurface.getBasePtr(0, MIN<int>(srcY + 1, _gameSurface.h - 1));
		byte *dest = (byte *)_screen.getBasePtr(screenRect.left, y);

		for (int i = 0; i < screenRect.width(); ++i) {
			int x0 = srcColumns[i] * 4;
			int x1 = MIN<int>(srcColumns[i] + 1, _gameSurface.w - 1) * 4;
			int colWeight = columnWeights[i];

			for (int channel = 0; channel < 4; ++channel) {
				int top = row0[x0 + channel] * (256 - colWeight) + row0[x1 + channel] * colWeight;
				int bottom = row1[x0 + channel] * (256 - colWeight) + row1[x1 + channel] * colWeight;
				*dest++ = (top * (256 - rowWeight) + bottom * rowWeight + (1 << 15)) >> 16;
			}
		}
	}

	_screen.addDirtyRect(screenRect);
}

Common::Rect GraphicsManager::getGameBounds() const {
	return isGameScaled() ? Common::Rect(_gameSurface.w, _gameSurface.h) : Common::Rect(_screen.w, _screen.h);
}

Common::Point GraphicsManager::screenToGame(const Common::Point &pos) const {
	if (!isGameScaled()) {
		return pos;
	}

	return Common::Point(pos.x * _gameSurface.w / _screen.w, pos.y * _gameSurface.h / _screen.h);
}

Common::Point GraphicsManager::gameToScreen(const Common::Point &pos) const {
	if (!isGameScaled()) {
		return pos;
	}

	return Common::Point(pos.x * _screen.w / _gameSurface.w, pos.y * _screen.h / _gameSurface.h);
}

// The screen area covering every pixel the game rect gets scaled onto
Common::Rect GraphicsManager::gameToScreen(const Common::Rect &rect) const {
	if (!isGameScaled()) {
		return rect;
	}

	return Common::Rect(rect.left * _screen.w / _gameSurface.w,
		rect.top * _screen.h / _gameSurface.h,
		(rect.right * _screen.w + _gameSurface.w - 1) / _gameSurface.w,
		(rect.bottom * _screen.h + _gameSurface.h - 1) / _gameSurface.h);
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

// Fonts get placed at their IDs, which font tags in text refer to. The bold versions
// follow them, and can only be found by name.
void GraphicsManager::loadSystemFonts(const IFF &fontIFF) {
	Common::SeekableReadStream *chunkStream = nullptr;
	for (uint i = 0; (chunkStream = fontIFF.getChunkStream("FONT", i)); ++i) {
		uint32 id = chunkStream->readUint32LE();
		if (id >= _fonts.size()) {
			_fonts.resize(id + 1);
		}

		_fonts[id].readSystemFont(*chunkStream, false);
		delete chunkStream;
	}

	for (uint i = 0; (chunkStream = fontIFF.getChunkStream("FONT", i)); ++i) {
		chunkStream->skip(4);
		_fonts.resize(_fonts.size() + 1);
		_fonts.back().readSystemFont(*chunkStream, true);
		delete chunkStream;
	}

	for (uint i = 0; (chunkStream = fontIFF.getChunkStream("COLR", i)); ++i) {
		uint32 id = chunkStream->readUint32LE();
		if (id >= _fontColors.size()) {
			_fontColors.resize(id + 1);
		}

		// ARGB colors for the normal, disabled, mouse over and pressed states. The
		// focused state looks normal, and the hidden one is transparent.
		FontColor &color = _fontColors[id];
		color.colors[kFontColorNormal] = chunkStream->readUint32BE();
		color.colors[kFontColorFocus] = color.colors[kFontColorNormal];
		color.colors[kFontColorDisabled] = chunkStream->readUint32BE();
		color.colors[kFontColorMouseOver] = chunkStream->readUint32BE();
		color.colors[kFontColorPressed] = chunkStream->readUint32BE();
		delete chunkStream;
	}
}

void GraphicsManager::loadSystemFonts(const ScriptBootData &boot) {
	const int32 kNormalWeight = 400;
	const int32 kBoldWeight = 700;

	for (const ScriptBootData::SystemFont &font : boot.fonts) {
		if (font.id < 0) {
			continue;
		}

		if ((uint)font.id >= _fonts.size()) {
			_fonts.resize(font.id + 1);
		}

		_fonts[font.id].setSystemFont(font.name, font.faceName, font.height, font.isBold ? kBoldWeight : kNormalWeight);
	}

	// Every font also comes in bold, after the ones with a number of their own
	for (const ScriptBootData::SystemFont &font : boot.fonts) {
		_fonts.resize(_fonts.size() + 1);
		_fonts.back().setSystemFont(font.name + "_Bold", font.faceName, font.height, kBoldWeight);
	}

	for (const ScriptBootData::FontColor &color : boot.fontColors) {
		if (color.id < 0) {
			continue;
		}

		if ((uint)color.id >= _fontColors.size()) {
			_fontColors.resize(color.id + 1);
		}

		FontColor &fontColor = _fontColors[color.id];
		fontColor.colors[kFontColorNormal] = color.normal;
		fontColor.colors[kFontColorFocus] = color.normal;
		fontColor.colors[kFontColorDisabled] = color.disabled;
		fontColor.colors[kFontColorMouseOver] = color.mouseOver;
		fontColor.colors[kFontColorPressed] = color.pressed;
	}
}

const Font *GraphicsManager::getFont(const Common::String &name) const {
	int id = getFontID(name);
	return id >= 0 ? &_fonts[id] : nullptr;
}

int GraphicsManager::getFontID(const Common::String &name) const {
	for (uint i = 0; i < _fonts.size(); ++i) {
		if (_fonts[i].getName().equalsIgnoreCase(name)) {
			return i;
		}
	}

	return -1;
}

uint32 GraphicsManager::getFontColor(uint id, const Graphics::PixelFormat &format, FontColorState state) const {
	return argbToColor(format, id < _fontColors.size() ? _fontColors[id].colors[state] : 0xFFFFFFFF);
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

void GraphicsManager::reorderObject(RenderObject *object) {
	// The array is kept sorted as objects are inserted, so an object that has
	// changed its z depth has to be taken out and put back to move. One that is
	// not in the list yet has nothing to reorder; it will be sorted when it is
	// added.
	for (auto &r : _objects) {
		if (r == object) {
			_objects.erase(&r);
			_objects.insert(object);
			return;
		}
	}
}

void GraphicsManager::removeObject(RenderObject *object) {
	for (auto &r : _objects) {
		if (r == object) {
			// Make sure the object gets properly cleared
			addDirtyRect(*r, r->getPreviousScreenPosition());
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
		obj->setNeedsRedraw(true);
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
			inSurf.setPalette(dec.getPalette().data(), paletteStart, paletteSize);
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
	assert(src.format.bytesPerPixel == dest.format.bytesPerPixel);

	const uint srcW = src.w;
	const uint srcH = src.h;
	const uint bpp = src.format.bytesPerPixel;

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

	if (rotation == 0) {
		// No rotation, just blit
		dest.rawBlitFrom(src, src.getBounds(), Common::Point());
		return;
	}

	for (uint y = 0; y < srcH; ++y) {
		const byte *s = (const byte *)src.getBasePtr(0, y);
		for (uint x = 0; x < srcW; ++x, s += bpp) {
			byte *d;
			switch (rotation) {
			case 1:
				// 90 degrees
				d = (byte *)dest.getBasePtr(srcH - y - 1, x);
				break;
			case 2:
				// 180 degrees
				d = (byte *)dest.getBasePtr(srcW - x - 1, srcH - y - 1);
				break;
			default:
				// 270 degrees
				d = (byte *)dest.getBasePtr(y, srcW - x - 1);
				break;
			}

			memcpy(d, s, bpp);
		}
	}
}

void GraphicsManager::crossDissolve(const Graphics::ManagedSurface &from, const Graphics::ManagedSurface &to, byte alpha, const Common::Rect &rect, Graphics::ManagedSurface &inResult) {
	assert(from.getBounds() == to.getBounds());
	inResult.blitFrom(from, rect, Common::Point());
	inResult.transBlitFrom(to, rect, Common::Point(), (uint32)-1, false, alpha);
}

void GraphicsManager::debugDrawToScreen(const Graphics::ManagedSurface &surf) {
	_screen.blitFrom(surf, Common::Point());
	_screen.update();
}

const Graphics::PixelFormat &GraphicsManager::getInputPixelFormat(uint bpp) {
	if (g_nancy->getGameType() == kGameTypeVampire)
		return _clut8Format;

	switch (bpp) {
	case 0:
		return g_nancy->getGameType() >= kGameTypeNancy13 ? _inputPixelFormat32 : _inputPixelFormat16;
	case 16:
		return _inputPixelFormat16;	// RGB555
	case 24:
		return _inputPixelFormat24;
	case 32:
		return _inputPixelFormat32;
	default:
		error("Unsupported input pixel format with bpp %d", bpp);
	}
}

Common::Rect GraphicsManager::getScreenBounds() {
	return (g_nancy->getGameType() >= kGameTypeNancy16) ? Common::Rect(800, 600) : Common::Rect(640, 480);
}

const Graphics::PixelFormat &GraphicsManager::getScreenPixelFormat() {
	return (g_nancy->getGameType() >= kGameTypeNancy13) ? _screenPixelFormat32 : _screenPixelFormat16;
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

// Draw a given screen-space rectangle of an object
void GraphicsManager::blitObject(Graphics::ManagedSurface &target, const RenderObject &src, Common::Rect screenRect) {
	// An object whose image is missing has nothing to draw
	if (src._drawSurface.empty()) {
		return;
	}

	target.blitFrom(src._drawSurface, src._drawSurface.getBounds().findIntersectingRect(src.convertToLocal(screenRect)), screenRect);
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
