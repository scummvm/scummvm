/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 *
 */

#include "engines/stark/actor.h"
#include "engines/stark/stark.h"

#include "common/archive.h"
#include "common/stream.h"

#include <SDL_opengl.h> // HACK: I just want to see something

namespace Stark {

Actor::Actor() {
	
}

Actor::~Actor() {
	for (Common::Array<MaterialNode *>::iterator it = _materials.begin(); it != _materials.end(); ++it)
		delete *it;

	for (Common::Array<MeshNode *>::iterator it = _meshes.begin(); it != _meshes.end(); ++it)
		delete *it;

	for (Common::Array<BoneNode *>::iterator it = _bones.begin(); it != _bones.end(); ++it)
		delete *it;
}

bool Actor::readFromStream(Common::ReadStream *stream) {
	_id = stream->readUint32LE();
	uint32 format = stream->readUint32LE();
	uint32 u1 = stream->readUint32LE();
	uint32 id2 = stream->readUint32LE();
	if (id2 != 0xDEADBABE)
		return false;

	uint32 u2 = stream->readUint32LE();


	uint32 numMaterials = stream->readUint32LE();
	
	for (uint i = 0; i < numMaterials; ++i) {
		MaterialNode *node = new MaterialNode();
		uint32 len = stream->readUint32LE();
		char *ptr = new char[len];
		stream->read(ptr, len);
		node->_name = Common::String(ptr, len);
		uint32 u3 = stream->readUint32LE();
		delete[] ptr;
		len = stream->readUint32LE();
		ptr = new char[len];
		stream->read(ptr, len);
		node->_texName = Common::String(ptr, len);
		delete[] ptr;
		ptr = new char[12];
		stream->read(ptr, 12);
		node->_r = get_float(ptr);
		node->_g = get_float(ptr + 4);
		node->_b = get_float(ptr + 8);
		_materials.push_back(node);
	}

	uint32 numUnknowns = stream->readUint32LE();

	for (uint32 i = 0; i < numUnknowns; ++i) {
		UnknownNode *node = new UnknownNode();
		char *ptr = new char[16];
		stream->read(ptr, 16);
		node->_u1 = get_float(ptr);
		node->_u2 = get_float(ptr + 4);
		node->_u3 = get_float(ptr + 8);
		node->_u4 = get_float(ptr + 12);
	}

	uint32 numBones = stream->readUint32LE();

	for (uint32 i = 0; i < numBones; ++i) {
		BoneNode *node = new BoneNode();

		uint32 len = stream->readUint32LE();
		char *ptr = new char[len + 4];
		stream->read(ptr, len + 4);
		node->_name = Common::String(ptr, len);
		node->_u1 = get_float(ptr + len);
		delete[] ptr;

		len = stream->readUint32LE();
		for (uint32 j = 0; j < len; ++j)
			node->_children.push_back(stream->readUint32LE());

		_bones.push_back(node);
	}

	for (uint32 i = 0; i < numBones; ++i) {
		BoneNode *node = _bones[i];
		for (int j = 0; j < node->_children.size(); ++j) {
			_bones[node->_children[j]]->_parent = i;
		}
	}

	uint32 numMeshes = stream->readUint32LE();

	for (uint32 i = 0; i < numMeshes; ++i) {
		MeshNode *node = new MeshNode();

		uint32 len = stream->readUint32LE();
		char *ptr = new char[len];
		stream->read(ptr, len);

		node->_name = Common::String(ptr, len);
		delete[] ptr;
		len = stream->readUint32LE();
		for (uint32 j = 0; j < len; ++j) {
			FaceNode *face = new FaceNode();
			face->_matIdx = stream->readUint32LE();

			uint32 childCount = stream->readUint32LE();
			for (uint32 k = 0; k < childCount; ++k) {
				VertNode *vert = new VertNode();
				ptr = new char[14 * 4];
				stream->read(ptr, 14 * 4);
				vert->_pos1 = Graphics::Vector3d(get_float(ptr), get_float(ptr + 4), get_float(ptr + 8));
				vert->_pos2 = Graphics::Vector3d(get_float(ptr + 12), get_float(ptr + 16), get_float(ptr + 20));
				vert->_normal = Graphics::Vector3d(get_float(ptr + 24), get_float(ptr + 28), get_float(ptr + 32));
				vert->_texS = get_float(ptr + 36);
				vert->_texT = get_float(ptr + 40);
				vert->_bone1 = READ_LE_UINT32(ptr + 44);
				vert->_bone2 = READ_LE_UINT32(ptr + 48);
				vert->_boneWeight = get_float(ptr + 52);
				delete[] ptr;
				face->_verts.push_back(vert);
			}

			childCount = stream->readUint32LE();
			for (uint32 k = 0; k < childCount; ++k) {
				TriNode *tri = new TriNode();
				tri->_vert1 = stream->readUint32LE();
				tri->_vert2 = stream->readUint32LE();
				tri->_vert3 = stream->readUint32LE();
				face->_tris.push_back(tri);
			}

			node->_faces.push_back(face);
		}

		_meshes.push_back(node);
	}

	return true;
}

bool Animation::readFromStream(Common::ReadStream *stream) {
	_id = stream->readUint32LE();
	_ver = stream->readUint32LE();
	if (_ver == 3) {
		_u1 = 0;
		_time = stream->readUint32LE();
		_u2 = stream->readUint32LE();
	} else {
		_u1 = stream->readUint32LE();
		_u2 = stream->readUint32LE();
		_time = stream->readUint32LE();
	}
	if (_u2 != 0xdeadbabe)
		return false;

	uint32 num = stream->readUint32LE();
	_anims.resize(num);
	for (uint32 i = 0; i < num; ++i) {
		AnimNode *node = new AnimNode();
		node->_bone = stream->readUint32LE();
		uint32 numKeys = stream->readUint32LE();

		for (uint32 j = 0; j < numKeys; ++j) {
			AnimKey *key = new AnimKey();
			key->_time = stream->readUint32LE();
			char *ptr = new char[7 * 4];
			stream->read(ptr, 7 * 4);
			key->_rot = Graphics::Vector3d(get_float(ptr), get_float(ptr + 4), get_float(ptr + 8));
			key->_rotW = get_float(ptr + 12);
			key->_pos = Graphics::Vector3d(get_float(ptr + 16), get_float(ptr + 20), get_float(ptr + 24));
			node->_keys.push_back(key);
		}

		_anims[node->_bone] = node;
	}

	return true;
}

SceneElementActor *SceneElementActor::load(const Common::Archive *archive, const Common::String &name) {
	Common::ReadStream *stream = archive->createReadStreamForMember(name);
	if (!stream)
		return NULL;

	SceneElementActor *cir = new SceneElementActor();
	cir->_actor = new Actor();
	
	cir->_actor->readFromStream(stream);
	return cir;
}

SceneElementActor::SceneElementActor() : _actor(NULL), _anim(NULL) {

}

SceneElementActor::~SceneElementActor() {
	if (_actor)
		delete _actor;
}

bool SceneElementActor::setAnim(const Common::Archive *archive, const Common::String &name) {
	Common::ReadStream *stream = archive->createReadStreamForMember(name);
	if (!stream)
		return false;

	Animation *oldAnim = _anim;

	_anim = new Animation();
	if (_anim->readFromStream(stream))
	{
		delete oldAnim;
		return true;
	}
	else {
		delete _anim;
		_anim = oldAnim;
		return false;
	}
}

// HACK: Matrix generation for below code
#define _Q_MAT(w, x, y, z) {1 - 2 * y * y - 2 * z * z,    2 * x * y + 2 * z * w,        2 * x * z - 2 * y * w, \
							   2 * x * y - 2 * w * z,    1 - 2 * x * x - 2 * z * z,      2 * y * z + 2 * w * x, \
							   2 * x * z + 2 * w * y,      2 * y * z - 2 * x * w,      1 - 2 * x * x - 2 * y * y, \
						};

