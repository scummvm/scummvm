/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Spriter - 3D Renderer
 */

#include "tinsel/noir/spriter.h"

#include "tinsel/handle.h"
#include "tinsel/tinsel.h"

#include "common/memstream.h"
#include "common/textconsole.h"
#include "common/rect.h"
#include "common/str.h"
#include "common/file.h"

#include "math/quat.h"

#if defined(USE_TINYGL)
#include "graphics/tinygl/tinygl.h"
#endif

namespace Tinsel {

#define PALETTE_COUNT 22
#define TEXTURE_COUNT 4
#define MERGE_VERTICES_OFFSET 0.1f // TODO: find the proper ratio, this one is perhaps too big?

static float ConvertAngle(uint32 angle) {
	return ((float(angle & 0xfff) / 4095.0f) * 360.0f);
}

Spriter::Spriter() {
	_textureGenerated = false;
	_sequencesCount = 0;
	_animId = 0;
	_animSpeed = 0;
}

Spriter::~Spriter() {
#if defined(USE_TINYGL)
	if (_textureGenerated) {
		tglDeleteTextures(TEXTURE_COUNT, _texture);
	}
#endif
}

const Math::Matrix4& Spriter::MatrixCurrent() const {
	return _currentMatrix->top();
}

void Spriter::MatrixReset() {
	_currentMatrix->clear();

	Math::Matrix4 m;
	_currentMatrix->push(m);
}

void Spriter::MatrixPop() {
	_currentMatrix->pop();
}

void Spriter::MatrixPush() {
	_currentMatrix->push(_currentMatrix->top());
}

void Spriter::MatrixTranslate(float x, float y, float z) {
	Math::Vector3d v {x,y,z};
	_currentMatrix->top().translate(v);
}

void Spriter::MatrixScale(float x, float y, float z) {
	Math::Matrix4 m;
	m.setValue(0, 0, x);
	m.setValue(1, 1, y);
	m.setValue(2, 2, z);
	_currentMatrix->top() = _currentMatrix->top() * m;
}

void Spriter::MatrixRotateX(float angle) {
	Math::Matrix4 &m = _currentMatrix->top();
	m = m * Math::Quaternion::xAxis(angle).toMatrix();
}

void Spriter::MatrixRotateY(float angle) {
	Math::Matrix4 &m = _currentMatrix->top();
	m = m * Math::Quaternion::yAxis(angle).toMatrix();
}

void Spriter::MatrixRotateZ(float angle) {
	Math::Matrix4 &m = _currentMatrix->top();
	m = m * Math::Quaternion::zAxis(angle).toMatrix();
}

void Spriter::SetViewport(int ap) {
	_view.viewport.ap = ap;

	int ratio = ((_view.screenRect.right - _view.screenRect.left) << 12) / ap;
	_view.viewport.width = ratio;
	_view.viewport.height = ratio;

	_view.viewport.ap = (ap + 1800) / 3600;

	_view.viewport.rect = _view.viewRect;
}

void Spriter::Init(int width, int height) {
	_currentMatrix = &_modelMatrix;
	_meshShadow.resize(50);

	_view.screenRect.left   = 0;
	_view.screenRect.top    = 0;
	_view.screenRect.right  = width;
	_view.screenRect.bottom = height;

	_view.centerX = width / 2;
	_view.centerY = height / 2;

	_view.viewRect.left   = -_view.centerX;
	_view.viewRect.top    = -_view.centerY;
	_view.viewRect.right  =  _view.screenRect.right  - _view.screenRect.left - _view.centerX - 1;
	_view.viewRect.bottom =  _view.screenRect.bottom - _view.screenRect.top  - _view.centerY;

	SetViewport(306030);
}

void Spriter::SetCamera(int rotX, int rotY, int rotZ, int posX, int posY, int posZ, int cameraAp) {
	_view.position.set(posX * 0.01f, posY * 0.01f, posZ * 0.01f);
	_view.rotation.set(ConvertAngle(rotX), ConvertAngle(rotY), ConvertAngle(rotZ));

	SetViewport(cameraAp);

	_modelIdle = true;
}

void Spriter::TransformSceneXYZ(int x, int y, int z, int& xOut, int& yOut) {
	MatrixReset();
	MatrixRotateX(_view.rotation.x());
	MatrixRotateY(_view.rotation.y());
	MatrixRotateZ(_view.rotation.z());
	MatrixTranslate(-_view.position.x(), -_view.position.y(), -_view.position.z());
	MatrixTranslate((float)x / 100.0f, (float)y / 100.0f, (float)z / 100.0f);

	Math::Vector3d v(0,0,0);

	MatrixCurrent().transform(&v, true);

	// Apply the viewport
	xOut = _view.centerX + v.x();
	yOut = _view.centerY + v.y();
}

void Spriter::LoadH(const Common::String& modelName) {
	Common::String filename = modelName + ".h";

	Common::File f;
	f.open(Common::Path(filename));

	while(!f.eos()) {
		Common::String line = f.readLine();
		if (!line.hasPrefix("#define")) {
			continue;
		}
		line.erase(0, 8); // remove "#define "

		size_t underscorePos = line.findFirstOf('_');

		AnimationInfo *anim = nullptr;

		Common::String name = line.substr(0, underscorePos);

		line.erase(0, name.size() + 1); // remove the underscore too

		if (name.hasPrefix("SHADOW")) {
			anim = &_animShadow;
		} else {
			for (Common::Array<AnimationInfo>::iterator it = _animMain.begin(); it != _animMain.end(); ++it) {
				if (it->name.equals(name)) {
					anim = it;
					break;
				}
			}

			if (anim == nullptr) {
				AnimationInfo tempAnim {};
				tempAnim.name = name;
				_animMain.push_back(tempAnim);
				anim = &_animMain.back();
			}
		}

		size_t spacePos = line.findFirstOf(' ');
		Common::String sub = line.substr(0, spacePos);
		auto val = atoi(line.substr(spacePos).c_str());

		if (sub.equals("MESH_NUM")) {
			anim->meshNum = val;
		} else if (sub.equals("SCALE_NUM")) {
			anim->scaleNum = val;
		} else if (sub.equals("TRANSLATE_NUM")) {
			anim->translateNum = val;
		} else if (sub.equals("ROTATE_NUM")) {
			anim->rotateNum = val;
		}
	}
}

void Spriter::LoadGBL(const Common::String& modelName) {
	Common::String filename = modelName + ".gbl";

	Common::File f;
	f.open(Common::Path(filename));

	while(!f.eos()) {
		Common::String line = f.readLine();
		if (!line.hasPrefix("#define")) {
			continue;
		}
		line.erase(0, 8); // remove "#define "

		size_t underscorePos = line.findFirstOf('_');

		AnimationInfo *anim = nullptr;
		MeshInfo *mesh = nullptr;

		Common::String name = line.substr(0, underscorePos);

		line.erase(0, name.size() + 1); // remove the underscore too

		uint meshId = 0;
		if (name.hasPrefix("SHADOW")) {
			anim = &_animShadow;
			meshId = atoi(name.substr(6).c_str());
			assert(meshId < _meshShadow.size());
			mesh = &_meshShadow[meshId];
		} else {
			for (Common::Array<AnimationInfo>::iterator it = _animMain.begin(); it != _animMain.end(); ++it) {
				if (it->name.equals(name)) {
					anim = it;
					break;
				}
			}
			mesh = &_meshMain;
		}

		assert(anim != nullptr);
		assert(mesh != nullptr);

		size_t spacePos = line.findFirstOf(' ');
		Common::String sub = line.substr(0, spacePos);
		auto val = atoi(line.substr(spacePos).c_str());

		if (sub.equals("MESH_TABLES")) {
			mesh->meshTables = val;
		} else if (sub.equals("MESH_TABLES_hunk")) {
			mesh->meshTablesHunk = val;
		} else if (sub.equals("RENDER_PROGRAM")) {
			mesh->program = val;
		} else if (sub.equals("RENDER_PROGRAM_hunk")) {
			mesh->programHunk = val;
		} else if (sub.equals("TRANSLATE_TABLES")) {
			anim->translateTables = val;
		} else if (sub.equals("TRANSLATE_TABLES_hunk")) {
			anim->translateTablesHunk = val;
		} else if (sub.equals("ROTATE_TABLES")) {
			anim->rotateTables = val;
		} else if (sub.equals("ROTATE_TABLES_hunk")) {
			anim->rotateTablesHunk = val;
		} else if (sub.equals("SCALE_TABLES")) {
			anim->scaleTables = val;
		} else if (sub.equals("SCALE_TABLES_hunk")) {
			anim->scaleTablesHunk = val;
		}
	}
}

#define kPIFF 0x46464950
#define kRBHF 0x46484252
#define kRBHH 0x48484252
#define kBODY 0x59444f42
#define kRELC 0x434c4552

void Spriter::LoadRBH(const Common::String& modelName, Hunks& hunks) {
	Common::String filename = modelName + ".rbh";

	Common::File f;
	f.open(Common::Path(filename));

	uint tag = f.readUint32LE();
	assert(tag == kPIFF);
	uint fileSize = f.readUint32LE();

	tag = f.readUint32LE();
	assert(tag == kRBHF);
	tag = f.readUint32LE();
	assert(tag == kRBHH);
	uint headerSize = f.readUint32LE();
	uint entriesCount = headerSize / 12;

	hunks.resize(entriesCount);
	for (Hunks::iterator it = hunks.begin(); it != hunks.end(); ++it) {
		f.skip(4); // pointer to data
		it->size = f.readUint32LE();
		it->flags = f.readUint16LE();
		f.skip(2); // padding
		it->data.resize(it->size);
	}

	uint entryIdx = 0;
	while (f.pos() < fileSize) {
		tag = f.readUint32LE();
		uint size = f.readUint32LE();
		if (tag == kBODY) {
			f.read(hunks[entryIdx].data.data(), size);
			++entryIdx;
		} else if (tag == kRELC) {
			uint srcIdx = f.readUint32LE();
			uint dstIdx = f.readUint32LE();
			f.skip(size - 8); // ScummVM's implementation does not need to read the offsets for relocation
			hunks[srcIdx].mappingIdx.push_back(dstIdx);
		} else {
			assert(false);
		}
	}
}

void Spriter::LoadVMC(const Common::String& textureName) {
	Common::String filename = textureName + ".vmc";

	Common::File f;
	f.open(Common::Path(filename));

	int16 buffer[4];

	_textureData.resize(4 * 256 * 256);

	while (true) {
		buffer[0] = f.readSint16LE();
		buffer[1] = f.readSint16LE();
		buffer[2] = f.readSint16LE();
		buffer[3] = f.readSint16LE();

		if ((buffer[3] | buffer[0] | buffer[2] | buffer[1]) == 0) break;

		int a = buffer[1];
		int size1 = buffer[2];
		int b = buffer[0];
		int size2 = buffer[3];

		int size = size2 * size1 * 2;

		uint texId = (((a >> 8) & 0xfffU) * 16 + ((b >> 7) & 0xffffU)) & 0xffff;
		if (texId > 3) {
			return;
		}
		uint aAdj = a & 0xff;
		if (a < 0) {
			aAdj = -(-a & 0xffU);
		}
		uint bAdj = b & 0x7f;
		if (b < 0) {
			bAdj = -(-b & 0x7fU);
		}

		uint pos = ((((aAdj & 0x1ff) * 128 + (bAdj & 0xffff)) & 0xffff) * 2) + (texId * 65536);

		f.read(_textureData.data() + pos, size);
	}

	UpdateTextures();
}

void Spriter::UpdateTextures() {
#if defined(USE_TINYGL)
	if (_textureGenerated) {
		tglDeleteTextures(TEXTURE_COUNT, _texture);
	}

	tglGenTextures(TEXTURE_COUNT, _texture);
	Common::Array<uint8> tex(256*256*3);

	bool hasPalette = _palette.size() > 0;

	for (uint i = 0; i < TEXTURE_COUNT; ++i) {
		for (uint j = 0; j < 256 * 256; ++j) {
			uint32 index = _textureData[(i * 65536) + j];
			if (hasPalette) {
				tex[(j * 3) + 0] = _palette[(index * 3) + 0];
				tex[(j * 3) + 1] = _palette[(index * 3) + 1];
				tex[(j * 3) + 2] = _palette[(index * 3) + 2];
			} else {
				tex[(j * 3) + 0] = index;
				tex[(j * 3) + 1] = index;
				tex[(j * 3) + 2] = index;
			}
		}
		tglBindTexture(TGL_TEXTURE_2D, _texture[i]);
		tglTexImage2D(TGL_TEXTURE_2D, 0, TGL_RGB, 256, 256, 0, TGL_RGB, TGL_UNSIGNED_BYTE, tex.data());
		tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_WRAP_S, TGL_REPEAT);
		tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_WRAP_T, TGL_REPEAT);
		tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_MAG_FILTER, TGL_NEAREST);
		tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_MIN_FILTER, TGL_NEAREST);
		tglBindTexture(TGL_TEXTURE_2D, 0);
	}
