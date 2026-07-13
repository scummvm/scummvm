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

#include "engines/stark/formats/gltf.h"

#include "engines/stark/debug.h"
#include "engines/stark/model/model.h"
#include "engines/stark/model/skeleton_anim.h"

#include "common/algorithm.h"
#include "common/debug.h"
#include "common/formats/json.h"
#include "common/memstream.h"

namespace Stark {
namespace Formats {

namespace {

// glTF constants
enum {
	kComponentByte = 5120,
	kComponentUnsignedByte = 5121,
	kComponentShort = 5122,
	kComponentUnsignedShort = 5123,
	kComponentUnsignedInt = 5125,
	kComponentFloat = 5126
};

const Common::JSONValue *jsGet(const Common::JSONObject &obj, const char *key) {
	return obj.contains(key) ? obj[key] : nullptr;
}

double jsNumber(const Common::JSONValue *v, double fallback = 0.0) {
	if (!v)
		return fallback;
	if (v->isIntegerNumber())
		return (double)v->asIntegerNumber();
	if (v->isNumber())
		return v->asNumber();
	return fallback;
}

int jsInt(const Common::JSONObject &obj, const char *key, int fallback = -1) {
	const Common::JSONValue *v = jsGet(obj, key);
	if (!v || !v->isIntegerNumber())
		return fallback;
	return (int)v->asIntegerNumber();
}

/** All the parsed glTF document state a load needs. */
struct GltfContext {
	Common::JSONValue *root = nullptr;
	Common::Array<byte> binData;
	const byte *bin = nullptr;
	uint32 binSize = 0;

	const Common::JSONArray *accessors = nullptr;
	const Common::JSONArray *bufferViews = nullptr;
	const Common::JSONArray *nodes = nullptr;
	const Common::JSONArray *materials = nullptr;
	const Common::JSONArray *textures = nullptr;
	const Common::JSONArray *images = nullptr;
	const Common::JSONArray *animations = nullptr;

	~GltfContext() {
		delete root;
	}
};

/** Parse the GLB container and the top-level document arrays. */
bool parseGlb(GltfContext &ctx, Common::SeekableReadStream *stream) {
	uint32 magic = stream->readUint32LE();
	uint32 version = stream->readUint32LE();
	stream->readUint32LE(); // total length
	if (magic != MKTAG('F', 'T', 'l', 'g') || version != 2) {
		warning("glTF: not a GLB v2 file");
		return false;
	}

	uint32 jsonLength = stream->readUint32LE();
	uint32 jsonType = stream->readUint32BE();
	if (jsonType != MKTAG('J', 'S', 'O', 'N')) {
		warning("glTF: first chunk is not JSON");
		return false;
	}

	Common::Array<char> jsonData(jsonLength + 1);
	stream->read(jsonData.begin(), jsonLength);
	jsonData[jsonLength] = '\0';

	if (stream->pos() + 8 <= stream->size()) {
		uint32 binLength = stream->readUint32LE();
		uint32 binType = stream->readUint32BE();
		if (binType == MKTAG('B', 'I', 'N', '\0')) {
			ctx.binData.resize(binLength);
			stream->read(ctx.binData.begin(), binLength);
		}
	}

	ctx.root = Common::JSON::parse(jsonData.begin());
	if (!ctx.root || !ctx.root->isObject()) {
		warning("glTF: JSON parse error");
		return false;
	}
	ctx.bin = ctx.binData.begin();
	ctx.binSize = ctx.binData.size();

	const Common::JSONObject &root = ctx.root->asObject();
	const Common::JSONValue *v;
	if ((v = jsGet(root, "accessors")) && v->isArray()) ctx.accessors = &v->asArray();
	if ((v = jsGet(root, "bufferViews")) && v->isArray()) ctx.bufferViews = &v->asArray();
	if ((v = jsGet(root, "nodes")) && v->isArray()) ctx.nodes = &v->asArray();
	if ((v = jsGet(root, "materials")) && v->isArray()) ctx.materials = &v->asArray();
	if ((v = jsGet(root, "textures")) && v->isArray()) ctx.textures = &v->asArray();
	if ((v = jsGet(root, "images")) && v->isArray()) ctx.images = &v->asArray();
	if ((v = jsGet(root, "animations")) && v->isArray()) ctx.animations = &v->asArray();

	return true;
}

/** Map node indices to Stark bone indices through their "bone_NN" names. */
Common::Array<int> mapNodesToBones(const GltfContext &ctx) {
	Common::Array<int> nodeToBone;
	if (!ctx.nodes)
		return nodeToBone;
	nodeToBone.resize(ctx.nodes->size());
	for (uint i = 0; i < ctx.nodes->size(); i++) {
		nodeToBone[i] = -1;
		const Common::JSONObject &node = (*ctx.nodes)[i]->asObject();
		const Common::JSONValue *nameVal = jsGet(node, "name");
		if (nameVal && nameVal->isString()) {
			const Common::String &name = nameVal->asString();
			// Accept "bone_07" and Blender duplicates like "bone_07.001"
			if (name.hasPrefix("bone_"))
				nodeToBone[i] = atoi(name.c_str() + 5);
		}
	}
	return nodeToBone;
}

/** Owns partially-built model arrays until they are committed to the Model. */
struct ModelDataGuard {
	Common::Array<VertNode *> vertices;
	Common::Array<Material *> materials;
	Common::Array<Face *> faces;
	Common::Array<BoneNode *> bones;
	bool committed = false;

