# RIPPER.LE Architecture

## Startup

- `_entry` at `0x592f0` jumps to `RuntimeStartupAndExit` at `0x59368`.
- `RuntimeStartupAndExit` performs the DOS-extender runtime bootstrap and calls
  `RunRuntimeInitAndProgramMain` at `0x6b118`.
- `RunRuntimeInitAndProgramMain` transfers to the first game-owned coordinator,
  `RunGameStartupAndMainLoop` at `0x100c2`.
- The game coordinator defaults to `ripper.run`, initializes resources, and
  plays `LOGO.AVI` with `PollPresentationEscOrSpaceCommand` at `0x49039`, so
  Escape skips the presentation and Space pauses it.
- `RunStartupFrontEndLoop` at `0x10778` loops `RIP_OPEN.SMK` as the animated
  menu background. It creates five UI controls from the rectangle table at
  `0x1001b`, uses cursor 14 normally and cursor 16 over a control, and maps
  control IDs `0x7a8` through `0x7ac` to New Game, Continue, Restore Game,
  View Intro, and Exit. Escape selects Exit.
- The same front-end loop loads `TITLE0.WAV`, `TITLE1.WAV`, and `TITLE2.WAV`
  from `SOUND.PL`. `TITLE0` loops under the menu, `TITLE1` is queued when the
  hovered control changes, and `TITLE2` is queued when a control is selected.
- `RunGameStartupAndMainLoop` handles those menu results: New Game resets game
  state and enters `RunSceneScriptLoop` at `0x124e9`, Continue restores the
  startup state, Restore Game opens the saved-game path, View Intro plays
  `PROINT.AVI`, and Exit shuts down.

## Input

- Keyboard commands are polled through `PollKeyboardCommand` at `0x4d364`.
  Commands use ASCII values or BIOS-style extended scan codes.
- `MouseDriverEventCallback` at `0x562f0` accumulates movement and button-edge
  state. `ServiceMouseCallbackStateTimer` at `0x56240` publishes that state to
  the UI selection layer.
- Packetized presentations optionally use
  `PollPresentationEscOrSpaceCommand` at `0x49039`: Escape stops playback and
  Space toggles its control/pause state. The callback does not poll the mouse.

## Resources

- `LoadAssetLibraryDirectoryFromOpenHandle` at `0x6ad80` reads a six-byte
  count/directory-offset prefix. A following `2BIL` tag selects the modern
  10-byte header and 20-byte records; otherwise the payload begins at byte six
  and the directory uses 12-byte legacy records.
- `OpenIndexedContainerEntryReadOnly` at `0x55a4c` performs case-insensitive
  member lookup and seeks the shared library handle to the selected member.
- `SCRIPT.PL` contains compiled `.RUN` scene scripts. The default entry is
  `RIPPER.RUN`, which leads to the initial `BA0.RUN` scene.
- Cursor `.PL` members are nested legacy asset libraries whose entries contain
  the game's custom compressed bitmap format. `DecodeCustomBitmapAsset` at
  `0x53f60` reads the 0x1c-byte bitmap header and
  `ExpandCustomBitmapCompressedPixels` at `0x53de0` expands its nibble-coded
  delta, literal, and repeat commands. The original descriptor stores height
  before width; `SampleBitmapDescriptorPaletteIndexAtPoint` at `0x61f10`
  resolves pixels in ordinary row-major order as `width * y + x`. Cursor
  `.BBM` members are standard IFF PBM images and are decoded by Ripper's
  `Image::IFFDecoder` subclass; `DecodeIffBitmapAssetToDescriptor` at
  `0x6aca4` maps their dimensions into the same height/width descriptor order.
  `ReadIffBitmapHeaderAndPayloadBuffer` at `0x6a6d4` rounds PBM scanlines to
  an even byte count before decoding them. For example, the 13-by-19
  `MOUSE14.BBM` and `MOUSE16.BBM` cursors store 14 bytes per BODY row; the
  padding byte is not part of the displayed width.

- `LoadFrontEndToolbarResources` at `0x18678` loads nine nested `TOOLBAR1.PL`
  through `TOOLBAR9.PL` bitmap sequences. Each sequence has ten frames; the
  first frame dimensions are the hit rectangles. `GAMETEXT.TF` is the startup
  resource-string table, and entries 1 through 9 provide the toolbar labels.