#endif
	_textureGenerated = true;
}

void Spriter::SetPalette(SCNHANDLE hPalette) {
	const
	uint32 paletteHeaderSize = 2 + 2;
	uint32 paletteBodySize = PALETTE_COUNT * 256 * sizeof(uint16);

	/* Select only one palette, as they are sorted by light intensity */
	uint32 paletteId = PALETTE_COUNT - 1;

	Graphics::PixelFormat paletteFormat(2, 5, 6, 5, 0, 11, 5, 0, 0);

	Common::MemoryReadStream s(_vm->_handle->LockMem(hPalette), paletteHeaderSize + paletteBodySize);

	uint zero = s.readUint16LE();
	uint size = s.readUint16LE();

	if (zero == 0 && size == paletteBodySize) {
		_palette.resize(256 * 3);

		s.skip(paletteId * sizeof(uint16) * 256);

		for (uint i = 0; i < 256; ++i) {
			uint16 color = s.readUint16LE();
			uint8 r, g, b;
			paletteFormat.colorToRGB(color, r, g, b);

			_palette[(i * 3) + 0] = r;
			_palette[(i * 3) + 1] = g;
			_palette[(i * 3) + 2] = b;
		}

		UpdateTextures();
	} else {
		warning("unknown palette data");
	}
}

