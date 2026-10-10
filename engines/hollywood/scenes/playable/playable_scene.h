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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_PLAYABLE_SCENE_H
#define HOLLYWOOD_SCENES_PLAYABLE_PLAYABLE_SCENE_H

#include "common/array.h"
#include "common/random.h"
#include "common/str.h"
#include "common/types.h"
#include "graphics/managed_surface.h"

#include "hollywood/gameplay/dialogue_menu.h"
#include "hollywood/gameplay/game_loop.h"
#include "hollywood/gameplay/panel_art.h"
#include "hollywood/graphics.h"
#include "hollywood/music.h"
#include "hollywood/resource.h"
#include "hollywood/scenes/animation_channels.h"
#include "hollywood/scenes/animation_layers.h"
#include "hollywood/scenes/animation_player.h"
#include "hollywood/scenes/playable/action_overlay.h"
#include "hollywood/scenes/playable/action_overlay_player.h"
#include "hollywood/scenes/playable/actor_path_controller.h"
#include "hollywood/scenes/playable/actor_types.h"
#include "hollywood/scenes/playable/ambient_audio.h"
#include "hollywood/scenes/playable/scene_config.h"
#include "hollywood/scenes/playable/scene_surface_state.h"
#include "hollywood/scenes/playable/speech_controller.h"
#include "hollywood/scenes/scene_data.h"
#include "hollywood/scenes/scene_resources.h"
#include "hollywood/scenes/scene_text_store.h"
#include "hollywood/scenes/speech_overlay.h"

namespace Graphics {
struct Surface;
}

