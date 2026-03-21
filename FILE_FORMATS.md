# File Formats

This document tracks file-format facts that are confirmed by the current Harvester reverse-engineering work. It is intentionally conservative: if a field or behavior is not supported by code or verified runtime notes, it is left out.

The sections below combine three sources of evidence: the current ScummVM Harvester engine code, named native functions and data types in Ghidra, and spot-checks against extracted sample files from the Harvester CD image. Unknown or only partially understood fields stay marked as such.

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

## RCS (Quick-tip text lists)

The only confirmed `.RCS` use so far is `ADJHEAD.RCS`, the quick-tips file shown by the startup/options overlay. Both native `run_quick_tips_screen` and the ScummVM `Flow::loadQuickTips` path treat it as plain text rather than as a binary container.

### Summary

| Property | Value |
| --- | --- |
| Format role | Plaintext list of quick-tip strings |
| Confirmed consumer | Startup quick-tips overlay and options-menu quick-tips screen |
| Encoding | Plain ASCII text in the sampled file |
| Header | None |
| Delimiters | CR/LF line endings in the sampled file; loaders split on `\n` and ignore `\r` |

### File layout

| Offset / unit | Size | Description |
| --- | --- | --- |
| `0x00..EOF` | variable | Sequence of text lines. Each non-empty trimmed line becomes one quick-tip string. |

### Notes

- The sampled `ADJHEAD.RCS` begins directly with readable text: `Double click the left mouse button...`.
- The native quick-tips path advances through the file with `read_line_from_file_stream`, wraps back to the start on EOF, and chooses a random starting point the first time it runs.
- This is a good example of Harvester's resource pipeline staying pragmatic: not every gameplay-facing asset is wrapped in a custom binary format.

## DIALOG.RSP (Dialogue response and keyword text)

`DIALOG.RSP` is a plaintext line table used by the dialogue UI. It does not appear to contain ids, offsets, or a binary header; the engine indexes it by zero-based line number and then uses the line text directly.

### Summary

| Property | Value |
| --- | --- |
| Format role | Plaintext dialogue response / keyword string table |
| Confirmed consumers | `load_dialogue_response_line`, response menu, keyword menu |
| Encoding | Plain ASCII text in the sampled file |
| Header | None |
| Addressing model | Zero-based line index |

### File layout

| Unit | Description |
| --- | --- |
| Line `n` | One response, keyword label, or menu text string |
| `/` within a line | In response/keyword UI contexts, splits one source line into multiple visible menu items |

### Notes

- The native loader reads a requested zero-based line, strips the trailing CR/LF, and returns the shared text buffer directly.
- `run_dialogue_response_menu` and `run_dialogue_keyword_menu` split visible options on `/`; there is no confirmed hidden topic-id layer behind those labels.
- `load_dialogue_index` also pulls zero-based line `13` from `DIALOG.RSP` to seed the default keyword topic; in the sampled file that line is `BYE` (one-based line 14).
- As a narrative point for a blog post, `DIALOG.RSP` shows Harvester leaning on designer-editable text tables even inside a fairly custom dialogue stack.

## DIALOGUE.IDX (Subtitle index)

`DIALOGUE.IDX` is an XOR-obfuscated text file that maps voice sample ids to subtitle strings. The native `load_dialogue_index` path and the ScummVM `Text::loadDialogueIndex` reimplementation both decode it with XOR `0xAA` while leaving CR/LF intact.

### Summary

| Property | Value |
| --- | --- |
| Format role | Voice-id to subtitle-text index |
| Encoding on disk | ASCII text obfuscated with XOR `0xAA` on every byte except CR/LF |
| Header | None |
| Confirmed delimiters | `NUL`, `LF`, `CR`, and form-feed (`0x0c`) |
| Key native type | `DialogueIndexEntry { wav_id, text_offset, text_length }` |

### Decoded stream layout

