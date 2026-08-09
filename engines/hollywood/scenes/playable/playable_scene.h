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
#include "hollywood/scenes/playable/action_overlay.h"
#include "hollywood/scenes/playable/action_overlay_player.h"
#include "hollywood/scenes/playable/actor_path_controller.h"
#include "hollywood/scenes/playable/actor_types.h"
#include "hollywood/scenes/playable/ambient_audio.h"
#include "hollywood/scenes/playable/animation_channels.h"
#include "hollywood/scenes/playable/animation_layers.h"
#include "hollywood/scenes/playable/scene_config.h"
#include "hollywood/scenes/playable/scene_resources.h"
#include "hollywood/scenes/playable/scene_surface_state.h"
#include "hollywood/scenes/playable/scene_text_store.h"
#include "hollywood/scenes/playable/speech_controller.h"
#include "hollywood/scenes/playable/speech_overlay.h"

namespace Graphics {
struct Surface;
}

namespace Hollywood {

class HollywoodEngine;

/*
 * PlayableScene is the common runtime for rooms where the player can move an
 * actor, inspect hotspots, use inventory, and trigger scene-specific actions.
 * Derived scenes mostly provide resource ids, entry behavior, draw ordering,
 * local animation channels, and custom action handlers.
 *
 * Scene execution has these phases:
 *
 * 1. Resource load
 *    play() starts by calling load(). This resolves the scene archive, loads
 *    the fixed framebuffer/palette/mask/metadata chunks, expands the saved
 *    background image, loads the sprite arena, actor banks, actor palettes,
 *    stage text, hotspot tables, panel art, music archive, and sound archive.
 *    Resource configuration callbacks such as resourceArchiveName(),
 *    sceneStageIndex(), sceneArenaFirstChunk(), sceneArenaLastChunk(),
 *    resource000ActorBankTableEntry(), musicArchiveName(), and
 *    soundBank0ArchiveName() are consulted during this phase.
 *
 * 2. Initial scene state
 *    After loading, initializePreviewState() prepares the runtime state used
 *    by both preview and gameplay. Scenes with stateful backgrounds or actors
 *    override hasCustomPreviewState()/initializeCustomPreviewState() to place
 *    actors, reset local animation layers, apply persistent flags, and rebuild
 *    hotspot maps. Scenes without a custom preview use the default actor pose
 *    from the constructor plus applySceneStateToHotspotsAndPatches(0xff).
 *
 * 3. Preview and entry sequence
 *    If the scene is not being restored from a saved actor pose, play() draws
 *    one preview frame with drawPreviewComposite(), presents it, then runs
 *    runEntryCutscene(). A scene can override hasCustomEntrySequence() and
 *    runCustomEntrySequence() for walking in from an exit, playing intro
 *    speech, or running a short non-interactive sequence before input is
 *    accepted. Entry sequences typically use runEntryPath(), speech helpers,
 *    overlays, and normal compositing, so they should leave the actor and
 *    persistent scene flags in the same state the gameplay loop expects.
 *
 * 4. Gameplay loop
 *    runBasicGameplayLoop() owns input and repeatedly calls this object as a
 *    GameplayLoopDelegate. prepareGameplayLoop() resets transient runtime
 *    state, calls prepareCustomGameplayLoop(), restores a saved actor pose if
 *    needed, and records the current actor pose for saves. Each frame,
 *    advanceGameplayLoop() advances speech animation, calls
 *    advanceCustomGameplayLoop(delta), updates ambient audio and viewport
 *    scrolling, then syncs the actor pose. drawGameplayFrame() composites the
 *    current frame and presentGameplayFrame() uploads it to the screen.
 *
 * 5. Frame composition
 *    drawCutsceneComposite() and drawPlayableComposite() are the only places
 *    that assemble a frame. The default renderer copies the mutable base
 *    framebuffer, draws actors, and draws the active overlay. Scenes with
 *    room-specific layer ordering override hasCustomComposite() and
 *    drawCustomComposite() to draw animated sprite layers, foreground blocks,
 *    occlusion patches, actors, speech actors, and overlays in the order used
 *    by the original room.
 *
 * 6. Hotspots, walking, and actions
 *    The gameplay loop resolves cursor input through the hotspot and inventory
 *    tables loaded from the resources. Walk requests use the palette-region
 *    route tables through ActorPathController; scenes may adjust final targets
 *    or route segments with adjustCustomWalkTargetToFloorMask(),
 *    customizeRouteSegment(), and customizeRouteFinal(). Once an action id is
 *    selected, dispatchSceneAction() first gives the derived scene a chance
 *    through dispatchCustomSceneAction(), then falls back to shared handlers in
 *    dispatchGenericSceneAction().
 *
 * 7. State patches and persistence
 *    Scene-specific flags live in GameplayState and are applied to the current
 *    base framebuffer, palette masks, walkable mask, hotspot map, and text rows
 *    through applySceneStateToHotspotsAndPatches(). Call this after changing a
 *    persistent room flag or taking an object. Actor position and viewport are
 *    synced during gameplay so saves reload to a stable playable pose; saves
 *    taken during entry/cutscenes normally replay that entry/cutscene.
 *
 * 8. Speech, overlays, and side effects
 *    Speech helpers block until the current line is finished and may drive
 *    actor animation depending on shouldAnimatePrimarySpeechLine() and related
 *    hooks. Action overlays are short blocking animations run through
 *    ActionOverlaySpec and runActionOverlay(); they may hide the actor, play
 *    sounds, patch scene state on a frame, or report frame hooks through
 *    handleActionOverlayFrameHook(). When the gameplay loop exits because the
 *    main-flow state changed, play() can run runExitSideEffectsAfterLoop()
 *    before the engine dispatches the next scene.
 */
class PlayableScene : public GameplayLoopDelegate, public DialogueMenuDelegate, public ActorPathControllerDelegate {
public:
	// Loads, runs, and exits a playable scene.
	bool play();

protected:
	// Creates a playable scene whose common metadata comes from a config struct.
	PlayableScene(HollywoodEngine *vm, const PlayableSceneConfig &config, const char *randomName,
		int defaultActorX, int defaultActorY, byte defaultActorFacing,
		byte secondarySpeechTextColor, byte primarySpeechTextColor);

