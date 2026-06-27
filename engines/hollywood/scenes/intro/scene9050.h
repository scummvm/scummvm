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

#ifndef HOLLYWOOD_SCENES_INTRO_SCENE9050_H
#define HOLLYWOOD_SCENES_INTRO_SCENE9050_H

#include "common/array.h"
#include "common/random.h"

#include "hollywood/music.h"
#include "hollywood/resource.h"

namespace Hollywood {

class HollywoodEngine;

class Scene9050 {
public:
	Scene9050(HollywoodEngine *vm);

	bool play();

private:
	bool loadResourceI06Assets();
	bool loadResourceI06Chunk(uint index, Common::Array<byte> &destination, uint fixedSize);
	bool loadResourceI06ArenaChunk(uint index);

	void runResourceI06AnimatedPresentation();
	void initializeResourceI06AnimatedPresentation();
	void copyResourceI06ScrolledBaseFrame();
	void presentResourceI06AnimatedFrame();
	void drawResourceI06AnimatedFrame(byte chunkIndex, byte frameIndex);
	void advanceResourceI06Timers(uint32 millis);
	void advanceResourceI06Scroll();
	void advanceResourceI06SecondarySprite();
	void advanceResourceI06PrimarySprite();
	void advanceResourceI06VerticalBob();
	void advanceResourceI06PalettePulse();
	void markResourceI06CompositeDirty();

	void revealSavedFramebufferBand(uint sweepOffset, byte bandWidth);
	void clearSceneFramebufferBand(uint sweepOffset, byte bandWidth);
	void presentFrame();

	bool pollEvents();
	bool delay(uint32 millis);
	void stopAudio();

	enum {
		kFrameBufferSize = 0x78000,
		kResourceChunkCount = 40,
		kI06RequiredChunkCount = 6,
		kI06AnimatedFrameDescriptorCount = 0x1f,
		kI06InitialBaseScrollOffset = 0xc0,
		kI06SequenceDoneFrame = 0x17f,
		kI06FrameCounterWrap = 0x27f
	};

	HollywoodEngine *_vm;
	MusicPlayer _music;
	Common::RandomSource _random;
	ResourceChunkTable _i06ChunkTable;
	uint32 _resourceChunkOffsets[kResourceChunkCount];
	Common::Array<byte> _paletteResource;
	Common::Array<byte> _paletteCurrent;
	Common::Array<byte> _resourceArena;
	Common::Array<byte> _sceneFramebuffer;
	Common::Array<byte> _savedFramebuffer;
	Common::Array<byte> _screen;
	uint32 _resourceArenaCursor;
	uint32 _i06ScrollAccumulator;
	uint32 _i06PrimarySpriteAccumulator;
	uint32 _i06SecondarySpriteAccumulator;
	uint32 _i06VerticalBobAccumulator;
	uint32 _i06PalettePulseAccumulator;
	uint16 _i06FrameCounter;
	uint16 _i06BaseImageScrollOffset;
	uint16 _i06PreviousBaseImageScrollOffset;
	byte _i06PrimarySpriteFrame;
	byte _i06PrimarySpriteSequenceState;
	byte _i06PrimarySpriteSequenceStep;
	byte _i06SecondarySpriteFrame;
	byte _i06PreviousSecondarySpriteFrame;
	byte _i06RandomBasePhase;
	byte _i06PreviousRandomBasePhase;
	byte _i06VerticalBobOffset;
	byte _i06VerticalBobTicksRemaining;
	int _i06VerticalBobDelta;
	byte _i06PalettePulseStepIndex;
	byte _currentMusicCue;
	bool _i06OptionalOverlayChunk5Enabled;
	bool _i06BaseFrameDirty;
	bool _i06PrimarySpriteDirty;
	bool _i06SecondarySpriteDirty;
	bool _i06CompositeForceDirty;
	bool _i06PaletteDirty;
	bool _i06SequenceFinished;
	bool _skipRequested;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_INTRO_SCENE9050_H
