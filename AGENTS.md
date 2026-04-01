# AGENTS.md

## Purpose

This project uses LLM-assisted reverse engineering with Ghidra and Ghidra MCP to analyze a DOS game binary. The goal is to progressively recover meaningful program structure by tracing execution from the application entry point, identifying functions, variables, globals, data structures, and subsystem boundaries, and renaming only when there is high confidence.

This repository also maintains an `ARCHITECTURE.md` file that records confirmed subsystem discoveries and their relationships.

Accuracy matters more than speed. Never guess.

---

## Primary Objectives

1. Start analysis at the application entry point.
2. Follow control flow outward to identify:
   - functions
   - global variables
   - local variables
   - data structures
   - tables
   - buffers
   - dispatch logic
   - subsystem boundaries
3. Rename symbols only when their purpose is supported by strong evidence.
4. Record confirmed subsystem discoveries in `ARCHITECTURE.md`.
5. Use strings, string cross-references, DOS interrupts, calling patterns, and data flow as primary sources of evidence.
6. Preserve uncertainty explicitly. If confidence is low, do not rename and do not document as fact.

---

## Ground Rules

### 1. Never guess
Do not rename a function, variable, struct, enum, field, or table unless the available evidence supports the meaning with high confidence.

Avoid speculative names such as:
- `maybe_draw_sprite`
- `probably_load_file`
- `unknown_audio_thing`

If confidence is insufficient, leave the original name in place or apply only a strictly descriptive neutral name if justified by observable behavior, such as:
- `memcpy_like`
- `int21_file_io_wrapper`
- `table_of_far_ptrs`
- `state_dispatch_table`

### 2. Evidence over intuition
Every rename and every `ARCHITECTURE.md` update must be grounded in evidence such as:
- string contents and string references
- DOS interrupt usage
- BIOS interrupt usage
- file access patterns
- video memory writes
- buffer shapes and access patterns
- call graph relationships
- repeated call-site behavior
- resource loading sequences
- script interpreter patterns
- structure layout and field usage

### 3. Work from the entry point outward
Begin at the program entry point and proceed in execution order as much as possible. Prefer understanding initialization, subsystem setup, and top-level dispatch before diving into leaf functions.

### 4. Prefer reversible, minimal, precise changes
Rename conservatively. A smaller number of correct renames is better than many wrong ones.

### 5. Separate confirmed facts from working hypotheses
Only confirmed facts belong in `ARCHITECTURE.md`.

Do not write:
- guesses
- possibilities
- loose speculation
- subsystem claims based on one weak clue

---

## Analysis Workflow

## Phase 1: Entry Point and Startup Recovery

Start at the application entry point and identify:

- startup and initialization flow
- memory/model setup
- segment register initialization
- heap/buffer setup
- resource/bootstrap loading
- video/audio/input initialization
- main loop entry
- shutdown/cleanup path

Tasks:
1. Trace the first layer of calls from the entry point.
2. Identify initialization clusters by behavior.
3. Mark wrappers around common DOS/BIOS interrupts.
4. Identify central state objects, global flags, mode variables, and dispatch tables.
5. Rename only high-confidence startup functions.

Examples of acceptable names if justified:
- `game_entry`
- `initialize_video`
- `initialize_audio`
- `initialize_input`
- `main_loop`
- `shutdown_and_exit`

Only use these names when the evidence is strong.

---

## Phase 2: Systematic Symbol Recovery

For each discovered function, variable, or data structure:

### Functions
Determine:
- what calls it
- what it calls
- what data it reads/writes
- whether it wraps a DOS/BIOS interrupt
- whether it processes strings, files, graphics, scripts, or resources
- whether it is a leaf helper or subsystem coordinator

Rename functions using:
- concrete behavior
- subsystem context
- observable side effects

Good examples:
- `open_resource_file`
- `read_resource_chunk`
- `draw_mouse_cursor`
- `decode_rle_scanline`
- `script_execute_opcode`
- `blit_backbuffer_to_vram`

Bad examples:
- `handle_game_stuff`
- `video_related`
- `sound_func`
- `do_script_maybe`

