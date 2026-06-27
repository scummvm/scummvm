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

#include "hollywood/scenes/intro/scene9120.h"

#include "common/debug.h"
#include "common/events.h"
#include "common/system.h"

#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const char *const kI12ArchiveName = "RESOURCE.I12";
const uint16 kScene9120MusicCueId = 0x000c;
const uint16 kScene9120OverlaySoundId = 0x001a;

Scene9120::Scene9120(HollywoodEngine *vm) :
		_vm(vm),
		_music(),
		_soundBank0(),
		_random("hollywood_scene9120"),
		_resourceArenaCursor(0),
		_overlayAccumulator(0),
		_scrollAccumulator(0),
		_actorBobAccumulator(0),
		_smallAnimAccumulator(0),
		_xOffset(0),
		_yOffset(0),
		_actorBobTicksRemaining(0),
		_actorBobFramePhase(0),
		_actorBobDelta(1),
		_smallAnimSequenceState(0),
		_smallAnimFrame(0),
		_viewportDirty(false),
		_skipRequested(false) {
	memset(_resourceChunkOffsets, 0, sizeof(_resourceChunkOffsets));
	_paletteResource.resize(kHollywoodPaletteSize);
	_paletteCurrent.resize(kHollywoodPaletteSize);
	_sceneFramebuffer.resize(kScene9120FramebufferSize);
	_savedFramebuffer.resize(kFrameBufferSize);
	_descriptorBackground.resize(kFrameBufferSize);
	_screen.resize(HollywoodEngine::kScreenWidth * HollywoodEngine::kScreenHeight);
	memset(_paletteResource.data(), 0, _paletteResource.size());
	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	memset(_sceneFramebuffer.data(), 0, _sceneFramebuffer.size());
	memset(_savedFramebuffer.data(), 0, _savedFramebuffer.size());
	memset(_descriptorBackground.data(), 0, _descriptorBackground.size());
}

bool Scene9120::play() {
	for (uint sweepOffset = 0; sweepOffset < 240 && !_skipRequested && !Engine::shouldQuit(); sweepOffset += 0x14) {
		if (pollEvents())
			break;
		clearViewportCurtainBand(sweepOffset, 0x14);
		presentFrame();
	}

	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	presentFrame();

	if (_skipRequested || Engine::shouldQuit())
		return true;

	if (!loadResourceI12Assets())
		return false;

	copyViewportToSavedFramebuffer();
	clearActiveViewport();
	presentFrame();

	memcpy(_paletteCurrent.data(), _paletteResource.data(), _paletteCurrent.size());
	presentFrame();

	if (!_music.isPlaying())
		_music.playMusicCue(kScene9120MusicCueId, 100);

	for (int sweepOffset = 0xdc; sweepOffset >= 0 && !_skipRequested && !Engine::shouldQuit(); sweepOffset -= 0x14) {
		if (pollEvents())
			break;
		revealSavedViewportCurtainBand((uint)sweepOffset, 0x14);
		presentFrame();
	}

	if (!_skipRequested && !Engine::shouldQuit())
		runTimedOverlayPhase();

	if (!_skipRequested && !Engine::shouldQuit()) {
		memcpy(_descriptorBackground.data(), _sceneFramebuffer.data(), _descriptorBackground.size());
		runHoldScrollAndIdlePhase();
	}

	for (uint sweepOffset = 0; sweepOffset < 240 && !_skipRequested && !Engine::shouldQuit(); sweepOffset += 0x14) {
		if (pollEvents())
			break;
		clearViewportCurtainBand(sweepOffset, 0x14);
		presentFrame();
	}

	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	_yOffset = 0;
	presentFrame();
	stopAudio();
	return true;
}

bool Scene9120::loadResourceI12Assets() {
	if (!_vm->resources()->readChunkTable(Common::Path(kI12ArchiveName), _i12ChunkTable)) {
		warning("Failed to read %s header", kI12ArchiveName);
		return false;
	}

	for (uint i = 0; i < kI12RequiredChunkCount; ++i) {
		if (!_i12ChunkTable.isValidChunk(i)) {
			warning("%s is missing required Scene 9120 chunk %u", kI12ArchiveName, i);
			return false;
		}
	}

	if (!loadResourceI12Chunk(0, _sceneFramebuffer, kScene9120FramebufferSize) ||
			!loadResourceI12Chunk(1, _paletteResource, kHollywoodPaletteSize))
		return false;

	uint32 resourceArenaSize = 0;
	for (uint i = 2; i < kI12RequiredChunkCount; ++i)
		resourceArenaSize += _i12ChunkTable.sizes[i];

	_resourceArena.resize(resourceArenaSize);
	memset(_resourceArena.data(), 0, _resourceArena.size());
	_resourceArenaCursor = 0;
	memset(_resourceChunkOffsets, 0, sizeof(_resourceChunkOffsets));

	for (uint i = 2; i < kI12RequiredChunkCount; ++i) {
		if (!loadResourceI12ArenaChunk(i))
			return false;
	}

	_xOffset = 0;
	_yOffset = kScene9120InitialYOffset;
	_actorBobTicksRemaining = 0;
	_actorBobFramePhase = 0;
	_actorBobDelta = 1;
	_smallAnimSequenceState = 0;
	_smallAnimFrame = 0;
	_viewportDirty = false;
	return true;
}

