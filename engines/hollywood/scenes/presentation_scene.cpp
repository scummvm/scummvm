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

#include "hollywood/scenes/presentation_scene.h"

#include "common/events.h"
#include "common/system.h"

#include "hollywood/font.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

PresentationScene::TimedPresentationLoop::TimedPresentationLoop(PresentationScene &scene,
		uint32 durationMillis, uint32 maximumSliceMillis) :
		_scene(scene),
		_durationMillis(durationMillis),
		_maximumSliceMillis(MAX<uint32>(maximumSliceMillis, 1)),
		_elapsedMillis(0),
		_sliceMillis(0) {
}

bool PresentationScene::TimedPresentationLoop::beginFrame() {
	if (_elapsedMillis >= _durationMillis || _scene._skipRequested || Engine::shouldQuit())
		return false;
	if (_scene.pollEvents())
		return false;

	_sliceMillis = MIN<uint32>(_durationMillis - _elapsedMillis, _maximumSliceMillis);
	return true;
}

uint32 PresentationScene::TimedPresentationLoop::finishFrame() {
	g_system->delayMillis(_sliceMillis);
	_elapsedMillis += _sliceMillis;
	return _sliceMillis;
}

PresentationScene::PresentationScene(HollywoodEngine *vm, const char *debugName,
		uint32 sceneFramebufferSize, uint32 savedFramebufferSize) :
		_vm(vm),
		_debugName(debugName),
		_resources(),
		_skipRequested(false) {
	_paletteCurrent.resize(kPaletteSize);
	_sceneFramebuffer.resize(sceneFramebufferSize);
	_savedFramebuffer.resize(savedFramebufferSize);
	_screen.create(HollywoodEngine::kScreenWidth, HollywoodEngine::kScreenHeight, Graphics::PixelFormat::createFormatCLUT8());
	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	_sceneFramebuffer.clear();
	_savedFramebuffer.clear();
}

uint PresentationScene::presentRowOffset() const {
	return 0;
}

uint PresentationScene::presentXOffset() const {
	return 0;
}

int PresentationScene::subtitleViewportXOffset() const {
	return 0;
}

int PresentationScene::subtitleViewportYOffset() const {
	return 0;
}

bool PresentationScene::showAnchoredSubtitle(const Common::String &text,
		byte colorIndex, int centerX, int anchorBottomY,
		SpeechOverlayWrapStyle wrapStyle) {
	return showAnchoredSubtitle(_subtitle, text, colorIndex, centerX,
		anchorBottomY, wrapStyle);
}

bool PresentationScene::showAnchoredSubtitle(SpeechOverlay &overlay,
		const Common::String &text, byte colorIndex, int centerX,
		int anchorBottomY, SpeechOverlayWrapStyle wrapStyle) {
	if (!showPositionedSubtitle(overlay, text, colorIndex, centerX, 0, wrapStyle))
		return false;

	layoutSpeechOverlay(overlay, _vm->font(), centerX, anchorBottomY,
		subtitleViewportXOffset());
	return true;
}

bool PresentationScene::showPositionedSubtitle(const Common::String &text,
		byte colorIndex, int centerX, int topY,
		SpeechOverlayWrapStyle wrapStyle) {
	return showPositionedSubtitle(_subtitle, text, colorIndex, centerX, topY,
		wrapStyle);
}

bool PresentationScene::showPositionedSubtitle(SpeechOverlay &overlay,
		const Common::String &text, byte colorIndex, int centerX, int topY,
		SpeechOverlayWrapStyle wrapStyle) {
	clearSubtitle(overlay);
	if (!_vm->subtitlesEnabled() || text.empty() || !_vm->font() ||
			!_vm->font()->isLoaded())
		return false;

	overlay.colorIndex = colorIndex;
	overlay.centerX = centerX;
	overlay.topY = topY;
	wrapSpeechOverlayText(text, centerX - subtitleViewportXOffset(), overlay.lines,
		wrapStyle);
	overlay.visible = !overlay.lines.empty();
	return overlay.visible;
}

void PresentationScene::clearSubtitle() {
	clearSubtitle(_subtitle);
}

void PresentationScene::clearSubtitle(SpeechOverlay &overlay) {
	overlay.visible = false;
	overlay.lines.clear();
}

bool PresentationScene::loadFixedChunk(uint index, Common::Array<byte> &destination,
		uint fixedSize) {
	return _resources.loadFixedChunk(_debugName, index, destination, fixedSize);
}

bool PresentationScene::loadFixedChunk(uint index, IndexedSurfaceBuffer &destination,
		uint fixedSize) {
	return _resources.loadFixedChunk(_debugName, index, destination, fixedSize);
}

