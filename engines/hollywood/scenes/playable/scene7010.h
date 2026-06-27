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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SCENE7010_H
#define HOLLYWOOD_SCENES_PLAYABLE_SCENE7010_H

#include "common/array.h"
#include "common/str.h"
#include "common/types.h"

#include "hollywood/gameplay/game_loop.h"
#include "hollywood/music.h"
#include "hollywood/resource.h"

namespace Hollywood {

class HollywoodEngine;

class Scene7010 : public GameplayLoopDelegate {
public:
	Scene7010(HollywoodEngine *vm);

	bool play();

private:
	struct ActiveActorSpriteDescriptor {
		uint32 runStreamOffset;
		uint32 opaqueRunCount;
		uint32 paletteRunCount;
		int16 anchorX;
		int16 anchorY;
		uint16 width;
		uint16 height;
	};

	struct SecondaryActorSpriteDescriptor {
		uint32 runStreamOffset;
		uint32 runCount;
		int16 anchorX;
		int16 anchorY;
	};

	struct SpeechOverlay {
		bool visible;
		byte colorIndex;
		uint16 centerX;
		uint16 topY;
		Common::Array<Common::String> lines;
	};

	enum {
		kFrameBufferSize = 0x78000,
		kG01InitialRequiredChunkCount = 19,
		kG01ArenaFirstChunk = 5,
		kG01ArenaLastChunk = 18,
		kG01PaletteMaskUsedBytes = 0x100,
		kResource000TableByteCount = 400,
		kResource000ActorSet00TableEntry = 0xd0,
		kResource000Owner1PaletteTableEntry = 0x108,
		kResource000ActorSet00SegmentCount = 14,
		kActorFacingCount = 6,
		kActorCelsPerFacing = 13,
		kActiveActorDescriptorSize = 28,
		kActiveActorFacingRunStride = 160000,
		kSecondaryActorDescriptorSize = 16,
		kSecondaryActorFramesPerFacing = 5,
		kSecondaryActorFacingRunStride = 16000,
		kOwner1ActorPaletteBytes = 0x90,
		kStage003DecodeKeySize = 0x141,
		kStage003StageOffsetTableSize = 0xff4,
		kStage003DescriptorTableSize = 0x186a0,
		kStage003SmallRowSize = 0x29,
		kStage003LargeRowSize = 0x141,
		kStage003LargeRowBaseIndex = 500,
		kG01StageIndex = 701,
		kOriginalSpeechLineHeight = 20
	};

	bool load();
	bool loadResource000RuntimeTables(Common::Array<byte> &offsetTable, Common::Array<byte> &sizeTable);
	bool loadResource000ActorBankSet00(const Common::Array<byte> &offsetTable, const Common::Array<byte> &sizeTable);
	bool loadResource000Owner1ActorPalette(const Common::Array<byte> &offsetTable);
	bool loadStage003SceneRows();
	bool loadFixedChunk(uint index, Common::Array<byte> &destination, uint fixedSize);
	bool loadVariableChunk(uint index, Common::Array<byte> &destination);
	bool loadArenaChunk(uint index);
	void expandFillRunsToSavedFramebuffer();
	void initializePreviewState();
	void drawPreviewComposite();
	void drawCutsceneComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY);
	void drawPlayableComposite();
	void drawActiveActorFrame(byte facing, byte cel, int worldX, int worldY);
	void drawSecondaryActorFrame(byte facing, byte frame, int worldX, int worldY);
	void drawActorRun(const Common::Array<byte> &runStreams, uint cursor, uint runBase, uint runCount, int spriteX, int spriteY);
	void runEntryCutscene();
	void runSueEntryPath();
	void runJuniorSpeech();
	bool runBasicGameplayLoop();
	const SceneHotspotTable &hotspots() const override;
	const Common::Array<byte> &savedFramebuffer() const override;
	uint16 viewportXOffset() const override;
	uint16 viewportYOffset() const override;
	void prepareGameplayLoop() override;
	void advanceGameplayLoop(uint32 delta) override;
	void drawGameplayFrame() override;
	void presentGameplayFrame(const SceneHoverCaption &hoverCaption) override;
	bool shouldExitGameplayLoop() const override;
	void handleLeftClick(const GameplayLoopCursorState &state) override;
	void advanceChunk8Cycle();
	void advanceChunk10IdleFrames();
	void advanceSecondaryActorIdleFrame();
	void advanceDialogueOverlay(uint32 delta);
	void processSceneActionClick(const GameplayLoopCursorState &state);
	void dispatchSceneAction(uint16 handlerId);
	void walkActiveActorTo(int targetX, int targetY, byte finalFacing, byte finalCel);
	void adjustWalkTargetToFloorMask(int &targetX, int &targetY) const;
	byte calculateFacingTowardPoint(int fromX, int fromY, int toX, int toY) const;
	void applySceneStateToHotspotsAndPatches(byte selector);
	bool hasInventoryItem(byte itemId) const;
	void addInventoryItem(byte itemId);
	void removeInventoryItem(byte itemId);
	void handleActionSlot00TransitionToG03();
	void handleActionSlot02SecondarySpeech();
	void handleActionSlot03DialogueSequence();
	void handleActionSlot04Item06Speech();
	void handleActionSlot06Item0BSequence();
	void handleActionSlot07DialogueAndReturn();
	void handleActionSlot08CommonSpeech();
	void runChunk8RevealSequence();
	void runChunk8HideSequence();
	void runChunk11FrameRange(byte startFrame, byte endFrame);
	void runChunk14FrameRange(byte startFrame, byte endFrame);
	void runChunk15ItemSequence();
	void runDialogueOverlayFrames(byte startFrame, byte endFrame, byte finalMode);
	bool waitSceneMillis(uint32 millis);
	void beginJuniorSpeech();
	void clearSpeechOverlay();
	void clearAllSpeechOverlays();
	void drawSpeechOverlay();
	void drawSpeechOverlay(const SpeechOverlay &overlay);
	void drawMappedSpriteFrame(uint chunkIndex, uint descriptorCount, const byte *frameMap, uint frameMapSize, byte frameIndex);
	void beginSecondarySpeechLine(uint16 rowIndex, byte frameIndex);
	void beginPrimarySpeechLine(uint16 rowIndex, byte frameIndex, uint16 centerX, uint16 topY,
		byte red, byte green, byte blue);
	void runSpeechLine(SpeechOverlay &overlay, uint16 rowIndex, byte frameIndex, uint16 centerX, uint16 topY,
		byte colorIndex, bool useRequestedTop);
	bool getStage003Cue(uint16 rowIndex, byte frameIndex, uint16 &textRecordId, byte &continuationCount,
		uint16 &voiceSampleId) const;
	void wrapActorSpeechText(const Common::String &text, uint16 anchorSceneX, Common::Array<Common::String> &lines) const;
	Common::String getStage003LargeTextRecord(uint16 recordId) const;
	uint actorSpeechTextWidth(const Common::String &text) const;
	void calculateSecondarySpeechBounds(int actorWorldX, int actorWorldY);
	bool waitForSpeechOrDelay(uint32 fallbackMillis);
	void presentFrame(const SceneHoverCaption *hoverCaption = nullptr);
	bool pollEvents(bool allowSkip);
	bool delay(uint32 millis);