void Spriter::SetSequence(uint animId, uint delay) {
	assert(animId < _animMain.size());

	_sequencesCount = _sequencesCount + 1;
	_animId = animId;

	if ((!_modelIdle) && (delay != 0)) {
		_modelMain.startFrame = -1;
		_modelMain.time = 0;
		_animDelay = delay;
		_animDelayMax = delay;
		if (!SetEndFrame(_modelMain, _animMain[animId], 0)) {
			error("Spr_SetSeq: Could not set end frame");
		}
	} else {
		if (!SetStartFrame(_modelMain, _animMain[animId], 0)) {
			error("Spr_SetSeq: Could not set start frame");
		}
		if (!SetEndFrame(_modelMain, _animMain[animId], _animMain[animId].maxFrame != 0)) {
			error("Spr_SetSeq: Could not set end frame");
		}
	}
}

Common::Rect Spriter::Draw(int direction, int x, int y, int z, int tDelta) {
	if (_modelIdle) {
		_modelIdle = false;
		_direction = direction;
	}

	_modelMain.position.set((float)x * 0.01f, (float)y * 0.01f, (float)z * 0.01f);
	_modelMain.scale.set(1.0f, 1.0f, 1.0f);

	if ((_animMain[_animId].maxFrame < 2) && (_modelMain.startFrame != -1)) {
		_modelMain.time = 0;
		if (!SetStartFrame(_modelMain, _animMain[_animId], _modelMain.endFrame)) {
			error("Spr_Step: Could not set start frame");
		}
	} else {
		if (_animDelay == 0) {
			_modelMain.time += tDelta;
		} else {
			_animDelay--;
			_modelMain.time += (uint)tDelta / _animDelayMax;
		}

		while (_modelMain.time > 0xFFFF) {
			_modelMain.time -= 0x10000;
			if (!SetStartFrame(_modelMain, _animMain[_animId], _modelMain.endFrame)) {
				error("Spr_Step: Could not set start frame");
			}

			_modelMain.endFrame++;
			if ((int)_animMain[_animId].maxFrame < _modelMain.endFrame) {
				_sequencesCount++;
				_modelMain.endFrame = 0;
			}
		}
	}

	// TODO: Do a gradual direction change when the model is idle - Noir is using 5 steps.
	_modelMain.rotation.set(0, ConvertAngle(direction), 0);

	RenderModel(_modelMain);

	// TODO: Add the shadow rendering after the lights are implemented
	// int shadowId = 3;
	// if (_animId == 2) {
	// 	shadowId = ((_modelMain.endFrame + 17) % 18) + 1;
	// }
	// _modelShadow.program = _modelShadow.hunks[_meshShadow[shadowId].programHunk].data.data() + _meshShadow[shadowId].program;
	// _modelShadow.tables.meshes = LoadMeshes(_modelShadow.hunks, _meshShadow[shadowId].meshTablesHunk, _meshShadow[shadowId].meshTables, 1);
	// float dx = _modelMain.position.x() - _modelMain.lightPosition[0].x;
	// float dz = _modelMain.position.z() - _modelMain.lightPosition[0].z;
	// float dy = _modelMain.position.y() - _modelMain.lightPosition[0].y;

	// float sheerx = dz * dz + dx * dx;
	// float sheerz = 0.0;

	// if (dy * dy < sheerx) {
	// 	dy = 1.0f / sqrt(sheerx);
	// 	sheerx = dx * dy;
	// 	sheerz = dy * dz;
	// } else {
	// 	sheerx = 0.0f;
	// 	sheerz = 0.0f;
	// 	if (0.0f != dy) {
	// 		sheerx = dx * (1.0f / dy);
	// 		sheerz = (1.0f / dy) * dz;
	// 	}
	// }

	// _modelShadow.scale.set(
	// 	abs(sheerx) + 1.0f,
	// 	_modelMain.scale.y(),
	// 	abs(sheerz) + 1.0f
	// );

	// _modelShadow.position.set(
	// 	_modelMain.position.x() + sheerx * 10.0f,
	// 	0,
	// 	_modelMain.position.z() + sheerz * 10.0f
	// );

	// _modelShadow.rotation.set(
	// 	_modelMain.rotation.x(),
	// 	_modelMain.rotation.y() + 180.0f,
	// 	_modelMain.rotation.z()
	// );

	// RenderModel(_modelShadow);

	return Common::Rect {0, 0};
}

