/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef CRYOMNI3D_ATLANTIS_F3DC_PARSER_H
#define CRYOMNI3D_ATLANTIS_F3DC_PARSER_H

#include "common/array.h"
#include "common/stream.h"
#include "common/str.h"

namespace CryOmni3D {
namespace Atlantis {

// Subformat tag at on-disk file offset 0x10 (= buffer offset 8 after
// the loader strips the 8-byte magic + class id prefix).  Dispatched by
// the original engine's FUN_0044b77c at atlantis.exe.c:44723.
enum F3dcSubFormat {
	kF3dcSubfmtMesh      = 1,  // .S3D mesh topology, .3DC body anim — scene graph
	kF3dcSubfmtMouthAnim = 2,  // .3DA "M" — per-frame full-mesh vertex animation
	kF3dcSubfmtSmallAnim = 4   // .3DA "A" — small / skeleton animation
};

// Loads a Cryo `.S3D` mesh topology file (DIALOG\PERSO\<name>.S3D) and
// exposes its 4 fixed-position relocation roots.  These point into the
// per-character body of the file:
//
//   root[0] (~file 0x180) → scene-graph node table — pairs of uint32
//                            (value, code), terminated by (-1, -1).
//                            Tentatively interpreted as a tree of
//                            triangle pools (see project_f3dc_format).
//   root[1] (~file 0x408) → bone/joint index array (uint32[N]),
//                            followed by float64 bind-pose transforms.
//   root[2] (~file 0x40C) → root[1] + 4 bytes (overlapping slice of
//                            the same bone array, skipping the first).
//   root[3] (~file 0x01C) → sentinel pointing back into the header.
//
// This minimal class only handles the FILE_S3D header and the 4-entry
// relocation table; the triangle-pool walker is the next implementation
// pass.  It serves as a foundation for that work and lets callers query
// "is this mesh known to be supported by the parser" before bothering
// to load it.
class F3dcMesh {
public:
	F3dcMesh();

	// Parse `stream` (caller-owned).  Returns true on success.
	bool loadFromStream(Common::SeekableReadStream &stream);

	void reset();

	bool isLoaded() const { return _loaded; }

	// Field at file 0x14, present in every .S3D shipped so far.  Always
	// equals the matching `.3DA` mouth frame count (14 for the standard
	// characters), so this is probably the per-mesh max frame index.
	uint headerCountA() const { return _countA; }

	// 4-entry relocation table from file 0x20..0x2F (post-strip of the
	// 8-byte magic these stay at the same file offsets).  Returned as
	// FILE offsets, not in-memory pointers — useful for follow-up reads.
	void getRootFileOffsets(uint32 out[4]) const;

	// Camera projection transforms (CAM2..CAM5) extracted from the tail
	// of the .S3D file.  Each cam record contains 8 float64 LE values
	// followed by a 4-byte ASCII tag "CAMn" and trailing zero padding;
	// the doubles are the transform components the original engine uses
	// to project per-frame .3DA M vertices to screen space.
	//
	// Exact interpretation of the 8 doubles is TBD — candidates (tested
	// in playSingleConLine) include 6-DOF + FOV pair, quaternion +
	// translation + FOV, and 2D affine projection.  Returns false when
	// camNum is out of [2..5] or the cam's record wasn't found.
	bool getCamTransform(int camNum, double out[8]) const;

private:
	bool   _loaded;
	uint32 _countA;
	uint32 _rootOff[4];

