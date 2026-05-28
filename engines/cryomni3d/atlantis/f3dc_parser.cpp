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

#include "common/debug.h"
#include "common/scummsys.h"
#include "common/textconsole.h"

#include "cryomni3d/cryomni3d.h"
#include "cryomni3d/atlantis/f3dc_parser.h"

namespace CryOmni3D {
namespace Atlantis {

// "F3DC" read as big-endian gives 0x46334443 — the bytes on disk are
// 0x46 0x33 0x44 0x43, so the first byte ('F' = 0x46) is the most
// significant byte of the BE u32.
static const uint32 kF3dcMagic = MKTAG('F', '3', 'D', 'C');

// Fixed-size on-disk header that precedes the per-frame data table.
//   0x00..0x07 = magic + class id
//   0x08..0x1F = subformat + zeros + frame count
//   0x20..     = uint32[frameCount] offset table
static const uint32 kHeaderBytesFixed = 32;

F3dcMouthAnim::F3dcMouthAnim() : _frameCount(0), _vertsPerFrame(0) {}

void F3dcMouthAnim::reset() {
	_frameCount    = 0;
	_vertsPerFrame = 0;
	_data.clear();
}

bool F3dcMouthAnim::loadFromStream(Common::SeekableReadStream &s) {
	reset();

	const int64 fileSize = s.size();
	if (fileSize < (int64)kHeaderBytesFixed) {
		warning("F3dcMouthAnim: file too small (%d bytes, need at least %d)",
		        (int)fileSize, (int)kHeaderBytesFixed);
		return false;
	}

	s.seek(0);
	if (s.readUint32BE() != kF3dcMagic) {
		warning("F3dcMouthAnim: bad F3DC magic");
		return false;
	}

	s.seek(0x10);
	const uint32 subfmt = s.readUint32LE();
	if (subfmt != (uint32)kF3dcSubfmtMouthAnim) {
		warning("F3dcMouthAnim: unsupported subformat %u (need %u)",
		        subfmt, (uint32)kF3dcSubfmtMouthAnim);
		return false;
	}

	s.seek(0x1C);
	const uint32 cnt = s.readUint32LE();
	// Empirically every shipped M file has 11..14 frames; cap at 1024
	// to flag corruption while leaving generous headroom.
	if (cnt == 0 || cnt > 1024) {
		warning("F3dcMouthAnim: implausible frame count %u", cnt);
		return false;
	}

	// Frame offset table — each entry i is a delta:
	//   frame_file_offset[i] = 28 + table[i]
	// Verified for HEROAN1M.3DA in the reverse-engineering pass.
	Common::Array<uint32> table;
	table.resize(cnt);
	s.seek(0x20);
	for (uint i = 0; i < cnt; i++)
		table[i] = s.readUint32LE();
	if (s.err()) {
		warning("F3dcMouthAnim: short read in offset table");
		reset();
		return false;
	}

	// Payload = file size minus fixed header minus table.  All shipped
	// mouth files split it into `cnt` equal frames of bytesPerFrame
	// bytes each, with bytesPerFrame a multiple of 12 (3 × int32 LE per
	// vertex).  Verified across all 19 mouth files in BIGCD1.BIG.
	const uint32 headerBytes  = kHeaderBytesFixed + 4 * cnt;
	if ((uint32)fileSize <= headerBytes) {
		warning("F3dcMouthAnim: no payload (%d bytes file, %u header)",
		        (int)fileSize, headerBytes);
		return false;
	}
	const uint32 payloadBytes = (uint32)fileSize - headerBytes;
	if (payloadBytes % cnt != 0) {
		warning("F3dcMouthAnim: payload %u not divisible by frame count %u",
		        payloadBytes, cnt);
		return false;
	}
	const uint32 bytesPerFrame = payloadBytes / cnt;
	if (bytesPerFrame % 12 != 0) {
		warning("F3dcMouthAnim: bytes/frame %u not a multiple of 12",
		        bytesPerFrame);
		return false;
	}

	_frameCount    = cnt;
	_vertsPerFrame = bytesPerFrame / 12;
	_data.resize((uint)_frameCount * _vertsPerFrame * 3);

	int32 *dst = _data.begin();
	for (uint fi = 0; fi < _frameCount; fi++) {
		const uint32 frameOff = 28 + table[fi];
		if (frameOff + bytesPerFrame > (uint32)fileSize) {
			warning("F3dcMouthAnim: frame[%u] OOB (off=0x%x size=%u)",
			        fi, frameOff, bytesPerFrame);
			reset();
			return false;
		}
		s.seek(frameOff);
		for (uint vi = 0; vi < _vertsPerFrame; vi++) {
			*dst++ = (int32)s.readUint32LE();
			*dst++ = (int32)s.readUint32LE();
			*dst++ = (int32)s.readUint32LE();
		}
	}
	if (s.err()) {
		warning("F3dcMouthAnim: short read in frame bodies");
		reset();
		return false;
	}

	debugC(1, kDebugMesh, "F3dcMouthAnim: loaded %u frames × %u verts (bind pose at frame 0)",
	      _frameCount, _vertsPerFrame);
	return true;
}

bool F3dcMouthAnim::getVertex(uint fi, uint vi, int32 &x, int32 &y, int32 &z) const {
	if (fi >= _frameCount || vi >= _vertsPerFrame) {
		x = y = z = 0;
		return false;
	}
	const int32 *src = _data.begin() + (fi * _vertsPerFrame + vi) * 3;
	x = src[0];
	y = src[1];
	z = src[2];
	return true;
}

const int32 *F3dcMouthAnim::frameVertices(uint fi) const {
	if (fi >= _frameCount)
		return nullptr;
	return _data.begin() + fi * _vertsPerFrame * 3;
}

void F3dcMouthAnim::getBounds(int32 &minX, int32 &minY, int32 &minZ,
                              int32 &maxX, int32 &maxY, int32 &maxZ) const {
	if (_data.empty()) {
		minX = minY = minZ = maxX = maxY = maxZ = 0;
		return;
	}
	minX = minY = minZ =  0x7FFFFFFF;
	maxX = maxY = maxZ = -0x7FFFFFFF;
	for (uint i = 0; i < _data.size(); i += 3) {
		const int32 x = _data[i];
		const int32 y = _data[i + 1];
		const int32 z = _data[i + 2];
		if (x < minX) minX = x;
		if (x > maxX) maxX = x;
		if (y < minY) minY = y;
		if (y > maxY) maxY = y;
		if (z < minZ) minZ = z;
		if (z > maxZ) maxZ = z;
	}
}

// ---------------------------------------------------------------------------
// F3dcSmallAnim — `.3DA A` per-bone skeletal animation (subfmt 4).
// ---------------------------------------------------------------------------
//
// See class-level comment in f3dc_parser.h and the project_f3dc_head_sway
// memory for the architecture overview.  The parser only decodes the
// file layout; it does NOT interpret the field semantics inside each
// keyframe (the time-axis convention is still uncertain pending dynamic
// capture).

F3dcSmallAnim::F3dcSmallAnim() : _boneCount(0) {}

void F3dcSmallAnim::reset() {
	_boneCount = 0;
	_curveA.clear();
	_curveB.clear();
}

bool F3dcSmallAnim::loadFromStream(Common::SeekableReadStream &s) {
	reset();

	const int64 fileSize = s.size();
	if (fileSize < 0x40) {
		warning("F3dcSmallAnim: file too small (%d bytes)", (int)fileSize);
		return false;
	}

	s.seek(0);
	if (s.readUint32BE() != kF3dcMagic) {
		warning("F3dcSmallAnim: bad F3DC magic");
		return false;
	}

	s.seek(0x10);
	const uint32 subfmt = s.readUint32LE();
	if (subfmt != (uint32)kF3dcSubfmtSmallAnim) {
		warning("F3dcSmallAnim: unsupported subformat %u (need %u)",
		        subfmt, (uint32)kF3dcSubfmtSmallAnim);
		return false;
	}

	s.seek(0x1C);
	const uint32 nRaw = s.readUint32LE();
	if (nRaw == 0 || nRaw > 64) {
		warning("F3dcSmallAnim: implausible bone count %u", nRaw);
		return false;
	}

	// Offset table at +0x24.  The last entry is a sentinel (value
	// 0x1e in every file we've inspected); real bones are 0..nRaw-2.
	Common::Array<uint32> table;
	table.resize(nRaw);
	s.seek(0x24);
	for (uint i = 0; i < nRaw; i++)
		table[i] = s.readUint32LE();
	if (s.err()) {
		warning("F3dcSmallAnim: short read in offset table");
		reset();
		return false;
	}

	const uint nBones = nRaw - 1;
	_curveA.resize(nBones);
	_curveB.resize(nBones);

	for (uint bi = 0; bi < nBones; bi++) {
		const uint32 hdrOff = 28 + table[bi];
		if (hdrOff + 20 > (uint32)fileSize) {
			warning("F3dcSmallAnim: bone[%u] header OOB (off=0x%x)",
			        bi, hdrOff);
			reset();
			return false;
		}
		s.seek(hdrOff);
		(void)s.readUint32LE();                      // +0x00 unused
		const uint32 countA = s.readUint32LE();       // +0x04
		const uint32 countB = s.readUint32LE();       // +0x08
		// +0x0c and +0x10 are ptr_A / ptr_B in file form (= file_off
		// minus 0x1c).  But x32dbg traces 2026-05-27 confirmed the
		// TRUE curve_A starts at bone_hdr + 0x14 in BOTH the file and
		// the runtime image (the engine doesn't transform the layout,
		// it just relocates the ptr values).  So we read the curves
		// directly off bone_hdr + 0x14 rather than chasing ptr_A.
		(void)s.readUint32LE();                      // +0x0c (file ptr_A, ignored)
		(void)s.readUint32LE();                      // +0x10 (file ptr_B, ignored)

		if (countA == 0 || countA > 256 || countB == 0 || countB > 256) {
			warning("F3dcSmallAnim: bone[%u] implausible counts A=%u B=%u",
			        bi, countA, countB);
			reset();
			return false;
		}
		const uint32 curveAOff = hdrOff + 0x14;
		const uint32 curveBOff = curveAOff + countA * 20;
		if (curveAOff + countA * 20 > (uint32)fileSize ||
		    curveBOff + countB * 16 > (uint32)fileSize) {
			warning("F3dcSmallAnim: bone[%u] curve OOB "
			        "(A_off=0x%x A*20=0x%x B_off=0x%x B*16=0x%x file=0x%x)",
			        bi, curveAOff, countA * 20, curveBOff, countB * 16,
			        (uint32)fileSize);
			reset();
			return false;
		}

		// curve_A keyframes: each is 20 bytes = (frame_idx, qx, qy, qz, qw)
		// at offsets (0, 4, 8, 0xc, 0x10).  Q15 quaternion in fields
		// +4..+0x13.  The engine reads the quaternion starting at +4
		// (skipping frame_idx) and applies via FUN_0045d128.
		_curveA[bi].resize(countA * 5);
		s.seek(curveAOff);
		for (uint i = 0; i < countA; i++) {
			_curveA[bi][i*5 + 0] = (int32)s.readUint32LE(); // frame_idx
			_curveA[bi][i*5 + 1] = (int32)s.readUint32LE(); // qx
			_curveA[bi][i*5 + 2] = (int32)s.readUint32LE(); // qy
			_curveA[bi][i*5 + 3] = (int32)s.readUint32LE(); // qz
			_curveA[bi][i*5 + 4] = (int32)s.readUint32LE(); // qw
		}

		// curve_B keyframes: each is 16 bytes = (frame_idx, tx, ty, tz).
		_curveB[bi].resize(countB * 4);
		s.seek(curveBOff);
		for (uint i = 0; i < countB; i++) {
			_curveB[bi][i*4 + 0] = (int32)s.readUint32LE(); // frame_idx
			_curveB[bi][i*4 + 1] = (int32)s.readUint32LE(); // tx
			_curveB[bi][i*4 + 2] = (int32)s.readUint32LE(); // ty
			_curveB[bi][i*4 + 3] = (int32)s.readUint32LE(); // tz
		}
	}
	if (s.err()) {
		warning("F3dcSmallAnim: short read in bone bodies");
		reset();
		return false;
	}

	_boneCount = nBones;
	debugC(1, kDebugMesh, "F3dcSmallAnim: loaded %u bones (raw count %u in file)",
	      _boneCount, nRaw);
	return true;
}

uint F3dcSmallAnim::curveACount(uint bi) const {
	if (bi >= _boneCount)
		return 0;
	return _curveA[bi].size() / 5;
}

uint F3dcSmallAnim::curveBCount(uint bi) const {
	if (bi >= _boneCount)
		return 0;
	return _curveB[bi].size() / 4;
}

bool F3dcSmallAnim::getCurveA(uint bi, uint i, int32 out[5]) const {
	if (bi >= _boneCount || i >= curveACount(bi)) {
		for (uint k = 0; k < 5; k++) out[k] = 0;
		return false;
	}
	const int32 *p = &_curveA[bi][i * 5];
	for (uint k = 0; k < 5; k++) out[k] = p[k];
	return true;
}

bool F3dcSmallAnim::getCurveB(uint bi, uint i, int32 out[4]) const {
	if (bi >= _boneCount || i >= curveBCount(bi)) {
		for (uint k = 0; k < 4; k++) out[k] = 0;
		return false;
	}
	const int32 *p = &_curveB[bi][i * 4];
	for (uint k = 0; k < 4; k++) out[k] = p[k];
	return true;
}

// ---------------------------------------------------------------------------
// F3dcTextureMap — `.3DM` per-character face texture.
// ---------------------------------------------------------------------------

F3dcTextureMap::F3dcTextureMap() : _loaded(false) {}

void F3dcTextureMap::reset() {
	_loaded = false;
	_tex.clear();
	_shade.clear();
}

bool F3dcTextureMap::loadFromStream(Common::SeekableReadStream &s) {
	reset();

	const int64 fileSize = s.size();
	const uint32 texBytes   = kSize * kSize;             // 0x10000
	const uint32 shadeCount = kShadeLevels * 256;        // 8192 entries
	// magic(8) + header(0x14) + shade table(8192*4) + texture(0x10000).
	const int64 minSize = 8 + 0x14 + (int64)shadeCount * 4 + texBytes;
	if (fileSize < minSize || fileSize > 0x4000000) {
		warning("F3dcTextureMap: implausible file size %d", (int)fileSize);
		return false;
	}

	s.seek(0);
	if (s.readUint32BE() != kF3dcMagic) {
		warning("F3dcTextureMap: bad F3DC magic");
		return false;
	}

	// Shade table at file 0x1C: 32*256 records of `00 00 <RGB555 u16>`.
	_shade.resize(shadeCount);
	s.seek(0x1C);
	for (uint32 i = 0; i < shadeCount; i++) {
		s.readUint16LE();             // the two leading zero bytes
		_shade[i] = s.readUint16LE();
	}
	if (s.err()) {
		warning("F3dcTextureMap: short read on shade table");
		reset();
		return false;
	}

	// The 256x256 face texture is the trailing 0x10000 bytes.
	_tex.resize(texBytes);
	s.seek(fileSize - (int64)texBytes);
	if (s.read(_tex.begin(), texBytes) != texBytes) {
		warning("F3dcTextureMap: short read on texture block");
		reset();
		return false;
	}

	_loaded = true;
	debugC(1, kDebugMesh, "F3dcTextureMap: loaded %ux%u texture + %u-level shade table",
	      kSize, kSize, kShadeLevels);
	return true;
}

void F3dcTextureMap::shadeRGB(byte index, uint shade,
                              byte &r, byte &g, byte &b) const {
	// The .3DM shade-table u16 is R5G6B5 — bit 15 (= high R bit) doubles
	// as the chroma flag; opaque (bit 15 = 0) entries therefore cap R at
	// 4 bits of saturation.  Verified empirically against the original
	// engine 2026-05-24 by patching BAD10000.3DM with known colours and
	// observing the displayed result (see memory `f3dc-colour-format`).
	const uint16 c  = _shade[(shade % kShadeLevels) * 256 + index];
	const byte   r5 = (c >> 11) & 31;       // R: 5 bits (high bit is the chroma flag)
	const byte   g6 = (c >>  5) & 63;       // G: 6 bits
	const byte   b5 =  c        & 31;       // B: 5 bits
	// Bit-fan to 8 bpc.
	r = (byte)((r5 << 3) | (r5 >> 2));
	g = (byte)((g6 << 2) | (g6 >> 4));
	b = (byte)((b5 << 3) | (b5 >> 2));
}

// ---------------------------------------------------------------------------
// F3dcMesh — FILE_S3D scene-graph mesh (foundation only, triangle-pool
// walker TBD).
// ---------------------------------------------------------------------------

// "FILE_S3D" first 4 bytes (BE): 'F','I','L','E' = 0x46494C45.
static const uint32 kS3dMagicHi = MKTAG('F', 'I', 'L', 'E');
// Second 4 bytes: '_','S','3','D' = 0x5F533344.
static const uint32 kS3dMagicLo = MKTAG('_', 'S', '3', 'D');

F3dcMesh::F3dcMesh() : _loaded(false), _countA(0) {
	for (uint i = 0; i < 4; i++) _rootOff[i] = 0;
	for (uint i = 0; i < 4; i++) {
		_camPresent[i] = false;
		for (uint j = 0; j < 8; j++) _camTransform[i][j] = 0.0;
	}
}

void F3dcMesh::reset() {
	_loaded = false;
	_countA = 0;
	for (uint i = 0; i < 4; i++) _rootOff[i] = 0;
	for (uint i = 0; i < 4; i++) {
		_camPresent[i] = false;
		for (uint j = 0; j < 8; j++) _camTransform[i][j] = 0.0;
	}
}

bool F3dcMesh::getCamTransform(int camNum, double out[8]) const {
	if (camNum < 2 || camNum > 5)
		return false;
	const int idx = camNum - 2;
	if (!_camPresent[idx]) {
		for (uint i = 0; i < 8; i++) out[i] = 0.0;
		return false;
	}
	for (uint i = 0; i < 8; i++) out[i] = _camTransform[idx][i];
	return true;
}

bool F3dcMesh::loadFromStream(Common::SeekableReadStream &s) {
	reset();
	const int64 fileSize = s.size();
	// Header (48 bytes) + at minimum some character-specific body.
	if (fileSize < 0x180) {
		warning("F3dcMesh: file too small (%d bytes)", (int)fileSize);
		return false;
	}

	s.seek(0);
	if (s.readUint32BE() != kS3dMagicHi || s.readUint32BE() != kS3dMagicLo) {
		warning("F3dcMesh: bad FILE_S3D magic");
		return false;
	}

	s.seek(0x10);
	const uint32 subfmt = s.readUint32LE();
	if (subfmt != (uint32)kF3dcSubfmtMesh) {
		warning("F3dcMesh: unsupported subformat %u (need %u)",
		        subfmt, (uint32)kF3dcSubfmtMesh);
		return false;
	}

	s.seek(0x14);
	_countA = s.readUint32LE();
	// All shipped meshes have countA = 14.  Cap at 256 to flag corruption.
	if (_countA == 0 || _countA > 256) {
		warning("F3dcMesh: implausible countA %u", _countA);
		reset();
		return false;
	}

	s.seek(0x1C);
	const uint32 relocCount = s.readUint32LE();
	if (relocCount != 4) {
		warning("F3dcMesh: unexpected reloc count %u (need 4)", relocCount);
		reset();
		return false;
	}

	// File-offset form of each root: (28 + raw_delta).  In-memory, the
	// runtime relocates by adding the loaded buffer base; for our
	// disk-driven parser we just store the file offsets so callers can
	// seek directly to the sub-structures.
	s.seek(0x20);
	for (uint i = 0; i < 4; i++) {
		const uint32 delta = s.readUint32LE();
		_rootOff[i] = 28 + delta;
	}
	if (s.err()) {
		warning("F3dcMesh: short read in reloc table");
		reset();
		return false;
	}

	// Scan the file tail for the four CAMn records.  Each tag ("CAM2"
	// through "CAM5") sits at variable file offsets (not 8-aligned, not
	// at a fixed stride — the layout has 2-byte alignment quirks
	// between records).  For each tag found, read the 64 bytes
	// IMMEDIATELY BEFORE the tag as 8 little-endian doubles, which the
	// original engine treats as the projection transform for that cam.
	// Reads directly from the input stream; no buffering or auxiliary
	// allocation needed (.S3D files are tiny — <2 KB).
	{
		const int64 fileBytes = s.size();
		for (int64 i = 64; i + 4 <= fileBytes; i++) {
			if (!s.seek(i)) break;
			byte tag[4];
			if (s.read(tag, 4) != 4) break;
			if (tag[0] != 'C' || tag[1] != 'A' || tag[2] != 'M')
				continue;
			if (tag[3] < '2' || tag[3] > '5')
				continue;
			const uint idx = (uint)(tag[3] - '2');
			if (!s.seek(i - 64))
				continue;
			for (uint j = 0; j < 8; j++)
				_camTransform[idx][j] = s.readDoubleLE();
			if (s.err()) {
				s.clearErr();
				continue;
			}
			_camPresent[idx] = true;
			// Skip past the tag so we don't re-match its bytes.
			i += 3;
		}
		s.clearErr();
	}

	_loaded = true;
	debugC(1, kDebugMesh, "F3dcMesh: loaded (countA=%u, roots: 0x%x 0x%x 0x%x 0x%x; "
	      "cams: %d %d %d %d)",
	      _countA, _rootOff[0], _rootOff[1], _rootOff[2], _rootOff[3],
	      _camPresent[0] ? 1 : 0, _camPresent[1] ? 1 : 0,
	      _camPresent[2] ? 1 : 0, _camPresent[3] ? 1 : 0);
	return true;
}

void F3dcMesh::getRootFileOffsets(uint32 out[4]) const {
	for (uint i = 0; i < 4; i++)
		out[i] = _rootOff[i];
}

// ---------------------------------------------------------------------------
// F3dcMouthMesh — dialog mouth mesh (verts + triangle topology) from a .3DC.
// ---------------------------------------------------------------------------

// Cam-offset chain-walk mode.  Default is `compose` -- ALWAYS compose
// the chain-node rotations into the offset, regardless of fvi+0x28's
// value.  This was confirmed by side-by-side testing 2026-05-27:
// `compose` produces visually-correct meshes for Passant cam3/4/5 +
// BadPan/Garde/Hero/Seth on chapter 1.  The previous `compose-iff-fvi-
// near-identity` classifier flipped to `sum` per-frame when the .3DA-A
// overwrote Passant's fvi+0x28 with a 90 deg quaternion, causing a
// visible mesh jump.
//
// (Passant cam2 still misrenders -- different bug, see project memory.)

// Walk the .3DC scene-graph chain from the fvi mouth node up to the skeleton
// root and compose the node transforms to obtain the fvi node's world
// position.  The dialog camera offset is O = -(that world position).
//
// Each scene-graph node has, relative to its 4..N-byte ASCII name:
//   +0x10  uint32  parent link   (parent node = link + bodySize)
//   +0x1C  int32x3 local position
//   +0x28  int32x9 local rotation, row-major, fixed-point /32768
// The root node has link == 0.  `bodySize` is a per-file constant; it is
// auto-detected as the value that yields a valid terminating node chain.
static bool f3dcComputeCamOffset(const byte *d, uint32 sz, uint32 fvi,
                                 double out[3], double rotOut[9]) {
	#define RDS(off) ((int32)((uint32)d[(off)] | ((uint32)d[(off)+1] << 8) | \
	                  ((uint32)d[(off)+2] << 16) | ((uint32)d[(off)+3] << 24)))
	for (uint32 bs = 0x60; bs <= 0x200; bs++) {
		uint32 chain[16];
		uint32 nNodes = 0;
		uint32 node = fvi;
		bool ok = false;
		for (uint32 hop = 0; hop < 16; hop++) {
			if (node + 0x4C > sz)
				break;
			const byte c0 = d[node];
			if (!((c0 >= 'A' && c0 <= 'Z') || (c0 >= 'a' && c0 <= 'z')))
				break;
			chain[nNodes++] = node;
			const uint32 link = (uint32)RDS(node + 0x10);
			if (link == 0) { ok = true; break; }
			node = link + bs;
		}
		// The root must sit near the file start (right after the header).
		if (!ok || nNodes < 2 || chain[nNodes - 1] >= 0x1000)
			continue;
		// Compose root -> fvi.  The chain-node rotations are always folded
		// into the offset (see the design note above); this `compose` path
		// is the exe-faithful one, confirmed by side-by-side testing.
		double wpos[3] = { 0.0, 0.0, 0.0 };
		double wrot[9] = { 1, 0, 0, 0, 1, 0, 0, 0, 1 };
		for (int i = (int)nNodes - 1; i >= 0; i--) {
			const uint32 n = chain[i];
			const double lp[3] = { (double)RDS(n + 0x1C),
			                       (double)RDS(n + 0x20),
			                       (double)RDS(n + 0x24) };
			double lr[9];
			for (int k = 0; k < 9; k++)
				lr[k] = (double)RDS(n + 0x28 + k * 4) / 32768.0;
			// worldPos += worldRot * localPos
			for (int r = 0; r < 3; r++)
				wpos[r] += wrot[r*3+0]*lp[0] +
				           wrot[r*3+1]*lp[1] +
				           wrot[r*3+2]*lp[2];
			// worldRot = worldRot * transpose(localRot)
			double nr[9];
			for (int r = 0; r < 3; r++)
				for (int c = 0; c < 3; c++)
					nr[r*3+c] = wrot[r*3+0]*lr[c*3+0] +
					            wrot[r*3+1]*lr[c*3+1] +
					            wrot[r*3+2]*lr[c*3+2];
			for (int k = 0; k < 9; k++)
				wrot[k] = nr[k];
		}
		out[0] = -wpos[0]; out[1] = -wpos[1]; out[2] = -wpos[2];
		for (int k = 0; k < 9; k++)
			rotOut[k] = wrot[k];
		return true;
	}
	#undef RDS
	return false;
}