Meshes Spriter::LoadMeshes(const Hunks &hunks, uint hunk, uint offset, int frame) {
	assert(hunk < hunks.size());

	Common::MemoryReadStream framesStream(hunks[hunk].data.data(), hunks[hunk].data.size());
	framesStream.skip(offset);

	uint numFrames = framesStream.readUint16LE();
	uint numEntries = framesStream.readUint16LE();

	assert(frame < (int)numFrames);

	framesStream.skip(frame * 4);

	uint meshListOffset = framesStream.readUint32LE();
	uint meshListHunk = hunks[hunk].mappingIdx[0];
	Common::MemoryReadStream meshListStream(hunks[meshListHunk].data.data(), hunks[meshListHunk].data.size());
	meshListStream.skip(meshListOffset);

	Meshes result;
	result.vertexCount = meshListStream.readUint32LE();
	result.normalCount = meshListStream.readUint32LE();
	result.meshes.resize(numEntries);

	// Read all meshes
	for (auto& mesh : result.meshes) {
		uint meshOffset = meshListStream.readUint32LE();
		uint meshHunk = hunks[meshListHunk].mappingIdx[0];

		Common::MemoryReadStream meshStream(hunks[meshHunk].data.data(), hunks[meshHunk].data.size());
		meshStream.skip(meshOffset);

		// Read vertices
		uint verticesOffset = meshStream.readUint32LE();
		uint verticesHunk = hunks[meshHunk].mappingIdx[1];

		mesh.vertices.resize(meshStream.readUint32LE());

		Common::MemoryReadStream verticesStream(hunks[verticesHunk].data.data(), hunks[verticesHunk].data.size());
		verticesStream.skip(verticesOffset);

		for (auto& v : mesh.vertices) {
			v.readFromStream(&verticesStream);
		}

		// Read normals
		uint normalsOffset = meshStream.readUint32LE();
		uint normalsHunk = hunks[meshHunk].mappingIdx[1];

		mesh.normals.resize(meshStream.readUint32LE());

		Common::MemoryReadStream normalsStream(hunks[normalsHunk].data.data(), hunks[normalsHunk].data.size());
		normalsStream.skip(normalsOffset);

		for (auto& n : mesh.normals) {
			n.readFromStream(&normalsStream);
		}

		// Read primitives
		uint primitivesOffset = meshStream.readUint32LE();
		uint primitivesHunk = hunks[meshHunk].mappingIdx[0];

		Common::MemoryReadStream primitivesStream(hunks[primitivesHunk].data.data(), hunks[primitivesHunk].data.size());
		primitivesStream.skip(primitivesOffset);

		while (true) {
			uint primitiveCount = primitivesStream.readUint16LE();
			uint primitiveType = primitivesStream.readUint16LE();
			uint dataSize = primitivesStream.readUint32LE();

			if (primitiveCount == 0) {
				break;
			}

			MeshPart part;
			part.numVertices = (primitiveType & 1) ? 4 : 3;
			part.type = static_cast<MeshPartType>((primitiveType & 0x7f) >> 1);
			part.cull = primitiveType & 0x80;
			part.primitives.resize(primitiveCount);

			int64 start = primitivesStream.pos();
			for (auto& prim : part.primitives) {
				for (uint i = 0; i < 8; ++i) {
					prim.indices[i] = primitivesStream.readUint16LE();
				}

				switch (part.type) {
					case MESH_PART_TYPE_COLOR:
						prim.color = primitivesStream.readUint32LE();
						break;
					case MESH_PART_TYPE_SOLID:
						assert(false); // TODO: not used? maybe in overlay model?
						break;
					case MESH_PART_TYPE_TEXTURE:
						// Has texture
						for (uint i = 0; i < part.numVertices; ++i) {
							prim.uv[i].readFromStream(&primitivesStream);
						}
						prim.texture = primitivesStream.readUint16LE();
						primitivesStream.skip(2); //padding
						break;
				}
			}
			int64 end = primitivesStream.pos();
			assert(dataSize == end - start);

			mesh.parts.push_back(part);
		}
	}

	return result;
}

