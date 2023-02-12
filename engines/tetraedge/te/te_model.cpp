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

#include "common/file.h"
#include "common/util.h"
#include "common/substream.h"
#include "common/compression/zlib.h"

#include "tetraedge/tetraedge.h"
#include "tetraedge/te/te_light.h"
#include "tetraedge/te/te_model.h"
#include "tetraedge/te/te_model_animation.h"
#include "tetraedge/te/te_renderer.h"
#include "tetraedge/te/te_trs.h"

namespace Tetraedge {

TeModel::TeModel() : _enableLights(false), _skipSkinOffsets(false), _matrixForced(false) {
	_modelAnim.setDeleteFn(&TeModelAnimation::deleteLaterStatic);
	_modelVertexAnim.setDeleteFn(&TeModelVertexAnimation::deleteLaterStatic);
	create();
}

TeModel::~TeModel() {
	destroy();
}

void TeModel::blendAnim(TeIntrusivePtr<TeModelAnimation> &anim, float seconds, bool repeat) {
	if (!_modelAnim) {
		setAnim(anim, repeat);
	} else {
		BonesBlender *blender = new BonesBlender(anim, seconds);
		anim->_repeatCount = (repeat ? -1 : 1);
		anim->play();
		_boneBlenders.push_back(blender);
	}
}

void TeModel::blendMesh(const Common::String &s1, const Common::String &s2, float amount) {
	_meshBlenders.push_back(new MeshBlender(s1, s2, amount, this));
}

int TeModel::checkFileType(Common::SeekableReadStream &instream) const {
	char buf[4];
	instream.seek(0);
	int sz = instream.read(buf, 4);
	instream.seek(0);
	if (sz == 4 && strncmp("TEMD", buf, 4) == 0) {
		return 1;
	} else if (sz == 4 && strncmp("TEAN", buf, 4) == 0) {
		return 2;
	}
	return 0;
}

void TeModel::create() {
	// TODO: set field_0x158 to 0
	_modelAnim.release();
	_modelVertexAnim.release();
	_matrixForced = false;
	_skipSkinOffsets = false;
}

void TeModel::destroy() {
	_weightElements.clear();
	_lerpedElements.clear();
	_meshes.clear();
	_bones.clear();
	_boneMatricies.clear();
	_skinOffsets.clear();
	for (MeshBlender *blender : _meshBlenders)
		delete blender;
	_meshBlenders.clear();
	for (BonesBlender *blender : _boneBlenders)
		delete blender;
	_boneBlenders.clear();
}

void TeModel::draw() {
	TeRenderer *renderer = g_engine->getRenderer();

	if (worldVisible()) {
		const TeMatrix4x4 transform = transformationMatrix();
		renderer->sendModelMatrix(transform);
		renderer->pushMatrix();
		renderer->multiplyMatrix(transform);
		/*
		if (name().contains("DEPLIANT")) {
			debug("Draw model %p (%s, %d meshes)", this, name().empty() ? "no name" : name().c_str(), _meshes.size());
			debug("   renderMatrix %s", renderer->currentMatrix().toString().c_str());
			//debug("   position   %s", position().dump().c_str());
			debug("   worldPos   %s", worldPosition().dump().c_str());
			//debug("   scale      %s", scale().dump().c_str());
			debug("   worldScale %s", worldScale().dump().c_str());
			//debug("   rotation   %s", rotation().dump().c_str());
			debug("   worldRot   %s", worldRotation().dump().c_str());
		}*/
		for (auto &mesh : _meshes) {
			// TODO: Set some flag (_drawWires?) in mesh to this->field_0x158??
			mesh->draw();
		}
		renderer->popMatrix();
		// Note: no corresponding enableAll - they can be enabled inside TeMaterial::apply
		renderer->disableAllLights();
	}
}

void TeModel::forceMatrix(const TeMatrix4x4 &matrix) {
	_matrixForced = true;
	_forcedMatrix = matrix;
}

TeTRS TeModel::getBone(TeIntrusivePtr<TeModelAnimation> anim, uint num) {
	if (anim) {
		int boneNo = anim->findBone(_bones[num]._name);
		if (boneNo != -1)
			return anim->getTRS(boneNo, anim->curFrame2(), false);
	}
	return _bones[num]._trs;
}

TeMatrix4x4 TeModel::lerpElementsMatrix(uint weightsNum, const Common::Array<TeMatrix4x4> &matricies) {
	TeMatrix4x4 retval;
	// Start with a 0 matrix.
	for (uint i = 0; i < 4; i++)
		retval.setValue(i, i, 0);

	const Common::Array<weightElement> &weights = _weightElements[weightsNum];
	for (const auto &weight : weights) {
		const TeMatrix4x4 offset = matricies[weight._x].meshScale(weight._weight);
		retval.meshAdd(offset);
	}

	return retval;
}

void TeModel::removeAnim() {
	for (TeModel::BonesBlender *blender : _boneBlenders) {
		delete blender;
	}
	_boneBlenders.clear();
	_modelAnim.release();
}

void TeModel::setColor(const TeColor &col) {
	Te3DObject2::setColor(col);
	for (auto &mesh : _meshes) {
		mesh->setColor(col);
	}
}

void TeModel::update() {
	//if (name().contains("Kate"))
	//	debug("TeModel::update model %s", name().c_str());
	if (_bones.size()) {
		Common::Array<TeMatrix4x4> matricies;
		matricies.resize(_bones.size());
		for (uint i = 0; i < _bones.size(); i++) {
			const Bone &b = _bones[i];
			const TeMatrix4x4 matrix = TeMatrix4x4::fromTRS(b._trs);
			if (b._parentBone == -1 || _bones.size() < 2) {
				matricies[0] = matrix;
			} else {
				matricies[i] = matricies[b._parentBone] * matrix;
			}
		}

		_boneMatricies.resize(_bones.size());
		_lerpedElements.resize(_weightElements.size());

		TeMatrix4x4 invertx;
		invertx.scale(TeVector3f32(-1, 1, 1));
		for (uint b = 0; b < _bones.size(); b++) {
			TeTRS trs = getBone(_modelAnim, b);
			for (uint i = 0; i < _boneBlenders.size(); i++) {
				BonesBlender *blender = _boneBlenders[i];
				float complete = blender->coef();
				TeTRS endTRS = getBone(blender->_anim, b);
				if (complete == 1.0f) {
					_modelAnim = blender->_anim;
					delete blender;
					_boneBlenders.remove_at(i);
					trs = endTRS;
					i--;
				} else {
					trs = trs.lerp(endTRS, complete);
				}
			}

			TeMatrix4x4 newBoneMatrix;
			if (!_matrixForced) {
				newBoneMatrix = TeMatrix4x4::fromTRS(trs);
			} else {
				newBoneMatrix = invertx * _forcedMatrix;
				_matrixForced = false;
			}

			if (_bones.size() < 2 || _bones[b]._parentBone == -1) {
				_boneMatricies[b] = newBoneMatrix;
				_boneMatricies[b].rotate(_boneRotation);
			} else {
				_boneMatricies[b] = (invertx * _boneMatricies[_bones[b]._parentBone]) * newBoneMatrix;
			}
			_boneMatricies[b] = invertx * _boneMatricies[b];
			_bonesUpdatedSignal.call(_bones[b]._name, _boneMatricies[b]);
		}

		if (!_skinOffsets.empty() && !_bones.empty()) {
			for (uint b = 0; b < _bones.size(); b++) {
				_boneMatricies[b] = _boneMatricies[b] * _skinOffsets[b];
			}
		}

		if (!_skipSkinOffsets && !_weightElements.empty()) {
			for (uint i = 0; i < _weightElements.size(); i++) {
				_lerpedElements[i] = lerpElementsMatrix(i, _boneMatricies);
			}
		}

		for (uint m = 0; m < _meshes.size(); m++) {
			TeMesh &mesh = *_meshes[m];
			if (!mesh.visible())
				continue;

			if (!_skipSkinOffsets && _bones.size() < 2 ) {
				if (_modelVertexAnim && mesh.name() == _modelVertexAnim->head()) {
					mesh.update(_modelVertexAnim);
					// TODO: lines 422 - 427.. set some vals and goto LAB_doMeshBlends;
				}
				mesh.update(&_boneMatricies, &_lerpedElements);
			} else {
				mesh.resizeUpdatedTables(mesh.numVerticies());
				Common::Array<TeVector3f32> verticies;
				if (_modelVertexAnim && mesh.name() == _modelVertexAnim->head())
					verticies = _modelVertexAnim->getVertices();

				for (uint i = 0; i < mesh.numVerticies(); i++) {
					TeVector3f32 vertex;
					if (verticies.empty()) {
						vertex = mesh.preUpdatedVertex(i);
					} else {
						if (i < verticies.size())
							vertex = verticies[i];
					}
					TeVector3f32 normal = mesh.preUpdatedNormal(i);
					int idx = (int)mesh.matrixIndex(i);

					TeVector3f32 updatedvertex;
					TeVector3f32 updatednormal;

					if (idx < (int)_bones.size()) {
						updatedvertex = vertex;
						if (verticies.empty())
							updatedvertex = _boneMatricies[idx] * updatedvertex;
						updatednormal = _boneMatricies[idx] * normal;
					} else {
						idx -= _bones.size();
						for (uint w = 0; w < _weightElements[idx].size(); w++) {
							const TeMatrix4x4 &wmatrix = _boneMatricies[_weightElements[idx][w]._x];
							float weight = _weightElements[idx][w]._weight;
							updatedvertex = updatedvertex + ((wmatrix * vertex) * weight);
							updatednormal = updatednormal + (wmatrix.mult3x3(normal) * weight);
						}
					}

					mesh.setUpdatedVertex(i, updatedvertex);
					mesh.setUpdatedNormal(i, updatednormal);
				}
			}

			for (MeshBlender *mb : _meshBlenders) {
				if (mesh.name().contains(mb->_name)) {
					error("TODO: Finish meshblend part of model::update");
					// TODO: Finish TeModel::update. (disasm 585 ~ 644), LAB_doMeshBlends
					//float blendamount = MIN(mb->_timer.getTimeFromStart() / 1000000.0f, 1.0f);

				}
			}
		}
	} else {
		// No bones..
		for (auto &mesh : _meshes) {
			if (!_modelVertexAnim) {
				mesh->update(nullptr, nullptr);
			} else {
				if (mesh->name() != _modelVertexAnim->head()) {
					mesh->update(nullptr, nullptr);
				} else {
					mesh->update(_modelVertexAnim);
				}
			}
		}
	}
}

TeModel::MeshBlender::MeshBlender(const Common::String &name, const Common::String &meshName, float amount, TeModel *model) :
_name(name), _amount(amount) {
	const auto &meshes = model->_meshes;
	uint i = 0;
	for (; i < meshes.size(); i++) {
		if (meshes[i]->name().contains(meshName))
			break;
	}
	_meshNo = i;
	_timer.start();
}

/*static*/
void TeModel::loadAlign(Common::SeekableReadStream &stream) {
	int64 pos = stream.pos();
	if (pos % 4) {
		stream.seek(4 - (pos % 4), SEEK_CUR);
	}
}

/*static*/
void TeModel::saveAlign(Common::SeekableWriteStream &stream) {
	int64 pos = stream.pos();
	if (pos % 4) {
		stream.seek(4 - (pos % 4), SEEK_CUR);
	}
}

bool TeModel::load(Common::SeekableReadStream &stream) {
	destroy();
	create();

	if (!loadAndCheckFourCC(stream, "TEMD")) {
		error("[TeModel::load] Unknown format.");
	}

	uint version = stream.readUint32LE();
	if (!((version == 11) || (version == 13))) {
		error("[TeModel::load] Unsupported version %d", version);
	}

	uint32 meshCount = stream.readUint32LE();
	if (meshCount > 100000)
		error("TeModel::load: Unexpected number of meshes %d", meshCount);
	_meshes.resize(meshCount);
	for (uint i = 0; i < meshCount; i++)
		_meshes[i].reset(TeMesh::makeInstance());

	uint32 weightCount = stream.readUint32LE();
	if (weightCount > 100000)
		error("TeModel::load: Unexpected number of weights %d", weightCount);
	_weightElements.resize(weightCount);
	uint32 bonecount = stream.readUint32LE();
	if (bonecount > 100000)
		error("TeModel::load: Unexpected number of bones %d", bonecount);
	_bones.resize(bonecount);
	_skinOffsets.resize(bonecount);

	if (version == 13) {
		_skipSkinOffsets = stream.readUint32LE();
	}

	if (!loadAndCheckFourCC(stream, "SKEL")) {
		error("[TeModel::load] Unable to find skeleton.");
	}

	for (uint i = 0; i < _bones.size(); i++) {
		_bones[i]._name = Te3DObject2::deserializeString(stream);
		loadAlign(stream);
		_bones[i]._parentBone = stream.readUint32LE();
		TeTRS::deserialize(stream, _bones[i]._trs);
		if (!_skipSkinOffsets) {
			_skinOffsets[i].deserialize(stream);
		}
	}

	for (uint m = 0; m < _meshes.size(); m++) {
		if (!loadMesh(stream, *_meshes[m])) {
			error("[TeModel::load] Error on meshes loading.");
		}
	}

	if (!loadAndCheckFourCC(stream, "WEIG")) {
		error("[TeModel::load] Unable to load weight.");
	}
	for (uint i = 0; i < _weightElements.size(); i++) {
		loadWeights(stream, _weightElements[i]);
	}

	if (_bones.empty())
		_bones.resize(1);
	return true;
}

bool TeModel::load(const Common::Path &path) {
	Common::File modelFile;
	if (!modelFile.open(path)) {
		warning("[TeModel::load] Can't open file : %s.", path.toString().c_str());
		return false;
	}

	bool retval;
	if (loadAndCheckFourCC(modelFile, "TEZ0")) {
		Common::SeekableReadStream *zlibStream = tryLoadZlibStream(modelFile);
		if (!zlibStream)
			return false;
		retval = load(*zlibStream);
		delete zlibStream;
	} else {
		modelFile.seek(0);
		retval = load(modelFile);
	}
	return retval;
}

Common::SeekableReadStream *TeModel::tryLoadZlibStream(Common::SeekableReadStream &stream) {
	byte version = stream.readByte();
	if (version != 1) {
		warning("[TeModel::load] invalid version number %d (expect 1)", version);
		return nullptr;
	}
	uint32 compressedSize = stream.readUint32LE();
	if (compressedSize > stream.size()) {
		warning("[TeModel::load] invalid size %d (file size %d)", compressedSize, (int)stream.size());
		return nullptr;
	}
	uint32 uncompressedSize = stream.readUint32LE();
	Common::SeekableSubReadStream *substream = new Common::SeekableSubReadStream(&stream, stream.pos(), stream.size());
	return Common::wrapCompressedReadStream(substream, uncompressedSize);
}

bool TeModel::loadWeights(Common::ReadStream &stream, Common::Array<weightElement> &weights) {
	uint32 nweights = stream.readUint32LE();
	if (nweights > 100000)
		error("Improbable number of weights %d", (int)nweights);
	weights.resize(nweights);
	for (uint i = 0; i < nweights; i++) {
		weights[i]._weight = stream.readFloatLE();
		weights[i]._x = stream.readUint16LE();
		stream.readUint16LE();
	}
	return true;
}

bool TeModel::loadMesh(Common::SeekableReadStream &stream, TeMesh &mesh) {
	if (!loadAndCheckFourCC(stream, "MESH"))
		return false;

	uint32 vertcount = stream.readUint32LE();
	uint32 matcount = stream.readUint32LE();
	uint32 matidxcount = stream.readUint32LE();
	uint32 idxcount = stream.readUint32LE();

	if (vertcount > 100000 || matcount > 100000 || matidxcount > 100000 || idxcount > 100000)
		error("Improbable mesh sizes %d %d %d %d", vertcount, matcount, matidxcount, idxcount);

	mesh.setConf(vertcount, idxcount, TeMesh::MeshMode_Triangles, matcount, matidxcount);

	uint32 flags = stream.readUint32LE();
	if (flags & 1)
		mesh.setColor(0, TeColor(0xff, 0xff, 0xff, 0xff));
	if (flags & 2)
		mesh.setTextureUV(0, TeVector2f32(0.0f, 0.0f));

	mesh.setName(Te3DObject2::deserializeString(stream));
	loadAlign(stream);

	if (!loadAndCheckFourCC(stream, "MTRL"))
		return false;

	for (uint i = 0; i < mesh.materials().size(); i++) {
		TeMaterial mat;
		TeMaterial::deserialize(stream, mat, _texturePath);
		if (_enableLights)
			mat._enableLights = true;
		mesh.attachMaterial(i, mat);
	}

	if (!loadAndCheckFourCC(stream, "VERT"))
		return false;

	for (uint i = 0; i < mesh.numVerticies(); i++) {
		TeVector3f32 v;
		TeVector3f32::deserialize(stream, v);
		mesh.setVertex(i, v);
	}
	if (mesh.hasUvs()) {
		if (!loadAndCheckFourCC(stream, "TUVS"))
			return false;
		for (uint i = 0; i < mesh.numVerticies(); i++) {
			TeVector2f32 v;
			TeVector2f32::deserialize(stream, v);
			mesh.setTextureUV(i, v);
		}
	}

	if (!loadAndCheckFourCC(stream, "NORM"))
		return false;

	for (uint i = 0; i < mesh.numVerticies(); i++) {
		TeVector3f32 v;
		TeVector3f32::deserialize(stream, v);
		mesh.setNormal(i, v);
	}

	if (mesh.hasColor()) {
		if (!loadAndCheckFourCC(stream, "COLS"))
			return false;

		for (uint i = 0; i < mesh.numVerticies(); i++) {
			TeColor c;
			c.deserialize(stream);
			mesh.setColor(i, c);
		}
	}

	if (!loadAndCheckFourCC(stream, "FCPM"))
		return false;

	for (uint i = 0; i < mesh.materials().size(); i++) {
		mesh.facesPerMaterial(i, stream.readUint16LE());
	}

	loadAlign(stream);
	if (!loadAndCheckFourCC(stream, "MTXI"))
		return false;

	for (uint i = 0; i < mesh.numVerticies(); i++) {
		mesh.matrixIndex(i, stream.readUint16LE());
	}

	loadAlign(stream);
	if (!loadAndCheckFourCC(stream, "IDXS"))
		return false;

	for (uint i = 0; i < mesh.numIndexes(); i++) {
		mesh.setIndex(i, stream.readUint16LE());
	}

	loadAlign(stream);
	return true;
}

void TeModel::setQuad(const TeIntrusivePtr<Te3DTexture> &tex, const Common::Array<TeVector3f32> &verts, const TeColor &col) {
	Common::SharedPtr<TeMesh> mesh(TeMesh::makeInstance());
	mesh->setConf(4, 4, TeMesh::MeshMode_TriangleStrip, 0, 0);
	mesh->defaultMaterial(tex);

	for (int i = 0; i < 2; i++) {
		float f = (i == 0 ? 0.0f : 1.0f);
		for (int j = 0; j < 2; j++) {
			int index = i * 2 + j;
			mesh->setVertex(index, verts[i * 2 + j]);
			mesh->setTextureUV(index, TeVector2f32(f, (j == 0 ? 0.0f : 1.0f)));
			mesh->setIndex(index, index);
			if (col.a() != 0)
				mesh->setColor(index, col);
		}
	}

	const TeVector3f32 v1 = verts[1] - verts[0];
	const TeVector3f32 v2 = verts[2] - verts[0];
	TeVector3f32 v3 = TeVector3f32::crossProduct(v1, v2);
	v3.normalize();
	for (int i = 0; i < 4; i++) {
		mesh->setNormal(i, v3);
	}
	_meshes.push_back(mesh);
}

void TeModel::setAnim(TeIntrusivePtr<TeModelAnimation> &anim, bool repeat) {
	for (TeModel::BonesBlender *blender : _boneBlenders) {
		delete blender;
	}
	_boneBlenders.clear();
	anim->_repeatCount = (repeat ? -1 : 1);
	_modelAnim = anim;
}

void TeModel::setVertexAnim(TeIntrusivePtr<TeModelVertexAnimation> &anim, bool repeat) {
	anim->_repeatCount = (repeat ? -1 : 1);
	_modelVertexAnim = anim;
}

void TeModel::setVisibleByName(const Common::String &name, bool vis) {
	for (auto &mesh : _meshes) {
		if (mesh->name().contains(name)) {
			mesh->setVisible(vis);
		}
	}
}

TeMatrix4x4 TeModel::skinOffset(uint boneno) const {
	if (boneno >= _skinOffsets.size())
		return TeMatrix4x4();
	return _skinOffsets[boneno];
}

void TeModel::setMeshCount(uint count) {
	assert(count < 100000);
	while (_meshes.size() < count)
		_meshes.push_back(Common::SharedPtr<TeMesh>(TeMesh::makeInstance()));

	if (_meshes.size() > count)
		_meshes.resize(count);
}

TeModel::BonesBlender::BonesBlender(TeIntrusivePtr<TeModelAnimation> anim, float seconds) : _anim(anim), _seconds(seconds) {
	_anim.setDeleteFn(&TeModelAnimation::deleteLaterStatic);
	_timer.stop();
	_timer.start();
}

float TeModel::BonesBlender::coef() {
	float elapsed = (_timer.getTimeFromStart() / 1000000.0) / _seconds;
	return MIN(elapsed, 1.0f);
}

} // end namespace Tetraedge
