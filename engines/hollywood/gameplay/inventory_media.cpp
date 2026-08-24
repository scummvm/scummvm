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

#include "hollywood/gameplay/inventory_media.h"

#include "common/path.h"
#include "common/ptr.h"
#include "common/stream.h"
#include "common/textconsole.h"

#include "hollywood/hollywood.h"
#include "hollywood/resource.h"

namespace Hollywood {

const char *const kInventoryMediaArchiveName = "RESOURCE.I04";
const uint kInventoryMediaFramebufferBytes = HollywoodEngine::kSceneBufferWidth * HollywoodEngine::kSceneBufferHeight;
const uint kSueTapePaletteChunk = 8;
const uint kSueTapeBaseChunk = 9;
const uint kSueTapeFramesChunk = 10;
const uint kPosterPaletteChunk = 11;
const uint kPosterWithoutPhotoPaletteChunk = 13;
const uint kFrankensteinDiaryPaletteChunk = 15;
const uint kTaffyPassPaletteChunk = 17;
const uint kForgedPassPaletteChunk = 19;

InventoryMediaPlayer::InventoryMediaPlayer(HollywoodEngine *vm) :
		_vm(vm),
		_palette(),
		_tapeResource(),
		_tapeDescriptorTableOffset(0),
		_framebuffer() {
}

bool InventoryMediaPlayer::loadStill(InventoryMediaId mediaId) {
	uint paletteChunk = 0;
	switch (mediaId) {
	case kInventoryMediaPoster:
		paletteChunk = kPosterPaletteChunk;
		break;
	case kInventoryMediaPosterWithoutPhoto:
		paletteChunk = kPosterWithoutPhotoPaletteChunk;
		break;
	case kInventoryMediaFrankensteinDiary:
		paletteChunk = kFrankensteinDiaryPaletteChunk;
		break;
	case kInventoryMediaTaffyPass:
		paletteChunk = kTaffyPassPaletteChunk;
		break;
	case kInventoryMediaForgedPass:
		paletteChunk = kForgedPassPaletteChunk;
		break;
	default:
		return false;
	}

	Common::Array<byte> pixels;
	if (!readChunk(paletteChunk, _palette, kPaletteSize) ||
			!readChunk(paletteChunk + 1, pixels, kInventoryMediaFramebufferBytes))
		return false;

	_framebuffer.resize(kInventoryMediaFramebufferBytes);
	memcpy(_framebuffer.data(), pixels.data(), pixels.size());
	_tapeResource.clear();
	_tapeDescriptorTableOffset = 0;
	return true;
}

bool InventoryMediaPlayer::loadSueTape() {
	Common::Array<byte> base;
	Common::Array<byte> frames;
	if (!readChunk(kSueTapePaletteChunk, _palette, kPaletteSize) ||
			!readChunk(kSueTapeBaseChunk, base) ||
			!readChunk(kSueTapeFramesChunk, frames))
		return false;

	_tapeDescriptorTableOffset = base.size();
	_tapeResource.resize(base.size() + frames.size());
	memcpy(_tapeResource.data(), base.data(), base.size());
	memcpy(_tapeResource.data() + base.size(), frames.data(), frames.size());

	_framebuffer.resize(kInventoryMediaFramebufferBytes);
	_framebuffer.clear();
	drawResourceBlockList(_tapeResource, 0, _framebuffer.surface());
	return true;
}

void InventoryMediaPlayer::drawSueTapeFrame(byte frameIndex) {
	if (frameIndex >= kSueTapeFrameCount || _tapeResource.empty())
		return;

	drawStripSpriteFrame(_tapeResource, 0, _tapeDescriptorTableOffset,
		kSueTapeFrameCount, frameIndex, _framebuffer.surface());
}

bool InventoryMediaPlayer::readChunk(uint index, Common::Array<byte> &destination, uint expectedSize) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(
		_vm->resources()->createChunkReadStream(Common::Path(kInventoryMediaArchiveName), index));
	if (!stream) {
		warning("Failed to open %s chunk %u", kInventoryMediaArchiveName, index);
		return false;
	}

	const uint size = (uint)stream->size();
	if (size == 0 || (expectedSize != 0 && size != expectedSize)) {
		warning("%s chunk %u has unexpected size %u", kInventoryMediaArchiveName, index, size);
		return false;
	}

	destination.resize(size);
	if (stream->read(destination.data(), destination.size()) != destination.size()) {
		warning("Failed to read %s chunk %u", kInventoryMediaArchiveName, index);
		return false;
	}

	return true;
}

} // End of namespace Hollywood
