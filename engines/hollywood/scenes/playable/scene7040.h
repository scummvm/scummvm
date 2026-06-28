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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SCENE7040_H
#define HOLLYWOOD_SCENES_PLAYABLE_SCENE7040_H

#include "common/array.h"
#include "common/random.h"
#include "common/str.h"
#include "common/types.h"

#include "hollywood/gameplay/game_loop.h"
#include "hollywood/gameplay/panel_art.h"
#include "hollywood/music.h"
#include "hollywood/resource.h"

namespace Graphics {
struct Surface;
}

namespace Hollywood {

class HollywoodEngine;

class Scene7040 : public GameplayLoopDelegate {
public:
	Scene7040(HollywoodEngine *vm);

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

	struct ActorPathFrame {
		byte drawOrderMode;
		byte facing;
		byte cel;
		int16 worldX;
		int16 worldY;
	};

	struct ActorPathBuildState {
		byte drawOrderMode;
		byte facing;
		byte cel;
		int x;
		int y;
	};

	enum {
		kFrameBufferSize = 0x78000,
		kG04InitialRequiredChunkCount = 20,
		kG04ArenaFirstChunk = 5,
		kG04ArenaLastChunk = 18,
		kG04PaletteMaskUsedBytes = 0x100,
		kScenePaletteRegionCount = 21,
		kScenePaletteRegionBoundaryCandidateCount = 3,
		kScenePaletteRegionRouteStepCount = 19,
		kSceneRouteBoundaryPointCount = kScenePaletteRegionCount * kScenePaletteRegionCount * kScenePaletteRegionBoundaryCandidateCount,
		kSceneRouteStepCount = kScenePaletteRegionCount * kScenePaletteRegionCount * kScenePaletteRegionRouteStepCount,
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
		kOwner1Resource003RowsOffsetIndex = 0x32,
		kOwner1SpeechCueDescriptorTableOffset = 0x5f58,
		kOwner1SpeechCueDescriptorTableSize = 20000,
		kStage003SmallRowSize = 0x29,
		kStage003LargeRowSize = 0x141,
		kStage003LargeRowBaseIndex = 500,
		kG04StageIndex = 704,
		kOriginalSpeechLineHeight = 20
	};