bool Scene9120::loadResourceI12Chunk(uint index, Common::Array<byte> &destination, uint fixedSize) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(_vm->resources()->createChunkReadStream(Common::Path(kI12ArchiveName), index));
	if (!stream) {
		warning("Failed to open %s chunk %u", kI12ArchiveName, index);
		return false;
	}

	if (stream->size() > fixedSize || destination.size() < fixedSize) {
		warning("%s chunk %u does not fit its fixed destination", kI12ArchiveName, index);
		return false;
	}

	memset(destination.data(), 0, destination.size());
	if (stream->read(destination.data(), stream->size()) != (uint32)stream->size()) {
		warning("Failed to read %s chunk %u", kI12ArchiveName, index);
		return false;
	}

	debugC(1, kDebugResources, "Loaded %s chunk %u: size=%u", kI12ArchiveName, index, (uint)stream->size());
	return true;
}

bool Scene9120::loadResourceI12ArenaChunk(uint index) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(_vm->resources()->createChunkReadStream(Common::Path(kI12ArchiveName), index));
	if (!stream) {
		warning("Failed to open %s chunk %u", kI12ArchiveName, index);
		return false;
	}

	if (_resourceArenaCursor + stream->size() > _resourceArena.size()) {
		warning("%s chunk %u does not fit the Scene 9120 arena", kI12ArchiveName, index);
		return false;
	}

	_resourceChunkOffsets[index] = _resourceArenaCursor;
	if (stream->read(_resourceArena.data() + _resourceArenaCursor, stream->size()) != (uint32)stream->size()) {
		warning("Failed to read %s chunk %u", kI12ArchiveName, index);
		return false;
	}

	debugC(1, kDebugResources, "Loaded %s arena chunk %u: offset=%u size=%u",
		kI12ArchiveName, index, _resourceArenaCursor, (uint)stream->size());
	_resourceArenaCursor += stream->size();
	return true;
}

void Scene9120::runTimedOverlayPhase() {
	_overlayAccumulator = kScene9120OverlayInterval;
	uint16 tickIndex = 0;
	uint32 lastFrameMillis = g_system->getMillis();
	while (tickIndex < kScene9120TimedOverlayTicks && !_skipRequested && !Engine::shouldQuit()) {
		if (pollEvents())
			return;

		const uint32 now = g_system->getMillis();
		const uint32 elapsed = now - lastFrameMillis;
		lastFrameMillis = now;
		_overlayAccumulator += elapsed;

		if (_overlayAccumulator >= kScene9120OverlayInterval) {
			_overlayAccumulator %= kScene9120OverlayInterval;
			tickIndex++;
			const byte chunkIndex = getTimedOverlayChunk(tickIndex);
			if (chunkIndex != 0) {
				if ((chunkIndex & 1) == 0)
					_soundBank0.playSample(kScene9120OverlaySoundId, 100);
				drawTimedOverlayChunk(_resourceChunkOffsets[chunkIndex]);
			}
		}

		g_system->delayMillis(1);
	}
}

void Scene9120::runHoldScrollAndIdlePhase() {
	_scrollAccumulator = kScene9120ScrollInterval;
	_actorBobAccumulator = kScene9120ActorBobInterval;
	_smallAnimAccumulator = kScene9120SmallAnimInterval;

	uint32 elapsedTotal = 0;
	uint32 lastFrameMillis = g_system->getMillis();
	while (elapsedTotal < kScene9120HoldMillis && !_skipRequested && !Engine::shouldQuit()) {
		if (pollEvents())
			return;

		const uint32 now = g_system->getMillis();
		const uint32 elapsed = now - lastFrameMillis;
		lastFrameMillis = now;
		elapsedTotal += elapsed;

		_scrollAccumulator += elapsed;
		if (_scrollAccumulator >= kScene9120ScrollInterval) {
			_scrollAccumulator %= kScene9120ScrollInterval;
			if (_yOffset != 0) {
				_viewportDirty = true;
				_yOffset = _yOffset >= 4 ? _yOffset - 4 : 0;
			}
		}

		_actorBobAccumulator += elapsed;
		if (_actorBobAccumulator >= kScene9120ActorBobInterval) {
			_actorBobAccumulator %= kScene9120ActorBobInterval;
			advanceActorBob();
		}

		_smallAnimAccumulator += elapsed;
		if (_smallAnimAccumulator >= kScene9120SmallAnimInterval) {
			_smallAnimAccumulator %= kScene9120SmallAnimInterval;
			advanceSmallAnimation();
		}

		if (_viewportDirty) {
			presentFrame();
			_viewportDirty = false;
		}

		g_system->delayMillis(1);
	}
}

