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
#include "common/file.h"
#include "common/random.h"

#include "hollywood/music.h"
#include "hollywood/resource.h"

namespace Hollywood {

class HollywoodEngine;

class Scene9100 {
public:
	Scene9100(HollywoodEngine *vm);

	bool play();

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
	};

	struct PopupDescriptor {
		uint16 textRecordId;
		byte continuationCount;
		uint16 voiceSampleId;
	};

	struct ActorSpriteDescriptor {
		uint32 runStreamOffset;
		uint32 opaqueRunCount;
		uint32 paletteRunCount;
		int16 anchorX;
		int16 anchorY;
		uint16 width;
		uint16 height;
	};

	struct ActorBank {
		Common::Array<byte> runStreams;
		Common::Array<ActorSpriteDescriptor> descriptors;
	};

	bool load();
	bool loadChunk(uint index, Common::Array<byte> &destination, uint fixedSize);
	bool loadVariableChunk(uint index, Common::Array<byte> &destination);
	bool loadArenaChunk(uint index);
	bool loadScratchChunk(uint index, uint32 destinationOffset);
	bool loadScratchChunkTo(uint index, Common::Array<byte> &destination, uint32 destinationOffset);
	bool loadStage003Descriptors();
	bool loadActorResources();
	bool loadI10ActorBank(uint runStreamChunkIndex, uint descriptorChunkIndex, ActorBank &bank);

	void applyActorHighlightColor(byte highlightRed, byte highlightGreen, byte highlightBlue);
	void runEntryActorAnimations();
	void showSueEntryActor();
	void playEntryActorAnimation(const ActorBank &bank, int worldX, int worldY, Common::Array<byte> &baseFramebuffer);
	void runRonEntryConversation();
	void drawRonEntryPathFrame(uint32 pathElapsedMillis, uint32 pathDurationMillis);
	void runSueEntrySequence();
	void runSueEntryPath();
	void drawSueEntryPathFrame(uint32 pathElapsedMillis, uint32 pathDurationMillis);
	void drawActorSpriteFrame(const ActorBank &bank, byte facing, byte cel, int worldX, int worldY);
	void runOpeningPrelude();
	void runCinematicSequence();
	void runEndingWipe();
	void runConversationStep(uint16 textBankIndex, byte descriptorIndex, TalkingOverlayBase talkingOverlayBase, byte talkingOverlayVariant, bool animateForegroundActor, bool animateClock, bool animateInsetActor = false, byte insetTalkBaseFrame = 0);
	void waitForSpeechOrDelay(uint32 fallbackMillis, TalkingOverlayBase talkingOverlayBase, byte talkingOverlayVariant, bool animateForegroundActor, bool animateClock, bool animateInsetActor = false, byte insetTalkBaseFrame = 0);

	void drawInitialForegroundFrame();
	void drawForegroundActorFrame(byte frameIndex);
	void drawDeskStaticActorFrame(uint32 baseOffset, uint16 descriptorCount, byte frameIndex, bool restoreBackground = true);
	void drawDeskPrimaryStaticFrame(byte frameIndex, bool restoreBackground = true);
	void drawDeskSecondaryStaticFrame(byte frameIndex, bool restoreBackground = true);
	void drawPersistentDeskActors();
	void animateForegroundFrames(byte firstFrame, byte lastFrame);
	void animateDeskPrimaryStaticFrames(byte firstFrame, byte lastFrame);
	void animateDeskSecondaryStaticFrames(byte firstFrame, byte lastFrame);
	void drawClockFrame(byte frameIndex);
	void drawTalkingOverlay(TalkingOverlayBase talkingOverlayBase, byte frameIndex, byte talkingOverlayVariant);
	void drawStripSpriteFrame(const Common::Array<byte> &resource, uint32 baseOffset, uint32 descriptorTableOffset, uint16 descriptorCount, uint16 descriptorIndex);
	void restoreSpriteBackground(const Common::Array<byte> &resource, uint32 baseOffset, uint32 descriptorTableOffset, uint16 descriptorCount, uint16 descriptorIndex);
	void drawResourceBlockListToBuffer(uint32 baseOffset, Common::Array<byte> &destination);
	void expandFillRunsToSavedFramebuffer();
	void restoreOfficeFrameAndPresent();
	void applyBackgroundMode(const CinematicStep &step);
	void copyPaletteSegment(byte segmentIndex);
	void copyDefaultPalette();
	void revealSavedFramebufferBand(uint sweepOffset, byte bandWidth);
	void clearSceneFramebufferBand(uint sweepOffset, byte bandWidth);
	void copySavedFramebufferRun(int y, int x, int width);
	void clearSceneFramebufferRun(int y, int x, int width);
	void presentFrame();

