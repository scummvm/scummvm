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
  those bands while `LoadStartupBitmapAssetTable` prepares the startup assets;
  the current slice captures them from the first decoded presentation until
  that startup bitmap table is implemented.

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
- Action dispatch at `0x190b7` maps the nine controls to remote setup, world
  map, inventory, WAC, save, restore, options, help, and quit. The ScummVM
  toolbar currently preserves the click/release routing and logs these
  handlers as explicit stubs until their subsystems are implemented.

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
- Cursor hotspots are centered except for the explicit startup-table cases:
  cursors 14 and 16 use the top-left pixel, while cursor 15 uses the top edge
  at half its width.

## Scripts And Scenes

- `CreateSceneRuntime` at `0x12be7` loads a compiled script and binds its frame,
  interaction, and callback tables.
- `ReadSceneCallbackOpcodeAndArguments` at `0x140e9` decodes callback commands
  and their typed arguments.
- `RunSceneCallbackCommandStream` at `0x14080` dispatches decoded commands.
- `ExecuteSceneFrameAndInteractions` at `0x13277` runs frame callbacks,
  presentations, chooser selection, and interaction callbacks.
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
- Opcode `0x1b` enters `HandleSceneEntryMediaPreviewOrPrompt` at `0x15b03`.
  That handler passes a target value of one and
  `MediaSequenceCounterEqualsTarget` at `0x15ac8` to `RunMediaSequence` at
  `0x1e516`. The callback is evaluated after each presented frame, so the
  command decodes and retains the first Smacker frame before returning to the
  scene chooser. The later BA0 frame presentation plays the full sequence.
- Engine-local media code should remain an adapter or demultiplexer wherever
  packet payloads can be handed to existing ScummVM codecs.