- The `7PT_FONT.FNT` member is an NF2T glyph descriptor used by
  `RenderFrontEndActionPreviewSprite` at `0x189b3`. The reimplementation
  decodes the descriptor and its custom bitmap instead of substituting a host
  font. Toolbar bitmap color maps target palette index 0, indices 4 through 9,
  and indices 246 through 255. `ApplySharedDisplayPalettePatch` at `0x205d0`
  restores those bands on every media palette update so interface pixels remain
  stable while the underlying Smacker palette changes. The original captures
  those bands while `LoadStartupBitmapAssetTable` and
  `InitializeSharedPresentationTemplates` prepare startup indexed assets; the
  reimplementation uses the first `MNU0` bitmap palette as that startup
  palette and applies only the reserved bands to later Smacker presentations.

## Scene Toolbar

- `RunFrontEndActionMenu` at `0x18b3a` is entered by
  `PollInteractionAndResolveSelection` at `0x13c8d` whenever the published
  vertical selection coordinate is below `0x32` (50). It creates nine controls
  with action IDs `0x514` through `0x51c`, lays them out right-to-left from
  x=`0x276` (630), and subtracts each bitmap width plus five pixels.
- The toolbar remains modal while the pointer is in that top 50-pixel band;
  scene hotspots do not receive those events. Scene Smacker coordinates are
  relative to the original 640x400 display page, whose top edge follows that
  band at y=50. `RefreshDisplayDimensionsAndInputBounds` at `0x5d8e0` derives
  the maximum input coordinate from the active descriptor height, so y>=400 is
  outside the scene hit-test region.
- While a control is hovered, `RunFrontEndActionMenu` advances that action's
  frame index once per extended DOS tick. Each icon is vertically centered in
  the 50-pixel band. The preview gate uses
  `front_end_action_preview_enabled` and a default delay of 27 DOS ticks from
  `PersistentSettingsBlob` (offsets 6 and 17). Once enabled,
  `RenderFrontEndActionPreviewSprite` builds a four-pixel-padded tooltip at
  the pointer's horizontal coordinate and 20 pixels below its vertical
  coordinate using the startup string and NF2T font. The preview uses palette
  index 0 for its border, index 253 for its fill, and index 4 for glyphs.
- `ServiceUiControlStateSelection` at `0x4a912` hit-tests the current pointer
  before returning a control ID. A toolbar press therefore dispatches only
  when its release remains over the same action; leaving the control cancels
  the pending selection.
- `PollInteractionAndResolveSelection` at `0x13c8d` checks the published
  vertical coordinate before scene interaction handling, so the toolbar band
  remains available during BA0 first-frame previews, normal scenes, and an
  active opcode `0x17` dialogue chooser. `RunFrontEndActionMenu` snapshots and
  restores both the chooser registry and scene UI-control list while it owns
  input. That band uses cursor `0xe` (14), matching the white cursor selected
  by `RunStartupFrontEndLoop` at `0x10778`; scene and chooser cursor selection
  resumes below y=50.
- Action dispatch at `0x190b7` maps the nine controls to remote setup, world
  map, inventory, WAC, save, restore, options, help, and quit. The ScummVM
  toolbar preserves the click/release routing. Action `0x515` and scene-entry
  action 2 both enter the shared `WorldMap` subsystem; action `0x517` enters
  WAC, and actions `0x518`/`0x519` enter the shared save/restore chooser after
  removing the toolbar presentation. Remote setup, inventory, options, help,
  and quit remain explicit stubs.
- `DispatchSceneEntryAction` at `0x36892` routes action 2 to the same
  `HandleSceneSelectionAction` at `0x191e2` used by toolbar action `0x515`.
  `RunSceneSelectionMenu` at `0x20808` groups the 25 travel-entry flags 20
  through 44 through the table at `0x20702`, exposing a location when any of
  its entries is unlocked. The resulting 15 locations use game-text resources
  100 through 114 for their names and resources 150 through 164 for their
  destination script patterns. `ResolveHighestSetSelectionFlag` at `0x20394`
  supplies the highest enabled chapter flag from 1 through 4 to those patterns.
  The menu loads `worldmap.pcx` as its 640x400 background and advances
  `worldcel.smk` beside the location chooser. A successful selection returns
  control code `-3` with the resolved scene script; cancellation leaves the
  current scene active.

## Save and Restore

- `RunGameStartupAndMainLoop` at `0x100c2` keeps the emergency Continue path
  separate from manual saves. Front-end result 2 restores the emergency save,
  while result 3 enters `RunSaveRestoreSlotMenu` at `0x1a1bd` in restore mode.
  `DispatchFrontEndAction` at `0x190b7` sends toolbar actions `0x518` and
  `0x519` to the same routine in save and restore modes respectively.