| Sequence element | Description |
| --- | --- |
| ASCII decimal token | `wav_id` for one spoken line |
| Delimiter run | One or more of `NUL`, `LF`, `CR`, `FF` |
| ASCII text token | Subtitle text for that voice id |
| Delimiter run | Ends the subtitle record and starts the next id |

### Derived index entry

| Field | Meaning |
| --- | --- |
| `wav_id` | Positive decimal voice/sample id |
| `text_offset` | Byte offset of the decoded subtitle text inside the decoded blob |
| `text_length` | Subtitle length, clamped to `0x19a` bytes by the native loader |

### Notes

- After XOR decode, the sampled file starts with a simple alternating pattern: `1`, `"junk init"`, `7`, `"Yes?"`, `11`, `"I need some help..."`, and so on.
- The native loader builds a 3000-entry in-memory table and keeps the decoded text blob around so `play_dialogue_line` can seek back into it cheaply.
- Architecturally, this splits dialogue cleanly in two: `DIALOGUE.IDX` carries spoken-line subtitles keyed by numeric wav ids, while `DIALOG.RSP` carries menu-facing response text.

```python
python3 -c 'import sys,pathlib,signal; signal.signal(signal.SIGPIPE, signal.SIG_DFL); d=pathlib.Path(sys.argv[1]).read_bytes(); sys.stdout.buffer.write(bytes(b if b in (10,13) else b ^ 0xAA for b in d))' DIALOGUE.IDX | head -n 10
1
"junk init"
7
"Yes?"
11
"I need some help... Mister...?"
15
"Postmaster Boyle.  What can I do you for today?"
25
"Sorry, youngster, we're out of applications right now."
```


## SCR (Town/world startup script)

`HARVEST.SCR` is the central world-definition and startup-script file. It is XOR-obfuscated text, not bytecode. The native `load_xor_obfuscated_town_script` path and the ScummVM `Script::decode` path both XOR each non-CR/LF byte with `0xAA`, then parse whitespace-separated records with quoted strings preserved.

### Summary

| Property | Value |
| --- | --- |
| Format role | World database, startup configuration script, and command graph |
| Encoding on disk | ASCII text obfuscated with XOR `0xAA` on every byte except CR/LF |
| Header | None |
| Comment syntax | Lines whose first non-space character is `{` are skipped |
| Tokenization | Whitespace-delimited tokens with quoted strings kept intact |

### Confirmed record forms

| Tag | Leading numeric fields | Fields after the tag |
| --- | --- | --- |
| `ENTRANCE` | `x y z` | `direction roomName entranceName` |
| `MAP_ENTRANCE` | `mapX mapY initialPanelIndex` | `entryName` |
| `MAP_LOCATION` | `minX minY maxX maxY panelIndex labelX labelY` | `labelText destinationEntranceName` |
| `ROOM` | `minZ maxZ maxZScreenY minZScreenY fullScaleZ maxZScalePercent` | `roomName musicPath reservedString38 reservedString3c reservedString40 palettePath dimmable onEnterCommand onExitCommand` |
| `OBJECT` | `initialX initialY boundsX2 boundsY2 initialZ zExtent` | `initialOwnerOrRoom objectName spritePath altSpritePath reservedString40 inventoryTextKey reservedXFlag identTextKey operatable visible actionTag interactionLabel` |
| `ANIM` | `x y z frameDelay` | `roomName resourcePath animName active visible looping backward pingPong remove` |
| `NPC` | `x y z frameDelay` | `roomName modelPath npcName monsterfyTargetName active visible onDeathActionTag audioPath entityInitArg` |
| `MONSTER` | `x y z ...` | Sample data and native analysis confirm sound-trigger timing columns for attack, hit, footstep, and death. The intermediate string columns at offsets `0x38`, `0x3c`, `0x44`, and `0x48` remain reserved in current data and have no recovered read-side consumers. |
| `REGION` | `left top right bottom minZ maxZ` | `regionName direction roomName actionTag startEnabled cursorEnabled` |
| `FLAG` | none | `name value` |
| `COMMAND` | none | `triggerTag opcodeName arg1 arg2 arg3 [arg4]` |
| `TEXT` | none | `key boxName value` |
| `HEAD` | none | `headId portraitPath` |
| `USEITEM` | none | `itemName ownerOrRoom targetName actionTag` |

