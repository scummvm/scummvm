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

#ifndef HOLLYWOOD_SCENES_INTRO_SCENE1000_H
#define HOLLYWOOD_SCENES_INTRO_SCENE1000_H

#include "common/array.h"
#include "common/random.h"

#include "hollywood/music.h"
#include "hollywood/resource.h"

namespace Hollywood {

class HollywoodEngine;

class Scene1000 {
public:
	Scene1000(HollywoodEngine *vm);

	bool play();

private:
	bool load();
	bool loadChunk(uint index, Common::Array<byte> &destination, uint fixedSize);
	bool loadArenaChunk(uint index);
	void runTitleFrontEndSequence();
	void applyTitlePatch();
	void advanceBlinkFrame();
	void renderOverlayFrame(bool forceDirty);
	void restoreSpriteBackground(uint32 baseOffset, uint16 descriptorCount, uint16 descriptorIndex);
	void drawStripSpriteFrame(uint32 baseOffset, uint16 descriptorCount, uint16 descriptorIndex);
	void drawResourceBlockListToSceneFramebuffer(uint32 baseOffset);
	void fadeInPalette();
	void fadeOutPalette();
	void presentFrame();
	bool pollEvents();
	bool delay(uint32 millis);
	uint16 readUint16(const Common::Array<byte> &source, uint offset) const;
	uint32 readUint32(const Common::Array<byte> &source, uint offset) const;

	enum {
		kFrameBufferSize = HollywoodEngine::kSceneBufferWidth * HollywoodEngine::kSceneBufferHeight,
		kPaletteSize = 0x300,
		kFrameDescriptorSize = 14,
		kBlinkDescriptorCount = 0x0c,
		kSecondaryDescriptorCount = 0x47,
		kResourceChunkCount = 40
	};

	HollywoodEngine *_vm;
	MusicPlayer _music;
	Common::RandomSource _random;
	ResourceChunkTable _chunkTable;
	uint32 _resourceChunkOffsets[kResourceChunkCount];
	uint32 _resourceArenaCursor;
	Common::Array<byte> _paletteResource;
	Common::Array<byte> _paletteCurrent;
	Common::Array<byte> _frameDecodeBuffer;
	Common::Array<byte> _sceneFramebuffer;
	Common::Array<byte> _resourceArena;
	Common::Array<byte> _screen;
	uint32 _lastBlinkMillis;
	byte _blinkPatternMode;
	byte _blinkFrameIndex;
	byte _secondaryFrameIndex;
	bool _blinkDirty;
	bool _secondaryDirty;
	bool _secondaryVisible;
	bool _titlePatchApplied;
	bool _skipRequested;
};

} // End of namespace Hollywood

#endif
