# RIPPER.LE Architecture

## Startup

- `_entry` at `0x592f0` jumps to `RuntimeStartupAndExit` at `0x59368`.
- `RuntimeStartupAndExit` performs the DOS-extender runtime bootstrap and calls
  `RunRuntimeInitAndProgramMain` at `0x6b118`.
- `RunRuntimeInitAndProgramMain` transfers to the first game-owned coordinator,
  `RunGameStartupAndMainLoop` at `0x100c2`.
- `RipperEngine` owns the engine-lifetime managers through scoped pointers;
  nested presentation owners use the same model, while their engine, input,
  mixer, and resource links remain non-owning dependencies.
- `SceneAudioManager` is an engine-lifetime service alongside `MediaPlayer`.
  Script execution, save-state serialization, and Cyber suspension address the
  scene trigger table directly; media presentation only services that table at
  the retail frame boundaries.
- The game coordinator defaults to `ripper.run`, initializes resources, and
  plays `LOGO.AVI` with `PollPresentationEscOrSpaceCommand` at `0x49039`, so
  Escape skips the presentation and Space pauses it. The ScummVM game option
  `skip_intro` bypasses only this startup presentation; saved-game restoration
  and the front-end loop retain their normal ordering.
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
- The demo has a separate coordinator. `FUN_000100d7` at `0x100d7` defaults to
  `RIPPER.RUN`, calls `FUN_000104bf` at `0x104bf` to initialize the script-test
  runtime, and passes the selected script directly to `FUN_000113cf` at
  `0x113cf`. It does not enter the retail `LOGO.AVI` or front-end-menu route.
  `FUN_0001084a` at `0x1084a` installs the demo's 18 cursor rows before script
  execution. Its startup still loads the demo-specific toolbar, WAC, and scene
  selection resources while omitting retail-only inventory and front-end
  managers whose resources are absent from the demo.
- Demo scene action 9999 is also the intended end-of-gameplay signal. When
  `RunSceneScriptLoop` at `0x113cf` returns `-4`, the coordinator at `0x100d7`
  does not shut down immediately: it displays `SOON.PCX` from `INTERFAC.PL`,
  waits `0x48` DOS ticks or one keyboard command, displays `RIPBOX.PCX`, plays
  `RIPBOX.WAV` from `SOUND.PL` until completion or Escape, waits another
  `0x48` ticks or keyboard command, and then shuts down. `BA0B.RUN` deliberately
  returns 9999 after the Newsroom's `PROLOG2.AVI`; the two promotional screens
  and audio are therefore part of the demo ending rather than another scene
  transition.
- The demo opcode `0x17` handler at `0x13783` allocates the complete accumulated
  choice model and creates one list chooser control with ID `0x4e2`. Unlike
  retail `HandleSceneEntryChoiceListLifecycle` at `0x1523d`, it does not create
  auxiliary scroll-arrow controls, and the demo `INTERFAC.PL` has no
  `MNARROW0..3` members. ScummVM therefore gives the arrowless demo chooser a
  four-row viewport in the available lower display band while retaining the
  retail three-row viewport and arrow controls when those assets are present.
  The demo handler stores presentation template `0x68d08` at `0x16342` before
  creating the control. `InitializeSharedPresentationTemplates` at `0x10c9d`
  configures that template with normal glyph lookup byte/background index
  `253/0`, selected glyph lookup byte/background index `254/250`, and the
  `small.fnt` source pixels are translated by display command `0x19` through
  the active lookup template. Captured demo output resolves the selected glyph
  to MENUB's dark-gray ramp index `7`; ScummVM's direct indexed rasterizer uses
  that resolved index rather than treating lookup byte `254` as the final
  color. Retail directly uses `251/0` and `4/248`.
- The demo also has a shorter opcode `0x1a` layout. Its handler at `0x13e46`
  forwards path, auxiliary text, Y, and X to `FUN_000142bb` at `0x142bb` and
  forces scene-relative presentation controls on. Retail
  retail script command executor `FUN_0001357f` at `0x1357f` accepts the additional
  keyboard-control argument before the same placement pair. The demo checks its
  subtitle setting and auxiliary pointer but has no body on that branch, so the
  setting does not create gameplay text there. The script runtime selects the
  matching argument ABI from the detected game variant and reserves authored
  auxiliary-text presentation for retail.
- Packetized Smacker branches do not necessarily begin with a palette record;
  such a branch retains the display palette installed by its predecessor.
  ScummVM caches that inherited palette as the stable base for per-frame text
  and combat callback transformations until the branch publishes a replacement.
- Demo opcode `0x1d` likewise omits the retail concurrency selector.
  `FUN_000140dc` at `0x140dc` stores its target and entry-label pointers in the
  pending scene fields and returns `-3` unconditionally. Demo scene action 200
  follows the `0x13bd1` branch of the dispatcher at `0x13a52`: it enqueues the
  `R_P_L1.WAV` descriptor loaded by `FUN_000104bf` at `0x10794` and applies its
  normal `0x2fff` packed volume. The zero-initialized descriptor receives no
  repeat setup, so the 11,025 Hz mono PCM cue plays once for 23.986 seconds;
  action 201 stops the retained audio handle if it is still active.

## Detection

- The detector distinguishes the retail release and demo without requiring
  either original executable. It fingerprints the first 5,000 bytes and sizes
  of `INTERFAC.PL` and `SOUND.PL`, two runtime data archives present in both
  distributions; the demo entry carries `ADGF_DEMO`.

## Input

- Keyboard commands are polled through `PollKeyboardCommand` at `0x4d364`.
  Commands use ASCII values or BIOS-style extended scan codes.
- `MouseDriverEventCallback` at `0x562f0` accumulates movement and button-edge
  state. `ServiceMouseCallbackStateTimer` at `0x56240` publishes that state to
  the UI selection layer.
- Packetized presentations optionally use
  `PollPresentationEscOrSpaceCommand` at `0x49039`: Escape stops playback and
  Space toggles its control/pause state. The callback does not poll the mouse.
- ScummVM's optional Escape fast-forward is an engine extension. Since Smacker
  pixels and palette packets are stateful, it reconstructs the terminal frame
  sequentially from a cleared decoder surface, applies that frame's decoder
  palette, and submits one final redraw. Retail Escape stops playback instead.

## Resources

- `LoadAssetLibraryDirectoryFromOpenHandle` at `0x6ad80` reads a six-byte
  count/directory-offset prefix. A following `2BIL` tag selects the modern
  10-byte header and 20-byte records; otherwise the payload begins at byte six
  and the directory uses 12-byte legacy records.
- `OpenIndexedContainerEntryReadOnly` at `0x55a4c` performs case-insensitive
  member lookup and seeks the shared library handle to the selected member.
  `AssetLibrary` retains directory order for member ranges and prefix
  enumeration, while a normalized-name index provides exact lookup and rejects
  duplicate names during parsing.
- Retail interface and nested options assets pass through the same
  indexed-surface extraction and bitmap-sequence decode paths after their
  owning `AssetLibrary` has selected the member. The two entry points retain
  distinct source labels in diagnostics and do not merge container search
  order. The demo's legacy `INTERFAC.PL` instead contains a PCX member named
  `OPTIONS` and no nested `OPTIONS.PL` library, so its direct script startup
  does not initialize the retail options source.
  Standalone puzzle PCX streams use that same indexed PCX codec only after
  their puzzle-specific path or nested-library lookup; each puzzle retains its
  own required dimensions and palette-size checks.
- The retail `SCRIPT.PL` contains compiled `.RUN` scene scripts. The demo leaves
  its `RIPPER.INI` `[FILES] script` value empty and supplies `RIPPER.RUN`, scene
  scripts, `MILESTON.DEF`, and `GAMETEXT.TF` as loose files, so the resource
  manager uses the game-directory search archive as its script source instead
  of opening an empty archive path. The default entry is `RIPPER.RUN`, which
  leads to the initial `BA0.RUN` scene. Binary header, frame/interaction table,
  argument-layout, and callback decoding live in `script/compiled_script.cpp`;
  `script.cpp` owns runtime state and command execution. This keeps parser
  validation separate from scene-loop mutation.
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

- Retail `LoadFrontEndToolbarResources` at `0x18678` loads nine nested `TOOLBAR1.PL`
  through `TOOLBAR9.PL` bitmap sequences. Each sequence has ten frames; the
  first frame dimensions are the hit rectangles. `GAMETEXT.TF` is the startup
  resource-string table, and entries 1 through 9 provide the toolbar labels.
  The resource manager caches the decoded table and NF2T fonts shared by the
  front-end presentations; callers still receive independent value objects.
- Demo `LoadFrontEndToolbarResources` at `0x16160` instead loads eight nested
  `TOOLBAR1.PL` through `TOOLBAR8.PL` sequences from `INTERFAC.PL`; the demo
  archive has no ninth toolbar member. `RunFrontEndActionMenu` at `0x165b9`
  creates exactly eight controls with IDs `0x514` through `0x51b`.
- `RIPPER.INI` supplies the `scene`, `puzzle`, `combat`, and `cyber` resource
  directories. Scene media and named scene-audio slots first preserve
  ScummVM's basename lookup, then use those configured directories as explicit
  fallback paths. This keeps nested Cyber media and suspended audio restoration
  tied to the original resource-path configuration when a flattened search
  entry is unavailable.
- The `7PT_FONT.FNT` member is an NF2T glyph descriptor used by
  `RenderFrontEndActionPreviewSprite` at `0x189b3`. The reimplementation
  decodes the descriptor and its custom bitmap instead of substituting a host
  font. NF2T measurement, mask painting, and optional clipping are shared by
  the engine's `BitmapFontRenderer`; presentation owners retain the choice of
  font, solid output color, and layout. Toolbar bitmap color maps target
  palette index 0, indices 4 through 9,
  and indices 246 through 255. The demo/retail
  `ApplySharedDisplayPalettePatch` functions at `0x1b1bd`/`0x205d0` restore
  those bands on every media palette update so interface pixels remain stable
  while the underlying Smacker palette changes. Both demo
  `InitializeSharedPresentationTemplates` at `0x10c9d` and retail
  `InitializeSharedPresentationTemplates` at `0x1196f` capture those bands from
  the loaded `MENUB` presentation palette through
  `CaptureSharedDisplayPalettePatch` at `0x1b176`/`0x205a9`. ScummVM likewise
  owns the shared patch with the modal presentation initialized for both
  variants, rather than the retail-only toolbar manager, and applies only those
  reserved bands to later scene and puzzle palettes.
- `IndexedBitmapRenderer` owns transparent indexed-bitmap blits and the
  row-major 3x3 frame walk translated from `ResolveChooserFrameTileIndex` at
  `0x55250` and `TileChooserControlFrame` at `0x54fbe`. MENUB and inventory
  frames share that exact walk; the WAC database retains its distinct tiling
  order while using the same transparent-pixel blitter.
- Indexed presentation suspension is represented by `IndexedDisplaySnapshot`,
  which captures an indexed screen rectangle together with all 256 palette
  entries. Cyber scene suspension uses a full 640x400 snapshot and restores
  its pixels and palette before resuming the scene runtime. Controlled IAVF
  presentations use the same snapshot service around `RunMediaPresentation`
  rather than maintaining a second framebuffer and palette representation.

## Scene Toolbar

- `RunFrontEndActionMenu` at `0x18b3a` is entered by
  `PollInteractionAndResolveSelection` at `0x13c8d` whenever the published
  vertical selection coordinate is below `0x32` (50). It creates nine controls
  with action IDs `0x514` through `0x51c`, lays them out right-to-left from
  x=`0x276` (630), and subtracts each bitmap width plus five pixels.
- The demo follows the same top-band gate through
  `PollInteractionAndResolveSelection` at `0x1218a`, but calls its eight-action
  `RunFrontEndActionMenu` at `0x165b9`. `DispatchFrontEndAction` at `0x16a2f`
  maps those controls to scene selection, an original no-op, WAC, save,
  restore, the demo OPTIONS settings panel, a second original no-op, and quit.
  This mapping is not a truncated or shifted copy of the retail nine-action
  table.
- Demo action `0x519` enters `RunTake2IniSliderSetupMenu` at `0x17fc0`. This is
  a different full-screen settings presentation from both retail options
  menus: it loads `OPTIONS.PCX` plus numerically ordered `OPTION1.BBM` through
  `OPTION11.BBM` from `INTERFAC.PL`, then creates six check boxes, three
  sliders, eight toolbar-hotkey controls, and Save/Cancel/Defaults/Help
  controls. The routine clones the 56-byte persistent settings record before
  polling input; Save copies the staged record back, Cancel discards it,
  Defaults calls `InitializeDefaultSettingsBlob` at `0x18c55`, and Help opens
  game-text resource `0xfb`.
- The demo record stores check boxes at offsets 4..9, slider percentages at
  offsets 14..16, and eight hotkeys at offsets 24..38. Their default values are
  `[1,0,1,0,1,1]`, `[100,50,100]`, and
  `[0x1100,0x1700,0x0077,0x1f00,0x1300,0x1800,0x3b00,0x1000]` respectively.
  `OPTIONS.PCX` labels the first four toggles Subtitles, Video Buffering,
  Toolbar Help, and Toolbar Permanent, and labels the sliders Sound, Music,
  and Mouse. `RunFrontEndActionMenu` reads offset 6 to gate delayed toolbar
  tooltips. No consumer has been confirmed for Toolbar Permanent or the two
  unlabeled toggle rows, so ScummVM preserves those staged values without
  assigning them new runtime behavior.
- The toolbar remains modal while the pointer is in that top 50-pixel band;
  scene hotspots do not receive those events. Scene Smacker coordinates are
  relative to the original 640x400 display page, whose top edge follows that
  band at y=50. `RefreshDisplayDimensionsAndInputBounds` at `0x5d8e0` derives
  the maximum input coordinate from the active descriptor height, so y>=400 is
  outside the scene hit-test region.
- While a control is hovered, `RunFrontEndActionMenu` advances that action's
  frame index once per extended DOS tick. Each invocation first constructs a
  new `UiControlState` from the toolbar resource's base pointer and draws that
  base bitmap, so every toolbar reveal initially presents frame zero rather
  than the frame visible when the preceding session ended. ScummVM resets its
  per-action animation indices at the same entry boundary. Each icon is
  vertically centered in the 50-pixel band. The preview gate uses
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
- `DispatchFrontEndAction` at `0x190b7` routes toolbar action `0x51c` through
  `RunBinaryPromptChooser` at `0x1803c` unless the Cyber transition flag is
  active. The ordinary path resolves `GAMETEXT.TF` resources `0x3f`
  (`Quit Game?`), `0x3c` (`Yes`), and `0x3d` (`No`), centers a two-row MENUB
  chooser, and initially selects `No`; Escape or `No` resumes the scene, while
  `Yes` returns the retail `-4` outer-runtime exit. ScummVM maps that confirmed
  exit to its engine-quit request, returning control to the host launcher.
- Engine-local rectangular controls use `UiControlRegistry`, which preserves
  the insertion-order first-hit behavior of `FindUiControlStateAtPoint` at
  `0x4aae8`. `ChooserModel` owns the selected and first-visible indices for
  inventory and dialogue lists; each presentation still owns its resource
  rows, bounds, input dispatch, and rendering.
- `PollInteractionAndResolveSelection` at `0x13c8d` checks the published
  vertical coordinate before scene interaction handling, so the toolbar band
  remains available during BA0 first-frame previews, normal scenes, and an
  active opcode `0x17` dialogue chooser. `RunFrontEndActionMenu` snapshots and
  restores both the chooser registry and scene UI-control list while it owns
  input. That band uses cursor `0xe` (14), matching the white cursor selected
  by `RunStartupFrontEndLoop` at `0x10778`; scene and chooser cursor selection
  resumes below y=50.
- For interactive type-1 scene media, `ExecuteSceneFrameAndInteractions` at
  `0x13277` passes `PollInteractionAndResolveSelection` to `RunMediaSequence`
  at `0x1e516` as its per-frame callback. Entering the top band synchronously
  runs `RunFrontEndActionMenu`; the callback does not return while the toolbar
  is displayed, so the Smacker sequence cannot decode another frame. The next
  media iteration snapshots the current timer tick rather than catching up on
  elapsed toolbar time. ScummVM pauses the decoder clock across the same input
  ownership interval and continues presenting toolbar animation and tooltips.
  When the sequence loop flag is set with a zero loop-start frame,
  `RunMediaSequence` retains the loaded Smacker state and resets its one-based
  frame counters to one without calling `SeekSmackerPlaybackFrame`. ScummVM
  likewise rewinds the same decoder without clearing its surface, preserving
  the wrapped framebuffer required by skipped blocks in the first packet.
- Retail `RunMediaSequence` advances frames against its timer callback slot;
  nested presentation work explicitly suspends that slot and resumes it on
  return. ScummVM global dialogs instead enter `Engine::pauseEngine()` while the
  RIPPER media loop remains blocked on the stack, so the engine pause hook
  recursively pauses every active Smacker clock along with the mixer. Space
  likewise applies one balanced decoder and mixer pause level, covering named
  scene-audio slots as well as packetized audio. Pause/resume traces record the
  media frame, decoder clock, external-audio clock, and scene-audio state.
- `ExecuteSceneFrameAndInteractions` enters that input loop when the frame has
  either registered interactions or an idle callback. Opcode `0x17` dialogue
  choosers created by an idle callback therefore hold their type-1 presentation
  until a response is selected. `PollInteractionAndResolveSelection` calls
  `StepFrameIdleCallbackCommandStream` at `0x143af` before reading chooser
  input. For a controlled type-0 presentation this occurs after the media has
  completed but before its hotspots become interactive. `EE2.RUN` frame 12
  (`EEZ1`) uses that path to test the played state of `q2_v5` and play
  `Q2_V5.WAV` once after `EEZ1.SMK`; the normal blocking-audio presentation
  supplies cursor 19 while the clip is active. `HandleSceneChooserSpecialCommand`
  at `0x17c5a` resolves Escape before hotspot lookup and returns `-4` for the
  normal nested Cyber-runtime exit.
- Retail chooser input rerenders its row surface only when selection or window
  state changes: `ProcessChooserControlInput` at `0x57372` calls
  `RenderChooserControlRows` at `0x582de` from its dirty path. ScummVM caches
  those rendered rows and composites the cache over each scene-movie frame,
  avoiding font and row reconstruction on every pointer poll or video tick.
  `CleanupCurrentSceneFrameInteractions` at `0x13832` also dispatches phase 3
  to `HandleSceneEntryChoiceListLifecycle` at `0x1523d` before every chooser
  exit; the shared chooser therefore restores and releases its backing before
  a nested Cyber runtime unwinds, including Escape and toolbar exits.
- If an action in that toolbar returns `-3`, `RunFrontEndActionMenu` at
  `0x18b3a` returns the same control code to `PollInteractionAndResolveSelection`
  at `0x13c8d`. `RunMediaSequence` stops its frame loop on the nonzero callback
  result, and `ExecuteSceneFrameAndInteractions` propagates the transition
  instead of resuming the interactive Smacker. ScummVM uses the queued script
  target as the equivalent unwind signal, closes the current decoder, and
  applies the scene handoff before another loop pass can begin.
- Action dispatch at `0x190b7` maps the nine controls to remote setup, world
  map, inventory, WAC, save, restore, options, help, and quit. The ScummVM
  toolbar preserves the click/release routing. Action `0x515` and scene-entry
  action 2 both enter the shared `WorldMap` subsystem; action `0x517` enters
  WAC, and actions `0x518`/`0x519` enter the shared save/restore chooser after
  removing the toolbar presentation. Action `0x51b` enters
  `RunModalSelectionTableDialogWithRestore`: it presents general help resource
  400 normally and conversation help resource `0x19b` while scene-runtime bit
  `0x20` marks a prompt/chooser active. The implemented choice-list lifecycle
  maps that state to the pending `DialogueChooser`. Action `0x516` and
  scene-entry action 3 both enter the engine-owned inventory service. Quit
  remains an explicit stub.
