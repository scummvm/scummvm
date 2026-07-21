# AGENTS.md

## Scope

These instructions apply when reimplementing an engine from Ghidra analysis.
Treat the disassembly as the behavioral source of truth and do not present
unverified interpretations as facts.

## Project conventions

- Follow `AI-GUIDELINES.md` and ScummVM's coding, portability, and commit
  guidelines linked from `CONTRIBUTING.md`. AI-assisted work must retain human
  ownership, review, testing, licensing compliance, and commit attribution.
- Prefer ScummVM APIs and types over the C/C++ standard library or direct
  platform APIs. Use `OSystem`/`g_system` for platform services and `Common::*`
  facilities for strings, containers, files, streams, serialization, and
  parsing (for example, use ScummVM string parsing instead of `atoi`).
- Search for an existing ScummVM decoder, parser, service, or utility before
  implementing equivalent engine-local code. Add game-specific infrastructure
  only for behavior or formats that ScummVM does not already support.
- Match the surrounding engine's naming, formatting, ownership, and error
  handling patterns.

## Reimplementation changes

- Implement only behavior supported by the disassembly, decompiler output,
  data flow, call sites, strings, or observed runtime behavior. Preserve
  uncertainty when the evidence is incomplete.
- Keep the relationship between the original program and the reimplementation
  easy to audit. Where useful, retain original addresses, constants, structure
  offsets, state transitions, or control-flow relationships in code comments.
- Match the original game-loop ordering and input semantics established in
  Ghidra. Translate platform interrupts through `OSystem` and ScummVM events,
  but preserve which keys, mouse transitions, and callbacks act in each state.
- Add `debugC` calls with every vertical slice. Log subsystem lifecycle and
  errors at level 1, resource and state transitions at level 2, and detailed
  offsets, opcodes, input translation, packets, and frames at level 3. Keep
  channel names stable so logs from ScummVM can be compared with Ghidra.
- Maintain a separate `{GAMEID}-ARCHITECTURE.md` for each analyzed game (for
  example, `HARVESTER-ARCHITECTURE.md`). Create it when analysis of a new game
  begins; do not mix findings from different binaries.
- Evolve the game architecture document with the implementation. Record only
  confirmed subsystem responsibilities, key data, Ghidra anchors, and the
  control/data flow connecting subsystems. Update existing sections when the
  model changes instead of accumulating chronological investigation notes.
- Keep hypotheses and unresolved interpretations out of the architecture
  document. The document should describe the current verified understanding
  and make subsystem boundaries and dependencies easy to trace.

## Commits

- Follow the project's commit-message guidelines and keep commits focused.
- Use real newline characters between the subject, body paragraphs, and
  trailers. Never embed the two-character sequence `\\n` in a commit message;
  use separate `-m` arguments or a commit-message file instead.
- End AI-assisted commits with an `Assisted-by` trailer whose Codex model value
  is `gpt-{version}` or `gpt-{version}-{model}`, as appropriate (for example,
  `Assisted-by: Codex:gpt-5.5` or `Assisted-by: Codex:gpt-5.6-sol`). Do not
  append `Ghidra` or other workflow/tool labels to the model identifier.
- Write commit messages for human reviewers. Explain the player-visible or
  engine behavior being implemented and how it maps to the disassembly.
- Include the most useful verification anchors when applicable: function
  addresses or names, relevant call paths, data offsets, constants, and notable
  deviations required by ScummVM abstractions. Identify Ghidra functions by
  function name and address only; do not include local disassembly or decompiler
  URLs in commit messages.
- Describe evidence and intent, not the mechanics of editing files. A reviewer
  should be able to use the commit message to compare the implementation with
  Ghidra and troubleshoot mismatches.
