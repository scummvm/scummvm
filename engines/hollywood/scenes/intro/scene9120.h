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

#ifndef HOLLYWOOD_SCENES_INTRO_SCENE9120_H
#define HOLLYWOOD_SCENES_INTRO_SCENE9120_H

#include "common/array.h"
#include "common/random.h"

#include "hollywood/music.h"
#include "hollywood/scenes/intro/intro_resource_set.h"
#include "hollywood/scenes/intro/intro_scene.h"

namespace Hollywood {

class HollywoodEngine;

class Scene9120 : public IntroSceneBase {
public:
	Scene9120(HollywoodEngine *vm);

	bool play();

private:
	bool loadResourceI12Assets();
	bool loadResourceI12Chunk(uint index, Common::Array<byte> &destination, uint fixedSize);
	bool loadResourceI12Chunk(uint index, IndexedSurfaceBuffer &destination, uint fixedSize);
	bool loadResourceI12ArenaChunk(uint index);

	void runTimedOverlayPhase();
	void runHoldScrollAndIdlePhase();
	void advanceActorBob();
	void advanceSmallAnimation();
	void restoreAndDrawResourceDescriptorFrame(byte localChunkIndex, byte descriptorCount, byte descriptorIndex, bool drawFrame);
	byte getTimedOverlayChunk(uint tickIndex) const;

	void copyViewportToSavedFramebuffer();
	void clearActiveViewport();
	void drawTimedOverlayChunk(uint32 baseOffset);
	void revealSavedViewportCurtainBand(uint sweepOffset, byte bandWidth);
	void clearViewportCurtainBand(uint sweepOffset, byte bandWidth);
	void copySavedRunToScene(int sourceY, int destinationY, int x, int width);
	void clearSceneRun(int destinationY, int x, int width);

	uint presentRowOffset() const override;
	uint presentXOffset() const override;
	void stopAudio() override;

	enum {
		kFrameBufferSize = 0x78000,
		kScene9120FramebufferSize = 0xeb000,
		kI12RequiredChunkCount = 10,
		kScene9120InitialYOffset = 0x1cc,
		kScene9120TimedOverlayTicks = 0x181,
		kScene9120OverlayInterval = 16,
		kScene9120ScrollInterval = 50,
		kScene9120ActorBobInterval = 60,
		kScene9120SmallAnimInterval = 60,
		kScene9120HoldMillis = 13000,
		kScene9120ActorBobDescriptorCount = 10,
		kScene9120SmallAnimDescriptorCount = 3
	};

	MusicPlayer *_music;
	SoundBank0Player _soundBank0;
	Common::RandomSource _random;
	IntroResourceSet _resources;
	Common::Array<byte> _paletteResource;
	IndexedSurfaceBuffer _descriptorBackground;
	uint32 _overlayAccumulator;
	uint32 _scrollAccumulator;
	uint32 _actorBobAccumulator;
	uint32 _smallAnimAccumulator;
	uint16 _xOffset;
	uint16 _yOffset;
	byte _actorBobTicksRemaining;
	byte _actorBobFramePhase;
	int _actorBobDelta;
	byte _smallAnimSequenceState;
	byte _smallAnimFrame;
	bool _viewportDirty;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_INTRO_SCENE9120_H