	bool pollEvents();
	bool delay(uint32 millis);
	bool delayFrame(uint32 millis, TalkingOverlayBase talkingOverlayBase, byte talkingOverlayVariant, bool animateForegroundActor, bool animateClock, bool animateInsetActor = false, byte insetTalkBaseFrame = 0);
	void stopAudio();

	byte nextTalkingFrameVariant();
	uint32 getSegmentOffset(byte segmentIndex) const;
	PopupDescriptor getStage003PopupDescriptor(uint16 textBankIndex, byte descriptorIndex) const;
	uint16 readUint16(const Common::Array<byte> &source, uint offset) const;
	int16 readSint16(const Common::Array<byte> &source, uint offset) const;
	uint32 readUint32(const Common::Array<byte> &source, uint offset) const;

	static const uint kFrameDecodeBufferSize = 0x78000;
	static const uint kPaletteSize = 0x300;
	static const uint kResourceChunkCount = 40;
	static const uint kI10ForegroundDescriptorCount = 0x24;
	static const uint kI10DeskPrimaryStaticDescriptorCount = 3;
	static const uint kI10DeskSecondaryStaticDescriptorCount = 6;
	static const uint kI10ClockDescriptorCount = 0x3c;
	static const uint kI10TalkingOverlayDescriptorCount = 10;
	static const uint kFrameDescriptorSize = 14;
	static const uint kStage003DescriptorTableSize = 0x186a0;
	static const uint kSecondaryScratchBufferSize = 96000;
	static const uint kDeskPrimaryStaticBase = 0;
	static const uint kDeskSecondaryStaticBase = 48000;
	static const uint kScratchChunk21Base = 0x4e200;
	static const uint kScratchPrimaryPayloadBase = 640000;

	HollywoodEngine *_vm;
	MusicPlayer _music;
	SpeechPlayer _speech;
	Common::RandomSource _random;
	ResourceChunkTable _i10ChunkTable;
	uint32 _resourceChunkOffsets[kResourceChunkCount];
	Common::Array<byte> _paletteDefault;
	Common::Array<byte> _paletteCurrent;
	Common::Array<byte> _sceneFillRuns;
	Common::Array<byte> _resourceArena;
	Common::Array<byte> _resourceScratchArena;
	Common::Array<byte> _secondaryScratchBuffer;
	Common::Array<byte> _frameDecodeBuffer;
	Common::Array<byte> _sceneFramebuffer;
	Common::Array<byte> _savedFramebuffer;
	Common::Array<byte> _screen;
	Common::Array<byte> _stage003Descriptors;
	ActorBank _actorBankI10Ron;
	ActorBank _actorBankI10Sue;
	uint32 _resourceArenaCursor;
	uint32 _lastClockFrameMillis;
	uint32 _lastTalkingFrameMillis;
	byte _foregroundActorFrame;
	byte _foregroundTalkBaseFrame;
	byte _clockFrame;
	byte _talkingFrame;
	byte _lastTalkingFrameVariant;
	byte _deskPrimaryActorFrame;
	byte _deskSecondaryActorFrame;
	bool _deskPrimaryActorVisible;
	bool _deskSecondaryActorVisible;
	bool _skipRequested;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_INTRO_SCENE9100_H