- `RunTake2IniSliderSetupMenu` at `0x1989b` edits eight live settings in this
  order: master, ambient, SFX, video, brightness, color, contrast, and tint.
  The descriptor table at `0x1856a` supplies each range, step, and default;
  contrast is the sole reversed slider, with range 80..120 and step -2.
  `SaveTake2IniVideoAndVolumeSettings` at `0x5c5e7` persists the values on
  exit. ScummVM retains the original percentages while mapping ambient, SFX,
  and video to its music, SFX, and speech mixer categories; master scales all
  three categories.
- `BuildVideoAdjustedVgaPaletteBytes` at `0x569c0` applies tint, brightness,
  contrast, and color in six-bit VGA space before clamping each component.
  ScummVM performs the same integer transform whenever a new indexed media
  palette is installed and expands the resulting six-bit components to the
  backend's eight-bit palette representation.
- The Remote Control loads `REMOTE.SMK` into a 216-by-148 modal region at
  physical (212,126), then overlays `REMOTE0.BBM` through `REMOTE5.BBM` and
  the `5pt_font.fnt` label. The six controls return Escape, defaults, previous,
  next, increase, and decrease. Left/right wrap through the eight sliders;
  Up/`+` and Down/`-` follow the descriptor's signed step. F1 opens help
  resource `0x19d`. The selected setting is represented by ten ticks at
  physical (278,171), using palette index 254 for filled ticks and 252 for
  empty ticks. Changes take effect immediately and the settings are saved when
  the modal exits, matching `RunTake2IniSliderSetupMenu` at `0x1989b`.
  The setup routine invokes `RunMediaSequence` at `0x1e516` in captured-backing
  mode. For every frame, the sequence stores decoded pixel zero as the color
  key at `0x1e996`, restores the captured 216-by-148 scene region, and issues
  the flagged blit at `0x1e9d3`; `GenericVideoScaledBlitCopyCallback` at
  `0x467f8` leaves destination pixels unchanged when they match that key. This
  keeps the active scene visible around the Remote Control. Sequence cleanup
  at `0x1eaf9` releases the temporary logical page without redrawing the
  backing, leaving the final keyed frame visible throughout the control loop.
  The modal's outer dirty-region update at `0x1a108` restores the pre-modal
  capture only when the Remote Control closes.
  `REMOTE.SMK` is an overlay sequence whose pixels use only shared palette
  indices 4 through 9 and 246 through 255; its unused chroma-key palette
  entries do not replace the suspended scene palette during playback. The
  captured scene palette remains active until a video slider changes, matching
  the shared-band patch in `RunMediaSequence` at `0x1e8e9` and
  `ApplySharedDisplayPalettePatch` at `0x205d0`. The increase and decrease
  controls share the same origin: the
  increase control clips the upper 19-by-27 pixels of the full 19-by-54
  decrease rocker. `FindUiControlStateAtPoint` at `0x4aae8` returns the first
  inserted overlapping control, so the clipped increase control owns that
  upper region and the decrease control owns the lower region.
- `RunOptionsMenu` at `0x1c001` opens the nested `OPTIONS` library embedded in
  `INTERFAC.PL`, replaces the display with `BG.PCX`, and continuously advances
  the 60-by-280 `OPT.SMK` animation at physical (544,44). It clones the
  60-byte persistent settings blob before entering its input loop, so toggle,
  video-mode, combat-difficulty, puzzle-difficulty, and action-key edits remain
  staged until Escape or exit control 1018 commits them. F1 presents help
  resource `0x191` without leaving the panel.
- The options state strips use `ONOFF0..8`, `SLIDE0..24`, `DIFF0..17`, and
  `PDIFF0..25`. `RenderOptionsStateStripSelections` at `0x1d935` selects
  anchor frames 0/8 for the buffered-video toggle, 0/8/16/24 for the video
  mode, 0/6/12 for combat levels 1/2/3, and 0/8/16 for puzzle levels 1/3/2.
  Control 1006 calls `AnimateThreeStateOptionsStripSweep` at `0x1d468` to
  cycle combat level 1 to 2 to 3 to 1. Controls 1007, 1008, and 1009 select
  puzzle levels 1, 3, and 2 respectively. Leaving the panel copies these
  staged bytes into the persistent settings blob before `SavePersistentSettings`
  at `0x1dd82` writes the puzzle and combat values; ScummVM stores the same
  values as `ripper_puzzle_level` and `ripper_combat_level`.
  Controls 1010 through 1017 invoke `RunOptionsKeyCaptureLoop` at `0x1d753`
  for action slots 0 through 6 and 8; slot 7 retains F1 help. Escape cancels an
  active key capture but exits and commits when the panel itself owns input.
- `DispatchSceneEntryAction` at `0x36892` routes action 2 to the same
  `HandleSceneSelectionAction` at `0x191e2` used by toolbar action `0x515`.
  `RunSceneSelectionMenu` at `0x20808` groups the 25 travel-entry flags 20
  through 44 through the table at `0x20702`, exposing a location when any of
  its entries is unlocked. The resulting 15 locations use game-text resources
  100 through 114 for their names and resources 150 through 164 for their
  destination script patterns. `ResolveHighestSetSelectionFlag` at `0x20394`
  supplies the highest enabled chapter flag from 1 through 4 to those patterns.
- The location list is chooser control `0x604`. Selecting it returns one of the
  15 group indices, which alone selects destination resource 150 through 164.
  The unlocked entry labels from resources 116 through 140 are concatenated
  into wrapped-text control `0x605`; `RunSceneSelectionMenu` immediately clears
  that control's activation bit, never handles events from it, and never maps
  an entry index to a destination. The right-hand panel is therefore
  informational: it has no mouse focus, selection highlight, or direct-travel
  behavior in the original executable.
- Both travel controls use the shared chooser template at `0x8a392`.
  `RenderChooserControlRows` at `0x582de` and `RenderChooserTextRow` at
  `0x58651` restore each row from the underlying `worldmap.pcx` presentation
  before drawing glyphs, so neither control paints an opaque row background.
  The normal template at `0x84030` renders palette index 4 (black); chooser
  `0x604` switches its selected row to the alternate template at `0x84032`,
  whose glyph color is palette index 254 (red). Deactivated detail control
  `0x605` always uses the normal black glyph template.
- Scene action 32 temporarily deactivates the selection presentation, dispatches
  display command `0x14`, and then reactivates selection. Entry `0x14` in
  `g_displayServiceCommandTable` targets `ClearGenericVideoLogicalPage` at
  `0x45ed8`, so this action clears the active scene display to black while
  preserving script and chooser state. `FA3.RUN` and `KC.RUN` use it
  immediately before playing replacement scene media; `DE1.RUN` reaches the
  same action from its `T1` navigation callback.
  The menu loads `worldmap.pcx` as its 640x400 background and advances
  `worldcel.smk` beside the location chooser.
- The `CreateListChooserControl` call at `0x20c2d` supplies an internal origin
  of (305, 45) to the transposed presentation layer. In screen coordinates the
  location viewport is x=45, y=305, width=190, height=80. The detail control
  created at `0x212eb` uses screen x=330, y=305 with the same dimensions and
  displays unlocked entry labels from game-text resources 116 through 140.
  Scene-selection layout rows 15 and 16 at `0x207ac` place the 23x15 up/down
  controls at screen (267, 306) and (267, 329).
- The list viewport holds five `small.fnt` chooser rows. `ProcessChooserControlInput`
  at `0x57372` keeps an absolute selection plus a first-visible row: Up and Down
  move one row and scroll at the viewport boundary, Page Up and Page Down move
  by one visible window while retaining the relative row when possible, and
  Home and End select the first or last row in the current window. The mouse
  updates the highlighted visible row and the two auxiliary controls step the
  same selection window. A successful selection returns control code `-3` with
  the resolved scene script; cancellation leaves the current scene active.
- `HandleSceneSelectionAction` at `0x191e2` stores a successful destination on
  the active runtime and returns `-3`. `RunSceneScriptLoop` at `0x124e9`
  destroys only that active runtime before loading the destination; its
  concurrent runtime remains live. This is required for `BA0B.RUN`: after
  `PROLOG2.AVI` sets milestone 300 and selects its sole frame, the preserved
  `PROLOGUE.RUN:loop` observes that milestone and opcode `0x1d` queues the next
  script. When opcode `0x1d` executes from the concurrent runtime itself,
  `HandleSceneEntryAndStartConcurrentSceneRuntime` at `0x15cd3` returns `-3`
  with the target and entry label, and the outer loop retires that concurrent
  runtime before continuing the handoff.
  The active-frame loop is not bounded by the number of frame records. Opcode
  `0x14` selecting the current frame still returns `-2`, which sends control
  back through the concurrent-runtime service before the active frame runs
  again. `BA0B.RUN` depends on this ordering: its sole frame selects frame 0
  after setting milestone 300, then `PROLOGUE.RUN:loop` observes the milestone
  and stages the next script instead of replaying `PROLOG2.AVI`.
- Scene transition targets retain the source-script extension in compiled
  opcode arguments. `RunSceneScriptLoop` at `0x124e9` replaces the target from
  its first dot onward with `.run` before opening the runtime, rather than
  appending another extension. A target such as `DK1.SCR` therefore resolves
  to `DK1.run`, not `DK1.SCR.run`.
- The demo performs the same normalization in `RunSceneScriptLoop` at
  `0x113cf`. `DispatchFrontEndAction` at `0x16a2f` formats the selected world-map
  destination into a bare name such as `BA0` and stores it on the active
  runtime; the outer loop replaces that name's terminator with `.run` before
  calling `CreateSceneRuntime`.

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
- A successful restore returns `-5` from `RunSaveRestoreSlotMenu` through
  `DispatchFrontEndAction`. `PollInteractionAndResolveSelection` at `0x13c8d`
  then cleans up the current frame interactions, and
  `ExecuteSceneFrameAndInteractions` at `0x13277` returns before running another
  frame callback. The reimplementation likewise stops active media and unwinds
  cached frame references before servicing the runtime installed by the load;
  its saved interactive presentation resumes on the following scene tick.
- `WriteEmergencySaveGame` at `0x1ae3c` writes the current scene identity,
  scene-resume state, runtime flags, played media state, audio triggers,
  briefing state, puzzle state, description, and preview. `LoadSavedRunStateBlob`
  at `0x1b492` reconstructs that data, and `RestoreSavedRunState` at `0x1b8dd`
  reinstates flags, concurrent scene context, media state, and the active scene.
  The ScummVM format is versioned and engine-local; it stores the confirmed
  script state, all 20 named scene-audio slots and their trigger/ramp state, and
  the indexed 640x400 framebuffer and palette so static scene presentations
  resume at the same visible boundary without serializing DOS pointers from the
  original fixed block. Format version 3 added the complete audio table and
  version 4 added the scene toolbar mask. Version 5 marks saves produced after
  startup Ripper-identity selection was implemented; older ScummVM saves with
  none of flags 6 through 9 set are assigned one identity when loaded. Existing
  identities are preserved, and version 2 load compatibility remains.
- `WriteConfiguredSaveCheckpointAndCleanupRuntime` at `0x1b274` writes the
  emergency save when the scene-script loop returns to the front end, then
  clears active audio triggers and the concurrent scene runtime. The
  reimplementation writes its reserved Continue slot when an active scene loop
  exits normally; Continue never substitutes a manual slot, while Restore Game
  always opens the manual chooser.

## Milestones and Progress Gates

- `g_namedFlagBitset` at `0x8a446` is the shared indexed progress store.
  `IsIndexedBitFlagSet` tests `index / 8` and `index % 8`, while
  `UpdateIndexedBitFlag` sets or clears the same bit. Script opcode `0x08`
  reaches `HandleSceneEntryPromptMilestoneCondition` at `0x1470a`, opcode
  `0x0e` reaches `HandleSceneEntrySetMilestoneFlag` at `0x14c4d`, and opcode
  `0x0f` reaches `HandleSceneEntryClearMilestoneFlag` at `0x14cc2`. Travel,
  WAC inventory, puzzles, dialogue, briefing media, and the startup milestone
  selection menu all read or update this same store.
- `MILESTON.DEF` supplies 222 distinct keyed labels for the indexed flags.
  `LoadStartupConfigAndInitializeResources` at `0x10d83` loads the table and
  `LoadStartupKeyedTextTable` at `0x1f169` enumerates each numeric key and its
  text. The ScummVM milestone service preserves that data-driven enumeration;
  level-three `milestones` debugging reports every defined key with its label
  and current bit value after definition loading and save synchronization. The
  confirmed ranges include chapter completion at 1 through 4, travel
  availability at 20 through 44, inventory unlocks at 50 through 69 with
  paired consumed bits at 100 through 119, WAC database scans beginning at
  70, story state beginning at 300, and cyberspace state beginning at 400.
  ScummVM's engine-local debug console reads this same service. Its
  case-insensitive `MILESTONES` command lists every `MILESTON.DEF` entry and
  current value, `MILESTONES ACTIVE` lists every set bit in the 1,000-flag
  store, `MILESTONES <ID>` reports one store entry, and `MILESTONES TOGGLE
  <ID>` flips and reports one store entry. The ScummVM-only
  `OVERLAY_MILESTONES` command toggles lower-left milestone-change
  notifications; `ON` and `OFF` select the state explicitly. Actual bit
  transitions enter an isolated FIFO, and each red-on-black message has a
  five-second dithered fade lifecycle before the next queued transition. The
  engine-wide `presentScreen` boundary composites the notification with shared
  interface red index 254 immediately before each backend update, then restores
  its scoped backing without presenting it. This keeps the logical indexed
  framebuffer clean across scene, media, and modal buffer changes. Presentation
  timing is suspended through `RipperEngine::pauseEngineIntern`, so closing the
  debugger with Escape does not consume a queued notification's lifetime.
  Milestone storage and retail validation remain independent of this ScummVM-only
  debug presentation.
  Flags 6 through 9 identify Magnotta, Falconetti, Burton, or Powell as the
  Ripper. `SeedRandomFreePersistentFlag6To9` at `0x106c0` counts zero-valued
  candidate bytes at persistent-settings offsets `+0x38` through `+0x3b`,
  seeds the retail random generator from the extended DOS tick count, selects
  a free candidate by remainder, and sets the corresponding indexed flag.
  Default settings zero all four candidate bytes. The routine runs once from
  `LoadStartupConfigAndInitializeResources` at call site `0x111fe`, before the
  front end, and again at `0x10679` after the new-game state reset. A save
  restore subsequently overwrites the provisional bitset, so every ordinary
  new game has exactly one identity from its beginning while a restored game
  keeps its prior selection. ScummVM uses its event-recorder-aware random
  source to choose among the same four default candidates.
  Opcode
  `0x1e` named flags are different: `HandleSceneEntrySetOrClearNamedFlag` at
  `0x15dfe` updates the string-keyed startup asset catalog rather than this
  indexed bitset.
- Flag 31 is `Magnotta's Apt Int OPEN`. `RunSceneSelectionMenu` uses it as one
  of the travel-entry gates, and `FA3.RUN` also tests it while controlling the
  apartment/keypad path. The `ACT2.RUN` and `ACT3.RUN` transition callbacks
  explicitly clear it as part of their chapter-wide location reset. It is not
  a WAC-message flag.
- `DispatchSceneEntryAction` at `0x36892` handles action 3 by setting the
  caller-supplied inventory unlock flag, clearing the paired consumed flag at
  `unlock + 50`, and calling `RunUnlockGatedSelectionMenu` at `0x360ae` with
  that unlock flag as the initial selection. The toolbar inventory action
  `0x516` enters the same chooser without granting or preselecting an item.
- `RunUnlockGatedSelectionMenu` scans unlock flags 50 through 69 and includes
  only entries whose flags 100 through 119 remain clear. Visible rows retain
  their original item IDs rather than being renumbered around unavailable
  entries. Labels come from game-text resources 200 through 219, F1 uses help
  resource `0x1bb`, and the chooser restores cursor selection zero on exit.
  `LoadStartupConfigAndInitializeResources` loads `INVTRY*.BBM`, and the menu
  attaches `INVTRY[item ID]` to each visible row; the shipped set provides nine
  50-by-40 item bitmaps. `RunUnlockGatedSelectionMenu` builds the centered
  secondary chooser from the 16 `MNU` frame assets and the small bitmap font,
  gives it the game-text resource 3 title (`Inventory`), and appends separate
  20-pixel Use and Done text-panel controls from resources `0x47` and `0x48`.
  `ProcessChooserControlInput` at `0x57372` keeps its hit-tested row separate
  from the committed chooser selection: pointer hover changes only focus and
  cursor feedback, while transition bit 2, identified as the primary-button
  press by `UpdateUiSelectionInputButtonFlags` at `0x4a230`, copies that row
  into the selected index. Moving across another item therefore leaves the
  clicked item selected. Only Use dispatches `ExecuteUnlockSelectionChoice`,
  while Done and Escape close the modal.
- `ExecuteUnlockSelectionChoice` at `0x364be` compares the active compiled
  frame label (the dword at frame-record offset `+5`) case-insensitively with
  game-text resource `270 + item ID`. A mismatch uses modal resource `0x4d`
  and leaves the chooser active. A successful use sets consumed flag
  `100 + item ID`; item IDs 1 and 7 clear that bit again after their dedicated
  presentation and therefore remain reusable. Items 5 and 8 have additional
  mutual-state gates through flags 58, 108, and 105. The ScummVM `Inventory`
  service owns this milestone-backed enumeration, acquisition, use gating,
  and the currently implemented generic and direct-media item branches; its
  state is already covered by milestone save synchronization.
- Inventory item 1 enters `RunQcsMag2UnlockMediaScreen` at `0x3bb08`. It
  preserves the active indexed page and palette, clears the display, retains
  `Q_CS_1.AVI`, clears again, retains `MAG2.AVI`, and plays the blocking
  `Q_P_40.WAV` cue. The final image remains until Escape or the central
  440-by-300 control is selected, after which the saved scene is restored.
  `ExecuteUnlockSelectionChoice` clears consumed flag 101 after this helper
  returns, leaving the item reusable.
- `ACT1_CHK.RUN` tests flag 311 (`Eddie finishes conversation in Act I on
  earth`) before its Act II handoff. When the flag is clear, opcode `0x08` at
  script offset `0x2e1` branches to the clear-branch entry at `0x301`; the
  callback then selects frame 0 at `0x305`. That false branch is normal scene
  routing, not an error condition.
- `BA0.RUN` sets flag 71 (`scan mug at murder scene`) immediately after
  `MUGSCAN2.AVI` completes. WAC maps flag 71 to database entry 1, whose handler
  opens `RunWacMugSelectionScene`. Solving that puzzle sets both 71 and flag 72
  (`completed mug`); flag 72 redirects later selections to the completed-mug
  presentation.
- `PROLOGUE.RUN` tests flag 301 (`played first wac message`) together with the
  played-state of `MUGSCAN2` and `BAZ1` and milestone 302 (`spoken to
  Stasiak`). When all gates pass, scene action 300 arms briefing selector 1 and
  the script immediately sets flag 301. `ArmBriefingMediaTrigger` at `0x1929a`
  reuses the ten frames of toolbar action four to create control `0x4e1` at the
  right edge of the lower presentation band, assigns cursor 16, and plays
  `WACICON0.WAV`. `ServiceBriefingMediaTrigger` at `0x1945b` advances the icon
  every three DOS ticks and plays `WACICON1.WAV` when the animation wraps. A
  later selection of control `0x4e1` runs `ClearBriefingMediaTrigger` at
  `0x193ab` to remove the control and stop its alert, dispatches selector 1 to
  `CP0_1_P1.AVI`, and sets travel flag 44. `ExecutePresentationEntry` at
  `0x1754b` deactivates the shared selection presentation for that AVI, so the
  cursor, toolbar, briefing animation, and its alert remain inactive until
  `RunMediaPresentation` at `0x17917` returns and selection presentation is
  restored. The script therefore records 301 at trigger-arm time, before the
  later UI event presents the media.