static void f3dcIdentity9(double m[9]) {
	m[0]=1; m[1]=0; m[2]=0; m[3]=0; m[4]=1; m[5]=0; m[6]=0; m[7]=0; m[8]=1;
}

F3dcMouthMesh::F3dcMouthMesh() : _loaded(false), _hasOffset(false),
	_hasUV(false) {
	_offsetO[0] = _offsetO[1] = _offsetO[2] = 0.0;
	f3dcIdentity9(_chainRot);
}

void F3dcMouthMesh::reset() {
	_loaded = false;
	_hasOffset = false;
	_hasUV = false;
	_offsetO[0] = _offsetO[1] = _offsetO[2] = 0.0;
	f3dcIdentity9(_chainRot);
	_verts.clear();
	_tris.clear();
	_uvs.clear();
	_texMapCandidates.clear();
	_boneNodeOff.clear();
	_bytes.clear();
	_fviOff = 0;
}

bool F3dcMouthMesh::loadFromStream(Common::SeekableReadStream &s) {
	reset();

	const int64 fileSize = s.size();
	if (fileSize < 0x100 || fileSize > 0x4000000) {
		warning("F3dcMouthMesh: implausible file size %d", (int)fileSize);
		return false;
	}

	// Slurp the whole file — the format needs random access and these
	// body files are small (under 256 KB).  Kept as a member so
	// computeCamOffsetForFrame can re-walk the scene-graph chain with
	// mutated bone transforms each UBB frame.
	_bytes.resize((uint)fileSize);
	s.seek(0);
	if (s.read(_bytes.begin(), (uint32)fileSize) != (uint32)fileSize) {
		warning("F3dcMouthMesh: short read");
		return false;
	}
	const byte *d = _bytes.begin();
	const uint32 sz = (uint32)fileSize;

	// Little-endian readers with bounds checks.
	#define RD_U32(off) ((uint32)d[(off)] | ((uint32)d[(off)+1] << 8) | \
	                     ((uint32)d[(off)+2] << 16) | ((uint32)d[(off)+3] << 24))

	// 1. Locate the "fvi\0" scene-graph node.
	uint32 fvi = 0;
	bool fviFound = false;
	for (uint32 i = 0; i + 4 <= sz; i++) {
		if (d[i] == 'f' && d[i+1] == 'v' && d[i+2] == 'i' && d[i+3] == 0) {
			fvi = i;
			fviFound = true;
			break;
		}
	}
	if (!fviFound || fvi + 0xD4 > sz) {
		warning("F3dcMouthMesh: no fvi node");
		return false;
	}
	_fviOff = fvi;

	// Read the reloc table at +0x20.  x32dbg traces 2026-05-27 confirmed
	// the engine iterates ALL reloc roots (Bass, Abdo, ..., fvi) and
	// pairs each with a .3DA-A bone via curve_B rec[0] translation
	// match (the .3DA-A bone whose first-frame translation equals the
	// scene-graph node's rest position drives that node).
	{
		const uint32 relocCount = RD_U32(0x1C);
		if (relocCount > 0 && relocCount < 32 &&
		    0x20 + (uint64)relocCount * 4 <= sz) {
			for (uint32 i = 0; i < relocCount; i++) {
				const uint32 delta = RD_U32(0x20 + i * 4);
				const uint32 off = 28 + delta;
				if (off + 0x4C > sz) {
					_boneNodeOff.clear();
					warning("F3dcMouthMesh: reloc root[%u] OOB (off=0x%x)",
					        i, off);
					break;
				}
				// Sanity-check that the offset points to a named node
				// (first byte must be a letter).
				const byte c0 = d[off];
				if (!((c0 >= 'A' && c0 <= 'Z') || (c0 >= 'a' && c0 <= 'z'))) {
					_boneNodeOff.clear();
					warning("F3dcMouthMesh: reloc root[%u] doesn't start "
					        "with a letter (got 0x%02x)", i, c0);
					break;
				}
				_boneNodeOff.push_back(off);
			}
			debugC(1, kDebugMesh, "F3dcMouthMesh: %u scene-graph root nodes",
			      _boneNodeOff.size());
		}
	}

	// Per-character dialog camera offset O + chain rotation, from the
	// scene-graph chain.
	_hasOffset = f3dcComputeCamOffset(d, sz, fvi, _offsetO, _chainRot);
	if (_hasOffset)
		debugC(1, kDebugMesh, "F3dcMouthMesh: cam offset O = (%.0f, %.0f, %.0f)",
		      _offsetO[0], _offsetO[1], _offsetO[2]);
	else
		warning("F3dcMouthMesh: scene-graph chain not decoded");

	// 2. Vertex pool — 40-byte records at fvi+0xD4, count at fvi+0x7C.
	const uint32 vcount = RD_U32(fvi + 0x7C);
	const uint32 p1ref  = RD_U32(fvi + 0x80);   // vertex-index base ref
	const uint32 vbase  = fvi + 0xD4;
	if (vcount == 0 || vcount > 4096 ||
	    vbase + (uint64)vcount * 40 > sz) {
		warning("F3dcMouthMesh: bad vertex pool (count=%u)", vcount);
		return false;
	}
	_verts.resize(vcount * 3);
	for (uint32 i = 0; i < vcount; i++) {
		// XYZ is at the very start of each 40-byte record (the runtime
		// form has a 4-byte link word ahead of XYZ; the on-disk form
		// does not).
		const uint32 o = vbase + i * 40;
		_verts[i*3+0] = (int32)RD_U32(o);
		_verts[i*3+1] = (int32)RD_U32(o + 4);
		_verts[i*3+2] = (int32)RD_U32(o + 8);
	}

	// 3. Triangle count — pool-3 (face-normal) count at fvi+0x94.
	const uint32 tcount = RD_U32(fvi + 0x94);
	if (tcount == 0 || tcount > 8192) {
		warning("F3dcMouthMesh: bad triangle count %u", tcount);
		reset();
		return false;
	}

	// 4. Locate the 64-byte triangle-struct array: the first file offset
	//    where `tcount` consecutive 64-byte structs all decode to valid
	//    vertex indices via (ref - p1ref) / 40.  Spot-check the first 40
	//    structs to find the start, then accept.
	const uint32 stride = 64;
	const uint64 span = (uint64)tcount * stride;
	if (span > sz) {
		warning("F3dcMouthMesh: triangle span exceeds file");
		reset();
		return false;
	}
	// A struct decodes cleanly when each of its three vertex references
	// (+0x08, +0x14, +0x20) maps to an in-range vertex index.
	// The check is a macro-free lambda-style helper inlined below.
	uint32 arr = 0;
	bool arrFound = false;
	const uint32 lastStart = sz - (uint32)span;
	for (uint32 start = 0; start <= lastStart; start += 4) {
		bool ok = true;
		const uint32 probe = (tcount < 40) ? tcount : 40;
		for (uint32 k = 0; k < probe && ok; k++) {
			const uint32 o = start + k * stride;
			for (uint f = 0; f < 3; f++) {
				static const uint kFieldOff[3] = { 0x08, 0x14, 0x20 };
				const uint32 ref = RD_U32(o + kFieldOff[f]);
				if (ref < p1ref) { ok = false; break; }
				const uint32 delta = ref - p1ref;
				if ((delta % 40) != 0 || (delta / 40) >= vcount) {
					ok = false;
					break;
				}
			}
		}
		if (ok) {
			arr = start;
			arrFound = true;
			break;
		}
	}
	if (!arrFound) {
		warning("F3dcMouthMesh: triangle struct array not found");
		reset();
		return false;
	}

	// 5. Extract triangle vertex indices, skipping chroma-key triangles.
	//
	//    The 64-byte triangle struct's first byte is a render-type flags
	//    field consumed by atlantis.exe FUN_00456d88 (the triangle setup
	//    that builds edge records for the deferred span rasteriser):
	//
	//        if ((*tri & 1) == 0) {            // bit 0 set  -> skip entirely
	//            if ((*tri & 2) == 0) {        // bit 1 set  -> partial only,
	//                ... full setup ...        //               no edge build
	//            }
	//        }
	//
	//    Triangles with bit 0 or bit 1 set never reach the rasteriser in
	//    the original engine, even though they have valid vertex/UV refs.
	//    Empirically these are the chroma-key triangles — the mesh covers
	//    the entire facial silhouette but only the bare-skin triangles are
	//    drawn; everything else (the head outline, eyes, hair, …) is left
	//    transparent so the UBB shows through.  Including them in our
	//    render produced the "green patches" artifact (e.g. BadPan eye
	//    region) and the seemingly-saturated edges.  Verified dynamically
	//    against atlantis.exe (x32dbg, 2026-05-23).
	Common::Array<bool> triKeep;
	triKeep.resize(tcount);
	uint32 keptCount = 0;
	for (uint32 k = 0; k < tcount; k++) {
		const byte flags = d[arr + k * stride];
		triKeep[k] = ((flags & 3) == 0);
		if (triKeep[k])
			keptCount++;
	}
	_tris.resize(keptCount * 3);
	uint32 dstIdx = 0;
	for (uint32 k = 0; k < tcount; k++) {
		if (!triKeep[k])
			continue;
		const uint32 o = arr + k * stride;
		static const uint kFieldOff[3] = { 0x08, 0x14, 0x20 };
		for (uint f = 0; f < 3; f++) {
			const uint32 idx = (RD_U32(o + kFieldOff[f]) - p1ref) / 40;
			_tris[dstIdx*3+f] = (uint16)idx;
		}
		dstIdx++;
	}
	debugC(1, kDebugMesh, "F3dcMouthMesh: %u/%u triangles kept (%u chroma-key skipped)",
	      keptCount, tcount, tcount - keptCount);

	// 6. Scrape candidate `.3DM` texture-map basenames from the .3DC header
	//    (a short word string after the `pA<material>` entries — e.g.
	//    "bad10000", "gard", "facer").  The caller resolves them against
	//    DIALOG\MAPS\<name>.3DM.
	{
		const uint32 scanEnd = (sz < 0x200) ? sz : 0x200;
		Common::String cur;
		for (uint32 i = 0x20; i <= scanEnd; i++) {
			const byte c = (i < scanEnd) ? d[i] : 0;
			const bool word = (c >= 'A' && c <= 'Z') ||
			                  (c >= 'a' && c <= 'z') ||
			                  (c >= '0' && c <= '9') || c == '_';
			if (word) {
				cur += (char)c;
			} else {
				if (cur.size() >= 3 && cur.size() <= 12 &&
				    !cur.equalsIgnoreCase("DEFAULT") &&
				    !cur.equalsIgnoreCase("WIRE") &&
				    !((cur[0] == 'p' || cur[0] == 'P') &&
				      (cur[1] == 'A' || cur[1] == 'a')))
					_texMapCandidates.push_back(cur);
				cur.clear();
			}
		}
	}

	// 7. Per-triangle texture coordinates.  The UV pool sits immediately
	//    after the vertex pool — vertex records are 40 bytes starting at
	//    fvi+0xD0, so the UVs begin at fvi+0xD0+vcount*40.  Each UV record
	//    is 8 bytes = (int32 U, int32 V) in 16.16 fixed-point texel coords.
	//    The triangle struct's +0x34/+0x38/+0x3C fields are UV references
	//    parallel to the +0x08/+0x14/+0x20 vertex refs; the UV index is
	//    (ref - poolhead2) / 8 with poolhead2 read from fvi+0x88.
	{
		const uint32 poolhead2 = RD_U32(fvi + 0x88);
		const uint32 uvbase    = fvi + 0xD0 + vcount * 40;
		static const uint kUvFieldOff[3] = { 0x34, 0x38, 0x3C };
		// UVs follow the kept-triangle order so _uvs[i] aligns with _tris[i].
		_uvs.resize(keptCount * 3 * 2);
		const int32 kLim = 256 << 16;
		uint32 nValid = 0;
		bool ok = (uvbase < sz);
		uint32 uvDst = 0;
		for (uint32 k = 0; k < tcount && ok; k++) {
			if (!triKeep[k])
				continue;
			const uint32 o = arr + k * stride;
			for (uint f = 0; f < 3; f++) {
				const uint32 ref = RD_U32(o + kUvFieldOff[f]);
				int32 u = 0, v = 0;
				if (ref >= poolhead2 && ((ref - poolhead2) % 8) == 0) {
					const uint32 uo = uvbase + (ref - poolhead2);
					if (uo + 8 <= sz) {
						u = (int32)RD_U32(uo);
						v = (int32)RD_U32(uo + 4);
						if (u >= 0 && u < kLim && v >= 0 && v < kLim)
							nValid++;
					}
				}
				_uvs[(uvDst*3+f)*2+0] = u;
				_uvs[(uvDst*3+f)*2+1] = v;
			}
			uvDst++;
		}
		// Accept the UV set only when nearly every coordinate landed inside
		// the texture — a wrong pool base scrambles them out of range.
		_hasUV = ok && keptCount > 0 &&
		         nValid >= (keptCount * 3) * 99 / 100;
		if (!_hasUV)
			_uvs.clear();
		debugC(1, kDebugMesh, "F3dcMouthMesh: UV pool @ 0x%x, %u/%u in range -> %s",
		      uvbase, nValid, keptCount * 3, _hasUV ? "textured" : "no UVs");
	}

	#undef RD_U32

	_loaded = true;
	debugC(1, kDebugMesh, "F3dcMouthMesh: loaded %u verts, %u/%u triangles drawable "
	      "(fvi @ 0x%x, tris @ 0x%x)",
	      vcount, keptCount, tcount, fvi, arr);
	return true;
}