### Notes

- The sampled file starts with readable structural records immediately after XOR decode, for example `ENTRANCE`, `OBJECT`, and later `ROOM`, `TEXT`, `HEAD`, and `COMMAND`.
- Paths embedded in script records are not uniform: archive-backed resources use `1:\...`, `2:\...`, `3:\...`, while some direct-file assets are bare relative paths such as `dialogue.idx`.
- `GOFLIC` and `GODEATHFLIC` are especially revealing command names. In the sampled script, those opcodes point to `.FST` paths, which suggests the script vocabulary preserved older naming while the shipping runtime movie path used FST files.
- As a blog-post narrative, `HARVEST.SCR` is the clearest proof that Harvester's resource architecture is data-driven at the top level: rooms, objects, dialogue portraits, commands, music, palettes, and cutscene triggers all converge here.

```bash
python3 -c 'import sys,pathlib,signal; signal.signal(signal.SIGPIPE, signal.SIG_DFL); d=pathlib.Path(sys.argv[1]).read_bytes(); sys.stdout.buffer.write(bytes(b if b in (10,13) else b ^ 0xAA for b in d))' HARVEST.SCR | head -n 10
{// HARVESTER (c) 1995-96 Scripting Language
{// Town
        0   0   0   ENTRANCE "BACK" "TOWN_2_LODGE" "SERGEANT_2_LODGE"
        0   0   0   ENTRANCE "FRONT" "" "SAVE_GAME"
        492 67  554 96   0  1 OBJECT "INVENTORY" "INV_EXIT" "" "" "" "" "" "X" "F" "T" "" "Inventory"
        299 0   0   0    2  1 OBJECT "NULL_ID"          "EXIT_BM"  "1:\GRAPHIC\OTHER\EXITSIGN.BM" "" "" "" ""  "" "F" "T" "" "exit"
        299 0   375 61   0  1 OBJECT "NULL_ID"          "EXIT_HS"  ""                             "" "" "" "X" "" "T" "T" "" "exit"

{// inventory health indicator
     72 314 0   0  -12  1 OBJECT "INVENTORY" "INV_STAT1" "1:\graphic\other\head-a1.bm" "" "" "INV_STAT_ST" "" "" "F" "F" "" "Health_Indicator"
```

## BM (Static indexed bitmap)

`.BM` is Harvester's simplest custom image format: a small fixed header followed by raw 8-bit indexed pixels. It is used for UI panels, portraits, inventory art, help screens, and other non-animated images.

### Summary

| Property | Value |
| --- | --- |
| Format role | Raw 8-bit indexed bitmap |
| Confirmed consumers | `spawn_bitmap_entity_from_resource`, `reload_bitmap_entity_pixels_from_resource`, menu/help/dialogue overlay loaders |
| Compression | None confirmed |
| Pixel format | 1 byte per pixel, palette-indexed |

### File layout

| Offset | Size | Type | Description |
| --- | --- | --- | --- |
| `0x00` | `0x04` | uint32le | `width` |
| `0x04` | `0x04` | uint32le | `height` |
| `0x08` | `0x04` | uint32le | Unused/reserved in current analysis. Both native and ScummVM loaders ignore it; sampled `MOUSHELP.BM` stores zero here. |
| `0x0c` | `width * height` | bytes | Raw indexed pixel payload, row-major |

### Notes