	~ModelDataGuard() {
		if (committed)
			return;
		for (uint i = 0; i < vertices.size(); i++)
			delete vertices[i];
		for (uint i = 0; i < materials.size(); i++)
			delete materials[i];
		for (uint i = 0; i < faces.size(); i++)
			delete faces[i];
		for (uint i = 0; i < bones.size(); i++)
			delete bones[i];
	}
};

uint componentSize(int componentType) {
	switch (componentType) {
	case kComponentByte:
	case kComponentUnsignedByte:
		return 1;
	case kComponentShort:
	case kComponentUnsignedShort:
		return 2;
	case kComponentUnsignedInt:
	case kComponentFloat:
		return 4;
	default:
		return 0;
	}
}

uint typeComponents(const Common::String &type) {
	if (type == "SCALAR") return 1;
	if (type == "VEC2") return 2;
	if (type == "VEC3") return 3;
	if (type == "VEC4") return 4;
	if (type == "MAT4") return 16;
	return 0;
}

/**
 * Read an accessor's data, converting every component to float.
 * Integer components are normalized to [0, 1] when the accessor requests it
 * (used by Blender for weights). Returns an empty array on error.
 */
Common::Array<float> readAccessorFloats(const GltfContext &ctx, int accessorIdx) {
	Common::Array<float> out;
	if (!ctx.accessors || accessorIdx < 0 || accessorIdx >= (int)ctx.accessors->size())
		return out;
	const Common::JSONObject &acc = (*ctx.accessors)[accessorIdx]->asObject();

	if (jsGet(acc, "sparse")) {
		warning("glTF: sparse accessors are not supported");
		return out;
	}

	int viewIdx = jsInt(acc, "bufferView");
	int componentType = jsInt(acc, "componentType");
	uint count = jsInt(acc, "count", 0);
	Common::String type = jsGet(acc, "type") ? jsGet(acc, "type")->asString() : "";
	bool normalized = jsGet(acc, "normalized") && jsGet(acc, "normalized")->asBool();
	uint accOffset = jsInt(acc, "byteOffset", 0);

	uint comps = typeComponents(type);
	uint csize = componentSize(componentType);
	if (!comps || !csize || viewIdx < 0 || !ctx.bufferViews || viewIdx >= (int)ctx.bufferViews->size())
		return out;

	const Common::JSONObject &view = (*ctx.bufferViews)[viewIdx]->asObject();
	uint viewOffset = jsInt(view, "byteOffset", 0);
	uint viewLength = jsInt(view, "byteLength", 0);
	uint stride = jsInt(view, "byteStride", 0);
	if (stride == 0)
		stride = comps * csize;

	if (viewOffset + viewLength > ctx.binSize)
		return out;
	if (count == 0 || accOffset + (count - 1) * stride + comps * csize > viewLength)
		return out;

	const byte *base = ctx.bin + viewOffset + accOffset;
	out.reserve(count * comps);

	for (uint i = 0; i < count; i++) {
		const byte *p = base + i * stride;
		for (uint c = 0; c < comps; c++) {
			const byte *cp = p + c * csize;
			float value;
			switch (componentType) {
			case kComponentFloat: {
				uint32 bits = READ_LE_UINT32(cp);
				memcpy(&value, &bits, sizeof(value));
				break;
			}
			case kComponentUnsignedByte:
				value = *cp;
				if (normalized)
					value /= 255.0f;
				break;
			case kComponentByte:
				value = (int8)*cp;
				if (normalized)
					value = MAX(value / 127.0f, -1.0f);
				break;
			case kComponentUnsignedShort:
				value = READ_LE_UINT16(cp);
				if (normalized)
					value /= 65535.0f;
				break;
			case kComponentShort:
				value = (int16)READ_LE_UINT16(cp);
				if (normalized)
					value = MAX(value / 32767.0f, -1.0f);
				break;
			case kComponentUnsignedInt:
				value = (float)READ_LE_UINT32(cp);
				break;
			default:
				value = 0.0f;
			}
			out.push_back(value);
		}
	}

	return out;
}

/** Read an accessor holding indices / joint ids as unsigned integers. */
Common::Array<uint32> readAccessorUints(const GltfContext &ctx, int accessorIdx) {
	Common::Array<uint32> out;
	if (!ctx.accessors || accessorIdx < 0 || accessorIdx >= (int)ctx.accessors->size())
		return out;
	const Common::JSONObject &acc = (*ctx.accessors)[accessorIdx]->asObject();

	int viewIdx = jsInt(acc, "bufferView");
	int componentType = jsInt(acc, "componentType");
	uint count = jsInt(acc, "count", 0);
	Common::String type = jsGet(acc, "type") ? jsGet(acc, "type")->asString() : "";
	uint accOffset = jsInt(acc, "byteOffset", 0);

	uint comps = typeComponents(type);
	uint csize = componentSize(componentType);
	if (!comps || !csize || viewIdx < 0 || !ctx.bufferViews || viewIdx >= (int)ctx.bufferViews->size())
		return out;

	const Common::JSONObject &view = (*ctx.bufferViews)[viewIdx]->asObject();
	uint viewOffset = jsInt(view, "byteOffset", 0);
	uint viewLength = jsInt(view, "byteLength", 0);
	uint stride = jsInt(view, "byteStride", 0);
	if (stride == 0)
		stride = comps * csize;

	if (viewOffset + viewLength > ctx.binSize)
		return out;
	if (count == 0 || accOffset + (count - 1) * stride + comps * csize > viewLength)
		return out;

	const byte *base = ctx.bin + viewOffset + accOffset;
	out.reserve(count * comps);

	for (uint i = 0; i < count; i++) {
		const byte *p = base + i * stride;
		for (uint c = 0; c < comps; c++) {
			const byte *cp = p + c * csize;
			switch (componentType) {
			case kComponentUnsignedByte:
				out.push_back(*cp);
				break;
			case kComponentUnsignedShort:
				out.push_back(READ_LE_UINT16(cp));
				break;
			case kComponentUnsignedInt:
				out.push_back(READ_LE_UINT32(cp));
				break;
			default:
				out.push_back(0);
			}
		}
	}

	return out;
}

/** Transform a point by a column-major 4x4 matrix (rigid, w = 1). */
Math::Vector3d transformPoint(const float *m, const Math::Vector3d &p) {
	return Math::Vector3d(
	        m[0] * p.x() + m[4] * p.y() + m[8] * p.z() + m[12],
	        m[1] * p.x() + m[5] * p.y() + m[9] * p.z() + m[13],
	        m[2] * p.x() + m[6] * p.y() + m[10] * p.z() + m[14]);
}

/** Rotate a direction by the 3x3 part of a column-major 4x4 matrix. */
Math::Vector3d transformDirection(const float *m, const Math::Vector3d &d) {
	return Math::Vector3d(
	        m[0] * d.x() + m[4] * d.y() + m[8] * d.z(),
	        m[1] * d.x() + m[5] * d.y() + m[9] * d.z(),
	        m[2] * d.x() + m[6] * d.y() + m[10] * d.z());
}

/** "face2" / "face2.png" / "face2.bmp" -> "face2.bmp", as texture sets expect. */
Common::String normalizeTextureName(const Common::String &name) {
	if (name.empty())
		return name;
	Common::String base = name;
	int dot = -1;
	for (int i = base.size() - 1; i >= 0; i--) {
		if (base[i] == '.') {
			dot = i;
			break;
		}
	}
	if (dot >= 0)
		base = Common::String(base.c_str(), dot);
	return base + ".bmp";
}

} // End of anonymous namespace

bool GltfModelReader::load(Model *model, Common::SeekableReadStream *stream) {
	GltfContext ctx;
	if (!parseGlb(ctx, stream))
		return false;

	const Common::JSONObject &root = ctx.root->asObject();

	if (!ctx.nodes || !ctx.accessors || !ctx.bufferViews) {
		warning("glTF: missing nodes / accessors / bufferViews");
		return false;
	}

	// --- find the skinned mesh node ------------------------------------------
	const Common::JSONValue *meshesVal = jsGet(root, "meshes");
	const Common::JSONValue *skinsVal = jsGet(root, "skins");
	if (!meshesVal || !meshesVal->isArray() || !skinsVal || !skinsVal->isArray()) {
		warning("glTF: no skinned mesh found");
		return false;
	}
	const Common::JSONArray &meshes = meshesVal->asArray();
	const Common::JSONArray &skins = skinsVal->asArray();

	int meshIdx = -1, skinIdx = -1;
	for (uint i = 0; i < ctx.nodes->size(); i++) {
		const Common::JSONObject &node = (*ctx.nodes)[i]->asObject();
		if (jsGet(node, "mesh") && jsGet(node, "skin")) {
			meshIdx = jsInt(node, "mesh");
			skinIdx = jsInt(node, "skin");
			break;
		}
	}
	if (meshIdx < 0 || meshIdx >= (int)meshes.size() || skinIdx < 0 || skinIdx >= (int)skins.size()) {
		warning("glTF: no node references both a mesh and a skin");
		return false;
	}

	// --- skeleton -------------------------------------------------------------
	const Common::JSONObject &skin = skins[skinIdx]->asObject();
	const Common::JSONValue *jointsVal = jsGet(skin, "joints");
	if (!jointsVal || !jointsVal->isArray()) {
		warning("glTF: skin has no joints");
		return false;
	}
	const Common::JSONArray &joints = jointsVal->asArray();
	uint numJoints = joints.size();

	Common::Array<float> ibms = readAccessorFloats(ctx, jsInt(skin, "inverseBindMatrices"));
	if (ibms.size() != numJoints * 16) {
		warning("glTF: bad inverse bind matrices");
		return false;
	}

	// Map joints to Stark bone indices via their "bone_NN" names, falling
	// back to joint order when the names don't follow the convention.
	Common::Array<int> jointToBone;
	jointToBone.resize(numJoints);
	Common::Array<int> nodeToJoint;
	nodeToJoint.resize(ctx.nodes->size());
	for (uint i = 0; i < nodeToJoint.size(); i++)
		nodeToJoint[i] = -1;

	bool namesOk = true;
	for (uint j = 0; j < numJoints; j++) {
		int nodeIdx = (int)jsNumber(joints[j], -1);
		if (nodeIdx < 0 || nodeIdx >= (int)ctx.nodes->size()) {
			warning("glTF: joint %d references invalid node", j);
			return false;
		}
		nodeToJoint[nodeIdx] = j;

		const Common::JSONObject &node = (*ctx.nodes)[nodeIdx]->asObject();
		const Common::JSONValue *nameVal = jsGet(node, "name");
		int boneIdx = -1;
		if (nameVal && nameVal->isString()) {
			const Common::String &name = nameVal->asString();
			// Accept "bone_07" and Blender duplicates like "bone_07.001"
			if (name.hasPrefix("bone_"))
				boneIdx = atoi(name.c_str() + 5);
		}
		if (boneIdx < 0 || boneIdx >= (int)numJoints) {
			namesOk = false;
			boneIdx = j;
		}
		jointToBone[j] = boneIdx;
	}
	if (!namesOk)
		warning("glTF: joints are not all named bone_NN, using joint order as bone order");

	ModelDataGuard data;
	Common::Array<BoneNode *> &bones = data.bones;
	bones.resize(numJoints);
	for (uint j = 0; j < numJoints; j++) {
		BoneNode *bone = new BoneNode();
		bone->_idx = jointToBone[j];
		bone->_u1 = 1.0f;
		bones[jointToBone[j]] = bone;
	}
	for (uint j = 0; j < numJoints; j++) {
		int nodeIdx = (int)jsNumber(joints[j], -1);
		const Common::JSONObject &node = (*ctx.nodes)[nodeIdx]->asObject();
		const Common::JSONValue *childrenVal = jsGet(node, "children");
		if (!childrenVal || !childrenVal->isArray())
			continue;
		const Common::JSONArray &children = childrenVal->asArray();
		for (uint c = 0; c < children.size(); c++) {
			int childNode = (int)jsNumber(children[c], -1);
			if (childNode < 0 || childNode >= (int)nodeToJoint.size() || nodeToJoint[childNode] < 0)
				continue;
			uint32 childBone = jointToBone[nodeToJoint[childNode]];
			bones[jointToBone[j]]->_children.push_back(childBone);
			bones[childBone]->_parent = jointToBone[j];
		}
	}

	// --- materials --------------------------------------------------------------
	Common::Array<Material *> &materials = data.materials;
	if (ctx.materials) {
		for (uint i = 0; i < ctx.materials->size(); i++) {
			const Common::JSONObject &mat = (*ctx.materials)[i]->asObject();
			Material *material = new Material();
			if (jsGet(mat, "name"))
				material->name = jsGet(mat, "name")->asString();

			const Common::JSONValue *pbrVal = jsGet(mat, "pbrMetallicRoughness");
			if (pbrVal && pbrVal->isObject()) {
				const Common::JSONObject &pbr = pbrVal->asObject();
				const Common::JSONValue *factor = jsGet(pbr, "baseColorFactor");
				if (factor && factor->isArray() && factor->asArray().size() >= 3) {
					material->r = jsNumber(factor->asArray()[0], 1.0);
					material->g = jsNumber(factor->asArray()[1], 1.0);
					material->b = jsNumber(factor->asArray()[2], 1.0);
				}
				const Common::JSONValue *texRef = jsGet(pbr, "baseColorTexture");
				if (texRef && texRef->isObject() && ctx.textures) {
					int texIdx = jsInt(texRef->asObject(), "index");
					if (texIdx >= 0 && texIdx < (int)ctx.textures->size()) {
						int imgIdx = jsInt((*ctx.textures)[texIdx]->asObject(), "source");
						if (ctx.images && imgIdx >= 0 && imgIdx < (int)ctx.images->size()) {
							const Common::JSONObject &img = (*ctx.images)[imgIdx]->asObject();
							Common::String texName;
							if (jsGet(img, "name"))
								texName = jsGet(img, "name")->asString();
							else if (jsGet(img, "uri"))
								texName = jsGet(img, "uri")->asString();
							material->texture = normalizeTextureName(texName);
						}
					}
				}
			}
			materials.push_back(material);
		}
	}

	// --- mesh primitives -> vertices and faces ------------------------------------
	Common::Array<VertNode *> &vertices = data.vertices;
	Common::Array<Face *> &faces = data.faces;

	const Common::JSONObject &mesh = meshes[meshIdx]->asObject();
	const Common::JSONValue *primsVal = jsGet(mesh, "primitives");
	if (!primsVal || !primsVal->isArray()) {
		warning("glTF: mesh has no primitives");
		return false;
	}
	const Common::JSONArray &prims = primsVal->asArray();

	for (uint p = 0; p < prims.size(); p++) {
		const Common::JSONObject &prim = prims[p]->asObject();
		if (jsInt(prim, "mode", 4) != 4) {
			warning("glTF: skipping non-triangle primitive %d", p);
			continue;
		}
		const Common::JSONValue *attrsVal = jsGet(prim, "attributes");
		if (!attrsVal || !attrsVal->isObject())
			continue;
		const Common::JSONObject &attrs = attrsVal->asObject();

		Common::Array<float> positions = readAccessorFloats(ctx, jsInt(attrs, "POSITION"));
		Common::Array<float> normals = readAccessorFloats(ctx, jsInt(attrs, "NORMAL"));
		Common::Array<float> uvs = readAccessorFloats(ctx, jsInt(attrs, "TEXCOORD_0"));
		Common::Array<uint32> jointIds = readAccessorUints(ctx, jsInt(attrs, "JOINTS_0"));
		Common::Array<float> weights = readAccessorFloats(ctx, jsInt(attrs, "WEIGHTS_0"));
		Common::Array<uint32> indices = readAccessorUints(ctx, jsInt(prim, "indices"));

		uint numVerts = positions.size() / 3;
		if (numVerts == 0 || indices.empty()) {
			warning("glTF: primitive %d has no geometry", p);
			continue;
		}
		if (jointIds.size() < numVerts * 4 || weights.size() < numVerts * 4) {
			warning("glTF: primitive %d is not skinned", p);
			return false;
		}

		uint vertexOffset = vertices.size();

		for (uint i = 0; i < numVerts; i++) {
			Math::Vector3d position(positions[i * 3], positions[i * 3 + 1], positions[i * 3 + 2]);

			// Keep the two strongest influences, matching the two-bone rig.
			uint best = 0, second = 1;
			for (uint k = 1; k < 4; k++) {
				if (weights[i * 4 + k] > weights[i * 4 + best])
					best = k;
			}
			second = (best == 0) ? 1 : 0;
			for (uint k = 0; k < 4; k++) {
				if (k != best && weights[i * 4 + k] > weights[i * 4 + second])
					second = k;
			}

			float w1 = weights[i * 4 + best];
			float w2 = weights[i * 4 + second];
			uint32 j1 = jointIds[i * 4 + best];
			uint32 j2 = jointIds[i * 4 + second];
			if (w2 <= 0.0f || j2 >= numJoints)
				j2 = j1;
			if (j1 >= numJoints) {
				warning("glTF: primitive %d vertex %d references joint out of range", p, i);
				return false;
			}
			float weight = (w1 + w2 > 0.0f) ? w1 / (w1 + w2) : 1.0f;

			const float *ibm1 = &ibms[j1 * 16];
			const float *ibm2 = &ibms[j2 * 16];

			VertNode *vert = new VertNode();
			// The inverse bind matrix is exactly the world -> bone-local
			// transform the .cir format stores its positions in.
			vert->_pos1 = transformPoint(ibm1, position);
			vert->_pos2 = transformPoint(ibm2, position);

			Math::Vector3d normal(0.0f, 1.0f, 0.0f);
			if (normals.size() >= (i + 1) * 3)
				normal = Math::Vector3d(normals[i * 3], normals[i * 3 + 1], normals[i * 3 + 2]);
			// The .cir stores one bone-local normal, rotated by each bone and
			// blended in the vertex shader. Expressing the world normal in the
			// dominant bone's frame reproduces it best.
			Math::Vector3d localNormal = transformDirection(weight >= 0.5f ? ibm1 : ibm2, normal);
			localNormal.normalize();
			vert->_normal = localNormal;

			vert->_texS = (uvs.size() >= (i + 1) * 2) ? uvs[i * 2] : 0.0f;
			vert->_texT = (uvs.size() >= (i + 1) * 2) ? uvs[i * 2 + 1] : 0.0f;

			vert->_bone1 = jointToBone[j1];
			vert->_bone2 = jointToBone[j2];
			vert->_boneWeight = weight;

			vertices.push_back(vert);
		}

		Face *face = new Face();
		int materialId = jsInt(prim, "material", 0);
		face->materialId = (materialId >= 0) ? materialId : 0;
		face->vertexIndices.resize(indices.size());
		for (uint i = 0; i < indices.size(); i++) {
			if (indices[i] >= numVerts) {
				warning("glTF: primitive %d index out of range", p);
				delete face;
				return false;
			}
			face->vertexIndices[i] = indices[i] + vertexOffset;
		}
		faces.push_back(face);
	}

	if (vertices.empty() || faces.empty()) {
		warning("glTF: model has no usable geometry");
		return false;
	}

	// --- commit to the model -------------------------------------------------------
	model->_vertices = vertices;
	model->_materials = materials;
	model->_faces = faces;
	model->_bones = bones;
	data.committed = true;
	model->buildBonesBoundingBoxes();

	debugC(kDebugModding, "glTF: loaded %d vertices, %d faces, %d bones, %d materials",
	       vertices.size(), faces.size(), bones.size(), materials.size());

	return true;
}

namespace {

/** One decoded animation channel: keyframe times (s) and raw values. */
struct GltfChannel {
	Common::Array<float> times;
	Common::Array<float> values; // comps floats per key
	uint comps = 0;