#define _VECT_ROTATE(v, r) (v.set(v.x() * r[0] + v.y() * r[1] + v.z() * r[2], \
								v.x() * r[3] + v.y() * r[4] + v.z() * r[5], \
								v.x() * r[6] + v.y() * r[7] + v.z() * r[8]));

void SceneElementActor::render(Stark::GfxDriver *gfx) {
	// Prepare vertex list and push to gfx driver
	// HACK: Purely because I just want to see something for now
static double ctr = 0;
ctr += .1;
	gfx->set3DMode();
	glPushMatrix();
	glLoadIdentity();
	glScalef(0.005f, .005f, -.005f);
	glTranslatef(0, -20.f, 100.f);
	glRotatef(180, 0, 1, 0);
	glRotatef((ctr * 10.3f), -4.0f, 10.0f, 1.0f);

	glBegin(GL_TRIANGLES);

	Common::Array<BoneNode *> bones = _actor->getBones();
	Common::Array<MeshNode *> meshes = _actor->getMeshes();
	Common::Array<MaterialNode *> mats = _actor->getMaterials();

	Common::Array<AnimNode *> anims = _anim->getNodes();

	for (Common::Array<MeshNode *>::iterator mesh = meshes.begin(); mesh != meshes.end(); ++mesh) {
		for (Common::Array<FaceNode *>::iterator face = (*mesh)->_faces.begin(); face != (*mesh)->_faces.end(); ++face) {
			// For each triangle to draw
			glColor3f(mats[(*face)->_matIdx]->_r, mats[(*face)->_matIdx]->_g, mats[(*face)->_matIdx]->_b);
			for (Common::Array<TriNode *>::iterator tri = (*face)->_tris.begin(); tri != (*face)->_tris.end(); ++tri) {
				// Contains 3 vertices
				// Each vertex relative to a bone coordinate
				// 'move' to join location and rotation, then place the vertex there
			
				for (int vert = 0; vert < 3; ++vert) {
					int vertIdx;
					if (vert == 0)
						vertIdx = (*tri)->_vert1;
					else if (vert == 1)
						vertIdx = (*tri)->_vert3;
					else
						vertIdx = (*tri)->_vert2;
				
					int idx = -1;
					for (uint32 i = 0; i < anims.size(); ++i) {
						if (anims[i]->_bone == (*face)->_verts[vertIdx]->_bone1) {
							idx = i;
							break;
						}
					}

					if (idx == -1)
						continue;

					AnimKey *key1 = anims[idx]->_keys[0];
					if (key1->_time != 0)
						continue;

					idx = -1;
					for (uint32 i = 0; i < anims.size(); ++i) {
						if (anims[i]->_bone == (*face)->_verts[vertIdx]->_bone2) {
							idx = i;
							break;
						}
					}

					if (idx == -1)
						continue;

					AnimKey *key2 = anims[idx]->_keys[0];
					if (key2->_time != 0)
						continue;

					Graphics::Vector3d b1 = (*face)->_verts[vertIdx]->_pos1;
					idx = (*face)->_verts[vertIdx]->_bone1;
					BoneNode *bone;
					do {
						bone = bones[idx];
						key1 = anims[idx]->_keys[0];
						Graphics::Vector3d tmp = key1->_pos;
						float tmpRot[] = _Q_MAT(-key1->_rotW, key1->_rot.x(), key1->_rot.y(), key1->_rot.z()); // - is LH to RH
						_VECT_ROTATE(b1, tmpRot);
						b1 += tmp;
						idx = bone->_parent;
						
					} while (idx != -1);

					Graphics::Vector3d b2 = (*face)->_verts[vertIdx]->_pos2;
					idx = (*face)->_verts[vertIdx]->_bone2;
					do {
						bone = bones[idx];
						key2 = anims[idx]->_keys[0];
						Graphics::Vector3d tmp = key2->_pos;
						float tmpRot[] = _Q_MAT(-key2->_rotW, key2->_rot.x(), key2->_rot.y(), key2->_rot.z()); // - is LH to RH
						_VECT_ROTATE(b2, tmpRot);
						b2 += tmp;
						idx = bone->_parent;
						
					} while (idx != -1);

					float w = (*face)->_verts[vertIdx]->_boneWeight;
					Graphics::Vector3d pos = b1 * w + b2 * (1.f - w);
					
					glVertex3f(pos.x(), pos.y(), -pos.z()); // - is LHS->RHS
				}
			}

		}
	}
	
	glEnd();

	glPopMatrix();
}

} // end of namespace Stark