- The native binary reuses the same `RawBitmap`/`BitmapBuffer` shape in memory: `{ width, height, pixels }`.
- The format itself does not encode transparency, but many callers render it with palette index `0` treated as transparent.
- Compared with many later adventure engines, this is an aggressively direct format: no chunking, no palette sidecar inside the file, and no per-row metadata.

## PAL (Standalone palette)

Harvester's `.PAL` files are raw palette payloads with no header. The native palette upload helpers and ScummVM `Art::loadPalette` both treat them as 256 RGB triplets.

### Summary

| Property | Value |
| --- | --- |
| Format role | Standalone 256-color palette resource |
| Confirmed consumers | Room setup, menus, help screens, wait overlay, town map |
| Header | None |
| Payload size | 768 bytes (`256 * 3`) |

### File layout

| Offset / range | Size | Description |
| --- | --- | --- |
| `0x000..0x2ff` | 768 bytes | 256 consecutive `(R, G, B)` triplets stored as 8-bit channel values |

### Notes

- Native `upload_palette_to_vga` forces palette index `0` to black at upload time, applies brightness scaling, and then shifts the stored 8-bit channels down to VGA's `0..63` DAC range.
- The sampled `INVHELP.PAL` contains full-range byte values up to `0xff`, which matches the native analysis that `.PAL` is stored as 8-bit RGB, not pre-divided 6-bit VGA values.
- This makes `.PAL` a nice contrast with FST's embedded movie palettes, which are stored in 6-bit VGA form inside each frame payload.


```bash
python3 -m pip install pillow
python3 -c 'from PIL import Image; import sys,struct; d=open(sys.argv[1],"rb").read(); w,h=struct.unpack_from("<II",d,0); i=Image.frombytes("P",(w,h),d[12:12+w*h]); p=open(sys.argv[2],"rb").read()[:768] if len(sys.argv)>2 else bytes(c for n in range(256) for c in (n,n,n)); i.putpalette(p); i.show()' GRAPHIC/OTHER/INVHELP.BM GRAPHIC/PAL/INVHELP.PAL
```

## CFT (Bitmap font)

`.CFT` packages a bitmap font as a fixed metrics header plus one raw 8-bit atlas image. The native `load_font_resource` and the ScummVM `Text::loadFont` / `HarvesterCftFont` code agree on the basic structure.

### Summary

| Property | Value |
| --- | --- |
| Format role | Bitmap font resource with metrics tables and atlas |
| Glyph count | 256 slots |
| Rendering model | Each glyph is a horizontal slice out of one shared atlas bitmap |
| Confirmed consumers | Menu text, room labels, dialogue, save/load UI, text-entry widgets |

### File layout

| Offset | Size | Type | Description |
| --- | --- | --- | --- |
| `0x000` | `0x40` | char[64] | NUL-terminated font name (`HARVFONT` in the sampled file) |
| `0x040` | `0x02` | uint16le | Font height |
| `0x042` | `0x200` | uint16le[256] | Glyph start-X table |
| `0x242` | `0x200` | uint16le[256] | Glyph width table |
| `0x442` | `0x02` | uint16le | Space width |
| `0x444` | `0x04` | unknown | Unused/reserved in current analysis |
| `0x448` | `0x04` | uint32le | Atlas width |
| `0x44c` | `0x04` | uint32le | Atlas height |
| `0x450` | `0x04` | unknown | Unused/reserved in current analysis; sampled `HARVFONT.CFT` stores zero here |
| `0x454` | `atlasWidth * atlasHeight` | bytes | Raw atlas pixels, row-major, 8-bit indexed |

### Notes

- The renderer derives each glyph by slicing `width` pixels from `x` inside the shared atlas. There is no per-glyph bitmap chunking.
- `drawHeight` is effectively `atlasHeight - 1` in the current font renderer, which matches the native behavior of treating the last row as non-drawing padding.
- The font renderer treats both `' '` and `'_'` as space-width characters. That ties neatly back to the script/text resources, where underscore-heavy identifiers and UI labels coexist with visible text.