- The demo `BA0.RUN` also dispatches scene action 300 with selector 1. Its
  `ArmBriefingMediaTrigger` at `0x16b39` uses toolbar frame-set pointer
  `0x698fc`, the third entry populated by `LoadFrontEndToolbarResources` at
  `0x16160`, rather than retail toolbar action four. It creates the same control
  `0x4e1` at `x = 635 - width` and `y = scene origin + 305`, plays
  `WACICON0.WAV`, and prepares `WACICON1.WAV`. The demo service helper at
  `0x16c4c` presents `CP0_1_P1.AVI` and sets travel flag 44 after selection.
- Briefing selector 2 is an intentional no-op in
  `ServiceBriefingMediaTrigger` at `0x1945b`: selecting its armed control
  clears the trigger but presents no media and changes no milestone state.
  Selector 3 is armed directly by `EF2.RUN`; jump-table entry `0x19624`
  presents `RIP_WAC2.AVI` and likewise changes no milestone state. Selector 5
  is armed by an Act II `WMAP2.RUN` checkpoint branch; the retail dispatch
  presents `VF2_2_P1.AVI`, sets flag 307 (`got farley wac message`), and opens
  Cafe Duchamp by setting travel flag 20. Selector 6 is armed by
  `ACT2_CHK.RUN` after its later Act II checklist passes; the retail dispatch
  presents `RIP_WAC3.AVI` and then sets flag 314 (`got 3'rd ripper wac message
  (act 2)`). Selector 8 is armed by `DA3.RUN`; it saves the current palette,
  presents `WACGAMBI.AVI` with both presentation controls disabled, sets flag
  406 (`Falconetti's WELL`) followed by flag 309 (`played gambit e-mail`), and
  leaves the final presentation visible until any keyboard command or a left
  mouse-button transition acknowledges it. The retail cleanup then rebuilds
  the prior display and restores its saved palette. Selector 7 is armed by the
  Act III `WMAP3.RUN` checkpoint and presents `RIPWAC31.AVI` with normal
  presentation controls without changing milestone state. ScummVM treats
  selectors without an implemented retail branch as runtime failures when
  scripts arm them or saves attempt to restore them, rather than leaving an
  unsupported trigger active.
- `HandleSceneSelectionAction` at `0x191e2` records the selected destination
  before `RunFrontEndActionMenu` at `0x18b3a` runs the chapter-specific
  `WMAP*.RUN` checkpoint. ScummVM preserves that boundary after the map UI and
  before the destination handoff. In Act II, `WMAP2.RUN` first derives flags
  303, 308, and 315 from completed scene state. Because its selector-4 gate
  precedes the flag-315 derivation in the same callback, the Jordan briefing is
  armed only on the following map transition. Selecting that briefing presents
  `SJ_WACM.AVI`, sets flag 305, and restores Web Runner's Loft flag 41.
- `WriteEmergencySaveGame` at `0x1ae3c` writes `0x7d` bytes for this store,
  exactly 125 bytes or 1,000 bits; `RestoreSavedRunState` at `0x1b8dd` caps the
  restored payload to the same size. ScummVM's engine-owned `Milestones`
  service mirrors the 1,000-bit store and centralizes script, world-map, WAC,
  and puzzle access. Its serializer retains the pre-existing ScummVM layout of
  one byte per flag so current engine saves remain compatible.

## Cyber

- Scene action 6 reaches `RunCyberMenuSceneTransition` at `0x2a86f`. It raises
  the Cyber-active flag at `0x843c0`, saves the current palette, chooser and UI
  registries, scene image, and audio-object table, switches the asset directory
  to `CYBER`, selects cursor row 22, presents `DECKIN.AVI`, and runs
  `CYBRMENU.RUN` as a nested scene runtime with toolbar action mask zero. On
  return it restores those objects and the captured scene before clearing the
  active flag. ScummVM's `CyberManager` keeps the same ownership boundary using
  in-memory display, palette, named-audio, and script-runtime snapshots around
  the nested script and leaves the toolbar inactive while that boundary owns
  input.
- The `DECKIN.AVI` call site at `0x2a991` supplies
  `PollPresentationEscOrSpaceCommand` at `0x49039` to
  `RunMediaPresentation`. That callback consumes Escape to stop the entry
  presentation and Space to pause or resume it; both commands remain active
  before `CYBRMENU.RUN` begins.
- `CYBRMENU.RUN` has 94 frames and 70 interactions. Frames 33 through 48 are
  the sixteen-position carousel: position 0 is Exit and positions 1 through 15
  are Cyber programs. Each position exposes `left`, `right`, and `choose`
  interactions with cursor indices 20, 21, and 23. The script's transition
  frames use `ICTxy.SMK`; its large-icon frames use `ICL_EXIT.SMK` and
  `ICL_1.SMK` through `ICL_15.SMK`. The initial callback loads the carousel's
  `KJ1.WAV` and `IC_*.WAV` audio set and selects frame 33.
- Thirteen carousel programs enter password frames `IC_P1`, `IC_P2`, `IC_P3`,
  `IC_P4`, `IC_P5`, `IC_P6`, `IC_P8`, `IC_P9`, `IC_P10`, `IC_P12`, `IC_P13`,
  `IC_P14`, and `IC_P15`. Each idle callback uses opcode `0x24`
  (`HandleSceneEntryWaitForSceneFrameCounter` at `0x1633e`) to delay the
  callback until a specific movie frame before opcode `0x19` invokes
  `HandleSceneEntryAsyncTextRequest` at `0x157a1`. Layout variant 2 creates a
  194-by-20 text field at screen (228,312) with a 60-character limit. The
  request retains the callback continuation while the movie continues; if the
  movie ends first, its final frame remains visible until input completes. Entered
  and expected strings are compared by
  `StringsEqualIgnoringNonAlnumCaseInsensitive` at `0x1eff3`, so ASCII case,
  spaces, and punctuation do not affect the result. A correct answer continues
  to the callback's success frame; Escape or a mismatch selects the encoded
  `IC_P*r` reverse frame without changing the location milestone. Once the
  success frame sets its milestone, later visits bypass the prompt. The encoded
  answers in position order are `odysseus`, `horoscope`, `berman4`,
  `circus maximus`, `exterminator`, `vulcan`, `anachrony station`,
  `digital eden`, `psy bard`, `warp`, `pegasus`, `orestes`, and
  `leather apron`; positions 7 and 11 dispatch KA and KR directly.
- `PollInteractionAndResolveSelection` at `0x13c8d` advances the suspended
  frame-idle callback and services its chooser control before translating the
  returned command into a scene interaction. Navigation commands that do not
  match an enabled interaction are consumed without ending the interaction
  poll; in particular, Left, Right, and Enter cannot interrupt an `IC_P*`
  movie while its frame-idle callback is still waiting to create the password
  control. While an async text request owns input, Enter and Escape therefore
  complete that request rather than selecting a Cyber carousel hotspot or
  terminating its media presentation.
- `CreateSceneRuntime` at `0x12be7` stores the toolbar action mask supplied by
  `RunSceneScriptLoop` at runtime offset `+0x183`. Both `CYBRMENU.RUN` and
  `KR.RUN` are entered with mask zero, so `RunFrontEndActionMenu` at `0x18b3a`
  creates no toolbar controls for either nested runtime. F1 still selects Cyber
  help table `0x1a4` through `PollInteractionAndResolveSelection` at `0x13c8d`.
  Scene action 37 updates this 16-bit mask in place. The same polling function
  passes it to `ResolveFrontEndActionIdFromInput` at `0x14001`, so mouse and
  keyboard front-end actions share the enabled set. Main scene scripts use zero
  to suppress the toolbar during automatic outcome presentations, `0xffff` to
  restore every action, and `EH3.RUN` uses `0x1fd` to exclude scene selection.
  The carousel maps Left, Right, Enter, and Escape through the 16-bit keyboard
  command at interaction-record offset `+0x13`; scene action 9999 is its
  explicit nested-runtime terminator. Scene action 31 is an explicit no-op
  branch in `DispatchSceneEntryAction` at `0x36892`.
- Carousel program callbacks ultimately enter the preserved-state dispatcher
  `DispatchKSceneActionBand` at `0x36e84`. Actions 40 through 56 select the
  KA, KB, KC/Wofford, KD, KF, KG, KH, KI, KJ, KK, KL, KM, KN, KP, KQ, and KR
  program families (action 44 is absent). That dispatcher has a second
  palette/UI/chooser/audio preservation boundary around the selected program;
  it is separate from the Cyber menu transition owned by action 6.
- Action 42 checks `ResolveHighestSetSelectionFlag` at `0x20394`. Chapter 3
  and later enter `KC.RUN`; earlier chapters call
  `RunWoffordInteractiveMediaScene` at `0x2ac04`, which sets the action
  argument as a milestone and loops `WOFFORD.SMK` from frame 1. The literal
  `0x0f` pushed at `0x2adbe` is the unused fourth `RunMediaSequence` argument;
  the loop-start argument is the zero pushed at `0x2adaa`, which selects the
  frame-1 reset path in `RunMediaSequence` at `0x1e516`. It starts
  `WOFFORD0.WAV` at entry, arms `WOFFORD1.WAV` and two choice controls at
  frame 20, and maps control commands `0x672` and `0x673` to `WOFFORD2.WAV`
  and `WOFFORD3.WAV`. The queue call at `0x2ad60` assigns all four descriptors
  audio mix profile 3, corresponding to the Video/Speech volume. The nested
  controls use cursor row 16, active follow-up audio uses row 19, and Escape
  returns through the surrounding Cyber snapshot. Nonzero loop starts used by
  other media sequences enter `SeekSmackerPlaybackFrame` at `0x50a88` and
  reparse the requested packet over the retail decoder's existing wrapped
  framebuffer. ScummVM instead clears its offscreen Smacker surface and
  deterministically replays through the requested frame. The control rows at
  `0x2a824` map to physical rectangles
  (205,203)-(416,296) and (16,73)-(618,335), with the smaller first control
  retaining hit-test priority inside the full-presentation second control.
- Action 40 is the bespoke branch `RunKaDialogueScene` at `0x2aef5`, represented
  by ScummVM's self-contained `scenes/LibrarianScene` rather than a scene-script
  loop. It presents
  `KA_DECK.AVI`, then advances
  `KA_LOOP.SMK` continuously while servicing a talk control at
  (294, 103)-(418, 147), a card control at (413, 213)-(467, 303), and a
  dialogue-choice chooser. The idle cursor is row 14; the two fixed controls
  select rows 11 and 6. `LIBRARY0.WAV` through `LIBRARY3.WAV` and
  `DECK10.WAV` provide the presentation cues, while the first and repeated talk
  actions select `LI1_1_V1.WAV` and one of `LI1_1_Z1.WAV`/`LI1_1_Z3.WAV`.
  Starting either a talk or choice voice applies and stores cursor row 19,
  disables the fixed and chooser controls, and leaves Escape as the only
  meaningful input until the managed voice completes. Completion restores
  cursor row 14 before rebuilding the available choices.
  Its scene-action argument is copied to a local but otherwise unused by the
  retail function.
  Escape stops an active managed voice first; when no voice is active it exits
  the dialogue and returns through the surrounding Cyber snapshot.
- The Ka entry brackets `KA_DECK.AVI` with display command `0x14`; the second
  call at `0x2b05b` reaches `ClearGenericVideoLogicalPage` at `0x45ed8` before
  the 640x300 loop is installed at physical y=50. This clears the full logical
  page so the top and bottom bands contain palette index zero rather than stale
  pixels from the preceding 640x400 deck presentation.
- The Ka chooser sources the one-based `GAMETEXT.TF` resource IDs `0xaa`
  through `0xad` through `ResolveStartupResourceString` at `0x1f7a2`.
  ScummVM's decoded string array is zero-based, so each resource ID is
  decremented only at lookup; resource `0xad` is therefore “Ask to sign out an
  audio editor,” not the following `0xae` “Decryption Completed:” string.
  Consuming a
  choice sets flags `0x14a` through `0x14d` and starts, respectively,
  `LI1_1_VA.WAV`, `LI1_1_VB.WAV`, `LI1_1_VD.WAV`, or `LI1_1_VC.WAV`.
  Each available item retains the retail choice ID 0 through 3 even when
  earlier choices have been filtered out; the selected ID drives both the
  progress-bit offset and the voice switch in `RunKaDialogueScene`.
  Choice `0x14c` is exposed after the card presentation sets flag `0xcc`; on
  voice completion it enters the separate book-code puzzle. Choice `0x14d` is
  gated by act-one flag 2 and the startup asset-catalog flag for `SB2_1_D`;
  once consumed, it presents `KA_CD.AVI` and sets flag `0x54`.
- `RunKaDialogueScene` directly calls `ConfigureSceneEntryChooserLayout` at
  `0x18740` for opcode `0x17`, builds a UI item-list model, and passes it to
  `CreateListChooserControl` at `0x56ec2` with control ID `0x4e2`. This is the
  same chooser construction and `ProcessChooserControlInput` path used by
  `HandleSceneEntryChoiceListLifecycle`; Ka does not draw a separate dialogue
  window. ScummVM therefore feeds the Ka game-text records into the shared
  `DialogueChooser`, including its three-row viewport, arrow controls,
  `small.fnt` metrics, normal colors 251/0, selected colors 4/248, mouse hover,
  and keyboard selection behavior.
- In the Ka loop, `AdvanceCustomPacketPlaybackFrame` is followed by
  `AcquireUiSelectionPresentationOverlay`, a complete
  `DispatchDisplayDirtyRegionUpdate`, and `ReleaseBorrowedPresentationOverlay`
  before `AdvanceSmackerPlaybackFrame`. ScummVM mirrors that presentation
  boundary by deferring sequence-callback screen submission until the shared
  chooser has been drawn, then presenting the composited frame once. Submitting
  the movie before the callback exposes an intermediate undecorated frame and
  makes the portion of the chooser inside y=50 through y=349 flash.
- Action 56 is the KR branch of `DispatchKSceneActionBand`; it calls
  `RunSceneScriptLoop("kr", 0, 0)`. ScummVM enters `kr.run` through a nested
  `CyberManager` snapshot which preserves the carousel script, active frame,
  interaction enablement, pending transition state, indexed display, palette,
  cursor state, and named audio. KR's runtime exit restores that snapshot and
  resumes the original `CYBRMENU.RUN` callback instead of unwinding to the
  pre-Cyber scene.
- `KR.RUN` uses only scene actions 35, 36, and 9999. `DispatchSceneEntryAction`
  at `0x36892` maps action 35 to `SetUiSelectionIndex` at `0x4a3fe`, which
  clamps and stores the requested cursor row without changing its presentation.
  Action 36 calls `DispatchUiSelectionIndexChange` at `0x4a630`, which applies
  a valid row to the active selection presentation. KR initializes both with
  row 14. Its startup callback then plays `VH_1.AVI` with keyboard controls
  disabled. `ExecutePresentationEntry` at `0x1754b` passes that zero control
  flag to `RunMediaPresentation` at `0x17917`, whose zero branch retains the
  movie's final rendered page for the following type-2 `interface` frame.
  `ServiceUiControlStateSelection` at `0x4a912` starts each poll from the stored
  row 14 and temporarily dispatches the hovered control's row; every visible KR
  interaction uses row 16. KR therefore shows the standard white arrow while
  idle and the red arrow over an interactive region. Its interface and paper
  frames bind Escape (`0x001b`) to an invisible `exit` interaction whose
  callback dispatches action 9999, returning to the suspended Cyber carousel.
  The zoomed `paper*b` frames instead bind Escape to `unzoom`, returning to the
  corresponding paper frame before a subsequent Escape exits KR.

## Self-contained Scene Handlers

- `DispatchSceneEntryAction` at `0x36892` calls bespoke scene handlers directly
  for actions whose state cannot be expressed by the compiled scene-script
  loop. These handlers keep their controls, media callbacks, audio descriptors,
  and progress transitions local. `RunTubeSwitchScene` at `0x25e18` and
  `RunKaDialogueScene` at `0x2aef5` are represented by `TubeScene` and
  `LibrarianScene`; their distinct state machines remain in those derived
  classes.
- The direct handlers share a verified ownership boundary. `RunKaDialogueScene`
  and `RunCainDialogueScene` at `0x2c160` snapshot the chooser registry, UI
  control list, active palette, and borrowed presentation origin, then restore
  them after stopping scene audio and releasing local controls. Puzzle handlers
  such as `RunCrystalPiecePlacementPuzzleScene` at `0x2710c`,
  `RunRolodexSequencePuzzleScene` at `0x280ae`, and
  `RunShockLeverPuzzleScene` at `0x3affb` likewise install a local cursor/control
  set and release it with their audio state on exit. ScummVM's `Scene` base
  models the common engine, chooser, indexed-palette, cursor/input, toolbar, and
  sound-handle boundary; media and interaction behavior remains owned by each
  concrete scene.
- Action 62 enters `RunCainDialogueScene` at `0x2c160`. The handler loops
  `CAINLOOP.SMK` with `CAINLOOP.WAV`, exposes the talk control at
  `(244,120)..(524,270)`, and builds chooser `0x4e2` from `GAMETEXT.TF`
  resources `0xbe..0xc5`. Selecting a branch sets its corresponding bit in
  `0x159..0x160` before playing `MC3_1_G1/G2`, `MC3_1_V3`, or
  `MC3_1_VA/VD/VE/VF`; the initial and repeat talk controls use
  `MC3_1_V1` and `MC3_1_Z1`. Choice `0x15f` gates the one-time
  `CAIN_ZAP.AVI` presentation through bit `0x60`, while completed branches
  propagate the verified `0x198`, `0x19e`, and exit-time `0x161` follow-up
  bits. Escape stops an active managed voice first and exits the handler only
  when no voice is pending.

## Combat Encounters

- `DispatchSceneEntryAction` routes actions 15, 26, and 27 through the same
  `RunCombatEncounterScene` at `0x31436`, supplying `MECHINI%d.INI`,
  `RATINI%d.INI`, or `ATKINI%d.INI`. The `%d` value is the configured combat
  level byte at `0x8a177`. This is a reusable combat controller rather than
  three unrelated scene handlers; ScummVM keeps it under `combat/`, with the
  concrete Mechini, Ratini, and Atkini encounters binding actions 15, 26, and
  27 to the shared controller. Atkini preserves the retail six-scene routing
  tables at `0x3134c` and `0x31358`: scenes 3/4 choose among `{0,4,5}`, while
  scene 5 chooses among `{1,2,3}`. Ratini loads `LOOP3.WAV` and `LOOP4.WAV`
  and uses the self-reinstalling callback `QueueRandomAudioTriggerPairEntry`
  at `0x313ec` to select another one whenever the current ambient loop ends.
  `combat/resources.cpp` owns INI, DAT, PRJ, and bitmap-set loading for that
  controller; `combat.cpp` owns the active encounter, input, timing, audio,
  meter, hit, effect, and rendering loop. Both remain methods of the same
  `CombatEncounter`, so this source split does not add a runtime abstraction
  between the retail resource tables and their consumers.
- `RunCombatEncounterScene` reads the combat level once at `0x31563` and uses
  its value `1`, `2`, or `3` only to format the numbered encounter INI. There
  is no separate difficulty multiplier. `LoadCombatEncounterResourceSet` at
  `0x34d6e` then takes the selected INI as the source of scene resources and
  every health, creature, weapon, and shield damage, recharge, drain, and DOS
  tick interval. ScummVM snapshots the current Options Panel combat level on
  encounter entry, selects the same numbered INI, and reports both values in
  the combat lifecycle log. Some retail data layouts contain only
  `ATKINI1.INI` and `RATINI1.INI` even though `RIPPER.INI` defaults combat
  level to 2. ScummVM therefore attempts the executable's numbered name first,
  then uses the packaged level-1 definition only when that requested file is
  absent; this compatibility fallback does not affect Mechini's complete
  three-file difficulty set.
