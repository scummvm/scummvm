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

#include "hollywood/scenes/intro/intro_scene.h"

#include "common/events.h"
#include "common/system.h"

#include "hollywood/hollywood.h"

namespace Hollywood {

IntroSceneBase::IntroSceneBase(HollywoodEngine *vm, const char *debugName,
		uint32 sceneFramebufferSize, uint32 savedFramebufferSize) :
		_vm(vm),
		_debugName(debugName),
		_skipRequested(false) {
	_paletteCurrent.resize(kPaletteSize);
	_sceneFramebuffer.resize(sceneFramebufferSize);
	_savedFramebuffer.resize(savedFramebufferSize);
	_screen.create(HollywoodEngine::kScreenWidth, HollywoodEngine::kScreenHeight, Graphics::PixelFormat::createFormatCLUT8());
	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	memset(_sceneFramebuffer.data(), 0, _sceneFramebuffer.size());
	memset(_savedFramebuffer.data(), 0, _savedFramebuffer.size());
}

uint IntroSceneBase::presentRowOffset() const {
	return 0;
}

uint IntroSceneBase::presentXOffset() const {
	return 0;
}

void IntroSceneBase::presentFrame() {
	_displayPalette.uploadFrom6Bit(_paletteCurrent);

	if (_screen.empty())
		_screen.create(HollywoodEngine::kScreenWidth, HollywoodEngine::kScreenHeight, Graphics::PixelFormat::createFormatCLUT8());

	const Graphics::Surface &framebuffer = _sceneFramebuffer.surface();
	const uint rowOffset = presentRowOffset();
	const uint xOffset = presentXOffset();
	for (uint y = 0; y < HollywoodEngine::kScreenHeight; ++y) {
		byte *destination = (byte *)_screen.getBasePtr(0, y);
		const uint sourceY = y + rowOffset;
		if (sourceY < (uint)framebuffer.h && xOffset + HollywoodEngine::kScreenWidth <= (uint)framebuffer.w) {
			memcpy(destination, framebuffer.getBasePtr(xOffset, sourceY), HollywoodEngine::kScreenWidth);
		} else {
			memset(destination, 0, HollywoodEngine::kScreenWidth);
		}
	}

	drawFrameOverlays();
	g_system->copyRectToScreen(_screen.getPixels(), _screen.pitch, 0, 0,
		HollywoodEngine::kScreenWidth, HollywoodEngine::kScreenHeight);
	g_system->updateScreen();
}

bool IntroSceneBase::pollEvents() {
	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_QUIT:
		case Common::EVENT_RETURN_TO_LAUNCHER:
			Engine::quitGame();
			stopAudio();
			return true;
		case Common::EVENT_KEYDOWN:
			if (event.kbd.keycode == Common::KEYCODE_ESCAPE ||
					event.kbd.keycode == Common::KEYCODE_RETURN ||
					event.kbd.keycode == Common::KEYCODE_SPACE) {
				_skipRequested = true;
				stopAudio();
				return true;
			}
			break;
		default:
			break;
		}
	}

	return false;
}

bool IntroSceneBase::delay(uint32 millis) {
	uint32 remaining = millis;
	while (remaining != 0 && !_skipRequested && !Engine::shouldQuit()) {
		if (pollEvents())
			return true;

		const uint32 slice = MIN<uint32>(remaining, 10);
		g_system->delayMillis(slice);
		remaining -= slice;
	}

	return _skipRequested || Engine::shouldQuit();
}

bool IntroSceneBase::revealSavedFramebufferWithCurtain(byte bandWidth) {
	for (int sweepOffset = 0xdc; sweepOffset >= 0 && !_skipRequested && !Engine::shouldQuit(); sweepOffset -= bandWidth) {
		if (pollEvents())
			return true;
		revealSavedFramebufferBand((uint)sweepOffset, bandWidth);
		presentFrame();
	}

	return _skipRequested || Engine::shouldQuit();
}

bool IntroSceneBase::clearSceneFramebufferWithCurtain(byte bandWidth) {
	for (uint sweepOffset = 0; sweepOffset < 240 && !_skipRequested && !Engine::shouldQuit(); sweepOffset += bandWidth) {
		if (pollEvents())
			return true;
		clearSceneFramebufferBand(sweepOffset, bandWidth);
		presentFrame();
	}

	return _skipRequested || Engine::shouldQuit();
}

void IntroSceneBase::revealSavedFramebufferBand(uint sweepOffset, byte bandWidth) {
	const int innerWidth = HollywoodEngine::kScreenWidth - (2 * (int)sweepOffset);
	if (innerWidth <= 0)
		return;

	const int combinedInset = sweepOffset + bandWidth;
	const int middleHeight = HollywoodEngine::kScreenHeight - (2 * combinedInset);
	const int leftInset = sweepOffset;

	for (uint row = 0; row < bandWidth; ++row) {
		copySavedFramebufferRun(sweepOffset + row, leftInset, innerWidth);
		copySavedFramebufferRun((HollywoodEngine::kScreenHeight - bandWidth - sweepOffset) + row,
			leftInset, innerWidth);
	}

	if (middleHeight > 0) {
		const int middleRightX = sweepOffset + innerWidth - bandWidth;
		for (int row = 0; row < middleHeight; ++row) {
			const int y = combinedInset + row;
			copySavedFramebufferRun(y, leftInset, bandWidth);
			copySavedFramebufferRun(y, middleRightX, bandWidth);
		}
	}
}

void IntroSceneBase::clearSceneFramebufferBand(uint sweepOffset, byte bandWidth) {
	const int innerWidth = HollywoodEngine::kScreenWidth - (2 * (int)sweepOffset);
	if (innerWidth <= 0)
		return;

	const int combinedInset = sweepOffset + bandWidth;
	const int middleHeight = HollywoodEngine::kScreenHeight - (2 * combinedInset);
	const int leftInset = sweepOffset;

	for (uint row = 0; row < bandWidth; ++row) {
		clearSceneFramebufferRun(sweepOffset + row, leftInset, innerWidth);
		clearSceneFramebufferRun((HollywoodEngine::kScreenHeight - bandWidth - sweepOffset) + row,
			leftInset, innerWidth);
	}

	if (middleHeight > 0) {
		const int middleRightX = sweepOffset + innerWidth - bandWidth;
		for (int row = 0; row < middleHeight; ++row) {
			const int y = combinedInset + row;
			clearSceneFramebufferRun(y, leftInset, bandWidth);
			clearSceneFramebufferRun(y, middleRightX, bandWidth);
		}
	}
}

void IntroSceneBase::copySavedFramebufferRun(int y, int x, int width) {
	copySurfaceRun(_savedFramebuffer.surface(), _sceneFramebuffer.surface(), y, x, width);
}

void IntroSceneBase::clearSceneFramebufferRun(int y, int x, int width) {
	clearSurfaceRun(_sceneFramebuffer.surface(), y, x, width);
}

} // End of namespace Hollywood