	// Cam transforms, indexed by camNum-2 (so [0] = CAM2, [3] = CAM5).
	bool   _camPresent[4];
	double _camTransform[4][8];
};

// Loads a Cryo F3DC ".3DA M" mouth vertex animation file
// (DIALOG\PERSO\<base>M.3DA).  After loadFromStream() the per-frame
// vertex positions are accessible via getVertex() / frameVertices().
//
// On-disk format (verified across all 19 mouth files in BIGCD1.BIG —
// see project_f3dc_format memory entry for full derivation):
//
//   offset   bytes  field
//   0x00     4      magic "F3DC"
//   0x04     4      class id 0x000001C2
//   0x08     8      zeros
//   0x10     4      subformat (must be 2 for this parser)
//   0x14     8      zeros
//   0x1C     4      N = frame count (typ. 14, ANNAAN1M=11, FELIAN1M=13)
//   0x20     4*N    frame offset table; frame i body lives at
//                   file offset (28 + table[i])
//   ...      ...    N frame bodies, each (verts/frame) × (3 × int32 LE)
//
// Frame 0 is the bind / rest pose, always physically first in the file at
// offset 0x58.  Frames 1..N-1 are animation poses stored in REVERSE order
// after the bind pose (N-1 first, then N-2, ..., 1 last).
//
// Vertex coordinates are signed integers in the Cryo authoring space:
// X is bilaterally symmetric around 0 (face left-right axis), Y is the
// up-down axis, Z is the forward-back axis.  Units are sub-pixel
// fixed-point — the exact divisor is determined at rasterise time by
// the camera projection.
class F3dcMouthAnim {
public:
	F3dcMouthAnim();

	// Parse `stream` (caller-owned).  Returns true on success.  On any
	// validation failure the object is left in a reset state and the
	// getters return zero.
	bool loadFromStream(Common::SeekableReadStream &stream);

	// Discard all parsed data; the object becomes equivalent to a
	// freshly-constructed instance.
	void reset();

	bool isLoaded()           const { return _frameCount > 0; }
	uint frameCount()         const { return _frameCount; }
	uint verticesPerFrame()   const { return _vertsPerFrame; }

	// Get a single (x, y, z) for vertex `vi` in frame `fi`.  Returns
	// false when either index is out of range and sets all outputs to 0.
	bool getVertex(uint fi, uint vi, int32 &x, int32 &y, int32 &z) const;

	// Returns a pointer to a flat int32 array of length
	// verticesPerFrame() * 3 for the requested frame, laid out as
	// (x0, y0, z0, x1, y1, z1, ...).  Returns nullptr if fi is out of
	// range.  Pointer is valid until reset() or destruction.
	const int32 *frameVertices(uint fi) const;

	// Get the bounding box across every vertex of every frame.
	// When the parser is unloaded all outputs are 0.
	void getBounds(int32 &minX, int32 &minY, int32 &minZ,
	               int32 &maxX, int32 &maxY, int32 &maxZ) const;

private:
	uint _frameCount;
	uint _vertsPerFrame;
	// Flat vertex pool: index = (frame * _vertsPerFrame + vertex) * 3 + axis.
	Common::Array<int32> _data;
};

// Loads a Cryo F3DC ".3DA A" per-bone skeletal animation file
// (DIALOG\PERSO\<base>A.3DA, subformat 4).  See project_f3dc_head_sway
// memory for the architecture: this file drives per-UBB-frame head
// rotation + translation; the original engine consumes it via
// FUN_00468990(0xff, animA + cur_frame_idx, 0) -> FUN_0046853c -> the
// quat-to-matrix routine FUN_0045d128 (atlantis.exe.c:54979).
//
// On-disk format (verified against BADPAN1A.3DA, 7960 bytes):
//
//   0x00..0x03  "F3DC"
//   0x04..0x07  class id 0x000001C2
//   0x10..0x13  subformat = 4
//   0x1C..0x1F  N_raw = bone count (last entry is a sentinel, so real
//               bones = N_raw - 1)
//   0x24..      uint32[N_raw] offset table; bone i header at file
//               offset (28 + table[i])
//
// Per-bone header (at 28 + table[i]):
//
//   +0x00  unused (the prior parser called this "count1"; its value
//          collides with the previous bone's trailing data)
//   +0x04  count_A  (e.g. 31 for BadPan) -- number of curve-A entries
//   +0x08  count_B  (e.g. 30 for BadPan) -- number of curve-B entries
//   +0x0c  ptr_A    file offset to curve-A array; at runtime this is
//                   relocated by FUN_0044b3ec to an absolute pointer
//   +0x10  ptr_B    file offset to curve-B array
//
// Curve A entry (stride 0x14 = 20 bytes, count_A entries from ptr_A):
//   5 int32 fields.  Field interpretation is not fully settled:
//   - From decompile static analysis: looks like a "time" key plus
//     a quaternion (qx, qy, qz, qw).
//   - From file-data dump (BADPAN1A bone[0]): the field at +0x08
//     monotonically counts 1, 2, 3, ..., count_A-2 across the body
//     of the array (with the first 2 entries holding header-overlay
//     garbage).  The other fields trace smooth sinusoidal curves.
//   Until we have dynamic capture to settle the semantics, the parser
//   exposes all 5 fields as raw int32 and lets callers interpret.
//
// Curve B entry (stride 0x10 = 16 bytes, count_B entries from ptr_B):
//   4 int32 fields.  Similar story -- looks like a "time" key plus a
//   3-component translation (tx, ty, tz), but rec[0]/rec[1] in each
//   bone overlap the header and produce garbage.  Real keyframes
//   start at index 2 of each array.
//
// Layout note: ptr_A typically equals (bone_header_offset - 8), so
// the bone header overlaps the first 8 bytes of curve-A's first
// entry.  The header's count_A / count_B / ptr_A / ptr_B u32s sit
// inside what would be rec[1]'s last 16 bytes.  This is intentional
// in the file format -- the original engine seems to either skip the
// first 2 entries or rely on their "time" values being outside the
// queried range.
class F3dcSmallAnim {
public:
	F3dcSmallAnim();