- `LoadCombatEncounterResourceSet` at `0x34d6e` reads numbered `sceneN` entries,
  loads each `<scene>.SMK`, `<scene>DAT.DAT`, and optional `<scene>PRJ.PRJ`, and
  loads the configured crosshair, explosion, shading, and audio resources.
  `LoadPresentationFrameAudioCueMap` at `0x3574a` leaves its zero-filled frame
  cue array in place when a PRJ is absent, as it is for Ratini scene `RAT0`.
  A DAT file has a 100-byte header, a 32-bit frame count, one six-byte state
  record per frame,
  then nine-byte hit regions. Each hit region stores its one-byte damage/effect
  type followed by signed 16-bit `y`, `x`, `height`, and `width` values;
  `RunCombatEncounterScene` compares those pairs with the logical 320-by-200
  pointer at `0x320d0`. A PRJ file has the same 100-byte header, a 32-bit frame
  rate, a 32-bit sound count, 60-byte sound paths, and two bytes per frame for
  cue index and volume. `COMBAT8.WAV` and `COMBAT12.WAV` are normalized to
  `MECHWAV6.WAV` and `MECHWAV5.WAV` by the original loader.
- The encounter owns four percent meters for health, creature, weapon, and
  shield. Their damage, recharge, drain, and tick intervals come from the INI;
  timing uses the DOS 18 Hz clock. Left mouse discharges the weapon and applies
  a hit region's damage/effect entry, while right mouse holds and drains the
  shield. `COMBAT0.BBM` supplies the right status panel, `COMBAT2.BBM` and
  `COMBAT3.BBM` are its filled and empty meter segments, and the four configured
  crosshair strips meet at the logical 320-by-200 pointer position. The fixed
  meter-anchor table places player health and creature health at the upper left
  and right, then weapon charge and shield charge at the lower left and right.
- Each frame updates the health and creature timers before routing an incoming
  attack. A shielded hit applies shield damage before the timed shield drain;
  an unshielded hit recharges the inactive shield before applying health damage.
  Weapon recharge or firing follows the attack branch. ScummVM saturates direct
  weapon and shield damage at zero rather than preserving the original's
  unbounded subtraction.
- `RunCombatEncounterScene` anchors `COMBAT0.BBM` at logical X
  `320 - bitmapWidth`; its meter coordinates are local to that right-hand
  panel. Visible status artwork uses the shared interface palette bands at
  indices 4 through 9 and 246 through 255, which remain patched while combat
  Smacker palettes change. `COMBAT1.BBM` and `COMBAT4.BBM` provide the active
  and inactive indicators for the current enemy attack, last fired shot's hit
  result, weapon-button state, and effective shield state. Effects and the
  crosshair are rendered before the complete panel, matching the original
  composite order. Player depletion shares the Escape cleanup path and leaves
  the supplied completion flag clear. The original keeps ten active hit effects
  and replaces the farthest-advanced effect when that table is full.
  `LoadBitmapAssetGroupRecord` at `0x35a41` caches negative half-frame extents,
  `SpawnBitmapAssetGroupEntry` at `0x35b01` starts an effect at the hit point and
  scale 100, and `AdvanceAndRenderBitmapAssetGroupEntries` at `0x35bf8` centers
  and advances it through the configured explosion library. The custom bitmap
  decoder at `0x53f60` maps the asset's transparency key through its
  zero-initialized color-remap table. The Atkini and Ratini explosion frames
  use an unmapped `0xff` key, which therefore resolves to palette index 0; the
  centered explosion is composited with that zero-valued backdrop transparent.
- Combat palettes are derived from the current Smacker palette. Indices 1-3 and
  10-239 use luminance `(30R + 59G + 11B) / 100`; shield, target-hit, and
  player-hit branches then apply the alternate blue, magenta, or red components
  built in `RunCombatEncounterScene`. Creature depletion sets the supplied
  completion flag, while player depletion or Escape exits without setting it.
  F1 opens the encounter-specific help resource, F10 captures a screenshot, and
  the hidden case-insensitive `arcade` keyword completes the encounter.

## Puzzles

- Scene action 17 calls `RunDateSelectionPuzzleScene` at `0x3d5ed` with the
  caller-supplied completion flag. It loops `EH_EEG0.SMK` or `EH_EEG1.SMK`
  from frame 10 while `ServiceDateSelectionTextEntryChooserCallback` at
  `0x3d464` owns a primary-style text control with a 22-character limit and
  help table `0x1b6`. Escape exits the puzzle; a submitted token reaches
  `ProcessDateSelectionTokenMatch` at `0x3d8f0`. The retail call pushes loop
  start `0x0a`; its callback does not request final-frame retention, so the
  decoder returns to that frame while the chooser remains active. The callback
  services the chooser registry but does not explicitly reapply the selected
  cursor on every media frame.
- That matcher accepts five case-insensitive spellings for each date from
  November 18 through November 23: numeric forms with `-` or `/`, abbreviated
  `nov` with or without a period, and the full month name. November 19 sets
  flag 339, November 20 sets flag 340, and any of the other four accepted
  dates sets flag 341. These three progress flags persist across visits. The
  supplied completion flag is set only during puzzle cleanup when all three
  are present; partial or invalid input does not set it.
- Scene action 18 calls `RunKiSkullMazePuzzleScene` at `0x2ff55` with the
  caller-supplied completion flag in EAX. It presents the 640-by-400
  `KI_1.PCX` backing, loads `KI_LID0.BBM` through `KI_LID4.BBM`, and builds
  controls `0x672` through `0x6b1` at the 64 physical X/Y points rooted at
  `0x84574` and `0x844f4`. `KI_SKULL.SMK` animates at the upper right while
  `KI_MAN0.SMK` through `KI_MAN4.SMK` supply idle, down, up, left, and right
  player motion. The foreground-cell list at `0x2ea22` keeps the ten nearest
  lids in front of the skull animation. The configured start is a zero-based
  control index: `start pos = 3` selects the fourth lid at `(137,261)`. Retail
  derives the idle sprite's top-left as `(131,172)` from that lid, the 112-pixel
  `KI_MAN0.SMK` height, and its local row/column offset tables. The display
  wrappers at `0x5d450` and `0x4e4b0` place the destination row in ECX and the
  column in EDX; `BlitScaledGenericVideoRectangle` at `0x464f3` confirms the
  ordering by multiplying the ECX-derived descriptor field by the display
  pitch. `InitializeSmackerPlaybackBitmapDescriptor` at `0x20684` copies the
  Smacker height from playback state `+0x08` to descriptor `+0x00` and width
  from state `+0x04` to descriptor `+0x04`. Consequently, the height-derived
  large offset applies to Y while the small second-table offset applies to X.
- The `[setting]` section of `KI%d.INI` supplies the MSVC-LCG seed, playback
  rate, post-move lid delay, starting cell, initial skull delay, delay decrease
  interval, decrement, and minimum. `GenerateRandomInt15` at `0x49a2f`
  initializes each cell to state `0..3`; states 1 and 3 are selectable while
  state 4 is a permanent skull block. After an adjacent move, a state-1 lid
  toggles its four cardinal neighbors and state 3 toggles its four diagonal
  neighbors. Timed hazards choose a non-skull cell other than the protected
  destination, then reduce their spawn delay according to the INI values.
  `KI_2.WAV` accompanies movement, `KI_0.WAV` accompanies neighbor changes,
  and `KI_1.WAV` accompanies new skulls.
- Reaching logical row 7 starts the 54-DOS-tick success delay and then sets the
  supplied flag. Having no selectable cardinal neighbor starts
  the same delay but exits without setting the flag. Escape also exits without
  completion, F1 opens help table `0x1b9`, and the case-insensitive hidden
  keyword `pretzel` completes immediately. Cleanup fades and clears the
  puzzle display before `KI.RUN` selects its win or loss presentation.
- `PUZZLE_HELP` adds a non-retail KI overlay without entering the retail state
  machine. A bounded breadth-first search copies the current 64-cell model,
  simulates only legal jump-and-toggle transitions, and dithers the first cell
  on a route to row 7. The recommendation is cleared during movement and lid
  animation and recomputed after a completed toggle sequence or skull spawn;
  it does not alter live cells, random state, input, hazards, or completion.
- Scene action 21 calls `RunSixDigitCodePuzzleScene` at `0x3e913` with the
  caller-supplied completion flag. It retains the current scene framebuffer,
  loads `BNKNUM0.BBM` through `BNKNUM9.BBM` for the six display cells and
  `BNKS0.BBM` through `BNKS11.BBM` for ten digit, Enter, and clear controls.
  Escape exits and F1 opens help table `0x1b7`; Enter compares all six cells
  against the short array at `0x84d50`, whose value is `185621`. Clear or a
  rejected entry empties the cells and queues `BNKWAV3.WAV`. A correct entry
  queues `BNKWAV2.WAV`, alternates the captured blank and filled display five
  times at five DOS ticks per phase, then sets the supplied completion flag.
- Scene action 23 calls `RunTarotCardPuzzleScene` at `0x3eece` with the
  caller-supplied completion flag. `ACT4.RUN` supplies flag 231. The handler
  presents `BEGIN.AVI`, opens `TAROT.PL`, reads `CARD%d.INI` for the optional
  time limit, and creates controls `0x672` through `0x680` from the 15
  ten-byte X/Y/card records at `0x3e78d`. After the routine removes the
  retail 90-pixel Y origin, the first six empty target slots lie at y=158 and
  the nine source cards lie at y=14. Those are logical coordinates in the
  640-by-300 scene page, which ScummVM places at physical y=50. `BKGRND2`
  supplies that scene page, `SI1` through `SI9` supply the 68-by-112 slotted
  cards, and `BI1` through `BI9` supply the 118-by-198 card attached to the
  pointer. `InitializeTransientPresentationOverlay` clips that held-card image
  to logical x=0..639 and y=0..299, preventing it from modifying the retained
  display bands above and below the scene page.
- A click swaps the selected slot's card ID with the held card ID. Placing a
  held card redraws the slot, taking an occupied card installs its large
  pointer overlay, and every swap compares the first six slots with the short
  target table `8, 3, 9, 7, 2, 1`. `REAPSONG` is the puzzle loop;
  `TAROT0`, `TAROT2`, and `SND0` through `SND8` are its placement, cleanup,
  and card cues. Escape or expiration leaves the completion flag clear and
  presents `TAROTDEA.AVI`. A match sets the supplied flag, shrinks away cards
  left in source slots over the retail eleven-step cleanup, and enters
  `RunEndingSelectionEpiloguesAndCredits` at `0x43adb`.
- The ending selector maps the randomly selected Ripper flags to retail ending
  indices as flag 6 to 3, flag 7 to 1, flag 8 to 0, and flag 9 to 2. It uses
  the 20-frame `END_CURS.PL` pointer while the third branch of `RIPMID.AVI`
  plays. `RunPacketizedMediaPlaybackCore` at `0x5b592` assigns the three media
  branches sequence IDs 1 through 3. `InitializeEndingSelectionPlaybackCallback`
  at `0x43840` installs the borrowed cursor presentation and moves it to logical
  position (100,160), physical position (200,320) in the 2:1 ScummVM output,
  only for sequence 3; the first two branches remain noninteractive. Once that
  state is active,
  `HandleEndingSelectionThrowTargetCallback` at `0x438cf` starts `SPIN_1.WAV`;
  a held primary button is accepted only inside one
  of the eight frame-gated logical 320-by-200 target rectangles at `0x84fe4`
  and queues `BALLTHRO.WAV`. Each 28-byte target record stores frame minimum
  and maximum followed by Y minimum, X minimum, Y maximum, and X maximum;
  the callback compares X through offsets `+0x0c/+0x14` and Y through
  `+0x08/+0x10`. The one-based target result wraps modulo four to
  choose `END_DB`, `END_DF`, `END_DC`, or `END_DM`. A correct choice conditionally
  adds `END_HIM` for ending 3 or `END_HER` for endings 0 and 2, always plays
  `QUIN_WIN`, and then selects `Q4_V4`, `Q4_V3`, `Q4_V6`, or `Q4_V5` by ending
  index. A wrong choice or no selection goes to `RIPFINAL`. All paths finish
  with `CREDITS1.AVI` and `CREDITS2.AVI`.
- `PUZZLE_HELP` adds a non-retail ending-selector visualization after each
  decoded frame without changing the retail input callback. It dithers and
  borders only the currently frame-gated click rectangle, using green when
  that rectangle's modulo-four ending index matches the Ripper flag and red
  otherwise. The helper derives its display state from the immutable retail
  target table and current frame; it never supplies a command or modifies the
  ending selection, milestone, or media route.
- Scene action 25 calls `RunKkTileMatchPuzzleScene` at `0x2fa31`. It loads the
  difficulty-selected `KK%d.INI` from `SCRIPT.PL`, opens `KK.PL`, and creates
  controls `0x672` through `0x681` at the 4-by-4 Y/X coordinate table rooted
  at `0x84474`. `BASE` supplies the 640-by-400 display and palette, `BIG_S`
  supplies the closed tile, and `FLIP1` through `FLIP13` are the 32-frame
  barcode reveal animations. Each click first applies that control's partial
  16-slot copy row from the INI, animating changed slots over 25 five-tick
  steps, and then opens the selected slot through the midpoint of its current
  state's `FLIP%d` sequence. `CHIP0` accompanies slot movement, `CHIP2` starts
  a reveal or reset, and `CHIP1` marks tile presentation and settling.
- After three slots are open, `CountActivePuzzleTilesMatchingFirstState` at
  `0x2f3be` succeeds only when all three state values match. A mismatch remains
  visible for up to 250 timer ticks or until input, then each active animation
  finishes and returns to `BIG_S`. Escape exits, F1 opens help table `0x1b8`,
  and the hidden case-insensitive `asparagus` keyword sets the caller-supplied
  completion flag through the same cleanup path as a three-tile match.
- The ScummVM-only `PUZZLE_HELP` overlay is isolated in
  `KkTileMatchPuzzle::DebugHelper`. It simulates the retail snapshot-copy,
  selected-slot reveal, three-active-slot test, and mismatch reset without
  mutating the live puzzle, then breadth-first searches up to six clicks for a
  winning sequence. A palette-255 yellow border and sparse fill mark only the
  next recommended control. The helper clears the stale recommendation during
  retail animations and recomputes after each reveal or reset and whenever the
  console toggle changes; it does not alter input or completion validation.
- `EH_WAIT.BBM` is drawn at physical `(186,275)` while the token matcher runs
  its 17-DOS-tick progress sweep. `EH_EEG0.WAV` and low-volume
  `EH_EEG2.WAV` start with the puzzle; `EH_EEG1.WAV` accompanies the two
  individually tracked dates, while `EH_EEG4.WAV` is the rejection cue.
  ScummVM maps the retail chooser callback to its scene-entry text control and
  keeps the movie callback responsible for all keyboard input while active.
- Opcode `0x18` passes its first argument to `DispatchSceneEntryAction` at
  `0x36892` and its second argument through the original EBX argument channel.
  Scene action 4 calls `RunCalculatorPuzzleScene` at `0x269a5`; scene action 29
  (`0x1d`) calls `RunCrystalPiecePlacementPuzzleScene` at `0x2710c`.
  `CA1.RUN` supplies flag 0 to the calculator and `JA1.RUN` supplies milestone
  206 (`solved crystal puzzle`) to the crystal puzzle.
- Scene action 1 calls `RunCircuitChipPlacementPuzzleScene` at `0x28aa4`
  with the caller-supplied completion flag. `ED2.RUN` callback `0x5e9`
  supplies milestone 210 from frame `T7`. The puzzle presents `EDZ10.SMK`,
  then manages sixteen `CHIP1.BBM` through `CHIP16.BBM` controls in the
  source bank and sixteen target-slot controls, with `EMPTY.PCX` providing
  the chip-removal backing. Both
  coordinate tables at `0x842fe` and `0x8433e` store Y/X pairs; ScummVM
  normalizes them to physical X/Y coordinates for the full 640-by-400
  presentation.
- Selecting a source chip or an occupied target attaches that chip to a
  cursor-centered transient overlay. The source-selection path restores the
  selected control rectangle from `EMPTY.PCX`, stores `-1` in the source array
  at `0x2941e`, and initializes the transient overlay at `0x29482`; source
  chips must therefore not be retained in the static redraw backing. A
  subsequent selection returns the held chip to any empty source position or
  places it in an empty target whose connector type matches. The target
  solution stored one based at `0x25db1` is
  `[14, 4, 11, 6, 15, 16, 5, 1, 7, 12, 9, 2, 13, 8, 3, 10]`.
  The meter groups at `0x25dd1` are
  `[0, 14, 7, 15]`, `[1, 11, 8, 2]`, `[3, 6, 13, 12]`, and
  `[4, 9, 5, 10]`; each group advances or regresses the corresponding
  `METER2.SMK` or `METER3.SMK` segment. `METER.PL` supplies the repeating
  idle overlay at physical `(27, 196)`.
- All four correct groups set the supplied flag and exit after the final
  meter transition. Escape exits without setting it, F1 opens help table
  `0x1ae`, and the case-insensitive hidden keyword `theory` completes the
  puzzle. `CIRC1.WAV` is the looping ambient cue, `CIRC0.WAV` accompanies
  the idle meter cycle, `CIRC2.WAV` and `CIRC3.WAV` accompany regression
  and progress, and `CIRC4.WAV` accompanies a valid drop.
- Scene actions 16 and 60 both pass their flag argument to
  `RunKeyGroupPuzzleScene` at `0x3ca1f`. The full-screen `KGP_2.PCX` backing
  presents twenty `KG_P0.BBM` through `KG_P19.BBM` book-index entries in the
  left column and fourteen empty key-title rows in the right column. The
  source control Y table at `0x84c9a` uses fixed X=14; the target Y table at
  `0x84cc2` uses fixed X=330. A selected entry is removed from its current
  row and follows the pointer with the original click offset until the player
  selects an empty row in either column. Both slot arrays persist across
  re-entry for the lifetime of the process.
- The exact right-column solution at `0x84cfa` is
  `[0, 4, 10, 15, 1, 13, 16, 5, 9, 7, 18, 19, 14, 11]`. A match or the
  case-insensitive hidden keyword `sponge` sets the caller's completion flag
  and exits. F1 opens help table `0x1b5`. Escape clears navigation flags 342
  and 343 and sets cancel flag 344; the left and right 50-pixel edge controls
  respectively set only flag 342 or flag 343 before returning to the script.
  Those edge controls are disabled while an entry is attached to the pointer,
  while the keyboard Left and Right commands remain active.
- If flag `0x55` is set, the circuit puzzle also presents `ED_WAC.SMK`,
  resource `0xb6`, and the `CIRCTMPL.PCX` backing; otherwise it selects
  `CIRCTMP2.PCX`. `CIRC5.WAV` accompanies the optional manual branch.
  The `RunMediaSequence` call at `0x28ed2` supplies physical Y=`0x144` in EBX
  and X=`0x5c` in ECX, placing the 320-by-76 `ED_WAC.SMK` patch at
  screen `(92,324)`. The wrapped text call at `0x28f04` places a
  275-by-50 control at `(114,350)` using
  `g_primaryChooserPresentationTemplate` at `0x8a284`, not the tertiary WAC
  template used by the database's standalone circuit-manual entry. The primary
  template uses `7PT_FONT.FNT`, zero padding, and 9-pixel rows. The wrapped
  control preserves the `ED_WAC.SMK` client backing. For its active text rows,
  `RenderChooserTextRow` at `0x58651` selects the style pointer at template
  offset `+0x0c`; startup initializes that pointer from `0x84038`, whose value
  is palette index 254. The shared `MNU0` palette maps index 254 to red
  `(200, 0, 0)`. Its two `MNARROW0.BBM` through `MNARROW3.BBM` scroll controls
  begin five pixels beyond the panel's right edge and align with its top and
  bottom.
- Scene action 10 calls `RunTableGateLeverPuzzleScene` at `0x38eb8` with the
  caller-supplied completion flag. `GB1.RUN` callback `0x74d` supplies milestone
  223 (`solved wofford table puzzle`), tests it after the puzzle returns, and
  selects the success frame only when that flag is set. Escape and failed
  marker traversals return without setting it.
