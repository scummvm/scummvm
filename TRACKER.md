# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `923`
- Named/non-`FUN_*`: `864`
- Remaining `FUN_*`: `59`
- Remaining real-space `FUN_*`: `16`
- Remaining zero-xref `.image` `FUN_*` artifacts: `43`

## Last Confirmed Action

- On April 16, 2026, compared ScummVM's Harvester FST decoder against native `run_fst_sequence_player @ 0x12b00` and patched the engine to match two confirmed codec details: two-color mask blocks map mask bits right-to-left within each nibble, and native decodes only `(height >> 2) - 1` four-scanline tile bands. Updated `ARCHITECTURE.md` with the confirmed FST bitstream evidence and verified `engines/harvester/fst_player.o` builds.

## Next Suggested Action

- Runtime-test representative FST playback with mask-heavy and tail-sensitive files such as `GRAPHIC/FST/FVLOGO.FST`, `GRAPHIC/FST/INTROFIN.FST`, `GRAPHIC/FST/C001B.FST`, `GRAPHIC/FST/C086.FST`, and `GRAPHIC/FST/RANGSHOT.FST` to confirm the visual artifacts are gone.
