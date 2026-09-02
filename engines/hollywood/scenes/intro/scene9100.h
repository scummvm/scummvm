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

#ifndef HOLLYWOOD_SCENES_INTRO_SCENE9100_H
#define HOLLYWOOD_SCENES_INTRO_SCENE9100_H

#include "common/array.h"
#include "common/random.h"
#include "common/str.h"

#include "hollywood/gameplay/actor_renderer.h"
#include "hollywood/music.h"
#include "hollywood/scenes/presentation_scene.h"
#include "hollywood/scenes/scene_text_store.h"

namespace Hollywood {

class HollywoodEngine;

class Scene9100 : public PresentationScene {
public:
	Scene9100(HollywoodEngine *vm);

	bool play();
	bool playDialogueBranch();

	struct SpeechTextStyle {
		uint16 centerX;
		uint16 topY;
		byte colorIndex;
		byte red;
		byte green;
		byte blue;
		bool updatePalette;
	};

private:
	enum TalkingOverlayBase {
		kTalkingOverlayNone = -1,
		kTalkingOverlayBase0 = 0,
		kTalkingOverlayBase320000 = 320000,
		kTalkingOverlayBase640000 = 640000
	};

	struct CinematicStep {
		byte backgroundMode;
		uint16 textBankIndex;
		byte descriptorIndex;
		TalkingOverlayBase talkingOverlayBase;
		byte talkingOverlayVariant;
		bool copyFrameToSavedBefore;
		bool animateForegroundActor;
		SpeechTextStyle speechTextStyle;
	};

	bool load(bool dialogueBranch);
	bool loadActorResources();
	bool loadI10ActorBank(uint runStreamChunkIndex, uint descriptorChunkIndex, ActorSpriteBank &bank);

	void applyActorHighlightColor(byte highlightRed, byte highlightGreen, byte highlightBlue);
	void runEntryActorAnimations();
	void playEntryActorAnimation(const ActorSpriteBank &bank, int worldX, int worldY, IndexedSurfaceBuffer &baseFramebuffer);
	void runRonEntryConversation();
	void drawRonEntryPathFrame(uint32 pathElapsedMillis, uint32 pathDurationMillis,
		bool playFootstep = false);
	void runSueEntrySequence();
	void runSueEntryPath();
	void drawSueEntryPathFrame(uint32 pathElapsedMillis, uint32 pathDurationMillis,
		bool playFootstep = false);
	void drawActorFrame(const ActorSpriteBank &bank, byte facing, byte cel, int worldX, int worldY);
	void playActorFootstepIfDue(int worldX, int worldY, byte cel);
	void runForegroundIdleBeat();
	void runOpeningPrelude();
	void runCinematicSequence();
	void initializeDialogueBranchOfficeState();
	void runDialogueBranchSequence();
	void prepareDialogueBranchOfficePatch();
	void runForegroundPoseToDialogueState();
	void runForegroundPoseBackToDeskIdle();
	void prepareSceneFadeFrameWithScratchActor(byte talkingOverlayVariant);
	void prepareChunkDFrameWithOptionalActorOverlay(bool actorOverlayEnabled);
	void runEndingWipe();
	void runConversationStep(uint16 textBankIndex, byte descriptorIndex, TalkingOverlayBase talkingOverlayBase, byte talkingOverlayVariant, bool animateForegroundActor, bool animateClock, const SpeechTextStyle &speechTextStyle, bool animateInsetActor = false, byte insetTalkBaseFrame = 0);
	void waitForSpeechOrDelay(uint32 fallbackMillis, TalkingOverlayBase talkingOverlayBase, byte talkingOverlayVariant, bool animateForegroundActor, bool animateClock, bool animateInsetActor = false, byte insetTalkBaseFrame = 0);
	void beginSubtitle(const SceneSpeechCue &popup, uint segmentIndex, const SpeechTextStyle &speechTextStyle);