// Standard quaternion (qx, qy, qz, qw) -> 3x3 rotation matrix, with all
// inputs and outputs in Q15 fixed-point (0x8000 = 1.0).  Mirrors the
// original engine's FUN_0045d128 (atlantis.exe.c:54979) bit-for-bit.
static void f3dcQuatToMat3Q15(int32 qx, int32 qy, int32 qz, int32 qw,
                              int32 m[9]) {
	const int32 xx = qx * qx;
	const int32 yy = qy * qy;
	const int32 zz = qz * qz;
	m[0] = 0x8000 - ((yy + zz) >> 14);
	m[4] = 0x8000 - ((xx + zz) >> 14);
	m[8] = 0x8000 - ((xx + yy) >> 14);
	const int32 xy = qx * qy;
	const int32 wz = qw * qz;
	m[1] = (xy - wz) >> 14;
	m[3] = (xy + wz) >> 14;
	const int32 xz = qx * qz;
	const int32 wy = qw * qy;
	m[2] = (xz + wy) >> 14;
	m[6] = (xz - wy) >> 14;
	const int32 yz = qy * qz;
	const int32 wx = qw * qx;
	m[5] = (yz - wx) >> 14;
	m[7] = (yz + wx) >> 14;
}

// Little-endian 32-bit store.
static void f3dcWriteI32LE(byte *p, int32 v) {
	p[0] = (byte) (v        & 0xFF);
	p[1] = (byte)((v >>  8) & 0xFF);
	p[2] = (byte)((v >> 16) & 0xFF);
	p[3] = (byte)((v >> 24) & 0xFF);
}