- `RunSaveRestoreSlotMenu` manages 20 manual `ripper%d.sav` slots. Save mode
  captures the current indexed presentation, accepts a description of up to 60
  characters, and confirms replacement of occupied slots. Restore mode exposes
  only populated slots and displays their descriptions and preview images.
  ScummVM maps those 20 slots to its standard save chooser and reserves a
  separate autosave slot for the original emergency Continue state.
- `WriteEmergencySaveGame` at `0x1ae3c` writes the current scene identity,
  scene-resume state, runtime flags, played media state, audio triggers,
  briefing state, puzzle state, description, and preview. `LoadSavedRunStateBlob`
  at `0x1b492` reconstructs that data, and `RestoreSavedRunState` at `0x1b8dd`
  reinstates flags, concurrent scene context, media state, and the active scene.
  The ScummVM format is versioned and engine-local; it stores the confirmed
  script state, the currently supported loaded/playing scene-audio trigger, and
  the indexed 640x400 framebuffer and palette so static scene presentations
  resume at the same visible boundary without serializing DOS pointers from the
  original fixed block.
- `WriteConfiguredSaveCheckpointAndCleanupRuntime` at `0x1b274` writes the
  emergency save when the scene-script loop returns to the front end, then
  clears active audio triggers and the concurrent scene runtime. The
  reimplementation writes its reserved Continue slot when an active scene loop
  exits normally; Continue never substitutes a manual slot, while Restore Game
  always opens the manual chooser.

## WAC

- `DispatchFrontEndAction` at `0x190b7` routes toolbar action `0x517` to the
  modal `RunWacFrontEndLoop` at `0x21865`. The loop snapshots the active scene
  presentation and chooser state, sets the default cursor to 14, loads the
  640x400 `wac.pcx` background, and creates four bottom-row controls from
  `wac0.bbm` through `wac3.bbm`. Controls use cursor 16.
- `g_astWacFrontEndButtonLayouts` at `0x84156` places those controls at y=349
  and x=172, 252, 326, and 390. Their action IDs are `0x1900` (exit), `0x2000`
  (object database), `0x3100` (text viewer), and `0x3b00` (help). Escape also
  leaves the modal loop. The reimplementation restores the indexed scene and
  palette when the WAC loop exits; database, text-viewer, and help dispatches
  remain explicit subsystem stubs at this boundary.
- `RunWacFrontEndLoop` loads the wildcard bitmap sets `wacwn1*.bbm` and
  `wacwn2*.bbm`. `ServiceWacSceneIdleEffects` at `0x21da2` advances both sets
  every three extended DOS ticks and redraws them at (64, 21) and (460, 19).
  The service remains active while WAC subviews own input, so the two upper
  window animations continue over the object database and puzzle scenes.
- The `0x2000` action enters `RunWacInventorySelectionLoop` at `0x2252a`.
  `BuildWacInventorySelectionMenu` at `0x22c91` scans 30 entries, using shared
  named/milestone flags `0x46 + entry` as availability bits and game-text IDs
  `0xdc + entry` as labels. It stores the original entry byte separately from
  the visible chooser row, so locked entries do not change dispatch IDs. The
  chooser control ID is `0x73a`; the call site supplies x=400, y=50, width=190,
  and height=282. `InitializeSharedPresentationTemplates` at `0x1196f` gives
  this chooser its own `wacmnu0` through `wacmnu15` skin and `small.fnt`.
  `TileChooserControlFrame` at `0x54fbe` uses the first nine bitmaps as a 3x3
  frame. Its original presentation coordinates are transposed relative to
  screen x/y; after translation, `wacmnu0` through `wacmnu8` are the row-major
  top, middle, and bottom tiles. `ComputeChooserControlLayout` at `0x54a74`
  applies a 20-pixel heading inset, 6-pixel bottom inset, 5-pixel left inset,
  20-pixel right inset, and 14-pixel rows. The heading, normal rows, and active
  row use indexed glyph colors 248, 251, and 254 over chooser background 4.
  `AnimateWacMenuChooserBlinkCallback` at `0x215e5` alternates the bitmap at
  the chooser origin between `wacmnu0` and `wacmnu15` every five extended DOS
  ticks. `SetWacMenuChooserBlinkActiveStateCallback` at `0x21775` selects the
  alternate bitmap when the chooser becomes active, producing the animated
  upper-left database icon without rebuilding the surrounding tiled frame.
  `RunWacInventorySelectionLoop` keeps the chooser inside
  `RunWacSceneInputLoopUntilExitAction` at `0x221e3`; each input tick services
  the chooser registry and the software selection presentation through
  `ServiceWacSceneInputAction` at `0x21eef`. The ScummVM loop therefore presents
  every tick as well, so cursor movement remains visible while the list itself
  is unchanged.
  Entry 1 dispatches `RunWacMugSelectionScene` at `0x236b9`, entry 2 dispatches
  `PlayMugSelectionCompletionMedia` at `0x2361c`, and the remaining database
  entry handlers are explicit stubs.