	// Fixed table and buffer sizes shared by playable scenes.
	enum {
		kFrameBufferSize = 0x78000,
		kPaletteMaskUsedBytes = 0x100,
		kScenePaletteRegionCount = 21,
		kScenePaletteRegionBoundaryCandidateCount = 3,
		kScenePaletteRegionRouteStepCount = 19,
		kSceneRouteBoundaryPointCount = kScenePaletteRegionCount * kScenePaletteRegionCount * kScenePaletteRegionBoundaryCandidateCount,
		kSceneRouteStepCount = kScenePaletteRegionCount * kScenePaletteRegionCount * kScenePaletteRegionRouteStepCount,
		kResource000TableByteCount = 400,
		kResource000DefaultActorBankTableEntry = 0xd0,
		kResource000DefaultActorPaletteTableEntry = 0x108,
		kResource000DefaultActorBankSegmentCount = 14,
		kActorFacingCount = 6,
		kActorCelsPerFacing = 13,
		kActiveActorDescriptorSize = 28,
		kActiveActorFacingRunStride = 160000,
		kSecondaryActorDescriptorSize = 16,
		kSecondaryActorFramesPerFacing = 5,
		kSecondaryActorFacingRunStride = 16000,
		kActorPaletteBytes = 0x90,
		kStage003DecodeKeySize = 0x141,
		kStage003StageOffsetTableSize = 0xff4,
		kStage003DescriptorTableSize = 0x186a0,
		kDefaultResource003InventoryRowsOffsetIndex = 0x32,
		kDefaultSpeechCueDescriptorTableOffset = 0x5f58,
		kSpeechCueDescriptorTableSize = 20000,
		kStage003SmallRowSize = 0x29,
		kStage003LargeRowSize = 0x141,
		kStage003LargeRowBaseIndex = 500,
		kOriginalSpeechLineHeight = 20
	};

	// Scene Resource Configuration
	// These read the PlayableSceneConfig every scene fills in its constructor.
	// They are deliberately not virtual: scenes configure, they do not override.
	// Names the scene resource archive, such as RESOURCE.G04.
	const char *resourceArchiveName() const;
	// Number of low-index chunks required before optional scene data.
	uint sceneInitialRequiredChunkCount() const;
	// First chunk copied into the scene resource arena.
	uint sceneArenaFirstChunk() const;
	// Last chunk copied into the scene resource arena.
	uint sceneArenaLastChunk() const;
	// RESOURCE.003 stage number used for scene text and metadata.
	uint sceneStageIndex() const;
	// Human-readable scene label for warnings and debug output.
	const char *sceneDebugName() const;
	// Default horizontal viewport offset for the scene.
	uint16 sceneViewportXOffset() const;
	// Minimum horizontal viewport offset allowed while scrolling.
	uint16 sceneViewportMinXOffset() const;
	// Maximum horizontal viewport offset allowed while scrolling.
	uint16 sceneViewportMaxXOffset() const;
	// Inventory owner/character index used by this scene.
	byte inventoryOwnerIndex() const;
	// Initializes character-specific inventory tables and defaults.
	void initializeInventoryOwnerState();
	// Number of RESOURCE.000 actor bank segments to load.
	uint resource000ActorBankSegmentCount() const;
	// RESOURCE.000 offset-table entry for actor palette colors.
	uint resource000ActorPaletteTableEntry() const;
	// Extra byte offset into RESOURCE.000 inventory action tables.
	uint32 inventoryActionTableExtraOffset() const;
	// RESOURCE.003 owner text table index for inventory item names.
	uint resource003InventoryRowsOffsetIndex() const;
	// RESOURCE.003 offset for fixed speech cue descriptors.
	uint32 speechCueDescriptorTableOffset() const;
	// Highest palette-region id treated as walkable by default.
	byte walkablePaletteMaxRegion() const;
	// Music archive used by the scene.
	const char *musicArchiveName() const;
	// Sound bank 0 archive used by the scene.
	const char *soundBank0ArchiveName() const;
	// Whether RESOURCE.000 inventory action tables should be loaded.
	bool shouldLoadInventoryActionTables() const;
	// Whether actor depth tables are available in chunk 4.
	bool shouldLoadActorDepthTables() const;
	// Enables per-pixel actor depth clipping while drawing actors.
	bool usesActorDepthTest() const;
	// Checks if a main-flow state belongs to this scene.
	bool isMainFlowStateInScene(uint16 stateId) const;