- Scene action 11 calls `RunCdInBookButtonSequencePuzzleScene` at `0x28427`
  with the caller-supplied completion flag. `JA2.RUN` callback `0xa41` supplies
  milestone 57 after retaining `CD_ZOOM.AVI`. The puzzle loads `CDINBOOK.PL`,
  presents `UP.PCX` in the 640-by-300 scene viewport, and creates button
  controls `0x672` through `0x677` from the positions at `0x25d29`. The
  six-button solution stored at `0x25d99` is `[1, 5, 5, 3, 6, 2]`; only that
  sequence sets the supplied flag. Escape and either 140-pixel side control
  exit without setting it, while F1 opens help table `0x1aa`.
- Scene action 7 calls `RunClockPuzzleScene` at `0x374c5` with the
  caller-supplied completion flag. `GB1.RUN` callback `0x577` supplies flag 222
  after presenting `GBZ3.SMK`. The puzzle retains its two analog positions and
  two digital-clock values across re-entry, and sets the flag only when the
  analog hands are at 7 and 8, the military clock reads 14:35, and the
  Egyptian clock reads 09:35 with its half-day marker selected. Escape exits
  without setting the flag, while F1 opens help table `0x1a2`.
- `RunClockPuzzleScene` creates controls `0x672` through `0x678` from the
  records at `0x846a6`. The clock face uses the indexed `CLOCK_TM.MCG` hit mask:
  left and right mouse select the minute and hour hands respectively from its
  twelve regions. The remaining controls advance or rewind the two digital
  clocks; advancing the Egyptian hour from 12 to 1 toggles its half-day marker.
  The scene loads this mask as one custom bitmap through
  `DecodeCustomBitmapAsset` at `0x53fdf`; unlike the numbered clock assets, it
  has no bitmap-set directory.
  `CLOCK1.SMK` and `CLOCK0.SMK` are entry and exit animations at scene-space
  `(282, 77)`. `PlayClockSmackerOverlayAnimation` at `0x38316` treats the first
  decoded pixel as transparent and restores the saved backing before every
  frame, preserving the retained `GBZ3.SMK` presentation around the overlay.
- Scene action 9 calls `RunGcCshFourChoiceSequencePuzzleScene` at `0x38871`
  with the caller-supplied completion flag. `GC1.RUN` callback `0x409`
  supplies milestone 224 (`solved cash register puzzle`) after presenting
  `GCZ1.SMK`; the next callback selects `GC_CSH_O.SMK` only when that flag is
  set, otherwise it presents `GCU1.SMK` and returns to the zoom interaction.
- The GC/CSH puzzle creates controls `0x672` through `0x675` from the rectangle
  table at `0x84700`, activates and applies default cursor row 14 on entry, uses
  cursor 16 over the four choices and cursor 7 over its two Escape regions, and
  restores row 0 on exit. Presentation activation refreshes the cursor frame and
  palette after GCZ1 and each puzzle media transition. Its input loop presents
  each cursor-service tick, matching the original active UI-selection
  presentation and keeping ScummVM's software cursor visible and animated. F1
  opens help resource `0x1a3`. Each choice plays
  `CSH_SND0.WAV`, presents the matching `GC_CSH0.AVI` through `GC_CSH3.AVI`
  at scene-space `(0, 64)` (physical `(0, 114)`), starts `CSH_SND1.WAV`, and
  animates its six `GC_CSHn0.BBM` through
  `GC_CSHn5.BBM` frames into the next slot from the table at `0x846ec`. The
  zero-based solution is `[1, 2, 3, 1]`. Every four-choice sequence rewinds
  and restores the captured 98-by-5 backing strip at physical `(421, 101)`;
  an incorrect sequence continues afterward, while a match sets the supplied
  completion flag and exits. Escape performs the same rewind for a partial
  sequence before exiting.
- The table-gate puzzle loads `TBLINI1.INI` through `TBLINI3.INI` according to
  the configured puzzle level. Each file supplies thirteen initial gate states,
  five gate indices for each of five levers, the number of lever activations
  before a traversal, and whether a lever can be reused within that activation
  group. The normalized path records at `0x84720` contain 123 marker nodes; the
  thirteen gate records at `0x84a02` replace one node's next link with either of
  two routes. Failed traversals preserve the toggled gate states, clear the five
  activation indicators, and re-enable all levers for another attempt.
- `RunTableGateLeverPuzzleScene` retains the final `GBZ1.SMK` frame and overlays
  `GB_TBL0.BBM` for the marker, `GB_TBL1` through `GB_TBL6` for the launch
  animation, `GB_TBL10` through `GB_TBL22` for gate states, `GB_TBL25` through
  `GB_TBL29` for activation indicators, and `GB_TBL30` through `GB_TBL34` for
  lever feedback. Lever controls use cursor 16, the two edge-exit controls use
  cursor 7, and entry stores and dispatches default row 14 before activating
  the UI-selection presentation. The active input loop presents every cursor
  service tick, preserving the visible software cursor after `GBZ1.SMK` and
  puzzle overlay redraws. Escape exits, and F1 opens help table `0x1a1` while
  the marker is inactive. A traversal begins at node zero, accelerates from one
  to six pixels per DOS tick, and sets the completion flag only after reaching
  the node whose terminal link is `-2`; the other terminal link is `-1`.
- The CD-in-book puzzle uses `P01.BBM` through `P06.BBM` for pressed-button
  feedback and `BUTON.WAV`/`BUTOFF.WAV` for the corresponding audio. It draws
  `P07.BBM` at the eight dirty-region origins stored at `0x25d59`; after each
  of the first five choices it restores a randomly selected marker backing,
  with repeated random selections allowed. A wrong six-button attempt pauses
  for four DOS timer ticks, plays `BUZER.WAV`, restores all eight markers, and
  waits for the buzzer before accepting another attempt.
- Scene action 12 calls `RunBoardArrangementPuzzleScene` at `0x39d8f` with
  the caller-supplied completion flag. `IB2.RUN` callback `0x536` supplies
  milestone 41 after retaining `IBZ1.AVI` and presenting `Q_P_15.WAV`.
  `BB_BG.PCX` supplies the 640-by-300 puzzle background; `SM_BB0.BBM`
  through `SM_BB7.BBM` are the eight draggable cards, while the corresponding
  `LG_BB` assets provide their 273-by-203 readable previews in the right-hand
  panel. The initial scene-space Y/X pairs at `0x84b39` persist across re-entry;
  the retail controls likewise store Y/X and height/width before ScummVM
  normalizes them to X/Y and width/height.
  Its active loop services `ServiceUiControlStateSelection` on every idle
  iteration, so the ScummVM loop presents every cursor-service tick even when
  the hovered card does not change.
- The board solution order stored at `0x374b1` is `[1, 0, 3, 6, 2]`.
  Each following card must sit entirely to the right of the previous card and
  overlap it vertically. There is no maximum horizontal gap, and exact vertical
  edge contact passes because the retail separation comparisons use strict
  less-than tests. The other three cards do not participate in validation.
  A newly selected card moves to the front of the hit-test and draw order.
  Drops wholly inside the preview panel animate back to their origin at fifteen
  pixels per DOS tick. The first valid arrangement sets the supplied flag and
  presents `Q_P_16.WAV`, but leaves the puzzle active. Escape or either
  80-pixel side control exits, F1 opens help table `0x1ab`, and `BB1.WAV`
  accompanies selection of a card that was not already frontmost.
- Scene action 38 calls `RunBoardGameScene` at `0x436c0` with the
  caller-supplied completion flag. `KL.RUN` supplies milestone 89 after
  presenting `KL_IN.AVI`. The puzzle opens `PUZZLE/CHESS.GL`; `BOARD` is its
  640-by-400 presentation, `BOARDTML` maps pointer positions directly to cells
  0 through 42 and exit control `0x81`, `PART_A00` through `PART_A04` and
  `PART_B00` through `PART_B04` are the two signed piece sets, and `CHESS0`
  through `CHESS4` are its archived audio cues. The board contains seven rows
  of six cells, plus shared destination 42 for a runner leaving a side edge
  inside its far three rows. Its initial signed layout is copied from `0x40c32`;
  positive pieces move first.
- `LoadBoardPuzzleAssetsAndLayout` at `0x4129e` does not load a destination
  highlight bitmap. `MarkLegalBoardMoveChoices` at `0x41dca` writes marker 8
  into the control-state array, and `UpdateBoardMoveHoverControlMode` at
  `0x42195` uses those bytes only to switch the pointer between cursor rows 14
  and 16. `RenderBoardState` at `0x418c8` draws the background and pieces, so
  selecting a piece adds no board-space outlines.
- `LoadBoardPuzzleAssetsAndLayout` loads the 0x1300-byte `BOARDPAL` shading
  block through `LoadShadingInfoBlock` at `0x4d160`. It consists of a
  0x300-byte VGA palette followed by a 0x1000-byte lookup cube that maps
  12-bit RGB (`rrrrggggbbbb`) back to an indexed palette color. When the cell
  passed to `RenderBoardPieceVisual` at `0x4160d` matches the selected-source
  global at `0x84f68`, the renderer attaches `BOARDPAL` with source opacity
  `0xa0`, blends the piece with the already-rendered board, and quantizes the
  result through that cube. `AnimateBoardIntroFadeIn` at `0x4365c` confirms
  the argument semantics by increasing the same effect value from zero toward
  `0xff` before the final opaque render. `HandleBoardPlayerTurnInput` at
  `0x42c1a` clears that selected source before interpreting the second click;
  clicking the source again skips the move and reaches the common opaque
  redraw.
- `ApplyBoardMoveAndUpdateState` at `0x428c9` removes the source from the live
  board and calls `AnimateBoardMove` at `0x42644` for both player and AI moves.
  That routine inserts the moving piece into the fixed depth traversal by
  comparing screen Y first and X second, then interpolates its anchor at 100 Hz.
  Adjacent paths take 100 ticks; paths whose cell delta is 2 or greater than 7
  take 200 ticks. Short and long moves start
  `CHESS3` and `CHESS2`, respectively; `CHESS1` accompanies captured-piece
  removal, `CHESS0` accompanies a piece transform, and `CHESS4` starts when the
  player begins choosing a marked destination and stops on cancel or confirm.
  Before `RunBoardAiTurn` at `0x43535`, `RunBoardGameScene` dispatches cursor
  row `0x13`, leaving the animated thinking cursor active through the AI move
  presentation.
- `TryGetLegalBoardMoveDestination` at `0x41c1e` selects movement offsets by
  absolute piece type. Type 1 moves orthogonally inside its two home rows;
  types 2 and 3 move to adjacent cells and can leave through a horizontal side
  edge only when they occupy one of their far three rows; type 2
  cannot capture types 4 or 5; type 4 makes capture-only two-cell leaps; and
  type 5 combines the adjacent and leap tables. `ApplyBoardMoveAndUpdateState`
  at `0x428c9` promotes a type-2 runner at the opposite edge or after it
  captures type 2 or 3. When a side has no type-2 or type-3 runners left, its
  type-4 pieces become type 5. Capturing type 1 ends the game with the moving
  side's signed result, and only positive result 1 sets the supplied milestone.
  The player may cancel a selected source with right click or a pending key;
  Escape and control `0x81` leave unsolved, F1 opens help table `0x1b4`, and
  the case-insensitive hidden keyword `aspirin` produces result 1. Negative
  turns are selected by `RunBoardAiTurn` at `0x43535`, whose recursive search
  depth follows the configured puzzle difficulty and randomizes equal scores.
- The ScummVM-only `PUZZLE_HELP` board overlay is isolated in
  `BoardGamePuzzle::DebugHelper`; it does not change the retail move model or
  AI turn. Once a player piece is selected, the helper uses `BOARDTML` regions
  to apply a sparse red dither to every legal destination beneath the normal
  piece layer and a denser yellow dither to the best destination. These use
  shared interface palette entries 254 and 255 instead of quantizing arbitrary
  colors through the limited `BOARDPAL` palette. The recommendation evaluates
  only that selected piece's legal moves with the existing minimax score at
  fixed maximum retail depth 3. Since the score favors the negative AI side,
  the positive player recommendation selects the lowest score and resolves
  ties by retaining the first legal destination, keeping the diagnostic stable
  between redraws.
- Scene action 22 calls `RunShockLeverPuzzleScene` at `0x3affb` with the
  caller-supplied completion flag. `EF2.RUN` callback `0x5d2` supplies
  milestone 208. The scene opens `EF_MONK.PL`, uses `SCREEN` and `DOWN` for
  the base presentation and submit control, and services the archived
  `SCREENN` Smacker at logical `(276, 3)`, physical `(276, 53)`.
  `ServiceShockPuzzleScreenPlayback` at `0x3ad26` posts that same X/Y dirty
  region. The scene loads seventeen frames for each lever from the nested
  `LEVER1N` through `LEVER3N` libraries. The levers begin at frame 3 at
  physical positions `(258, 146)`, `(305, 146)`, and `(342, 150)`.
  When the ScummVM-only `PUZZLE_HELP` console toggle is active, palette-254
  red labels `1`, `2`, and `3` appear beside the exact target positions for
  those respective levers. The diagnostic overlay shares the target-frame
  table used by validation and does not change the retail one-frame tolerance.
- Dragging a lever maps the pointer's vertical position into frame `0..16` and
  advances toward it one frame per three DOS timer ticks.
  `AnimateShockLeverToFrame` at `0x3ad98` restores a scoped backing for the
  lever's dirty rectangle before copying its next transparent frame. Submitting
  compares the three current frames against `[7, 14, 9]`, accepting the target
  or either adjacent frame. `M_SCREAM`, `M_NORMAL`, `M_1`, and `M_2` correspond
  to zero through three correct levers and are archived `IAVF2.00`
  presentations dispatched through `RunMediaPresentation` at `0x17917`. A
  three-lever match records success but leaves the puzzle active; cleanup sets
  the supplied milestone when the player exits. Escape or either side control
  exits, F1 opens help table `0x1af`, and the case-insensitive hidden keyword
  `sparky` presents `M_2`, sets success, and exits. `SHOCK0` through `SHOCK4`
  supply the outcome, ambient, submit, and lever-motion audio cues.
- Scene action 33 calls `RunStainedGlassPuzzleScene` at `0x2e366` with the
  caller-supplied completion flag. `KG.RUN` callback `0x263` supplies milestone
  217 after retaining `KG_GLASS.AVI`. The puzzle opens `STGLASS.PL`, presents
  the 640-by-400 `GLASS.PCX` background with its palette, and loads sixteen
  99-by-99 stained-glass tiles from `P01.BBM` through `P16.BBM`. `P17.BBM` is
  the blank tile backing restored under moving pieces, while `FRAME3.PCX` is
  attached to the retail transition update as a full-screen stencil. Its
  color-zero openings expose moving tiles; the remaining area preserves the
  existing `GLASS.PCX` backdrop and lattice instead of replacing it with the
  frame bitmap's palette indices.
- `RunStainedGlassPuzzleScene` reads `STGLASS1.INI` through `STGLASS3.INI`
  according to the configured puzzle level. Sixteen `Piece n` sections map
  destination slots to one-based source slots for each selectable control;
  the `Initial Positions` section supplies the starting permutation. The
  sixteen control origins at `0x843f0` are ordinary physical X/Y pairs. The
  routine cancels an active 50-pixel scene origin before registering them, so
  ScummVM does not add the toolbar band to their Y coordinates.
- Entry starts from solved order `1..16`, holds that preview for 125 timer ticks,
  then animates to the configured initial order. Each move advances all changed
  tiles to their destination over sixteen line steps paced five 100-Hz ticks
  apart. Space repeats the solved-to-initial preview; Alt+H temporarily shows
  solved order for 250 ticks and restores the saved order; F1 opens help table
  `0x1b1`; Escape exits; and the case-insensitive hidden keyword `headache`
  sets the supplied flag. A normal solve sets the flag only when every slot
  contains its matching one-based tile. `G-PUZZLE.WAV` loops under the scene,
  while `GLAS0.WAV` through `GLAS4.WAV` accompany transitions. Shared cleanup
  fades the palette and clears the display before `KG.RUN` resumes.
- Scene action 34 calls `RunKeypadSequencePuzzleScene` at `0x3bd30` with
  the caller-supplied completion flag. `JB2.RUN` callback `0x2da` supplies
  milestone 220 after presenting `JB_DOOR.SMK`. The initial `KPEXTRA2.BBM`
  code control opens `KPOPEN.AVI`; Escape, the left-edge control, or the code
  control then presents `KPCLOSE.AVI` and exits. F1 opens help table `0x1ac`.
  Retail clears off-screen logical page 1 before installing the controls, while
  the visible `JB_DOOR` frame remains behind the puzzle. Both keypad media
  presentations are 220-by-236 overlays at physical position `(100, 50)`.
- The twelve keypad controls use the scene-space Y/X positions at `0x84b8e`.
  `JB_KEY0.BBM` through `JB_KEY8.BBM` are digits 1 through 9,
  `JB_KEY9.BBM` is EX, `JB_KEY10.BBM` is zero, and `JB_KEY11.BBM` is RV.
  EX and RV both run the reject/clear sweep. The nine-digit solution at
  `0x84b7c` is `450144286`; matching it runs the success sweep and sets the
  supplied completion flag. `KPEXTRA0.BBM` through `KPEXTRA3.BBM` provide the
  entry-slot and blinking-prompt feedback, while `JB_LOC0.WAV` through
  `JB_LOC5.WAV` supply control and result cues.
- Scene action 61 calls `RunEightButtonSequencePuzzleScene` at `0x4043d`
  with the caller-supplied completion flag. `FA3.RUN` callback `0x402`
  supplies flag 31. The routine advances `XARMED.SMK` at physical position
  `(248, 254)`, installs eight `XKP0.BBM` through `XKP7.BBM` controls at the
  scene-space Y/X positions stored at `0x84e31`, and records six one-based
  button selections. F1 opens help table `0x1b2`, while the 100-by-300
  left-edge control or Escape exits.
- The fixed sequence at `0x84e25` is `2,2,6,4,8,1`. Each accepted press adds
  the corresponding positional `DKP1.BBM` through `DKP5.BBM` or `DKPO.BBM`
  marker using the Y/X table at `0x84e51`. A mismatch clears the captured
  marker row and plays `BNKWAV3.WAV`; a match plays `BNKWAV2.WAV`, draws
  `XDARK.BBM` at physical position `(276, 291)`, flashes the completed row
  five times at five DOS ticks per phase, and sets the supplied flag.
  `BNKWAV0.WAV` and `BNKWAV1.WAV` are the active/inactive control cues;
  retail loads `BNKWAV4.WAV` with the same descriptor set but does not queue
  it in this routine.
- Scene action 63 calls `AppendResourceStringToRipperTextFile` at `0x22392`.
  It loads up to `0x27c0` bytes of the current `ripper.txt` through
  `LoadResolvedRipperTextFileBuffer` at `0x22252`, appends two newlines, the
  one-based `GAMETEXT.TF` resource supplied by the script, and one trailing
  newline, then rewrites the file through `SaveAndFreeResolvedRipperTextFileBuffer`
  at `0x222fd`. The routine sets `g_ripperTextUpdatePending` at `0x84151`;
  `RunWacFrontEndLoop` at `0x21865` consumes it by queuing `WACNOTE.WAV` once.
  ScummVM keeps the writable notebook as target-scoped `<target>-ripper.txt`
  data in the save directory, falls back to the installed `ripper.txt` when
  importing an existing playthrough, and truncates the writable copy when a
  new game begins. This preserves retail's external notebook lifecycle without
  writing into the game installation.
