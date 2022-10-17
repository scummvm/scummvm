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
#include "common/path.h"
#include "common/textconsole.h"

#include "tetraedge/tetraedge.h"
#include "tetraedge/game/application.h"
#include "tetraedge/game/billboard.h"
#include "tetraedge/game/game.h"
#include "tetraedge/game/in_game_scene.h"
#include "tetraedge/game/character.h"
#include "tetraedge/game/characters_shadow.h"
#include "tetraedge/game/object3d.h"
#include "tetraedge/game/scene_lights_xml_parser.h"

#include "tetraedge/te/te_bezier_curve.h"
#include "tetraedge/te/te_camera.h"
#include "tetraedge/te/te_free_move_zone.h"
#include "tetraedge/te/te_light.h"
#include "tetraedge/te/te_renderer.h"

namespace Tetraedge {

InGameScene::InGameScene() : _character(nullptr), _charactersShadow(nullptr), _shadowLightNo(-1) {
}

void InGameScene::activateAnchorZone(const Common::String &name, bool val) {
	for (AnchorZone *zone : _anchorZones) {
		if (zone->_name == name)
			zone->_activated = val;
	}
}

void InGameScene::draw() {
	TeScene::draw();

	if (currentCameraIndex() >= (int)cameras().size())
		return;

	currentCamera()->apply();
	TeLight::updateGlobal();
	for (unsigned int i = 0; i < _lights.size(); i++)
		_lights[i].update(i);

	currentCamera()->restore();
}

void InGameScene::drawPath() {
	if (currentCameraIndex() >= (int)cameras().size())
		return;

	currentCamera()->apply();
	g_engine->getRenderer()->disableZBuffer();

	warning("TODO: Do free move zones in InGameScene::drawPath");
	//for (unsigned int i = 0; i < _freeMoveZones.size(); i++)
	//	_freeMoveZones[i]->

	g_engine->getRenderer()->enableZBuffer();
}


bool InGameScene::changeBackground(const Common::String &name) {
	if (Common::File::exists(name)) {
		_bgGui.spriteLayoutChecked("root")->load(name);
		return true;
	}
	return false;
}

/*static*/
float InGameScene::angularDistance(float a1, float a2) {
	float result;

	result = a2 - a1;
	if (result >= -3.141593 && result > 3.141593) {
		result = result + -6.283185;
	} else {
		result = result + 6.283185;
	}
	return result;
}

TeLayout *InGameScene::background() {
	return _bgGui.layout("background");
}

void InGameScene::close() {
	reset();
	error("TODO: Implement InGameScene::close");
}

void InGameScene::reset() {
	if (_character)
		_character->setFreeMoveZone(nullptr);
	freeSceneObjects();
	_bgGui.unload();
	unloadSpriteLayouts();
	_gui2.unload();
	_gui3.unload();
}

void InGameScene::deleteAllCallback() {
	warning("TODO: implement InGameScene::deleteAllCallback");
}

void InGameScene::freeSceneObjects() {
	if (_character) {
		_character->setCharLookingAt(nullptr);
		_character->deleteAllCallback();
	}
	if (_characters.size() == 1) {
		_characters[0]->deleteAllCallback();
	}

	Game *game = g_engine->getGame();
	game->unloadCharacters();

	_characters.clear();

	for (Object3D *obj : _object3Ds) {
		obj->deleteLater();
	}
	_object3Ds.clear();

	for (Billboard *bb : _billboards) {
		bb->deleteLater();
	}
	_billboards.clear();

	for (TeSpriteLayout *sprite : _sprites) {
		sprite->deleteLater();
	}
	_sprites.clear();

	deleteAllCallback();
	_markers.clear();

	for (InGameScene::AnchorZone *zone : _anchorZones) {
		delete zone;
	}
	_anchorZones.clear();
}

void InGameScene::unloadSpriteLayouts() {
	for (auto *animobj : _animObjects) {
		delete animobj;
	}
	_animObjects.clear();
}

Character *InGameScene::character(const Common::String &name) {
	error("TODO: Implement InGameScene::character");
}

bool InGameScene::load(const Common::Path &path) {
	_actZones.clear();
	Common::File actzonefile;
	if (actzonefile.open(getActZoneFileName())) {
		if (Te3DObject2::loadAndCheckFourCC(actzonefile, "0TCA")) {
			uint32 count = actzonefile.readUint32LE();
			if (count > 1000000)
				error("Improbable number of actzones %d", count);
			_actZones.resize(count);
			for (unsigned int i = 0; i < _actZones.size(); i++) {
				_actZones[i].s1 = Te3DObject2::deserializeString(actzonefile);
				_actZones[i].s2 = Te3DObject2::deserializeString(actzonefile);
				for (int j = 0; j < 4; j++)
					TeVector2f32::deserialize(actzonefile, _actZones[i].points[j]);
				_actZones[i].flag1 = (actzonefile.readByte() != 0);
				_actZones[i].flag2 = true;
			}
		}
	}
	if (!_lights.empty()) {
		TeLight::disableAll();
		for (unsigned int i = 0; i < _lights.size(); i++) {
			_lights[i].disable(i);
		}
		_lights.clear();
	}
	_shadowLightNo = -1;

	const Common::Path lightspath = getLightsFileName();
	if (Common::File::exists(lightspath))
		loadLights(lightspath);

	if (!Common::File::exists(path))
		return false;

	TeScene::close();
	_loadedPath = path;
	Common::File scenefile;
	if (!scenefile.open(path))
		return false;

	uint32 ncameras = scenefile.readUint32LE();
	for (unsigned int i = 0; i < ncameras; i++) {
		TeIntrusivePtr<TeCamera> cam = new TeCamera();
		deserializeCam(scenefile, cam);
		cameras().push_back(cam);
	}

	uint32 nobjects = scenefile.readUint32LE();
	for (unsigned int i = 0; i < nobjects; i++) {
		TeIntrusivePtr<TeModel> model = new TeModel();
		const Common::String modelname = Te3DObject2::deserializeString(scenefile);
		model->setName(modelname);
		const Common::String objname = Te3DObject2::deserializeString(scenefile);
		TePickMesh2 *pickmesh = new TePickMesh2();
		deserializeModel(scenefile, model, pickmesh);
		if (modelname.contains("Clic")) {
			_hitObjects.push_back(model);
			// TODO: double-check this, probably right?
			model->setVisible(false);
			model->setColor(TeColor(0, 0xff, 0, 0xff));
			models().push_back(model);
			pickmesh->setName(modelname);
		} else {
			delete pickmesh;
			if (modelname.substr(0, 2) != "ZB") {
				if (objname.empty()) {
					warning("[InGameScene::load] Unknown type of object named : %s", modelname.c_str());
				} else {
					InGameScene::Object obj;
					obj._name = objname;
					obj._model = model;
					_objects.push_back(obj);
					model->setVisible(false);
					models().push_back(model);
				}
			}
		}
	}

	uint32 nfreemovezones = scenefile.readUint32LE();
	for (unsigned int i = 0; i < nfreemovezones; i++) {
		TeFreeMoveZone *zone = new TeFreeMoveZone();
		TeFreeMoveZone::deserialize(scenefile, *zone, &_blockers, &_rectBlockers, &_actZones);

	}

	uint32 ncurves = scenefile.readUint32LE();
	for (unsigned int i = 0; i < ncurves; i++) {
		TeIntrusivePtr<TeBezierCurve> curve = new TeBezierCurve();
		TeBezierCurve::deserialize(scenefile, *curve);
		curve->setVisible(true);
		_bezierCurves.push_back(curve);
	}

	uint32 ndummies = scenefile.readUint32LE();
	for (unsigned int i = 0; i < ndummies; i++) {
		InGameScene::Dummy dummy;
		TeVector3f32 vec;
		TeQuaternion rot;
		dummy._name = Te3DObject2::deserializeString(scenefile);
		TeVector3f32::deserialize(scenefile, vec);
		dummy._position = vec;
		TeQuaternion::deserialize(scenefile, rot);
		dummy._rotation = rot;
		TeVector3f32::deserialize(scenefile, vec);
		dummy._scale = vec;
		_dummies.push_back(dummy);
	}

	for (TeFreeMoveZone *zone : _freeMoveZones) {
		convertPathToMesh(zone);
	}
	_charactersShadow = new CharactersShadow();
	_charactersShadow->create(this);
	onMainWindowSizeChanged();
	return true;
}

void InGameScene::convertPathToMesh(TeFreeMoveZone *zone) {
	error("TODO: Implement InGameScene::convertPathToMesh");
}

void InGameScene::onMainWindowSizeChanged() {
	error("TODO: Implement InGameScene::onMainWindowSizeChanged");
}

bool InGameScene::loadLights(const Common::Path &path) {
	SceneLightsXmlParser parser;

	parser.setLightArray(&_lights);

	if (!parser.loadFile(path.toString()))
		error("InGameScene::loadLights: Can't load %s", path.toString().c_str());
	if (!parser.parse())
		error("InGameScene::loadLights: Can't parse %s", path.toString().c_str());

	_shadowColor = parser.getShadowColor();
	_shadowLightNo = parser.getShadowLightNo();
	_shadowFarPlane = parser.getShadowFarPlane();
	_shadowNearPlane = parser.getShadowNearPlane();
	_shadowFov = parser.getShadowFov();

	return true;
}

bool InGameScene::loadCharacter(const Common::String &name) {
	error("TODO: Implement InGameScene::loadCharacter");
}

void InGameScene::deserializeCam(Common::ReadStream &stream, TeIntrusivePtr<TeCamera> &cam) {
	cam->_projectionMatrixType = 2;
	cam->viewport(0, 0, _viewportSize.getX(), _viewportSize.getY());
	Te3DObject2::deserialize(stream, *cam);
	cam->_focalLenMaybe = stream.readFloatLE();
	cam->_somePerspectiveVal = stream.readFloatLE();
	cam->_orthNearVal = stream.readFloatLE();
	cam->_orthFarVal = 3000.0;
}

void InGameScene::deserializeModel(Common::ReadStream &stream, TeIntrusivePtr<TeModel> &model, TePickMesh2 *pickmesh) {
	TeVector3f32 vec;
	TeVector2f32 vec2;
	TeQuaternion rot;
	TeColor col;
	TeMesh mesh;

	TeVector3f32::deserialize(stream, vec);
	model->setPosition(vec);
	pickmesh->setPosition(vec);
	TeQuaternion::deserialize(stream, rot);
	model->setRotation(rot);
	pickmesh->setRotation(rot);
	TeVector3f32::deserialize(stream, vec);
	model->setScale(vec);
	pickmesh->setScale(vec);
	uint32 indexcount = stream.readUint32LE();
	uint32 vertexcount = stream.readUint32LE();

	mesh.setConf(vertexcount, indexcount, TeMesh::MeshMode_Triangles, 0, 0);
	for (unsigned int i = 0; i < indexcount; i++)
		mesh.setIndex(i, stream.readUint32LE());
	for (unsigned int i = 0; i < vertexcount; i++) {
		TeVector3f32::deserialize(stream, vec);
		mesh.setVertex(i, vec);
	}
	for (unsigned int i = 0; i < vertexcount; i++) {
		TeVector3f32::deserialize(stream, vec);
		mesh.setNormal(i, vec);
	}
	for (unsigned int i = 0; i < vertexcount; i++) {
		TeVector2f32::deserialize(stream, vec2);
		mesh.setTextureUV(i, vec2);
	}
	for (unsigned int i = 0; i < vertexcount; i++) {
		col.deserialize(stream);
		mesh.setColor(i, col);
	}
	pickmesh->setNbTriangles(indexcount / 3);
	for (unsigned int i = 0; i < indexcount; i++) {
		vec = mesh.vertex(mesh.index(i));
		pickmesh->verticies().push_back(vec);
	}
	model->addMesh(mesh);
}

bool InGameScene::loadPlayerCharacter(const Common::String &name) {
	if (_character == nullptr) {
		_character = new Character();
		if (!_character->loadModel(name, true)) {
			_playerCharacterModel.release();
			return false;
		}

		_playerCharacterModel = _character->_model;

		if (!findKate()) {
			Common::Array<TeIntrusivePtr<TeModel>> &ms = models();
			ms.push_back(_character->_model);
			ms.push_back(_character->_shadowModel[0]);
			ms.push_back(_character->_shadowModel[1]);
		}
	}

	_character->_model->setVisible(true);
	return true;
}

void InGameScene::unloadCharacter(const Common::String &name) {
	warning("TODO: Implement InGameScene::unloadCharacter %s", name.c_str());
}

bool InGameScene::findKate() {
	for (auto &m : models()) {
		if (m->name() == "Kate")
			return true;
	}
	return false;
}

static Common::Path _sceneFileNameBase() {
	Game *game = g_engine->getGame();
	Common::Path retval("scenes");
	retval.joinInPlace(game->currentZone());
	retval.joinInPlace(game->currentScene());
	return retval;
}

Common::Path InGameScene::getLightsFileName() const {
	return _sceneFileNameBase().joinInPlace("lights.xml");
}

Common::Path InGameScene::getActZoneFileName() const {
	return _sceneFileNameBase().joinInPlace("actions.bin");
}

Common::Path InGameScene::getBlockersFileName() const {
	return _sceneFileNameBase().joinInPlace("blockers.bin");
}

void InGameScene::loadBlockers() {
	_blockers.clear();
	_rectBlockers.clear();
	const Common::Path blockersPath = getBlockersFileName();
	if (!Common::File::exists(blockersPath))
		return;

	Common::File blockersfile;
	if (!blockersfile.open(blockersPath)) {
		warning("Couldn't open blockers file %s.", blockersPath.toString().c_str());
		return;
	}

	bool hasHeader = Te3DObject2::loadAndCheckFourCC(blockersfile, "BLK0");
	if (!hasHeader)
		blockersfile.seek(0);

	uint32 nblockers = blockersfile.readUint32LE();
	_blockers.resize(nblockers);
	for (unsigned int i = 0; i < nblockers; i++) {
		_blockers[i]._s = Te3DObject2::deserializeString(blockersfile);
		TeVector2f32::deserialize(blockersfile, _blockers[i]._pts[0]);
		TeVector2f32::deserialize(blockersfile, _blockers[i]._pts[1]);
		_blockers[i]._x = 1;
	}

	if (hasHeader) {
		uint32 nrectblockers = blockersfile.readUint32LE();
		_rectBlockers.resize(nrectblockers);
		for (unsigned int i = 0; i < nrectblockers; i++) {
			_rectBlockers[i]._s = Te3DObject2::deserializeString(blockersfile);
			for (unsigned int j = 0; j < 4l; j++) {
				TeVector2f32::deserialize(blockersfile, _rectBlockers[i]._pts[j]);
			}
			_rectBlockers[i]._x = 1;
		}
	}
}

void InGameScene::loadBackground(const Common::Path &path) {
	_bgGui.load(path);
	TeLayout *bg = _bgGui.layout("background");
	TeLayout *root = _bgGui.layout("root");
	bg->setRatioMode(TeILayout::RATIO_MODE_NONE);
	root->setRatioMode(TeILayout::RATIO_MODE_NONE);
	TeCamera *wincam = g_engine->getApplication()->mainWindowCamera();
	bg->disableAutoZ();
	bg->setZPosition(wincam->_orthNearVal);

	for (auto layoutEntry : _bgGui.spriteLayouts()) {
		AnimObject *animobj = new AnimObject();
		animobj->_name = layoutEntry._key;
		animobj->_layout = layoutEntry._value;
		animobj->_layout->_tiledSurfacePtr->_frameAnim.onFinished().add<AnimObject>(animobj, &AnimObject::onFinished);
		if (animobj->_name != "root")
			animobj->_layout->setVisible(false);
		_animObjects.push_back(animobj);
	}
}

void InGameScene::loadInteractions(const Common::Path &path) {
	_gui3.load(path);
	TeLayout *bgbackground = _bgGui.layoutChecked("background");
	Game *game = g_engine->getGame();
	TeSpriteLayout *root = game->findSpriteLayoutByName(bgbackground, "root");
	TeLayout *background = _gui3.layoutChecked("background");
	// TODO: For all TeButtonLayout childen of background, call
	// setDoubleValidationProtectionEnabled(false)
	// For now our button doesn't implement that.
	background->setRatioMode(TeILayout::RATIO_MODE_NONE);
	root->addChild(background);
}

void InGameScene::setStep(const Common::String &scene, const Common::String &step1, const Common::String &step2) {
	SoundStep ss;
	ss._stepSound1 = step1;
	ss._stepSound2 = step2;
	_soundSteps[scene] = ss;
}

void InGameScene::initScroll() {
	_someScrollVector = TeVector2f32(0.5f, 0.0f);
}

bool InGameScene::AnimObject::onFinished() {
	error("TODO: Implement InGameScene::AnimObject::onFinished");
}

} // end namespace Tetraedge