	// Parse `stream` (caller-owned).  Returns true on success.
	bool loadFromStream(Common::SeekableReadStream &stream);

	void reset();

	bool isLoaded()  const { return _boneCount > 0; }
	uint boneCount() const { return _boneCount; }

	// Number of curve-A / curve-B entries for bone bi (raw — includes
	// the two header-overlap entries at index 0 and 1).
	uint curveACount(uint bi) const;
	uint curveBCount(uint bi) const;

	// Fetch one curve-A entry (5 int32) or curve-B entry (4 int32).
	// Returns false if out of range; outputs are then zero.  The
	// fields are returned as raw int32; field interpretation is the
	// caller's responsibility (see class-level comment).
	bool getCurveA(uint bi, uint i, int32 out[5]) const;
	bool getCurveB(uint bi, uint i, int32 out[4]) const;

private:
	uint _boneCount;
	// Per-bone curve A: flat int32 array of size curveACount(bi) * 5.
	Common::Array<Common::Array<int32> > _curveA;
	// Per-bone curve B: flat int32 array of size curveBCount(bi) * 4.
	Common::Array<Common::Array<int32> > _curveB;
};

// Loads a Cryo `.3DM` texture-map file (DIALOG\MAPS\<base>.3DM).  The dialog
// mouth mesh is texture-mapped: the original engine samples a per-character
// 256x256 face texture whose bytes are *colour indices*, then resolves each
// (colour index, light level) pair through a shade table to a final R5G6B5
// colour — so the `.3DM` is fully self-contained, no external palette.
//
// On-disk format (verified across the 19 shipped .3DM files, all 98332 bytes):
//   0x0000  8        "F3DC" + class id 0x1C2  (stripped by the engine loader)
//   0x0008  0x14     header
//   0x001C  0x8000   shade table: 32 light levels x 256 colours, each a
//                    4-byte record `00 00 <R5G6B5 u16>`.  Bit 15 is the
//                    high R bit, NOT a chroma flag — confirmed by
//                    matching the .3DM lvl-15 row against RHEA.BMP's
//                    palette (119 bit-15-set entries are all legitimate
//                    face colours, deltas ≤ ±16 RGB).
//   0x801C  0x10000  the 256x256 face texture (one colour index per texel)
class F3dcTextureMap {
public:
	static const uint kSize        = 256;   // texture is 256 x 256
	static const uint kShadeLevels = 32;    // shade-table light levels

	F3dcTextureMap();