	void drawInitialForegroundFrame();
	void drawForegroundActorFrame(byte frameIndex);
	void restoreForegroundActorLayer();
	void drawForegroundActorLayer();
	void drawDeskActorLayer(uint32 baseOffset, uint16 descriptorCount, byte frameIndex, bool restoreBackground);
	void drawDeskPrimaryStaticFrame(byte frameIndex, bool restoreBackground = true);
	void drawDeskSecondaryStaticFrame(byte frameIndex, bool restoreBackground = true);
	void drawDeskPrimaryStaticLayer(bool restoreBackground);
	void drawDeskSecondaryStaticLayer(bool restoreBackground);
	void drawPersistentDeskActors();
	void drawOfficeCompositeLayers();
	void syncOfficeRestoreBaseFromSaved();
	void animateForegroundFrames(byte firstFrame, byte lastFrame);
	void animateDeskPrimaryStaticFrames(byte firstFrame, byte lastFrame);
	void animateDeskSecondaryStaticFrames(byte firstFrame, byte lastFrame);
	void advanceClockFrame();
	void restoreClockAreaBackground();
	void drawClockLayers(bool restoreBackground);
	void drawTalkingOverlay(TalkingOverlayBase talkingOverlayBase, byte frameIndex, byte talkingOverlayVariant);
	void drawStripSpriteFrame(const Common::Array<byte> &resource, uint32 baseOffset, uint32 descriptorTableOffset, uint16 descriptorCount, uint16 descriptorIndex);
	void restoreSpriteBackground(const Common::Array<byte> &resource, uint32 baseOffset, uint32 descriptorTableOffset, uint16 descriptorCount, uint16 descriptorIndex);
	void applyResourceSpanPatchToFrameDecodeBuffer(uint32 baseOffset);
	void drawResourceBlockListToSceneFramebuffer(uint32 baseOffset);
	void restoreResourceBlockListFromCleanOfficeBase(uint32 baseOffset, IndexedSurfaceBuffer &destination);
	void removeInitialOfficeTitlePatch(IndexedSurfaceBuffer &destination);
	void expandFillRunsToSavedFramebuffer();
	void restoreOfficeFrameAndPresent();
	void applyBackgroundMode(const CinematicStep &step);
	void copyPaletteSegment(byte segmentIndex);
	void copyDefaultPalette();
	bool advanceClockTimer(uint32 millis);
	bool advanceTalkingTimer(uint32 millis);
	bool delayFrame(uint32 millis, TalkingOverlayBase talkingOverlayBase, byte talkingOverlayVariant, bool animateForegroundActor, bool animateClock, bool animateInsetActor = false, byte insetTalkBaseFrame = 0);
	void stopAudio() override;

	byte nextTalkingFrameVariant();
	uint32 getSegmentOffset(byte segmentIndex) const;
	uint16 readUint16(const Common::Array<byte> &source, uint offset) const;
	int16 readSint16(const Common::Array<byte> &source, uint offset) const;
	uint32 readUint32(const Common::Array<byte> &source, uint offset) const;

	enum {
		kFrameDecodeBufferSize = 0x78000,
		kPaletteSize = 0x300,
		kI10ForegroundDescriptorCount = 0x24,
		kI10DeskPrimaryStaticDescriptorCount = 3,
		kI10DeskSecondaryStaticDescriptorCount = 6,
		kI10ClockDescriptorCount = 0x3c,
		kI10TalkingOverlayDescriptorCount = 10,
		kSecondaryScratchBufferSize = 96000,
		kDeskPrimaryStaticBase = 0,
		kDeskSecondaryStaticBase = 48000,
		kScratchChunk21Base = 0x4e200,
		kScratchPrimaryPayloadBase = 640000
	};

	MusicPlayer _music;
	SpeechPlayer _speech;
	SceneTextStore _text;
	SoundBank0Player _effectSound;
	SoundBank0Player _clockSound;
	SoundBank0Player _ambientSound;
	ResidentSoundEffectPlayer _residentSoundEffects;
	Common::RandomSource _random;
	Common::Array<byte> _paletteDefault;
	Common::Array<byte> _scenePaletteMask;
	Common::Array<byte> _sceneFillRuns;
	Common::Array<byte> _resourceScratchArena;
	Common::Array<byte> _secondaryScratchBuffer;
	Common::Array<byte> _presentationPaletteRemapTable;
	IndexedSurfaceBuffer _frameDecodeBuffer;
	IndexedSurfaceBuffer _cleanOfficeBaseFramebuffer;
	ActorSpriteBank _actorBankI10Ron;
	ActorSpriteBank _actorBankI10Sue;
	uint32 _clockFrameAccumulator;
	uint32 _talkingFrameAccumulator;
	byte _foregroundActorFrame;
	byte _foregroundTalkBaseFrame;
	byte _clockChunk7Frame;
	byte _clockChunk8Frame;
	byte _clockChunk9Frame;
	byte _clockChunk7CarryGate;
	byte _talkingFrame;
	byte _lastTalkingFrameVariant;
	byte _deskPrimaryActorFrame;
	byte _deskSecondaryActorFrame;
	bool _clockVisible;
	bool _deskPrimaryActorVisible;
	bool _deskSecondaryActorVisible;
	bool _dialogueBranch;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_INTRO_SCENE9100_H
