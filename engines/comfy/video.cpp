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
	_resolutionChangeCount = 0;
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
	_resolutionChangeCount = 0;
	_renderDirtyCount = 0;
	_videoInitialized = false;
}

void ComfyEngine::videoSetResolution() {
	VideoRectRecord record;
	record.left = 0;
	record.top = 0;
	record.right = _logicalScreenWidth;
	record.bottom = _logicalScreenHeight;
	record.area = MIN<uint32>(framebufferBytes(), 0xFFFF);
	videoFindBestMode(record);
}

void ComfyEngine::renderSetDirty() {
	_renderDirtyCount = 1;
}

void ComfyEngine::renderFlushDirty() {
	_resolutionChangeCount = 0;
	if (_renderDirtyCount) {
		_renderDirtyCount--;
		videoSetResolution();
	}
}

void ComfyEngine::renderInvalidateFullFrame() {
	renderFlushDirty();
	videoSetResolution();
}

void ComfyEngine::videoFindBestMode(VideoRectRecord record) {
	record.left = CLIP<int16>(record.left, 0, _logicalScreenWidth);
	record.top = CLIP<int16>(record.top, 0, _logicalScreenHeight);
	record.right = CLIP<int16>(record.right, 0, _logicalScreenWidth);
	record.bottom = CLIP<int16>(record.bottom, 0, _logicalScreenHeight);

	if (_resolutionChangeCount >= COMFY_RESOLUTION_CHANGE_CAPACITY) {
		renderFlushDirty();
		videoSetResolution();
		return;
	}

	for (int i = _resolutionChangeCount - 1; i >= 0; i--) {
		VideoRectRecord &existing = _resolutionChanges[i];
		if (existing.right < record.left && existing.left <= record.right &&
				existing.bottom >= record.top && existing.top <= record.bottom)
			continue;

		VideoRectRecord merged;
		merged.left = MIN(existing.left, record.left) / 2 * 2;
		merged.right = (MAX(existing.right, record.right) + 1) / 2 * 2;
		merged.top = MIN(existing.top, record.top) / 2 * 2;
		merged.bottom = (MAX(existing.bottom, record.bottom) + 1) / 2 * 2;
		merged.area = (uint32)(merged.right - merged.left) * (merged.bottom - merged.top);

		if ((uint32)existing.area + record.area >= merged.area) {
			existing = merged;
			return;
		}
	}

	_resolutionChanges[_resolutionChangeCount] = record;
	_resolutionChangeCount++;
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
		for (uint i = 0; i < _resolutionChangeCount; i++)
			dirtyArea += _resolutionChanges[i].area;

		if (dirtyArea > framebufferBytes() - 1000) {
			framebufCopyAll(_presentBuffer, _framebufPtr);
			_screen->copyRectToSurface(_presentBuffer, _logicalScreenWidth, 0, 0,
				_logicalScreenWidth, _logicalScreenHeight);
		} else {
			for (uint i = 0; i < _resolutionChangeCount; i++) {
				VideoRectRecord &record = _resolutionChanges[i];
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
