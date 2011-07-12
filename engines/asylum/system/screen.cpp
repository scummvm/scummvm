/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "asylum/system/screen.h"

#include "asylum/resources/worldstats.h"

#include "asylum/system/graphics.h"

#include "asylum/views/scene.h"

#include "asylum/asylum.h"
#include "asylum/respack.h"

#include <stdarg.h>	// For va_list etc.

namespace Asylum {

Screen::Screen(AsylumEngine *vm) : _vm(vm) ,
	_useColorKey(false), _transTableCount(0), _transTableIndex(NULL), _transTableData(NULL), _transTableBuffer(NULL) {
	_backBuffer.create(640, 480, Graphics::PixelFormat::createFormatCLUT8());

	_flag = -1;
	_clipRect = Common::Rect(0, 0, 640, 480);
}

Screen::~Screen() {
	_backBuffer.free();

	clearTransTables();

	// Zero-out passed pointers
	_vm = NULL;
}

//////////////////////////////////////////////////////////////////////////
// Drawing
//////////////////////////////////////////////////////////////////////////
void Screen::draw(ResourceId resourceId) {
	draw(resourceId, 0, Common::Point(0, 0), kDrawFlagNone, kResourceNone, Common::Point(0, 0), false);
}

void Screen::draw(ResourceId resourceId, uint32 frameIndex, const Common::Point &source, DrawFlags flags, bool colorKey) {
	draw(resourceId, frameIndex, source, flags, kResourceNone, Common::Point(0, 0), colorKey);
}

void Screen::draw(ResourceId resourceId, uint32 frameIndex, const Common::Point &source, DrawFlags flags, int32 transTableNum) {
	byte *index = _transTableIndex;
	selectTransTable(transTableNum);

	draw(resourceId, frameIndex, source, (DrawFlags)(flags | 0x90000000));

	_transTableIndex = index;
}

void Screen::draw(ResourceId resourceId, uint32 frameIndex, const Common::Point &source, DrawFlags flags, ResourceId resourceIdDestination, const Common::Point &destination, bool colorKey) {
	// Get the frame to draw
	GraphicResource *resource = new GraphicResource(_vm, resourceId);
	GraphicFrame *frame = resource->getFrame(frameIndex);
	ResourceEntry *resourceMask = NULL;

	// Compute coordinates
	Common::Rect src;
	Common::Rect dest;
	Common::Rect srcMask;
	Common::Rect destMask;

	dest.left = source.x + frame->x;
	if (flags & kDrawFlagMirrorLeftRight) {
		if (_flag == -1) {
			if ((resource->getData().flags & 15) >= 2) {
				dest.left = source.x + resource->getData().maxWidth - frame->getWidth() - frame->x;
			}
		} else {
			dest.left += 2 * (_flag - (frame->getHeight() * 2 - frame->x));
		}
	}

	dest.top = source.y + frame->y;
	dest.right = dest.left + frame->getWidth();
	dest.bottom = dest.top + frame->getHeight();

	src.left = 0;
	src.top = 0;
	src.right = frame->getWidth();
	src.bottom = frame->getHeight();

	clip(&src, &dest, flags);

	bool masked = false;
	if (resourceIdDestination) {
		masked = true;

		// Get the resource to use as a mask
		resourceMask = getResource()->get(resourceIdDestination);

		// Adjust masked rectangles
		srcMask = Common::Rect(0, 0, resourceMask->getData(0), resourceMask->getData(4));

		destMask = Common::Rect(destination.y,
		                        destination.x,
		                        destination.y + resourceMask->getData(0),
		                        destination.x + resourceMask->getData(4));

		clip(&srcMask, &destMask, 0);

		if (dest.intersects(destMask))
			masked = false;
	}

	// Set the color key (always 0 if set)
	_useColorKey = colorKey;

	if (masked)
		blitMasked(frame, &src, resourceMask->data + 8, &srcMask, &destMask, resourceMask->getData(4), &dest, flags);
	else
		blit(frame, &src, &dest, flags);

	delete resource;
}

//////////////////////////////////////////////////////////////////////////
// Misc
//////////////////////////////////////////////////////////////////////////
void Screen::clear() {
	_backBuffer.fillRect(Common::Rect(0, 0, 640, 480), 0);
	copyBackBufferToScreen();
}

void Screen::drawWideScreenBars(int16 barSize) const {
	if (barSize > 0) {
		_vm->_system->lockScreen()->fillRect(Common::Rect(0, 0, 640, barSize), 0);
		_vm->_system->unlockScreen();
		_vm->_system->lockScreen()->fillRect(Common::Rect(0, 480 - barSize, 640, 480), 0);
		_vm->_system->unlockScreen();
	}
}

void Screen::fillRect(int32 x, int32 y, int32 width, int32 height, int32 color) {
	_backBuffer.fillRect(Common::Rect(x, y, x + width, y + height), color);
}

void Screen::copyBackBufferToScreen() {
	_vm->_system->copyRectToScreen((byte *)_backBuffer.pixels, _backBuffer.w, 0, 0, _backBuffer.w, _backBuffer.h);
}

void Screen::clip(Common::Rect *source, Common::Rect *destination, int32 flags) {
	int32 diffLeft = _clipRect.left - destination->left;

	if (diffLeft > 0) {
		destination->left = _clipRect.left;

		if (flags & 2)
			source->right -= diffLeft;
		else
			source->left += diffLeft;
	}

	int32 diffRight = destination->right - _clipRect.right;
	if (diffRight > 0) {
		destination->right -= diffRight;

		if (flags & 2)
			source->left += diffRight;
		else
			source->right -= diffRight;
	}

	int32 diffTop = _clipRect.top - destination->top;
	if (diffTop > 0) {
		destination->top = _clipRect.top;
		source->top += diffTop;
	}

	int32 diffBottom = destination->bottom - _clipRect.bottom;
	if (diffBottom > 0) {
		source->bottom -= diffBottom;
		destination->bottom -= diffBottom;
	}

	// Check validity
	if (!source->isValidRect())
		error("[Screen::clip] Invalid resulting source rectangle");

	if (!destination->isValidRect())
		error("[Screen::clip] Invalid resulting destination rectangle");
}

void Screen::takeScreenshot() {
	warning("[Screen::takeScreenshot] Screenshot function not implemented!");
}

//////////////////////////////////////////////////////////////////////////
// Palette
//////////////////////////////////////////////////////////////////////////
void Screen::setPalette(ResourceId id) {
	setPalette(getResource()->get(id)->data + 32);
}

void Screen::setPalette(byte *rgbPalette) const {
	byte palette[256 * 3];
	byte *p = rgbPalette;

	// skip first color and set it to black always.
	memset(palette, 0, 3);
	p += 3;

	for (int32 i = 1; i < 256; i++) {
		palette[i * 3 + 0] = (byte)(*p++ << 2);
		palette[i * 3 + 1] = (byte)(*p++ << 2);
		palette[i * 3 + 2] = (byte)(*p++ << 2);
	}

	_vm->_system->getPaletteManager()->setPalette(palette, 0, 256);
}

void Screen::setPaletteGamma(ResourceId id) {
	warning("[Screen::setGammaPalette] not implemented");
}

void Screen::setupPaletteAndStartFade(uint32 red, int32 milliseconds, int32 param) {
	warning("[Screen::setupPaletteAndStartFade] Not implemented!");
}

void Screen::stopFadeAndSetPalette(ResourceId id, int32 milliseconds, int32 param) {
	// HACK so we have a proper screen palette
	setPalette(id);
	warning("[Screen::stopFadeAndSetPalette] Not implemented!");
}

void Screen::paletteFade(uint32 red, int32 milliseconds, int32 param) {
	warning("[Screen::paletteFade] Not implemented!");
}

void Screen::startPaletteFade(ResourceId resourceId, int32 milliseconds, int32 param) {
	warning("[Screen::startPaletteFade] Not implemented!");
}

void Screen::updatePalette() {
	error("[Screen::updatePalette] Not implemented!");
}

void Screen::updatePalette(int32 param) {
	error("[Screen::updatePalette] Not implemented!");
}

void Screen::makeGreyPalette() {
	warning("[Screen::makeGreyPalette] Not implemented!");
}

void Screen::setupPalette(byte *buffer, int start, int count) {
	warning("[Screen::setupPalette] Not implemented!");
}

//////////////////////////////////////////////////////////////////////////
// Gamma
//////////////////////////////////////////////////////////////////////////
void Screen::setGammaLevel(ResourceId id, int32 val) {
	warning("[Screen::setGammaLevel] not implemented");
}

//////////////////////////////////////////////////////////////////////////
// Transparency tables
//////////////////////////////////////////////////////////////////////////
void Screen::setupTransTable(ResourceId resourceId) {
	if (resourceId)
		setupTransTables(1, resourceId);
	else
		setupTransTables(0);
}

void Screen::setupTransTables(uint32 count, ...) {
	if (!count) {
		clearTransTables();
		return;
	}

	// Load tables
	va_list va;
	va_start(va, count);

	if (_transTableCount != count)
		clearTransTables();

	_transTableCount = count;

	if (!_transTableData) {
		_transTableData = (byte *)malloc((count + 1) * 65536);
		_transTableBuffer = _transTableData + 65536;
		_transTableIndex = _transTableBuffer;
	}

	uint32 index = 0;
	for (uint32 i = 0; i < _transTableCount; i++) {
		ResourceId id = va_arg(va, ResourceId);

		memcpy(&_transTableBuffer[index], getResource()->get(id)->data, 65536);
		index += 65536;
	}
}

void Screen::clearTransTables() {
	free(_transTableData);
	_transTableData = NULL;
	_transTableBuffer = NULL;
	_transTableIndex = NULL;
	_transTableCount = 0;
}

void Screen::selectTransTable(uint32 index) {
	if (index >= _transTableCount)
		return;

	_transTableIndex = &_transTableBuffer[65536 * index];
}

//////////////////////////////////////////////////////////////////////////
// Graphic queue
//////////////////////////////////////////////////////////////////////////
void Screen::addGraphicToQueue(ResourceId resourceId, uint32 frameIndex, const Common::Point &point, DrawFlags flags, int32 transTableNum, int32 priority) {
	GraphicQueueItem item;
	item.priority = priority;

	item.type = kGraphicItemNormal;
	item.source = point;
	item.resourceId = resourceId;
	item.frameIndex = frameIndex;
	item.flags = flags;
	item.transTableNum = transTableNum;

	_queueItems.push_back(item);
}

void Screen::addGraphicToQueueMasked(ResourceId resourceId, uint32 frameIndex, const Common::Point &source, int32 resourceIdDestination, const Common::Point &destination, DrawFlags flags, int32 priority) {
	GraphicQueueItem item;
	item.priority = priority;

	item.type = kGraphicItemMasked;
	item.source = source;
	item.resourceId = resourceId;
	item.frameIndex = frameIndex;
	item.flags = flags;
	item.resourceIdDestination = resourceIdDestination;
	item.destination = destination;

	_queueItems.push_back(item);
}

void Screen::addGraphicToQueueCrossfade(ResourceId resourceId, uint32 frameIndex, Common::Point point, int32 objectResourceId, Common::Point objectPoint, int32 transTableNum) {
	// Save current transparency index
	byte *transparencyIndex= _transTableIndex;
	selectTransTable(transTableNum);

	// Get graphic frames
	GraphicResource *resource = new GraphicResource(_vm, resourceId);
	GraphicFrame *frame = resource->getFrame(frameIndex);

	GraphicResource *resourceObject = new GraphicResource(_vm, objectResourceId);
	GraphicFrame *frameObject = resourceObject->getFrame(0);

	// Compute rectangles
	Common::Rect src(0, 0, frame->getWidth(), frame->getHeight());
	Common::Rect dst = src;
	dst.translate(point.x + frame->x, point.y + frame->y);

	clip(&src, &dst, 0);

	// Set the color key (always 0)
	_useColorKey = true;

	blitCrossfade((byte *)_backBuffer.pixels          + dst.top                   * _backBuffer.pitch          + dst.left,
	              (byte *)frame->surface.pixels       + src.top                    * frame->surface.pitch       + src.left,
	              (byte *)frameObject->surface.pixels + (objectPoint.y + dst.top) * frameObject->surface.pitch + (dst.left + objectPoint.x),
	              dst.width() + (dst.height() << 16),
	              frame->surface.pitch       - dst.width(),
	              _backBuffer.pitch          - dst.width(),
	              frameObject->surface.pitch - dst.width());

	// Restore transparency table
	_transTableIndex = transparencyIndex;

	delete resource;
	delete resourceObject;
}

void Screen::addGraphicToQueue(GraphicQueueItem const &item) {
	_queueItems.push_back(item);
}

void Screen::drawGraphicsInQueue() {
	// Sort by priority first
	graphicsSelectionSort();

	for (Common::Array<GraphicQueueItem>::const_iterator i = _queueItems.begin(); i != _queueItems.end(); i++) {
		const GraphicQueueItem *item = i;

		if (item->type == kGraphicItemNormal) {
			if (item->transTableNum <= 0 || Config.performance <= 1)
				draw(item->resourceId, item->frameIndex, item->source, item->flags);
			else
				draw(item->resourceId, item->frameIndex, item->source, item->flags, item->transTableNum - 1);
		} else if (item->type == kGraphicItemMasked) {
			draw(item->resourceId, item->frameIndex, item->source, item->flags, item->resourceIdDestination, item->destination);
		}
	}
}

void Screen::clearGraphicsInQueue() {
	_queueItems.clear();
}

void Screen::graphicsSelectionSort() {
	uint32 minIdx;

	for (uint32 i = 0; i < _queueItems.size(); i++) {
		minIdx = i;

		for (uint32 j = i + 1; j < _queueItems.size(); j++)
			if (_queueItems[j].priority > _queueItems[i].priority)
				minIdx = j;

		if (i != minIdx)
			swapGraphicItem(i, minIdx);
	}
}

void Screen::swapGraphicItem(int32 item1, int32 item2) {
	GraphicQueueItem temp;
	temp = _queueItems[item1];
	_queueItems[item1] = _queueItems[item2];
	_queueItems[item2] = temp;
}

void Screen::deleteGraphicFromQueue(ResourceId resourceId) {
	for (uint32 i = 0; i < _queueItems.size(); i++) {
		if (_queueItems[i].resourceId == resourceId) {
			_queueItems.remove_at(i);
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Graphic Data
//////////////////////////////////////////////////////////////////////////
void Screen::blit(GraphicFrame *frame, Common::Rect *source, Common::Rect *destination, int32 flags) {
	if (flags & 0x80000000) {
		// Used when closing from the menu (and more?)
		error("[Screen::blit] not implemented");
	} else if (flags) {
		blt(destination, frame, source, flags);
	} else {
		bltFast(destination->left, destination->top, frame, source);
	}
}

void Screen::blitMasked(GraphicFrame *frame, Common::Rect *source, byte *maskData, Common::Rect *sourceMask, Common::Rect *destMask, int maskHeight, Common::Rect *destination, int32 flags) {
	// TODO
	blit(frame, source, destination, flags);
}

void Screen::blitCrossfade(byte *dstBuffer, byte *srcBuffer, byte *objectBuffer, int widthHeight, uint32 srcPitch, uint32 dstPitch, uint32 objectPitch) {
	error("[Screen::blitCrossfade] Not implemented");
}

void Screen::blt(Common::Rect *dest, GraphicFrame* frame, Common::Rect *source, int32 flags) {
	if (_useColorKey) {
		copyToBackBufferWithTransparency((byte *)frame->surface.pixels + (source->top * frame->surface.w + source->left),
		                                 frame->surface.w,
		                                 dest->left,
		                                 dest->top,
		                                 source->width(),
		                                 source->height(),
		                                 flags & kDrawFlagMirrorLeftRight);
	} else {
		copyToBackBuffer((byte *)frame->surface.pixels + (source->top * frame->surface.w + source->left),
		                 frame->surface.w,
		                 dest->left,
		                 dest->top,
		                 source->width(),
		                 source->height(),
		                 flags & kDrawFlagMirrorLeftRight);
	}
}

void Screen::bltFast(int32 dX, int32 dY, GraphicFrame* frame, Common::Rect *source) {
	if (_useColorKey) {
		copyToBackBufferWithTransparency((byte *)frame->surface.pixels + (source->top * frame->surface.w + source->left),
		                                 frame->surface.w,
		                                 dX,
		                                 dY,
		                                 source->width(),
		                                 source->height(),
		                                 kDrawFlagNone);
	} else {
		copyToBackBuffer((byte *)frame->surface.pixels + (source->top * frame->surface.w + source->left),
		                 frame->surface.w,
		                 dX,
		                 dY,
		                 source->width(),
		                 source->height(),
		                 kDrawFlagNone);
	}
}

void Screen::copyToBackBuffer(byte *buffer, int32 pitch, int32 x, int32 y, uint32 width, uint32 height, bool mirrored) {
	byte *dest = (byte *)_backBuffer.pixels;

	if (!mirrored) {
		while (height--) {
			memcpy(dest + y * _backBuffer.pitch + x, buffer, width);
			dest += 640;
			buffer += pitch;
		}
	} else {
		error("[Screen::copyToBackBuffer] Mirrored drawing not implemented (no color key)");
	}
}

void Screen::copyToBackBufferWithTransparency(byte *buffer, int32 pitch, int32 x, int32 y, int32 width, int32 height, bool mirrored) {
	byte *dest = (byte *)_backBuffer.pixels;

	int32 left = (x < 0) ? -x : 0;
	int32 top = (y < 0) ? -y : 0;
	int32 right = (x + width > 640) ? 640 - abs(x) : width;
	int32 bottom = (y + height > 480) ? 480 - abs(y) : height;

	for (int32 curY = top; curY < bottom; curY++) {
		for (int32 curX = left; curX < right; curX++) {
			uint32 offset = (mirrored ? right - curX - 1 : curX) + curY * pitch;

			if (buffer[offset] != 0 )
				dest[x + curX + (y + curY) * 640] = buffer[offset];
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Debug
//////////////////////////////////////////////////////////////////////////
void Screen::drawLine(const Common::Point &source, const Common::Point &destination) {
	_backBuffer.drawLine(source.x, source.y, destination.x, destination.y, 0xFF);
}

void Screen::copyToBackBufferClipped(Graphics::Surface *surface, int x, int y) {
	Common::Rect screenRect(getWorld()->xLeft, getWorld()->yTop, getWorld()->xLeft + 640, getWorld()->yTop + 480);
	Common::Rect animRect(x, y, x + surface->w, y + surface->h);
	animRect.clip(screenRect);

	if (!animRect.isEmpty()) {
		// Translate animation rectangle
		animRect.translate(-(int16)getWorld()->xLeft, -(int16)getWorld()->yTop);

		int startX = animRect.right  == 640 ? 0 : surface->w - animRect.width();
		int startY = animRect.bottom == 480 ? 0 : surface->h - animRect.height();

		if (surface->w > 640)
			startX = getWorld()->xLeft;
		if (surface->h > 480)
			startY = getWorld()->yTop;

		_vm->screen()->copyToBackBufferWithTransparency(
			((byte*)surface->pixels) +
			startY * surface->pitch +
			startX * surface->format.bytesPerPixel,
			surface->pitch,
			animRect.left,
			animRect.top,
			animRect.width(),
			animRect.height());
	}
}

} // end of namespace Asylum
