# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `927`
- Named/documented: `823`
- Remaining `FUN_*`: `104`
- Remaining undocumented total: `104`

## Last Confirmed Action

- On March 31, 2026, decoded the authored CD1/CD2 `HARVEST.SCR` records for `GO_BROOMPICCU` and confirmed that `BROOMPICCU` is a valid closeup room with a single visible sprite-backed object at `(0,0)` named `BROOMPIC_CU`. Updated Harvester room materialization so it still prefers true `640x480` backgrounds, but now falls back to the explicit origin sprite-backed room object when no fullscreen match exists, and carries that chosen object name into scene-entity classification so closeup backdrops remain non-interactive. Verified with `git diff --check` and a successful `make -C build-vscode-harvester-debug -j4 engines/harvester/libharvester.a`.

## Next Suggested Action

- Replay the remaining inventory document/photo closeups such as `GO_BOYLCOPYCU`, `GO_CASKTPICCU`, `GO_REGISTERCU`, and `GO_TVDEED1CU` to confirm the fallback background selection covers every authored closeup room and that clicks only land on the intended `EXIT_BM` / `EXIT_HS` hotspots.