namespace Hollywood {

class HollywoodEngine;
class InventoryMediaPlayer;

/**
 * Common runtime for rooms managed by the shared point-and-click gameplay loop.
 *
 * It coordinates resource loading, saved actor poses, pathfinding, speech, and
 * presentation while GameplayLoop owns input dispatch.
 *
 * Derived scenes supply a PlayableSceneConfig and override hooks for resource
 * layouts, entry sequences, frame composition, path routing, persistent state,
 * and local actions. The outer lifecycle is:
 *
 * - load() loads the scene resources.
 * - initializePreviewState() prepares preview and runtime state.
 * - Unless restoring a saved pose, draw the preview, optionally present it, then runEntryCutscene().
 * - runBasicGameplayLoop() runs until the configured main-flow state changes.
 * - Unless restarting, invalidate stale saved poses and run optional exit hooks.
 *
 * Persistent background, mask, and hotspot changes belong in
 * applyCustomSceneStateToHotspotsAndPatches(); room-specific transient layers
 * belong in drawCustomComposite(). Entry and action hooks must leave the actor
 * pose and GameplayState ready for the next gameplay frame.
 */
class PlayableScene : public GameplayLoopDelegate, public DialogueMenuDelegate,
		public ActorPathControllerDelegate, public SceneAnimationPlayerDelegate {
public:
	bool play();

protected:
	PlayableScene(HollywoodEngine *vm, const PlayableSceneConfig &config);

	// Executes blocking steps immediately; it coordinates control flow without storing a scene script.
	// Final-state steps survive input fast-forward, but not shutdown or a scene restart.
	class BlockingSequence {
	public:
		explicit BlockingSequence(PlayableScene &scene);

		template<class FrameTarget>
		BlockingSequence &layerFrames(FrameTarget &target, const AnimationFrameRange &range) {
			if (canRun())
				finishMediaStep(_scene.playAnimationFrames(target, range));
			return *this;
		}

		template<class FrameTarget>
		BlockingSequence &presentedLayerFrames(FrameTarget &target, const AnimationFrameRange &range) {
			if (canRun())
				finishMediaStep(_scene.playAndPresentAnimationFrames(target, range));
			return *this;
		}

		template<class FrameTarget>
		BlockingSequence &presentedLayerTransition(FrameTarget &target,
				const AnimationTransition &transition) {
			if (canRun())
				finishMediaStep(_scene.playAndPresentAnimationTransition(target, transition));
			return *this;
		}

		// Configures an existing stack layer, presents the range, and optionally clears it.
		BlockingSequence &resourceLayerFrames(uint layerId, uint chunkIndex,
			uint16 descriptorCount, const byte *frameMap, uint frameMapSize,
			const AnimationFrameRange &range, bool clearAtEnd = true);
		BlockingSequence &resourceLayerFrames(uint layerId, uint chunkIndex,
				uint16 descriptorCount, const AnimationFrameRange &range,
				bool clearAtEnd = true) {
			return resourceLayerFrames(layerId, chunkIndex, descriptorCount,
				nullptr, 0, range, clearAtEnd);
		}

		template<uint size>
		BlockingSequence &resourceLayerFrames(uint layerId, uint chunkIndex,
				uint16 descriptorCount, const byte (&frameMap)[size],
				const AnimationFrameRange &range, bool clearAtEnd = true) {
			return resourceLayerFrames(layerId, chunkIndex, descriptorCount,
				frameMap, size, range, clearAtEnd);
		}

		BlockingSequence &layerFrames(uint layerId, const AnimationFrameRange &range);
		BlockingSequence &presentedLayerFrames(uint layerId, const AnimationFrameRange &range);
		BlockingSequence &presentedLayerTransition(uint layerId,
			const AnimationTransition &transition);
		BlockingSequence &actorReplacement(uint chunkIndex, uint descriptorCount,
			const byte *frameMap, uint frameMapSize, uint32 frameMillis);
		BlockingSequence &actorReplacement(const ActionOverlaySpec &spec);
		BlockingSequence &actorPose(const SceneActorPose &pose, byte cel = 0);
		BlockingSequence &actorPath(const SceneActorPose &target, byte cel = 0,
			bool cancelOnSkip = false);
		BlockingSequence &sound(uint16 cueId, byte volumePercent = 100);
		BlockingSequence &loopingSound(uint16 cueId, byte volumePercent = 100);
		BlockingSequence &stopSound();
		BlockingSequence &secondarySpeech(uint16 rowIndex, byte frameIndex);
		BlockingSequence &primarySpeech(uint16 rowIndex, byte frameIndex, uint16 centerX,
			uint16 topY, byte red, byte green, byte blue);
		BlockingSequence &framebufferPatch(byte selector);
		BlockingSequence &fadeFromBlack();

		template<class T>
		BlockingSequence &commit(T &target, const T &value) {
			if (canFinalize())
				target = value;
			return *this;
		}

		bool completed() {
			refresh();
			return _running;
		}

	private:
		bool canRun();
		bool canFinalize();
		void finishMediaStep(bool completed);
		void refresh();

		PlayableScene &_scene;
		bool _running;
		bool _aborted;
	};

	enum TransitionFrameMode {
		kIndependentTransitionFrames,
		kCumulativeTransitionFrames
	};

	class TransitionFrameRenderer {
	public:
		virtual ~TransitionFrameRenderer() {}
		virtual void drawFrame(byte frame) = 0;
	};

	template<class Callback>
	class TransitionFrameCallbackRenderer : public TransitionFrameRenderer {
	public:
		explicit TransitionFrameCallbackRenderer(const Callback &callback) :
				_callback(callback) {
		}

		void drawFrame(byte frame) override { _callback(frame); }

	private:
		const Callback &_callback;
	};

	// Resource format constants
	enum {
		kPaletteMaskUsedBytes = 0x100,
		kScenePaletteRegionCount = 21,
		kScenePaletteRegionBoundaryCandidateCount = 3,
		kScenePaletteRegionRouteStepCount = 19,
		kSceneRouteBoundaryPointCount = kScenePaletteRegionCount * kScenePaletteRegionCount * kScenePaletteRegionBoundaryCandidateCount,
		kSceneRouteStepCount = kScenePaletteRegionCount * kScenePaletteRegionCount * kScenePaletteRegionRouteStepCount,
		kResource000TableByteCount = 400,
		kAmbientSoundSlotCount = 3,
		kActorFacingCount = 6,
		kActorBankSegmentCount = 2 * (kActorFacingCount + 1),
		kActorCelsPerFacing = 13,
		kActiveActorDescriptorSize = 28,
		kActiveActorFacingRunStride = 160000,
		kSecondaryActorDescriptorSize = 16,
		kSecondaryActorFramesPerFacing = 5,
		kSecondaryActorFacingRunStride = 16000,
		kActorPaletteFirstColor = 0xd0,
		kActorPaletteColorCount = 0x19,
		kActorPaletteBaseBytes = kActorPaletteColorCount * 3,
		kActorPaletteAdjustmentClassCount = (kRouteBoundaryPoints - kPaletteAdjustTable) / 2,
		kActorPaletteBytes = 0x90,
		kStage003DecodeKeySize = 0x141,
		kSpeechCueDescriptorTableSize = 20000,
		kStage003SmallRowSize = 0x29,
		kStage003LargeRowSize = 0x141,
		kStage003LargeRowBaseIndex = 500,
		kOriginalSpeechLineHeight = 20
	};

	// Config-backed resource data
	// These accessors are nonvirtual; exceptional resource layouts use the hooks below.
	const char *resourceArchiveName() const;
	uint sceneInitialRequiredChunkCount() const;
	uint sceneArenaFirstChunk() const;
	uint sceneArenaLastChunk() const;
	uint sceneStageIndex() const;
	const char *sceneDebugName() const;
	uint16 sceneViewportXOffset() const;
	uint16 sceneViewportMinXOffset() const;
	uint16 sceneViewportMaxXOffset() const;
	byte inventoryOwnerIndex() const;
	void initializeInventoryOwnerState();
	uint resource000ActorPaletteTableEntry() const;
	uint32 inventoryActionTableExtraOffset() const;
	uint resource003InventoryRowsOffsetIndex() const;
	uint32 speechCueDescriptorTableOffset() const;
	byte walkablePaletteMaxRegion() const;
	const char *musicArchiveName() const;
	const char *soundBank0ArchiveName() const;
	bool shouldLoadInventoryActionTables() const;
	bool shouldLoadActorDepthTables() const;
	bool usesActorDepthTest() const;
	bool isMainFlowStateInScene(uint16 stateId) const;
	// True when the loop ended by moving to a valid state outside this scene.
	bool didLeaveSceneAfterLoop() const;

	// Resource layout hooks
	virtual uint resource000ActorBankTableEntry() const;
	virtual const byte *actorPathStepDeltaTable() const;
	virtual uint actorPathStepDeltaTableSize() const;
	// Release-specific archives may store the initial framebuffer outside chunk 0.
	virtual uint framebufferResourceChunkIndex() const;
	virtual int alternatePaletteResourceChunkIndex() const;
	virtual bool isAlternatePaletteResourceActive() const;
	// State-dependent room variants may replace chunks 2 and 3 before snapshots and paths are built.
	virtual int replacementFillRunsResourceChunkIndex() const;
	virtual int replacementPaletteMaskResourceChunkIndex() const;
	virtual bool shouldConvertSavedFramebufferFF() const;
	virtual bool shouldLoadArenaChunk(uint index) const;
	// Runs after the gameplay loop unless the scene requested an internal restart.
	virtual void runExitSideEffectsAfterLoop();
	// Returning false lets a custom entry sequence own the first presentation.
	virtual bool shouldPresentPreviewBeforeEntrySequence() const;
	// Allows scene geometry to disable actor depth masking in specific regions.
	virtual bool shouldUseActorDepthTest(int actorWorldX, int actorWorldY) const;

	// Scene hooks
	virtual void initializeCustomPreviewState();
	virtual void drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode);
	// Standard composition prepares draw state before copying the base, then draws
	// custom background and foreground content around the actor-relative strata.
	virtual void prepareCustomComposite(bool drawActors, byte activeFacing, int activeWorldX,
		int activeWorldY, byte actorDrawOrderMode);
	virtual void drawCustomBackgroundComposite(int activeWorldX, int activeWorldY);
	// Draws scene occlusion immediately above the actor slot and below front layers.
	virtual void drawCustomActorForegroundComposite(int activeWorldX, int activeWorldY,
		byte actorDrawOrderMode);
	virtual void drawCustomForegroundComposite(int activeWorldX, int activeWorldY);
	virtual bool shouldDrawSecondaryActorInPlayableComposite() const;
	virtual bool shouldApplyGameplayPanelObjectPalette() const;
	virtual void runCustomEntrySequence();
	virtual void prepareCustomGameplayLoop();
	virtual void advanceCustomGameplayLoop(uint32 delta);
	// The base loop invokes these services every frame; overrides only replace their strategy.
	virtual void advancePrimarySpeechAnimation(uint32 delta);
	virtual void advanceAmbientAudio(uint32 delta);
	// Lets scene-owned animation state distinguish natural completion from cancellation.
	virtual void realtimeSpeechEnded(byte speechId, bool completed);
	// Returning true suppresses generic action dispatch.
	virtual bool dispatchCustomSceneAction(uint16 handlerId);
	// Controls actor paths started by scene clicks, including free walk and item relations.
	virtual bool shouldPlayGameplayClickPath() const;
	// Allows scene staging to adjust metadata-derived facing before path playback.
	virtual byte customizeSceneActionFacing(uint16 handlerId, byte calculatedFacing) const;
	// Returning true validates the adjusted target instead of running the default floor-mask search.
	virtual bool adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const;
	// Called before route lookup so a scene can adjust resource routes for this path origin.
	virtual void prepareCustomActorPathRoute(int startX, int startY);
	bool customizeRouteSegment(byte currentRegion, byte nextRegion, const ActorPathBuildState &state,
		const ScenePoint &boundary, int &requestedFacing, bool &restoredStepDeltas) override;
	bool customizeRouteFinal(byte currentRegion, byte targetRegion, const ActorPathBuildState &state,
		int targetX, int targetY, int &requestedFacing, bool &restoredStepDeltas) override;
	// Returning true suppresses the shared selector handling.
	virtual bool applyCustomSceneStateToHotspotsAndPatches(byte selector);
	virtual bool shouldAnimatePrimarySpeechLine() const;
	virtual void setPrimaryLeftSpeechFrame(byte frameIndex);
	virtual AmbientAudioProfile ambientAudioProfile() const;
	void handleAnimationFrameHook(byte hookId, uint frame) override;
	// Advances scene services while a transition renderer owns the framebuffer.
	virtual void advanceTransitionAnimation(uint32 delta);
	virtual void advanceFullscreenAnimation(uint32 delta);