- `RunWacMugSelectionScene` owns nine draggable controls (`0x640` through
  `0x648`) backed by `mug0.smk` through `mug8.smk`. It initially advances each
  32-frame asset to orientation state 2, draws the controls over the WAC media
  viewport at x=50, y=50, width=350, height=282, and patches only palette
  entries 10 through 149 so the WAC interface colors remain available. The WAC
  display service consumes each control's coordinate pair in transposed screen
  order, so the second recovered coordinate is screen x and the first is
  screen y; the solved deltas use the same mapping. The first left click selects
  a piece and promotes its transient drag overlay to the front; the overlay
  follows the pointer without requiring the button to remain held. A second
  left click drops the piece after that button is released. A right click while
  the piece is selected advances eight frames and rotates it to the next of
  four orientation states. The puzzle implementation is isolated under
  `engines/ripper/puzzles/`.
- `RunWacInventorySelectionLoop` keeps the database chooser alive while it
  dispatches `RunWacMugSelectionScene`. The mug scene redraws only the left
  media viewport, so the Object Database panel and its selected Broken Mug row
  remain visible and continue to frame the puzzle on the right.
- After loading the nine controls, `RunWacMugSelectionScene` draws them back to
  front, presents the completed WAC page, and only then calls
  `PlayBlockingAudioClip` at `0x1f0ea` for `q_p_1.wav`. The WAV is resolved
  through startup sound-library handle 2 (`sound.pl`), not only as a loose
  file. The call result is not a puzzle-load guard; a missing voice clip does
  not discard the already-presented controls or leave the scene.
- `PlayBlockingAudioClip` selects cursor index `0x13`, activates its
  presentation, and services only keyboard command polling plus managed audio
  while the clip is active. Mouse transitions therefore cannot select a mug
  piece during the narration. `RunWacMugSelectionScene` activates its normal
  puzzle cursor only after the blocking clip returns and the manipulation loop
  becomes active. ScummVM services the software cursor presentation on every
  blocking-audio and puzzle-input tick; this is the backend equivalent of the
  original UI selection presentation remaining active while those modal loops
  run.
- The mug is solved only when all nine orientation states are zero and every
  piece is within five pixels of its recovered x/y offset from piece 0. The
  completion path centers the 160-by-164 `mug9.smk` at x=143, y=111 in the WAC
  media viewport, plays
  `q_p_2.wav`, then sets shared flags `0x47` and `0x48`; `q_p_1.wav` is played
  before the manipulation loop. Once flag `0x48` is set, selecting either the
  Broken Mug path or database entry 2 replays the completed-mug presentation
  through `PlayMugSelectionCompletionMedia` at `0x2361c`.

## Cursor

- `LoadStartupBitmapAssetTable` at `0x113d0` loads 24 cursor rows from
  `MOUSE0` through `MOUSE23`. Most are animated nested `.PL` libraries;
  cursors 8 and 14 through 17 are single `.BBM` images.
- `InitializeUiSelectionTable` at `0x4a3e4` makes cursor 0 the default.
  Animated selection entries request the shared timer callback at 10 Hz.
- `ExecuteSceneFrameAndInteractions` at `0x13277` stores each interaction
  record's cursor byte at offset `+0x15` in its UI control. Interactions with a
  condition callback initially use cursor 8 instead.
- `ServiceUiControlStateSelection` at `0x4a912` hit-tests the current pointer,
  selects the hovered control's cursor index, and restores cursor 0 when no
  control is hit.
- `ConfigureSceneEntryChooserLayout` at `0x18740` assigns selection index
  `0x10` (cursor 16) to an opcode `0x17` choice list. `ProcessChooserControlInput`
  at `0x57372` applies that cursor only while the pointer is inside the chooser
  row client area. Toolbar controls keep cursor 14, scene hotspots keep their
  script cursor byte, and empty scene space falls back to cursor 0.
