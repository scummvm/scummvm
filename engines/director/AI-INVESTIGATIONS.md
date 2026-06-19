# AI-Assisted Investigations for the Director Engine

This document describes how to run focused AI-assisted
investigations into the ScummVM Director engine (`engines/director/`). The goal
is to keep an AI agent anchored to authoritative source material — 
original Lingo/Director documentation and decompiled game scripts —
rather than letting it reason purely from ScummVM's own implementation.

It is important to note that while these techniques will significantly increase the accuracy of AI-driven investigations, they will not make them perfect. Agents can still make mistakes, and it is important to verify their findings. 

> **Reminder** Any code produced during these investigations must
> follow [`AI-GUIDELINES.md`](../../AI-GUIDELINES.md). Disclose AI assistance in
> the commit with `Assisted-by: Claude:<model>`, never list an agent as
> (co-)author, and never submit code you do not understand and have not tested.

## Common Hallucinations

ScummVM's Director engine implementation is incomplete and, in
places, deliberately or accidentally divergent from the original.
If not properly prompted with the right context, agents will try to make changes to match the codebase, rather than adhere to original Director behavior.

The fix is to give the agent better sources of truth than the code under
investigation:

1. **The official Director and Lingo documentation** (as plain text) — the
   specification of how a command, handler, or property is supposed to behave.
2. **The actual game's decompiled Lingo** (via ProjectorRays) — the concrete
   behavior the game expects from the engine.

With both in context, the agent can reason: *"The game script calls X; the
Lingo docs say X should do Y; ScummVM currently does Z — therefore the bug is in
ScummVM's implementation of X."*
## Tool 1: ProjectorRays — decompiling Lingo to readable text

[ProjectorRays](https://github.com/ProjectorRays/ProjectorRays) reconstructs
Lingo source from published/protected Director files. An agent cannot read a
`.dcr`/`.dxr` binary, but it can read decompiled Lingo.

### Usage

```bash
# Decompile a single movie/cast (or a directory of them) and restore source.
./projectorrays decompile <input path> [-o <output path>]

# Dump the reconstructed Lingo scripts as text — the part an agent reads.
./projectorrays decompile <input path> --dump-scripts

# Identify the Director version a file was authored with (drives D2–D12 handling).
./projectorrays version <input path>
./projectorrays version <input path> --style integer

# Lower-level debugging of the file format itself.
./projectorrays decompile <input path> --dump-chunks
./projectorrays decompile <input path> --dump-json -v
```

### Recommended investigation flow

1. Run `version` on the target file first. The Director version determines which
   bytecode format, Lingo dialect, and ScummVM code paths are relevant.
2. Run `decompile --dump-scripts` and save the output to a plain `.txt`/`.lingo`
   file in your scratch area.
3. Feed that decompiled text to the agent. Now the agent can point at the exact
   handler, the exact arguments, and the exact properties the game relies on.

## Tool 2: Director & Lingo documentation as plain text

The most important habit for accurate investigations: provide the official
Director and Lingo reference documentation to the agent as plain text, and
explicitly instruct the agent to treat it as the source of truth.

### Why plain text, and why "source of truth"

- **Plain text is what the agent can actually read and search.** PDFs, CHM help
  files, and HTML help bundles should be converted to `.txt`/`.md` up front so
  the agent can grep, quote, and cite specific passages.
- **It outranks the ScummVM code.** When the docs and the C++ disagree, the docs
  describe the intended behavior and the C++ describes the current (possibly
  incorrect) behavior. State this priority order to the agent directly, e.g.:
  *"Use the attached Lingo Dictionary as the definition of correct behavior."*

### Documents worth converting and keeping handy

Pull the Windows Help files directly from your Director installation. You can use
[helpdeco](https://github.com/pmachapman/helpdeco) to convert them to rich-text and
then convert those files to plain text, then feed them to the agent as reference material.

Keep these converted documents in a stable scratch location and reference them
by path in your prompts so they are easy to re-attach across sessions.

## Tool 3: ScummVM logs

Collecting logs from scummvm is essential for debugging. Use the following
command to collect logs from scummvm, and save them to a file.

```bash
./scummvm <target> --debugflags=<debugflags> 2>&1 | tee scummvm.log
```

Use the `--debugflags` option to specify the debug flags to use. Common debug
flags include:

- `lingoexec`
- `compile`
- `loading`
- `xobj`
- `lingostrict`

## Focused Investigation Strategy

A reliable investigation usually combines all three sources. Example workflow
for "Game X misbehaves when it does Y":

1. **Locate the behavior in the game.** Decompile the relevant movie/cast with
   ProjectorRays (`--dump-scripts`) and find the handler responsible for Y.
2. **Define correct behavior from the docs.** Look up every Lingo command,
   property, and function that handler uses in the plain-text Lingo Dictionary.
   Write down what each is *supposed* to do for this Director version.
3. **Conclude with a citation.** The fix should be justified by the docs and the
   game's actual usage — not by "this looks wrong." Cite the doc passage and the
   decompiled call site.

An investigation can still be yield results if you are relaxed in the specifics of any of the above areas. In this case, just be aware that the agent may target its investigation in an incorrect manner. Always verify what it tells you.

### Prompt scaffolding to guide the agent
When kicking off an investigation, set the ground rules explicitly:

- "Here is the decompiled Lingo for the failing movie (`<path>`). Here is the
  official Lingo Dictionary as plain text (`<path>`)."
- "Here is the ScummVM log for the failing movie (`<path>`)."
- "Treat the Lingo Dictionary as the source of truth for intended behavior.
  ScummVM's C++ is a re-implementation and may be wrong."
- "For every claim about how a Lingo command behaves, quote the relevant
  documentation passage."
- "Before proposing a code change, show: (a) the game's call site, (b) the
  documented behavior, (c) the current ScummVM behavior, (d) the specific
  divergence."

## Quick reference

```bash
# Decompile and read a game's scripts
./projectorrays version <file>
./projectorrays decompile <file> --dump-scripts > scripts.lingo

# Trace ScummVM's runtime behavior for comparison
./scummvm <target> --debugflags=lingoexec   # bytecode execution trace
./scummvm <target> --debugflags=compile      # Lingo compilation
./scummvm <target> --debugflags=loading      # archive/resource loading
./scummvm <target> --debugflags=xobj         # XObject open/close
./scummvm <target> --debugflags=lingostrict  # drop into debugger on Lingo error
```

Pair the decompiled scripts and the runtime trace with the plain-text Lingo
documentation, and the agent can produce conclusions grounded in what Director
*should* do — not just what ScummVM currently does.