	// Scene Resource Overrides
	// Resource behavior that individual scenes genuinely replace.
	// RESOURCE.000 offset-table entry for the active actor bank.
	virtual uint resource000ActorBankTableEntry() const;
	// Actor path step table used to advance cels while walking.
	virtual const byte *actorPathStepDeltaTable() const;
	// Byte count for the actor path step table.
	virtual uint actorPathStepDeltaTableSize() const;
	// Optional palette chunk replacing chunk 1 for scene state.
	virtual int alternatePaletteResourceChunkIndex() const;
	// Whether the optional alternate palette chunk is active.
	virtual bool isAlternatePaletteResourceActive() const;
	// Whether 0xff fill pixels need conversion after fill expansion.
	virtual bool shouldConvertSavedFramebufferFF() const;
	// Allows scenes to skip individual arena chunks.
	virtual bool shouldLoadArenaChunk(uint index) const;
	// Whether to run scene-specific side effects after loop exit.
	virtual bool shouldRunExitSideEffectsAfterLoop() const;
	// Runs scene-specific side effects after loop exit.
	virtual void runExitSideEffectsAfterLoop();

	// Scene Hooks
	// Whether the scene replaces default preview initialization.
	virtual bool hasCustomPreviewState() const;
	// Initializes scene-local preview/entry state.
	virtual void initializeCustomPreviewState();
	// Whether the scene replaces the default composite renderer.
	virtual bool hasCustomComposite() const;
	// Draws a scene-specific frame composite.
	virtual void drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode);
	// Decides if the speech/secondary actor should be drawn.
	virtual bool shouldDrawSecondaryActorInPlayableComposite() const;
	// Whether the scene has a custom entry sequence.
	virtual bool hasCustomEntrySequence() const;
	// Runs the scene-specific entry sequence.
	virtual void runCustomEntrySequence();
	// Initializes scene-specific state before the gameplay loop.
	virtual bool prepareCustomGameplayLoop();
	// Advances scene-specific timers and animation during gameplay.
	virtual bool advanceCustomGameplayLoop(uint32 delta);
	// Handles scene-specific action handler ids.
	virtual bool dispatchCustomSceneAction(uint16 handlerId);
	// Adjusts a target click to a legal walkable point.
	virtual bool adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const;
	// Lets scenes alter routing for an intermediate region hop.
	bool customizeRouteSegment(byte currentRegion, byte nextRegion, const ActorPathBuildState &state,
		const ScenePoint &boundary, int &requestedFacing, bool &restoredStepDeltas) override;
	// Lets scenes alter routing for the final path segment.
	bool customizeRouteFinal(byte currentRegion, byte targetRegion, const ActorPathBuildState &state,
		int targetX, int targetY, int &requestedFacing, bool &restoredStepDeltas) override;
	// Applies scene state to hotspot maps and framebuffer patches.
	virtual bool applyCustomSceneStateToHotspotsAndPatches(byte selector);
	// Whether primary speech should drive primary actor animation.
	virtual bool shouldAnimatePrimarySpeechLine() const;
	// Updates the legacy primary-left speech animation frame.
	virtual void setPrimaryLeftSpeechFrame(byte frameIndex);
	// Describes ambient sound/music behavior for this scene.
	virtual AmbientAudioProfile ambientAudioProfile() const;
	// Receives frame hooks from action overlay playback.
	virtual void handleActionOverlayFrameHook(byte hookId, uint frame);

	// Ambient Audio
	// Builds a simple looping ambient audio profile.
	AmbientAudioProfile createLoopingAmbientAudioProfile(byte volumePercent) const;
	// Builds a random sound/music ambient audio profile.
	AmbientAudioProfile createRandomAmbientAudioProfile(byte soundFirstCueId, byte soundCueCount,
		byte soundVolumePercent, byte soundProbabilityModulus, byte musicFirstCueId,
		byte musicCueCount, byte musicVolumePercent, byte musicProbabilityModulus) const;