```bash
python3 -c 'from PIL import Image; import sys,struct; d=open(sys.argv[1],"rb").read(); aw,ah=struct.unpack_from("<II",d,0x448); h=max(1,min(struct.unpack_from("<H",d,0x40)[0] or ah-1,ah-1)); sw=struct.unpack_from("<H",d,0x442)[0] or 1; s=[struct.unpack_from("<H",d,0x42+i*2)[0] for i in range(256)]; w=[struct.unpack_from("<H",d,0x242+i*2)[0] for i in range(256)]; a=Image.frombytes("L",(aw,ah),d[0x454:0x454+aw*ah]); t=sys.argv[2]; W=sum(sw if ord(c)>255 or c in " _" or w[ord(c)]<=0 else w[ord(c)] for c in t); i=Image.new("L",(max(1,W),h),0); x=0; exec("for c in t:\n o=ord(c)\n gw=sw if o>255 or c in \" _\" or w[o]<=0 or s[o]>=aw else w[o]\n if gw and o<=255 and c not in \" _\" and w[o]>0 and s[o]<aw: i.paste(255,(x,0,x+gw,h),a.crop((s[o],0,s[o]+gw,h)))\n x+=gw"); i.resize((max(1,i.width*4),max(1,i.height*4)),Image.NEAREST).show()' "GRAPHIC/FONT/HARVFNT2.CFT" "Hello World"
```

## ABM (Animated bitmap / sprite strip)

`.ABM` is Harvester's main custom sprite/animation format. It backs cursor art, actor sprites, room animations, combat entities, and wait-overlay animation frames.

### Summary

| Property | Value |
| --- | --- |
| Format role | Multi-frame indexed animation resource |
| Confirmed consumers | `attach_abm_resource_to_entity`, `spawn_abm_entity_from_resource`, startup art loaders, room animation runtime |
| Pixel format | 8-bit indexed pixels |
| Compression | Optional per-frame RLE-like stream |

### File header

| Offset | Size | Type | Description |
| --- | --- | --- | --- |
| `0x00` | `0x04` | uint32le | `frame_count` |
| `0x04` | `0x04` | uint32le | Native runtime uses this value to size the temporary decoded-frame buffer before adding `0x10` bytes of slack. The current ScummVM loader does not otherwise interpret it. Sampled `BLOOD.ABM` stores `0x888` here. |
| `0x08` | variable | sequence | First frame record begins here |

### Per-frame record

| Offset within frame | Size | Type | Description |
| --- | --- | --- | --- |
| `0x00` | `0x04` | int32le | `x_offset` |
| `0x04` | `0x04` | int32le | `y_offset` |
| `0x08` | `0x04` | uint32le | `width` |
| `0x0c` | `0x04` | uint32le | `height` |
| `0x10` | `0x01` | byte | `compressed_flag` |
| `0x11` | `0x04` | uint32le | `encoded_size` |
| `0x15` | `encoded_size` | bytes | Encoded or raw pixel payload for this frame |

### Compressed frame stream

| Control byte form | Meaning |
| --- | --- |
| `0x00..0x7f` | Copy the next `control` bytes literally |
| `0x80..0xff` | Read one byte and repeat it `control & 0x7f` times |

### Notes

- Each frame decodes to exactly `width * height` bytes of indexed pixels.
- The sampled `BLOOD.ABM` starts with 5 frames; its first frame is offset `(4, 0)`, size `38 x 50`, and marked compressed.
- ABM is central to Harvester's scene graph. The same format scales from tiny cursor sprites up to actor and monster visual state banks, which makes it a strong candidate for a blog-post "one format, many subsystems" section.