void Scene9120::advanceActorBob() {
	if (_actorBobTicksRemaining == 0) {
		_actorBobTicksRemaining = (byte)(_random.getRandomNumber(3) + 2);
		_actorBobDelta = _random.getRandomNumber(1) == 0 ? -1 : 1;
		return;
	}

	const int nextPhase = (int)_actorBobFramePhase + _actorBobDelta;
	if (nextPhase == 5)
		_actorBobFramePhase = 0;
	else if (nextPhase < 0)
		_actorBobFramePhase = 4;
	else
		_actorBobFramePhase = (byte)nextPhase;

	_actorBobTicksRemaining--;
	restoreAndDrawResourceDescriptorFrame(8, kScene9120ActorBobDescriptorCount, (byte)(_actorBobFramePhase + 5), true);
	if (_yOffset == 0)
		_viewportDirty = true;
}

void Scene9120::advanceSmallAnimation() {
	if (_smallAnimSequenceState == 3) {
		_smallAnimFrame = 0;
		_smallAnimSequenceState = 0;
	} else if (_smallAnimSequenceState == 2) {
		_smallAnimFrame = 1;
		_smallAnimSequenceState = 3;
	} else if (_smallAnimSequenceState == 1) {
		_smallAnimFrame = 2;
		_smallAnimSequenceState = 2;
	} else {
		if (_random.getRandomNumber(0x18) != 0)
			return;

		_smallAnimFrame = 1;
		_smallAnimSequenceState = 1;
	}

	restoreAndDrawResourceDescriptorFrame(9, kScene9120SmallAnimDescriptorCount, _smallAnimFrame, true);
	if (_yOffset == 0)
		_viewportDirty = true;
}

void Scene9120::restoreAndDrawResourceDescriptorFrame(byte localChunkIndex, byte descriptorCount, byte descriptorIndex,
		bool drawFrame) {
	if (localChunkIndex >= kResourceChunkCount)
		return;

	const uint32 baseOffset = _resourceChunkOffsets[localChunkIndex];
	restoreSpriteBackground(_resourceArena, baseOffset, 0, descriptorCount, descriptorIndex,
		_descriptorBackground, _sceneFramebuffer);
	if (drawFrame)
		drawStripSpriteFrame(_resourceArena, baseOffset, 0, descriptorCount, descriptorIndex, _sceneFramebuffer);
}

byte Scene9120::getTimedOverlayChunk(uint tickIndex) const {
	switch (tickIndex) {
	case 80:
		return 2;
	case 85:
		return 3;
	case 116:
		return 4;
	case 121:
		return 5;
	case 153:
		return 6;
	case 158:
		return 7;
	default:
		break;
	}

	return 0;
}

void Scene9120::copyViewportToSavedFramebuffer() {
	const uint32 sourceOffset = (uint32)_yOffset * HollywoodEngine::kSceneBufferWidth + _xOffset;
	if (sourceOffset + _savedFramebuffer.size() > _sceneFramebuffer.size())
		return;

	memcpy(_savedFramebuffer.data(), _sceneFramebuffer.data() + sourceOffset, _savedFramebuffer.size());
}

void Scene9120::clearActiveViewport() {
	const uint32 destinationOffset = (uint32)_yOffset * HollywoodEngine::kSceneBufferWidth + _xOffset;
	if (destinationOffset + kFrameBufferSize > _sceneFramebuffer.size())
		return;

	memset(_sceneFramebuffer.data() + destinationOffset, 0, kFrameBufferSize);
}

