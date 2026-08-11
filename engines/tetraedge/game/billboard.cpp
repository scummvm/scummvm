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

#include "common/textconsole.h"

#include "tetraedge/tetraedge.h"
#include "tetraedge/game/billboard.h"
#include "tetraedge/game/syberia_game.h"

#include "tetraedge/te/te_core.h"

namespace Tetraedge {

Billboard::Billboard() : _hasPos2(false) {
}

bool Billboard::load(const Common::Path &path) {
	_model = new TeModel();
	TeIntrusivePtr<Te3DTexture> texture = Te3DTexture::makeInstance();
	SyberiaGame *game = dynamic_cast<SyberiaGame *>(g_engine->getGame());
	TeCore *core = g_engine->getCore();
	TetraedgeFSNode texnode = core->findFile(game->sceneZonePath().join(path));
	texture->load(texnode);
	_model->setName(path.toString('/'));
	Common::Array<TeVector3f32> quad;
	quad.resize(4);
	_model->setQuad(texture, quad, TeColor(0xff, 0xff, 0xff, 0xff));
	_model->setVisible(false);
	game->scene().models().push_back(_model);
	return true;
}

void Billboard::calcVertex() {
	Game *game = g_engine->getGame();
	TeIntrusivePtr<TeCamera> currentCam = game->scene().currentCamera();
	assert(currentCam);
	currentCam->apply();
	const TeMatrix4x4 camProjMatrix = currentCam->projectionMatrix();
	TeMatrix4x4 camWorldInverse = currentCam->worldTransformationMatrix();
	camWorldInverse.inverse();

	const TeMatrix4x4 camTotalTransform = camProjMatrix * camWorldInverse;
	TeMatrix4x4 camTotalInverse = camTotalTransform;
	camTotalInverse.inverse();

	TeVector3f32 posvec(0.0f, 0.0f, _pos.z());
	if (_hasPos2) {
		posvec = _pos2;
	}
	posvec = camTotalTransform * posvec;

	TeVector3f32 meshVertex;
	float fx, fy;

	fx = _pos.x();
	fy = _pos.y();
	meshVertex = camTotalInverse * TeVector3f32(fx + fx - 1.0f, fy + fy - 1.0f, posvec.z());
	_model->meshes()[0]->setVertex(0, meshVertex);

	fx = _pos.x();
	fy = _pos.y() + _size.getY();
	meshVertex = camTotalInverse * TeVector3f32(fx + fx - 1.0f, fy + fy - 1.0f, posvec.z());
	_model->meshes()[0]->setVertex(1, meshVertex);

	fx = _pos.x() + _size.getX();
	fy = _pos.y();
	meshVertex = camTotalInverse * TeVector3f32(fx + fx - 1.0f, fy + fy - 1.0f, posvec.z());
	_model->meshes()[0]->setVertex(2, meshVertex);

	fx = _pos.x() + _size.getX();
	fy = _pos.y() + _size.getY();
	meshVertex = camTotalInverse * TeVector3f32(fx + fx - 1.0f, fy + fy - 1.0f, posvec.z());
	_model->meshes()[0]->setVertex(3, meshVertex);
}

void Billboard::position(const TeVector3f32 &pos) {
	_pos = pos;
	calcVertex();
}

void Billboard::position2(const TeVector3f32 &pos) {
	_pos2 = pos;
	_hasPos2 = true;
	calcVertex();
}

void Billboard::size(const TeVector2f32 &size) {
	_size = size;
	calcVertex();
}

} // end namespace Tetraedge