	// Parse `stream` (caller-owned).  Returns true on success.
	bool loadFromStream(Common::SeekableReadStream &stream);

	void reset();

	bool isLoaded() const { return _loaded; }

	// Colour index of the texel at (u, v); u and v are wrapped to [0, 255].
	byte texel(int u, int v) const {
		return _tex[((uint)v & 255) * kSize + ((uint)u & 255)];
	}

	// Resolve a texel colour index at light level `shade` (0..31) to RGB
	// through the `.3DM` shade table.
	void shadeRGB(byte index, uint shade, byte &r, byte &g, byte &b) const;

private:
	bool _loaded;
	Common::Array<byte>   _tex;     // kSize*kSize colour indices
	Common::Array<uint16> _shade;   // kShadeLevels*256 R5G6B5 entries
};

// Extracts the dialog mouth MESH (bind-pose vertices + triangle topology)
// from a Cryo `.3DC` body file (DIALOG\PERSO\<base>.3DC).
//
// The format was reverse-engineered by a hardware-breakpoint trace of the
// running engine (see the project memory notes).  The `.3DC` contains an
// "fvi" scene-graph node whose pools hold the mouth mesh:
//
//   fvi node      : 4-byte ASCII magic "fvi\0"
//     +0x7C  uint32  vertex count
//     +0x80  uint32  vertex-index base reference (used to decode tris)
//     +0x94  uint32  triangle count
//     +0xD4  ...     vertex pool — 40-byte records, XYZ int32 LE at +4
//
//   triangle struct array : 64-byte records, located by scanning for a
//     run of `triangleCount` structs that decode cleanly.  Per struct:
//     +0x08 / +0x14 / +0x20  uint32  vertex references.
//       vertexIndex = (reference - vertexBaseRef) / 40
//
// Verified against BADPAN1A, GARDAN1A, HEROAN1A, GROAN1A, ANNAAN1A —
// every file yields a closed mesh with zero degenerate triangles.
class F3dcMouthMesh {
public:
	F3dcMouthMesh();

	// Parse `stream` (caller-owned).  Returns true on success.
	bool loadFromStream(Common::SeekableReadStream &stream);

	void reset();

	bool isLoaded()        const { return _loaded; }
	uint vertexCount()     const { return _verts.size() / 3; }
	uint triangleCount()   const { return _tris.size() / 3; }

	// Vertex `vi` as (x, y, z).  Returns false if out of range.
	bool getVertex(uint vi, int32 &x, int32 &y, int32 &z) const;

	// Triangle `ti` as three vertex indices.  Returns false if out of
	// range; the indices are guaranteed < vertexCount() on success.
	bool getTriangle(uint ti, uint &v0, uint &v1, uint &v2) const;

	// Per-character dialog camera offset O.  The engine places the mouth
	// mesh at  P = d[6]*camAnchor + O, where O = -(the fvi node's world
	// position).  That world position is obtained by walking the .3DC
	// scene-graph chain from the fvi mouth node up to the skeleton root,
	// composing each node's local position + rotation:
	//     worldPos += worldRot * localPos
	//     worldRot  = worldRot * transpose(localRot)
	// Verified exact against the guard (GARDAN1A).  Valid only when
	// hasCamOffset() is true.
	bool hasCamOffset() const { return _hasOffset; }
	void getCamOffset(double out[3]) const {
		out[0] = _offsetO[0]; out[1] = _offsetO[1]; out[2] = _offsetO[2];
	}

	// The composed scene-graph chain rotation R_world (row-major).  The
	// full dialog transform is  camera = R_view * (R_world*model - P);
	// the engine bakes this chain rotation into the mouth mesh.  It is
	// identity for most characters but a small rotation for some (e.g.
	// the hero), where it must be applied to keep the mesh aligned.
	void getChainRot(double out[9]) const {
		for (int i = 0; i < 9; i++) out[i] = _chainRot[i];
	}

	// True when per-triangle texture coordinates were decoded (the mesh can
	// be texture-mapped).  When false the caller should fall back to shading.
	bool hasUV() const { return _hasUV; }