template<bool convert>
AnimationData Spriter::LoadAnimationData(const Hunks& hunks, uint hunk, uint offset) {
	assert(hunk < hunks.size());

	Common::MemoryReadStream framesStream(hunks[hunk].data.data(), hunks[hunk].data.size());
	framesStream.skip(offset);

	uint numFrames = framesStream.readUint16LE();
	uint numEntries = framesStream.readUint16LE();

	AnimationData result;
	result.resize(numFrames);

	uint vectorsHunk = hunks[hunk].mappingIdx[0];
	assert(vectorsHunk < hunks.size());

	for (uint frame = 0; frame < numFrames; frame++) {
		auto& vectors = result[frame];
		uint vectorsOffset = framesStream.readUint32LE();

		Common::MemoryReadStream vectorsStream(hunks[vectorsHunk].data.data(), hunks[vectorsHunk].data.size());
		vectorsStream.skip(vectorsOffset);

		vectors.resize(numEntries);

		for (auto& v : vectors) {
			if (convert) {
				uint32 x = vectorsStream.readUint32LE();
				uint32 y = vectorsStream.readUint32LE();
				uint32 z = vectorsStream.readUint32LE();
				v.set(ConvertAngle(x), ConvertAngle(y), ConvertAngle(z));
			} else {
				v.readFromStream(&vectorsStream);
			}
		}
	}

	return result;
}

void Spriter::InitModel(Model &model, MeshInfo &meshInfo, Common::Array<AnimationInfo> &animInfos, uint flags) {
	model.flags = flags;

	model.program             = model.hunks[meshInfo.programHunk].data.data() + meshInfo.program;

	AnimationInfo &animInfo   = animInfos[0];

	model.tables.meshes       = LoadMeshes(model.hunks, meshInfo.meshTablesHunk, meshInfo.meshTables, 0);
	model.tables.translations = LoadAnimationData<false>(model.hunks, animInfo.translateTablesHunk, animInfo.translateTables)[0];
	model.tables.rotations    = LoadAnimationData<true>(model.hunks, animInfo.rotateTablesHunk, animInfo.rotateTables)[0];
	model.tables.scales       = LoadAnimationData<false>(model.hunks, animInfo.scaleTablesHunk, animInfo.scaleTables)[0];

	model.position.set(0.0f, 0.0f, 0.0f);
	model.rotation.set(0.0f, 0.0f, 0.0f);
	model.scale.set(1.0f, 1.0f, 1.0f);

	_currentMatrix = &_modelMatrix;

	MatrixReset();

	// Preprocess model - merge vertices that are close to each other
	RunRenderProgram(model, true);

	bool valid = true;
	if (model.flags & MODEL_HAS_TRANSLATION_TABLE) {
		for (const auto &anim : animInfos) {
			if (anim.translateNum != animInfo.translateNum) {
				valid = false;
			}
		}
		model.tables.translations.clear();
		model.tables.translations.resize(animInfo.translateNum);
	}

	assert(valid); // Warn if animation tables are incorrect

	for (uint i = 0; i < model.animationCount; ++i) {
		// TODO: check if animation data has same number of frames for all transformation types
		// SetStartFrame(model, animInfos[i], 0);
	}
}