```bash
python3 -c $'from PIL import Image\nimport sys,struct,tempfile,pathlib,webbrowser\n\ndef dec(s,n):\n i=0; o=bytearray()\n while i<len(s) and len(o)<n:\n  k=s[i]; i+=1\n  if k<128:\n   c=min(k,len(s)-i,n-len(o)); o+=s[i:i+c]; i+=c\n  elif i<len(s):\n   o.extend([s[i]]*min(k&127,n-len(o))); i+=1\n return bytes(o)\n\nd=open(sys.argv[1],"rb").read()\npal=open(sys.argv[2],"rb").read()[:768] if len(sys.argv)>2 else bytes(c for n in range(256) for c in (n,n,n))\nfc=struct.unpack_from("<I",d,0)[0]; off=8; F=[]; minx=miny=10**9; maxx=maxy=-10**9\nfor _ in range(fc):\n x,y,w,h=struct.unpack_from("<iiii",d,off); c=d[off+16]; n=struct.unpack_from("<I",d,off+17)[0]; s=d[off+25:off+25+n]; off+=25+n; p=s[:w*h] if not c else dec(s,w*h); F.append((x,y,w,h,p)); minx=min(minx,x); miny=min(miny,y); maxx=max(maxx,x+w); maxy=max(maxy,y+h)\nminx=min(0,minx); miny=min(0,miny); W=maxx-minx; H=maxy-miny; G=[]\nfor x,y,w,h,p in F:\n src=Image.frombytes("P",(w,h),p); src.putpalette(pal); m=src.point(lambda v:0 if v==0 else 255,"L"); fr=Image.new("RGBA",(W,H),(0,0,0,0)); fr.paste(src.convert("RGBA"),(x-minx,y-miny),m); G.append(fr)\nf=tempfile.NamedTemporaryFile(suffix=".gif",delete=False).name; G[0].save(f,save_all=True,append_images=G[1:],duration=100,loop=0,disposal=2,transparency=0); webbrowser.open(pathlib.Path(f).as_uri())' "BLOOD.ABM" "WAIT.PAL"
```

## CMP (FCMP-compressed audio)

`.CMP` is Harvester's custom compressed audio wrapper. The sampled files begin with `FCMP`, and the native `load_sound_sample` / `load_dialogue_voice_sample` code plus the ScummVM `decodeHarvesterFcmp` path all treat that payload as IMA-ADPCM-like compressed audio.

### Summary

| Property | Value |
| --- | --- |
| Format role | Compressed audio for music, dialogue voice, and sound effects |
| Confirmed magic | `FCMP` |
| Confirmed codec | IMA-ADPCM-style nibble stream using the standard step/index tables recovered in the binary |
| Supported output depths | 8-bit and 16-bit PCM |

### File layout

| Offset | Size | Type | Description |
| --- | --- | --- | --- |
| `0x00` | `0x04` | ASCII | `FCMP` magic |
| `0x04` | `0x04` | uint32le | Compressed payload size |
| `0x08` | `0x04` | uint32le | Sample rate in Hz |
| `0x0c` | `0x02` | uint16le | Output bits per sample (`8` or `16`) |
| `0x0e` | variable | bytes | ADPCM payload |

### Notes

- Sampled `MENACE.CMP` starts with `FCMP`, a payload size of `0x0db48b`, sample rate `22050`, and `16` bits per sample.
- The loader family is tolerant: some call paths accept either `FCMP` or raw `WAVE` data. The `.CMP` extension itself points to the compressed path, but the runtime checks the actual file signature before decoding.
- From an architectural perspective, this is the audio-side equivalent of Harvester's XFILE abstraction: one wrapper format reused across music, speech, and effects.

## FST (Cutscene / streamed animation)

`.FST` is Harvester's custom streamed movie format. It combines a file header, a compact per-frame index table, block-coded video payloads, optional per-frame palettes, and per-frame audio chunks.

### Summary