	bool empty() const { return times.empty(); }

	/** Linearly sample the channel at a given time into out[comps]. */
	void sample(float time, float *out) const {
		uint n = times.size();
		if (time <= times[0]) {
			for (uint c = 0; c < comps; c++)
				out[c] = values[c];
			return;
		}
		if (time >= times[n - 1]) {
			for (uint c = 0; c < comps; c++)
				out[c] = values[(n - 1) * comps + c];
			return;
		}
		uint hi = 1;
		while (hi < n - 1 && times[hi] < time)
			hi++;
		uint lo = hi - 1;
		float span = times[hi] - times[lo];
		float t = (span > 0.0f) ? (time - times[lo]) / span : 0.0f;

		// For rotations this is a normalized lerp; over the small spans
		// between baked keyframes it is indistinguishable from slerp.
		const float *a = &values[lo * comps];
		const float *b = &values[hi * comps];
		float sign = 1.0f;
		if (comps == 4) {
			float dot = a[0] * b[0] + a[1] * b[1] + a[2] * b[2] + a[3] * b[3];
			if (dot < 0.0f)
				sign = -1.0f;
		}
		for (uint c = 0; c < comps; c++)
			out[c] = a[c] + (sign * b[c] - a[c]) * t;
		if (comps == 4) {
			float len = sqrtf(out[0] * out[0] + out[1] * out[1] + out[2] * out[2] + out[3] * out[3]);
			if (len > 0.0f) {
				for (uint c = 0; c < 4; c++)
					out[c] /= len;
			}
		}
	}
};

/** Decode a sampler into a GltfChannel, reducing CUBICSPLINE to its values. */
bool readChannel(const GltfContext &ctx, const Common::JSONObject &sampler, uint comps, GltfChannel &out) {
	Common::Array<float> times = readAccessorFloats(ctx, jsInt(sampler, "input"));
	Common::Array<float> values = readAccessorFloats(ctx, jsInt(sampler, "output"));
	if (times.empty() || values.empty())
		return false;

	Common::String interpolation = "LINEAR";
	if (jsGet(sampler, "interpolation"))
		interpolation = jsGet(sampler, "interpolation")->asString();

	if (interpolation == "CUBICSPLINE") {
		// Values come as (in-tangent, value, out-tangent) triples; keep the
		// values and let the engine interpolate linearly between them.
		if (values.size() < times.size() * comps * 3)
			return false;
		Common::Array<float> plain;
		plain.reserve(times.size() * comps);
		for (uint k = 0; k < times.size(); k++) {
			for (uint c = 0; c < comps; c++)
				plain.push_back(values[(k * 3 + 1) * comps + c]);
		}
		values = plain;
	} else if (values.size() < times.size() * comps) {
		return false;
	}

	out.times = times;
	out.values = values;
	out.comps = comps;
	return true;
}

} // End of anonymous namespace

bool GltfAnimReader::load(SkeletonAnim *anim, Common::SeekableReadStream *stream,
                          const Common::String &clipName) {
	GltfContext ctx;
	if (!parseGlb(ctx, stream))
		return false;

	if (!ctx.nodes || !ctx.accessors || !ctx.bufferViews || !ctx.animations || ctx.animations->size() == 0) {
		warning("glTF: the file contains no animations");
		return false;
	}

	// Pick the clip whose name matches the .ani being replaced. A file with
	// a single clip is unambiguous, so its clip is used regardless of name
	// (Blender's default action names rarely match). A multi-clip file with
	// no matching name fails the load, so the original .ani still plays
	// instead of some unrelated clip.
	int clipIdx = -1;
	for (uint i = 0; i < ctx.animations->size(); i++) {
		const Common::JSONObject &clip = (*ctx.animations)[i]->asObject();
		if (jsGet(clip, "name") && jsGet(clip, "name")->asString().equalsIgnoreCase(clipName)) {
			clipIdx = i;
			break;
		}
	}
	if (clipIdx < 0) {
		if (ctx.animations->size() == 1) {
			clipIdx = 0;
		} else {
			warning("glTF: no animation clip named '%s' among the %d clips in the file",
			        clipName.c_str(), ctx.animations->size());
			return false;
		}
	}
	const Common::JSONObject &clip = (*ctx.animations)[clipIdx]->asObject();

	Common::Array<int> nodeToBone = mapNodesToBones(ctx);
	int maxBone = -1;
	for (uint i = 0; i < nodeToBone.size(); i++)
		maxBone = MAX(maxBone, nodeToBone[i]);
	if (maxBone < 0) {
		warning("glTF: no nodes are named bone_NN, cannot map animation to the skeleton");
		return false;
	}
	uint numBones = maxBone + 1;

	// Decode the clip's channels, bucketed per bone.
	Common::Array<GltfChannel> rotChannels;
	Common::Array<GltfChannel> posChannels;
	rotChannels.resize(numBones);
	posChannels.resize(numBones);

	const Common::JSONValue *channelsVal = jsGet(clip, "channels");
	const Common::JSONValue *samplersVal = jsGet(clip, "samplers");
	if (!channelsVal || !channelsVal->isArray() || !samplersVal || !samplersVal->isArray()) {
		warning("glTF: animation clip has no channels");
		return false;
	}
	const Common::JSONArray &channels = channelsVal->asArray();
	const Common::JSONArray &samplers = samplersVal->asArray();

	float duration = 0.0f;
	for (uint i = 0; i < channels.size(); i++) {
		const Common::JSONObject &channel = channels[i]->asObject();
		const Common::JSONValue *targetVal = jsGet(channel, "target");
		if (!targetVal || !targetVal->isObject())
			continue;
		const Common::JSONObject &target = targetVal->asObject();
		int node = jsInt(target, "node");
		if (node < 0 || node >= (int)nodeToBone.size() || nodeToBone[node] < 0)
			continue;
		int bone = nodeToBone[node];

		Common::String path;
		if (jsGet(target, "path"))
			path = jsGet(target, "path")->asString();
		int samplerIdx = jsInt(channel, "sampler");
		if (samplerIdx < 0 || samplerIdx >= (int)samplers.size())
			continue;
		const Common::JSONObject &sampler = samplers[samplerIdx]->asObject();

		GltfChannel decoded;
		if (path == "rotation" && readChannel(ctx, sampler, 4, decoded)) {
			rotChannels[bone] = decoded;
		} else if (path == "translation" && readChannel(ctx, sampler, 3, decoded)) {
			posChannels[bone] = decoded;
		} else {
			continue; // scale and weights are not part of the .ani model
		}
		duration = MAX(duration, decoded.times[decoded.times.size() - 1]);
	}

	if (duration <= 0.0f) {
		warning("glTF: animation clip animates no skeleton bones");
		return false;
	}

	// Build per-bone keys: the union of the bone's channel key times, with
	// both channels sampled at each. Bones the clip doesn't animate hold
	// their node's rest transform.
	anim->_id = 3;
	anim->_ver = 3;
	anim->_u1 = 0;
	anim->_u2 = 0xdeadbabe;
	anim->_time = (uint32)(duration * 1000.0f + 0.5f);
	anim->_boneAnims.clear();
	anim->_boneAnims.resize(numBones);

	for (uint bone = 0; bone < numBones; bone++) {
		const GltfChannel &rc = rotChannels[bone];
		const GltfChannel &pc = posChannels[bone];

		// Rest transform, for missing channels.
		float restRot[4] = {0.0f, 0.0f, 0.0f, 1.0f};
		float restPos[3] = {0.0f, 0.0f, 0.0f};
		for (uint n = 0; n < nodeToBone.size(); n++) {
			if (nodeToBone[n] != (int)bone)
				continue;
			const Common::JSONObject &node = (*ctx.nodes)[n]->asObject();
			const Common::JSONValue *rv = jsGet(node, "rotation");
			if (rv && rv->isArray() && rv->asArray().size() == 4) {
				for (uint c = 0; c < 4; c++)
					restRot[c] = jsNumber(rv->asArray()[c]);
			}
			const Common::JSONValue *tv = jsGet(node, "translation");
			if (tv && tv->isArray() && tv->asArray().size() == 3) {
				for (uint c = 0; c < 3; c++)
					restPos[c] = jsNumber(tv->asArray()[c]);
			}
			break;
		}

		// Merged, deduplicated key times in ms.
		Common::Array<uint32> keyTimes;
		for (uint k = 0; k < rc.times.size(); k++)
			keyTimes.push_back((uint32)(rc.times[k] * 1000.0f + 0.5f));
		for (uint k = 0; k < pc.times.size(); k++)
			keyTimes.push_back((uint32)(pc.times[k] * 1000.0f + 0.5f));
		if (keyTimes.empty())
			keyTimes.push_back(0);
		Common::sort(keyTimes.begin(), keyTimes.end());
		uint unique = 0;
		for (uint k = 0; k < keyTimes.size(); k++) {
			if (k == 0 || keyTimes[k] != keyTimes[unique - 1])
				keyTimes[unique++] = keyTimes[k];
		}
		keyTimes.resize(unique);
		// Make sure the last key covers the whole clip, so time queries near
		// the end don't fall off the key list.
		if (keyTimes[keyTimes.size() - 1] < anim->_time)
			keyTimes.push_back(anim->_time);

		SkeletonAnim::BoneAnim &boneAnim = anim->_boneAnims[bone];
		boneAnim._keys.resize(keyTimes.size());
		for (uint k = 0; k < keyTimes.size(); k++) {
			float timeSec = keyTimes[k] / 1000.0f;
			float rot[4] = {restRot[0], restRot[1], restRot[2], restRot[3]};
			float pos[3] = {restPos[0], restPos[1], restPos[2]};
			if (!rc.empty())
				rc.sample(timeSec, rot);
			if (!pc.empty())
				pc.sample(timeSec, pos);

			SkeletonAnim::AnimKey &key = boneAnim._keys[k];
			key._time = keyTimes[k];
			key._rot = Math::Quaternion(rot[0], rot[1], rot[2], rot[3]);
			key._pos = Math::Vector3d(pos[0], pos[1], pos[2]);
		}
	}

	debugC(kDebugModding, "glTF: loaded animation clip '%s' (%d ms, %d bones)",
	       clipName.c_str(), anim->_time, numBones);

	return true;
}

} // End of namespace Formats
} // End of namespace Stark