void Spriter::RunRenderProgram(Model &model, bool preprocess) {
	uint8* program = model.program;
	uint ip = 0;

	Vectors vertices;
	vertices.reserve(model.tables.meshes.vertexCount);

	Vectors normals;
	normals.resize(model.tables.meshes.normalCount);

	Common::Array<uint16> sameVertices;
	sameVertices.resize(model.tables.meshes.vertexCount);

	bool stop = false;
	do {
		RenderProgramOp opCode = (RenderProgramOp)READ_LE_UINT16(&program[ip]);
		ip += 2;

		switch(opCode) {
			case MATRIX_DUPLICATE: {
				MatrixPush();
				break;
			}
			case MATRIX_REMOVE: {
				MatrixPop();
				break;
			}
			case UNUSED: {
				// TODO: Check where is this used. In some model overlay?
				// uint16 entry = READ_LE_UINT16(&program[ip]);
				ip += 2;
				break;
			}
			case TRANSFORM: {
				uint16 entry = READ_LE_UINT16(&program[ip]);

				Mesh& mesh = model.tables.meshes.meshes[entry];
				ip += 2;

				if (preprocess) {
					FindSimilarVertices(mesh, vertices, sameVertices);
					MergeVertices(mesh, sameVertices);
				} else {
					TransformMesh(mesh, vertices);
					CalculateNormals(mesh, vertices, normals);
					RenderMesh(mesh, vertices, normals);
				}

				break;
			}
			case TRANSLATE_X: {
				uint16 entry = READ_LE_UINT16(&program[ip]);
				ip += 2;

				Math::Vector3d& v = model.tables.translations[entry];
				MatrixTranslate(v.x(), 0, 0);

				break;
			}
			case TRANSLATE_Y: {
				uint16 entry = READ_LE_UINT16(&program[ip]);
				ip += 2;

				Math::Vector3d& v = model.tables.translations[entry];
				MatrixTranslate(0, v.y(), 0);

				break;
			}
			case TRANSLATE_Z: {
				uint16 entry = READ_LE_UINT16(&program[ip]);
				ip += 2;

				Math::Vector3d& v = model.tables.translations[entry];
				MatrixTranslate(0, 0, v.z());

				break;
			}
			case TRANSLATE_XYZ: {
				uint16 entry = READ_LE_UINT16(&program[ip]);
				ip += 2;

				Math::Vector3d& v = model.tables.translations[entry];
				MatrixTranslate(v.x(), v.y(), v.z());

				break;
			}
			case ROTATE_X: {
				uint16 entry = READ_LE_UINT16(&program[ip]);
				ip += 2;

				Math::Vector3d& v = model.tables.rotations[entry];
				MatrixRotateX(v.x());

				break;
			}
			case ROTATE_Y: {
				uint16 entry = READ_LE_UINT16(&program[ip]);
				ip += 2;

				Math::Vector3d& v = model.tables.rotations[entry];
				MatrixRotateY(v.y());

				break;
			}
			case ROTATE_Z: {
				uint16 entry = READ_LE_UINT16(&program[ip]);
				ip += 2;

				Math::Vector3d& v = model.tables.rotations[entry];
				MatrixRotateZ(v.z());

				break;
			}
			case STOP: {
				stop = true;
				break;
			}
			case ROTATE_XYZ: {
				uint entry = READ_LE_UINT16(&program[ip]);
				ip += 2;

				Math::Vector3d& v = model.tables.rotations[entry];
				MatrixRotateX(v.x());
				MatrixRotateY(v.y());
				MatrixRotateZ(v.z());

				break;
			}
			default:
			{
				error("UNKNOWN RENDER OP %i\n", opCode);
			}
		}
	} while (!stop);
}

void Spriter::FindSimilarVertices(Mesh& mesh, Vectors& vertices, Common::Array<uint16>& sameVertices) const {
	const Math::Matrix4 &m = MatrixCurrent();

	uint i_start = vertices.size();
	for (uint i = 0; i < mesh.vertices.size(); ++i) {
		Math::Vector3d& vIn = mesh.vertices[i];

		Math::Vector3d vOut = vIn;
		m.transform(&vOut, true);
		vertices.push_back(vOut);

		for (uint j = 0; j < vertices.size() - 1; ++j) {
			float d = vOut.getDistanceTo(vertices[j]);
			if (d < MERGE_VERTICES_OFFSET) {
				sameVertices[i_start + i] = j + 1; // 0 is reserved for "not found"
				break;
			}
		}
	}
}

void Spriter::MergeVertices(Mesh &mesh, Common::Array<uint16>& sameVertices) {
	for (auto& part : mesh.parts) {
		for (auto& prim : part.primitives) {
			for (uint i = 0; i < part.numVertices; ++i) {
				if (sameVertices[prim.indices[i]] != 0) {
					prim.indices[i] = sameVertices[prim.indices[i]] - 1;
				}
			}
		}
	}
}

void Spriter::TransformMesh(Mesh& mesh, Vectors& vertices) {
	// Transformed vertices from previous meshes might be used by the current mesh.
	const Math::Matrix4 &m = MatrixCurrent();

	for (auto& vIn : mesh.vertices) {
		Math::Vector3d vOut = vIn;
		m.transform(&vOut, true);
		vertices.push_back(vOut);
	}
}