- Scene action 19 calls `RunWebGridShiftPuzzleScene` at `0x2cdce` with the
  caller-supplied completion flag. `KF.RUN` callback `0x2b6` supplies flag 221
  after retaining the final `SPIDER.AVI` presentation. The routine captures
  that display into a 5-by-5 grid of 85-by-70 tiles: row Y coordinates
  `26, 96, 166, 236, 306` come from `0x843c2`, and column X coordinates
  `107, 192, 277, 362, 447` come from `0x843d6`. These are physical screen
  coordinates; retail cancels the 50-pixel presentation origin before
  initializing the controls.
- The web puzzle starts with the tracked tile at row 0, column 4 and performs
  `150 * puzzle level` random adjacent moves, rejecting immediate reversal
  directions. Selecting any other cell in the tracked tile's row or column
  rotates that inclusive segment by one cell and moves the tracked position to
  the selection. The identity tile order `0..24` is the solution. Escape exits,
  F1 opens help table `0x1ad`, Alt+H (DOS BIOS command `0x2300`) temporarily
  previews the solved grid, and the case-insensitive hidden keyword `zztop`
  completes the puzzle.
  `WEB0.WAV` accompanies autoplay while `WEB1.WAV` and `WEB2.WAV` provide
  randomized shift and preview feedback. Shared cleanup fades the 256-color
  palette to black in nine steps and clears logical page 0.
- Scene action 20 calls `RunHorusWordPuzzleScene` at `0x3f976` with the
  caller-supplied completion flag. `KQ.RUN` callback `0x2d9` supplies flag 228.
  The routine opens `EGYPT.PL`, presents the 640-by-400 `BKGRND` custom bitmap,
  and creates controls `0x672` through `0x68b` from the 26 physical X/Y pairs
  at `0x3e82f`. Although the retail path string is `BKGRND.PCX`, the archived
  member has the GCMS-extended custom bitmap header; `A` through `Z` use the PCX
  decoder. `BUT1` through `BUT26` are the selection overlays. Selecting a
  letter disables that control, so each five-letter attempt contains distinct
  characters.
- The five selected letters occupy the physical anchors (228,347), (264,347),
  (299,347), (334,347), and (369,347). `EGYPT.RTP` supplies the palette and
  12-bit color cube used to pulse each letter between effect phases 14 and 25;
  the strength passed to the display effect is the phase multiplied by 10.
  After five selections, the routine compares against the uppercase codes at
  `0x3e8ff`, spelling `HORUS`. A correct word sets the supplied flag after a
  36-DOS-tick settle interval. An incorrect word fades away and re-enables its
  letters; selecting a new letter before that timeout starts the next attempt
  immediately. Escape fades the active letters and exits without setting the
  flag, while F1 opens help table `0x1b3`. `AZTEC0`, `AZTEC2`, and `AZTEC3`
  provide the active-word, control, and reset audio cues.
- The ScummVM-only `PUZZLE_HELP` overlay is isolated in
  `HorusWordPuzzle::DebugHelper`. It reads the live attempt without modifying
  input, letter state, validation, or milestone completion. Unselected letters
  from `HORUS` receive sparse palette-254 guidance, while the next letter for
  the current slot receives the denser palette-255 recommendation. The helper
  recomputes after selections, rejected-letter release, and debugger toggle
  changes; a completed incorrect attempt recommends `H` as soon as that control
  is available so the retail immediate-retry path starts a fresh attempt.
- Scene action 24 calls `RunTubeSwitchScene` at `0x25e18` with the
  caller-supplied completion flag. `GA1.RUN` callback `0xaf2` supplies flag 215
  (`solved vacuum tube puzzle`). The scene counts consumed inventory flags 102,
  103, and 104, selects `GA_TUBE1.SMK` through `GA_TUBE4.SMK` from that count,
  and retains its presentation at logical (18, 52), physical (18, 102).
  `TUBESW.PL` supplies the seven 43-by-54 switch frames at physical
  (554, 223). Cursor 16 owns that switch, cursor 7 owns the exit region
  (115, 115)-(515, 325), and Escape exits without setting the completion flag.
- `RunTubeSwitchScene` plays frames 0 through 45 when the switch is turned on,
  then seeks the same active Smacker decoder back to frame 15 and repeats
  frames 15 through 45 while power remains on. Retaining that decoder preserves
  the delta-coded frame surface and palette across the loop. Turning it off
  stops `TUBE1.WAV` and plays frames 46 through 59. With all three tubes
  installed, reaching frame 46 presents `TUBE_WIN.AVI` and sets the supplied
  completion flag. The scene restricts `RunFrontEndActionMenu` to mask `0x84`,
  exposing only inventory and help. Inventory is built from unlocked flags 50
  through 69 whose corresponding consumed flags 100 through 119 are clear;
  only flags 52 through 54 have the required `GAZ2` use label here, setting
  flags 102 through 104 through the shared `Inventory` service.
  Each accepted tube presents `TUBEHAND.AVI`, then reloads the numbered Smacker
  for the new installed count. Other inventory items present modal resource
  `0x4d`; F1 uses resource `0x1bb` in the inventory chooser and resource 400 in
  the main scene.
- Scene action 28 calls `RunGymSelectorScene` at `0x3c64b` with the
  caller-supplied completion flag. `FD2.RUN` callback `0xb59` supplies milestone
  209 from frame `FDZ1`. The selector repeatedly presents `FD_GYM4.SMK` until
  it receives input, then uses Up (`0x4800`) and Down (`0x5000`) to wrap through
  states 0 through 3 and presents the matching `FD_GYM0.SMK` through
  `FD_GYM3.SMK`. Reaching state 2 sets the supplied flag; the selector remains
  active until Escape or until a state presentation finishes without input.
- `RunGymSelectorScene` builds its two cursor-16 controls from
  `FD_GYMB0.BBM` and `FD_GYMB1.BBM` at physical rectangles
  (374, 308)-(397, 322) and (373, 327)-(395, 340). `GYM0_W.WAV` and
  `GYM1_W.WAV` are the shared press/release cues. Each state transition starts
  `GYM2_W.WAV`; `PollGymSelectorMediaSequenceInputCallback` at `0x3c58f`
  stops it at one-based media frame `0x26`. The left-side
  (0, 50)-(166, 350) control and Escape leave the selector, while F1 opens help
  table `0x1a9`. Entry stores and dispatches cursor row 14, and cleanup stores
  row 0 after restoring the prior control list.
- `RunKaBookCodeEntryPrompt` at `0x2bc33` is a separate blocking puzzle called
  by `RunKaDialogueScene` after the book-response voice completes. It owns
  `KA_PUZ.PCX`, the seven input cells, `KA_KEY.WAV` feedback, F1 help table
  `0x1a6`, uppercase alphanumeric filtering, backspace, and the embedded answer
  `HC2021R`. A match plays `LI1_1_VE.WAV`; Escape or failure plays
  `LI1_1_VF.WAV`. The Ka scene consumes only that result: success presents
  `KA_BOOK.AVI` and sets flags `0xe1` and `0x53`, while failure clears choice
  flag `0x14c`.
- Scene action 8 passes its argument in EAX to
  `RunKdShootingGalleryScene` at `0x3288e`. The routine selects `KD1.INI`
  through `KD3.INI` from the combat-level setting, then owns the full-screen
  `KD.SMK` presentation, custom `KD_MOUSE.BBM` cursor, `KD.DAT` frame regions,
  `KD.PRJ` frame audio, `KD_FLAME.PL` result animation,
  `KD_CHECK.BBM` result marks, and `KD0.WAV` through `KD12.WAV`.
  The original reads the configured scene frame rate, while ScummVM delegates
  presentation timing to the Smacker decoder. It also reads the `penalty`
  score key but does not use that value in the final score.
  The shared DAT loader preserves each region as
  `type, coordinate1, coordinate2, extent1, extent2`. Both KD and combat
  consume those original vertical/horizontal fields as
  `type, y, x, height, width`.
- KD target-group changes reset the current hit count and assign a new serial
  to each nonzero group. A shot spends the configured weapon percentage,
  applies the configured rapid-fire delay, and increments raw, distinct-target,
  and completed-target counts for the hit region's class. A zero `shots1` or
  `shots2` value scores every raw hit; a nonzero value scores only groups that
  reach the required hit count. Class-1 points are subtracted from class-0
  points. The result screen draws the configured goal, final score, and target
  marks. `RunKdShootingGalleryScene` loads each mark's logical X coordinate into
  EDX, beginning at `0x6c` and advancing by the bitmap width plus one; ECX holds
  the fixed class row at `0x8f` or `0x7a`. The result flame animation uses the
  same display-update convention at logical `(0x6e, 0x84)`. The scene sets the
  caller's milestone only when the score reaches the goal. Escape exits without
  setting it; F1 opens help table `0x1a7`, and the hidden keyword `paradise`
  sets the milestone and exits immediately.
- `KD.PRJ` supplies the per-frame cue/volume pairs. The gallery separately
  fires `KD10.WAV` and `KD11.WAV` at frame 118, `KD7.WAV` at frame 995,
  `KD12.WAV` at frame 1005, stops cues 10, 7, and the looping `KD9.WAV`
  ambience at frame 1952, and starts `KD8.WAV` at frame 1965.
- Scene action 13 passes its flag argument to `RunBlobShooterScene` at
  `0x338a4`. The handler selects `KJ%d.INI` from the same combat-level byte as
  the combat controller, loops the full-screen `KJ.SMK` background, and owns
  `KJ_MOUSE.BBM`, five `KJ_BLOB%d.PL` animation libraries, `KJ_EXP1.PL`,
  `KJ_FIRE.PL`, and `KJ0.WAV` through `KJ8.WAV`. The available retail data set
  contains only `KJ1.INI`; ScummVM reports the requested numbered file and
  falls back to that available tuning file when necessary.
- `RunBlobShooterScene` enters `InitializeSceneDisplayModeAndContext` before
  presenting `KJ.SMK` and returns through `InitializeStartupDisplayModeAndContext`
  on every exit. The active script page therefore remains distinct from the
  full-screen shooter page. ScummVM snapshots and restores that indexed page so
  a following controlled presentation cannot preserve shooter pixels around a
  640x300 scene loop.
- The blob origin tables at `0x31383` and `0x31397` use the original
  vertical/horizontal selection-point order. Converted to logical `(x, y)`,
  the five origins are `(185, 0)`, `(0, 0)`, `(0, 62)`, `(0, 0)`, and
  `(210, 0)`. Each blob advances one library frame per presentation tick.
  Reaching the configured `start whoosh` frame starts its movement cue;
  reaching the end without a hit increments the miss count. A pointer hit
  checks the current frame's nontransparent pixel, removes that blob, and
  starts the fire and explosion bitmap groups at the hit point.
- The KJ win condition is a rate race rather than a point total. New blobs use
  the configured spawn delay; at the configured decrease interval that delay
  falls by `spawn dec`. Reaching `win rate` sets the caller's flag and begins
  a timed finale. The handler later enables scripted hits at the fixed
  per-type points from `0x313ab` and `0x313bf`, continues accelerating down to
  the two-tick minimum, stops spawning, waits for every active blob to clear,
  and exits. Reaching `misses allowed` first exits without setting the flag.
  Weapon charge, discharge, and rapid-fire timing also come from the INI.
  Escape exits, F1 opens help table `0x1a8`, F10 captures a screenshot, and
  the case-insensitive `caffeine` keyword sets the supplied flag immediately.
- Scene action 14 calls `RunEbz2sUnlockGatedActionMenuScene` at `0x36783`.
  `EB2.RUN` callback `0x58b` reaches it from the `EB_HEAD` take interaction.
  The scene loads `EBZ2S.PCX`, then conditionally grants inventory flag 58
  (Eyeball) or 55 (Bug) through action 3 before looping with front-end mask
  `0x84`, which exposes only Inventory and Help. The loop ends when consumed
  flag 108 or 105 is set. `ExecuteUnlockSelectionChoice` at `0x364be` accepts
  both items only in the `EB_HEAD` frame, presents `EBU2.PCX` for either, and
  additionally plays `EYE_BUG.WAV` for the Bug.
- Scene action 5 calls `RunRolodexSequencePuzzleScene` at `0x280ae` with the
  caller-supplied named flag. It creates an advance control over physical
  rectangle (121, 101)-(486, 293) with cursor 16 and an Escape control over
  (539, 72)-(640, 327) with cursor 7. `rolodex8.smk` initializes the scene;
  the first numbered sequence then starts automatically. Enter or the advance
  control steps through `rolodex1.smk` to `rolodex6.smk`, wrapping to sequence
  1 after 6. Sequences 1 through 5 loop from frame 20 and sequence 6 loops from
  frame 22. Reaching sequence 6 sets the supplied flag before playback starts.
  Escape stops the active mechanical audio, starts `rolo2.wav`, and presents
  `rolodex7.smk`; F1 opens help table `0x19f`.
- `ServiceRolodexMediaSequenceCallback` at `0x27e38` synchronizes the puzzle's
  four `rolo%d.wav` descriptors to media frames. The introduction starts
  `rolo2.wav` at frame 1 and the low-volume repeating `rolo0.wav` at frame 12.
  Each numbered sequence starts `rolo3.wav` at frame 1 and `rolo1.wav` at frame
  15, while sequence-specific frame counters 10, 13, 17, 16, 16, and 22 stop
  `rolo3.wav` for sequences 1 through 6 respectively.
- The calculator loads `CALC0.BBM` through `CALC24.BBM` for its controls and
  `CALCNM0.BBM` through `CALCNM11.BBM` for the right-aligned numeric display.
  The 25 records at `0x84170` store display-relative Y, X, and command values;
  `RunCalculatorPuzzleScene` adds the 50-pixel scene origin to Y. The controls
  expose on/off, digits, decimal point, the four binary operators, Enter,
  square, reciprocal, square root, inverse, sine, tangent, and cosine. F1 opens
  modal help table `0x1a0`, Escape exits, and the original lower control at
  physical rectangle (160, 240)-(490, 350) also dispatches Escape.
- `InitializeUiControlState` at `0x4b90a` captures the scene beneath each
  calculator button. `TriggerUiControlStateByControlId` at `0x4ce19` displays
  the corresponding `CALC*.BBM` feedback bitmap for three DOS ticks, then
  restores the captured scene through `DeactivateUiControlStateFeedback` at
  `0x4b43a`; the bitmaps are not the calculator's persistent button faces.
- `ApplyCalculatorOperator` at `0x265a5` evaluates pending binary operations and
  the scientific keys. `RedrawCalculatorInputDisplay` at `0x268e3` clears the
  153-by-15 display at physical position (220, 107), limits the displayed
  string to ten characters, and places glyphs from right to left with a
  three-pixel overlap. The power-off path clears that strip and leaves the
  button controls active.
- When flag 2 is set and flag 55 is clear, the calculator tracks the command
  sequence `4`, `6`, square, divide, `7`, inverse, Enter, `5`. A mismatch
  disables further matching until the calculator is powered on again. A full
  match sets the caller-supplied flag and exits the puzzle.
- The crystal puzzle loads sixteen-piece `CRYSP*`, `CRYST*`, and `CRYSB*` BBM
  sets and four `CRYSTAL%d.WAV` cues. It presents the pieces in the recovered
  tray coordinates beside an eight-by-nine grid. A selected tray or occupied
  grid piece follows the pointer until the player selects an empty grid cell or
  returns it to the tray. Escape exits without setting the completion flag;
  F1 invokes modal selection table `0x19e` through
  `RunModalSelectionTableDialogWithRestore`.
- `RunCrystalPiecePlacementPuzzleScene` stores UI rectangles in the original
  display's transposed axis order. The tray-piece table at `0x8429c` supplies
  display-relative Y values and the table at `0x842bc` supplies X values; the
  scene origin `g_8a1ec` is added to Y. Each piece control uses its `CRYSB`
  blanking bitmap as a pixel mask, selects cursor 16 on hover, and dispatches
  control IDs `0x6d6` through `0x6e5` into the drag overlay. Starting a drag
  blits `CRYSB` over the original tray position and presents the moving `CRYSP`
  piece; returning it to the tray restores `CRYST`. The grid and tray-return
  controls use the same 50-pixel scene origin, so rendering, selection, and
  dropping share one physical coordinate space in ScummVM.
- `RunModalSelectionTableDialogWithRestore` at `0x1f7f8` snapshots the active
  chooser state, resolves title resource `0x42` (`HELP`) and the caller's body
  resource, then calls `RunModalTextDialog` at `0x58ef2`. The shared dialog uses
  its caller-supplied chooser template and `SMALL.FNT`, centers the requested
  control, wraps at most ten visible 14-pixel rows, drains pending keyboard
  input, scrolls longer text with the navigation keys, and closes on Escape or
  Enter before restoring the underlying indexed framebuffer. The general help
  wrapper supplies the `MENUB0` through `MENUB14` template and a 300-pixel
  width.
  Retained panel measurement, wrapping, frame/scroll rendering, and WAC panel
  variants live in `modal/presentation.cpp`; `modal_dialog.cpp` owns resource
  initialization, text-entry state, and modal input loops. Both use the single
  retail layout constant set in `modal/constants.h`.
  RIPPER's bitmap descriptors and presentation coordinates use
  vertical/horizontal order. After translation to screen x/y,
  `ResolveChooserFrameTileIndex` at `0x55250` selects `MENUB0` through `MENUB8`
  as the row-major top, middle, and bottom frame tiles. For overflowing text,
  `TileChooserControlFrame` at `0x54fbe` draws `MENUB9` and `MENUB10` at the
  ends of a right-edge strip filled with `MENUB14`.
  `ComputeChooserControlLayout` at `0x54e9a` through `0x54f17` reserves that
  strip before wrapping: the 15-pixel `MENUB14` width plus its 4-pixel edge
  inset, less the template's existing 5-pixel right padding, narrows the
  overflowing text client by 14 pixels.
- `InitializeSharedPresentationTemplates` at `0x1196f` loads `SMALL.FNT` for
  the modal and captures MENUB palette indices 4 through 9 and 246 through 255
  with `CaptureSharedDisplayPalettePatch` at `0x205a9`.
  Other presentation paths restore those bands with
  `ApplySharedDisplayPalettePatch` at `0x205d0`; `RunModalTextDialog` itself
  does not change the active palette. The MENUB modal body uses background
  index 253 and glyph index 4; its heading uses index 255 and the centered
  `HELP` title uses index 254.
- `PollInteractionAndResolveSelection` at `0x13c8d` maps extended key `0x3b00`
  (F1) to general help resource 400, or resource `0x19b` while a prompt is
  active. While the Cyber transition flag is active it instead selects resource
  `0x1a4`, still using the `MENUB` template. That path makes no palette call
  around `RunModalTextDialog`, so the modal's indexed pixels retain the active
  Cyber scene palette. Toolbar action `0x51b` follows the same branch through
  `DispatchFrontEndAction` at `0x190b7`. `ProcessChooserControlInput` at
  `0x57372` maintains the first visible row and handles Home, Up, Page Up, End,
  Down, and Page Down. `InitializeSharedPresentationTemplates` at `0x119ac`
  assigns cursor 16 to the modal template. The input handler applies it only
  inside the modal bounds; outside, it services the retained scene selection
  state so the current hotspot cursor remains animated without dispatching the
  underlying action. The wrapped-text constructor creates no separate close
  or scroll controls, so the overflow strip is decorative rather than a mouse
  hotspot. ScummVM additionally maps mouse-wheel steps to the same one-row
  movement and services screen updates while the modal owns the game loop so
  the software cursor remains movable.
- `RunCrystalPiecePlacementPuzzleScene` reads the persistent puzzle level. Level
  1 starts piece 0 in cell 9 and piece 1 in cell 44, with an 18-DOS-tick pause
  between their placement; level 2 starts piece 0 in cell 21; level 3 starts
  with an empty grid. ScummVM reads the same `puzzle level` key from the
  `RIPPER.INI` `[game]` section and retains the original default of level 2.
- Completion requires exactly ten occupied cells at indices 2, 9, 21, 25, 32,
  37, 44, 48, 60, and 62. The original also accepts the case-insensitive hidden
  keyword `pisces`, resetting progress to zero after any mismatch. Either path
  sets the caller-supplied milestone, queues the solved audio cues, presents
  `CRYSHEAD.SMK` at physical position (328, 50), fades the active palette out
  through display command `0x1e`, and then plays `CRYSOLVE.AVI` before returning
  to the scene callback.