- Cursor hotspots are centered except for the explicit startup-table cases:
  cursors 14 and 16 use the top-left pixel, while cursor 15 uses the top edge
  at half its width.

## Scripts And Scenes

- `ScriptOpcode` in `script.h` assigns stable symbolic names to every dispatch
  entry from `0x00` through `0x24`. The explicit byte values and adjacent Ghidra
  handler anchors mirror the callback table at `0x84040`; decoded script data
  retains those serialized values while engine control flow uses the labels.
- `CreateSceneRuntime` at `0x12be7` loads a compiled script and binds its frame,
  interaction, and callback tables.
- `ReadSceneCallbackOpcodeAndArguments` at `0x140e9` decodes callback commands
  and their typed arguments.
- `RunSceneCallbackCommandStream` at `0x14080` dispatches decoded commands.
- The callback handler table at `0x84040` maps opcode `0x15` to
  `HandleSceneEntryPushChoiceRecordAndStepPrompt` at `0x15085`, opcode `0x16`
  to `HandleSceneEntryPushFrameChoiceOnPlayedStateCondition` at `0x150ea`, and
  opcode `0x17` to `HandleSceneEntryChoiceListLifecycle` at `0x1523d`.
  Opcode `0x16` omits a choice once its 16-bit BA0 response-frame result has
  been marked played. Opcode `0x17` presents the remaining records through
  chooser control `0x4e2` and returns control code `-2` with the selected
  response frame.
- `InitializeSharedPresentationTemplates` at `0x1196f` loads `small.fnt` into
  chooser template `0x8a392`; the primary scene-entry chooser template at
  `0x8a2de` reuses that NF2T descriptor. Dialogue rows therefore use the
  `small.fnt` space width, character spacing, per-glyph offsets, transparency,
  and line height. `7pt_font.fnt` belongs to a separate interface template and
  is not the dialogue-choice font.
- The primary chooser's normal glyph template at `0x84034` maps text to palette
  index `0xfb` over background index `0x00`. Its selected glyph template at
  `0x84035` maps text to index `0x04`, while
  `InitializeSharedPresentationTemplates` stores selected background index
  `0xf8` at template offset `+0x44`. `RenderChooserTextRow` at `0x58651`
  selects between those templates from the row state, producing white-on-black
  normal rows and black-on-white selected rows.
- `InitializeUiChoiceControlEntryRows` at `0x4c0b3` binds three visible row
  controls to a list chooser. `ProcessChooserControlInput` at `0x57372` keeps
  a separate first-visible index and moves it one row for the chooser's up and
  down control IDs. The opcode `0x17` lifecycle places those controls five
  pixels beyond the chooser's right edge, aligned to its top and bottom, with
  cursor 16. Startup loads their normal and hover frames in order from
  `mnarrow0.bbm` through `mnarrow3.bbm`.
- `CreateListChooserControl` at `0x56ec2` measures the widest NF2T choice when
  the scene-entry layout supplies a negative width. `ComputeChooserControlLayout`
  at `0x54a74` adds the template padding and centers the resulting chooser,
  instead of stretching dialogue rows across a fixed screen-width rectangle.
- When `HandleSceneEntryChoiceListLifecycle` at `0x1523d` installs the chooser's
  arrow controls, it deactivates the shared selection presentation, dispatches
  two complete dirty-region updates through `DispatchDisplayDirtyRegionUpdate`
  at `0x4e4b0`, and then reactivates presentation. The scene movie owns only
  y=50 through y=349, so the ScummVM presentation rebuild clears the uncovered
  top and bottom indexed bands before drawing the chooser. This prevents pixels
  retained from the preceding presentation from being reinterpreted by the
  active Smacker palette.
- `RunMediaPresentation` at `0x168af` performs the corresponding restoration
  after a controlled response movie: it restores display state, submits a
  full-display dirty-region update, reapplies the display palette, and then
  restores selection presentation state. The ScummVM response path therefore
  rebuilds the same top and bottom bands immediately after type-0 media returns,
  including the exhausted-dialogue path where no new chooser is installed.
- Opcode `0x0a` maps to `HandleSceneEntryStepPromptCondition` at `0x149b4`.
  In the dialogue entry callback it branches around the exhausted-dialogue
  media path while at least one choice record exists. The frame's persistent
  callback at `+0x12` then invokes opcode `0x17`; `StepFrameIdleCallbackCommandStream`
  at `0x143af` services that callback during interaction polling. Consequently,
  `VM0_1_P3.AVI` is reached only when every response-frame choice is already
  marked played.