// Little-endian 32-bit signed load.
static int32 f3dcReadI32LE(const byte *p) {
	return (int32)((uint32)p[0] | ((uint32)p[1] << 8) |
	               ((uint32)p[2] << 16) | ((uint32)p[3] << 24));
}

bool F3dcMouthMesh::computeCamOffsetForFrame(uint frame,
                                             const F3dcSmallAnim &anim,
                                             double O[3], double rot[9]) const {
	if (!_loaded || _bytes.empty() || !anim.isLoaded() || _fviOff == 0)
		return false;

	// Copy the .3DC bytes so we can mutate without disturbing _bytes.
	Common::Array<byte> tmp;
	tmp.resize(_bytes.size());
	memcpy(tmp.begin(), _bytes.begin(), _bytes.size());

	// Engine-faithful model (verified 2026-05-27 via x32dbg).
	//
	// The engine iterates ALL 8 scene-graph roots and pairs each with
	// a .3DA-A bone via the "curve_B rec[0] translation matches the
	// scene-graph node's rest position" rule.  For each (scene_root,
	// bone) pair, the engine writes:
	//   - bone+0x28..0x4F: 3x3 matrix derived from curve_A[N].quat
	//                       (where N = cur_frame_idx mod count_A).
	//   - bone+0x1C..0x27: 3 ints from curve_B[N].(tx,ty,tz).
	// The matrix is the raw quat-to-matrix conversion (no delta-from-
	// rec[0], no conjugation): the engine uses curve_A's stored
	// quaternion ABSOLUTELY -- it IS the bone's local-to-parent
	// rotation at frame N.
	//
	// Build the bone-to-scene-graph mapping by translation match.
	// For each .3DA-A bone bi, find the scene-graph root whose rest
	// position equals curve_B rec[0] (tx, ty, tz).  Tolerance is set
	// to 0 since these are exact int32 matches per x32dbg.

	const uint nBones = anim.boneCount();
	const uint nNodes = boneNodeCount();

	// boneToNode[bi] = scene-graph node index, or (uint)-1 if no match.
	Common::Array<uint> boneToNode;
	boneToNode.resize(nBones);
	for (uint bi = 0; bi < nBones; bi++)
		boneToNode[bi] = (uint)-1;

	for (uint bi = 0; bi < nBones; bi++) {
		if (anim.curveBCount(bi) < 1)
			continue;
		int32 kf0[4];
		if (!anim.getCurveB(bi, 0, kf0))
			continue;
		const int32 tx = kf0[1], ty = kf0[2], tz = kf0[3];
		for (uint ni = 0; ni < nNodes; ni++) {
			const uint32 nodeOff = _boneNodeOff[ni];
			if (nodeOff + 0x28 > tmp.size())
				continue;
			const int32 rx = f3dcReadI32LE(tmp.begin() + nodeOff + 0x1C);
			const int32 ry = f3dcReadI32LE(tmp.begin() + nodeOff + 0x20);
			const int32 rz = f3dcReadI32LE(tmp.begin() + nodeOff + 0x24);
			if (rx == tx && ry == ty && rz == tz) {
				boneToNode[bi] = ni;
				break;
			}
		}
	}

	// Apply each animated bone's pose at frame N to its scene-graph node.
	// curve_A keyframe layout (file = runtime, 20 bytes):
	//   +0x00 (idx 0): frame_idx (unused by the engine; just a counter)
	//   +0x04 (idx 1): qx (Q15)
	//   +0x08 (idx 2): qy
	//   +0x0C (idx 3): qz
	//   +0x10 (idx 4): qw
	// curve_B keyframe layout (16 bytes):
	//   +0x00 (idx 0): frame_idx
	//   +0x04 (idx 1): tx
	//   +0x08 (idx 2): ty
	//   +0x0C (idx 3): tz
	// The engine indexes by cur_frame_idx directly (0..count-1).
	for (uint bi = 0; bi < nBones; bi++) {
		const uint ni = boneToNode[bi];
		if (ni == (uint)-1)
			continue;
		const uint32 nodeOff = _boneNodeOff[ni];
		if (nodeOff + 0x4C > tmp.size())
			continue;

		// --- Curve A: write rotation matrix from quaternion at frame N ---
		// Note: our chain walker (f3dcComputeCamOffset) uses
		// `wrot * transpose(localRot)`, so the effective rotation
		// applied to the chain is the inverse of what we store.  To
		// make the chain apply matrix(quat) (the engine's intent), we
		// store matrix(conj(quat)) = transpose(matrix(quat)).
		const uint cA = anim.curveACount(bi);
		if (cA > 0) {
			const uint kIdx = frame % cA;
			int32 kf[5];
			anim.getCurveA(bi, kIdx, kf);
			const int32 qx = kf[1];
			const int32 qy = kf[2];
			const int32 qz = kf[3];
			const int32 qw = kf[4];
			int32 mat[9];
			f3dcQuatToMat3Q15(-qx, -qy, -qz, qw, mat);
			for (int j = 0; j < 9; j++)
				f3dcWriteI32LE(tmp.begin() + nodeOff + 0x28 + j * 4, mat[j]);
		}

		// --- Curve B: write translation at frame N ---
		const uint cB = anim.curveBCount(bi);
		if (cB > 0) {
			const uint kIdx = frame % cB;
			int32 kf[4];
			anim.getCurveB(bi, kIdx, kf);
			f3dcWriteI32LE(tmp.begin() + nodeOff + 0x1C, kf[1]);
			f3dcWriteI32LE(tmp.begin() + nodeOff + 0x20, kf[2]);
			f3dcWriteI32LE(tmp.begin() + nodeOff + 0x24, kf[3]);
		}
	}

	// Re-run the standard chain walk with the mutated bone transforms.
	return f3dcComputeCamOffset(tmp.begin(), (uint32)tmp.size(),
	                            _fviOff, O, rot);
}


bool F3dcMouthMesh::getVertex(uint vi, int32 &x, int32 &y, int32 &z) const {
	if (vi * 3 + 2 >= _verts.size()) {
		x = y = z = 0;
		return false;
	}
	x = _verts[vi*3+0];
	y = _verts[vi*3+1];
	z = _verts[vi*3+2];
	return true;
}

bool F3dcMouthMesh::getTriangle(uint ti, uint &v0, uint &v1, uint &v2) const {
	if (ti * 3 + 2 >= _tris.size()) {
		v0 = v1 = v2 = 0;
		return false;
	}
	v0 = _tris[ti*3+0];
	v1 = _tris[ti*3+1];
	v2 = _tris[ti*3+2];
	return true;
}

bool F3dcMouthMesh::getTriangleUV(uint ti, uint corner,
                                  int32 &u, int32 &v) const {
	if (!_hasUV || corner > 2 || (ti * 3 + corner) * 2 + 1 >= _uvs.size()) {
		u = v = 0;
		return false;
	}
	u = _uvs[(ti*3+corner)*2+0];
	v = _uvs[(ti*3+corner)*2+1];
	return true;
}

} // namespace Atlantis
} // namespace CryOmni3D
