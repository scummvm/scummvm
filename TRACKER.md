# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `906`
- Named/documented: `791`
- Remaining `FUN_*`: `115`
- Remaining undocumented total: `115`

## Last Confirmed Action

- On March 22, 2026, compared DOSBox-X output against ScummVM's startup room/menu palette handling and re-opened the native palette path in Ghidra.
- Confirmed the executable's room/menu brightness behavior still matches the existing reverse-engineered gamma logic (`update_gamma_brightness_scale` keeps `1.0 + 0.1 * GAMMA` and `upload_palette_to_vga` still writes 6-bit DAC values after truncating `.PAL` bytes with `>> 2`), but ScummVM's `expandHarvesterVgaDacColor()` was re-expanding those DAC values with rounded `*255/63`. Patched `engines/harvester/palette_utils.h` to use VGA-faithful bit replication (`(value << 2) | (value >> 4)`), which matches DOSBox-X's 6-bit DAC expansion and should reduce the default "washed out" bias in midtones.

## Next Suggested Action

- Immediate: rerun the DOSBox-vs-ScummVM startup menu / first-room comparison from the same save or boot path. Confirm the darker wall gradients and red menu lettering now sit closer to DOSBox-X at `GAMMA=0`, then sanity-check one dim room and one nonzero gamma setting to make sure the palette shift stays faithful outside the default case.