- `BindSceneRuntimeCurrentFrame` at `0x145d6` saves the outgoing frame label at
  runtime offset `+0x177` before rebinding the current frame record. Opcode
  `0x0b`, handled by `HandleSceneEntryPromptPreviousSceneCondition` at
  `0x14a37`, compares that saved label case-insensitively with the label of its
  16-bit frame-index argument. Its first argument is the expected boolean and
  its third argument replaces the callback program counter when the comparison
  does not match that expectation. The `BA0.RUN` command at `0x8a3` uses this
  to distinguish the initial dialogue-hub arrival from lead-in frame 16
  (`KS0_1_X1`) from a return after one of the response frames.
- Opcode `0x10` maps to `HandleSceneEntryClearInteractionSelectionAndStepPrompt`
  at `0x14d41`. Its 16-bit argument is relative to the current frame's
  interaction list. The handler clears bit 0 in both the runtime UI proxy at
  `+0x2c` and its backing interaction descriptor at `+0x21`, disabling that
  hotspot for the current frame entry. `ExecuteSceneFrameAndInteractions` at
  `0x13277` recreates and enables eligible proxies before each enter callback,
  so the change does not persist when the frame is entered again. The handler's
  `Step` prompt is visible only when the original script-debug flag pair is set.
- Opcode `0x1a` maps to `HandleSceneEntryMediaAndSetBasenameFlag` at `0x159e1`.
  `ExecutePresentationEntry` at `0x1652a` routes IAVF media through
  `RunMediaPresentation` at `0x168af`. It first deactivates the UI selection
  presentation, removing the active cursor before playback; the next frame's
  interaction presentation makes the cursor visible again. The media wrapper
  preserves the current logical display page before packetized playback, then
  fades out, redraws the saved page through `DispatchDisplayDirtyRegionUpdate`
  at `0x4e4b0`, and fades its palette back in before returning. The
  reimplementation snapshots and restores the indexed framebuffer and palette
  around opcode `0x1a` media for the same separation. This is required both
  when a following opcode `0x1b` preview draws only the 300-pixel scene area and
  when the callback returns directly to a type-2 frame such as `BAZ2`, which has
  no media to redraw itself.
  The handler then removes the media extension and sets that basename in the
  shared played-entry catalog.
  Opcode `0x09`, handled by `HandleSceneEntryPromptScenePlayedCondition` at
  `0x1488f`, can therefore skip a completed lead-in such as `VM0_1_P2` when a
  dialogue response returns to the same frame, while the response-frame flags
  continue to control which opcode `0x16` choices remain available.
- `ExecuteSceneFrameAndInteractions` at `0x13277` runs frame callbacks,
  presentations, chooser selection, and interaction callbacks.
- `ExecuteSceneFrameAndInteractions` constructs the frame's hotspot controls
  before running its entry and persistent callbacks. An opcode `0x17` chooser
  is added to the same live control registry rather than replacing those
  hotspots. `PollInteractionAndResolveSelection` services both registries;
  when a scene hotspot wins, `CleanupCurrentSceneFrameInteractions` at
  `0x13832` dispatches phase 3 to the active chooser handler before running
  the hotspot callback. Dialogue choices therefore remain visible while the
  player can still use the current frame's navigation and action targets.
- `RIPPER.RUN` sets milestone flag 32, plays `PROINT.AVI` and `PROLOG1.AVI`,
  then transitions to `BA0.RUN`.
- The transition from startup presentations to the scene runtime changes to a
  newly initialized display page through `InitializeSceneDisplayModeAndContext`
  at `0x1e28a`. Its backing pages are zero-initialized, so pixels outside the
  first scene movie do not retain the final introduction frame.
- `BA0.RUN` is a bridge rather than the durable opening scene. Its `start`
  callback creates the concurrent `PROLOGUE.RUN` loop, loads and starts
  `R_P_L1.WAV`, and presents only the first frame of `BAW1A.SMK`; its sole
  chooser callback hands control to the concurrent prologue runtime, which
  eventually transitions to ACT1.
- Opcode `0x1f` loads a named WAV into the original's 20-slot trigger table
  through `HandleSceneEntryLoadResourceIntoFirstFreeSlot` at `0x15e48`.
  Opcode `0x20` configures that slot through
  `HandleSceneEntryConfigureOrStartNamedAudioTrigger` at `0x15eea`; a zero
  trigger starts it immediately, while control bit 0 makes
  `StartAudioTriggerSlot` at `0x37297` set the descriptor repeat field to -1.
  ScummVM maps that state to an infinite looping audio stream. Choice-list
  activation does not stop or replace the named trigger, so `R_P_L1.WAV`
  continues beneath dialogue selection until a later explicit stop or clear.
