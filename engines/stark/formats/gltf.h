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

#ifndef STARK_FORMATS_GLTF_H
#define STARK_FORMATS_GLTF_H

#include "common/stream.h"

namespace Stark {

class Model;
class SkeletonAnim;

namespace Formats {

/**
 * Loader for skinned actor models in glTF 2.0 binary form (.glb).
 *
 * This is a modding format: replacement models exported from Blender (or
 * produced by the cir_obj.py companion tool) are converted at load time
 * into the same Model / VertNode structures the .cir loader produces, so
 * the renderer and the animation system are unaffected.
 *
 * Requirements on the file:
 *  - one skinned mesh, joints named "bone_NN" matching the original
 *    skeleton indices (falling back to joint order if names don't parse)
 *  - per-vertex JOINTS_0 / WEIGHTS_0; the two strongest influences are
 *    kept and renormalized, matching the engine's two-bone rig
 *  - triangle primitives with POSITION, NORMAL and TEXCOORD_0
 *  - materials named like the original .cir materials; the base color
 *    texture's image name selects the texture within the actor's
 *    texture set
 *
 * Only the subset of glTF that Blender exports is supported: a single
 * buffer, non-sparse accessors, float / unsigned byte / short / int
 * component types.
 */
class GltfModelReader {
public:
	/**
	 * Populate model from a GLB stream.
	 * Returns false (leaving model untouched or partially built) on failure.
	 */
	static bool load(Model *model, Common::SeekableReadStream *stream);
};

/**
 * Loader for skeletal animations in glTF 2.0 binary form (.glb).
 *
 * Replaces an original .ani: the clip's rotation / translation channels
 * are mapped to skeleton bones through the "bone_NN" node names and
 * resampled into the engine's per-bone keyframe representation.
 *
 * If the file holds several clips, the one named like the .ani being
 * replaced (without extension) is used; otherwise the first clip is.
 */
class GltfAnimReader {
public:
	/**
	 * Populate anim from a GLB stream.
	 * clipName is the .ani basename without extension, used to select the clip.
	 * Returns false on failure.
	 */
	static bool load(SkeletonAnim *anim, Common::SeekableReadStream *stream,
	                 const Common::String &clipName);
};

} // End of namespace Formats
} // End of namespace Stark

#endif // STARK_FORMATS_GLTF_H