## WAC

- The reimplementation keeps the long-lived WAC front-end resources and
  persistent controls in `WacManager`, while each object-database invocation
  creates a `WacDatabaseSession` that owns the chooser rows, selection,
  shared media viewport, text-panel scrolling, and entry dispatch state.
  Dedicated WAC subscene classes own their presentation-specific state. This
  mirrors `RunWacFrontEndLoop` retaining the shared controls while
  `RunWacInventorySelectionLoop` builds and owns the database chooser.
- `DispatchFrontEndAction` at `0x190b7` routes toolbar action `0x517` to the
  modal `RunWacFrontEndLoop` at `0x21865`. The loop snapshots the active scene
  presentation and chooser state, sets the default cursor to 14, loads the
  640x400 `wac.pcx` background, and creates four bottom-row controls from
  `wac0.bbm` through `wac3.bbm`. Controls use cursor 16.
- `g_astWacFrontEndButtonLayouts` at `0x84156` places those controls at y=349
  and x=172, 252, 326, and 390. Their action IDs are `0x1900` (exit), `0x2000`
  (object database), `0x3100` (text viewer), and `0x3b00` (help). Escape also
  leaves the modal loop. The reimplementation restores the indexed scene and
  palette when the WAC loop exits.
- `RunWacFrontEndLoop` leaves all four bottom controls in the shared UI-control
  list while database and puzzle controls are added. Every WAC input owner calls
  `ServiceWacSceneInputAction` at `0x21eef`, so the bottom controls retain cursor
  16 hover and click behavior in subscenes. A `0x1900` power action propagates
  through the nested loop and closes the WAC, while Escape only returns from the
  current subscene; a nested `0x2000` database action is ignored while the
  database is already active. Help uses context resources 404 on the front page,
  405 in the mug puzzle, and 406 in the database.
- `ServiceWacSceneInputAction` at `0x21eef` presents those help resources by
  passing the tertiary chooser template at `0x8a3ec` to `RunModalTextDialog`.
  That template uses `WACMNU0` through `WACMNU15`, a 20-pixel heading, 6-pixel
  bottom inset, 5-pixel left inset, 20-pixel right inset, and 14-pixel rows.
  Its client background is palette index 4, the centered title is index 248,
  and normal body text is index 251. `DrawPromptChooserTemplateLabelCallback`
  at `0x16d0f` places the title one pixel below the control's top edge using
  template byte `+0x19`, rather than vertically centering `SMALL.FNT` within
  the full heading. It keeps the active WAC palette rather than applying the
  generic MENUB palette patch; `WAC.PCX` and the WACMNU assets share those
  indexed colors.
- The `0x3100` control calls `RunRipperTextFileViewer` at `0x223ef`. It loads at
  most `0x27c0` bytes from `ripper.txt` through
  `LoadResolvedRipperTextFileBuffer` at `0x22252` and presents the text with
  resource string `0x49` as the title. The active-viewer guard prevents the
  notebook control from recursively opening another viewer.
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
  reimplementation records those three columns and the recovered implemented
  handler mapping in `wac/database_catalog.cpp`; `WacDatabaseSession` owns
  filtering, presentation state, and handler execution rather than duplicating
  entry-ID tests throughout those responsibilities. The
  chooser control ID is `0x73a`; the call site supplies x=400, y=50, width=190,
  and height=282. `InitializeSharedPresentationTemplates` at `0x1196f` gives
  this chooser its own `wacmnu0` through `wacmnu15` skin and `small.fnt`.
  `TileChooserControlFrame` at `0x54fbe` uses the first nine bitmaps as a 3x3
  frame. The original presentation coordinates are transposed relative to
  screen x/y; after translation, `wacmnu0` through `wacmnu8` are the row-major
  top, middle, and bottom tiles. `ComputeChooserControlLayout` at `0x54a74`
  applies a 20-pixel heading inset, 6-pixel bottom inset, 5-pixel left inset,
  20-pixel right inset, and 14-pixel rows. The heading, normal rows, and active
  row use indexed glyph colors 248, 251, and 254 over chooser background 4.
  Its centered `Object Database` title uses the template's one-pixel top inset,
  matching the shared label callback.
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
  Entry 1 dispatches `RunWacMugSelectionScene` at `0x236b9` and entry 2
  dispatches `PlayMugSelectionCompletionMedia` at `0x2361c`. Completing the
  puzzle sets both database flags 71 and 72, then replaces the active entry-1
  row's label pointer with resource string `0xde` (Coffee Cup) and redraws that
  row without changing its dispatch ID. `BuildWacInventorySelectionMenu` at
  `0x22c91` independently includes every set flag on a later menu rebuild, so
  the two underlying database records remain distinct even though the row used
  to solve the puzzle is replaced in place for the current chooser session.
- The demo has no object-database chooser: `RunWacFrontEndLoop` at `0x1c085`
  routes action `0x2000` directly to `RunWacMugSelectionScene` at `0x1c58e`.
  Demo startup `LoadConfiguredAssetLibraryDirectories` at `0x10f20` opens the
  `[FILES] puzzle` library, stores it in the puzzle asset-library slot at
  `0x68ba4`, and clears the corresponding search path. The mug scene loads
  `MUG0.SMK` through `MUG8.SMK` from that archive; its solved presentation
  loads `MUG9.SMK` through the same slot. ScummVM therefore retains a separate
  puzzle-library source with loose configured-path fallback rather than
  treating those Smackers as ordinary filesystem WAC media.
- Entry 3 dispatches `RunWacJournalRevealScene` at `0x24261`, implemented by
  `WacJournalPuzzle`. It wraps game-text
  resource `0xaf` once into control `0x7b2`, marks reveal-source flag `0xfa`,
  and uses named flag `0xd8` as the decryption gate. While locked, text-entry
  control `0x7b3` prompts with resource `0xb5`; its expected answer is resource
  `0xb6 + puzzle level`, compared case-insensitively while ignoring
  non-alphanumeric bytes. The unlocked scene queues `wacjrnl.wav`, performs
  twenty random byte swaps in each unrevealed wrapped row, and restores rows
  at the eight 36-DOS-tick delays stored at `0x215b1`. Blank wrapped rows
  advance the current section. Source flags `0xfa + section` allow that
  section to advance and completion flags `0x104 + section` record revealed
  progress. Resource `0xae` supplies the label for the continuously redrawn
  percentage in the 330-by-222 journal panel's heading. The one-tick shuffle
  path calls `RenderChooserTextRow` for only the changed visible row; the
  percentage and complete panel are redrawn on the separate 36-tick reveal
  cadence. The
  `SetActiveTextDrawPosition` call at `0x24587` uses panel top plus 1 and panel
  left plus 50. The locked-scene call sequence at `0x24538` places the
  200-pixel password control at panel top plus panel height plus 5; its
  automatic one-row height includes the WAC heading and frame insets, so the
  prompt is above the editable row rather than inline with it. The database
  chooser, scrollbar, persistent WAC controls, and help context 407 remain
  active while this scene owns the left media viewport.
- Entry 0 dispatches `WACINV0.PCX` through
  `RunWacStillImageScreenWithOptionalAudio` at `0x22f1f`, implemented by
  `WacStillImageViewer`, with no audio. The
  Prescription Pad uses the shared WAC still-image viewport, palette patch,
  and scroll controls while the database chooser remains active.
- Entry 4 dispatches `WACINV4.PCX` through the same
  `RunWacStillImageScreenWithOptionalAudio` path with no audio. The Document
  retains the database chooser and shares the still-image palette and vertical
  scrolling behavior.
- Entries 7 and 16 dispatch `MAG_WAC.PCX`/`MAG_WAC.AVI` and
  `ED_WAC.PCX`/`ED_WAC.AVI` through
  `RunWacStillImageScreenWithOptionalPresentation` at `0x22a32`. Both always
  show their 300-by-200 still image. Flag 87 adds the 70-by-14 `WACVID1A` and
  `WACVID1B` presentation control immediately below it; activating the control
  temporarily selects retail video mode 1 and presents the paired 300-by-200
  AVI unscaled at the still image's centered, four-byte-aligned origin inside
  the 350-by-282 database viewport. It then restores the prior mode and WAC
  display and sets flag 213 for Magnotta or 212 for Eddie. This matches
  `ConfigureMediaPresentationDisplayModeCallback` at `0x16ae3`, whose mode-1
  path does not install the 2:1 small-video display descriptor.
- Entry 6 dispatches `RunWacVoiceLockPuzzleScene` at `0x24ba4`, implemented by
  `WacVoiceLockPuzzle`. Its input controller retains the retail event order,
  while `WacVoiceLockEditorState` owns mutable selection/playback state,
  `WacVoiceLockRenderer` owns waveform and transient-overlay presentation, and
  `voice_lock_model.cpp` owns PCM span math and table-`0x215d1` validation.
  The pure validation boundary is characterized independently of the WAC loop.
  It always
  presents `VOXLOK.WAV` in a 340-by-116 source panel at physical screen
  position 50,50, with the read-only `WACWAV0` Play control at 50,167.
  The resource-`0xb1` and resource-`0xb2` strings are centered control titles,
  not client-area text rows. `DrawAudioDescriptorWaveform` at `0x25b73`
  renders palette-255 samples directly in the unbordered black client area
  below each title. `SetChooserPresentationTemplateLayoutMetrics` at
  `0x542ba`, called for the tertiary WAC template at `0x11b38`, installs a
  20-pixel heading, 6-pixel bottom inset, 5-pixel left inset, and 20-pixel
  right inset. The resulting source waveform client is 315 by 90 at 55,70,
  not 330 by 90. The original chooser record orders its coordinate axes
  differently from `Common::Rect`: `DrawAudioDescriptorWaveform` consumes
  fields `+0x0a/+0x0e` as the horizontal origin and width. Preserving that
  distinction keeps the quantized PCM features aligned with the validator's
  fixed physical-screen coordinates.
  Milestone 84, the audio-editor software, enables a second panel at 50,176
  and moves Play beside the `WACWAV1` Clear and `WACWAV2` Quantize controls
  beginning at 50,293. Quantize plays `WACJRNL.WAV`, replaces
  the source descriptor with `VOXLOK1.WAV`, and enables the completion test.
  Game-text resource 407 describes two distinct source gestures: a horizontal
  drag highlights a PCM span, then a second drag carries that retained
  highlight from the source into the editor. The retail source-press path at
  `0x2505c` reads `GetBiosKeyboardShiftFlags` at `0x4d2ca` when adjusting the
  highlighted endpoints. It sets display write mode 3 at `0x25006` and fills
  the span with color `0xff`; the update path applies the same operation to the
  old span at `0x25265` before drawing the new span at `0x25290`, confirming an
  XOR highlight that leaves the waveform visible in inverse colors. The
  reimplementation normalizes that logical `0xff` selection against the
  retained WAC panel's palette-index-4 black client so the selected background
  is palette index 255 (yellow), while a second pass still restores the exact
  original indices. A plain
  press on the retained highlight captures the composed source rectangle with
  display operation `0x18` at `0x250de`, then creates an overlay through
  `InitializeTransientPresentationOverlay` at `0x29caf`; the loop updates it
  with `UpdateTransientPresentationOverlay` at `0x2a1ff`. Each update restores
  the overlay's saved backing before capturing the new destination and drawing
  the selected pixels there.
  Release destroys the overlay through `DestroyTransientPresentationOverlay`
  at `0x2a7fa`. The display operation at `0x25403` copies the selected
  source-screen rectangle to the next editor position without horizontally
  rescaling it; the separate `MemCopy` call at `0x254ae` appends the
  corresponding PCM bytes. Both occur only when the release point passes the
  editor-control bounds check at `0x25307`, and later selections are
  concatenated at that original source scale. Clicking either waveform makes
  that control active.
  `GetActiveChooserControlId` at `0x52d94` selects whether Play uses the source
  descriptor or assembled editor descriptor, and a retained source highlight
  limits source playback to that span. The five pairs at table `0x215d1` are
  240..252, 70..82, 87..99, 171..184, and 190..199; order is irrelevant, but
  both ends must be within three pixels of one pair. The nested comparison at
  `0x25972` does not mark a pair as consumed, so each of the five submitted
  spans independently matches the same five-pair table. Exactly five quantized
  spans solve only from scene label `eez1`, after which `ACCESED.AVI` plays,
  milestone 29 opens the Secret Animal Lab, and WAC exits.
  `RunWacVoiceLockPuzzleScene` dispatches display command `0x14` to
  `ClearGenericVideoLogicalPage` at `0x45ed8` before that completion movie,
  then dispatches command `0x1e` to `FadePaletteOut` at `0x479ef` afterward.
  `RunMediaPresentation` at `0x17917` likewise treats the movie palette as
  presentation-local before it restores the surrounding display state. The
  reimplementation clears the full retained 640-by-400 page before the
  640-by-300 movie, so its uncovered bands remain black, then restores that
  cleared page and its indexed palette before the outer WAC snapshot returns
  to the scene. The temporary movie therefore cannot overwrite the remembered
  scene palette used by later toolbar-band redraws.
  The ScummVM-only debugger command `PUZZLE_HELP` toggles diagnostic puzzle
  overlays without changing retail validation. In the voice-lock source
  waveform it draws palette-254 red guides at those five targets, numbered in
  table order, only after Quantize has replaced the source descriptor with
  `VOXLOK1.WAV`. Each guide thickens the accepted `start ± 3` and `end ± 3`
  endpoint bands while taller ticks retain the nominal table pair; alternating
  baselines keep neighboring tolerance bands distinct. This keeps the fixed
  answer coordinates from being presented against the unquantized samples and
  exposes the exact retail grace instead of implying pixel-perfect input.
  `PUZZLE_HELP ON` and `PUZZLE_HELP OFF` select the state explicitly; the
  no-argument form toggles it.
  While Play is active, `GetManagedAudioTriggerPlaybackPosition` at `0x60795`
  advances a vertical marker across the source or assembled waveform selected
  for playback. The scene's input tick publishes one mouse state,
  services the persistent WAC controls and database chooser, then presents one
  cursor after also testing the waveform and audio-editor buttons.
- Entry 5, the second database row labeled Electronic Journal, dispatches
  game-text resource `0xb3` through `RunCenteredTextPanelUntilExitAction` at
  `0x2330c`. It uses the same retained 330-by-222 WAC text panel as entry 15.
  When the viewer returns, `RunWacInventorySelectionLoop` at `0x2252a` sets
  journal read flag `0x10a` before processing another database selection. This
  is separate from entry 3's `RunWacJournalRevealScene` password puzzle even
  though `GAMETEXT.TF` gives both rows the Electronic Journal label.
- Entry 9 dispatches the silent `wacinv9.smk` member from `INTERFAC.PL`
  through `RunStaticMediaScreenWithOptionalVoiceover` at `0x2339d`. The
  152-by-140, 15-frame Scanned Knife sequence is centered at screen (149,121)
  in the 350-by-282 media viewport and loops from frame one while the WAC
  controls remain active. It writes no milestone or completion state.
- Entries 10 and 11 dispatch `wacinv10.pcx` and `wacinv11.pcx` through
  `RunWacStillImageScreenWithOptionalAudio` at `0x22f1f`. These 300-by-393
  documents retain the database chooser, show a 282-row slice in the left WAC
  media viewport, and use `MNARROW0` through `MNARROW3` controls at screen
  positions 355,60 and 355,90 to scroll vertically in 10-pixel steps.
- Entry 8 tests shared flag 8 and dispatches `wacinv8a.pcx` when it is set or
  `wacinv8b.pcx` when it is clear. Entries 12 and 24 dispatch
  `wacinv12.pcx` and `wacinv24.pcx`. All three cases use
  `RunWacStillImageScreenWithOptionalAudio` at `0x22f1f` with no audio and
  retain the database chooser and shared scroll controls. Scrollable PCX
  entries initially show their final 282 rows; the reimplementation translates
  that retail bitmap-presentation origin to an offset in its top-down decoded
  surface.
- Entries 13 and 14 dispatch the silent `wacinv13.smk` and `wacinv14.smk`
  members from `INTERFAC.PL`
  through `RunStaticMediaScreenWithOptionalVoiceover` at `0x2339d`. The
  320-by-200, 40-frame Ripper Book and Audio Editing Software sequences are
  centered at screen (65,91) in the 350-by-282 media viewport and loop from
  frame one until WAC input dismisses or replaces them. Their presentations
  patch only palette entries 10 through 149. Escape returns to the database,
  power/F10 exits WAC, and selecting a different visible database row stops
  the sequence and immediately dispatches that row; neither entry writes
  milestone or completion state.
- Entries 17, 19 through 23, and 26 use that same silent centered Smacker
  viewer for their matching `wacinvNN.smk` members from `INTERFAC.PL`.
  Entries 18 and 27 through 29 have no retail switch cases; selecting them
  clears the shared media viewport and returns to the retained chooser without
  presenting content or changing state.
- Entry 25 is the one voiced media exception. `RunWacInventorySelectionLoop`
  combines `rip_game.smk` and `rip_game.wav` with the configured scene path at
  `0x22971..0x229a5`, then calls `RunStaticMediaScreenWithOptionalVoiceover`
  with optional scene-library handle 5. The installed layout keeps both files
  loose under `PATHS/scene`. The viewer presents the first video frame before
  starting the WAV, loops the Smacker from frame one, and returns automatically
  when managed narration ends. The entry then unconditionally sets shared flag
  `0x14f` (335), matching `0x229b9..0x229c8`. WAC input can still dismiss or
  replace the entry; either path stops the narration before the next database
  action.
- Entry 15 dispatches game-text resource `0xb6` through
  `RunCenteredTextPanelUntilExitAction` at `0x2330c`. The untitled, wrapped
  330-by-222 panel begins at the WAC media origin (50,50) and uses the same
  tertiary WACMNU chooser template and `SMALL.FNT` line-wrapping path as WAC
  modal help. `InitializeSharedPresentationTemplates` at `0x1196f` gives that
  template one/twenty-pixel base/alternate top padding,
  five/six/twenty-pixel left/bottom/right padding, a two-pixel horizontal
  glyph inset, and 14-pixel rows. The template's `layout56` bit 0 makes
  `ComputeChooserControlLayout` at `0x54a74` use the 20-pixel alternate top
  padding even though circuit-manual resource `0xb6` embeds its heading in the
  body and `RunWacInventorySelectionLoop` at `0x2252a` supplies no separate
  title resource.
  `HandleChooserPresentationFrame` at `0x552b1` binds `WACMNU9..12` as the
  right-edge up/down controls, `WACMNU13` as the proportional range thumb, and
  `WACMNU14` as its track. The arrow controls move the first visible line and
  the range thumb is pointer-draggable. The template callback
  `AnimateWacMenuChooserBlinkCallback` at `0x215e5` alternates the top-left
  `WACMNU0`/`WACMNU15` frame every five extended DOS ticks. The panel retains
  the database chooser and persistent WAC controls while
  `ServiceWacSceneInputAction` services chooser navigation and range scrolling;
  the circuit-manual text does not change milestone state.
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
  `engines/ripper/wac/`, beside the database session that directly dispatches it.
- `RunWacInventorySelectionLoop` keeps the database chooser alive while it
  dispatches `RunWacMugSelectionScene`. The mug scene redraws only the left
  media viewport, so the Object Database panel and its selected Broken Mug row
  remain visible and continue to frame the puzzle on the right.
- After loading the nine controls, `RunWacMugSelectionScene` draws them back to
  front, presents the completed WAC page, and only then calls
  `PlayBlockingAudioWithOptionalText` at `0x206e0` for `q_p_1.wav`. The WAV is resolved
  through startup sound-library handle 2 (`sound.pl`), not only as a loose
  file. The call result is not a puzzle-load guard; a missing voice clip does
  not discard the already-presented controls or leave the scene.