	// Actor Save State
	// Checks whether saved actor pose belongs to the current scene state.
	bool hasSavedActiveActorPoseForCurrentState() const;
	// Restores active actor position/facing from game state.
	void restoreActiveActorPoseFromGameState();
	// Saves active actor position/facing into game state.
	void syncActiveActorPoseToGameState();

	// Framebuffers
	// Allocates the fixed CLUT8 scene framebuffers.
	void initializeFramebuffers();
	// Returns the byte count of one full scene framebuffer.
	uint framebufferByteCount() const;
	// Returns a mutable pixel pointer for an owning scene framebuffer.
	byte *framebufferPixels(Graphics::ManagedSurface &surface);
	// Returns a const pixel pointer for an owning scene framebuffer.
	const byte *framebufferPixels(const Graphics::ManagedSurface &surface) const;
	// Copies the current base framebuffer into the scene framebuffer.
	void copyBaseFramebufferToSceneFramebuffer();
	// Copies the original base framebuffer back to the mutable base framebuffer.
	void restoreBaseFramebufferFromOriginal();
	// Tests a flat offset into the fixed scene framebuffer.
	bool isFramebufferOffsetValid(uint offset) const;
	// Reads one pixel from the saved scene framebuffer.
	byte savedFramebufferPixelAt(uint offset) const;

	// Resource Loading
	// Loads all scene resources and runtime tables.
	bool load();
	// Loads RESOURCE.000 offset and size tables.
	bool loadResource000RuntimeTables(Common::Array<byte> &offsetTable, Common::Array<byte> &sizeTable);
	// Loads active and secondary actor banks from RESOURCE.000.
	bool loadResource000ActorBank(const Common::Array<byte> &offsetTable, const Common::Array<byte> &sizeTable);
	// Loads actor palette colors from RESOURCE.000.
	bool loadResource000ActorPalette(const Common::Array<byte> &offsetTable);
	// Loads global inventory verb/relation action tables.
	bool loadResource000InventoryActionTables(const Common::Array<byte> &offsetTable);
	// Loads scene and inventory text rows from RESOURCE.003.
	bool loadStage003SceneRows();
	// Loads a fixed-size scene chunk into a preallocated buffer.
	bool loadFixedChunk(uint index, Common::Array<byte> &destination, uint fixedSize);
	// Loads a fixed-size scene chunk into a framebuffer surface.
	bool loadFixedChunk(uint index, Graphics::ManagedSurface &destination, uint fixedSize);
	// Loads a variable-size scene chunk into a resized buffer.
	bool loadVariableChunk(uint index, Common::Array<byte> &destination);
	// Appends an arena chunk and records its arena offset.
	bool loadArenaChunk(uint index);
	// Initializes actor depth tables from metadata and palette maps.
	bool initializeActorDepthTables();
	// Applies scene-specific depth threshold changes for draw order.
	void updateActorDepthThresholds(byte actorDrawOrderMode);
	// Expands fill runs into the saved background framebuffer.
	void expandFillRunsToSavedFramebuffer();
	// Initializes walkable regions and route boundary tables.
	bool initializeScenePathTables();