void Spriter::CalculateNormals(Mesh& mesh, Vectors& vertices, Vectors &normals) {
	for (auto& part : mesh.parts) {
		for (auto& prim : part.primitives) {
			Math::Vector3d v0 = vertices[prim.indices[0]];
			Math::Vector3d v1 = vertices[prim.indices[1]];
			Math::Vector3d v2 = vertices[prim.indices[2]];

			Math::Vector3d norm = Math::Vector3d::crossProduct(v2 - v0, v1 - v0).getNormalized();

			for (uint i = 0; i < part.numVertices; ++i) {
				normals[prim.indices[i]] += norm;
			}
		}
	}
}

void Spriter::RenderMesh(Mesh& mesh, Vectors& vertices, Vectors &normals) {
	for(auto& part : mesh.parts) {
		switch(part.type) {
		case MESH_PART_TYPE_COLOR:
			RenderMeshPartColor(part, vertices, normals);
			break;
		case MESH_PART_TYPE_SOLID:
			// TODO: Check where is this used. In some model overlay?
			// This just uses white color
			// RenderMeshPartColor(part, vertices, normals);
			break;
		case MESH_PART_TYPE_TEXTURE:
			RenderMeshPartTexture(part, vertices, normals);
			break;
		}
	}
	return;
}

void Spriter::RenderMeshPartColor(MeshPart& part, Vectors& vertices, Vectors &normals) {
#if defined(USE_TINYGL)
	if(!part.cull) {
		tglEnable(TGL_CULL_FACE);
	}

	for (auto& prim : part.primitives) {
		TGLubyte r = (prim.color >> 0) & 0xff;
		TGLubyte g = (prim.color >> 8) & 0xff;
		TGLubyte b = (prim.color >> 16) & 0xff;

		tglColor3ub(r, g, b);

		if (part.numVertices == 4) {
			tglBegin(TGL_QUADS);
		} else {
			tglBegin(TGL_TRIANGLES);
		}
		for (uint32 i = 0; i < part.numVertices; ++i) {
			uint32 index = prim.indices[i];

			Math::Vector3d n = normals[index].getNormalized();
			tglNormal3f(n.x(), n.y(), n.z());

			Math::Vector3d& v = vertices[index];
			tglVertex3f(v.x(), v.y(), v.z());
		}
		tglEnd();
	}
	tglDisable(TGL_CULL_FACE);
#endif
}

void Spriter::RenderMeshPartTexture(MeshPart& part, Vectors& vertices, Vectors &normals) {
#if defined(USE_TINYGL)
	if (!part.cull) {
		tglEnable(TGL_CULL_FACE);
	}

	tglEnable(TGL_TEXTURE_2D);
	tglColor3f(1.0f, 1.0f, 1.0f);

	for (auto& prim : part.primitives) {
		tglBindTexture(TGL_TEXTURE_2D, _texture[prim.texture]);

		if (part.numVertices == 4) {
			tglBegin(TGL_QUADS);
		} else {
			tglBegin(TGL_TRIANGLES);
		}
		for (uint32 i = 0; i < part.numVertices; ++i) {
			uint index = prim.indices[i];
			tglTexCoord2f(prim.uv[i].getX() / 256.0f, prim.uv[i].getY() / 256.0f);

			Math::Vector3d n = normals[index].getNormalized();
			tglNormal3f(n.x(), n.y(), n.z());

			Math::Vector3d& v = vertices[index];
			tglVertex3f(v.x(), v.y(), v.z());
		}
		tglEnd();
	}
	tglDisable(TGL_TEXTURE_2D);
	tglDisable(TGL_CULL_FACE);
#endif
}

void Spriter::Load(const Common::String &modelName, const Common::String &textureName) {
	LoadH(modelName);
	LoadGBL(modelName);
	LoadRBH(modelName, _modelMain.hunks);
	LoadVMC(modelName);

	InitModel(_modelMain, _meshMain, _animMain, MODEL_HAS_TRANSLATION_TABLE | MODEL_HAS_ROTATION_TABLE);

	// TODO: Check where is this used. In some model overlay?
	// for (uint i = 0; i < _animMain.size(); ++i) { }

	_modelIdle = true;
	_modelMain.time = 0;
}

void lerp3(Vectors &dst, const Vectors &src1, const Vectors &src2, uint t) {
	assert(dst.size() == src1.size() && src1.size() == src2.size());
	float interpolator = static_cast<float>(t) / 65536.0f;
	float interpolatorInv = 1.0f - interpolator;
	for (uint i = 0; i < dst.size(); ++i) {
		dst[i] = src1[i] * interpolator + src2[i] * interpolatorInv;
	}

}

