/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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
 */

#include "engines/stark/visual/actor.h"

#include "engines/stark/actor.h"
#include "engines/stark/skeleton.h"
#include "engines/stark/texture.h"
#include "engines/stark/gfx/driver.h"

#include "common/archive.h"
#include "common/stream.h"

#include <SDL_opengl.h> // HACK: I just want to see something

namespace Stark {

VisualActor *VisualActor::load(const Common::Archive *archive, const Common::String &name) {
	Common::ReadStream *stream = archive->createReadStreamForMember(name);
	if (!stream)
		return NULL;

	VisualActor *cir = new VisualActor();
	cir->_actor = new Actor();

	cir->_actor->readFromStream(stream);
	return cir;
}

VisualActor::VisualActor() :
		Visual(TYPE),
		_actor(nullptr) {

}

VisualActor::~VisualActor() {
	if (_actor)
		delete _actor;
}

bool VisualActor::setAnim(const Common::Archive *archive, const Common::String &name) {
	Common::ReadStream *stream = archive->createReadStreamForMember(name);
	if (!stream)
		return false;

	return _actor->setAnim(stream);
}

bool VisualActor::setTexture(const Common::Archive *archive, const Common::String &name) {
	Common::ReadStream *stream = archive->createReadStreamForMember(name);
	if (!stream)
		return false;

	return _actor->setTexture(stream);
}

void VisualActor::update(uint32 delta) {
	_actor->getSkeleton()->animate(delta);
}

void VisualActor::render(Stark::GfxDriver *gfx) {
	// Prepare vertex list and push to gfx driver
	// HACK: Purely because I just want to see something for now
static double ctr = 0;
ctr += .1;
	gfx->set3DMode();
	glPushMatrix();
	glLoadIdentity();
	glScalef(0.005f, .005f, -.005f);
	glTranslatef(0, -20.f, 100.f);
	glRotatef(20, .3f, 1.f, 0.f);

	glRotatef(180, 0.f, 1.f, 0.f);
	//glRotatef((ctr * 10.3f), -4.0f, 10.0f, 1.0f);

	glEnable(GL_TEXTURE_2D);

	Common::Array<BoneNode *> bones = _actor->getSkeleton()->getBones();
	Common::Array<MeshNode *> meshes = _actor->getMeshes();
	Common::Array<MaterialNode *> mats = _actor->getMaterials();
	const Texture *texture = _actor->getTexture();

	for (Common::Array<MeshNode *>::iterator mesh = meshes.begin(); mesh != meshes.end(); ++mesh) {
		for (Common::Array<FaceNode *>::iterator face = (*mesh)->_faces.begin(); face != (*mesh)->_faces.end(); ++face) {
			// For each triangle to draw
			uint32 tex = texture->getTexture(mats[(*face)->_matIdx]->_texName);
				if (tex)
					glColor3f(1.f, 1.f, 1.f);
				else
					glColor3f(mats[(*face)->_matIdx]->_r, mats[(*face)->_matIdx]->_g, mats[(*face)->_matIdx]->_b);

			glBindTexture(GL_TEXTURE_2D, tex);

			glBegin(GL_TRIANGLES);
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


					Coordinate b1 = (*face)->_verts[vertIdx]->_pos1;
					BoneNode *bone = bones[(*face)->_verts[vertIdx]->_bone1];
					b1.rotate(bone->_animPos);
					b1.translate(bone->_animPos);

					Coordinate b2 = (*face)->_verts[vertIdx]->_pos2;
					bone = bones[(*face)->_verts[vertIdx]->_bone2];
					b2.rotate(bone->_animPos);
					b2.translate(bone->_animPos);

					float w = (*face)->_verts[vertIdx]->_boneWeight;
					Coordinate pos = b1 * w + b2 * (1.f - w);
					if (tex)
						glTexCoord2f(-(*face)->_verts[vertIdx]->_texS, (*face)->_verts[vertIdx]->_texT);

					glVertex3f(pos.getX(), pos.getY(), -pos.getZ()); // - is LHS->RHS
				}
			}
			glEnd();
		}
	}

	glPopMatrix();
}

} // End of namespace Stark
