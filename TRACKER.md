# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `923`
- Named/non-`FUN_*`: `864`
- Remaining `FUN_*`: `59`
- Remaining real-space `FUN_*`: `16`
- Remaining zero-xref `.image` `FUN_*` artifacts: `43`

## Last Confirmed Action

- On April 16, 2026, continued the `FUN_*` triage pass in Ghidra from the remaining 104 entries and reduced the counted `FUN_*` total to 59. Renamed 39 high-confidence functions across the Chessmaster FLC path, VESA banked blit helper, FST dialogue wrappers, IRQ0 timer chaining, NO87 extended-precision arithmetic/polynomial helpers, decimal conversion helpers, and exception-runtime throw/catch transfer path; one renamed function, `runtime_throw_or_rethrow_exception`, had been custom-marked and was not included in the 104-count query. Deleted 7 bogus function starts that were proven to be continuation blocks or data, including `FUN_000c55f8`, which is now labeled as the IRQ0 timer callback far-pointer slot. Updated `ARCHITECTURE.md` with confirmed NO87/exception-runtime and FLIC playback notes.

## Next Suggested Action

- Continue with the 16 remaining real-space `FUN_*` entries, starting by proving the exact opcode contracts for the NO87 redispatch/conversion handlers (`FUN_0008c57a`, `FUN_0008c5cc`, `FUN_0008e0ab`, `FUN_0008e0b0`) before considering any rename. Treat the 43 zero-xref `.image` entries as a separate cleanup audit and delete them only after confirming each is raw-image duplicate/non-runtime code.
