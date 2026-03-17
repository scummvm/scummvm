# TRACKER

## Current Focus

- Rebuild talk-handler ports from corrected native control flow
- Keep each NPC port scoped to confirmed native behavior only
- Commit each completed NPC update as an isolated change

## Progress

- Program: `HARVEST.LE`
- Total functions: `902`
- Named/documented: `583`
- Still `FUN_*` / undocumented: `319`

## Last Confirmed Action

- Rebuilt `handle_talk_to_mom @ 0x31140` from corrected native control flow and fixed the engine port to follow the full recovered sequence.
  - Mom now uses the confirmed native line sequences, head variants, FST calls, action tags, and keyword / response menu branches instead of the earlier truncated decompile.
  - The engine-side dialogue shared state now exposes Mom's day-5 cross-handler state so the remaining audit can wire the native producer back in.

## Next Suggested Action

1. Audit the remaining `handle_talk_to_*` ports against the corrected native disassembly.
   - The highest-value follow-up is `handle_talk_to_whaley @ 0x23ec0`, which still contains major unported menu and event logic, including the native producer for Mom's day-5 shared dialogue state.
2. Apply any additional high-confidence Ghidra renames or comments that fall directly out of those audited handlers before each commit.
