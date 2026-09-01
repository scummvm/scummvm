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

#ifndef HOLLYWOOD_SCENES_INTRO_SCENE9110_H
#define HOLLYWOOD_SCENES_INTRO_SCENE9110_H

#include "common/array.h"
#include "common/random.h"
#include "common/str.h"

#include "hollywood/music.h"
#include "hollywood/scenes/presentation_scene.h"
#include "hollywood/scenes/scene_resources.h"
#include "hollywood/scenes/scene_text_store.h"

namespace Hollywood {

class HollywoodEngine;

class Scene9110 : public PresentationScene {
public:
	Scene9110(HollywoodEngine *vm);

	bool play();

private:
	enum SpeechWaitMode {
		kWaitChunk2MouthMotion,
		kWaitChunk3Animation
	};

	struct SpeechTextStyle {
		uint16 centerX;
		uint16 topY;
		byte colorIndex;
		byte red;
		byte green;
		byte blue;
		bool updatePalette;
	};

	struct SpeechStep {
		uint16 rowIndex;
		byte frameIndex;
		SpeechTextStyle speechTextStyle;
		SpeechWaitMode waitMode;
	};

	bool load();
	bool loadResourceI11Assets();
	void initializeCompositeState();
	void runSpeechSequence();
	void runSpeechStep(const SpeechStep &step);
	void waitForSpeechOrDelay(uint32 fallbackMillis, SpeechWaitMode waitMode);
	bool advanceAnimationTimers(uint32 millis, SpeechWaitMode waitMode);
	void drawCompositeToFramebuffer();
	void presentComposite();
	void drawDescriptorFrame(byte localChunkIndex, byte descriptorCount, byte descriptorIndex);
	byte nextMouthFrameVariant();

	void beginSubtitle(const SceneSpeechCue &popup, const SpeechTextStyle &speechTextStyle);

	void stopAudio() override;

	enum {
		kFrameBufferSize = 0x78000,
		kI11RequiredChunkCount = 4,
		kI11Chunk2DescriptorCount = 15,
		kI11Chunk3DescriptorCount = 8,
		kStage911Index = 911
	};

	MusicPlayer *_music;
	SpeechPlayer _speech;
	SceneTextStore _text;
	Common::RandomSource _random;
	Common::Array<byte> _paletteResource;
	IndexedSurfaceBuffer _baseFramebuffer;
	uint32 _mouthAccumulator;
	uint32 _chunk3Accumulator;
	uint32 _idleAccumulator;
	uint32 _cycleAccumulator;
	uint32 _musicFadeAccumulator;
	byte _chunk2MouthFrame;
	byte _chunk2IdleFrame;
	byte _chunk2CycleFrame;
	byte _chunk3Frame;
	byte _chunk2CycleDirection;
	byte _lastMouthVariant;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_INTRO_SCENE9110_H
