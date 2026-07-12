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

#include "hollywood/scenes/intro/scene9030.h"

#include "common/debug.h"
#include "common/file.h"
#include "common/path.h"

#include "hollywood/hollywood.h"

namespace Hollywood {

const char *const kScene9030ArchiveName = "RESOURCE.I03";
const uint kScene9030FrameDecodeBufferSize = 0x78000;
const uint kScene9030FramebufferSize = 0x100000;
const uint kScene9030TallFramebufferSize = HollywoodEngine::kSceneBufferWidth * HollywoodEngine::kScreenHeight * 2;
const uint kScene9030HeaderSize = 0x140;
const uint kScene9030SpanHeaderOffset = 0xa0;
const uint kScene9030FinalRowOffset = 0x118;
const uint kScene9030InitialHoldMillis = 5000;
const uint kScene9030ScrollMillis = 6000;
const uint kScene9030FrameMillis = 50;
const byte kScene9030CurtainBandWidth = 0x14;

Scene9030::Scene9030(HollywoodEngine *vm, const IndexedSurfaceBuffer *previousFramebuffer,
		const Common::Array<byte> *previousPalette) :
		IntroSceneBase(vm, "Scene 9030", kScene9030FramebufferSize, kFrameBufferSize),
		_previousFramebuffer(previousFramebuffer),
		_previousPalette(previousPalette),
		_rowOffset(0) {
}

bool Scene9030::play() {
	if (!load())
		return false;

	presentFrame();
	if (delay(kScene9030InitialHoldMillis)) {
		finish();
		return true;
	}

	uint32 elapsedRevealMillis = 0;
	while (elapsedRevealMillis < kScene9030ScrollMillis && !_skipRequested && !Engine::shouldQuit()) {
		if (_rowOffset < kScene9030FinalRowOffset) {
			_rowOffset += 4;
			presentFrame();
		}
		if (delay(kScene9030FrameMillis)) {
			finish();
			return true;
		}
		elapsedRevealMillis += kScene9030FrameMillis;
	}

	_rowOffset = kScene9030FinalRowOffset;
	for (uint sweepOffset = 0; sweepOffset < HollywoodEngine::kScreenHeight &&
			!_skipRequested && !Engine::shouldQuit(); sweepOffset += kScene9030CurtainBandWidth) {
		clearFinalSweepBand(kScene9030FinalRowOffset, sweepOffset, kScene9030CurtainBandWidth);
		presentFrame();
		if (delay(kScene9030FrameMillis)) {
			finish();
			return true;
		}
	}

	finish();
	return true;
}

bool Scene9030::load() {
	_rowOffset = 0;
	memset(_sceneFramebuffer.data(), 0, _sceneFramebuffer.size());
	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());

	if (_previousFramebuffer) {
		const uint copySize = MIN<uint>(_sceneFramebuffer.size(), _previousFramebuffer->size());
		memcpy(_sceneFramebuffer.data(), _previousFramebuffer->data(), copySize);
	}
	if (_previousPalette) {
		const uint copySize = MIN<uint>(_paletteCurrent.size(), _previousPalette->size());
		memcpy(_paletteCurrent.data(), _previousPalette->data(), copySize);
	}

	Common::File file;
	if (!file.open(Common::Path(kScene9030ArchiveName))) {
		warning("Failed to open %s", kScene9030ArchiveName);
		return false;
	}

	if (file.size() < kScene9030HeaderSize) {
		warning("%s is too small", kScene9030ArchiveName);
		return false;
	}

	file.seek(kScene9030SpanHeaderOffset);
	const uint32 sceneByteCount = file.readUint32LE();
	const uint32 paletteByteCount = file.readUint32LE();
	if (sceneByteCount == 0 || sceneByteCount > _sceneFramebuffer.size() ||
			kScene9030HeaderSize + sceneByteCount + paletteByteCount > (uint32)file.size()) {
		warning("%s has invalid scene span: scene=%u palette=%u",
			kScene9030ArchiveName, sceneByteCount, paletteByteCount);
		return false;
	}

	file.seek(kScene9030HeaderSize);
	byte *sceneDestination = _sceneFramebuffer.data() + kScene9030FrameDecodeBufferSize;
	if (sceneByteCount == kScene9030TallFramebufferSize) {
		if (sceneByteCount > _sceneFramebuffer.size()) {
			warning("%s first-edition scene does not fit scene buffer", kScene9030ArchiveName);
			return false;
		}
		sceneDestination = _sceneFramebuffer.data();
	} else if (kScene9030FrameDecodeBufferSize + sceneByteCount > _sceneFramebuffer.size()) {
		warning("%s scene does not fit appended scene buffer", kScene9030ArchiveName);
		return false;
	}

	if (file.read(sceneDestination, sceneByteCount) != sceneByteCount) {
		warning("Failed to read %s scene", kScene9030ArchiveName);
		return false;
	}

	if (paletteByteCount != 0) {
		if (paletteByteCount > _paletteCurrent.size()) {
			warning("%s palette is too large: %u", kScene9030ArchiveName, paletteByteCount);
			return false;
		}
		if (file.read(_paletteCurrent.data(), paletteByteCount) != paletteByteCount) {
			warning("Failed to read %s palette", kScene9030ArchiveName);
			return false;
		}
	}

	debugC(1, kDebugResources, "Loaded %s scene=%u palette=%u",
		kScene9030ArchiveName, sceneByteCount, paletteByteCount);
	return true;
}

uint Scene9030::presentRowOffset() const {
	return _rowOffset;
}

void Scene9030::clearFinalSweepBand(uint rowOffset, uint sweepOffset, byte bandWidth) {
	const int innerWidth = HollywoodEngine::kScreenWidth - (2 * (int)sweepOffset);
	if (innerWidth <= 0)
		return;

	const int combinedInset = sweepOffset + bandWidth;
	const int middleHeight = HollywoodEngine::kScreenHeight - (2 * combinedInset);
	const int leftInset = sweepOffset;

	for (uint row = 0; row < bandWidth; ++row) {
		clearSceneFramebufferRun(rowOffset + sweepOffset + row, leftInset, innerWidth);
		clearSceneFramebufferRun(rowOffset + (HollywoodEngine::kScreenHeight - bandWidth - sweepOffset) + row,
			leftInset, innerWidth);
	}

	if (middleHeight > 0) {
		const int middleLeftX = leftInset;
		const int middleRightX = sweepOffset + innerWidth - bandWidth;
		for (int row = 0; row < middleHeight; ++row) {
			const int y = rowOffset + combinedInset + row;
			clearSceneFramebufferRun(y, middleLeftX, bandWidth);
			clearSceneFramebufferRun(y, middleRightX, bandWidth);
		}
	}
}

void Scene9030::finish() {
	memset(_sceneFramebuffer.data(), 0, _sceneFramebuffer.size());
	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	_rowOffset = 0;
	presentFrame();
}

} // End of namespace Hollywood
