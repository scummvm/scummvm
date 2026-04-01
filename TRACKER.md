# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `927`
- Named/documented: `823`
- Remaining `FUN_*`: `104`
- Remaining undocumented total: `104`

## Last Confirmed Action

- On March 31, 2026, aligned the Harvester engine with the documented ScummVM portability and formatting conventions by replacing remaining `std::function`/`std::unique_ptr` usage with ScummVM-native patterns, removing hidden function-local static object state, documenting the `g_engine` global reset behavior, switching startup-config writes to `FSNode`-based file opening, removing leftover `<functional>` includes from the NPC dialogue sources, and codifying the ScummVM engine rules in `AGENTS.md`. Verified with `git diff --check` and a successful `make -C build-vscode-harvester-debug -j4 engines/harvester/libharvester.a`.

## Next Suggested Action

- Do a runtime smoke test that exercises room-NPC dialogue, response menus, keyword entry, and startup config persistence to confirm the refactored dialogue session plumbing and `FSNode`-based config writes behave identically in-game.