- Opcode `0x14` stores its argument as the next frame index and returns control
  code `-2`. `RunSceneScriptLoop` responds by servicing the concurrent runtime
  once before continuing BA0 at that stored frame.
- From BA0 `start`, the `straight` interaction selects frame 1 (`BAW1A`). The
  concurrent `PROLOGUE.RUN:loop` frame evaluates milestone and played-scene
  conditions, BA0 plays and marks `BAW1A`, its exit callback selects frame 3,
  and a second concurrent pass leads into the three-way `T1` chooser.
- PROLOGUE opcodes `0x08` and `0x09` compare milestone and played-scene state
  against an expected boolean and replace the callback program counter with
  their third argument when the condition fails. Opcode `0x0c` takes its
  default target only when no preceding condition failed; both `0x0c` and
  opcode `0x0d` then clear the original runtime's branch/prompt state.
- PROLOGUE opcode `0x18` action 300 arms the deferred briefing-media selector.
  `ServiceSceneFrameAudioAndBriefingTriggers` at `0x138c9` services that state;
  the ScummVM implementation records the armed selector, while presentation of
  the briefing control remains a later vertical slice.
- Scene interaction records contain an 11-byte label followed by an `x/y/width/
  height` rectangle at offsets `+0x0b/+0x0d/+0x0f/+0x11`. The scene UI uses
  transposed selection axes relative to the displayed frame, so ScummVM maps
  this to screen `x=y`, `y=x`, `width=height`, `height=width`. This mapping was
  confirmed live with the central `start` region and T1's left region.
- The interaction cursor selector is the byte at record offset `+0x15`; it is
  separate from the keyboard selector word at `+0x13` and the condition
  callback pointer at `+0x16`.
- The normal option-bit-zero control path selects on the primary mouse-button
  press transition. BA0's `start` interaction uses this path and returns
  control code `-2` from opcode `0x14`.
- `ExecuteSceneFrameAndInteractions` at `0x13277` builds the frame controls,
  runs the enter callback and any frame media, then calls
  `ServiceUiControlStateSelection` once before activating the blocking chooser
  loop. That first result is discarded, consuming mouse transitions accumulated
  during the media so they cannot immediately select the next interaction.
- T1's left and right callbacks test whether `VM0_1_P3` or `KS0_1_P1` has been
  played and use opcode `0x0c` to select the appropriate default travel frame.
  Zoom selects `BAZ1` directly. With no prior played-state flags, left follows
  `T1 -> BAT14A -> VM0_1_X1`, playing `BAT14A.SMK` and `BAT4AS.SMK` before
  exposing the next right/left/talk chooser.

## Media

- Direct `SMK2` assets use the game's Smacker path. The ScummVM engine should
  delegate these streams to `Video::SmackerDecoder`.
- Files such as `PROINT.AVI` and `PROLOG1.AVI` are `IAVF2.00` packetized media,
  not RIFF AVI. `RunPacketizedMediaPlaybackCore` at `0x5b592` demultiplexes
  descriptor records containing mono signed 16-bit PCM and segmented Smacker
  setup/frame payloads.
- The IAVF header is 145 bytes. Across all 475 retail assets, the 32-bit value
  at offset `0x10` exactly matches the number of opcode `0x67` playback gates,
  not the number of rendered video frames. The compact PCM fields occupy
  offsets `0x1c` through `0x27`. Offset `0x2f` stores presentation height and
  offset `0x31` stores presentation width; the values are `200, 320` for
  `PROINT.AVI` and `PROLOG1.AVI`.
- The reimplementation reconstructs each segmented Smacker stream and delegates
  frame decoding to `Video::SmackerDecoder`; PCM is delegated to the ScummVM
  mixer. `PROINT.AVI` contains 16 Smacker segments and `PROLOG1.AVI` contains
  two.
- `RenderCustomPacketFrameAndOverlays` at `0x6c486` installs an IAVF packet's
  active palette directly through display service `0x1d`. It does not call
  `ApplySharedDisplayPalettePatch`; that patch belongs to the separate
  `RunMediaSequence` Smacker path. Reconstructed IAVF segments must therefore
  retain their complete decoded palettes.