bool PresentationScene::loadVariableChunk(uint index,
		Common::Array<byte> &destination) {
	return _resources.loadVariableChunk(index, destination);
}

bool PresentationScene::loadChunkTo(uint index, Common::Array<byte> &destination,
		uint32 destinationOffset) {
	return _resources.loadChunkTo(_debugName, index, destination, destinationOffset);
}

bool PresentationScene::loadArenaChunk(uint index) {
	return _resources.loadArenaChunk(_debugName, index);
}

bool PresentationScene::loadArenaChunk(uint archiveIndex, uint localChunkIndex) {
	return _resources.loadArenaChunk(_debugName, archiveIndex, localChunkIndex);
}

bool PresentationScene::loadArenaChunkAlias(uint sourceIndex, uint aliasIndex,
		uint targetIndex) {
	return _resources.loadArenaChunkAlias(_debugName, sourceIndex, aliasIndex,
		targetIndex);
}

void PresentationScene::drawFrameOverlays() {
	drawSpeechOverlayText(_subtitle, _vm->font(), *_screen.surfacePtr(),
		subtitleViewportXOffset(), subtitleViewportYOffset());
}

void PresentationScene::presentFrame() {
	presentFrame(presentRowOffset(), presentXOffset());
}

void PresentationScene::presentFrame(uint rowOffset, uint xOffset) {
	_displayPalette.uploadFrom6Bit(_paletteCurrent);

	if (_screen.empty())
		_screen.create(HollywoodEngine::kScreenWidth, HollywoodEngine::kScreenHeight, Graphics::PixelFormat::createFormatCLUT8());

	const Graphics::Surface &framebuffer = _sceneFramebuffer.surface();
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

bool PresentationScene::pollEvents() {
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

bool PresentationScene::delay(uint32 millis) {
	TimedPresentationLoop loop(*this, millis);
	while (loop.beginFrame())
		loop.finishFrame();

	return _skipRequested || Engine::shouldQuit();
}

bool PresentationScene::revealSavedFramebufferWithCurtain(byte bandWidth) {
	for (int sweepOffset = 0xdc; sweepOffset >= 0 && !_skipRequested && !Engine::shouldQuit(); sweepOffset -= bandWidth) {
		if (pollEvents())
			return true;
		revealSavedFramebufferBand((uint)sweepOffset, bandWidth);
		presentFrame();
	}

	return _skipRequested || Engine::shouldQuit();
}

bool PresentationScene::clearSceneFramebufferWithCurtain(byte bandWidth) {
	for (uint sweepOffset = 0; sweepOffset < 240 && !_skipRequested && !Engine::shouldQuit(); sweepOffset += bandWidth) {
		if (pollEvents())
			return true;
		clearSceneFramebufferBand(sweepOffset, bandWidth);
		presentFrame();
	}

	return _skipRequested || Engine::shouldQuit();
}

void PresentationScene::revealSavedFramebufferBand(uint sweepOffset, byte bandWidth, uint xOffset) {
	const int innerWidth = HollywoodEngine::kScreenWidth - (2 * (int)sweepOffset);
	if (innerWidth <= 0)
		return;

	const int combinedInset = sweepOffset + bandWidth;
	const int middleHeight = HollywoodEngine::kScreenHeight - (2 * combinedInset);
	const int leftInset = xOffset + sweepOffset;

	for (uint row = 0; row < bandWidth; ++row) {
		copySavedFramebufferRun(sweepOffset + row, leftInset, innerWidth);
		copySavedFramebufferRun((HollywoodEngine::kScreenHeight - bandWidth - sweepOffset) + row,
			leftInset, innerWidth);
	}

	if (middleHeight > 0) {
		const int middleRightX = xOffset + sweepOffset + innerWidth - bandWidth;
		for (int row = 0; row < middleHeight; ++row) {
			const int y = combinedInset + row;
			copySavedFramebufferRun(y, leftInset, bandWidth);
			copySavedFramebufferRun(y, middleRightX, bandWidth);
		}
	}
}

void PresentationScene::clearSceneFramebufferBand(uint sweepOffset, byte bandWidth) {
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

void PresentationScene::copySavedFramebufferRun(int y, int x, int width) {
	copySurfaceRun(_savedFramebuffer.surface(), _sceneFramebuffer.surface(), y, x, width);
}

void PresentationScene::clearSceneFramebufferRun(int y, int x, int width) {
	clearSurfaceRun(_sceneFramebuffer.surface(), y, x, width);
}

} // End of namespace Hollywood