- `PlayBlockingAudioWithOptionalText` selects cursor index `0x13`, activates its
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
  interaction, and callback tables. A nonempty entry name is matched
  case-insensitively by `FindSceneFrameIndexByLabel` at `0x1464d`; an
  unmatched name resolves to frame zero rather than failing the transition.
  `HA2.RUN` depends on that fallback when its `hcw1in` handoff enters
  `HC2.RUN` at first-frame label `HCW1CIN`.
- `SceneRuntimeState` owns the active and concurrent compiled scripts together
  with their frame, interaction, pending-transition, and Cyber control state.
  Entering a nested Cyber program moves this object into a snapshot and starts
  with a fresh runtime, then restores the same object on exit. Cyber performs
  that lifecycle through `ScriptManager` operations rather than accessing the
  interpreter's private state.
- `ReadSceneCallbackOpcodeAndArguments` at `0x140e9` decodes callback commands
  and their typed arguments.
- `RunSceneCallbackCommandStream` at `0x14080` dispatches decoded commands.
  `ScriptManager` retains that callback interpreter, while
  `SceneActionDispatcher` owns opcode `0x18`'s action routing to match the
  separate `DispatchSceneEntryAction` boundary at `0x36892`.
- Scene action 9999 makes `DispatchSceneEntryAction` return control code `-4`.
  `RunSceneScriptLoop` at `0x124e9` treats that code as a normal runtime exit
  whether it comes from an interaction or an automatic frame callback. A
  top-level concurrent runtime returning `-4` is destroyed and the active
  runtime immediately resumes; `ACT3_CHK.RUN` uses this path after setting flag
  336 and presenting `BC3_1_P3.AVI`. Cyber programs use both callback forms:
  action 9999 still returns `-4`, while the dispatcher additionally requests
  their nested snapshot restoration instead of reporting the terminating
  callback as a script failure. A non-Cyber active runtime propagates the same
  clean exit to the outer front end; ScummVM maps that handoff to a return to
  its launcher.
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
- Opcode `0x19` layout variant 2 reaches `ConfigureSceneEntryChooserLayout` at
  `0x18740`, which selects the unskinned primary template at `0x8a2de` and
  fixes the control at logical `(228, 262)` with size `194x20`. Scene playback
  contributes the 50-pixel presentation origin, placing it at screen y=312.
  The zero stored in the layout record at `+0x14` is the chooser flags word,
  not a background palette index. `BuildChooserControlVisuals` at `0x54413`
  snapshots the movie pixels beneath the unskinned control, and
  `RedrawTextEntryChooserTextSpan` at `0x5aecb` restores that backing before
  drawing text and caret updates. `ReleaseChooserControlVisualState` at
  `0x5489f` restores it again when the control is destroyed. Text and the caret
  use the normal `small.fnt` palette index `0xfb`; the generic MENUB frame and
  palette are not applied to this control.
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
- `ExecutePresentationEntry` at `0x1754b` distinguishes the controlled response
  cleanup by filename extension. An `.AVI` enters `RunMediaPresentation` at
  `0x17917`, which restores display state, submits a full-display dirty-region
  update, reapplies the display palette, and restores selection presentation
  state. The ScummVM response path rebuilds the top and bottom scene bands only
  after that controlled AVI route, including the exhausted-dialogue path where
  no new chooser is installed. Other video extensions enter `RunMediaSequence`
  at `0x1e516` and retain their final frame. That sequence player clamps a
  full-size 640-by-400 movie requested at scene origin y=50 back to y=0; type-0
  document presentations such as `KK_Z12.SMK` therefore remain visible across
  the complete display while their unzoom hotspot is active.
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
- Opcode `0x1a` is handled by the retail script command executor
  `FUN_0001357f` at `0x1357f`. `ExecutePresentationEntry` at `0x1754b` first
  dispatches `.WAV` entries to `PlayBlockingAudioWithOptionalText` at `0x206e0`.
  This path selects cursor 19, keeps the
  cursor presentation active, and waits for audio completion or Escape before
  the opcode marks the media basename played. It covers scene-script speech
  entries such as `DB1.RUN` presenting `Q_OD_36.WAV`; WAV data does not enter
  the video decoder. `ExecutePresentationEntry` routes IAVF media through
  `RunMediaPresentation` at `0x17917`. It first deactivates the UI selection
  presentation, removing the active cursor before playback; the next frame's
  interaction presentation makes the cursor visible again. When keyboard
  controls are enabled, the media wrapper preserves the current logical display
  page before packetized playback, then fades out, redraws the saved page through
  `DispatchDisplayDirtyRegionUpdate` at `0x4e4b0`, and fades its palette back in
  before returning. With keyboard controls disabled, the zero branch retains
  the media's final rendered page. The reimplementation therefore snapshots and
  restores the indexed framebuffer and palette only for the controlled branch.
  This is required both
  when a following opcode `0x1b` preview draws only the 300-pixel scene area and
  when the callback returns directly to a type-2 frame such as `BAZ2`, which has
  no media to redraw itself.
  Direct Smacker media remains in the scene display context initialized by
  `InitializeSceneDisplayModeAndContext` at `0x1e28a`. `RunMediaSequence` at
  `0x1e516` interprets its coordinates against that logical 640x300 descriptor,
  so y=0 maps to physical y=50 between the toolbar and lower presentation
  bands. ScummVM applies that retained-framebuffer origin to opcode `0x1a`
  Smackers; otherwise a transition such as `DAW25.SMK` leaves scene pixels in
  the toolbar band before the following interactive frame is drawn.
  The handler then removes the media extension and sets that basename in the
  shared played-entry catalog.
  Opcode `0x09`, handled by `HandleSceneEntryPromptScenePlayedCondition` at
  `0x1488f`, can therefore skip a completed lead-in such as `VM0_1_P2` when a
  dialogue response returns to the same frame, while the response-frame flags
  continue to control which opcode `0x16` choices remain available.
- Opcode `0x1c` maps to `HandleSceneEntryStepPromptTransition` at `0x15c30`.
  Its first argument selects the palette direction and its signed second
  argument supplies the transition step count, defaulting to nine when that
  value is not positive. A nonzero direction dispatches `FadePaletteIn` at
  `0x47b06` before the optional script-debug `Step` prompt; zero shows that
  prompt first and then dispatches `FadePaletteOut` at `0x47a16`. Both paths
  cover palette indices 0 through 255 and synchronize each interpolation step
  to VGA retrace. ScummVM applies the same full-palette interpolation through
  `PaletteManager` with one 60 Hz interval between intermediate steps.
- `ExecuteSceneFrameAndInteractions` at `0x13277` runs frame callbacks,
  presentations, chooser selection, and interaction callbacks.
- `ExecuteSceneFrameAndInteractions` constructs the frame's hotspot controls
  before running its entry and persistent callbacks. Type-0 controls remain
  inactive while `ExecutePresentationEntry` plays their controlled media, then
  become visible and enter the polling loop only after that presentation
  returns; the demo implementation follows this ordering at `0x11be0`. An
  opcode `0x17` chooser is added to the same live control registry rather than
  replacing those hotspots. `PollInteractionAndResolveSelection` services both
  registries;
  when a scene hotspot wins, `CleanupCurrentSceneFrameInteractions` at
  `0x13832` dispatches phase 3 to the active chooser handler before running
  the hotspot callback. Phase 3 destroys the chooser, and
  `ReleaseChooserControlVisualState` at `0x5489f` restores its dirty-region
  snapshot before navigation media begins. Dialogue choices therefore remain
  visible while the player can use the current frame's navigation and action
  targets, but they are removed before the selected target's transition.
  ScummVM refreshes the chooser backing after each decoded dialogue-loop frame,
  then restores that backing and clears the uncovered top and bottom bands at
  the same teardown boundary.
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
- Opcode `0x1e` reaches `HandleSceneEntrySetOrClearNamedFlag` at `0x15dfe`.
  Its first argument selects clear (zero) or set (nonzero), while its second
  names an entry in the same persistent play-list bitset queried by opcode
  `0x09`. `SetStartupAssetCatalogEntryFlag` at `0x20125` resolves the name and
  `UpdateIndexedBitFlag` at `0x20e4e` changes the bit. ScummVM mirrors this in
  its serialized played-scene list; for example, `HA1.RUN` offset `0xa0e`
  clears `T1_1_B` before continuing the `T1_1_XB` callback.
- Opcode `0x1f` loads a named WAV into the first unoccupied entry in the
  original's 20-slot trigger table through
  `HandleSceneEntryLoadResourceIntoFirstFreeSlot` at `0x15e48`. Argument bit 0
  becomes the slot's scene-preserve flag. `DA1.RUN` depends on the table rather
  than a single current resource: its entry callback loads eight WAVs before
  configuring `POLICE1` and `ELDOR_O`, and later scenes configure preserved
  door sounds without loading them again. `SceneAudioManager` owns this table,
  its mixer handles, trigger/ramp service, and serialization. `MediaPlayer`
  owns only transient clips and video presentation, and calls the engine-owned
  scene-audio service when a presented frame reaches the retail service point.
- Opcode `0x20` scans all occupied slots by case-insensitive basename through
  `HandleSceneEntryConfigureOrStartNamedAudioTrigger` at `0x15eea`. A missing
  name is a no-op rather than a script error. A zero trigger starts immediately;
  otherwise `ServiceSceneFrameAudioAndBriefingTriggers` at `0x138c9` starts it
  when the one-based `RunMediaSequence` frame counter reaches that value.
  Control bit 0 makes `StartAudioTriggerSlot` at `0x37297` set the descriptor
  repeat field to -1, which ScummVM maps to an infinite looping stream.
- Opcodes `0x21` and `0x22` respectively clear a named slot through
  `ClearAudioTriggerSlot` at `0x37382` or stop its live handle while retaining
  the resource through `StopAudioTriggerSlot` at `0x37407`. Opcode `0x23`
  applies an immediate volume when its start frame is zero or schedules the
  same frame-driven integer ramp serviced at `0x138c9`.
- `RunSceneScriptLoop` at `0x124e9` clears occupied slots without the preserve
  bit when an active runtime changes scenes. A concurrent-runtime handoff and
  the scene-selection menu first clear every preserve bit, so those transitions
  retire the entire table. Each active-frame `-2` yield resets pending trigger
  frame and volume-ramp scheduling without discarding the loaded resources.
  Choice-list activation does not stop or replace named triggers, so the
  one-shot `R_P_L1.WAV` cue continues beneath dialogue selection until it ends
  naturally or a later explicit stop or clear occurs.
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
  `ServiceSceneFrameAudioAndBriefingTriggers` at `0x138c9` services that state
  both in the scene chooser and as a presentation-frame callback. The armed
  control remains visible and animated across automatic media and interactive
  frames until its `0x4e1` selection is dispatched.
- Scene interaction records contain an 11-byte label followed by an `x/y/width/
  height` rectangle at offsets `+0x0b/+0x0d/+0x0f/+0x11`. The scene UI uses
  transposed selection axes relative to the displayed frame, so ScummVM maps
  this to screen `x=y`, `y=x+50`, `width=height`, `height=width`.
  `ExecuteSceneFrameAndInteractions` adds the active presentation origin at
  `0x8a1ec` to record offset `+0x0b` before calling
  `InitializeUiControlState` at `0x4b90a`;
  `InitializeStartupDisplayModeAndContext` at `0x1e335` sets that origin to
  `0x32`. `IsPointInsideUiRect` at `0x533b1` then applies inclusive
  `origin..origin+extent-1` bounds, equivalent to `Common::Rect::contains`.
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
  delegate these streams to `Video::SmackerDecoder`. Presentation entry points
  describe palette, placement, timeline, looping, callback, and segment policy
  through a `SmackerPlaybackPlan` before entering the shared decoder loop. The
  plan groups placement, input, timeline, palette, frame, loop, callback, and
  rendering policy so a call site exposes which retail behavior it overrides.
- Files such as `PROINT.AVI` and `PROLOG1.AVI` are `IAVF2.00` packetized media,
  not RIFF AVI. `RunPacketizedMediaPlaybackCore` at `0x5b592` demultiplexes
  descriptor records containing mono signed 16-bit PCM and segmented Smacker
  setup/frame payloads. The engine's `iavf` module owns descriptor parsing and
  Smacker stream reconstruction; `MediaPlayer` owns presentation and timing.
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
- Retail `LoadSmackerPlaybackState` at `0x4f140` and
  `AdvanceSmackerPlaybackFrame` at `0x4ffe8` consume Smacker frame-table DWORDs
  directly as byte counts. The extracted `EFW4.SMK` differs from that contract
  at zero-based frame 62: its `0x2000a070` entry overruns the file, while
  `0x8070` makes all 110 declared payloads end exactly at EOF and is consistent
  with the adjacent frame sizes. The engine repairs only that complete
  filename, frame-count, file-size, frame-index, and damaged-value signature
  before handing playback to the shared decoder.
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
- Opcode `0x70` ends packet dispatch but does not immediately end the retail
  presentation. `RunPacketizedMediaPlaybackCore` at `0x5b592` continues polling
  `GetManagedAudioTriggerActiveDescriptor` until the final managed-audio
  descriptor completes before it destroys the packet stream. The
  reimplementation likewise retains the final decoded frame until the IAVF PCM
  timeline ends; this matters for `KA_BOOK.AVI`, whose audio continues after
  its last custom-video frame.
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
- `RunMediaPresentation` at `0x17917` installs
  `ConfigureMediaPresentationDisplayModeCallback` at `0x16ae3` as the
  packetized branch callback. `PreparePacketizedMediaPlaybackBranchSetup` at
  `0x5b237` invokes it again for each embedded branch, so the scale decision is
  based on that branch's extents rather than only the IAVF header canvas. In
  the original 640x400 mode, a branch smaller than 321x201 receives the 2:1
  display descriptor and its effective scaled extents are centered. This also
  expands `PROLOG2.AVI`'s 320x200 Smacker branches even though its IAVF canvas
  declares 640x400.
- Retail's text setting at `0x9a58f` controls authored auxiliary strings, not a
  generated transcript track. `ExecutePresentationEntry` at `0x1754b` copies a
  nonempty opcode `0x1a` auxiliary string into presentation state and marks the
  text callback active. The compiled script stores that string in the inline
  type-7 argument payload; entries without authored text use a length-one NUL
  payload, which fails the retail nonempty-string gate and must not create a
  text control. WAV entries then use
  `PlayBlockingAudioWithOptionalText` at `0x206e0`; IAVF entries install
  `ServiceMediaPresentationTextControl` at `0x17014`. Direct Smacker entries do
  not consume the auxiliary string. ScummVM exposes this through the
  engine-specific `ripper_subtitles` setting because the authored presentation
  text is separate from the launcher's conventional speech/subtitle mode. The
  `ripper_subtitle_autoscroll` option maps the retail auto-scroll setting at
  `0x9a593` and is grouped under the subtitle checkbox, so the launcher disables
  it while subtitles are off. All three retail engine checkboxes share the
  established skip-intro GUI flag so targets stored before the subtitle options
  were added enumerate the complete option set without requiring re-detection.
- `ResolvePresentationControlLayout` at `0x19a3d` selector `0x13` creates a
  centered 400-pixel wrapped chooser at active-display top plus 300. On a later
  packetized branch, `MoveMediaPresentationTextControl` at `0x17454` moves the
  chooser to active-display top plus 260 for an unscaled small branch, or to
  y=400 for a large or scaled branch. The first branch calculates its automatic
  scroll interval from the branch frame count and remaining wrapped lines;
  subsequent callback services advance one line at each interval when retail's
  auto-scroll setting at `0x9a593` is enabled.
- `ConfigureMediaPresentationDisplayModeCallback` forces video mode zero to the
  unscaled mode while authored text is active. The text callback services the
  shared chooser during playback; Escape stops it. At media completion,
  `ServiceMediaPresentationTextOverlayCallback` at `0x16ca6` redraws remaining
  text, appends startup resource `0x48` immediately to the chooser's right, and
  waits for Escape or that control before destroying the chooser. ScummVM maps
  this flow onto its retained wrapped-text renderer and shared palette patch,
  preserving the packetized movie palette and the retail panel geometry.
- The same callback switches branches at least 321 pixels wide or 201 pixels
  high from the scene display context to the full display context. A branch
  whose scaled output already fills the 640x400 display therefore does not
  inherit the scene viewport's 50-pixel physical Y origin. ScummVM applies
  that origin only when the resulting rectangle remains inside the active
  display and rejects any Smacker rectangle that still exceeds its bounds.
- IAVF opcode `0x68` is a display boundary. `RunPacketizedMediaPlaybackCore`
  dispatches palette service `0x1d` and then display service `0x14`, whose
  target is `ClearGenericVideoLogicalPage` at `0x45ed8`. `PROINT.AVI` places
  this command before each of its 16 differently sized Smacker segments, so
  pixels from a larger segment must be cleared before the next segment's
  palette is applied.
- Opcode `0x1a` enables the presentation callback when its third argument is
  zero. In that mode only keyboard Escape stops the whole presentation and
  Space pauses or resumes video and audio; mouse buttons do not skip it.
- As an explicit ScummVM extension, Right Arrow advances an IAVF presentation
  to its next embedded Smacker segment. The engine restarts the shared PCM
  stream at that segment's first absolute audio offset and rebases its frame
  gates to the new mixer clock. Right Arrow on the final or only segment exits
  successfully so the surrounding script performs its normal played-state
  update. This key is not part of the original callback:
  `PollPresentationEscOrSpaceCommand` at `0x49039` recognizes only Escape and
  Space, and the packetized-media path has no `0x4d00` comparison.
- `ExecuteSceneFrameAndInteractions` at `0x13277` marks a type-0 frame label
  played before calling `ExecutePresentationEntry` at `0x1754b` with keyboard
  controls enabled. That path installs `PollPresentationEscOrSpaceCommand` at
  `0x49039`: Space pauses or resumes the response video and its IAVF audio,
  while Escape advances to the normal post-presentation callback with the
  response still marked played. Type-1 interactive loops retain their separate
  mouse and hotspot path and do not install these keyboard controls.
- For every keyboard-controlled Smacker presentation, including direct opcode
  `0x1a` media and Smacker branches embedded in IAVF, advancing with Escape
  presents the terminal decoded frame and reports normal completion before
  returning to the enclosing script callback. This preserves the completed
  transition image and allows played-state updates, milestone changes, and
  later callback commands to run in their normal order. It follows the
  control-flow contract established by `PollPresentationEscOrSpaceCommand` at
  `0x49039` and `ExecutePresentationEntry` at `0x1754b`.
- Opcode `0x1b` enters `HandleSceneEntryMediaPreviewOrPrompt` at `0x15b03`.
  That handler passes a target value of one and
  `MediaSequenceCounterEqualsTarget` at `0x15ac8` to `RunMediaSequence` at
  `0x1e516`. The callback is evaluated after each presented frame, so the
  command decodes and retains the first Smacker frame before returning to the
  scene chooser. The later BA0 frame presentation plays the full sequence.
- Engine-local media code should remain an adapter or demultiplexer wherever
  packet payloads can be handed to existing ScummVM codecs.
- Media implementation units live under `engines/ripper/media/`: `video.cpp`
  owns Smacker and IAVF presentation timing, `audio.cpp` owns transient audio
  playback, `display.cpp` owns palette fades and static PCX presentation, and
  `source.cpp` owns stream resolution and validation. Long-lived named scene
  audio remains in the engine-level `SceneAudioManager`.
- Every Smacker entry point emits one stable level-2 playback-plan trace before
  loading the decoder. The trace names the retail route and records placement,
  palette, frame-range, loop, input, callback, and timeline policy so structural
  refactors can be compared without relying on C++ call layout.
- Media source resolution uses explicit caller policies for direct files,
  configured resource paths, `INTERFAC.PL`, ordinary `SOUND.PL` effects, and
  blocking-audio fallback. Those policies share opening and ownership mechanics
  without merging their different retail search orders. All Smacker entry
  points pass through one format-validation gateway before `RunMediaSequence`
  presentation begins.
