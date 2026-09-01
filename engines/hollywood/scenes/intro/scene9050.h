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
#include "hollywood/scenes/scene_resources.h"
#include "hollywood/scenes/presentation_scene.h"

namespace Hollywood {

class HollywoodEngine;

class Scene9050 : public PresentationScene {
public:
	Scene9050(HollywoodEngine *vm);

	bool play();

private:
	enum BlockingAnimationMode {
		kNoBlockingAnimation,
		kResourceI05ClipAnimation,
		kInterClipRevealAnimation,
		kInterClipReverseAnimation,
		kResourceI08BlinkAnimation,
		kResourceI07FinalAnimation
	};

	bool loadResourceI06Assets();
	bool loadResourceI05ClipSegment(byte segmentId);
	bool loadResourceI08BlinkAssets();
	bool loadResourceI07FinalAssets();

	void runResourceI06AnimatedPresentation();
	void initializeResourceI06AnimatedPresentation();
	bool runResourceI06ScrollInterlude();
	bool runResourceI06SpriteInterlude();
	bool runResourceI06Interlude(bool runScriptedSpriteSequence);
	void initializeResourceI06Interlude();
	void copyResourceI06ScrolledBaseFrame();
	void presentResourceI06AnimatedFrame();
	void drawResourceI06AnimatedFrame(byte chunkIndex, byte frameIndex);
	void advanceResourceI06Timers(uint32 millis);
	void advanceResourceI06InterludeTimers(uint32 millis, bool runScriptedSpriteSequence);
	void advanceResourceI06Scroll();
	void advanceResourceI06InterludeScroll(bool runScriptedSpriteSequence);
	void advanceResourceI06SecondarySprite();
	void advanceResourceI06PrimarySprite();
	void advanceResourceI06InterludePrimarySprite(bool runScriptedSpriteSequence);
	void advanceResourceI06VerticalBob();
	void advanceResourceI06PalettePulse();
	void markResourceI06CompositeDirty();
	bool runResourceI06AnimationLoop(bool interlude, bool runScriptedSpriteSequence);
	void ensureContinuousSound(byte cueId, byte volumePercent);
	void stopContinuousSound();

	void runResourceI05Clip(byte segmentId, byte lastFrameIndex, bool fadeInBeforePlayback);
	void drawResourceI05ClipFrameDelta(byte lastFrameIndex, byte frameIndex);
	bool waitResourceI05ClipHold();
	bool playResourceI05ClipSegment(byte segmentId, byte lastFrameIndex, bool fadeInBeforePlayback);
	void runStage9050InterClipSpriteReveal();
	void advanceStage9050Cutscene();
	void runResourceI05InterClipRevealPhase(byte localChunkIndex);
	void runResourceI05InterClipReversePhase();
	void restoreAndDrawResourceDescriptorFrame(byte localChunkIndex, byte descriptorCount, byte descriptorIndex, bool drawFrame);
	bool runResourceI08BlinkSequence();
	bool waitResourceI08BlinkLoop(uint32 millis);
	bool runResourceI07FinalAnimation();
	bool waitSceneCounterPast(uint threshold);
	bool playBlockingAnimation(BlockingAnimationMode mode, byte firstFrame,
		byte lastFrame, uint32 frameMillis, byte chunkIndex = 0,
		bool waitAfterFinalFrame = false);
	void presentAnimationFrame() override;
	bool waitForAnimationFrame(uint32 millis, bool allowSkip) override;

	void stopAudio() override;

	enum {
		kI06RequiredChunkCount = 6,
		kI08RequiredChunkCount = 3,
		kI07RequiredChunkCount = 3,
		kI06AnimatedFrameDescriptorCount = 0x1f,
		kI05InterClipFrameDescriptorCount = 10,
		kI08BlinkFrameDescriptorCount = 2,
		kI07FinalFrameDescriptorCount = 0x15,
		kI06InitialBaseScrollOffset = 0xc0,
		kI06SequenceDoneFrame = 0x17f,
		kI06InterludeStartFrame = 0x80,
		kI06InterludeDoneFrame = 0x1ff,
		kI06FrameCounterWrap = 0x27f
	};

	MusicPlayer _music;
	SoundBank0Player _continuousSound;
	SoundBank0Player _effectSound;
	Common::RandomSource _random;
	Common::Array<byte> _paletteResource;
	IndexedSurfaceBuffer _clipBaseFramebuffer;
	uint32 _i05ClipChunkSize;
	byte _i05ClipFrameCount;
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
	byte _continuousSoundCue;
	byte _i05EntriesPerSegment;
	BlockingAnimationMode _blockingAnimationMode;
	byte _blockingAnimationFrame;
	byte _blockingAnimationChunk;
	bool _i06OptionalOverlayChunk5Enabled;
	bool _i06BaseFrameDirty;
	bool _i06PrimarySpriteDirty;
	bool _i06SecondarySpriteDirty;
	bool _i06CompositeForceDirty;
	bool _i06PaletteDirty;
	bool _i06SequenceFinished;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_INTRO_SCENE9050_H