| Property | Value |
| --- | --- |
| Format role | Cutscene / transition movie format |
| Confirmed magic | `FST2` (`0x32545346` on disk, little-endian) |
| Video model | 8-bit indexed frames decoded in `4 x 4` blocks |
| Audio model | Per-frame audio chunks described by the frame index table |
| Confirmed consumers | `run_fst_sequence_player`, `play_fst_sequence`, startup intro path, scripted room transitions |

### File header

| Offset | Size | Type | Description |
| --- | --- | --- | --- |
| `0x00` | `0x04` | ASCII | `FST2` magic |
| `0x04` | `0x04` | uint32le | Frame width |
| `0x08` | `0x04` | uint32le | Frame height |
| `0x0c` | `0x04` | uint32le | `max_frame_size` |
| `0x10` | `0x04` | uint32le | `frame_count` |
| `0x14` | `0x04` | uint32le | `frame_rate` |
| `0x18` | `0x04` | uint32le | `sample_rate` |
| `0x1c` | `0x04` | uint32le | `bits_per_sample` |
| `0x20` | `frame_count * 6` | array | `FstFrameIndexEntry[frame_count]` |

### Frame index entry

| Offset within entry | Size | Type | Description |
| --- | --- | --- | --- |
| `0x00` | `0x04` | uint32le | `video_size` |
| `0x04` | `0x02` | uint16le | `audio_size` |

### Video payload for one frame

| Sequence element | Description |
| --- | --- |
| `uint16 bit_count` | Number of bits in the control bitstream |
| `((bit_count >> 3) + 1)` bytes | Packed control-bit stream |
| Optional `256 * 3` bytes | Present only when the first control bit is set; this is a VGA-style 6-bit palette block, not a `.PAL` resource block |
| Block payload stream | One record per changed `4 x 4` tile across the frame |

### Block coding

| Control bits / payload | Meaning |
| --- | --- |
| Changed-bit `0` | Leave the existing `4 x 4` block unchanged |
| Changed-bit `1`, mode-bit `0`, 16-byte payload | Literal `4 x 4` pixel block |
| Changed-bit `1`, mode-bit `1`, 4-byte payload | Two colors plus a 16-bit mask describing a `4 x 4` block |

### Notes

- Sampled `VIRGLOGO.FST` is `320 x 200`, stores `131` frames, plays at `15` fps, and carries `22050 Hz` / `16-bit` audio.
- The native and ScummVM players both treat FST as a streaming format: read one frame, queue its audio, decode its video, and advance without loading the whole movie at once.
- `HARVEST.SCR` uses `GOFLIC`/`GODEATHFLIC` opcodes to trigger `.FST` files. That mismatch between opcode names and file extension is a strong architectural clue that FST replaced or wrapped an older movie concept without rewriting the script vocabulary.
- The censorship path also shows how self-contained the format is: FST can carry its own palette updates frame-by-frame, while the player temporarily swaps in an external `CENSORED.PCX` overlay when gore is disabled.

## PCX (Standard indexed still image)

Harvester does use standard `.PCX` files in at least one confirmed place: the censorship overlay shown during certain FST sequences when gore is disabled.

### Summary

| Property | Value |
| --- | --- |
| Format role | Standard 8-bit indexed PCX still image |
| Confirmed consumer | Native `load_pcx_bitmap`; ScummVM FST censorship overlay loader |
| Encoding | PCX RLE |
| Palette | Trailing 256-color palette block when present |

### Confirmed header fields

| Offset | Size | Type | Description |
| --- | --- | --- | --- |
| `0x00` | `0x01` | byte | Manufacturer (`0x0a` in sampled `CENSORED.PCX`) |
| `0x01` | `0x01` | byte | Version (`0x05` in the sample) |
| `0x02` | `0x01` | byte | Encoding (`0x01` = RLE) |
| `0x03` | `0x01` | byte | Bits per pixel per plane (`0x08` in the sample) |
| `0x04` | `0x02` | uint16le | `xMin` |
| `0x06` | `0x02` | uint16le | `yMin` |
| `0x08` | `0x02` | uint16le | `xMax` |
| `0x0a` | `0x02` | uint16le | `yMax` |
| `0x41` | `0x01` | byte | Color planes (`0x01` in the sample) |
| `0x42` | `0x02` | uint16le | Bytes per line (`320` in the sample) |
| `0x80` | variable | bytes | RLE-compressed image data |
| `EOF - 769` | `0x01` | byte | Standard palette marker `0x0c` in the sampled file |
| `EOF - 768` | `0x300` | bytes | 256-color palette block when present |