	// Ambient audio
	AmbientAudioProfile createLoopingAmbientAudioProfile(byte volumePercent) const;
	AmbientAudioProfile createRandomAmbientAudioProfile(byte soundFirstCueId, byte soundCueCount,
		byte soundVolumePercent, byte soundProbabilityModulus, byte musicFirstCueId,
		byte musicCueCount, byte musicVolumePercent, byte musicProbabilityModulus) const;
	AmbientAudioProfile createMineAmbientAudioProfile(byte musicFirstCueId = 0x0b,
		byte musicCueCount = 5, byte musicProbabilityModulus = 50) const;
	bool playResidentSoundEffect(byte soundEffectId, byte volumePercent = 100);
	bool playActiveActorFootstep();

	// Actor save state
	bool hasSavedActiveActorPoseForCurrentState() const;
	void restoreActiveActorPoseFromGameState();
	void syncActiveActorPoseToGameState();
	void setActiveActorPose(int worldX, int worldY, byte facing, byte cel = 0);

	// Framebuffers
	void initializeFramebuffers();
	uint framebufferByteCount() const;
	byte *framebufferPixels(Graphics::ManagedSurface &surface);
	const byte *framebufferPixels(const Graphics::ManagedSurface &surface) const;
	void copyBaseFramebufferToSceneFramebuffer();
	void restoreBaseFramebufferFromOriginal();
	bool isFramebufferOffsetValid(uint offset) const;
	byte savedFramebufferPixelAt(uint offset) const;

