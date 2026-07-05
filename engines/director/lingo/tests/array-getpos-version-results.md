# Director `getOne` / `getPos` — ARRAY/PARRAY comparison behaviour

How `getOne(list, value)` and `getPos(list, value)` match a value that is itself
a list or property list, verified across real Director 4 / 5 / 6 / 7.

## Behaviour

- **Director 4 & 5** — match by **object identity**: a freshly built, value-equal
  list never matches; only the *same* list object matches.
- **Director 6 & 7** — match by **contents** (element-wise, recursive; strings
  stay case-sensitive): a value-equal list matches even when freshly built.

The switch happens **between Director 5 and 6**, matching ScummVM's
`LC::eqDataStrict` threshold `getVersion() >= 600`
(`engines/director/lingo/lingo-code.cpp`).

## Results matrix

All 9 tests confirmed on real Director 4/5/6/7 (2026-07-04) — identical to
ScummVM's emulation at every point.

| #  | Test | D4 | D5 | D6 | D7 |
|----|------|:--:|:--:|:--:|:--:|
| A1 | `getPos([[13,14],[20,5],[7,7]], [20,5])` | 0 | 0 | 2 | 2 |
| A2 | `getOne([#a,#b,[1,2,3]], [1,2,3])` | 0 | 0 | 3 | 3 |
| A3 | `getPos([#x:[1,2],#y:[3,4]], [3,4])` — PARRAY | 0 | 0 | 2 | 2 |
| A4 | `getPos([[1,[2,3]],[4,[5,6]]], [4,[5,6]])` — nested | 0 | 0 | 2 | 2 |
| C1 | `getOne([#a,#b,sub], sub)` — same object | 3 | 3 | 3 | 3 |
| C2 | `getPos([5,10,15], 10)` — scalar | 2 | 2 | 2 | 2 |
| C3 | `getPos([[13,14],[20,5],[7,7]], [99,99])` — not found | 0 | 0 | 0 | 0 |
| S1 | `getOne([#a,"urgh","URGH"], "URGH")` — exact case | 3 | 3 | 3 | 3 |
| S2 | `getOne([#a,"urgh"], "URGH")` — case mismatch | 0 | 0 | 0 | 0 |

- **A1–A4** flip from `0` (identity) to a hit (contents) between **D5 → D6** — the version boundary.
- **C1–C3** and **S1–S2** are constant across all versions; they are controls that
  isolate the change to the list-value case (scalars, same-object refs and
  case-sensitive string matching are version-independent).

## Relevance

Confirms the `eqDataStrict` version-dependent fix (branch `xtras&more`, commit
*"Compare ARRAY/PARRAY in getOne/getPos by version-correct rules"*). Example
dependency: TKKG 5's Sokoban does `getOne(mZielListe, [x, y])` with a freshly
computed coordinate under Director 6.02 — it needs contents matching to detect a
box reaching a target entrance.

Test script: [`array-getpos-version-tests.lingo`](array-getpos-version-tests.lingo).