### Notes

- Sampled `CENSORED.PCX` is a textbook single-plane `320 x 200` PCX, which is why the ScummVM port can use the generic PCX decoder for it.
- The native loader contains two Harvester-specific wrinkles: it trims the optional `0x0c` palette marker by jumping straight to the last `0x300` bytes, and when the logical width from `xMax` is one pixel smaller than `bytesPerLine`, it trims the padded stride byte after decode.
- This is another useful contrast point for a blog post: Harvester mostly ships custom image formats (`BM`, `ABM`, `FST`), but it still keeps a standards-based escape hatch where that was convenient.

## FLC (Standard Autodesk FLIC animation)

Harvester does ship `.FLC` files, and the sampled files match the standard Autodesk FLIC/FLC header rather than a Harvester-specific wrapper. The current Harvester engine work has not yet recovered the exact native Harvester-side call path that consumes them, so this section stays limited to what is directly supported by sample bytes, binary strings, and the generic ScummVM FLIC decoder.

### Summary

| Property | Value |
| --- | --- |
| Format role | Standard 8-bit FLIC/FLC animation files bundled with Harvester |
| Evidence | Sample files such as `GRAPHIC/ROOMANIM/HARVPNTR.FLC` and `GRAPHIC/FST/CHESMOV1.FLC`; native strings `Could not load flic.` and `flic.cpp`; hardcoded `.flc` paths in the binary |
| Harvester-specific loader status | Not yet fully recovered in current Harvester analysis |
| Decoder in repo | Generic ScummVM `video/flic_decoder.cpp` |

### Confirmed standard header fields

| Offset | Size | Type | Description |
| --- | --- | --- | --- |
| `0x00` | `0x04` | uint32le | File size |
| `0x04` | `0x02` | uint16le | Magic `0xaf12` |
| `0x06` | `0x02` | uint16le | Frame count |
| `0x08` | `0x02` | uint16le | Width |
| `0x0a` | `0x02` | uint16le | Height |
| `0x0c` | `0x02` | uint16le | Color depth (`8` in the sampled file) |
| `0x0e` | `0x02` | uint16le | Flags |
| `0x10` | `0x04` | uint32le | Frame delay in milliseconds |
| `0x50` | `0x04` | uint32le | Offset of frame 1 |
| `0x54` | `0x04` | uint32le | Offset of frame 2 |
| `0x80` | variable | bytes | First frame/chunk stream begins here in the generic decoder |

### Notes

- Sampled `HARVPNTR.FLC` starts with a valid FLC header: size `0x214a`, magic `0xaf12`, `10` frames, `26 x 26`, and `8`-bit color.
- The decoded `HARVEST.SCR` sample does not reference `.FLC` files directly; its `GOFLIC` opcodes currently point to `.FST` files instead. That suggests `.FLC` belongs to a parallel or older asset path rather than the main shipping story-transition pipeline. This last point is an inference from the sampled script and should stay provisional.
- Taken together, the resource set shows three animation strata: lightweight sprite ABMs, streamed FST movies, and a residual standard FLC layer. That is a strong narrative hook for explaining how mixed-tool resource pipelines often survive into shipped games.

```
ffplay -loop -1 "GRAPHIC/ROOMANIM/HARVPNTR.FLC"
ffmpeg -i "GRAPHIC/ROOMANIM/HARVPNTR.FLC" out.gif
```