	// Texture coordinates for triangle `ti`, corner 0..2.  Returned as
	// 16.16 fixed-point texel coordinates into the 256x256 .3DM texture
	// (corner i pairs with the vertex i of getTriangle()).  Returns false
	// when ti / corner is out of range or no UVs were decoded.
	bool getTriangleUV(uint ti, uint corner, int32 &u, int32 &v) const;

	// Candidate basenames for the per-character `.3DM` texture map, scraped
	// from the `.3DC` header (e.g. "bad10000", "gard", "facer").  The caller
	// resolves these against DIALOG\MAPS\<name>.3DM and uses the first hit.
	const Common::Array<Common::String> &textureMapCandidates() const {
		return _texMapCandidates;
	}

	// Number of animated bones in the .3DC scene graph.  The .3DA-A
	// skeletal track maps bone[bi] -> .3DC reloc root[bi + 1], i.e.
	// root[0] (the character's world "Bass" root) is NOT driven by
	// .3DA-A.  All subsequent reloc roots are animated, including the
	// trailing `fvi` mouth anchor.  Verified across BadPan/Garde:
	// each .3DA-A bone[bi]'s rec[2] (frame 1) translation EXACTLY
	// matches .3DC reloc root[bi + 1]'s rest position.
	uint boneNodeCount() const { return _boneNodeOff.size(); }

	// File offset of the animated scene-graph node corresponding to
	// .3DA-A bone[bi] (i.e. .3DC reloc root[bi + 1]).  The node header
	// starts here: name at +0x00, parent ptr at +0x10, local position
	// at +0x1C, local rotation matrix at +0x28.  Returns 0 when bi is
	// out of range.
	uint32 getBoneNodeOffset(uint bi) const {
		return (bi < _boneNodeOff.size()) ? _boneNodeOff[bi] : 0;
	}

	// Per-UBB-frame cam offset O and chain rotation R_world, with the
	// .3DA-A skeletal animation applied.  Conceptually: take the .3DC
	// rest-pose, OVERWRITE each animated bone's local rotation matrix
	// (+0x28) and local position (+0x1C) with the values interpolated
	// from `anim` at UBB frame `frame`, then re-run the standard chain
	// walk from fvi to root.  The result drifts the mouth-mesh anchor
	// to match the head's per-frame motion in the UBB video.
	//
	// `frame` is the UBB frame counter; the routine maps it to the
	// keyframe index modulo the animation length.  Returns false when
	// the mesh wasn't loaded with bytes retained, or when `anim` has
	// no usable data.  On false, O and rot are unchanged.
	bool computeCamOffsetForFrame(uint frame, const F3dcSmallAnim &anim,
	                              double O[3], double rot[9]) const;

private:
	bool _loaded;
	bool _hasOffset;
	bool _hasUV;
	double _offsetO[3];
	double _chainRot[9];
	// Flat vertex pool: index = vertex*3 + axis.
	Common::Array<int32>  _verts;
	// Flat triangle list: index = triangle*3 + corner.
	Common::Array<uint16> _tris;
	// Per-triangle UVs: index = (triangle*3 + corner)*2 + (0=u, 1=v),
	// 16.16 fixed-point.  Empty when _hasUV is false.
	Common::Array<int32>  _uvs;
	// `.3DM` texture-map basename candidates from the .3DC header.
	Common::Array<Common::String> _texMapCandidates;
	// File offsets of body-part scene-graph nodes (reloc roots 0..N-2;
	// the final root is the fvi mouth anchor and is excluded).
	Common::Array<uint32> _boneNodeOff;
	// Retained copy of the .3DC file bytes (kept post-load so
	// computeCamOffsetForFrame can re-walk the chain with mutated bone
	// transforms).  Empty if !_loaded.
	Common::Array<byte> _bytes;
	// File offset of the fvi scene-graph node (the mouth anchor).
	uint32 _fviOff;
};

} // namespace Atlantis
} // namespace CryOmni3D

#endif // CRYOMNI3D_ATLANTIS_F3DC_PARSER_H