### Variables and globals
Determine:
- lifetime
- scope
- initialization site
- write/read locations
- relation to mode/state/subsystem operation
- whether it is a pointer, counter, flag, buffer, handle, or table

Prefer names like:
- `current_video_mode`
- `resource_file_handle`
- `mouse_x`
- `mouse_y`
- `active_script_pc`
- `palette_buffer`

Only if proven.

### Data structures
Look for:
- repeated field offsets
- arrays of records
- pointer tables
- object/state records
- decoded resource headers
- animation/script/resource metadata

Name structures only after enough field usage is understood.

Good examples:
- `ResourceHeader`
- `SpriteDescriptor`
- `ScriptContext`
- `CursorState`

If not enough is known, prefer temporary neutral names such as:
- `struct_XXXX_candidate`
- `resource_record_candidate`

---

## Strings-Driven Analysis

The strings table is a major source of context and must be used aggressively.

For each meaningful string:
1. Identify cross-references to the string.
2. Determine whether it is used for:
   - error reporting
   - debug/logging
   - file/resource names
   - script commands
   - UI text
   - copy protection
   - device/system checks
   - command dispatch
3. Follow the referencing function outward and inward in the call graph.
4. Use clustered strings to infer subsystem boundaries.

Examples:
- File extensions or resource names may indicate resource loading or archive management.
- UI/status messages may reveal menu, inventory, cursor, or script systems.
- Error strings may expose file handling, memory allocation, decompression, or driver init paths.

Do not infer more than the string supports.

A string saying `AdLib` may suggest audio relevance. It does not by itself prove the exact role of the entire function.

---

## DOS and BIOS Interrupt Heuristics

Interrupt usage is a strong clue and must be incorporated into analysis.

### DOS interrupts
Pay particular attention to:
- `int 21h` for file management, memory allocation, program termination, device I/O, directory access, etc.
- FCB- or handle-based file operations
- load/execute behaviors
- DTA manipulation
- PSP/environment interactions

### BIOS interrupts
Pay particular attention to:
- `int 10h` for video mode changes, cursor, text output, palette/video services
- `int 13h` for disk access
- `int 16h` for keyboard input
- `int 1Ah` for timer/time services
- `int 33h` for mouse services, if present via driver interrupt interface

### Hardware-facing patterns
Also look for:
- direct writes to VGA memory
- palette register I/O
- PIT/PC speaker programming
- AdLib/Sound Blaster port I/O
- keyboard controller access
- DMA-related setup
- timer hooks or interrupt vector manipulation

Use these clues to classify behavior, but only rename once supported by surrounding code and data flow.

Example:
- A function invoking `int 21h` alone is not necessarily `load_file`.
- A function opening a named asset, seeking, reading into a buffer, and returning a handle-sized or byte-count result may justify `open_resource_file` or `read_resource_data`.

---

## Subsystem Discovery Rules

As subsystem boundaries become clear, record them in `ARCHITECTURE.md`.

Candidate subsystems include:
- video
- graphics rendering
- sprite or animation handling
- palette management
- cursor management
- keyboard/mouse input
- audio/music/sfx
- script engine
- text/dialogue
- decoders/decompression
- resource/archive management
- save/load
- memory management
- scene/state management

### Only document a subsystem when at least one of the following is true:
1. There is a clear cluster of related functions with consistent behavior.
2. There are clear shared globals/structures that define subsystem state.
3. Strings or resources strongly tie the functions together.
4. Interrupt/hardware usage and data flow clearly indicate a distinct responsibility.

### For each confirmed subsystem, record:
- subsystem name
- confidence level: `High`
- why it is considered confirmed
- key functions
- key globals/structures
- notable strings
- relevant interrupts or hardware clues
- known relationships to other subsystems

Do not add low-confidence or speculative subsystems.

---

## ARCHITECTURE.md Update Policy

`ARCHITECTURE.md` is a record of confirmed understanding, not a scratchpad.

Only add content when:
- the subsystem or relationship is supported by multiple strong clues
- names used are stable and justified
- the finding would still make sense to another analyst reviewing the evidence later

Each entry should be concise and factual.

Recommended format:

```markdown
## Video Subsystem

**Confidence:** High

**Evidence**
- Functions at `FUN_xxxx`, `FUN_yyyy`, and `FUN_zzzz` change video mode via `int 10h`
- Shared global buffer used as backbuffer before copy to VRAM
- Palette update routine writes through VGA-related I/O sequence
- Strings referencing mode/setup failure are used by the initialization path

**Key Functions**
- `initialize_video`
- `set_video_mode`
- `blit_backbuffer_to_vram`
- `update_palette`

**Key Data**
- `video_state`
- `backbuffer`
- `palette_buffer`

**Notes**
- Video initializes before the main loop
- Rendering appears to be separated from resource decoding
````

Do not include unresolved claims.

---

## Confidence Standard for Renaming

A rename is allowed only when the name is supported by multiple converging signals.

High-confidence signals include combinations of:

* clear interrupt semantics
* clear string references
* clear file/resource names
* repeated consistent call-site usage
* obvious buffer or structure behavior
* direct hardware interaction
* strong structural relationships in the call graph

### Rename threshold

Rename only if at least two or more strong signals converge, or one signal is exceptionally definitive.

Examples of sufficiently strong evidence:

* function opens a named asset file, uses DOS file interrupts, reads into a destination buffer, and is called by resource init code
* function writes to video memory or uses video BIOS services and is called by rendering flow
* function dispatches on bytecodes read from a script stream and updates script context fields
* function uses mouse interrupt services and updates cursor coordinates/state

### If evidence is incomplete

Do one of the following:

* leave the original name unchanged
* apply a narrowly descriptive placeholder based on directly observable mechanics only

Examples:

* `reads_buffer_with_length_prefix`
* `far_ptr_dispatcher`
* `int10_video_service_wrapper`
* `copies_words_to_segment`

Avoid semantic overreach.

---

## Naming Conventions

Use clear, consistent, descriptive names.

### Functions

Use verb-oriented names:

* `initialize_video`
* `load_palette`
* `decode_sprite_frame`
* `execute_script_command`
* `poll_keyboard_input`

### Variables

Use noun-oriented names:

* `current_room_id`
* `resource_index`
* `cursor_visible`
* `audio_driver_type`

### Structures

Use PascalCase:

* `VideoState`
* `ScriptContext`
* `ResourceEntry`

### Constants / enums

Use uppercase when appropriate:

* `VIDEO_MODE_13H`
* `RESOURCE_TYPE_SPRITE`

### Unknowns

When forced to use an interim name, keep it descriptive and non-speculative:

* `bytecode_stream_ptr`
* `video_buffer_candidate`
* `file_io_ctx_candidate`

---

## Preferred Investigation Tactics

When analyzing a function, prefer this order:

1. Identify callers.
2. Identify callees.
3. Inspect strings referenced directly or indirectly.
4. Inspect interrupts and I/O operations.
5. Track major buffers and globals touched.
6. Look for repeated structural patterns.
7. Determine whether the function belongs to an already-known subsystem.
8. Decide whether rename confidence is high enough.

When analyzing a global or structure:

1. Find all writes.
2. Find all reads.
3. Determine initialization.
4. Determine whether access patterns imply flags, counters, coordinates, handles, or pointers.
5. Associate with a subsystem only if the evidence is strong.

---

## MCP / Ghidra Usage Expectations

When operating through Ghidra MCP:

* begin from the entry point unless continuing an already-confirmed analysis thread
* inspect decompiler output, disassembly, xrefs, and data definitions together
* follow string references systematically
* inspect interrupt usage and surrounding setup/register state
* examine tables and indirect call/jump targets
* improve type information when supported by evidence
* rename incrementally and conservatively
* update `ARCHITECTURE.md` only after confirmation

Do not mass-rename symbols based on pattern matching alone.

---

## What to Avoid

Do not:

* invent subsystem names without proof
* rename based on vague resemblance
* treat every `int 21h` call as generic file loading
* treat every memory copy as rendering
* assume every byte stream is a script
* collapse unrelated helpers into a subsystem prematurely
* document tentative conclusions in `ARCHITECTURE.md`
* overwrite neutral names with stronger semantic names unless the new evidence truly supports it

---

## Output Expectations

During analysis, produce:

1. Conservative symbol renames with high confidence
2. Confirmed subsystem notes appended to `ARCHITECTURE.md`
3. Clear explanation of evidence for each non-trivial rename
4. Explicit acknowledgment of uncertainty where confidence is not high enough

For every important rename, include rationale in working notes or commit messages such as:

* string references
* interrupt semantics
* caller/callee context
* buffer usage
* structure field evidence

---

## Operating Principle

Recover the program one confirmed fact at a time.

Start from the entry point.
Use strings and xrefs aggressively.
Use DOS/BIOS interrupts as behavioral clues.
Track data flow carefully.
Rename only with high confidence.
Record only confirmed architecture.
Record the last confirmed action we completed, and the next suggested action in TRACKER.md
Record high level progress indicator that summarizes how many total functions are in Ghidra, and how many are still not renamed (ex: FUN_*) in TRACKER.md
Ensure TRACKER.md only contains the *last* action take and the *next suggested* action

Never guess.

---

## ScummVM Engine Conventions

When editing ScummVM engine code in this repository, including `engines/harvester`, follow these rules in addition to the reverse-engineering workflow above.

### Portability and language subset

- Use ScummVM/common types and utilities in engine code instead of `std::` library types.
- Prefer `Common::String`, `Common::Array`, `Common::HashMap`, `Common::ScopedPtr`, `Common::SharedPtr`, and other `common/` facilities.
- If code is being ported from an external codebase and genuinely needs an STL-style wrapper, include the relevant `common/std/...` header and use `Std::`, never `std::`.
- Do not use C++ exceptions.
- Do not add global or function-local static objects that require constructors.
- Non-const static locals inside function bodies are forbidden.
- If a non-const global is unavoidable, document why it exists and where it is reset when the engine starts or shuts down.

### Endianness, serialization, and struct layout

- Never rely on host endianness for persisted data, imported assets, or binary parsing.
- Use `Common::Serializer`, stream endian helpers, or `READ_/WRITE_` macros from `common/endian.h`.
- Do not serialize raw structs directly.
- Do not assume native struct packing or layout.
- If packed structs are unavoidable, use `common/pack-start.h`, `PACKED_STRUCT`, and `common/pack-end.h`.

### File access

- Do not use `fopen`, `fread`, `fwrite`, `open`, `close`, or other raw C/POSIX file APIs in engine code.
- Use ScummVM file APIs such as `Common::File`, `Common::DumpFile`, `Common::FSNode`, `SearchMan`, and archive streams.
- Use `File::open("relative/name")` only for SearchMan-relative lookups.
- Do not pass absolute paths to `File::open()`.
- If a path comes from config or another filesystem source, first build a `Common::FSNode`, then open via `File::open(node)`, `DumpFile::open(node)`, `node.createReadStream()`, or `node.createWriteStream()`.
- When traversing directories or handling platform-native filesystem locations, prefer `Common::FSNode` operations such as `getChild()`, `getParent()`, and `getPath()`.

### Formatting and naming

- Apply common sense, but default to ScummVM formatting conventions.
- Use tabs for indentation with a tab width of 4.
- Use hugging braces: `if (...) {` / `} else {`.
- Avoid composite one-liners; prefer one statement per line.
- Use braces for nested conditionals and for `if`/`else` chains when omission would reduce clarity.
- Put spaces around operators, after commas, after comment markers, and between reserved words and `(`.
- Use `char *ptr` and `int &ref` style spacing.
- Keep namespace contents unindented.
- Keep preprocessor directives starting in column 1.
- For intentional `switch` fallthrough, use the exact comment `// fall through`.
- Add vertical space between logical blocks when it improves readability.
- Use ScummVM naming conventions: types in `UpperCamelCase`, methods/functions/locals in `lowerCamelCase`, member variables in `_lowerCamelCase`, globals with `g_` prefix, and constants either as `kCamelCase` or `ALL_CAPS_WITH_UNDERSCORES`.

### Comments and documentation

- Use `FIXME`, `TODO`, `WORKAROUND`, and `I18N:` comments only with specific explanatory text.
- Prefer JavaDoc-style Doxygen comments for public APIs when new documentation is warranted.