	bool load();
	bool loadResource000RuntimeTables(Common::Array<byte> &offsetTable, Common::Array<byte> &sizeTable);
	bool loadResource000ActorBankSet00(const Common::Array<byte> &offsetTable, const Common::Array<byte> &sizeTable);
	bool loadResource000Owner1ActorPalette(const Common::Array<byte> &offsetTable);
	bool loadResource000InventoryActionTables(const Common::Array<byte> &offsetTable);
	bool loadStage003SceneRows();
	bool loadFixedChunk(uint index, Common::Array<byte> &destination, uint fixedSize);
	bool loadVariableChunk(uint index, Common::Array<byte> &destination);
	bool loadArenaChunk(uint index);
	bool initializeActorDepthTables();
	void updateActorDepthThresholds(byte actorDrawOrderMode);
	void expandFillRunsToSavedFramebuffer();
	bool initializeScenePathTables();
	void initializePreviewState();
	void drawPreviewComposite();
	void drawCutsceneComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode = 0);
	void drawActionOverlayComposite();
	void drawPlayableComposite();
	void drawActiveActorFrame(byte facing, byte cel, int worldX, int worldY, int minimumYExclusive);
	int drawSecondaryActorFrame(byte facing, byte frame, int worldX, int worldY);
	int drawActorRun(const Common::Array<byte> &runStreams, uint cursor, uint runBase, uint runCount,
		int spriteX, int spriteY, int minimumYExclusive, int actorWorldY);
	void runEntryCutscene();
	void runEntryPath(int startX, int startY, byte startFacing, int targetX, int targetY);
	bool runBasicGameplayLoop();
	const SceneHotspotTable &hotspots() const override;
	const Common::Array<byte> &savedFramebuffer() const override;
	uint16 viewportXOffset() const override;
	uint16 viewportYOffset() const override;
	void prepareGameplayLoop() override;
	void advanceGameplayLoop(uint32 delta) override;
	void drawGameplayFrame() override;
	void presentGameplayFrame(const SceneHoverCaption &hoverCaption, const GameplayPanelState &panelState) override;
	void prepareOptionsMenuPalette(Common::Array<byte> &palette) const override;
	bool shouldExitGameplayLoop() const override;
	Common::String inventoryItemName(byte owner, byte itemId) const override;
	void handleLeftClick(const GameplayLoopCursorState &state) override;
	void handleInventoryItemClick(const GameplayLoopCursorState &state) override;
	void updateAmbientAudioAndMusicCues(uint32 delta);
	void advanceChunk5AmbientOverlay();
	void advanceChunk6IdleFrames();
	void advanceChunk11PreItemIdleAnimation(uint32 delta);
	void advanceChunk16PostItemAnimation(uint32 delta);
	void advanceSecondaryActorSpeechFrame();
	void advancePrimaryLeftSpeechFrame();
	void processSceneActionClick(const GameplayLoopCursorState &state);
	void processSceneRelationClick(const GameplayLoopCursorState &state, byte itemId);
	SceneVerbActionRecord relationActionRecord(byte inventoryItemId, byte sceneItemId, byte relationMode) const;
	void dispatchSceneAction(uint16 handlerId);
	void walkActiveActorTo(int targetX, int targetY, byte finalFacing, byte finalCel);
	void adjustWalkTargetToFloorMask(int &targetX, int &targetY) const;
	void queueActorPathWithPaletteRegionRouting(int startX, int startY, int targetX, int targetY,
		byte finalFacing, byte finalCel);
	void buildActorPathFramesBetweenPoints(ActorPathBuildState &state, int targetX, int targetY,
		byte finalFacing, byte finalCel, int requestedFacing);
	void appendActorPathFrame(const ActorPathBuildState &state);
	ScenePoint nearestPaletteRouteBoundaryPoint(int startX, int startY, byte currentRegion, byte nextRegion) const;
	ScenePoint bestPaletteRouteBoundaryPoint(int startX, int startY, int targetX, int targetY,
		byte currentRegion, byte targetRegion) const;
	byte paletteRegionAt(int x, int y) const;
	byte calculateMovementFacingForPath(int fromX, int fromY, int toX, int toY, int requestedFacing) const;
	uint calculateWalkStepCountForAxisDelta(int startAxis, int targetAxis, byte facing, byte cel) const;
	byte nextActorPathCel(byte cel) const;
	uint actorPathStepDelta(byte facing, byte cel) const;
	byte calculateFacingTowardPoint(int fromX, int fromY, int toX, int toY) const;
	void applySceneStateToHotspotsAndPatches(byte selector);
	void rebuildWalkablePaletteMask();
	bool hasInventoryItem(byte itemId) const;
	void addInventoryItem(byte itemId);
	void removeInventoryItem(byte itemId);
	void handleActionSlot00ReturnToG03();
	void handleActionSlot01ProgressSpeech();
	void handleActionSlot02MajorHotspotAction();
	void handleActionSlot03TransitionToState7060();
	void handleActionSlot05ExitProgressSpeech();
	void handleActionSlot06TransitionToG05();
	void handleActionSlot09PickupItem0FThenExit();
	void handleActionSlot10CommonSpeech();
	void handleActionHandler312ProgressSpeech();
	void handleActionHandler313ConversationGate();
	void handleActionHandler314Item0BSpeech();
	void runMappedActionOverlay(uint chunkIndex, uint descriptorCount, const byte *frameMap, uint frameMapSize,
		uint32 frameMillis, int statePatchFrame = -1);
	void runMappedActionOverlay(uint chunkIndex, uint descriptorCount, const byte *frameMap, uint frameMapSize,
		uint32 frameMillis, int statePatchFrame, bool hideActiveActor);
	bool waitSceneMillis(uint32 millis);
	void clearSpeechOverlay();
	void clearAllSpeechOverlays();
	void drawSpeechOverlay();
	void drawSpeechOverlay(const SpeechOverlay &overlay);
	void drawMappedSpriteFrame(uint chunkIndex, uint descriptorCount, const byte *frameMap, uint frameMapSize, byte frameIndex);
	void beginSecondarySpeechLine(uint16 rowIndex, byte frameIndex);
	bool startSecondarySpeechLine(uint16 rowIndex, byte frameIndex);
	void beginStaticSecondarySpeechLine(uint16 rowIndex, byte frameIndex);
	void beginPrimarySpeechLine(uint16 rowIndex, byte frameIndex, uint16 centerX, uint16 topY,
		byte red, byte green, byte blue);
	void beginPrimaryLeftSpeechLine(uint16 rowIndex, byte frameIndex);
	void runSpeechLine(SpeechOverlay &overlay, uint16 rowIndex, byte frameIndex, uint16 centerX, uint16 topY,
		byte colorIndex, bool useRequestedTop, bool animatePrimaryLeft);
	void runSpeechCue(SpeechOverlay &overlay, uint16 textRecordId, byte continuationCount, uint16 voiceSampleId,
		uint16 centerX, uint16 topY, byte colorIndex, bool useRequestedTop, bool animatePrimaryLeft);
	bool getStage003Cue(uint16 rowIndex, byte frameIndex, uint16 &textRecordId, byte &continuationCount,
		uint16 &voiceSampleId) const;
	bool getStaticSpeechCue(uint16 rowIndex, byte frameIndex, uint16 &textRecordId, byte &continuationCount,
		uint16 &voiceSampleId) const;
	void wrapActorSpeechText(const Common::String &text, uint16 anchorSceneX, Common::Array<Common::String> &lines) const;
	Common::String getResource003LargeTextRecord(uint16 recordId) const;
	uint actorSpeechTextWidth(const Common::String &text) const;
	void calculateSecondarySpeechBounds(int actorWorldX, int actorWorldY);
	bool waitForSpeechOrDelay(uint32 fallbackMillis, bool animatePrimaryLeft);
	void applyGameplayPanelPalette();
	void drawGameplayPanel(Graphics::Surface &surface, const GameplayPanelState &panelState);
	void drawVerbPanel(Graphics::Surface &surface, const GameplayPanelState &panelState);
	void drawInventoryPanel(Graphics::Surface &surface, const GameplayPanelState &panelState);
	void presentFrame(const SceneHoverCaption *hoverCaption = nullptr, const GameplayPanelState *panelState = nullptr);
	bool pollEvents(bool allowSkip);
	bool delay(uint32 millis);

