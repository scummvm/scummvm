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

#ifndef TETRAEDGE_TE_TE_MODEL_H
#define TETRAEDGE_TE_TE_MODEL_H

#include "common/array.h"
#include "common/ptr.h"
#include "common/stream.h"
#include "tetraedge/te/te_timer.h"
#include "tetraedge/te/te_trs.h"
#include "tetraedge/te/te_mesh.h"
#include "tetraedge/te/te_model_animation.h"
#include "tetraedge/te/te_model_vertex_animation.h"
#include "tetraedge/te/te_tiled_texture.h"
#include "tetraedge/te/te_intrusive_ptr.h"
#include "tetraedge/te/te_quaternion.h"
#include "tetraedge/te/te_resource.h"

namespace Tetraedge {

class TeModelVertexAnimation;
class TeModelAnimation;
class TeMesh;

class TeModel : public Te3DObject2, public TeResource {
public:
	class BonesBlender {
	public:
		// Note: original takes a TeModel & but ignores it.
		BonesBlender(TeIntrusivePtr<TeModelAnimation> anim, float seconds);
		float coef();

		TeIntrusivePtr<TeModelAnimation> _anim;
		TeTimer _timer;
		float _seconds;
	};

	class MeshBlender {
	public:
		MeshBlender(const Common::String &s1, const Common::String &s2, float amount, TeModel *model);
		Common::String _name;
		uint _meshNo;
		float _amount;
		TeTimer _timer;
	};

	struct Bone {
		Common::String _name;
		short _parentBone;
		TeTRS _trs;
	};

	struct weightElement {
		float _weight;
		unsigned short _x;
	};

	TeModel();
	virtual ~TeModel();

	void addMesh(Common::SharedPtr<TeMesh> mesh) {
		_meshes.push_back(mesh);
	}
	TeIntrusivePtr<TeModelAnimation> anim() {
		return _modelAnim;
	}

	void blendAnim(TeIntrusivePtr<TeModelAnimation>& anim, float amount, bool repeat);
	void blendMesh(const Common::String &s1, const Common::String &s2, float amount);

	int checkFileType(Common::SeekableReadStream &instream) const;

	void create();
	void destroy();

	void draw() override;

	int findModelBone(const Common::String &bname);
	int findOrAddWeights(const Common::Array<weightElement> &weights);
	void forceMatrix(const TeMatrix4x4 &matrix);
	TeTRS getBone(TeIntrusivePtr<TeModelAnimation> anim, uint num);
	void invertNormals();

	/* Align the stream to the nearest 4 byte boudary*/
	static void loadAlign(Common::SeekableReadStream &stream);
	static void saveAlign(Common::SeekableWriteStream &stream);

	bool load(const Common::Path &path);
	bool load(Common::SeekableReadStream &stream);

	bool loadWeights(Common::ReadStream &stream, Common::Array<weightElement> &weights);
	bool loadMesh(Common::SeekableReadStream &stream, TeMesh &mesh);

	void removeAnim();
	void update();

	void saveBone(Common::SeekableWriteStream &stream, uint boneno);
	void saveMesh(Common::SeekableWriteStream &stream, const TeMesh &mesh);
	void saveModel(Common::SeekableWriteStream &stream, uint num);
	void saveWeights(Common::SeekableWriteStream &stream, const Common::Array<weightElement> weights);

	void setAnim(TeIntrusivePtr<TeModelAnimation> &anim, bool repeat);
	virtual void setColor(const TeColor &col) override;
	void setQuad(const TeIntrusivePtr<Te3DTexture> &tex, const Common::Array<TeVector3f32> &verts, const TeColor &col);
	void setVertexAnim(TeIntrusivePtr<TeModelVertexAnimation> &anim, bool repeat);
	void setVisibleByName(const Common::String &mname, bool vis);

	TeMatrix4x4 skinOffset(uint boneno) const;

	static Common::SeekableReadStream *tryLoadZlibStream(Common::SeekableReadStream &instr);

	TeSignal2Param<const Common::String &, TeMatrix4x4 &> &bonesUpdatedSignal() { return _bonesUpdatedSignal; }
	Common::Array<BonesBlender *> &boneBlenders() { return _boneBlenders; }
	Common::Array<Common::SharedPtr<TeMesh>> &meshes() { return _meshes; }
	TeIntrusivePtr<TeTiledTexture> tiledTexture() { return _tiledTexture; }

	void setEnableLights(bool val) { _enableLights = val; }
	void setTexturePath(const Common::String &path) { _texturePath = path; }
	void setMeshCount(uint count);

protected:
	TeMatrix4x4 lerpElementsMatrix(uint weightNum, const Common::Array<TeMatrix4x4> &matricies);
	void optimize();

	Common::String _texturePath;
	TeIntrusivePtr<TeTiledTexture> _tiledTexture;

	bool _enableLights;
	bool _matrixForced;
	bool _skipSkinOffsets;

	TeMatrix4x4 _forcedMatrix;
	Common::Array<BonesBlender *> _boneBlenders;
	Common::Array<MeshBlender *> _meshBlenders;
	Common::Array<Bone> _bones;
	Common::Array<TeMatrix4x4> _skinOffsets;
	Common::Array<TeMatrix4x4> _boneMatricies;
	Common::Array<TeMatrix4x4> _lerpedElements;
	Common::Array<Common::Array<weightElement>> _weightElements;
	Common::Array<Common::SharedPtr<TeMesh>> _meshes;

	TeQuaternion _boneRotation;

	TeIntrusivePtr<TeModelAnimation> _modelAnim;
	TeIntrusivePtr<TeModelVertexAnimation> _modelVertexAnim;

	TeSignal2Param<const Common::String &, TeMatrix4x4 &> _bonesUpdatedSignal;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_MODEL_H