void Spriter::RenderModel(Model &model) {
	if (model.flags & MODEL_HAS_TRANSLATION_TABLE) {
		if (model.startFrame == -1) {
			lerp3(model.tables.translations, model.tables.translations, model.endTranslateTables[model.endFrame], model.time);
		} else {
			lerp3(model.tables.translations,  model.startTranslateTables[model.startFrame], model.endTranslateTables[model.endFrame], model.time);
		}
	}

	if (model.flags & MODEL_HAS_ROTATION_TABLE) {
		if (model.startFrame == -1) {
			lerp3(model.tables.rotations, model.tables.rotations, model.endRotateTables[model.endFrame], model.time);
		} else {
			lerp3(model.tables.rotations,  model.startRotateTables[model.startFrame], model.endRotateTables[model.endFrame], model.time);
		}
	}

	if (model.flags & MODEL_HAS_SCALE_TABLE) {
		if (model.startFrame == -1) {
			lerp3(model.tables.scales, model.tables.scales, model.endScaleTables[model.endFrame], model.time);
		} else {
			lerp3(model.tables.scales,  model.startScaleTables[model.startFrame], model.endScaleTables[model.endFrame], model.time);
		}
	}

	MatrixReset();
	MatrixTranslate(model.position.x(), model.position.y(), model.position.z());
	MatrixScale(model.scale.x(), model.scale.y(), model.scale.z());
	MatrixRotateX(model.rotation.x());
	MatrixRotateY(model.rotation.y());
	MatrixRotateZ(model.rotation.z());

	MatrixRotateX(_view.rotation.x());
	MatrixRotateY(_view.rotation.y());
	MatrixRotateZ(_view.rotation.z());
	MatrixTranslate(-_view.position.x(), -_view.position.y(), -_view.position.z());

#if defined(USE_TINYGL)
	tglViewport(0, 0, _vm->screen().w, _vm->screen().h);

	tglMatrixMode(TGL_PROJECTION);
	tglLoadIdentity();
	tglFrustum(-1.0f, 1.0f, -3.0f / 4.0f, 3.0f / 4.0f, 1.0f, 1000.0f);
	// GL uses bottom left system
	tglScalef(1.0f, -1.0f, 1.0f);
	// Z is inverted in GL, we need to invert the face orientation too.
	tglScalef(1.0f, 1.0f, -1.0f);
	tglFrontFace(TGL_CW);

	tglMatrixMode(TGL_MODELVIEW);
	tglLoadIdentity();

	// TODO: Use GL's view matrix instead of the game one.
	// tglRotatef(_view.rotation.x(), 1.0f, 0.0f, 0.0f);
	// tglRotatef(_view.rotation.y(), 0.0f, 1.0f, 0.0f);
	// tglRotatef(_view.rotation.z(), 0.0f, 0.0f, 1.0f);
	// tglTranslatef(-_view.position.x(), -_view.position.y(), -_view.position.z());

	tglEnable(TGL_DEPTH_TEST);
	tglDepthFunc(TGL_LESS);
	tglDepthMask(TGL_TRUE);
	tglClearDepth(1.0f);
	tglShadeModel(TGL_SMOOTH);

	tglClear(TGL_DEPTH_BUFFER_BIT);

#if 0
	// TODO: Remove. Used only for debugging of the normals calculation and rendering.
	tglEnable(TGL_LIGHTING);
	TGLfloat light_position[] = { 0.0, 0.0, 0.0, 1.0 };
	tglLightfv(TGL_LIGHT0, TGL_POSITION, light_position);
	tglLightf(TGL_LIGHT0, TGL_CONSTANT_ATTENUATION, 1000.0f);
	tglEnable(TGL_LIGHT0);
#endif
#endif

	RunRenderProgram(model, false);

#if defined(USE_TINYGL)
	TinyGL::presentBuffer();
#endif
}

bool Spriter::SetStartFrame(Model &model, const AnimationInfo &anim, int frame) {
	const Hunks &hunks = model.hunks;
	uint numFrames = 0;
	if ((model.flags & MODEL_HAS_TRANSLATION_TABLE) != 0) {
		model.startTranslateTables = LoadAnimationData<false>(hunks, anim.translateTablesHunk, anim.translateTables);
		numFrames = model.startTranslateTables.size();
	}
	if ((model.flags & MODEL_HAS_ROTATION_TABLE) != 0) {
		model.startRotateTables = LoadAnimationData<true>(hunks, anim.rotateTablesHunk, anim.rotateTables);
		numFrames = model.startRotateTables.size();
	}
	if ((model.flags & MODEL_HAS_SCALE_TABLE) != 0) {
		model.startScaleTables = LoadAnimationData<false>(hunks, anim.scaleTablesHunk, anim.scaleTables);
		numFrames = model.startScaleTables.size();
	}
	if (frame < 0 || frame >= (int)numFrames) {
		return false;
	}
	model.startFrame = frame;
	return true;
}

bool Spriter::SetEndFrame(Model &model, const AnimationInfo &anim, int frame) {
	const Hunks &hunks = model.hunks;
	uint numFrames = 0;
	if ((model.flags & MODEL_HAS_TRANSLATION_TABLE) != 0) {
		model.endTranslateTables = LoadAnimationData<false>(hunks, anim.translateTablesHunk, anim.translateTables);
		numFrames = model.endTranslateTables.size();
	}
	if ((model.flags & MODEL_HAS_ROTATION_TABLE) != 0) {
		model.endRotateTables = LoadAnimationData<true>(hunks, anim.rotateTablesHunk, anim.rotateTables);
		numFrames = model.endRotateTables.size();
	}
	if ((model.flags & MODEL_HAS_SCALE_TABLE) != 0) {
		model.endScaleTables = LoadAnimationData<false>(hunks, anim.scaleTablesHunk, anim.scaleTables);
		numFrames = model.endScaleTables.size();
	}
	if (frame < 0 || frame >= (int)numFrames) {
		return false;
	}
	model.endFrame = frame;
	return true;
}


} // End of namespace Tinsel
