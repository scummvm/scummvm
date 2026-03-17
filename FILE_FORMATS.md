# File Formats

This document tracks file-format facts that are confirmed by the current Harvester reverse-engineering work. It is intentionally conservative: if a field or behavior is not supported by code or verified runtime notes, it is left out.

## DAT (XFILE archive payload)

Harvester's `.DAT` files act as payload containers for the game's numbered XFILE resource sets. The important wrinkle is that the `.DAT` file is not the whole format by itself: the loader depends on a sidecar `INDEX.00N` file for the directory. In other words, the `.DAT` holds the bytes, while the `INDEX.00N` records tell the engine where each member starts, how large it is, and whether it must be unpacked after reading.

At confirmed cold start, the original game mounts these pairs in this order:

| Set | Index file | Data file | Notes |
| --- | --- | --- | --- |
| 1 | `INDEX.001` | `HARVEST.DAT` | First archive-backed set mounted during startup. Confirmed startup art and UI assets are loaded through this set. |
| 2 | `INDEX.002` | `SOUND.DAT` | Second numbered set mounted during startup. |
| 3 | `INDEX.003` | `HARVEST2.DAT` | Third numbered set mounted during startup. |

### Summary

| Property | Value |
| --- | --- |
| Format role | Archive payload file used by the XFILE resource layer |
| Companion metadata | Required sidecar `INDEX.00N` directory file |
| Archive-wide header in `.DAT` | No confirmed archive-wide header or in-file directory has been required by the current loader |
| Addressing model | Members are looked up by path in `INDEX.00N`, then read from absolute offsets in the `.DAT` |
| Path syntax in game code | Archive-backed lookups use `<set-number>:\path\to\file.ext` |
| Endianness | Mixed in the sidecar index: signature is checked as big-endian `XFLE`; numeric fields are little-endian 32-bit values |
| Compression | Optional per-entry packing; unpacked entries are read directly, packed entries are expanded after read |

### `.DAT` payload layout

| Offset | Size | Description |
| --- | --- | --- |
| `0x00` | variable | Raw member data addressed by the sidecar index. The current loader treats the file as a byte reservoir and does not require a confirmed global header before opening entries. |

This is sparse compared with many archive formats, but that sparseness is itself useful for a blog post: the format's structure is directory-driven rather than self-describing. All meaningful per-file metadata currently comes from the matching `INDEX.00N` file.

### `INDEX.00N` directory record layout

Each confirmed sidecar index is read as a flat array of `0x94`-byte records:

| Offset | Size | Type | Name | Description |
| --- | --- | --- | --- | --- |
| `0x00` | `0x04` | ASCII | `signature` | `XFLE` magic. The loader rejects records whose first four bytes do not match this tag. |
| `0x04` | `0x80` | char[128] | `path` | NUL-terminated resource path string. This is the logical member name used for later lookups. |
| `0x84` | `0x04` | uint32le | `archive_offset` | Absolute offset of the member data inside the companion `.DAT` file. |
| `0x88` | `0x04` | uint32le | `stored_size` | Number of bytes stored in the `.DAT` for this member. |
| `0x8c` | `0x04` | uint32le | `packed_flag` | `0` means the member is stored verbatim. Any nonzero value takes the packed-entry decode path. The current loader does not distinguish between different nonzero flag values. |
| `0x90` | `0x04` | uint32le | `unpacked_size` | Expected output size after decode for packed entries. |

### Packed entry stream

When `packed_flag != 0`, the loader reads `stored_size` bytes from the `.DAT` and expands them into an `unpacked_size` output buffer using a simple control-byte stream:

| Control byte range | Meaning |
| --- | --- |
| `0x00` to `0x80` | Copy the next `control` bytes literally into the output stream. |
| `0x81` to `0xff` | Read one byte and repeat it `control - 0x80` times. |

The decode loop stops once either the compressed input is exhausted or the output buffer reaches `unpacked_size`. Entries that fail to produce exactly `unpacked_size` bytes are treated as invalid.

### Path handling notes

| Behavior | Detail |
| --- | --- |
| Separators | Resource paths are normalized from DOS-style backslashes to forward slashes during lookup. |
| Prefix stripping | Leading `./`, leading `/`, and `<digit>:/` prefixes are stripped during normalization. |
| Case handling | Archive member lookups are case-insensitive in the current implementation. |
| Loose files vs archive paths | Bare relative paths are handled separately by the direct-file path builder; `<digit>:\...` paths select an archive set instead. |

### Practical interpretation

For extraction or tooling work, the current evidence supports treating Harvester's `.DAT` archives as a two-file format:

1. Read the matching `INDEX.00N` file as a sequence of `0x94`-byte `XFLE` records.
2. Use each record's `archive_offset` and `stored_size` to slice bytes from the companion `.DAT`.
3. If `packed_flag` is zero, the slice is the final file.
4. If `packed_flag` is nonzero, expand the slice with the literal/repeat decoder above until `unpacked_size` bytes are produced.

That model is enough to explain the startup resource mounts already confirmed in the reverse-engineering notes, and it matches the current ScummVM-side archive loader.
