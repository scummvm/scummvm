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
#include "tetraedge/te/te_tiled_texture.h"
#include "tetraedge/te/te_intrusive_ptr.h"

namespace Tetraedge {

class TeModelAnimation;

class TeModel : public Te3DObject2, public TeResource {
public:
	TeModel();

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
	private:
		Common::String _name;
		uint _meshNo;
		float _amount;
		TeTimer _timer;
	};

	struct bone {
		Common::String _name;
		unsigned short _x;
		TeTRS _trs;
	};
	struct weightElement {
		float _w;
		unsigned short _x;
	};

	void addMesh(const TeMesh &mesh) {
		_meshes.push_back(mesh);
	}
	TeIntrusivePtr<TeModelAnimation> anim() {
		return _modelAnim;
	}

	void blendAnim(TeIntrusivePtr<TeModelAnimation>& anim, float amount, bool repeat);
	void blendMesh(const Common::String &s1, const Common::String &s2, float amount);

	int checkFileType(Common::SeekableReadStream &instream);

	void draw() override;
	virtual void setColor(const TeColor &col) override;

	/* Align the stream to the nearest 4 byte boudary*/
	static void loadAlign(Common::SeekableReadStream &stream);
	static void saveAlign(Common::SeekableWriteStream &stream);

	bool load(const Common::Path &path);
	bool load(Common::SeekableReadStream &stream);

	bool loadWeights(Common::ReadStream &stream, Common::Array<weightElement> weights);
	bool loadMesh(Common::SeekableReadStream &stream, TeMesh &mesh);

	void update();
	void removeAnim();
	void setVisibleByName(const Common::String &name, bool vis);
	void setQuad(const TeIntrusivePtr<Te3DTexture> &tex, const Common::Array<TeVector3f32> &verts, const TeColor &col);

	void setAnim(TeIntrusivePtr<TeModelAnimation> &anim, bool repeat);

	static bool loadAndCheckString(Common::ReadStream &stream, const char *str);
	static Common::SeekableReadStream *tryLoadZlibStream(Common::SeekableReadStream &instr);
	TeIntrusivePtr<TeTiledTexture> _tiledTexture;

	Common::Path _texturePath;
	bool _enableLights;
	bool _skipBoneMatricies;

	Common::Array<TeMesh> _meshes;

protected:
	Common::Array<MeshBlender *> _meshBlenders;
	Common::Array<bone> _bones;
	Common::Array<TeMatrix4x4> _boneMatrices;
	Common::Array<Common::Array<weightElement>> _weightElements;
	Common::Array<BonesBlender *> _boneBlenders;

	TeIntrusivePtr<TeModelAnimation> _modelAnim;
	TeIntrusivePtr<TeModelVertexAnimation> _modelVertexAnim;
	// TODO add private members

};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_MODEL_H
