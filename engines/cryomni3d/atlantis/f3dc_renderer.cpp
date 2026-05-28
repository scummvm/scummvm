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

#include "common/array.h"
#include "common/debug.h"
#include "common/scummsys.h"
#include "common/system.h"

#include "graphics/surface.h"

#include "cryomni3d/cryomni3d.h"
#include "cryomni3d/atlantis/f3dc_parser.h"
#include "cryomni3d/atlantis/f3dc_renderer.h"

namespace CryOmni3D {
namespace Atlantis {

// ---------------------------------------------------------------------------
// Software rasteriser helpers (moved verbatim from engine.cpp).
// ---------------------------------------------------------------------------

// Z-buffered edge-function triangle rasteriser.  A pixel is inside the
// triangle iff all three barycentric weights share a sign (accepts both
// windings).  Depth is interpolated per pixel from the vertex Z values;
// the pixel paints only when nearer than the z-buffer's current value.
// Cryo Z points OUT of the screen, so larger Z = nearer the camera.
static void fillTriangleZ(Graphics::Surface &surf, int32 *zbuf,
                          int x0, int y0, int z0,
                          int x1, int y1, int z1,
                          int x2, int y2, int z2,
                          uint32 color) {
	const int W = surf.w;
	const int H = surf.h;
	const int bpp = surf.format.bytesPerPixel;
	int minX = MIN(MIN(x0, x1), x2);
	int maxX = MAX(MAX(x0, x1), x2);
	int minY = MIN(MIN(y0, y1), y2);
	int maxY = MAX(MAX(y0, y1), y2);
	if (minX < 0) minX = 0;
	if (minY < 0) minY = 0;
	if (maxX >= W) maxX = W - 1;
	if (maxY >= H) maxY = H - 1;
	if (minX > maxX || minY > maxY) return;
	const int area = (x1 - x0) * (y2 - y0) - (y1 - y0) * (x2 - x0);
	if (area == 0) return;
	const int A01 = y0 - y1, B01 = x1 - x0;
	const int A12 = y1 - y2, B12 = x2 - x1;
	const int A20 = y2 - y0, B20 = x0 - x2;
	int w0_row = (x2 - x1) * (minY - y1) - (y2 - y1) * (minX - x1);
	int w1_row = (x0 - x2) * (minY - y2) - (y0 - y2) * (minX - x2);
	int w2_row = (x1 - x0) * (minY - y0) - (y1 - y0) * (minX - x0);
	for (int py = minY; py <= maxY; py++) {
		int w0 = w0_row, w1 = w1_row, w2 = w2_row;
		for (int px = minX; px <= maxX; px++) {
			// Inside test: all three barycentric weights share a sign
			// (zero counts as both — edge pixels belong to BOTH adjacent
			// triangles, then the z-buffer picks one).  The previous
			// form `(w0 & w1 & w2) < 0` required all three STRICTLY
			// negative, dropping every shared-edge pixel of CW (negative-
			// area) triangles — visible as the 1-pixel cracks through
			// which the UBB background showed.
			if (((w0 >= 0) && (w1 >= 0) && (w2 >= 0)) ||
			    ((w0 <= 0) && (w1 <= 0) && (w2 <= 0))) {
				const int64 zNum = (int64)w0 * z0 + (int64)w1 * z1 +
				                   (int64)w2 * z2;
				const int32 z = (int32)(zNum / area);
				int32 *zp = &zbuf[py * W + px];
				if (z > *zp) {
					*zp = z;
					byte *p = (byte *)surf.getBasePtr(px, py);
					if (bpp == 2)      *(uint16 *)p = (uint16)color;
					else if (bpp == 4) *(uint32 *)p = color;
					else               memcpy(p, &color, bpp);
				}
			}
			w0 += A12; w1 += A20; w2 += A01;
		}
		w0_row += B12; w1_row += B20; w2_row += B01;
	}
}

// Z-buffered line — draws a triangle edge to close the hairline cracks
// that a point-sampled rasteriser leaves between adjacent faces.  Uses
// z >= zbuf (>=, not >) so edge pixels reliably win ties against the
// fill at shared seams.
static void drawLineZ(Graphics::Surface &surf, int32 *zbuf,
                      int x0, int y0, int z0, int x1, int y1, int z1,
                      uint32 color) {
	const int W = surf.w, H = surf.h;
	const int bpp = surf.format.bytesPerPixel;
	const int dx = ABS(x1 - x0), dy = ABS(y1 - y0);
	int steps = (dx > dy) ? dx : dy;
	if (steps == 0) steps = 1;
	for (int i = 0; i <= steps; i++) {
		const int px = x0 + (x1 - x0) * i / steps;
		const int py = y0 + (y1 - y0) * i / steps;
		const int z  = z0 + (z1 - z0) * i / steps;
		if (px < 0 || px >= W || py < 0 || py >= H) continue;
		int32 *zp = &zbuf[py * W + px];
		if (z >= *zp) {
			*zp = z;
			byte *p = (byte *)surf.getBasePtr(px, py);
			if (bpp == 2)      *(uint16 *)p = (uint16)color;
			else if (bpp == 4) *(uint32 *)p = color;
			else               memcpy(p, &color, bpp);
		}
	}
}

// Light level the dialog mouth mesh is shaded at — the `param_2` value
// captured from the original engine's rasteriser (atlantis.exe FUN_00456d88).
static const uint kMouthShadeLevel = 15;

// Z-buffered, perspective-correct textured triangle fill.  Mirrors
// fillTriangleZ's edge-function rasteriser, but instead of a flat colour it
// samples the .3DM face texture per pixel: U/z, V/z and 1/z are interpolated
// barycentrically and divided back to recover (u, v).  Each texel's colour
// index resolves through the `.3DM` shade table at light level `shade`.
static void fillTriangleTex(Graphics::Surface &surf, int32 *zbuf,
                            const int sx[3], const int sy[3],
                            const int sd[3], const double cz[3],
                            const double tu[3], const double tv[3],
                            const F3dcTextureMap &tex, uint shade) {
	const int W = surf.w, H = surf.h;
	const int bpp = surf.format.bytesPerPixel;
	int minX = MIN(MIN(sx[0], sx[1]), sx[2]);
	int maxX = MAX(MAX(sx[0], sx[1]), sx[2]);
	int minY = MIN(MIN(sy[0], sy[1]), sy[2]);
	int maxY = MAX(MAX(sy[0], sy[1]), sy[2]);
	if (minX < 0) minX = 0;
	if (minY < 0) minY = 0;
	if (maxX >= W) maxX = W - 1;
	if (maxY >= H) maxY = H - 1;
	if (minX > maxX || minY > maxY) return;
	const int area = (sx[1]-sx[0])*(sy[2]-sy[0]) - (sy[1]-sy[0])*(sx[2]-sx[0]);
	if (area == 0) return;
	const double invArea = 1.0 / (double)area;
	// Perspective-correct attribute setup: interpolate u/z, v/z and 1/z.
	double iz[3], uoz[3], voz[3];
	for (int i = 0; i < 3; i++) {
		double z = cz[i];
		if (z < 1.0) z = 1.0;
		iz[i]  = 1.0 / z;
		uoz[i] = tu[i] * iz[i];
		voz[i] = tv[i] * iz[i];
	}
	const int A01 = sy[0]-sy[1], B01 = sx[1]-sx[0];
	const int A12 = sy[1]-sy[2], B12 = sx[2]-sx[1];
	const int A20 = sy[2]-sy[0], B20 = sx[0]-sx[2];
	int w0_row = (sx[2]-sx[1])*(minY-sy[1]) - (sy[2]-sy[1])*(minX-sx[1]);
	int w1_row = (sx[0]-sx[2])*(minY-sy[2]) - (sy[0]-sy[2])*(minX-sx[2]);
	int w2_row = (sx[1]-sx[0])*(minY-sy[0]) - (sy[1]-sy[0])*(minX-sx[0]);
	for (int py = minY; py <= maxY; py++) {
		int w0 = w0_row, w1 = w1_row, w2 = w2_row;
		for (int px = minX; px <= maxX; px++) {
			// Inside test: all three barycentric weights share a sign
			// (zero counts as both — edge pixels belong to BOTH adjacent
			// triangles, then the z-buffer picks one).  The previous
			// form `(w0 & w1 & w2) < 0` required all three STRICTLY
			// negative, dropping every shared-edge pixel of CW (negative-
			// area) triangles — visible as the 1-pixel cracks through
			// which the UBB background showed.
			if (((w0 >= 0) && (w1 >= 0) && (w2 >= 0)) ||
			    ((w0 <= 0) && (w1 <= 0) && (w2 <= 0))) {
				const int64 zNum = (int64)w0*sd[0] + (int64)w1*sd[1] +
				                   (int64)w2*sd[2];
				const int32 z = (int32)(zNum / area);
				int32 *zp = &zbuf[py * W + px];
				if (z > *zp) {
					const double a0 = w0 * invArea;
					const double a1 = w1 * invArea;
					const double a2 = w2 * invArea;
					const double izp = a0*iz[0] + a1*iz[1] + a2*iz[2];
					// The .3DC UV pair is the standard (u, v) — first
					// component is the horizontal axis (verified by
					// dynamic-trace of atlantis.exe 0x004743C0, the textured
					// span filler used for BadPan's mouth: it fetches
					// `tex[V*256+U]` per pixel).
					int u = 0, v = 0;
					if (izp > 1e-9) {
						u = (int)((a0*uoz[0]+a1*uoz[1]+a2*uoz[2]) / izp);
						v = (int)((a0*voz[0]+a1*voz[1]+a2*voz[2]) / izp);
					}
					// Just sample the texel and shade.  No chroma test —
					// the mesh defines what gets drawn; if a triangle
					// belongs to the face, all of its sampled texels
					// are valid skin colours.  An earlier session
					// hypothesised that bit 15 of the shade-table u16
					// was a chroma flag (R5G5B5 with inverted alpha
					// polarity), but cross-checking RHEA.3DM against
					// the artist-source RHEA.BMP proved bit 15 is just
					// the high R bit of the R5G6B5 colour: 119/256
					// level-15 entries have bit 15 set and they're all
					// legitimate face colours.  Discarding them left
					// gaps in the rendered face that the UBB video
					// showed through (the "green/black speckles"
					// artefact reported 2026-05-24).  See memory
					// `f3dc-colour-format`.
					byte tr, tg, tb;
					tex.shadeRGB(tex.texel(u, v), shade, tr, tg, tb);
					const uint32 c = surf.format.RGBToColor(tr, tg, tb);
					*zp = z;
					byte *p = (byte *)surf.getBasePtr(px, py);
					if (bpp == 2)      *(uint16 *)p = (uint16)c;
					else if (bpp == 4) *(uint32 *)p = c;
					else               memcpy(p, &c, bpp);
				}
			}
			w0 += A12; w1 += A20; w2 += A01;
		}
		w0_row += B12; w1_row += B20; w2_row += B01;
	}
}

// ---------------------------------------------------------------------------
// F3dcMouthRenderer
// ---------------------------------------------------------------------------

F3dcMouthRenderer::F3dcMouthRenderer()
	: _mesh(nullptr), _s3d(nullptr), _anim(nullptr), _smallAnim(nullptr),
	  _tex(nullptr),
	  _ready(false), _animated(false), _textured(false), _vertOffset(0),
	  _camHasXform(false), _camFovDeg(20.0) {
	for (int i = 0; i < 9; i++) {
		_camRot[i]      = (i % 4 == 0) ? 1.0 : 0.0;
		_camChainRot[i] = (i % 4 == 0) ? 1.0 : 0.0;
	}
	_camP[0] = _camP[1] = _camP[2] = 0.0;
}

bool F3dcMouthRenderer::init(const F3dcMouthMesh &mesh, const F3dcMesh &s3d,
                             const F3dcMouthAnim &anim,
                             const F3dcTextureMap *tex, int camNum,
                             const F3dcSmallAnim *smallAnim) {
	_mesh = &mesh;
	_s3d  = &s3d;
	_anim = &anim;
	_smallAnim = (smallAnim && smallAnim->isLoaded()) ? smallAnim : nullptr;
	_tex  = tex;
	_ready = mesh.isLoaded();
	if (!_ready)
		return false;

	deriveCamera(camNum);

	// CAM2 close-up fallback -- the real exe-faithful values.
	//
	// REVERSE-ENGINEERED 2026-05-26 via x32dbg.  Set BP1 at the cam-setup
	// function entry (atlantis.exe.c FUN_0044d7b0, VA 0x0044d7b0), ran
	// the inn-scene Lascoyt dialog, waited for an explicit [cam2] line
	// (EDX=0 = cam_idx 0).  Captured slot 0's cam-data pointer
	// `[0xbc16a0]` and dumped the 8 doubles at that pointer:
	//
	//   eye    = ( 0.4874, -9.1076,  84.1815)
	//   target = (-0.1898,  1.6789, -189.3910)
	//   scale  = 150.0
	//   fov    = 13.6822  (degrees)
	//
	// Those exactly match the adult-character cam2 template that ships
	// with the 12 .S3D files that DO have a non-zero CAM2 record
	// (HEROS.S3D, AGAT.S3D, SERVAN1M.S3D, DAGAN.S3D, FELICIE.S3D,
	// PRET.S3D, VIEAN1.S3D, CONT30.S3D and a few more).  So the exe
	// PATCHES the all-zero CAM2 records at runtime with this template
	// (the patch wasn't visible from static analysis -- it must happen
	// in a different code path than FUN_0044d5f0).  Cainan, Rhea and
	// RHEA2 are the only characters whose .S3D ships a DIFFERENT cam2
	// template (the "child face" variant, eye z=33 instead of 84) --
	// for those, deriveCamera already succeeded above and this fallback
	// is skipped.
	//
	// Re-run the standard look-at pipeline with the captured doubles so
	// the rendered mesh lands in the cam2 UBB's lower-face chroma-key
	// hole exactly like the original.
	if (!_camHasXform && camNum == 2) {
		// Adult cam2 template (8 doubles in the .S3D layout), captured
		// at runtime via x32dbg dump of [0xbc16a0] -- the engine patches
		// every all-zero cam2 record with this template (or the "small"
		// variant below).
		const double kCam2AdultEye[3]    = {  0.4874, -9.1076,  84.1815 };
		const double kCam2AdultTarget[3] = { -0.1898,  1.6789, -189.3910 };
		const double kCam2AdultScale     = 150.0;
		const double kCam2AdultFovDeg    = 13.6822;

		// "Small" cam2 template -- adult / 2.540, matching the cam3 size
		// ratio of the small-character group (Passant, Meljan, Cain,
		// Chasrat, Lasc, Saddam).  All cam3 records in those characters'
		// .S3D files have tgt.z near -74.6 (vs -189.5 for adults), and
		// the cam2 patch the engine applies follows the same proportion.
		// Memory `mouth-pipeline-synthesis` records the "child face"
		// variant with eye.z=33 (== 84.18 / 2.54) consistent with this.
		const double kCam2SmallEye[3]    = {  0.4874 / 2.540, -9.1076 / 2.540,  84.1815 / 2.540 };
		const double kCam2SmallTarget[3] = { -0.1898 / 2.540,  1.6789 / 2.540, -189.3910 / 2.540 };
		const double kCam2SmallScale     = 150.0;          // unchanged
		const double kCam2SmallFovDeg    = 13.6822;        // unchanged

		// Pick the template by inspecting the character's cam3 tgt.z:
		// abs(tgt.z) > ~130 = adult group, otherwise = small group.
		// Falls back to adult if cam3 isn't readable.
		bool useSmall = false;
		{
			double xf[8];
			if (_s3d->getCamTransform(3, xf)) {
				const double tgtZ = xf[5];
				if (-tgtZ < 130.0)
					useSmall = true;
			}
		}

		const double *eye      = useSmall ? kCam2SmallEye      : kCam2AdultEye;
		const double *target   = useSmall ? kCam2SmallTarget   : kCam2AdultTarget;
		const double scaleSel  = useSmall ? kCam2SmallScale    : kCam2AdultScale;
		const double fovDegSel = useSmall ? kCam2SmallFovDeg   : kCam2AdultFovDeg;
		debugC(1, kDebugMesh, "F3dcMouthRenderer: cam2 fallback -- %s template",
		      useSmall ? "small" : "adult");
		double fwd[3] = { target[0] - eye[0],
		                  target[1] - eye[1],
		                  target[2] - eye[2] };
		double fl = sqrt(fwd[0]*fwd[0] + fwd[1]*fwd[1] + fwd[2]*fwd[2]);
		if (fl > 1e-6) {
			fwd[0] /= fl; fwd[1] /= fl; fwd[2] /= fl;
			double rgt[3] = { fwd[2], 0.0, -fwd[0] };
			double rl = sqrt(rgt[0]*rgt[0] + rgt[1]*rgt[1] + rgt[2]*rgt[2]);
			if (rl > 1e-6) {
				rgt[0] /= rl; rgt[1] /= rl; rgt[2] /= rl;
				const double up[3] = {
				    fwd[1]*rgt[2] - fwd[2]*rgt[1],
				    fwd[2]*rgt[0] - fwd[0]*rgt[2],
				    fwd[0]*rgt[1] - fwd[1]*rgt[0] };
				_camRot[0] = -rgt[0]; _camRot[1] = -rgt[1]; _camRot[2] = -rgt[2];
				_camRot[3] =  up[0];  _camRot[4] =  up[1];  _camRot[5] =  up[2];
				_camRot[6] = -fwd[0]; _camRot[7] = -fwd[1]; _camRot[8] = -fwd[2];
				_camP[0] = scaleSel * target[0];
				_camP[1] = scaleSel * target[1];
				_camP[2] = scaleSel * target[2];
				_camFovDeg = fovDegSel;
				_camHasXform = true;
			}
		}
	}
	if (!_camHasXform) {
		debugC(1, kDebugMesh, "F3dcMouthRenderer: no cam%d transform -- "
		         "renderer disabled for this line", camNum);
		_ready = false;
		return false;
	}

	// Save the cam translation P before the scene-graph offset is added.
	// Per-frame head-sway will reset _camP from this base and add a
	// fresh offset O derived from the .3DA-A keyframes at the current
	// UBB frame.
	_camPBase[0] = _camP[0];
	_camPBase[1] = _camP[1];
	_camPBase[2] = _camP[2];

	// Apply the .3DC rest-pose scene-graph offset O + chain rotation
	// R_world.  These are the initial (UBB frame 0) values; renderFrame
	// overrides them per UBB frame when _smallAnim is loaded.
	if (_mesh->hasCamOffset()) {
		double off[3];
		_mesh->getCamOffset(off);
		_camP[0] += off[0];
		_camP[1] += off[1];
		_camP[2] += off[2];
		_mesh->getChainRot(_camChainRot);
	}

	// Wire the per-frame .3DA-M lip-sync animation onto the .3DC topology.
	_vertOffset = detectVertexOffset();
	_animated   = (_vertOffset >= 0);
	if (_animated)
		debugC(1, kDebugMesh, "F3dcMouthRenderer: lip-sync wired (%u frames, "
		         "vertex offset %d)", _anim->frameCount(), _vertOffset);
	else
		debugC(1, kDebugMesh, "F3dcMouthRenderer: no lip-sync — static bind pose");

	// Texture-mapped rendering enabled.  The texel->colour math
	// (`shadeRGB(texel, 15)` against the .3DM's [32 light][256 colour]
	// R5G6B5 shade table — bit 15 = chroma flag + high R bit, see
	// `f3dc_parser.h`) is verified empirically against the original
	// engine on 2026-05-24: patch_3dm probes (V1 magenta / V2 red+blue /
	// V3 chroma flip) prove the on-disk colour format and the chroma
	// rule.  See memory `f3dc-colour-format`.
	const bool kTexturedRenderingReady = true;
	_textured = kTexturedRenderingReady &&
	            (_tex != nullptr) && _tex->isLoaded() && _mesh->hasUV();
	debugC(1, kDebugMesh, "F3dcMouthRenderer: %s", _textured ? "texture-mapped"
	      : "flat-shaded (textured rendering gated off)");
	return true;
}

void F3dcMouthRenderer::deriveCamera(int camNum) {
	_camHasXform = false;

	// The .S3D cam record is a look-at camera: d[0..2] = look-at point,
	// d[3..5] = camera anchor, d[6] = fixed scale, d[7] = FOV in degrees.
	// Build the model->camera view matrix M and translation P; the
	// per-character offset O from the .3DC scene graph completes P.
	double xf[8];
	if (camNum >= 2 && camNum <= 5 && _s3d->getCamTransform(camNum, xf)) {
		const double eye[3]    = { xf[0], xf[1], xf[2] };
		const double target[3] = { xf[3], xf[4], xf[5] };
		double fwd[3] = { target[0] - eye[0],
		                  target[1] - eye[1],
		                  target[2] - eye[2] };
		double fl = sqrt(fwd[0]*fwd[0] + fwd[1]*fwd[1] + fwd[2]*fwd[2]);
		if (fl > 1e-6) {
			fwd[0] /= fl; fwd[1] /= fl; fwd[2] /= fl;
			// right = normalize(cross(worldUp, fwd)), worldUp = (0,1,0).
			double rgt[3] = { fwd[2], 0.0, -fwd[0] };
			double rl = sqrt(rgt[0]*rgt[0] + rgt[1]*rgt[1] + rgt[2]*rgt[2]);
			if (rl > 1e-6) {
				rgt[0] /= rl; rgt[1] /= rl; rgt[2] /= rl;
				// camUp = cross(fwd, right).
				const double up[3] = {
				    fwd[1]*rgt[2] - fwd[2]*rgt[1],
				    fwd[2]*rgt[0] - fwd[0]*rgt[2],
				    fwd[0]*rgt[1] - fwd[1]*rgt[0] };
				// Rows: (-right, +camUp, -forward).
				_camRot[0] = -rgt[0]; _camRot[1] = -rgt[1]; _camRot[2] = -rgt[2];
				_camRot[3] =  up[0];  _camRot[4] =  up[1];  _camRot[5] =  up[2];
				_camRot[6] = -fwd[0]; _camRot[7] = -fwd[1]; _camRot[8] = -fwd[2];
				_camP[0] = xf[6] * target[0];
				_camP[1] = xf[6] * target[1];
				_camP[2] = xf[6] * target[2];
				_camFovDeg = xf[7];
				_camHasXform = true;
			}
		}
	}

	debugC(1, kDebugMesh, "F3dcMouthRenderer: cam%d %s (fov %.1f, P=(%.0f, %.0f, %.0f))",
	      camNum, _camHasXform ? "look-at" : "tumble",
	      _camFovDeg, _camP[0], _camP[1], _camP[2]);
}

int F3dcMouthRenderer::detectVertexOffset() const {
	if (!_anim->isLoaded())
		return -1;
	const uint nv  = _mesh->vertexCount();
	const uint vpf = _anim->verticesPerFrame();
	if (nv == 0 || vpf < nv)
		return -1;

	// Mesh bounding-box diagonal — the scale a candidate offset's residual
	// is judged against.
	int32 lo[3] = {  0x7FFFFFFF,  0x7FFFFFFF,  0x7FFFFFFF };
	int32 hi[3] = { -0x7FFFFFFF, -0x7FFFFFFF, -0x7FFFFFFF };
	for (uint gi = 0; gi < nv; gi++) {
		int32 v[3];
		_mesh->getVertex(gi, v[0], v[1], v[2]);
		for (int a = 0; a < 3; a++) {
			if (v[a] < lo[a]) lo[a] = v[a];
			if (v[a] > hi[a]) hi[a] = v[a];
		}
	}
	double diag = 0.0;
	for (int a = 0; a < 3; a++) {
		const double d = (double)hi[a] - lo[a];
		diag += d * d;
	}
	diag = sqrt(diag);
	if (diag < 1.0)
		diag = 1.0;

	// .3DA-M frame 0 is the rest pose; the .3DC bind pose is the same mesh
	// preceded by `vpf - nv` leading sentinel vertices.  The two are
	// authored slightly differently, so don't demand an exact match — pick
	// the shift with the smallest residual and accept it when that residual
	// is small relative to the mesh (a wrong shift scrambles the vertices
	// and lands a residual of tens of percent of the mesh size).
	const uint maxOff    = vpf - nv;
	const uint scanLimit = (maxOff < 4) ? maxOff : 4;
	debugC(1, kDebugMesh, "F3dcMouthRenderer: vertex-offset scan "
	         "(.3DC %u verts, .3DA-M %u verts, mesh diag %.0f)",
	      nv, vpf, diag);
	int    bestOff = -1;
	double bestRms = 0.0;
	for (uint off = 0; off <= scanLimit; off++) {
		double sse = 0.0;
		for (uint gi = 0; gi < nv; gi++) {
			int32 bx, by, bz, ax, ay, az;
			_mesh->getVertex(gi, bx, by, bz);
			_anim->getVertex(0, gi + off, ax, ay, az);
			const double dx = (double)ax - bx;
			const double dy = (double)ay - by;
			const double dz = (double)az - bz;
			sse += dx*dx + dy*dy + dz*dz;
		}
		const double rms = sqrt(sse / nv);
		debugC(1, kDebugMesh, "F3dcMouthRenderer:   offset %u -> RMS %.0f (%.1f%% of mesh)",
		      off, rms, 100.0 * rms / diag);
		if (bestOff < 0 || rms < bestRms) {
			bestOff = (int)off;
			bestRms = rms;
		}
	}
	// Sample pairs at the structurally-expected offset, to diagnose a
	// scrambled vertex order should the scan reject every shift.
	{
		const uint gis[3] = { 0, nv / 2, nv - 1 };
		for (int s = 0; s < 3; s++) {
			int32 bx, by, bz, ax, ay, az;
			_mesh->getVertex(gis[s], bx, by, bz);
			_anim->getVertex(0, gis[s] + maxOff, ax, ay, az);
			debugC(1, kDebugMesh, "F3dcMouthRenderer:   .3DC[%u]=(%d,%d,%d) "
			         ".3DA-M[%u]=(%d,%d,%d)",
			      gis[s], bx, by, bz, gis[s] + maxOff, ax, ay, az);
		}
	}
	if (bestOff >= 0 && bestRms < diag * 0.15)
		return bestOff;
	return -1;
}

void F3dcMouthRenderer::renderFrame(Graphics::Surface &dst, uint ubbFrame,
                                    uint32 elapsedMs) {
	(void)elapsedMs;   // used by sub-frame interpolation (later phase)
	if (!_ready)
		return;

	const Graphics::PixelFormat &fmt = dst.format;
	const uint gNV = _mesh->vertexCount();
	const uint gNT = _mesh->triangleCount();
	if (gNV == 0 || gNT == 0)
		return;

	// Source vertices for this frame: the per-frame .3DA-M viseme when
	// lip-sync is wired, else the static .3DC bind pose.
	Common::Array<int32> mvX, mvY, mvZ;
	mvX.resize(gNV); mvY.resize(gNV); mvZ.resize(gNV);

	const int32 *frameVerts = nullptr;
	if (_animated) {
		// Frame 0 is the rest pose; the lip-sync loops the animation
		// frames 1..N-1, advancing one viseme per UBB video frame (the
		// original advances the mouth index once per UBB tick).
		const uint nFrames = _anim->frameCount();
		const uint k = (nFrames > 1) ? (1 + ubbFrame % (nFrames - 1)) : 0;
		frameVerts = _anim->frameVertices(k);
	}
	if (frameVerts) {
		for (uint gi = 0; gi < gNV; gi++) {
			const int32 *v = frameVerts + (gi + _vertOffset) * 3;
			mvX[gi] = v[0]; mvY[gi] = v[1]; mvZ[gi] = v[2];
		}
	} else {
		for (uint gi = 0; gi < gNV; gi++) {
			int32 vX, vY, vZ;
			_mesh->getVertex(gi, vX, vY, vZ);
			mvX[gi] = vX; mvY[gi] = vY; mvZ[gi] = vZ;
		}
	}

	// Head-sway: apply the .3DA-A skeletal animation at the current UBB
	// frame to recompute the cam offset O.  The .3DA-A's per-bone
	// quaternion/translation tracks replace each bone's rest-pose
	// transform; the scene-graph chain walk in computeCamOffsetForFrame
	// then yields a per-frame O that tracks the head's motion.  See
	// project_f3dc_head_sway memory for the file-format details and the
	// verified-empirical keyframe layout.
	if (_smallAnim && _mesh->boneNodeCount() > 0) {
		double O[3], rot[9];
		if (_mesh->computeCamOffsetForFrame(ubbFrame, *_smallAnim, O, rot)) {
			_camP[0] = _camPBase[0] + O[0];
			_camP[1] = _camPBase[1] + O[1];
			_camP[2] = _camPBase[2] + O[2];
			for (uint i = 0; i < 9; i++)
				_camChainRot[i] = rot[i];
			// One-shot debug: confirm per-frame O actually varies.
			debugC(1, kDebugMesh, "head-sway frame=%u O=(%.0f, %.0f, %.0f) rotZcol=(%.3f %.3f %.3f)",
			      ubbFrame, O[0], O[1], O[2], rot[2], rot[5], rot[8]);
		} else {
			debugC(1, kDebugMesh, "head-sway frame=%u computeCamOffsetForFrame returned FALSE",
			      ubbFrame);
		}
	} else {
		debugC(1, kDebugMesh, "head-sway frame=%u skipped (smallAnim=%p bonecount=%u)",
		      ubbFrame, (const void *)_smallAnim, _mesh->boneNodeCount());
	}

	// Mesh bounds (centroid used by the no-transform debug tumble).
	int32 gxMin =  0x7FFFFFFF, gxMax = -0x7FFFFFFF;
	int32 gyMin =  0x7FFFFFFF, gyMax = -0x7FFFFFFF;
	int32 gzMin =  0x7FFFFFFF, gzMax = -0x7FFFFFFF;
	for (uint gi = 0; gi < gNV; gi++) {
		if (mvX[gi] < gxMin) gxMin = mvX[gi];
		if (mvX[gi] > gxMax) gxMax = mvX[gi];
		if (mvY[gi] < gyMin) gyMin = mvY[gi];
		if (mvY[gi] > gyMax) gyMax = mvY[gi];
		if (mvZ[gi] < gzMin) gzMin = mvZ[gi];
		if (mvZ[gi] > gzMax) gzMax = mvZ[gi];
	}
	const int32 gMidX = (gxMin + gxMax) / 2;
	const int32 gMidV = (gzMin + gzMax) / 2;
	const int32 gMidY = (gyMin + gyMax) / 2;

	// Per-frame z-buffer, cleared to "infinitely far" (most-negative —
	// larger Z = nearer the camera).
	Common::Array<int32> zbuf;
	zbuf.resize((uint)dst.w * dst.h);
	for (uint zi = 0; zi < zbuf.size(); zi++)
		zbuf[zi] = -0x7FFFFFFF;

	// gR* hold camera-space coords (used for flat shading); gS* hold the
	// projected screen coords.
	Common::Array<int> gSX, gSY, gSD;
	Common::Array<double> gRX, gRY, gRZ;
	gSX.resize(gNV); gSY.resize(gNV); gSD.resize(gNV);
	gRX.resize(gNV); gRY.resize(gNV); gRZ.resize(gNV);

	// Full engine pipeline -- init() returns early when no transform was
	// derived (CAM2 ships an all-zero record on most characters), so we
	// can assume _camHasXform here.
	//   camera = M * (R_world*model - P)
	//   screen = camXY * focal / camZ + centre
	const double fovR = _camFovDeg * M_PI / 180.0;
	double focal = (dst.w * 0.5) / tan(fovR * 0.5);
	if (focal < 1.0) focal = 1.0;
	const double pcx = dst.w * 0.5;
	const double pcy = dst.h * 0.5;
	for (uint vi = 0; vi < gNV; vi++) {
		// Bake in the scene-graph chain rotation R_world, then translate.
		const double mx = mvX[vi];
		const double my = mvY[vi];
		const double mz = mvZ[vi];
		const double rx = _camChainRot[0]*mx +
		    _camChainRot[1]*my + _camChainRot[2]*mz;
		const double ry = _camChainRot[3]*mx +
		    _camChainRot[4]*my + _camChainRot[5]*mz;
		const double rz = _camChainRot[6]*mx +
		    _camChainRot[7]*my + _camChainRot[8]*mz;
		const double dx = rx - _camP[0];
		const double dy = ry - _camP[1];
		const double dz = rz - _camP[2];
		const double cX = _camRot[0]*dx + _camRot[1]*dy + _camRot[2]*dz;
		const double cY = _camRot[3]*dx + _camRot[4]*dy + _camRot[5]*dz;
		double cZ = _camRot[6]*dx + _camRot[7]*dy + _camRot[8]*dz;
		if (cZ < 1.0) cZ = 1.0;
		gRX[vi] = cX; gRY[vi] = cY; gRZ[vi] = cZ;
		gSX[vi] = (int)(cX * focal / cZ + pcx);
		gSY[vi] = (int)(cY * focal / cZ + pcy);
		// Nearer = smaller camZ; store -camZ so the z-buffer's
		// "larger wins" keeps the nearest surface.
		gSD[vi] = (int)(-cZ);
	}
	(void)gMidX; (void)gMidY; (void)gMidV;

	for (uint ti = 0; ti < gNT; ti++) {
		uint i0, i1, i2;
		_mesh->getTriangle(ti, i0, i1, i2);
		if (i0 >= gNV || i1 >= gNV || i2 >= gNV)
			continue;

		// Backface cull — match atlantis.exe FUN_00456d88's `if (area < 0.0)`
		// test (the original engine only emits edge records for triangles
		// whose screen-space signed area is negative).  Without this,
		// back-of-head triangles whose UVs map to the unused green chroma
		// region of the .3DM win the z-test in spots and paint green
		// patches at the mesh silhouette.  Verified dynamically against
		// atlantis.exe (x32dbg, 2026-05-23).
		{
			const int64 sArea =
			    (int64)(gSX[i1] - gSX[i0]) * (gSY[i2] - gSY[i0]) -
			    (int64)(gSY[i1] - gSY[i0]) * (gSX[i2] - gSX[i0]);
			if (sArea >= 0)
				continue;
		}

		if (_textured) {
			// Perspective-correct textured fill — sample the .3DM face
			// texture through the UBB palette.  UV corner c pairs with
			// triangle vertex c (the .3DC stores them as parallel fields).
			const uint vi[3] = { i0, i1, i2 };
			int    sx[3], sy[3], sd[3];
			double cz[3], tu[3], tv[3];
			for (int c = 0; c < 3; c++) {
				sx[c] = gSX[vi[c]];
				sy[c] = gSY[vi[c]];
				sd[c] = gSD[vi[c]];
				cz[c] = gRZ[vi[c]];
				int32 u16, v16;
				_mesh->getTriangleUV(ti, (uint)c, u16, v16);
				tu[c] = u16 / 65536.0;
				tv[c] = v16 / 65536.0;
			}
			fillTriangleTex(dst, zbuf.begin(), sx, sy, sd, cz, tu, tv,
			                *_tex, kMouthShadeLevel);
			continue;
		}

		// Flat shade from the rotated triangle normal.
		const double e1x = gRX[i1] - gRX[i0];
		const double e1y = gRY[i1] - gRY[i0];
		const double e1z = gRZ[i1] - gRZ[i0];
		const double e2x = gRX[i2] - gRX[i0];
		const double e2y = gRY[i2] - gRY[i0];
		const double e2z = gRZ[i2] - gRZ[i0];
		const double nz = e1x * e2y - e1y * e2x;
		const double nx = e1y * e2z - e1z * e2y;
		const double ny = e1z * e2x - e1x * e2z;
		double nlen = sqrt(nx*nx + ny*ny + nz*nz);
		if (nlen < 1.0) nlen = 1.0;
		double lit = (nz < 0 ? -nz : nz) / nlen;
		lit = 0.35 + 0.65 * lit;
		const uint8 r  = (uint8)(70 + 150 * lit);
		const uint8 g  = (uint8)(45 + 110 * lit);
		const uint8 bl = (uint8)(35 + 80 * lit);
		const uint32 col = fmt.RGBToColor(r, g, bl);
		fillTriangleZ(dst, zbuf.begin(),
		              gSX[i0], gSY[i0], gSD[i0],
		              gSX[i1], gSY[i1], gSD[i1],
		              gSX[i2], gSY[i2], gSD[i2], col);
		// Close hairline cracks between adjacent faces by stroking the
		// 3 edges.
		drawLineZ(dst, zbuf.begin(),
		          gSX[i0], gSY[i0], gSD[i0],
		          gSX[i1], gSY[i1], gSD[i1], col);
		drawLineZ(dst, zbuf.begin(),
		          gSX[i1], gSY[i1], gSD[i1],
		          gSX[i2], gSY[i2], gSD[i2], col);
		drawLineZ(dst, zbuf.begin(),
		          gSX[i2], gSY[i2], gSD[i2],
		          gSX[i0], gSY[i0], gSD[i0], col);
	}
}

} // namespace Atlantis
} // namespace CryOmni3D
