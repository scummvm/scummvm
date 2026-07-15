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

#include "comfy/comfy.h"

#include "engines/util.h"

namespace Comfy {

uint32 ComfyEngine::framebufferBytes() {
	return (uint32)_logicalScreenWidth * _logicalScreenHeight;
}

void ComfyEngine::videoInit() {
	if (_videoInitialized)
		return;

	_videoMode = 2;
	_videoScale = 1;
	_screenWidth = _logicalScreenWidth;
	_screenHeight = _logicalScreenHeight;
	_renderWidth = _screenWidth;
	_renderHeight = _screenHeight;
	_viewOffsetX = 0;
	_viewOffsetY = 0;
	initGraphics(_logicalScreenWidth, _logicalScreenHeight);
	_screen = new Graphics::Screen(_logicalScreenWidth, _logicalScreenHeight);
	_framebufPtr = new byte[framebufferBytes()]();
	_presentBuffer = new byte[framebufferBytes()]();
	if (_isPanther)
		_backgroundFramebuf.resize(framebufferBytes());

	_backgroundFrame = 0;
	_dirtyRectCount = 0;
	_renderDirtyCount = 0;
	videoSetResolution();
	videoPresentFrame();
	_videoInitialized = true;
}

void ComfyEngine::videoShutdown(byte restorePalette) {
	if (restorePalette)
		paletteRealize(nullptr);

	colorDatClose();
	delete[] _presentBuffer;
	_presentBuffer = nullptr;
	delete[] _framebufPtr;
	_framebufPtr = nullptr;
	_backgroundFramebuf.clear();
	_backgroundFrame = 0;
	delete _screen;
	_screen = nullptr;
	_dirtyRectCount = 0;
	_renderDirtyCount = 0;
	_videoInitialized = false;
}

void ComfyEngine::videoSetResolution() {
	ComfyRect record;
	record.left = 0;
	record.top = 0;
	record.right = _logicalScreenWidth;
	record.bottom = _logicalScreenHeight;
	record.area = _engineVersion == 1 ? (uint16)framebufferBytes() : framebufferBytes();
	videoFindBestMode(record);
}

void ComfyEngine::renderSetDirty() {
	_renderDirtyCount = 1;
}

void ComfyEngine::renderFlushDirty() {
	_dirtyRectCount = 0;
	if (_renderDirtyCount) {
		_renderDirtyCount--;
		videoSetResolution();
	}
}

void ComfyEngine::renderInvalidateFullFrame() {
	renderFlushDirty();
	videoSetResolution();
}

void ComfyEngine::renderAddDirtyRectMerged(ComfyRect record) {
	int16 alignment = _isPanther ? 4 : 2;
	uint32 mergeThreshold = _game->dirtyRectMergeThreshold;
	record.left = CLIP<int16>(record.left, 0, _logicalScreenWidth);
	record.left = record.left / alignment * alignment;
	record.top = CLIP<int16>(record.top, 0, _logicalScreenHeight);
	record.right = CLIP<int16>(record.right, 0, _logicalScreenWidth);
	record.right = (record.right + alignment - 1) / alignment * alignment;
	record.right = MIN<int16>(record.right, _logicalScreenWidth);
	record.bottom = CLIP<int16>(record.bottom, 0, _logicalScreenHeight);
	record.area = (uint32)((int32)(record.right - record.left) * (record.bottom - record.top));
	if (!record.area)
		return;

	if (_dirtyRectCount >= COMFY_DIRTY_RECT_CAPACITY) {
		renderFlushDirty();
		videoSetResolution();
		return;
	}

	for (int i = _dirtyRectCount - 1; i >= 0; i--) {
		ComfyRect &existing = _dirtyRects[i];
		if (existing.right <= record.left || existing.left >= record.right ||
				existing.bottom <= record.top || existing.top >= record.bottom)
			continue;

		ComfyRect merged;
		merged.left = MIN(existing.left, record.left) / alignment * alignment;
		merged.top = MIN(existing.top, record.top);
		merged.right = (MAX(existing.right, record.right) + alignment - 1) / alignment * alignment;
		merged.bottom = MAX(existing.bottom, record.bottom);
		merged.area = (uint32)((int32)(merged.right - merged.left) * (merged.bottom - merged.top));

		ComfyRect intersection;
		intersection.left = (MAX(existing.left, record.left) + alignment - 1) / alignment * alignment;
		intersection.top = MAX(existing.top, record.top);
		intersection.right = MIN(existing.right, record.right) / alignment * alignment;
		intersection.bottom = MIN(existing.bottom, record.bottom);
		intersection.area = (uint32)((int32)(intersection.right - intersection.left) *
			(intersection.bottom - intersection.top));

		uint32 extraArea = merged.area + intersection.area - record.area - existing.area;
		if (extraArea < mergeThreshold) {
			_dirtyRectCount--;
			_dirtyRects[i] = _dirtyRects[_dirtyRectCount];
			renderAddDirtyRectMerged(merged);
			return;
		}

		if (intersection.area <= mergeThreshold)
			continue;

		ComfyRect *upperRect;
		ComfyRect *lowerRect;
		if (existing.top > record.top) {
			upperRect = &record;
			lowerRect = &existing;
		} else {
			upperRect = &existing;
			lowerRect = &record;
		}

		ComfyRect *bottomRect = existing.bottom < record.bottom ? &record : &existing;
		ComfyRect upperStrip;
		upperStrip.left = upperRect->left;
		upperStrip.top = upperRect->top;
		upperStrip.right = upperRect->right;
		upperStrip.bottom = lowerRect->top;
		upperStrip.area = (uint32)((int32)(upperStrip.right - upperStrip.left) *
			(upperStrip.bottom - upperStrip.top));

		ComfyRect middleStrip;
		middleStrip.left = MIN(lowerRect->left, upperRect->left);
		middleStrip.top = lowerRect->top;
		middleStrip.right = MAX(lowerRect->right, upperRect->right);
		middleStrip.bottom = MIN(lowerRect->bottom, upperRect->bottom);
		middleStrip.area = (uint32)((int32)(middleStrip.right - middleStrip.left) *
			(middleStrip.bottom - middleStrip.top));

		ComfyRect bottomStrip;
		bottomStrip.left = bottomRect->left;
		bottomStrip.top = middleStrip.bottom;
		bottomStrip.right = bottomRect->right;
		bottomStrip.bottom = bottomRect->bottom;
		bottomStrip.area = (uint32)((int32)(bottomStrip.right - bottomStrip.left) *
			(bottomStrip.bottom - bottomStrip.top));

		_dirtyRectCount--;
		_dirtyRects[i] = _dirtyRects[_dirtyRectCount];
		if (upperStrip.area)
			renderAddDirtyRectMerged(upperStrip);

		if (middleStrip.area)
			renderAddDirtyRectMerged(middleStrip);

		if (bottomStrip.area)
			renderAddDirtyRectMerged(bottomStrip);

		return;
	}

	_dirtyRects[_dirtyRectCount] = record;
	_dirtyRectCount++;
}

void ComfyEngine::videoFindBestMode(ComfyRect record) {
	if (_engineVersion >= 2) {
		renderAddDirtyRectMerged(record);
		return;
	}

	if (_engineVersion == 1)
		record.area = (uint16)record.area;

	record.left = CLIP<int16>(record.left, 0, _logicalScreenWidth);
	record.top = CLIP<int16>(record.top, 0, _logicalScreenHeight);
	record.right = CLIP<int16>(record.right, 0, _logicalScreenWidth);
	record.bottom = CLIP<int16>(record.bottom, 0, _logicalScreenHeight);

	if (_dirtyRectCount >= COMFY_DIRTY_RECT_CAPACITY_V1) {
		renderFlushDirty();
		videoSetResolution();
		return;
	}

	for (int i = _dirtyRectCount - 1; i >= 0; i--) {
		ComfyRect &existing = _dirtyRects[i];
		if (existing.right < record.left && existing.left <= record.right &&
				existing.bottom >= record.top && existing.top <= record.bottom)
			continue;

		ComfyRect merged;
		merged.left = MIN(existing.left, record.left) / 2 * 2;
		merged.right = (MAX(existing.right, record.right) + 1) / 2 * 2;
		merged.top = MIN(existing.top, record.top) / 2 * 2;
		merged.bottom = (MAX(existing.bottom, record.bottom) + 1) / 2 * 2;
		merged.area = (uint32)(merged.right - merged.left) * (merged.bottom - merged.top);
		if (_engineVersion == 1)
			merged.area = (uint16)merged.area;

		if (existing.area + record.area >= merged.area) {
			existing = merged;
			return;
		}
	}

	_dirtyRects[_dirtyRectCount] = record;
	_dirtyRectCount++;
}

void ComfyEngine::framebufCopyAll(byte *destination, byte *source) {
	memcpy(destination, source, framebufferBytes());
}

void ComfyEngine::framebufClear(byte *destination, uint16 color) {
	if (destination)
		memset(destination, (byte)color, framebufferBytes());
}

void ComfyEngine::videoPresentFrame() {
	if (!_screen || !_framebufPtr || !_presentBuffer)
		return;

	if (_renderInterleaved && framebufferBytes() == 0xFA00) {
		uint32 byteOffset = 0;
		for (uint16 i = 0; i < 0x3E80;) {
			memcpy(_presentBuffer + byteOffset, _framebufPtr + byteOffset, 4);
			byteOffset += 4;
			if ((i % 0x4E20) > 0x4268)
				i++;

			i++;
		}

		_screen->copyRectToSurface(_presentBuffer, _logicalScreenWidth, 0, 0,
			_logicalScreenWidth, _logicalScreenHeight);
	} else {
		uint32 dirtyArea = 0;
		if (_engineVersion == 1) {
			for (uint i = 0; i < _dirtyRectCount; i++)
				dirtyArea += _dirtyRects[i].area;
		}

		if (_engineVersion == 1 && dirtyArea > framebufferBytes() - 1000) {
			framebufCopyAll(_presentBuffer, _framebufPtr);
			_screen->copyRectToSurface(_presentBuffer, _logicalScreenWidth, 0, 0,
				_logicalScreenWidth, _logicalScreenHeight);
		} else {
			for (uint i = 0; i < _dirtyRectCount; i++) {
				ComfyRect &record = _dirtyRects[i];
				int16 width = record.right - record.left;
				int16 height = record.bottom - record.top;
				if (width <= 0 || height <= 0)
					continue;

				for (int16 y = record.top; y < record.bottom; y++) {
					byte *source = _framebufPtr + (uint32)y * _logicalScreenWidth + record.left;
					byte *destination = _presentBuffer + (uint32)y * _logicalScreenWidth + record.left;
					memcpy(destination, source, width);
				}

				byte *source = _presentBuffer + (uint32)record.top * _logicalScreenWidth + record.left;
				_screen->copyRectToSurface(source, _logicalScreenWidth, record.left, record.top, width, height);
			}
		}
	}

	_screen->update();
	renderFlushDirty();
}

} // End of namespace Comfy