void Scene9120::drawTimedOverlayChunk(uint32 baseOffset) {
	if (baseOffset + 2 > _resourceArena.size())
		return;

	const uint16 blockCount = readUint16LE(_resourceArena, baseOffset);
	uint cursor = baseOffset + 2;
	for (uint blockIndex = 0; blockIndex < blockCount; ++blockIndex) {
		if (cursor + 6 > _resourceArena.size())
			return;

		const uint32 packedDestination = readUint32LE(_resourceArena, cursor);
		const uint16 size = readUint16LE(_resourceArena, cursor + 4);
		cursor += 6;

		if (cursor + size > _resourceArena.size())
			return;

		const uint32 destinationOffset = ((packedDestination >> 16) + _yOffset) * HollywoodEngine::kSceneBufferWidth +
			(packedDestination & 0xffff) + 0x2800;
		if (destinationOffset + size <= _sceneFramebuffer.size())
			memcpy(_sceneFramebuffer.data() + destinationOffset, _resourceArena.data() + cursor, size);

		cursor += size;
	}

	presentFrame();
}

void Scene9120::revealSavedViewportCurtainBand(uint sweepOffset, byte bandWidth) {
	const int innerWidth = HollywoodEngine::kScreenWidth - (2 * (int)sweepOffset);
	if (innerWidth <= 0)
		return;

	const int combinedInset = sweepOffset + bandWidth;
	const int middleHeight = HollywoodEngine::kScreenHeight - (2 * combinedInset);
	const int leftInset = sweepOffset;

	for (uint row = 0; row < bandWidth; ++row) {
		const int topY = sweepOffset + row;
		const int bottomY = (HollywoodEngine::kScreenHeight - bandWidth - sweepOffset) + row;
		copySavedRunToScene(topY, topY + _yOffset, leftInset, innerWidth);
		copySavedRunToScene(bottomY, bottomY + _yOffset, leftInset, innerWidth);
	}

	if (middleHeight > 0) {
		const int middleRightX = sweepOffset + innerWidth - bandWidth;
		for (int row = 0; row < middleHeight; ++row) {
			const int y = combinedInset + row;
			copySavedRunToScene(y, y + _yOffset, leftInset, bandWidth);
			copySavedRunToScene(y, y + _yOffset, middleRightX, bandWidth);
		}
	}
}

void Scene9120::clearViewportCurtainBand(uint sweepOffset, byte bandWidth) {
	const int innerWidth = HollywoodEngine::kScreenWidth - (2 * (int)sweepOffset);
	if (innerWidth <= 0)
		return;

	const int combinedInset = sweepOffset + bandWidth;
	const int middleHeight = HollywoodEngine::kScreenHeight - (2 * combinedInset);
	const int leftInset = sweepOffset;

	for (uint row = 0; row < bandWidth; ++row) {
		const int topY = sweepOffset + row + _yOffset;
		const int bottomY = (HollywoodEngine::kScreenHeight - bandWidth - sweepOffset) + row + _yOffset;
		clearSceneRun(topY, leftInset, innerWidth);
		clearSceneRun(bottomY, leftInset, innerWidth);
	}

	if (middleHeight > 0) {
		const int middleRightX = sweepOffset + innerWidth - bandWidth;
		for (int row = 0; row < middleHeight; ++row) {
			const int y = combinedInset + row + _yOffset;
			clearSceneRun(y, leftInset, bandWidth);
			clearSceneRun(y, middleRightX, bandWidth);
		}
	}
}

void Scene9120::copySavedRunToScene(int sourceY, int destinationY, int x, int width) {
	if (width <= 0 || sourceY < 0 || destinationY < 0 || x < 0)
		return;

	const uint sourceOffset = x + sourceY * HollywoodEngine::kSceneBufferWidth;
	const uint destinationOffset = x + destinationY * HollywoodEngine::kSceneBufferWidth;
	if (sourceOffset + width > _savedFramebuffer.size() || destinationOffset + width > _sceneFramebuffer.size())
		return;

	memcpy(_sceneFramebuffer.data() + destinationOffset, _savedFramebuffer.data() + sourceOffset, width);
}

void Scene9120::clearSceneRun(int destinationY, int x, int width) {
	if (width <= 0 || destinationY < 0 || x < 0)
		return;

	const uint destinationOffset = x + destinationY * HollywoodEngine::kSceneBufferWidth;
	if (destinationOffset + width > _sceneFramebuffer.size())
		return;

	memset(_sceneFramebuffer.data() + destinationOffset, 0, width);
}

void Scene9120::presentFrame() {
	presentIndexedFrame(_sceneFramebuffer, _paletteCurrent, _screen, _yOffset, _xOffset);
}

bool Scene9120::pollEvents() {
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

bool Scene9120::delay(uint32 millis) {
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

void Scene9120::stopAudio() {
	_music.stop();
	_soundBank0.stop();
}

} // End of namespace Hollywood
