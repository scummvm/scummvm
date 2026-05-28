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

#ifndef CRYOMNI3D_ATLANTIS_F3DC_RENDERER_H
#define CRYOMNI3D_ATLANTIS_F3DC_RENDERER_H

#include "common/scummsys.h"

namespace Graphics {
struct Surface;
}

namespace CryOmni3D {
namespace Atlantis {

class F3dcMesh;
class F3dcMouthMesh;
class F3dcMouthAnim;
class F3dcSmallAnim;
class F3dcTextureMap;

// Renders the animated dialog "talking head" mouth mesh into the chroma-key
// hole of an NPC's UBB talking-head video.  Mirrors the original engine's
// per-frame draw routine (atlantis.exe FUN_0044e2e0): project the per-character
// 3D mouth mesh through the cam-specific look-at camera and software-rasterise
// it over the UBB frame.
//
// The three parsed source files are supplied to init():
//   - F3dcMouthMesh : .3DC bind-pose vertices + triangle topology (the
//                     "fvi" scene-graph node) + the per-character cam offset
//   - F3dcMesh      : .S3D per-camera look-at transform
//   - F3dcMouthAnim : .3DA "M" per-frame mouth vertex animation (the lip-sync
//                     visemes; frame 0 is the rest pose)
// All three are caller-owned and must outlive the renderer.
class F3dcMouthRenderer {
public:
	F3dcMouthRenderer();

	// Bind the parsed dialog-mesh files for camera `camNum` (2..5) and
	// derive the projection.  `tex` is the per-character `.3DM` face
	// texture (may be nullptr — the renderer then flat-shades).  Returns
	// true when there is a mesh to draw.
	bool init(const F3dcMouthMesh &mesh, const F3dcMesh &s3d,
	          const F3dcMouthAnim &anim, const F3dcTextureMap *tex,
	          int camNum, const F3dcSmallAnim *smallAnim = nullptr);

	bool isReady() const { return _ready; }

	// True when per-frame .3DA-M lip-sync animation is wired (the .3DA-M
	// loaded and its vertex order matched the .3DC bind pose).  When false
	// renderFrame() falls back to the static .3DC bind pose.
	bool isAnimated() const { return _animated; }

	// True when the mouth mesh is texture-mapped (a `.3DM` texture and
	// `.3DC` UVs were both available).  When false renderFrame() flat-shades.
	bool isTextured() const { return _textured; }

	// Rasterise the mouth mesh into `dst`.  `ubbFrame` advances once per
	// fresh UBB video frame and selects the lip-sync viseme.  `elapsedMs`
	// is the time since the dialog line started (sub-frame interpolation).
	// Texture colours come from the `.3DM` shade table — no external palette.
	void renderFrame(Graphics::Surface &dst, uint ubbFrame, uint32 elapsedMs);

private:
	void deriveCamera(int camNum);
	// Find the integer index shift between .3DC mesh vertex N and the
	// matching .3DA-M frame-0 vertex.  Returns the shift, or -1 if the two
	// vertex sets do not correspond (animation then stays disabled).
	int  detectVertexOffset() const;

	const F3dcMouthMesh *_mesh;
	const F3dcMesh      *_s3d;
	const F3dcMouthAnim *_anim;
	const F3dcSmallAnim *_smallAnim;
	const F3dcTextureMap *_tex;
	bool  _ready;
	bool  _animated;
	bool  _textured;
	int   _vertOffset;     // .3DA-M index = .3DC mesh index + _vertOffset

	bool   _camHasXform;
	double _camRot[9];      // model->camera view rotation M (row-major)
	double _camChainRot[9]; // scene-graph chain rotation R_world
	double _camP[3];        // camera translation P (with scene-graph offset)
	double _camPBase[3];    // _camP without the scene-graph offset O --
	                        // per-frame head-sway uses this + .3DA-A's O
	double _camFovDeg;      // field of view, degrees
};

} // namespace Atlantis
} // namespace CryOmni3D

#endif // CRYOMNI3D_ATLANTIS_F3DC_RENDERER_H