- Audio command `0x66` distinguishes its timeline byte count from the smaller
  stored payload byte count. `SubmitMediaAudioChunkDescriptors` at `0x490e6`
  fills the difference with silence and submits the timeline byte count to the
  managed-audio path.
- Command `0x67` is the playback gate. Its tag is paired with the same-tagged
  `0x66` audio descriptor, and the observed second argument repeats that
  descriptor's cumulative effective PCM byte offset. While
  `ServiceManagedAudioTriggerEntry840ControlLoop` at `0x48ad3` waits for the
  active managed-audio descriptor tag to advance past the gate, the
  reimplementation maps that tag to the equivalent mixer byte offset.
  Embedded Smacker header rates are therefore not the presentation clock; the
  reimplementation uses mixer elapsed time to reproduce this audio-master
  scheduling.
- Opcode `0x6c` loads the next custom packet through
  `LoadCustomPacketPaletteStateBlock` at `0x6c430`; it does not itself present
  the frame. Opcode `0x77` calls `RenderCustomPacketFrameAndOverlays` at
  `0x6c486`. Every retail `0x6c` packet is followed by one `0x77`, for 155486
  load/render pairs. The reimplementation commits a reconstructed Smacker frame
  only when the corresponding render command is encountered.
- Opcode `0x75` primes the original buffered packet stream and enables its
  managed-audio control state. ScummVM parses the seekable stream eagerly and
  owns the audio timeline through the mixer, so this command is an explicit
  no-op at the engine boundary. The executable also contains FLIC setup, packet,
  and decode branches at opcodes `0x69`, `0x6b`, and `0x76`, but none of the 475
  retail IAVF assets exercise them.
- After opcode `0x70` stops the original dispatch loop, 425 retail files carry
  a trailing `0x79` record and 50 end immediately. For those trailers, the
  second descriptor argument equals the remaining payload size and the third
  equals that size minus the first argument. Because the original player exits
  on `0x70`, the trailer is not a playback command and the reimplementation
  leaves it unread as well.
- `RunMediaPresentation` at `0x168af` installs
  `InitializeMediaPresentationDisplayModeCallback` at `0x163a8`. For
  presentation extents no larger than 320x200, that callback selects a 2:1
  display scale. `PreparePacketizedMediaPlaybackBranchSetup` at `0x5b237`
  then centers the effective scaled extents. The reimplementation applies the
  same scale to the embedded Smacker frames, so a 320x200 presentation uses
  the full 640-pixel window width while preserving the original aspect ratio.
- IAVF opcode `0x68` is a display boundary. `RunPacketizedMediaPlaybackCore`
  dispatches palette service `0x1d` and then display service `0x14`, whose
  target is `ClearGenericVideoLogicalPage` at `0x45ed8`. `PROINT.AVI` places
  this command before each of its 16 differently sized Smacker segments, so
  pixels from a larger segment must be cleared before the next segment's
  palette is applied.
- Opcode `0x1a` enables the presentation callback when its third argument is
  zero. In that mode only keyboard Escape stops the whole presentation and
  Space pauses or resumes video and audio; mouse buttons do not skip it.
- `ExecuteSceneFrameAndInteractions` at `0x13277` marks a type-0 frame label
  played before calling `ExecutePresentationEntry` at `0x1652a` with keyboard
  controls enabled. That path installs `PollPresentationEscOrSpaceCommand` at
  `0x49039`: Space pauses or resumes the response video and its IAVF audio,
  while Escape advances to the normal post-presentation callback with the
  response still marked played. Type-1 interactive loops retain their separate
  mouse and hotspot path and do not install these keyboard controls.
- For a controlled Smacker scene transition, advancing with Escape presents
  the terminal decoded frame before returning to the ordinary frame-exit
  callback. This preserves the completed transition image while following the
  same type-0 ordering established by `ExecuteSceneFrameAndInteractions` at
  `0x13277` and `ExecutePresentationEntry` at `0x1652a`.
- Opcode `0x1b` enters `HandleSceneEntryMediaPreviewOrPrompt` at `0x15b03`.
  That handler passes a target value of one and
  `MediaSequenceCounterEqualsTarget` at `0x15ac8` to `RunMediaSequence` at
  `0x1e516`. The callback is evaluated after each presented frame, so the
  command decodes and retains the first Smacker frame before returning to the
  scene chooser. The later BA0 frame presentation plays the full sequence.
- Engine-local media code should remain an adapter or demultiplexer wherever
  packet payloads can be handed to existing ScummVM codecs.
