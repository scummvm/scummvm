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

#ifndef HOLLYWOOD_SCENES_INTRO_SCENE9090_H
#define HOLLYWOOD_SCENES_INTRO_SCENE9090_H

#include "common/random.h"

#include "hollywood/gameplay/actor_renderer.h"
#include "hollywood/music.h"
#include "hollywood/scenes/intro/intro_resource_set.h"
#include "hollywood/scenes/intro/intro_scene.h"
#include "hollywood/scenes/intro/intro_text.h"

namespace Hollywood {

class HollywoodEngine;

class Scene9090 : public IntroSceneBase {
public:
	Scene9090(HollywoodEngine *vm);

	bool play();

private:
	// Sue's five speech cels replace her upper body; her full-body bank remains on cel 0.
	struct ActorSpeechDescriptor {
		uint32 runStreamOffset;
		uint32 runCount;
		int16 anchorX;
		int16 anchorY;
	};

	struct ActorSpeechBank {
		Common::Array<byte> runStreams;
		Common::Array<ActorSpeechDescriptor> descriptors;
	};

	struct SubtitleOverlay {
		bool visible;
		byte colorIndex;
		uint16 centerX;
		uint16 topY;
		Common::Array<Common::String> lines;
	};

	bool load();
	bool loadChunk(uint index, Common::Array<byte> &destination, uint fixedSize);
	bool loadChunk(uint index, IndexedSurfaceBuffer &destination, uint fixedSize);
	bool loadArenaChunk(uint index);
	bool loadActorResources();
	void initializeOfficeState();
	void composeFrame();
	void drawOfficePatch(uint chunkIndex);
	void drawDeskActor();
	void drawInsetActor();
	void drawClock();
	void drawSecondaryActor();
	int drawSecondarySpeechFrame();
	void runDialogueSequence();
	void setDeskFacing(byte facingMode);
	void returnDeskActorToForward();
	void animateDeskFrames(byte firstFrame, byte lastFrame);
	void setInsetVariant(byte variant);
	void animateSecondaryTurn(byte facing);
	void runSpeechLine(byte stepIndex);
	void runConcurrentSpeechLines(byte firstStepIndex, byte secondStepIndex);
	void runSpeechSteps(const byte *stepIndices, uint stepCount);
	void advanceDialogueAnimations(uint32 deltaMillis, bool deskSpeaking,
		bool secondarySpeaking, bool insetSpeaking);
	void advanceClock(uint32 deltaMillis);
	byte nextFrameExcluding(byte maximumFrame, byte previousFrame);
	void beginSubtitle(SubtitleOverlay &subtitle, uint16 textRecordId, byte colorIndex,
		uint16 centerX, uint16 topY);
	void clearSubtitles();
	void drawFrameOverlays() override;
	void drawSubtitle(const SubtitleOverlay &subtitle);
	void wrapSubtitleText(const Common::String &text, uint16 anchorSceneX, Common::Array<Common::String> &lines) const;
	uint subtitleTextWidth(const Common::String &text) const;
	void stopAudio() override;

	IntroResourceSet _resources;
	MusicPlayer *_music;
	SpeechPlayer _primarySpeech;
	SpeechPlayer _secondarySpeech;
	SoundBank0Player _clockSound;
	SoundBank0Player _ambientSound;
	IntroTextStore _text;
	Common::RandomSource _random;
	Common::Array<byte> _paletteResource;
	Common::Array<byte> _presentationPaletteRemapTable;
	IndexedSurfaceBuffer _baseFramebuffer;
	ActorSpriteBank _secondaryActorBank;
	ActorSpeechBank _secondarySpeechBank;
	SubtitleOverlay _primarySubtitle;
	SubtitleOverlay _secondarySubtitle;
	byte _deskFrame;
	byte _deskFacingMode;
	byte _insetFrame;
	byte _insetVariant;
	byte _secondaryFacing;
	byte _secondaryCel;
	byte _secondarySpeechFrame;
	byte _clockFrames[3];
	byte _lastDeskTalkFrame;
	byte _lastInsetTalkFrame;
	uint32 _deskFrameAccumulator;
	uint32 _insetFrameAccumulator;
	uint32 _secondaryFrameAccumulator;
	uint32 _clockAccumulator;
	bool _secondarySpeechVisible;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_INTRO_SCENE9090_H