	HollywoodEngine *_vm;
	ResourceChunkTable _g01ChunkTable;
	uint32 _resourceChunkOffsets[HollywoodEngine::kResourceChunkCount];
	uint32 _resourceArenaCursor;

	Common::Array<byte> _paletteResource;
	Common::Array<byte> _paletteCurrent;
	Common::Array<byte> _baseFramebuffer;
	Common::Array<byte> _sceneFramebuffer;
	Common::Array<byte> _savedFramebuffer;
	Common::Array<byte> _fillRuns;
	Common::Array<byte> _paletteMask;
	Common::Array<byte> _metadata;
	Common::Array<byte> _resourceArena;
	Common::Array<byte> _screen;
	Common::Array<byte> _resource000OffsetTable;
	Common::Array<byte> _resource000SizeTable;
	Common::Array<byte> _activeActorRunStreams;
	Common::Array<byte> _secondaryActorRunStreams;
	Common::Array<ActiveActorSpriteDescriptor> _activeActorDescriptors;
	Common::Array<SecondaryActorSpriteDescriptor> _secondaryActorDescriptors;
	Common::Array<byte> _stage003DecodeKey;
	Common::Array<byte> _stage003StageBlock;
	Common::Array<byte> _stage003SmallRows;
	Common::Array<byte> _stage003LargeRows;
	SceneHotspotTable _hotspots;
	SpeechPlayer _speech;
	SpeechOverlay _speechOverlay;
	SpeechOverlay _primarySpeechOverlay;

	bool _inventoryItems[121];
	byte _sceneStateFlags[8];
	byte _chunk8FrameIndex;
	byte _chunk9AmbientOverlayFrameIndex;
	byte _chunk10IdleFrameA;
	byte _chunk10IdleFrameB;
	byte _chunk10IdleFrameC;
	byte _chunk10IdleFrameD;
	byte _chunk11FrameIndex;
	byte _chunk14FrameIndex;
	byte _chunk15FrameIndex;
	byte _dialogueOverlayFrameIndex;
	byte _dialogueOverlayMode;
	bool _chunk11Visible;
	bool _chunk14Visible;
	bool _chunk15Visible;
	bool _chunk10IdlePairAAltPhase;
	bool _chunk10IdlePairBAltPhase;
	byte _chunk10IdlePairATicksRemaining;
	byte _chunk10IdlePairBTicksRemaining;
	byte _chunk9AmbientDecisionCounter;
	byte _chunk10DeterministicCounter;
	uint32 _chunk8TimerAccumulator;
	uint32 _chunk10TimerAccumulator;
	uint32 _secondaryActorTimerAccumulator;
	uint32 _dialogueOverlayTimerAccumulator;
	uint16 _secondaryActorIdleTick;
	int _activeActorWorldX;
	int _activeActorWorldY;
	byte _activeActorFacing;
	byte _activeActorCel;
	byte _secondaryActorFrame;
	bool _skipRequested;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE7010_H