	// Rendering
	// Initializes preview state, delegating to scene custom code if needed.
	void initializePreviewState();
	// Initializes the default preview state.
	void initializeDefaultPreviewState();
	// Draws the initial preview frame without actors.
	void drawPreviewComposite();
	// Draws a cutscene frame with explicit actor parameters.
	void drawCutsceneComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode = 0);
	// Draws the current playable frame.
	void drawPlayableComposite();
	// Draws active and secondary actors in their shared ordering.
	void drawActiveAndSecondaryActorFrames(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		int minimumYExclusive);
	// Draws one active actor cel.
	void drawActiveActorFrame(byte facing, byte cel, int worldX, int worldY, int minimumYExclusive);
	// Draws one secondary actor frame and returns its bottom Y.
	int drawSecondaryActorFrame(byte facing, byte frame, int worldX, int worldY);
	// Draws an actor run stream with optional depth clipping.
	int drawActorRun(const Common::Array<byte> &runStreams, uint cursor, uint runBase, uint runCount,
		int spriteX, int spriteY, int minimumYExclusive, int actorWorldY, uint *nextCursor = nullptr);
	// Draws active actor palette-remap runs used by original walking shadows.
	int drawActorPaletteRemapRun(const Common::Array<byte> &runStreams, uint cursor, uint runBase, uint runCount,
		int spriteX, int spriteY, int minimumYExclusive, int actorWorldY);
	// Runs the scene entry cutscene or default preview.
	void runEntryCutscene();
	// Runs an entry walking path from offscreen/door to target.
	void runEntryPath(int startX, int startY, byte startFacing, int targetX, int targetY);

	// Gameplay Loop Delegate
	// Runs the common point-and-click gameplay loop.
	bool runBasicGameplayLoop();
	// Exposes loaded scene hotspots to the gameplay loop.
	const SceneHotspotTable &hotspots() const override;
	// Exposes the saved background for panel/cursor drawing.
	const Graphics::Surface &savedFramebuffer() const override;
	// Current horizontal viewport offset.
	uint16 viewportXOffset() const override;
	// Current vertical viewport offset.
	uint16 viewportYOffset() const override;
	// Gameplay-loop hook for per-loop initialization.
	void prepareGameplayLoop() override;
	// Gameplay-loop hook for per-frame advancement.
	void advanceGameplayLoop(uint32 delta) override;
	// Gameplay-loop hook for drawing the scene frame.
	void drawGameplayFrame() override;
	// Gameplay-loop hook for presenting scene, captions, and panel.
	void presentGameplayFrame(const SceneHoverCaption &hoverCaption, const GameplayPanelState &panelState) override;
	// Marks the hardware palette as needing a full scene restore.
	void invalidatePresentationPalette() override;
	// Provides a palette for the ScummVM options menu.
	void prepareOptionsMenuPalette(Common::Array<byte> &palette) const override;
	// Provides the active scene SFX archive for options-menu test playback.
	Common::Path optionsMenuSoundBank0ArchiveName() const override;
	// Provides the fixed voice sample used by options-menu test playback.
	bool optionsMenuSpeechPreviewSampleId(uint16 &sampleId) const override;
	// Stops scene-owned speech/SFX while the options menu is open.
	void suspendAudioForOptionsMenu() override;
	// Tells the gameplay loop when scene state has exited.
	bool shouldExitGameplayLoop() const override;
	// Returns an inventory item display name for the active owner.
	Common::String inventoryItemName(byte owner, byte itemId) const override;
	// Speaks a shared inventory response line.
	void beginSharedInventorySpeechLine(uint16 rowIndex, byte frameIndex) override;
	// Returns a random shared inventory speech frame.
	byte randomSharedInventorySpeechFrame(byte maxFrameIndex) override;
	// Plays a shared inventory sound effect.
	void playSharedInventorySound(byte sampleId) override;
	// Shows the travel screen viewer.
	void showTravelScreenViewer() override;
	// Handles a scene left-click from the gameplay loop.
	void handleLeftClick(const GameplayLoopCursorState &state) override;
	// Handles an inventory item click from the gameplay loop.
	void handleInventoryItemClick(const GameplayLoopCursorState &state) override;

	// Ambient Runtime
	// Advances ambient sound and music timers.
	void updateAmbientAudioAndMusicCues(uint32 delta);
	// Clears ambient audio state for a new scene loop.
	void resetAmbientAudioState();
	// Updates ambient sound according to the profile.
	void updateAmbientSoundCue(const AmbientAudioProfile &profile);
	// Updates ambient music according to the profile.
	void updateAmbientMusicCue(const AmbientAudioProfile &profile);

	// Speech Animation
	// Advances the secondary actor speech animation timer.
	void advanceSecondaryActorSpeechAnimation(uint32 delta);
	// Advances one secondary actor speech frame.
	void advanceSecondaryActorSpeechFrame();
	// Advances the primary-left speech frame.
	void advancePrimaryLeftSpeechFrame();
	// Advances primary dialogue speech animation.
	void advancePrimaryDialogueSpeechFrame(uint32 delta);

	// Scene Action Dispatch
	// Resolves and dispatches a scene click action.
	void processSceneActionClick(const GameplayLoopCursorState &state);
	// Resolves and dispatches an item-to-scene relation click.
	void processSceneRelationClick(const GameplayLoopCursorState &state, byte itemId);
	// Handles generic/shared inventory action handler ids before scene dispatch.
	bool dispatchGenericInventoryAction(const GameplayLoopCursorState &state);
	// Dispatches an action handler id to custom or generic code.
	void dispatchSceneAction(uint16 handlerId);
	// Handles generic/shared action handler ids.
	bool dispatchGenericSceneAction(uint16 handlerId);

	// Pathfinding
	// Walks the active actor to a target and optional final pose.
	bool walkActiveActorTo(int targetX, int targetY, byte finalFacing, byte finalCel, bool cancelOnSkip = false);
	// Applies default or scene-specific walk target adjustment.
	bool adjustWalkTargetToFloorMask(int &targetX, int &targetY) const;
	// Builds path frames using palette-region routing.
	void queueActorPathWithPaletteRegionRouting(int startX, int startY, int targetX, int targetY,
		byte finalFacing, byte finalCel);
	// Appends path frames between two points.
	void buildActorPathFramesBetweenPoints(ActorPathBuildState &state, int targetX, int targetY,
		byte finalFacing, byte finalCel, int requestedFacing);
	// Appends one actor path frame from path build state.
	void appendActorPathFrame(const ActorPathBuildState &state);
	// Finds the nearest boundary point for a palette-region hop.
	ScenePoint nearestPaletteRouteBoundaryPoint(int startX, int startY, byte currentRegion, byte nextRegion) const;
	// Finds the best boundary point toward a final destination.
	ScenePoint bestPaletteRouteBoundaryPoint(int startX, int startY, int targetX, int targetY,
		byte currentRegion, byte targetRegion) const;
	// Reads the palette-region id at scene coordinates.
	byte paletteRegionAt(int x, int y) const override;
	// Reads the walkable mask value at scene coordinates.
	byte walkableMaskAt(int x, int y) const;
	// Chooses actor facing for a movement vector.
	byte calculateMovementFacingForPath(int fromX, int fromY, int toX, int toY, int requestedFacing) const;
	// Calculates walk steps needed for one axis.
	uint calculateWalkStepCountForAxisDelta(int startAxis, int targetAxis, byte facing, byte cel) const;
	// Returns the next cel in the walking cycle.
	byte nextActorPathCel(byte cel) const;
	// Returns the movement delta for a facing/cel pair.
	uint actorPathStepDelta(byte facing, byte cel) const;
	// Restores the actor path step table to scene defaults.
	void resetActorPathStepDeltas();
	// Calculates facing from one point toward another.
	byte calculateFacingTowardPoint(int fromX, int fromY, int toX, int toY) const;

	// Scene State And Hotspots
	// Applies state-driven hotspot and framebuffer patches.
	void applySceneStateToHotspotsAndPatches(byte selector);
	// Rebuilds the walkable palette-region mask from full region data.
	void rebuildWalkablePaletteMask();

	// Inventory
	// Checks whether the active owner has an inventory item.
	bool hasInventoryItem(byte itemId) const;
	// Adds an item to the active owner inventory.
	void addInventoryItem(byte itemId);
	// Removes an item from the active owner inventory.
	void removeInventoryItem(byte itemId);

	// Dialogue Menu Delegate
	// Provides dialogue menu text for a stage/text row.
	Common::String dialogueMenuText(byte stageId, byte textRowId) const override;
	// Dialogue-menu hook for advancing the underlying scene.
	void advanceDialogueMenu(uint32 delta) override;
	// Dialogue-menu hook for drawing the underlying scene.
	void drawDialogueMenuFrame() override;
	// Dialogue-menu hook for presenting the menu frame.
	void presentDialogueMenuFrame(const DialogueMenuState &state) override;

	// Action Overlays
	// Runs an action overlay with default options.
	void runActionOverlay(uint chunkIndex, uint descriptorCount, const byte *frameMap, uint frameMapSize,
		uint32 frameMillis);
	// Runs an action overlay from a named specification.
	void runActionOverlay(const ActionOverlaySpec &spec);
	// Runs an overlay while hiding the active actor.
	void runHiddenActorActionOverlay(uint chunkIndex, uint descriptorCount, const byte *frameMap,
		uint frameMapSize, uint32 frameMillis);
	// Runs an overlay while forcing the active actor visible.
	void runVisibleActorActionOverlay(uint chunkIndex, uint descriptorCount, const byte *frameMap,
		uint frameMapSize, uint32 frameMillis);

	// Speech Animation Hooks
	// Maps a primary dialogue animation group to a base frame.
	virtual byte primarySpeechAnimationBaseFrame(byte animationGroup) const;
	// Updates a primary dialogue animation group frame.
	virtual void setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex);
	// Notifies scenes that a primary dialogue animation started.
	virtual void primarySpeechAnimationStarted(byte animationGroup, byte baseFrame);
	// Notifies scenes after a primary dialogue animation returns to its base frame.
	virtual void primarySpeechAnimationRestored(byte animationGroup, byte baseFrame);

	// Timing
	// Waits while pumping events and redrawing scene frames.
	bool waitSceneMillis(uint32 millis);

	// Viewport
	// Resets viewport offsets from scene defaults.
	void resetViewportFromScene();
	// Advances automatic viewport scrolling.
	void advanceViewportScroll(uint32 delta);

	// Speech Overlays
	// Clears the secondary speech overlay.
	void clearSpeechOverlay();
	// Clears all speech overlays.
	void clearAllSpeechOverlays();
	// Draws the active secondary speech overlay.
	void drawSpeechOverlay();
	// Draws a specific speech overlay.
	void drawSpeechOverlay(const SpeechOverlay &overlay);

	// Resource Sprite Layers
	// Draws a resource sprite using a frame map.
	void drawMappedSpriteFrame(uint chunkIndex, uint descriptorCount, const byte *frameMap, uint frameMapSize, byte frameIndex);
	// Restores current and previous sprite-layer bounds from background.
	void restoreResourceSpriteLayerBackground(const ResourceSpriteLayer &layer, const Graphics::Surface &background);
	// Draws the current frame of a resource sprite layer.
	void drawResourceSpriteLayer(const ResourceSpriteLayer &layer);
	// Draws an ordered stack of temporary resource sprite layers.
	void drawTransientLayers(const TransientLayerCompositor &compositor);
	// Draws the currently active action overlay layer.
	void drawActionOverlayLayer();

	// Resource Delta Clips
	// Draws one resource delta frame directly over the current scene framebuffer.
	void drawClipFrameDeltaFromResource(const Common::Array<byte> &resource, uint32 frameTableOffset,
		uint32 chunkSize, uint tableEntryCount, byte frameIndex);
	// Draws one resource delta frame from a loaded scene-arena chunk.
	void drawClipFrameDelta(uint chunkIndex, uint tableEntryCount, byte frameIndex);
	// Plays a resource delta frame range from caller-owned data.
	void playDeltaClipFromResource(const Common::Array<byte> &resource, uint32 frameTableOffset,
		uint32 chunkSize, uint tableEntryCount, uint frameCount, uint32 frameMillis, uint firstFrame = 0);
	// Plays a resource delta frame range from a loaded scene-arena chunk.
	void playDeltaClip(uint chunkIndex, uint tableEntryCount, uint frameCount, uint32 frameMillis,
		uint firstFrame = 0);
	// Waits for a delta clip frame while pumping only skip/quit/menu events.
	bool waitDeltaClipFrameMillis(uint32 millis);

	// Speech Playback
	// Runs a scene-local secondary speech line.
	void beginSecondarySpeechLine(uint16 rowIndex, byte frameIndex);
	// Starts a secondary speech line without waiting for completion.
	bool startSecondarySpeechLine(uint16 rowIndex, byte frameIndex);
	// Runs a fixed/shared secondary speech line.
	void beginStaticSecondarySpeechLine(uint16 rowIndex, byte frameIndex);
	// Runs a primary speech line with explicit placement/color.
	void beginPrimarySpeechLine(uint16 rowIndex, byte frameIndex, uint16 centerX, uint16 topY,
		byte red, byte green, byte blue);
	// Runs a primary speech line tied to an animation group.
	void beginPrimarySpeechLineWithAnimationGroup(uint16 rowIndex, byte frameIndex, uint16 centerX, uint16 topY,
		byte red, byte green, byte blue, byte animationGroup);
	// Runs a primary-left speech line.
	void beginPrimaryLeftSpeechLine(uint16 rowIndex, byte frameIndex);
	// Resolves cue data and runs a speech overlay.
	void runSpeechLine(SpeechOverlay &overlay, uint16 rowIndex, byte frameIndex, uint16 centerX, uint16 topY,
		byte colorIndex, bool useRequestedTop, bool animatePrimaryLeft, bool animatePrimaryDialogue,
		byte primaryAnimationGroup = 0xff);
	// Plays speech audio/text from an already resolved cue.
	void runSpeechCue(SpeechOverlay &overlay, uint16 textRecordId, byte continuationCount, uint16 voiceSampleId,
		uint16 centerX, uint16 topY, byte colorIndex, bool useRequestedTop, bool animatePrimaryLeft,
		bool animatePrimaryDialogue, byte primaryAnimationGroup = 0xff);
	// Looks up a scene-local speech cue.
	bool getStage003Cue(uint16 rowIndex, byte frameIndex, uint16 &textRecordId, byte &continuationCount,
		uint16 &voiceSampleId) const;
	// Looks up a fixed/shared speech cue.
	bool getStaticSpeechCue(uint16 rowIndex, byte frameIndex, uint16 &textRecordId, byte &continuationCount,
		uint16 &voiceSampleId) const;
	// Wraps actor speech text near an actor anchor.
	void wrapActorSpeechText(const Common::String &text, uint16 anchorSceneX, Common::Array<Common::String> &lines) const;
	// Returns a decoded RESOURCE.003 large text record.
	Common::String getResource003LargeTextRecord(uint16 recordId) const;
	// Measures speech text using the original font.
	uint actorSpeechTextWidth(const Common::String &text) const;
	// Measures the widest line in a speech overlay.
	uint speechOverlayTextWidth(const SpeechOverlay &overlay) const;
	// Calculates clamped speech overlay screen bounds.
	void calculateSpeechOverlayBounds(SpeechOverlay &overlay, int centerX, int topY, bool useRequestedTop,
		int actorWorldY);
	// Calculates secondary speech bounds from actor coordinates.
	void calculateSecondarySpeechBounds(int actorWorldX, int actorWorldY);
	// Waits for speech playback or a fallback text delay.
	bool waitForSpeechOrDelay(uint32 fallbackMillis, bool animatePrimaryLeft);

	// Palette
	// Updates one 6-bit palette entry in the scene palette.
	void setPaletteEntry6Bit(byte colorIndex, byte red, byte green, byte blue);
	// Reads one component from a 6-bit scene palette entry.
	byte paletteEntryComponent6Bit(byte colorIndex, uint component) const;
	// Applies bottom-panel colors to the current palette.
	void applyGameplayPanelPalette();

	// Panel And Presentation
	// Draws the active gameplay panel.
	void drawGameplayPanel(Graphics::Surface &surface, const GameplayPanelState &panelState);
	// Draws the verb panel.
	void drawVerbPanel(Graphics::Surface &surface, const GameplayPanelState &panelState);
	// Draws the inventory panel.
	void drawInventoryPanel(Graphics::Surface &surface, const GameplayPanelState &panelState);
	// Uploads palette and presents scene/panel/menu layers.
	void presentFrame(const SceneHoverCaption *hoverCaption = nullptr,
		const GameplayPanelState *panelState = nullptr, const DialogueMenuState *dialogueMenuState = nullptr);

	// Event Pumping
	// Polls system events and optionally treats input as skip.
	bool pollEvents(bool allowSkip);

	// Engine And Resources
	HollywoodEngine *_vm;
	PlayableSceneConfig _config;

	// Scene Resource State
	SceneResources _resources;
	ResourceChunkTable &_sceneChunkTable;
	uint32 (&_resourceChunkOffsets)[HollywoodEngine::kResourceChunkCount];
	uint32 &_resourceArenaCursor;
	Common::Array<byte> &_resourceArena;
	Common::Array<byte> &_metadata;

	// Scene Surface State
	SceneSurfaceState _surfaceState;
	Common::Array<byte> &_paletteResource;
	Common::Array<byte> &_paletteCurrent;
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

	// Actor Resources
	Common::Array<byte> _resource000OffsetTable;
	Common::Array<byte> _resource000SizeTable;
	Common::Array<byte> _activeActorRunStreams;
	Common::Array<byte> _secondaryActorRunStreams;
	Common::Array<ActiveActorSpriteDescriptor> _activeActorDescriptors;
	Common::Array<SecondaryActorSpriteDescriptor> _secondaryActorDescriptors;

	// Scene Text Store
	SceneTextStore _textStore;
	Common::Array<byte> &_stage003DecodeKey;
	Common::Array<byte> &_stage003StageBlock;
	Common::Array<byte> &_stage003SmallRows;
	Common::Array<byte> &_stage003LargeRows;
	Common::Array<byte> &_staticSpeechCueDescriptors;
	Common::Array<byte> &_inventoryOwnerSmallRows;
	Common::Array<byte> &_inventoryOwnerLargeRows;

	// Pathfinding State
	ActorPathController _pathController;
	Common::Array<ScenePoint> &_routeBoundaryPoints;
	Common::Array<byte> &_routeSteps;
	Common::Array<ActorPathFrame> &_actorPathFrames;
	Common::Array<byte> &_actorPathStepDeltas;

	// Runtime Systems
	GameplayPanelArt _panelArt;
	SceneHotspotTable _hotspots;
	SoundBank0Player _soundBank0;
	SoundBank0Player _ambientSoundBank0;
	Common::RandomSource _random;

	// Speech Runtime State
	SpeechController _speechController;
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

	// Action Overlay Runtime State
	ActionOverlayPlayer _actionOverlayPlayer;
	bool &_actionOverlayVisible;
	ResourceSpriteLayer &_actionOverlayLayer;
	byte &_actionOverlayChunkIndex;
	byte &_actionOverlayDescriptorCount;
	byte &_actionOverlayFrameIndex;
	bool &_hideActiveActor;

	// Scene Runtime State
	bool _inventoryItems[121];
	byte _sceneStateFlags[8];
	uint32 _ambientMusicTimerAccumulator;
	byte _previousAmbientMusicTrackId;
	byte _currentAmbientSoundCueId;
	byte _previousAmbientSoundCueId;
	uint16 _viewportXOffset;
	uint16 _viewportMinXOffset;
	uint16 _viewportMaxXOffset;
	uint32 _viewportScrollTimerAccumulator;
	bool _actorPathPlaybackActive;
	int _activeActorWorldX;
	int _activeActorWorldY;
	byte _activeActorFacing;
	byte _activeActorCel;
	byte _activeActorDrawOrderMode;
	byte _lastSceneActionItemId;
	byte _lastInventoryActionItemId;
	byte _lastInventoryPrimaryItemId;
	bool _skipRequested;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_PLAYABLE_SCENE_H