	// Resource loading
	bool load();
	bool loadResource000RuntimeTables(Common::Array<byte> &offsetTable, Common::Array<byte> &sizeTable);
	bool loadResource000ActorBank(const Common::Array<byte> &offsetTable, const Common::Array<byte> &sizeTable);
	bool loadResource000ActorPalette(const Common::Array<byte> &offsetTable);
	void captureActorPaletteBase();
	bool loadResource000InventoryActionTables(const Common::Array<byte> &offsetTable);
	bool loadStage003SceneRows();
	bool loadFixedChunk(uint index, Common::Array<byte> &destination, uint fixedSize);
	bool loadFixedChunk(uint index, Graphics::ManagedSurface &destination, uint fixedSize);
	bool loadVariableChunk(uint index, Common::Array<byte> &destination);
	bool loadArenaChunk(uint index);
	bool initializeActorDepthTables();
	void updateActorDepthThresholds(byte actorDrawOrderMode);
	void expandFillRunsToSavedFramebuffer();
	bool initializeScenePathTables();

	// Rendering
	void initializePreviewState();
	void initializeDefaultPreviewState();
	void drawPreviewComposite();
	void drawCutsceneComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode = 0);
	void drawPlayableComposite();
	void drawActiveAndSecondaryActorFrames(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		int minimumYExclusive);
	void drawActiveActorFrame(byte facing, byte cel, int worldX, int worldY, int minimumYExclusive);
	void updateActorPaletteForWorldPoint(int worldX, int worldY);
	int drawSecondaryActorFrame(byte facing, byte frame, int worldX, int worldY);
	int drawActorRun(const Common::Array<byte> &runStreams, uint cursor, uint runBase, uint runCount,
		int spriteX, int spriteY, int minimumYExclusive, int actorWorldX, int actorWorldY, uint *nextCursor = nullptr);
	int drawActorPaletteRemapRun(const Common::Array<byte> &runStreams, uint cursor, uint runBase, uint runCount,
		int spriteX, int spriteY, int minimumYExclusive, int actorWorldX, int actorWorldY);
	void runEntryCutscene();
	void runEntryPath(int startX, int startY, byte startFacing, int targetX, int targetY);

	// GameplayLoopDelegate
	bool runBasicGameplayLoop();
	const SceneHotspotTable &hotspots() const override;
	const Graphics::Surface &savedFramebuffer() const override;
	uint16 viewportXOffset() const override;
	uint16 viewportYOffset() const override;
	void prepareGameplayLoop() override;
	void advanceGameplayLoop(uint32 delta) override;
	void drawGameplayFrame() override;
	void presentGameplayFrame(const SceneHoverCaption &hoverCaption, const GameplayPanelState &panelState) override;
	void invalidatePresentationPalette() override;
	void prepareOptionsMenuPalette(Common::Array<byte> &palette) const override;
	Common::Path optionsMenuSoundBank0ArchiveName() const override;
	bool optionsMenuSpeechPreviewSampleId(uint16 &sampleId) const override;
	void suspendAudioForOptionsMenu() override;
	bool shouldExitGameplayLoop() const override;
	Common::String inventoryItemName(byte owner, byte itemId) const override;
	void beginSharedInventorySpeechLine(uint16 rowIndex, byte frameIndex) override;
	byte randomSharedInventorySpeechFrame(byte maxFrameIndex) override;
	void playSharedInventorySound(byte sampleId) override;
	void showTravelScreenViewer() override;
	bool showInventoryMedia(InventoryMediaId mediaId) override;
	bool playSueTapeRecording() override;
	void handleLeftClick(const GameplayLoopCursorState &state) override;
	void handleInventoryItemClick(const GameplayLoopCursorState &state) override;

	// Ambient runtime
	void updateAmbientAudioAndMusicCues(uint32 delta);
	void resetAmbientAudioState();
	void ensureAmbientSoundCuePlaying(byte slotIndex, uint16 cueId, byte volumePercent);
	void stopAmbientSoundCues();
	void updateAmbientSoundCue(const AmbientAudioProfile &profile);
	void updateAmbientMusicCue(const AmbientAudioProfile &profile);

	// Speech animation
	void advanceSecondaryActorSpeechAnimation(uint32 delta);
	void advanceSecondaryActorSpeechFrame();
	void advancePrimaryLeftSpeechFrame();
	void advancePrimaryDialogueSpeechFrame(uint32 delta);
	// Realtime speech advances inside the gameplay loop, leaving panels and scene input responsive.
	bool startRealtimePrimarySpeechLine(uint16 rowIndex, byte frameIndex, uint16 centerX,
		uint16 topY, byte red, byte green, byte blue, byte animationGroup, byte speechId,
		byte volumePercent = 0xff);
	bool startRealtimeSecondarySpeechLine(uint16 rowIndex, byte frameIndex, byte speechId);
	bool waitForRealtimeSpeech(bool allowSkip = true);
	void advanceRealtimeSpeech(uint32 delta);
	void stopRealtimeSpeech();
	bool isRealtimeSpeechActive() const { return _realtimeSpeechActive; }
	bool isRealtimeSecondarySpeechActive() const {
		return _realtimeSpeechActive && !_realtimeSpeechPrimary;
	}
	byte realtimeSpeechId() const { return _realtimeSpeechId; }
	void startRealtimeSpeechPart();
	void finishRealtimeSpeech(bool completed);

	// Scene action dispatch
	void processSceneActionClick(const GameplayLoopCursorState &state);
	void processSceneRelationClick(const GameplayLoopCursorState &state, byte itemId);
	bool gameplayActionInputBlocked() const;
	void runGameplayAction(uint16 handlerId);
	bool dispatchGenericInventoryAction(const GameplayLoopCursorState &state);
	void dispatchSceneAction(uint16 handlerId);
	bool dispatchGenericSceneAction(uint16 handlerId);

	// Pathfinding
	bool walkActiveActorTo(int targetX, int targetY, byte finalFacing, byte finalCel, bool cancelOnSkip = false);
	void startConcurrentActorPath(int targetX, int targetY, byte finalFacing, byte finalCel,
		uint32 frameMillis, bool snapToTargetIfNoPath = false);
	// Player-directed paths stay in the gameplay loop so a later click can replace them.
	bool startPlayerDirectedActorPath(int targetX, int targetY, byte finalFacing,
		byte finalCel, uint16 actionHandlerId);
	void finishConcurrentActorPath(bool allowSkip = false);
	void cancelConcurrentActorPath();
	bool concurrentActorPathActive() const { return _concurrentActorPathActive; }
	bool adjustWalkTargetToFloorMask(int &targetX, int &targetY) const;
	void queueActorPathWithPaletteRegionRouting(int startX, int startY, int targetX, int targetY,
		byte finalFacing, byte finalCel);
	byte paletteRegionAt(int x, int y) const override;
	byte walkableMaskAt(int x, int y) const;
	void resetActorPathStepDeltas();
	byte calculateFacingTowardPoint(int fromX, int fromY, int toX, int toY) const;

	// Scene state and hotspots
	// Selector 0xff reapplies the complete state; other values request scene-specific partial updates.
	void applySceneStateToHotspotsAndPatches(byte selector);
	void rebuildWalkablePaletteMask();
	// Adds a notebook destination and presents its original unlock transition.
	bool unlockTravelDestination(byte destinationId);
	void requestTravelScreenSelection(byte currentChapterId);

	// Inventory
	bool hasInventoryItem(byte itemId) const;
	void addInventoryItem(byte itemId);
	void removeInventoryItem(byte itemId);

	// DialogueMenuDelegate
	Common::String dialogueMenuText(byte stageId, byte textRowId) const override;
	void advanceDialogueMenu(uint32 delta) override;
	void drawDialogueMenuFrame() override;
	void presentDialogueMenuFrame(const DialogueMenuState &state) override;

	// Action overlays
	// Actor actions replace the regular actor; scene overlays retain it.
	void runActorReplacement(uint chunkIndex, uint descriptorCount, const byte *frameMap, uint frameMapSize,
		uint32 frameMillis);
	void runActorReplacement(const ActionOverlaySpec &spec);
	void runSceneOverlay(uint chunkIndex, uint descriptorCount, const byte *frameMap,
		uint frameMapSize, uint32 frameMillis);
	void runSceneOverlay(const ActionOverlaySpec &spec);

	// Speech animation hooks
	virtual byte primarySpeechAnimationBaseFrame(byte animationGroup) const;
	virtual byte primarySpeechAnimationInitialFrame(byte animationGroup, byte baseFrame) const;
	// Number of consecutive logical mouth frames starting at the base frame.
	virtual byte primarySpeechAnimationFrameCount(byte animationGroup) const;
	// Allows resource-specific actor layers to retain their original mouth-frame cadence.
	virtual uint32 primarySpeechAnimationFrameMillis(byte animationGroup) const;
	// Preserves a speaker group's original level relative to the configured speech volume.
	virtual byte primarySpeechVolumePercent(byte animationGroup) const;
	virtual void setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex);
	virtual void primarySpeechAnimationStarted(byte animationGroup, byte baseFrame);
	virtual void primarySpeechAnimationRestored(byte animationGroup, byte baseFrame);

	// Timing
	// Pumps gameplay and redraws while waiting; returns true if playback should stop.
	bool waitSceneMillis(uint32 millis, bool allowSkip = true);

	// Viewport
	void resetViewportFromScene();
	void centerViewportOnActor(int actorWorldX);
	void advanceViewportScroll(uint32 delta);

	// Speech overlays
	void clearSpeechOverlay();
	void clearAllSpeechOverlays();
	void drawSpeechOverlay();

	// Resource sprite layers
	void drawMappedSpriteFrame(uint chunkIndex, uint descriptorCount, const byte *frameMap, uint frameMapSize, byte frameIndex);
	void restoreResourceSpriteLayerBackground(const ResourceSpriteLayer &layer, const Graphics::Surface &background);
	void drawResourceSpriteLayer(const ResourceSpriteLayer &layer);
	void restoreSceneLayerBackground(uint layerId, const Graphics::Surface &background);
	void drawSceneLayer(uint layerId);
	void drawLayerStack(SceneAnimationStratum stratum);
	void drawActionOverlayAtStratum(SceneAnimationStratum stratum);
	void drawActionOverlayLayer();
	template<class FrameTarget>
	bool playAnimationFrames(FrameTarget &target, const AnimationFrameRange &range) {
		return _animationPlayer.play(target, range);
	}
	template<class FrameTarget>
	bool playAndPresentAnimationFrames(FrameTarget &target, const AnimationFrameRange &range) {
		return _animationPlayer.playAndPresent(target, range);
	}
	bool playAnimationFrames(uint layerId, const AnimationFrameRange &range) {
		if (!_sceneLayers.hasLayer(layerId))
			return false;
		return playAnimationFrames(_sceneLayers.layer(layerId), range);
	}
	bool playAndPresentAnimationFrames(uint layerId, const AnimationFrameRange &range) {
		if (!_sceneLayers.hasLayer(layerId))
			return false;
		return playAndPresentAnimationFrames(_sceneLayers.layer(layerId), range);
	}
	template<class FrameTarget>
	bool playAnimationTransition(FrameTarget &target, const AnimationTransition &transition) {
		return _animationPlayer.transition(target, transition);
	}
	template<class FrameTarget>
	bool playAndPresentAnimationTransition(FrameTarget &target, const AnimationTransition &transition) {
		return _animationPlayer.transitionAndPresent(target, transition);
	}
	bool playAnimationTransition(uint layerId, const AnimationTransition &transition) {
		if (!_sceneLayers.hasLayer(layerId))
			return false;
		return playAnimationTransition(_sceneLayers.layer(layerId), transition);
	}
	bool playAndPresentAnimationTransition(uint layerId,
			const AnimationTransition &transition) {
		if (!_sceneLayers.hasLayer(layerId))
			return false;
		return playAndPresentAnimationTransition(_sceneLayers.layer(layerId), transition);
	}
	bool playResourceLayerSequence(uint layerId, uint chunkIndex, uint16 descriptorCount,
			const byte *frameMap, uint frameMapSize, const AnimationFrameRange &range,
			bool clearAtEnd = true);
	bool playResourceLayerSequence(uint layerId, uint chunkIndex, uint16 descriptorCount,
			const AnimationFrameRange &range, bool clearAtEnd = true) {
		return playResourceLayerSequence(layerId, chunkIndex, descriptorCount,
			nullptr, 0, range, clearAtEnd);
	}
	template<uint size>
	bool playResourceLayerSequence(uint layerId, uint chunkIndex, uint16 descriptorCount,
			const byte (&frameMap)[size], const AnimationFrameRange &range,
			bool clearAtEnd = true) {
		return playResourceLayerSequence(layerId, chunkIndex, descriptorCount,
			frameMap, size, range, clearAtEnd);
	}
	void clearSceneLayer(uint layerId);

	// Resource delta clips
	void drawClipFrameDeltaFromResource(const Common::Array<byte> &resource, uint32 frameTableOffset,
		uint32 chunkSize, uint tableEntryCount, byte frameIndex);
	void drawClipFrameDelta(uint chunkIndex, uint tableEntryCount, byte frameIndex);
	void playDeltaClip(uint chunkIndex, uint tableEntryCount, uint frameCount, uint32 frameMillis,
		uint firstFrame = 0);
	// Cumulative transitions apply every elapsed delta; independent transitions draw only the newest frame.
	bool playTransitionFrames(TransitionFrameRenderer &renderer, byte firstFrame,
		byte lastFrame, uint32 frameMillis, TransitionFrameMode mode, bool allowSkip = true);
	template<class Callback>
	bool playTransitionFrames(const Callback &drawFrame, byte firstFrame, byte lastFrame,
			uint32 frameMillis, TransitionFrameMode mode, bool allowSkip = true) {
		TransitionFrameCallbackRenderer<Callback> renderer(drawFrame);
		return playTransitionFrames(static_cast<TransitionFrameRenderer &>(renderer),
			firstFrame, lastFrame, frameMillis, mode, allowSkip);
	}
	// Polls events without advancing or redrawing the scene; returns true if playback should stop.
	bool waitDeltaClipFrameMillis(uint32 millis);
	// Advances full-screen scene hooks without redrawing the ordinary scene composite.
	bool waitFullscreenAnimationFrame(uint32 millis, bool allowSkip);
	// Temporarily owns the framebuffer and palette, then restores the playable composite.
	bool playFullscreenDeltaAnimation(const Common::Array<byte> &base,
		const Common::Array<byte> &palette, const Common::Array<byte> &frames,
		uint frameCount, uint32 frameMillis);
	bool fadePaletteFromBlack();
	bool fadePaletteToBlack();

	// SceneAnimationPlayerDelegate
	bool animationPlaybackShouldStop() const override;
	void presentAnimationFrame() override;
	bool waitForAnimationFrame(uint32 millis, bool allowSkip) override;
	bool consumeAnimationFastForwardRequest() override;
	void handleAnimationFrameEvent(const AnimationFrameEvent &event, uint frame) override;

	// Speech playback
	// The begin*() helpers block until speech finishes. startSecondarySpeechLine() does not and
	// returns whether voice playback started.
	void beginSecondarySpeechLine(uint16 rowIndex, byte frameIndex);
	bool startSecondarySpeechLine(uint16 rowIndex, byte frameIndex);
	void beginStaticSecondarySpeechLine(uint16 rowIndex, byte frameIndex);
	void beginPrimarySpeechLine(uint16 rowIndex, byte frameIndex, uint16 centerX, uint16 topY,
		byte red, byte green, byte blue);
	void beginPrimarySpeechLineWithAnimationGroup(uint16 rowIndex, byte frameIndex, uint16 centerX, uint16 topY,
		byte red, byte green, byte blue, byte animationGroup);
	void beginPrimaryLeftSpeechLine(uint16 rowIndex, byte frameIndex);
	void runSpeechLine(SpeechOverlay &overlay, uint16 rowIndex, byte frameIndex, uint16 centerX, uint16 topY,
		byte colorIndex, bool useRequestedTop, bool animatePrimaryLeft, bool animatePrimaryDialogue,
		byte primaryAnimationGroup = 0xff);
	void runSpeechCue(SpeechOverlay &overlay, uint16 textRecordId, byte continuationCount, uint16 voiceSampleId,
		uint16 centerX, uint16 topY, byte colorIndex, bool useRequestedTop, bool animatePrimaryLeft,
		bool animatePrimaryDialogue, byte primaryAnimationGroup = 0xff);
	bool getStage003Cue(uint16 rowIndex, byte frameIndex, uint16 &textRecordId, byte &continuationCount,
		uint16 &voiceSampleId) const;
	bool getStaticSpeechCue(uint16 rowIndex, byte frameIndex, uint16 &textRecordId, byte &continuationCount,
		uint16 &voiceSampleId) const;
	void wrapActorSpeechText(const Common::String &text, uint16 anchorSceneX, Common::Array<Common::String> &lines) const;
	Common::String getResource003LargeTextRecord(uint16 recordId) const;
	void calculateSpeechOverlayBounds(SpeechOverlay &overlay, int centerX, int topY, bool useRequestedTop,
		int actorWorldY);
	void calculateSecondarySpeechBounds(int actorWorldX, int actorWorldY);
	bool waitForSpeechOrDelay(uint32 fallbackMillis, bool animatePrimaryLeft);
	bool consumeStepAdvanceRequest();
	bool runSueTapeSpeechLine(InventoryMediaPlayer &media, uint16 rowIndex, byte frameIndex,
		byte red, byte green, byte blue, byte &animationFrame, uint32 &animationMillis);
	bool waitForFullscreenMediaDismissal();
	bool pollFullscreenMediaEvents(bool &dismissed);
	void installFullscreenInventoryMedia(const InventoryMediaPlayer &media,
		Graphics::ManagedSurface &savedScene, Common::Array<byte> &savedPalette, uint16 &savedViewportX);
	void restoreFullscreenInventoryMedia(const Graphics::ManagedSurface &savedScene,
		const Common::Array<byte> &savedPalette, uint16 savedViewportX);

	// Palette
	void setPaletteEntry6Bit(byte colorIndex, byte red, byte green, byte blue);
	byte paletteEntryComponent6Bit(byte colorIndex, uint component) const;
	void applyGameplayPanelPalette();

	// Panel and presentation
	void drawGameplayPanel(Graphics::Surface &surface, const GameplayPanelState &panelState);
	void drawVerbPanel(Graphics::Surface &surface, const GameplayPanelState &panelState);
	void drawInventoryPanel(Graphics::Surface &surface, const GameplayPanelState &panelState);
	void presentFrame(const SceneHoverCaption *hoverCaption = nullptr,
		const GameplayPanelState *panelState = nullptr, const DialogueMenuState *dialogueMenuState = nullptr);

	// Event pumping
	// Returns true for quit, scene restart, or accepted skip input.
	virtual bool pollEvents(bool allowSkip);

	// Engine and resources
	HollywoodEngine *_vm;
	PlayableSceneConfig _config;
	uint16 _sceneStateId;

	// In helper-backed groups, reference members alias storage owned by the first object
	// in the group, preserving the protected names used by scene-specific hooks.

	// Scene resource state
	SceneResources _resources;
	ResourceChunkTable &_sceneChunkTable;
	uint32 (&_resourceChunkOffsets)[kResourceChunkCount];
	Common::Array<byte> &_resourceArena;
	Common::Array<byte> &_metadata;

	// Scene surface state
	SceneSurfaceState _surfaceState;
	Common::Array<byte> &_paletteResource;
	Common::Array<byte> &_paletteCurrent;
	Common::Array<byte> &_actorPaletteBase;
	Graphics::ManagedSurface &_baseFramebufferOriginal;
	Graphics::ManagedSurface &_baseFramebuffer;
	Graphics::ManagedSurface &_sceneFramebuffer;
	Graphics::ManagedSurface &_savedFramebuffer;
	Common::Array<byte> &_fillRuns;
	Common::Array<byte> &_paletteMaskOriginal;
	Common::Array<byte> &_paletteMask;
	Common::Array<byte> &_fullPaletteRegionMask;
	Common::Array<byte> &_walkablePaletteMask;
	Common::Array<byte> &_colorToActorDepthClassMap;
	Common::Array<byte> &_presentationPaletteRemapTable;
	Common::Array<uint16> &_actorDepthYThresholds;
	Common::Array<uint16> &_drawActorDepthYThresholds;
	Graphics::ManagedSurface &_screen;
	Palette6Bit &_displayPalette;

	// Actor resources
	Common::Array<byte> _resource000OffsetTable;
	Common::Array<byte> _resource000SizeTable;
	Common::Array<byte> _activeActorRunStreams;
	Common::Array<byte> _secondaryActorRunStreams;
	Common::Array<ActiveActorSpriteDescriptor> _activeActorDescriptors;
	Common::Array<SecondaryActorSpriteDescriptor> _secondaryActorDescriptors;

	// Scene text store
	SceneTextStore _textStore;
	Common::Array<byte> &_stage003DecodeKey;
	Common::Array<byte> &_stage003SmallRows;

	// Pathfinding state
	ActorPathController _pathController;
	Common::Array<ScenePoint> &_routeBoundaryPoints;
	Common::Array<byte> &_routeSteps;
	Common::Array<ActorPathFrame> &_actorPathFrames;
	Common::Array<byte> &_actorPathStepDeltas;

	// Runtime systems
	GameplayPanelArt _panelArt;
	SceneHotspotTable _hotspots;
	SoundBank0Player _soundBank0;
	SoundBank0Player _ambientSoundBank0;
	SoundBank0Player _additionalAmbientSoundBank0Slots[kAmbientSoundSlotCount - 1];
	ResidentSoundEffectPlayer _residentSoundEffects;
	Common::RandomSource _random;
	SceneLayerStack _sceneLayers;
	RealtimeAnimationTracks _realtimeAnimationTracks;
	SceneAnimationPlayer _animationPlayer;
	TimedAnimationChannel _concurrentActorPathChannel;

	// Speech runtime state
	SpeechController _speechController;
	SpeechPlayer _realtimeSpeechPlayer;
	SpeechPlayer &_speech;
	SpeechOverlay &_speechOverlay;
	SpeechOverlay &_primarySpeechOverlay;
	byte &_primaryLeftSpeechLastFrame;
	byte &_primaryDialogueSpeechLastFrame;
	byte &_primaryDialogueSpeechGroup;
	bool &_primaryLeftSpeechActive;
	bool &_primaryDialogueSpeechActive;
	uint32 &_secondaryActorTimerAccumulator;
	uint32 &_primaryLeftSpeechTimerAccumulator;
	uint32 &_primaryDialogueSpeechTimerAccumulator;
	byte &_secondaryActorFrame;
	uint32 _realtimeSpeechElapsed;
	uint32 _realtimeSpeechDuration;
	uint16 _realtimeSpeechTextRecordId;
	uint16 _realtimeSpeechVoiceSampleId;
	uint16 _realtimeSpeechCenterX;
	uint16 _realtimeSpeechTopY;
	byte _realtimeSpeechContinuationPart;
	byte _realtimeSpeechContinuationCount;
	byte _realtimeSpeechAnimationGroup;
	byte _realtimeSpeechId;
	byte _realtimeSpeechVolumePercent;
	bool _realtimeSpeechActive;
	bool _realtimeSpeechPrimary;

	// Action overlay runtime state
	ActionOverlayPlayer _actionOverlayPlayer;
	bool &_hideActiveActor;

	// Scene runtime state
	bool _inventoryItems[121];
	byte _sceneStateFlags[8];
	uint32 _ambientMusicTimerAccumulator;
	byte _previousAmbientMusicTrackId;
	byte _currentAmbientSoundCueId;
	byte _previousAmbientSoundCueId;
	uint16 _viewportXOffset;
	uint16 _viewportMinXOffset;
	uint16 _viewportMaxXOffset;
	int _lastViewportScrollActorWorldX;
	uint _concurrentActorPathFrameIndex;
	bool _concurrentActorPathActive;
	bool _actorPathPlaybackActive;
	bool _playerDirectedActorPathActive;
	bool _playerDirectedActorPathFootstepPlayed;
	uint16 _pendingGameplayActionHandlerId;
	int _activeActorWorldX;
	int _activeActorWorldY;
	byte _activeActorFacing;
	byte _activeActorCel;
	byte _activeActorDrawOrderMode;
	byte _lastSceneActionItemId;
	byte _lastInventoryActionItemId;
	byte _lastInventoryPrimaryItemId;
	bool _skipRequested;
	bool _stepAdvanceRequested;

private:
	void runActionOverlay(const ActionOverlaySpec &spec, SceneAnimationStratum stratum,
		bool hideActiveActor);
	void advanceConcurrentActorPath(uint32 delta);
	void handleActorPathFootstep(bool terminalFrame, bool &footstepPlayed);
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_PLAYABLE_SCENE_H