	HollywoodEngine *_vm;
	ResourceChunkTable _g04ChunkTable;
	uint32 _resourceChunkOffsets[HollywoodEngine::kResourceChunkCount];
	uint32 _resourceArenaCursor;

	Common::Array<byte> _paletteResource;
	Common::Array<byte> _paletteCurrent;
	Common::Array<byte> _baseFramebufferOriginal;
	Common::Array<byte> _baseFramebuffer;
	Common::Array<byte> _sceneFramebuffer;
	Common::Array<byte> _savedFramebuffer;
	Common::Array<byte> _fillRuns;
	Common::Array<byte> _paletteMaskOriginal;
	Common::Array<byte> _paletteMask;
	Common::Array<byte> _fullPaletteRegionMask;
	Common::Array<byte> _walkablePaletteMask;
	Common::Array<byte> _colorToActorDepthClassMap;
	Common::Array<uint16> _actorDepthYThresholds;
	Common::Array<uint16> _drawActorDepthYThresholds;
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
	Common::Array<byte> _owner1SpeechCueDescriptors;
	Common::Array<byte> _owner1SmallRows;
	Common::Array<byte> _owner1LargeRows;
	Common::Array<ScenePoint> _routeBoundaryPoints;
	Common::Array<byte> _routeSteps;
	Common::Array<ActorPathFrame> _actorPathFrames;
	Common::Array<byte> _actorPathStepDeltas;
	GameplayPanelArt _panelArt;
	SceneHotspotTable _hotspots;
	SpeechPlayer _speech;
	SoundBank0Player _soundBank0;
	SoundBank0Player _ambientSoundBank0;
	SpeechOverlay _speechOverlay;
	SpeechOverlay _primarySpeechOverlay;
	Common::RandomSource _random;

	bool _inventoryItems[121];
	byte _sceneStateFlags[8];
	byte _chunk5FrameIndex;
	byte _chunk6IdleFrameA;
	byte _chunk6IdleFrameB;
	byte _chunk6IdleFrameC;
	byte _chunk6IdleFrameD;
	byte _primaryLeftSpeechLastFrame;
	bool _chunk6IdlePairAAltPhase;
	bool _chunk6IdlePairBAltPhase;
	bool _primaryLeftSpeechActive;
	byte _chunk6IdlePairATicksRemaining;
	byte _chunk6IdlePairBTicksRemaining;
	byte _chunk9AmbientDecisionCounter;
	int _chunk5FrameDirection;
	uint32 _chunk5TimerAccumulator;
	uint32 _chunk6TimerAccumulator;
	uint32 _chunk5FrameMillis;
	uint32 _ambientMusicTimerAccumulator;
	uint32 _secondaryActorTimerAccumulator;
	uint32 _primaryLeftSpeechTimerAccumulator;
	byte _previousAmbientMusicTrackId;
	int _activeActorWorldX;
	int _activeActorWorldY;
	byte _activeActorFacing;
	byte _activeActorCel;
	byte _activeActorDrawOrderMode;
	byte _secondaryActorFrame;
	bool _actionOverlayVisible;
	byte _actionOverlayChunkIndex;
	byte _actionOverlayDescriptorCount;
	byte _actionOverlayFrameIndex;
	byte _chunk11FrameIndex;
	byte _chunk12FrameIndex;
	byte _chunk14ActionFrameIndex;
	byte _chunk14AltFrameIndex;
	byte _chunk16FrameIndex;
	byte _chunk17FrameIndex;
	byte _preItemIdleState;
	byte _postItemIdleState;
	bool _chunk12OverlayVisible;
	bool _chunk14ActionVisible;
	bool _chunk14AltVisible;
	bool _hideActiveActor;
	uint32 _chunk11TimerAccumulator;
	uint32 _chunk12TimerAccumulator;
	uint32 _chunk16TimerAccumulator;
	uint32 _chunk17TimerAccumulator;
	bool _skipRequested;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE7040_H
