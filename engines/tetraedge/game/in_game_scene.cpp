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
#include "tetraedge/game/syberia_game.h"
#include "tetraedge/game/in_game_scene.h"
#include "tetraedge/game/in_game_scene_xml_parser.h"
#include "tetraedge/game/character.h"
#include "tetraedge/game/characters_shadow.h"
#include "tetraedge/game/object3d.h"
#include "tetraedge/game/particle_xml_parser.h"
#include "tetraedge/game/scene_lights_xml_parser.h"

#include "tetraedge/te/te_bezier_curve.h"
#include "tetraedge/te/te_camera.h"
#include "tetraedge/te/te_core.h"
#include "tetraedge/te/te_free_move_zone.h"
#include "tetraedge/te/te_light.h"
#include "tetraedge/te/te_renderer.h"
#include "tetraedge/te/te_lua_script.h"
#include "tetraedge/te/te_lua_thread.h"

//#define TETRAEDGE_DEBUG_PATHFINDING
//#define TETRAEDGE_DEBUG_LIGHTS

namespace Tetraedge {

/*static*/
const int InGameScene::MAX_FIRE = 50;
const int InGameScene::MAX_SNOW = 250;
const int InGameScene::MAX_SMOKE = 350;
const float InGameScene::DUREE_MAX_FIRE = 32000.0f;
const float InGameScene::SCALE_FIRE = 0.1f;
const int InGameScene::MAX_FLAKE = 10;
const float InGameScene::DUREE_MIN_FLAKE = 3000.0f;
const float InGameScene::DUREE_MAX_FLAKE = 5000.0f;
const float InGameScene::SCALE_FLAKE = 0.1f;
const float InGameScene::DEPTH_MAX_FLAKE = 0.1f;


InGameScene::InGameScene() : _character(nullptr), _charactersShadow(nullptr),
_shadowLightNo(-1), _waitTime(-1.0), _shadowColor(0, 0, 0, 0x80), _shadowFov(20.0f),
_shadowFarPlane(1000), _shadowNearPlane(1), _maskAlpha(false),
_verticalScrollTime(1000000.0f), _verticalScrollPlaying(false) {
}

void InGameScene::activateAnchorZone(const Common::String &name, bool val) {
	for (AnchorZone *zone : _anchorZones) {
		if (zone->_name == name)
			zone->_activated = val;
	}
}

void InGameScene::addAnchorZone(const Common::String &s1, const Common::String &name, float radius) {
	for (AnchorZone *zone : _anchorZones) {
		if (zone->_name == name) {
			zone->_radius = radius;
			return;
		}
	}

	assert(currentCamera());
	currentCamera()->apply();
	AnchorZone *zone = new AnchorZone();
	zone->_name = name;
	zone->_radius = radius;
	zone->_activated = true;

	if (s1.contains("Int")) {
		TeButtonLayout *btn = hitObjectGui().buttonLayoutChecked(name);
		TeVector3f32 pos = btn->position();
		pos.x() += g_engine->getDefaultScreenWidth() / 2.0f;
		pos.y() += g_engine->getDefaultScreenHeight() / 2.0f;
		zone->_loc = currentCamera()->worldTransformationMatrix() * currentCamera()->transformPoint2Dto3D(pos);
	} else {
		if (s1.contains("Dummy")) {
			Dummy d = dummy(name);
			zone->_loc = d._position;
		}
	}
	_anchorZones.push_back(zone);
}

bool InGameScene::addMarker(const Common::String &markerName, const Common::Path &imgPath, float x, float y, const Common::String &locType, const Common::String &markerVal, float anchorX, float anchorY) {
	const TeMarker *marker = findMarker(markerName);
	if (!marker) {
		SyberiaGame *game = dynamic_cast<SyberiaGame *>(g_engine->getGame());
		assert(game);
		Application *app = g_engine->getApplication();
		TeSpriteLayout *markerSprite = new TeSpriteLayout();
		// Note: game checks paths here but seems to just use the original?
		markerSprite->setName(markerName);
		markerSprite->setAnchor(TeVector3f32(anchorX, anchorY, 0.0f));
		if (!markerSprite->load(imgPath) && imgPath.baseName().hasSuffix(".anim"))
			markerSprite->load(imgPath.append("cached"));
		markerSprite->setSizeType(TeILayout::RELATIVE_TO_PARENT);
		markerSprite->setPositionType(TeILayout::RELATIVE_TO_PARENT);
		TeVector3f32 newPos;
		if (locType == "PERCENT") {
			TeVector3f32 parentSize;
			//if (g_engine->gameType() == TetraedgeEngine::kSyberia)
				parentSize = app->frontLayout().userSize();
			//else
			//	parentSize = app->getMainWindow().size();
			newPos.x() = parentSize.x() * (x / 100.0f);
			newPos.y() = parentSize.y() * (y / 100.0f);
		} else {
			newPos.x() = x / g_engine->getDefaultScreenWidth();
			newPos.y() = y / g_engine->getDefaultScreenHeight();
		}
		markerSprite->setPosition(newPos);

		const TeVector3f32 winSize = app->getMainWindow().size();
		float xscale = 1.0f;
		float yscale = 1.0f;

		// Originally this is only done in Syberia 2, but
		// should be fine to calculate in Syberia 1, as long
		// as the root layout is loaded.
		TeLayout *bglayout = _bgGui.layoutChecked("background");
		TeSpriteLayout *rootlayout = Game::findSpriteLayoutByName(bglayout, "root");
		if (rootlayout && rootlayout->_tiledSurfacePtr && rootlayout->_tiledSurfacePtr->tiledTexture()) {
			TeVector2s32 bgSize = rootlayout->_tiledSurfacePtr->tiledTexture()->totalSize();
			xscale = 800.0f / bgSize._x;
			yscale = 600.0f / bgSize._y;
		}

		if (g_engine->getCore()->fileFlagSystemFlag("definition") == "SD") {
			markerSprite->setSize(TeVector3f32(xscale * 0.07f, yscale * (4.0f / ((winSize.y() / winSize.x()) * 4.0f)) * 0.07f, 0.0));
		} else {
			markerSprite->setSize(TeVector3f32(xscale * 0.04f, yscale * (4.0f / ((winSize.y() / winSize.x()) * 4.0f)) * 0.04f, 0.0));
		}
		markerSprite->setVisible(game->markersVisible());
		markerSprite->_tiledSurfacePtr->_frameAnim.setLoopCount(-1);
		markerSprite->play();

		TeMarker newMarker;
		newMarker._name = markerName;
		newMarker._val = markerVal;
		_markers.push_back(newMarker);
		TeLayout *bg = game->forGui().layout("background");
		if (bg)
			bg->addChild(markerSprite);
		_sprites.push_back(markerSprite);
	} else  {
		setImagePathMarker(markerName, imgPath);
	}
	return true;
}

/*static*/
float InGameScene::angularDistance(float a1, float a2) {
	float result = a2 - a1;
	if (result >= -M_PI && result > M_PI) {
		result = result - (M_PI * 2);
	} else {
		result = result + (M_PI * 2);
	}
	return result;
}

bool InGameScene::aroundAnchorZone(const AnchorZone *zone) {
	if (!zone->_activated)
		return false;
	const TeVector3f32 charpos = _character->_model->position();

	float xoff = charpos.x() - zone->_loc.x();
	float zoff = charpos.z() - zone->_loc.z();
	return sqrt(xoff * xoff + zoff * zoff) <= zone->_radius;
}

TeLayout *InGameScene::background() {
	return _bgGui.layout("background");
}

Billboard *InGameScene::billboard(const Common::String &name) {
	for (Billboard *billboard : _billboards) {
		if (billboard->model()->name() == name)
			return billboard;
	}
	return nullptr;
}

bool InGameScene::changeBackground(const Common::Path &name) {
	TetraedgeFSNode node = g_engine->getCore()->findFile(name);
	if (node.isReadable()) {
		_bgGui.spriteLayoutChecked("root")->load(name);
		if (g_engine->gameType() == TetraedgeEngine::kSyberia2)
			_bgGui.spriteLayoutChecked("root")->play();
		return true;
	}
	return false;
}

Character *InGameScene::character(const Common::String &name) {
	if (_character && _character->_model->name() == name)
		return _character;

	for (Character *c : _characters) {
		if (c->_model->name() == name)
			return c;
	}

	// WORKAROUND: Didn't find char, try again with case insensitive
	// for "OScar" typo in scenes/ValTrain/19000.
	for (Character *c : _characters) {
		if (c->_model->name().compareToIgnoreCase(name) == 0)
			return c;
	}

	return nullptr;
}

void InGameScene::close() {
	reset();
	_loadedPath = "";
	TeScene::close();
	freeGeometry();
	if (_character && _character->_model && !findKate()) {
		models().push_back(_character->_model);
		if (_character->_shadowModel[0]) {
			models().push_back(_character->_shadowModel[0]);
			models().push_back(_character->_shadowModel[1]);
		}
	}
	_objects.clear();
	for (TeFreeMoveZone *zone : _freeMoveZones)
		delete zone;
	_freeMoveZones.clear();
	_hitObjects.clear();
	for (TePickMesh2 *mesh : _clickMeshes)
		delete mesh;
	_clickMeshes.clear();
	_bezierCurves.clear();
	_dummies.clear();
	freeSceneObjects();
}

void InGameScene::convertPathToMesh(TeFreeMoveZone *zone) {
	TeIntrusivePtr<TeModel> model = new TeModel();
	model->meshes().clear();
	model->setMeshCount(1);
	model->setName("shadowReceiving");
	model->setPosition(zone->position());
	model->setRotation(zone->rotation());
	model->setScale(zone->scale());
	uint64 nverticies = zone->freeMoveZoneVerticies().size();
	TeMesh *mesh0 = model->meshes()[0].get();
	mesh0->setConf(nverticies, nverticies, TeMesh::MeshMode_Triangles, 0, 0);
	for (uint i = 0; i < nverticies; i++) {
		mesh0->setIndex(i, i);
		mesh0->setVertex(i, zone->freeMoveZoneVerticies()[i]);
		mesh0->setNormal(i, TeVector3f32(0, 0, 1));
	}
	_zoneModels.push_back(model);
}

TeIntrusivePtr<TeBezierCurve> InGameScene::curve(const Common::String &curveName) {
	for (TeIntrusivePtr<TeBezierCurve> &c : _bezierCurves) {
		if (c->name() == curveName)
			return c;
	}
	return TeIntrusivePtr<TeBezierCurve>();
}

void InGameScene::deleteAllCallback() {
	for (auto &pair : _callbacks) {
		for (auto *cb : pair._value) {
			delete cb;
		}
		pair._value.clear();
	}
	_callbacks.clear();
}

void InGameScene::deleteMarker(const Common::String &markerName) {
	if (!isMarker(markerName))
		return;

	for (uint i = 0; i < _markers.size(); i++) {
		if (_markers[i]._name == markerName) {
			_markers.remove_at(i);
			break;
		}
	}

	Game *game = g_engine->getGame();
	TeLayout *bg = game->forGui().layout("background");
	if (!bg)
		return;
	for (Te3DObject2 *child : bg->childList()) {
		if (child->name() == markerName) {
			bg->removeChild(child);
			break;
		}
	}
}

void InGameScene::deserializeCam(Common::ReadStream &stream, TeIntrusivePtr<TeCamera> &cam) {
	cam->setProjMatrixType(2);
	cam->viewport(0, 0, _viewportSize.getX(), _viewportSize.getY());
	// load name/position/rotation/scale
	Te3DObject2::deserialize(stream, *cam);
	cam->setFov(stream.readFloatLE());
	cam->setAspectRatio(stream.readFloatLE());
	// Original loads the second val then ignores it and sets 3000.
	cam->setOrthoPlanes(stream.readFloatLE(), 3000.0);
	stream.readFloatLE();
}

void InGameScene::deserializeModel(Common::ReadStream &stream, TeIntrusivePtr<TeModel> &model, TePickMesh2 *pickmesh) {
	TeVector3f32 vec;
	TeVector2f32 vec2;
	TeQuaternion rot;
	TeColor col;
	Common::SharedPtr<TeMesh> mesh(TeMesh::makeInstance());

	assert(pickmesh);

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

	if (indexcount > 100000 || vertexcount > 100000)
		error("InGameScene::deserializeModel: Unxpected counts %d %d", indexcount, vertexcount);

	mesh->setConf(vertexcount, indexcount, TeMesh::MeshMode_Triangles, 0, 0);
	for (uint i = 0; i < indexcount; i++)
		mesh->setIndex(i, stream.readUint32LE());

	for (uint i = 0; i < vertexcount; i++) {
		TeVector3f32::deserialize(stream, vec);
		mesh->setVertex(i, vec);
	}
	for (uint i = 0; i < vertexcount; i++) {
		TeVector3f32::deserialize(stream, vec);
		mesh->setNormal(i, vec);
	}
	for (uint i = 0; i < vertexcount; i++) {
		TeVector2f32::deserialize(stream, vec2);
		mesh->setTextureUV(i, vec2);
	}
	for (uint i = 0; i < vertexcount; i++) {
		col.deserialize(stream);
		mesh->setColor(i, col);
	}

	pickmesh->setNbTriangles(indexcount / 3);
	for (uint i = 0; i < indexcount; i++) {
		vec = mesh->vertex(mesh->index(i));
		pickmesh->verticies()[i] = vec;
	}
	model->addMesh(mesh);
}

void InGameScene::draw() {
	if (currentCameraIndex() >= (int)cameras().size())
		return;

	currentCamera()->apply();

	drawMask();
	drawReflection();

#ifdef TETRAEDGE_DEBUG_PATHFINDING
	if (_character && _character->curve()) {
		_character->curve()->setVisible(true);
		_character->curve()->draw();
	}

	for (TeFreeMoveZone *zone : _freeMoveZones) {
		zone->setVisible(true);
		zone->draw();
	}

	for (TePickMesh2 *mesh : _clickMeshes) {
		mesh->setVisible(true);
		mesh->draw();
	}
#endif

	g_engine->getRenderer()->updateGlobalLight();
	for (uint i = 0; i < _lights.size(); i++)
		_lights[i]->update(i);

	TeCamera::restore();

	drawKate();

	TeScene::draw();
}

void InGameScene::drawKate() {
	if (_rippleMasks.size())
		error("TODO: Implement InGameScene::drawKate");
}

void InGameScene::drawMask() {
	if (_masks.empty())
		return;

	TeIntrusivePtr<TeCamera> cam = currentCamera();
	if (!cam)
		return;

	cam->apply();

	TeRenderer *rend = g_engine->getRenderer();
	if (!_maskAlpha)
		rend->colorMask(false, false, false, false);

	for (auto &mask : _masks)
		mask->draw();

	if (!_maskAlpha)
		rend->colorMask(true, true, true, true);
}

void InGameScene::drawReflection() {
	if (_rippleMasks.empty() || currentCameraIndex() >= (int)cameras().size())
		return;

	currentCamera()->apply();
	if (!_maskAlpha)
		g_engine->getRenderer()->colorMask(false, false, false, false);

	for (uint i = _rippleMasks.size() - 1; i > 0; i--) {
		_rippleMasks[i]->draw();
	}

	if (!_maskAlpha)
		g_engine->getRenderer()->colorMask(true, true, true, true);
}

void InGameScene::drawPath() {
	if (currentCameraIndex() >= (int)cameras().size())
		return;

	currentCamera()->apply();
	g_engine->getRenderer()->disableZBuffer();

	for (uint i = 0; i < _freeMoveZones.size(); i++)
		_freeMoveZones[i]->draw();

	g_engine->getRenderer()->enableZBuffer();
}

InGameScene::Dummy InGameScene::dummy(const Common::String &name) {
	for (const Dummy &dummy : _dummies) {
		if (dummy._name == name)
			return dummy;
	}
	return Dummy();
}

bool InGameScene::findKate() {
	for (auto &m : models()) {
		if (m->name() == "Kate")
			return true;
	}
	return false;
}

const InGameScene::TeMarker *InGameScene::findMarker(const Common::String &name) {
	for (const TeMarker &marker : _markers) {
		if (marker._name == name)
			return &marker;
	}
	return nullptr;
}

const InGameScene::TeMarker *InGameScene::findMarkerByInt(const Common::String &val) {
	for (const TeMarker &marker : _markers) {
		if (marker._val == val)
			return &marker;
	}
	return nullptr;
}

InGameScene::SoundStep InGameScene::findSoundStep(const Common::String &name) {
	for (const auto &step : _soundSteps) {
		if (step._key == name)
			return step._value;
	}
	return SoundStep();
}

void InGameScene::freeGeometry() {
	_loadedPath.set("");
	_youkiManager.reset();
	freeSceneObjects();
	if (_character)
		_character->setFreeMoveZone(nullptr);
	for (Character *character : _characters)
		character->setFreeMoveZone(nullptr);
	for (TeFreeMoveZone *zone : _freeMoveZones)
		delete zone;
	_freeMoveZones.clear();
	_bezierCurves.clear();
	_dummies.clear();
	cameras().clear();
	models().clear();
	_zoneModels.clear();
	_masks.clear();
	_shadowReceivingObjects.clear();
	if (_charactersShadow)
		_charactersShadow->destroy();
	_sceneLights.clear();
	if (_charactersShadow) {
		delete _charactersShadow;
		_charactersShadow = nullptr;
	}
}

void InGameScene::freeSceneObjects() {
	if (_character) {
		_character->setCharLookingAt(nullptr);
		_character->deleteAllCallback();
	}
	if (_characters.size() == 1) {
		_characters[0]->deleteAllCallback();
	}

	SyberiaGame *game = dynamic_cast<SyberiaGame *>(g_engine->getGame());
	assert(game);
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

	// TODO: Clean up snows, waterCones, smokes, snowCones

	_particles.clear();
	TeParticle::deleteAll();

	deleteAllCallback();
	_markers.clear();

	// TODO: Clean up randomAnims

	for (RippleMask *rmask : _rippleMasks) {
		delete rmask;
	}
	_rippleMasks.clear();

	for (InGameScene::AnchorZone *zone : _anchorZones) {
		delete zone;
	}
	_anchorZones.clear();
}

float InGameScene::getHeadHorizontalRotation(Character *cter, const TeVector3f32 &vec) {
	TeVector3f32 pos = vec - cter->_model->position();
	TeVector3f32 zvec = TeVector3f32(0, 0, 1.0f);
	zvec.rotate(cter->_model->rotation());
	float angle = atan2f(-pos.x(), pos.z()) - atan2f(-zvec.x(), zvec.z());
	if (angle < -M_PI)
		angle += (float)(M_PI * 2);
	else if (angle > M_PI)
		angle -= (float)(M_PI * 2);
	return angle;
}

float InGameScene::getHeadVerticalRotation(Character *cter, const TeVector3f32 &vec) {
	TeVector3f32 modelPos = cter->_model->position();
	TeVector3f32 headWorldTrans = cter->_model->worldTransformationMatrix() * cter->lastHeadBoneTrans();
	modelPos.y() = headWorldTrans.y();
	TeVector3f32 offsetPos = vec - modelPos;
	currentCamera()->apply();
	float angle = atan2f(offsetPos.y(), TeVector2f32(offsetPos.x(), offsetPos.z()).length());
	return angle;
}

Common::Path InGameScene::imagePathMarker(const Common::String &name) {
	if (!isMarker(name))
		return Common::Path();
	Game *game = g_engine->getGame();
	TeLayout *bg = game->forGui().layoutChecked("background");
	for (Te3DObject2 *child : bg->childList()) {
		TeSpriteLayout *spritelayout = dynamic_cast<TeSpriteLayout *>(child);
		if (spritelayout && spritelayout->name() == name) {
			return spritelayout->_tiledSurfacePtr->loadedPath();
		}
	}
	return Common::Path();
}

void InGameScene::initScroll() {
	_scrollOffset = TeVector2f32(0.5f, 0.0f);
}

bool InGameScene::isMarker(const Common::String &name) {
	for (const TeMarker &marker : _markers) {
		if (marker._name == name)
			return true;
	}
	return false;
}

bool InGameScene::isObjectBlocking(const Common::String &name) {
	for (const Common::String &b: _blockingObjects) {
		if (name == b)
			return true;
	}
	return false;
}

TeVector2f32 InGameScene::layerSize() {
	TeLayout *bglayout = _bgGui.layout("background");
	TeVector3f32 sz;
	if (bglayout) {
		TeLayout *rootlayout = Game::findSpriteLayoutByName(bglayout, "root");
		if (!rootlayout)
			error("InGameScene::layerSize: No root layout inside the background");
		sz = rootlayout->size();
		_scrollScale = TeVector2f32(sz.x(), sz.y());
	} else {
		sz = g_engine->getApplication()->getMainWindow().size();
	}
	return TeVector2f32(sz.x(), sz.y());
}

bool InGameScene::load(const TetraedgeFSNode &sceneNode) {
	// Syberia 1 has loadActZones function contents inline.
	loadActZones();

	if (!_lights.empty()) {
		g_engine->getRenderer()->disableAllLights();
		for (uint i = 0; i < _lights.size(); i++) {
			_lights[i]->disable(i);
		}
		_lights.clear();
	}
	_shadowLightNo = -1;

	TeCore *core = g_engine->getCore();
	const TetraedgeFSNode lightsNode(core->findFile(getLightsFileName()));
	if (lightsNode.isReadable())
		loadLights(lightsNode);

	if (!sceneNode.exists())
		return false;

	close();
	_loadedPath = sceneNode.getPath();
	Common::ScopedPtr<Common::SeekableReadStream> scenefile(sceneNode.createReadStream());
	if (!scenefile)
		return false;

	uint32 ncameras = scenefile->readUint32LE();
	if (ncameras > 1024)
		error("Improbable number of cameras %d", ncameras);
	for (uint i = 0; i < ncameras; i++) {
		TeIntrusivePtr<TeCamera> cam = new TeCamera();
		deserializeCam(*scenefile, cam);
		cameras().push_back(cam);
	}

	uint32 nobjects = scenefile->readUint32LE();
	if (nobjects > 1024)
		error("Improbable number of objects %d", nobjects);
	for (uint i = 0; i < nobjects; i++) {
		TeIntrusivePtr<TeModel> model = new TeModel();
		const Common::String modelname = Te3DObject2::deserializeString(*scenefile);
		model->setName(modelname);
		const Common::String objname = Te3DObject2::deserializeString(*scenefile);
		TePickMesh2 *pickmesh = new TePickMesh2();
		deserializeModel(*scenefile, model, pickmesh);
		if (modelname.contains("Clic")) {
			//debug("Loaded clickMesh %s", modelname.c_str());
			_hitObjects.push_back(model);
			model->setVisible(false);
			model->setColor(TeColor(0, 0xff, 0, 0xff));
			models().push_back(model);
			pickmesh->setName(modelname);
			_clickMeshes.push_back(pickmesh);
		} else {
			delete pickmesh;
			if (modelname.substr(0, 2) != "ZB") {
				if (objname.empty()) {
					debug("[InGameScene::load] Unknown type of object named : %s", modelname.c_str());
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

	uint32 nfreemovezones = scenefile->readUint32LE();
	if (nfreemovezones > 1024)
		error("Improbable number of free move zones %d", nfreemovezones);
	for (uint i = 0; i < nfreemovezones; i++) {
		TeFreeMoveZone *zone = new TeFreeMoveZone();
		TeFreeMoveZone::deserialize(*scenefile, *zone, &_blockers, &_rectBlockers, &_actZones);
		_freeMoveZones.push_back(zone);
		zone->setVisible(false);
	}

	uint32 ncurves = scenefile->readUint32LE();
	if (ncurves > 1024)
		error("Improbable number of curves %d", ncurves);
	for (uint i = 0; i < ncurves; i++) {
		TeIntrusivePtr<TeBezierCurve> curve = new TeBezierCurve();
		TeBezierCurve::deserialize(*scenefile, *curve);
		curve->setVisible(true);
		_bezierCurves.push_back(curve);
	}

	uint32 ndummies = scenefile->readUint32LE();
	if (ndummies > 1024)
		error("Improbable number of dummies %d", ndummies);
	for (uint i = 0; i < ndummies; i++) {
		InGameScene::Dummy dummy;
		TeVector3f32 vec;
		TeQuaternion rot;
		dummy._name = Te3DObject2::deserializeString(*scenefile);
		TeVector3f32::deserialize(*scenefile, vec);
		dummy._position = vec;
		TeQuaternion::deserialize(*scenefile, rot);
		dummy._rotation = rot;
		TeVector3f32::deserialize(*scenefile, vec);
		dummy._scale = vec;
		_dummies.push_back(dummy);
	}

	for (TeFreeMoveZone *zone : _freeMoveZones) {
		convertPathToMesh(zone);
	}
	_charactersShadow = CharactersShadow::makeInstance();
	_charactersShadow->create(this);
	onMainWindowSizeChanged();

	return true;
}

static Common::Path _sceneFileNameBase(const Common::String &zone, const Common::String &scene) {
	Common::Path retval("scenes");
	retval.joinInPlace(zone).joinInPlace(scene);
	return retval;
}

static Common::Path _sceneFileNameBase() {
	const Game *game = g_engine->getGame();
	return _sceneFileNameBase(game->currentZone(), game->currentScene());
}

bool InGameScene::loadXml(const Common::String &zone, const Common::String &scene) {
	_maskAlpha = false;
	_zoneName = zone;
	_sceneName = scene;
	_blockers.clear();
	_rectBlockers.clear();
	TeFreeMoveZone::setCollisionSlide(false);
	loadBlockers();

	Common::Path xmlpath = _sceneFileNameBase(zone, scene).joinInPlace("Scene")
												.appendInPlace(scene).appendInPlace(".xml");
	TetraedgeFSNode node = g_engine->getCore()->findFile(xmlpath);
	InGameSceneXmlParser parser(this);
	parser.setAllowText();

	Common::String fixedbuf;
	if (g_engine->gameType() == TetraedgeEngine::kSyberia2 && scene == "GangcarVideo") {
		//
		// WORKAROUND: scenes/A1_RomHaut/GangcarVideo/SceneGangcarVideo.xml
		// in Syberia 2 has an embedded comment, which is invalid XML.
		// Patch the contents of the file before loading.
		//
		Common::ScopedPtr<Common::SeekableReadStream> xmlFile(node.createReadStream());
		if (!xmlFile)
			error("InGameScene::loadXml: Can't open %s", node.toString().c_str());
		const int64 bufsize = xmlFile->size();
		char *buf = new char[bufsize+1];
		buf[bufsize] = '\0';
		xmlFile->read(buf, bufsize);
		fixedbuf = Common::String(buf);
		delete [] buf;
		size_t offset = fixedbuf.find("<!-- <rippleMask");
		if (offset != Common::String::npos)
			fixedbuf.replace(offset, 4, "    ");  // replace the <! at the start
		offset = fixedbuf.find("texture=\"R11280-1-00.png\"/> -->");
		if (offset != Common::String::npos)
			fixedbuf.replace(offset + 29, 3, "   "); // replace the > at the end
		offset = fixedbuf.find("<!--<light ");
		if (offset != Common::String::npos)
			fixedbuf.replace(offset, 4, "    ");  // replace the <! at the start
		parser.loadBuffer((const byte *)fixedbuf.c_str(), bufsize);
	} else {
		// Regular loading.
		if (!node.loadXML(parser))
			error("InGameScene::loadXml: Can't load %s", node.toString().c_str());
	}

	if (!parser.parse())
		error("InGameScene::loadXml: Can't parse %s", node.toString().c_str());

	// loadFlamme and loadSnowCustom are handled by the above.

	_charactersShadow = CharactersShadow::makeInstance();
	_charactersShadow->create(this);

	for (uint i = 0; i < _lights.size(); i++)
		_lights[i]->disable(i);
	_lights.clear();
	_shadowLightNo = -1;

	TeCore *core = g_engine->getCore();
	const TetraedgeFSNode lightsNode(core->findFile(getLightsFileName()));
	if (lightsNode.isReadable())
		loadLights(lightsNode);

	Common::Path pxmlpath = _sceneFileNameBase(zone, scene).joinInPlace("particles.xml");
	TetraedgeFSNode pnode = g_engine->getCore()->findFile(pxmlpath);
	if (pnode.isReadable()) {
		ParticleXmlParser pparser;
		pparser._scene = this;
		if (!pnode.loadXML(pparser))
			error("InGameScene::loadXml: Can't load %s", pnode.toString().c_str());
		if (!pparser.parse())
			error("InGameScene::loadXml: Can't parse %s", pxmlpath.toString(Common::Path::kNativeSeparator).c_str());
	}

	TeMatrix4x4 camMatrix = currentCamera() ?
		currentCamera()->worldTransformationMatrix() : TeMatrix4x4();
	for (auto &particle : _particles) {
		particle->setMatrix(camMatrix);
		particle->realTimer().start();
		particle->update(particle->startLoop());
	}

	return true;
}

void InGameScene::loadActZones() {
	_actZones.clear();
	Common::File actzonefile;
	if (actzonefile.open(getActZoneFileName())) {
		if (Te3DObject2::loadAndCheckFourCC(actzonefile, "ACT0")) {
			uint32 count = actzonefile.readUint32LE();
			if (count > 1000000)
				error("Improbable number of actzones %d", count);
			_actZones.resize(count);
			for (uint i = 0; i < _actZones.size(); i++) {
				_actZones[i]._s1 = Te3DObject2::deserializeString(actzonefile);
				_actZones[i]._s2 = Te3DObject2::deserializeString(actzonefile);
				for (int j = 0; j < 4; j++)
					TeVector2f32::deserialize(actzonefile, _actZones[i]._points[j]);
				_actZones[i]._flag1 = (actzonefile.readByte() != 0);
				_actZones[i]._flag2 = true;
			}
		} else {
			warning("loadActZones: Incorrect header in %s", actzonefile.getName());
		}
	}
}

bool InGameScene::loadCamera(const Common::String &name) {
	Common::Path p = _sceneFileNameBase().joinInPlace(name).appendInPlace(".xml");
	TeCamera *cam = new TeCamera();
	cam->loadXml(p);
	// Original doesn't do this? but we seem to need it
	cam->setName(name);
	TeVector3f32 winSize = g_engine->getApplication()->getMainWindow().size();
	cam->viewport(0, 0, winSize.x(), winSize.y());
	cameras().push_back(TeIntrusivePtr<TeCamera>(cam));
	return true;
}

bool InGameScene::loadCharacter(const Common::String &name) {
	Character *c = character(name);
	if (!c) {
		c = new Character();
		if (!c->loadModel(name, false)) {
			delete c;
			return false;
		}
		models().push_back(c->_model);
		if (_character->_shadowModel[0]) {
			models().push_back(c->_shadowModel[0]);
			models().push_back(c->_shadowModel[1]);
		}
		_characters.push_back(c);
	}
	c->_model->setVisible(true);
	return true;
}

bool InGameScene::loadFreeMoveZone(const Common::String &name, TeVector2f32 &gridSize) {
	TeFreeMoveZone *zone = new TeFreeMoveZone();
	zone->setName(name);
	Common::Path p = _sceneFileNameBase().joinInPlace(name).appendInPlace(".bin");
	zone->loadBin(p, &_blockers, &_rectBlockers, &_actZones, gridSize);
	_freeMoveZones.push_back(zone);
	zone->setVisible(false);
	return true;
}

bool InGameScene::loadLights(const TetraedgeFSNode &node) {
	SceneLightsXmlParser parser(&_lights);

	if (!node.loadXML(parser))
		error("InGameScene::loadLights: Can't load %s", node.toString().c_str());
	if (!parser.parse())
		error("InGameScene::loadLights: Can't parse %s", node.toString().c_str());

	_shadowColor = parser.getShadowColor();
	_shadowLightNo = parser.getShadowLightNo();
	_shadowFarPlane = parser.getShadowFarPlane();
	_shadowNearPlane = parser.getShadowNearPlane();
	_shadowFov = parser.getShadowFov();

	g_engine->getRenderer()->enableAllLights();
	for (uint i = 0; i < _lights.size(); i++) {
		//
		// WORKAROUND: Some lights in Syberia 2 have 0 for all attenuation
		// values, which causes textures to all be black.  eg,
		// scenes/A2_Sommet/25210/lights.xml, light 0.
		// Correct them to have the default attenuation of 1, 0, 0.
		//
		_lights[i]->correctAttenuation();
		_lights[i]->enable(i);
	}

	if (_shadowLightNo >= (int)_lights.size()) {
		warning("Disabling scene shadows: invalid shadow light no.");
		_shadowLightNo = -1;
	}

#ifdef TETRAEDGE_DEBUG_LIGHTS
	debug("--- Scene lights ---");
	debug("Shadow: %s no:%d far:%.02f near:%.02f fov:%.02f", _shadowColor.dump().c_str(), _shadowLightNo, _shadowFarPlane, _shadowNearPlane, _shadowFov);
	debug("Global: %s", TeLight::globalAmbient().dump().c_str());
	for (uint i = 0; i < _lights.size(); i++) {
		debug("%s", _lights[i]->dump().c_str());
	}
	debug("---  end lights  ---");
#endif

	return true;
}

void InGameScene::loadMarkers(const TetraedgeFSNode &node) {
	_markerGui.load(node);
	TeLayout *bg = _bgGui.layoutChecked("background");
	TeSpriteLayout *root = Game::findSpriteLayoutByName(bg, "root");
	bg->setRatioMode(TeILayout::RATIO_MODE_NONE);
	root->addChild(bg);
}

bool InGameScene::loadObject(const Common::String &name) {
	Object3D *obj = object3D(name);
	if (!obj) {
		obj = new Object3D();
		if (!obj->loadModel(name)) {
			warning("InGameScene::loadObject: Loading %s failed", name.c_str());
			delete obj;
			return false;
		}
		models().push_back(obj->model());
		_object3Ds.push_back(obj);
	}
	obj->model()->setVisible(true);
	return true;
}

bool InGameScene::loadObjectMaterials(const Common::String &name) {
	TeImage img;
	bool retval = false;
	TeCore *core = g_engine->getCore();
	for (auto &obj : _objects) {
		// FIXME: This should probably only do something for the
		// object where the model name matches?  It won't find the file
		// anyway so it probably works as-is but it's a bit wrong.
		if (obj._name.empty())
			continue;

		Common::Path mpath = _loadedPath.join(name).join(obj._name + ".png");
		if (img.load(core->findFile(mpath))) {
			Te3DTexture *tex = Te3DTexture::makeInstance();
			tex->load(img);
			obj._model->meshes()[0]->defaultMaterial(tex);
			retval = true;
		}
	}
	return retval;
}

bool InGameScene::loadObjectMaterials(const Common::Path &path, const Common::String &name) {
	// Seems like this is never used?
	error("InGameScene::loadObjectMaterials(%s, %s)", path.toString(Common::Path::kNativeSeparator).c_str(), name.c_str());
}

bool InGameScene::loadPlayerCharacter(const Common::String &name) {
	if (_character == nullptr) {
		_character = new Character();
		if (!_character->loadModel(name, true)) {
			_playerCharacterModel.release();
			return false;
		}

		_playerCharacterModel = _character->_model;

		bool kateFound = findKate();

		if (g_engine->gameType() == TetraedgeEngine::kSyberia) {
			if (!kateFound) {
				models().push_back(_character->_model);
				if (_character->_shadowModel[0]) {
					models().push_back(_character->_shadowModel[0]);
					models().push_back(_character->_shadowModel[1]);
				}
			}
		} else {
			if (kateFound) {
				for (uint i = 0; i < models().size(); i++) {
					if (models()[i] == _character->_model) {
						models().remove_at(i);
						break;
					}
				}
			}
			models().push_back(_character->_model);
		}
	}

	_character->_model->setVisible(true);
	_character->setFreeMoveZone(nullptr);
	return true;
}

bool InGameScene::loadCurve(const Common::String &name) {
	TeCore *core = g_engine->getCore();
	TetraedgeFSNode node = core->findFile(_sceneFileNameBase().joinInPlace(name).appendInPlace(".bin"));
	if (!node.isReadable()) {
		warning("[InGameScene::loadCurve] Can't open file : %s.", node.toString().c_str());
		return false;
	}
	TeIntrusivePtr<TeBezierCurve> curve = new TeBezierCurve();
	curve->loadBin(node);
	_bezierCurves.push_back(curve);
	return true;
}

bool InGameScene::loadDynamicLightBloc(const Common::String &name, const Common::String &texture, const Common::String &zone, const Common::String &scene) {
	const Common::Path pdat = _sceneFileNameBase(zone, scene).joinInPlace(name).appendInPlace(".bin");
	const Common::Path ptex = _sceneFileNameBase(zone, scene).joinInPlace(texture);
	TetraedgeFSNode datNode = g_engine->getCore()->findFile(pdat);
	TetraedgeFSNode texNode = g_engine->getCore()->findFile(ptex);
	if (!datNode.isReadable()) {
		warning("[InGameScene::loadDynamicLightBloc] Can't open file : %s.", pdat.toString('/').c_str());
		return false;
	}

	Common::ScopedPtr<Common::SeekableReadStream> file(datNode.createReadStream());

	TeModel *model = new TeModel();
	model->setMeshCount(1);
	model->setName(datNode.getPath().baseName());

	// Read position/rotation/scale.
	model->deserialize(*file, *model);

	uint32 verts = file->readUint32LE();
	uint32 tricount = file->readUint32LE();
	if (verts > 100000 || tricount > 10000)
		error("Improbable number of verts (%d) or triangles (%d)", verts, tricount);

	TeMesh *mesh = model->meshes()[0].get();
	mesh->setConf(verts, tricount * 3, TeMesh::MeshMode_Triangles, 0, 0);

	for (uint i = 0; i < verts; i++) {
		TeVector3f32 vec;
		TeVector3f32::deserialize(*file, vec);
		mesh->setVertex(i, vec);
		mesh->setNormal(i, TeVector3f32(0, 0, 1));
	}
	for (uint i = 0; i < verts; i++) {
		TeVector2f32 vec2;
		TeVector2f32::deserialize(*file, vec2);
		vec2.setY(1.0 - vec2.getY());
		mesh->setTextureUV(i, vec2);
	}

	for (uint i = 0; i < tricount * 3; i++)
		mesh->setIndex(i, file->readUint16LE());

	file.reset();

	if (texNode.exists()) {
		TeIntrusivePtr<Te3DTexture> tex = Te3DTexture::makeInstance();
		tex->load2(texNode, false);
		mesh->defaultMaterial(tex);
	} else if (texture.size()) {
		warning("loadDynamicLightBloc: Failed to load texture %s", texture.c_str());
	}

	model->setVisible(false);

	_zoneModels.push_back(TeIntrusivePtr<TeModel>(model));
	return true;
}

bool InGameScene::loadLight(const Common::String &name, const Common::String &zone, const Common::String &scene) {
	Common::Path datpath = _sceneFileNameBase(zone, scene).joinInPlace(name).appendInPlace(".bin");
	TetraedgeFSNode datnode = g_engine->getCore()->findFile(datpath);
	if (!datnode.isReadable()) {
		warning("[InGameScene::loadLight] Can't open file : %s.", datpath.toString(Common::Path::kNativeSeparator).c_str());
		return false;
	}

	Common::ScopedPtr<Common::SeekableReadStream> file(datnode.createReadStream());
	SceneLight light;
	light._name = name;
	TeVector3f32::deserialize(*file, light._v1);
	TeVector3f32::deserialize(*file, light._v2);
	light._color.deserialize(*file);
	light._f = file->readFloatLE();

	_sceneLights.push_back(light);
	return true;
}

bool InGameScene::loadMask(const Common::String &name, const Common::String &texture, const Common::String &zone, const Common::String &scene) {
	TeCore *core = g_engine->getCore();
	TetraedgeFSNode texnode = core->findFile(_sceneFileNameBase(zone, scene).joinInPlace(texture));
	TetraedgeFSNode datnode = core->findFile(_sceneFileNameBase(zone, scene).joinInPlace(name).appendInPlace(".bin"));
	if (!datnode.isReadable()) {
		warning("[InGameScene::loadMask] Can't open file : %s.", datnode.toString().c_str());
		return false;
	}
	TeModel *model = new TeModel();
	model->setMeshCount(1);
	model->setName(name);

	Common::ScopedPtr<Common::SeekableReadStream> file(datnode.createReadStream());

	// Load position, rotation, size.
	Te3DObject2::deserialize(*file, *model, false);

	uint32 verts = file->readUint32LE();
	uint32 tricount = file->readUint32LE();
	if (verts > 100000 || tricount > 10000)
		error("Improbable number of verts (%d) or triangles (%d)", verts, tricount);

	TeMesh *mesh = model->meshes()[0].get();
	mesh->setConf(verts, tricount * 3, TeMesh::MeshMode_Triangles, 0, 0);

	for (uint i = 0; i < verts; i++) {
		TeVector3f32 vec;
		TeVector3f32::deserialize(*file, vec);
		mesh->setVertex(i, vec);
		mesh->setNormal(i, TeVector3f32(0, 0, 1));
		if (_maskAlpha) {
			mesh->setColor(i, TeColor(255, 255, 255, 128));
		}
	}

	for (uint i = 0; i < verts; i++) {
		TeVector2f32 vec2;
		TeVector2f32::deserialize(*file, vec2);
		vec2.setY(1.0 - vec2.getY());
		mesh->setTextureUV(i, vec2);
	}

	// For some reason this one has the indexes in reverse order :(
	for (uint i = 0; i < tricount * 3; i += 3) {
		mesh->setIndex(i + 2, file->readUint16LE());
		mesh->setIndex(i + 1, file->readUint16LE());
		mesh->setIndex(i, file->readUint16LE());
	}

	file.reset();
	TeIntrusivePtr<Te3DTexture> tex = Te3DTexture::load2(texnode, !_maskAlpha);

	if (tex) {
		mesh->defaultMaterial(tex);
		if (!_maskAlpha) {
			mesh->materials()[0]._mode = TeMaterial::MaterialMode2;
		}

		_masks.push_back(model);
		return true;
	} else {
		warning("Failed to load mask texture %s", texture.c_str());
		return false;
	}
}

bool InGameScene::loadRBB(const Common::String &fname, const Common::String &zone, const Common::String &scene) {
	warning("TODO: Implement InGameScene::loadRBB");
	return true;
}

bool InGameScene::loadRippleMask(const Common::String &name, const Common::String &texture, const Common::String &zone, const Common::String &scene) {
	warning("TODO: Implement InGameScene::loadRippleMask");
	return true;
}

bool InGameScene::loadRObject(const Common::String &fname, const Common::String &zone, const Common::String &scene) {
	warning("TODO: Implement InGameScene::loadRObject");
	return true;
}

bool InGameScene::loadShadowMask(const Common::String &name, const Common::String &texture, const Common::String &zone, const Common::String &scene) {
	warning("TODO: Implement InGameScene::loadShadowMask");
	return true;
}

bool InGameScene::loadShadowReceivingObject(const Common::String &name, const Common::String &zone, const Common::String &scene) {
	TetraedgeFSNode datnode = g_engine->getCore()->findFile(_sceneFileNameBase(zone, scene).joinInPlace(name).appendInPlace(".bin"));
	if (!datnode.isReadable()) {
		warning("[InGameScene::loadShadowReceivingObject] Can't open file : %s.", datnode.toString().c_str());
		return false;
	}
	TeModel *model = new TeModel();
	model->setMeshCount(1);
	model->setName(name);

	Common::ScopedPtr<Common::SeekableReadStream> file(datnode.createReadStream());

	// Load position, rotation, size.
	Te3DObject2::deserialize(*file, *model, false);

	uint32 verts = file->readUint32LE();
	uint32 tricount = file->readUint32LE();
	if (verts > 100000 || tricount > 10000)
		error("Improbable number of verts (%d) or triangles (%d)", verts, tricount);

	TeMesh *mesh = model->meshes()[0].get();
	mesh->setConf(verts, tricount * 3, TeMesh::MeshMode_Triangles, 0, 0);

	for (uint i = 0; i < verts; i++) {
		TeVector3f32 vec;
		TeVector3f32::deserialize(*file, vec);
		mesh->setVertex(i, vec);
		mesh->setNormal(i, TeVector3f32(0, 0, 1));
	}

	// Indexes in reverse order :(
	for (uint i = 0; i < tricount * 3; i += 3) {
		mesh->setIndex(i + 2, file->readUint16LE());
		mesh->setIndex(i + 1, file->readUint16LE());
		mesh->setIndex(i, file->readUint16LE());
	}

	file.reset();

	_shadowReceivingObjects.push_back(model);
	return true;
}

bool InGameScene::loadZBufferObject(const Common::String &name, const Common::String &zone, const Common::String &scene) {
	TetraedgeFSNode datnode = g_engine->getCore()->findFile(_sceneFileNameBase(zone, scene).joinInPlace(name).appendInPlace(".bin"));
	if (!datnode.isReadable()) {
		warning("[InGameScene::loadZBufferObject] Can't open file : %s.", datnode.toString().c_str());
		return false;
	}
	TeModel *model = new TeModel();
	model->setMeshCount(1);
	model->setName(name);

	Common::ScopedPtr<Common::SeekableReadStream> file(datnode.createReadStream());

	// Load position, rotation, size.
	Te3DObject2::deserialize(*file, *model, false);

	uint32 verts = file->readUint32LE();
	uint32 tricount = file->readUint32LE();
	if (verts > 100000 || tricount > 10000)
		error("Improbable number of verts (%d) or triangles (%d)", verts, tricount);

	TeMesh *mesh = model->meshes()[0].get();
	mesh->setConf(verts, tricount * 3, TeMesh::MeshMode_Triangles, 0, 0);

	for (uint i = 0; i < verts; i++) {
		TeVector3f32 vec;
		TeVector3f32::deserialize(*file, vec);
		mesh->setVertex(i, vec);
		mesh->setNormal(i, TeVector3f32(0, 0, 1));
		mesh->setColor(i, TeColor(128, 0, 255, 128));
	}

	for (uint i = 0; i < tricount * 3; i++) {
		mesh->setIndex(i, file->readUint16LE());
	}

	_zoneModels.push_back(model);
	return true;
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
		warning("Couldn't open blockers file %s.", blockersPath.toString(Common::Path::kNativeSeparator).c_str());
		return;
	}

	bool hasHeader = Te3DObject2::loadAndCheckFourCC(blockersfile, "BLK0");
	if (!hasHeader)
		blockersfile.seek(0);

	uint32 nblockers = blockersfile.readUint32LE();
	if (nblockers > 1024)
		error("Improbable number of blockers %d", nblockers);
	_blockers.resize(nblockers);
	for (uint i = 0; i < nblockers; i++) {
		_blockers[i]._s = Te3DObject2::deserializeString(blockersfile);
		TeVector2f32::deserialize(blockersfile, _blockers[i]._pts[0]);
		TeVector2f32::deserialize(blockersfile, _blockers[i]._pts[1]);
		_blockers[i]._enabled = true;
	}

	if (hasHeader) {
		uint32 nrectblockers = blockersfile.readUint32LE();
		if (nrectblockers > 1024)
			error("Improbable number of rectblockers %d", nrectblockers);
		_rectBlockers.resize(nrectblockers);
		for (uint i = 0; i < nrectblockers; i++) {
			_rectBlockers[i]._s = Te3DObject2::deserializeString(blockersfile);
			for (uint j = 0; j < 4l; j++) {
				TeVector2f32::deserialize(blockersfile, _rectBlockers[i]._pts[j]);
			}
			_rectBlockers[i]._enabled = true;
		}
	}
}

void InGameScene::loadBackground(const TetraedgeFSNode &node) {
	_youkiManager.reset();
	_bgGui.load(node);
	TeLayout *bg = _bgGui.layout("background");
	TeLayout *root = _bgGui.layout("root");
	bg->setRatioMode(TeILayout::RATIO_MODE_NONE);
	root->setRatioMode(TeILayout::RATIO_MODE_NONE);
	TeCamera *wincam = g_engine->getApplication()->mainWindowCamera();
	bg->disableAutoZ();
	bg->setZPosition(wincam->orthoNearPlane());

	for (const auto &layoutEntry : _bgGui.spriteLayouts()) {
		AnimObject *animobj = new AnimObject();
		animobj->_name = layoutEntry._key;
		animobj->_layout = layoutEntry._value;
		animobj->_layout->_tiledSurfacePtr->_frameAnim.onFinished().add(animobj, &AnimObject::onFinished);
		if (animobj->_name != "root")
			animobj->_layout->setVisible(false);
		_animObjects.push_back(animobj);
	}
}

bool InGameScene::loadBillboard(const Common::String &name) {
	Billboard *b = billboard(name);
	if (b)
		return true;
	b = new Billboard();
	if (b->load(Common::Path(name))) {
		_billboards.push_back(b);
		return true;
	} else {
		delete b;
		return false;
	}
}

void InGameScene::loadInteractions(const TetraedgeFSNode &node) {
	_hitObjectGui.load(node);
	TeLayout *bgbackground = _bgGui.layoutChecked("background");
	Game *game = g_engine->getGame();
	TeSpriteLayout *root = game->findSpriteLayoutByName(bgbackground, "root");
	TeLayout *background = _hitObjectGui.layoutChecked("background");
	for (auto *child : background->childList()) {
		TeButtonLayout *btn = dynamic_cast<TeButtonLayout *>(child);
		if (btn)
			btn->setDoubleValidationProtectionEnabled(false);
	}
	background->setRatioMode(TeILayout::RATIO_MODE_NONE);
	root->addChild(background);
}

void InGameScene::moveCharacterTo(const Common::String &charName, const Common::String &curveName, float curveOffset, float curveEnd) {
	Character *c = character(charName);
	if (c != nullptr && c != _character) {
		SyberiaGame *game = dynamic_cast<SyberiaGame *>(g_engine->getGame());
		assert(game);
		if (!game->_movePlayerCharacterDisabled) {
			c->setCurveStartLocation(c->characterSettings()._cutSceneCurveDemiPosition);
			TeIntrusivePtr<TeBezierCurve> crve = curve(curveName);
			if (!curveName.empty() && !crve)
				warning("moveCharacterTo: curve %s not found", curveName.c_str());
			c->placeOnCurve(crve);
			c->setCurveOffset(curveOffset);
			const Common::String walkStartAnim = c->walkAnim(Character::WalkPart_Start);
			if (walkStartAnim.empty()) {
				c->setAnimation(c->walkAnim(Character::WalkPart_Loop), true);
			} else {
				c->setAnimation(c->walkAnim(Character::WalkPart_Start), false);
			}
			c->walkTo(curveEnd, false);
		}
	}
}

void InGameScene::onMainWindowSizeChanged() {
	TeVector3f32 winSize = g_engine->getApplication()->getMainWindow().size();
	_viewportSize = TeVector2f32(winSize.x(), winSize.y());
	Common::Array<TeIntrusivePtr<TeCamera>> &cams = cameras();
	for (uint i = 0; i < cams.size(); i++) {
		cams[i]->viewport(0, 0, _viewportSize.getX(), _viewportSize.getY());
	}
}

Object3D *InGameScene::object3D(const Common::String &name) {
	for (Object3D *obj : _object3Ds) {
		if (obj->model()->name() == name)
			return obj;
	}
	return nullptr;
}

TeFreeMoveZone *InGameScene::pathZone(const Common::String &name) {
	for (TeFreeMoveZone *zone: _freeMoveZones) {
		if (zone->name() == name)
			return zone;
	}
	return nullptr;
}

void InGameScene::playVerticalScrolling(float time) {
	_verticalScrollTimer.start();
	_verticalScrollTimer.stop();
	_verticalScrollTimer.start();
	_verticalScrollTime = time * 1000000.0f;
	_verticalScrollPlaying = true;
}

void InGameScene::reset() {
	for (auto *character : _characters)
		character->setFreeMoveZone(nullptr);
	_youkiManager.reset();
	if (_character)
		_character->setFreeMoveZone(nullptr);
	freeSceneObjects();
	_bgGui.unload();
	unloadSpriteLayouts();
	_markerGui.unload();
	_hitObjectGui.unload();
}

TeLight *InGameScene::shadowLight() {
	if (_shadowLightNo == -1 || (uint)_shadowLightNo >= _lights.size()) {
		return nullptr;
	}
	return _lights[_shadowLightNo].get();
}

void InGameScene::setImagePathMarker(const Common::String &markerName, const Common::Path &path) {
	if (!isMarker(markerName))
		return;

	Game *game = g_engine->getGame();
	TeLayout *bg = game->forGui().layoutChecked("background");

	for (Te3DObject2 *child : bg->childList()) {
		if (child->name() == markerName) {
			TeSpriteLayout *sprite = dynamic_cast<TeSpriteLayout *>(child);
			if (sprite) {
				sprite->load(path);
				sprite->_tiledSurfacePtr->_frameAnim.setLoopCount(-1);
				sprite->play();
			}
		}
	}
}

void InGameScene::setPositionCharacter(const Common::String &charName, const Common::String &freeMoveZoneName, const TeVector3f32 &position) {
	Character *c = character(charName);
	if (!c) {
		warning("[SetCharacterPosition] Character not found %s", charName.c_str());
	} else if (c == _character && c->positionFlag()) {
		c->setFreeMoveZoneName(freeMoveZoneName);
		c->setPositionCharacter(position);
		c->setPositionFlag(false);
		c->setNeedsSomeUpdate(true);
	} else {
		c->stop();
		TeFreeMoveZone *zone = pathZone(freeMoveZoneName);
		if (!zone) {
			warning("[SetCharacterPosition] PathZone not found %s", freeMoveZoneName.c_str());
			for (TeFreeMoveZone *z : _freeMoveZones)
				warning("zone: %s", z->name().c_str());
			return;
		}
		TeIntrusivePtr<TeCamera> cam = currentCamera();
		zone->setCamera(cam, false);
		c->setFreeMoveZone(zone);
		SoundStep step = findSoundStep(freeMoveZoneName);
		c->setStepSound(step._stepSound1, step._stepSound2);
		bool correctFlag = true;
		const TeVector3f32 corrected = zone->correctCharacterPosition(position, &correctFlag, true);
		c->_model->setPosition(corrected);
		if (!correctFlag)
			warning("[SetCharacterPosition] Warning : The character is not above the ground %s", charName.c_str());
	}
}

void InGameScene::setStep(const Common::String &scene, const Common::String &step1, const Common::String &step2) {
	SoundStep ss;
	ss._stepSound1 = step1;
	ss._stepSound2 = step2;
	_soundSteps[scene] = ss;
}

void InGameScene::setVisibleMarker(const Common::String &markerName, bool val) {
	if (!isMarker(markerName))
		return;

	Game *game = g_engine->getGame();
	TeLayout *bg = game->forGui().layout("background");
	if (!bg)
		return;

	for (Te3DObject2 *child : bg->childList()) {
		if (child->name() == markerName) {
			child->setVisible(val);
			break;
		}
	}
}

void InGameScene::unloadCharacter(const Common::String &name) {
	if (_character && _character->_model->name() == name) {
		_character->removeAnim();
		_character->deleteAnim();
		_character->deleteAllCallback();
		if (_character->_model->anim())
			_character->_model->anim()->stop(); // TODO: added this
		_character->setFreeMoveZone(nullptr); // TODO: added this
		_character->deleteLater();
		_character = nullptr;
	}

	// NOTE: There may be multiple characters with the same
	// model to delete here.
	for (uint i = 0; i < _characters.size(); i++) {
		Character *c = _characters[i];
		if (c && c->_model->name() == name) {
			c->removeAnim();
			c->deleteAnim();
			c->deleteAllCallback();
			c->deleteLater();
			if (c->_model->anim())
				c->_model->anim()->stop(); // TODO: added this
			c->setFreeMoveZone(nullptr); // TODO: added this
			_characters.remove_at(i);
			i--;
		}
	}
}

void InGameScene::unloadObject(const Common::String &name) {
	for (uint i = 0; i < _object3Ds.size(); i++) {
		if (_object3Ds[i]->model()->name() == name) {
			// Remove from the scene models.
			for (uint j = 0; j < models().size(); j++) {
				if (models()[j] == _object3Ds[i]->model())	{
					models().remove_at(j);
					break;
				}
			}
			_object3Ds[i]->deleteLater();
			_object3Ds.remove_at(i);
			break;
		}
	}
}

void InGameScene::unloadSpriteLayouts() {
	for (auto *animobj : _animObjects) {
		delete animobj;
	}
	_animObjects.clear();
}

void InGameScene::update() {
	SyberiaGame *game = dynamic_cast<SyberiaGame *>(g_engine->getGame());
	assert(game);
	if (_bgGui.loaded()) {
		_bgGui.layoutChecked("background")->setZPosition(0.0f);
	}
	if (_character) {
		_character->setHasAnchor(false);
		for (AnchorZone *zone : _anchorZones) {
			if (aroundAnchorZone(zone)) {
				TeVector2f32 headRot(getHeadHorizontalRotation(_character, zone->_loc),
					getHeadVerticalRotation(_character, zone->_loc));
				if (fabs(headRot.getX() * 180.0 / M_PI) > 90.0 || fabs(headRot.getY() * 180.0 / M_PI) > 45.0) {
					_character->setHasAnchor(false);
					_character->setLastHeadRotation(_character->headRotation());
				} else {
					_character->setHeadRotation(headRot);
					_character->setHasAnchor(true);
				}
			}
		}
		if (_character->charLookingAt()) {
			Character *targetc = _character->charLookingAt();
			TeVector3f32 targetpos;
			if (g_engine->gameType() == TetraedgeEngine::kSyberia)
				targetpos = targetc->_model->position();
			else
				targetpos = targetc->_model->worldTransformationMatrix() * targetc->lastHeadBoneTrans();

			//
			// Note: The below general code for NPCs is different in Syberia 2,
			// and uses c->charLookingAtOffset(), but the player look-at code
			// is the same in both games and always adds 17 for "tall" characters.
			//

			if (_character->lookingAtTallThing())
				targetpos.y() += 17;
			TeVector2f32 headRot(getHeadHorizontalRotation(_character, targetpos),
					getHeadVerticalRotation(_character, targetpos));
			float hangle = headRot.getX() * 180.0f / M_PI;
			if (hangle > 90.0f)
				headRot.setX((float)M_PI_2);
			else if (hangle < -90.0f)
				headRot.setX((float)-M_PI_2);
			_character->setHeadRotation(headRot);
			_character->setHasAnchor(true);
		}
	}
	for (Character *c : _characters) {
		Character *targetc = c->charLookingAt();
		if (targetc) {
			TeVector3f32 targetpos;
			if (g_engine->gameType() == TetraedgeEngine::kSyberia) {
				targetpos = targetc->_model->position();
				if (c->lookingAtTallThing())
					targetpos.y() += 17;
			} else {
				targetpos = targetc->_model->worldTransformationMatrix() * targetc->lastHeadBoneTrans();
			}
			TeVector2f32 headRot(getHeadHorizontalRotation(c, targetpos),
					getHeadVerticalRotation(c, targetpos));
			float hangle = headRot.getX() * 180.0f / M_PI;
			if (hangle > 90)
				headRot.setX((float)M_PI_2);
			else if (hangle < -90)
				headRot.setX((float)-M_PI_2);

			if (g_engine->gameType() == TetraedgeEngine::kSyberia2) {
				if (c->lookingAtTallThing())
					headRot.setY(c->charLookingAtOffset());
			}

			c->setHeadRotation(headRot);
			c->setHasAnchor(true);
		}
	}

	TeLuaGUI::StringMap<TeSpriteLayout *> &sprites = bgGui().spriteLayouts();
	for (auto &sprite : sprites) {
		if (_callbacks.contains(sprite._key)) {
			error("TODO: handle sprite callback in InGameScene::update");
		}
	}

	TeScene::update();
	_youkiManager.update();

	uint64 waitTime = _waitTimeTimer.timeFromLastTimeElapsed();
	if (_waitTime != -1.0 && waitTime > _waitTime) {
		_waitTime = -1.0;
		_waitTimeTimer.stop();
		bool resumed = false;
		for (uint i = 0; i < game->yieldedCallbacks().size(); i++) {
			SyberiaGame::YieldedCallback &yc = game->yieldedCallbacks()[i];
			if (yc._luaFnName == "OnWaitFinished") {
				TeLuaThread *thread = yc._luaThread;
				game->yieldedCallbacks().remove_at(i);
				thread->resume();
				resumed = true;
			}
		}
		if (!resumed)
			game->luaScript().execute("OnWaitFinished");
	}

	// TODO: Update Flammes

	// Original does this, but snowCustoms are never actually created?
	//for (auto snow : _snowCustoms)
	//	snow->addFlake();

	TeParticle::updateAll(1);

	for (Object3D *obj : _object3Ds) {
		if (obj->_translateTime >= 0) {
			float time = MIN((float)(obj->_translateTimer.getTimeFromStart() / 1000000.0), obj->_translateTime);
			TeVector3f32 trans = obj->_translateStart + (obj->_translateAmount * (time / obj->_translateTime));
			obj->model()->setPosition(trans);
		}
		if (obj->_rotateTime >= 0) {
			float time = MIN((float)(obj->_rotateTimer.getTimeFromStart() / 1000000.0), obj->_rotateTime);
			TeVector3f32 rot = (obj->_rotateAmount * (time / obj->_rotateTime));
			TeQuaternion rotq = TeQuaternion::fromEulerDegrees(rot);
			obj->model()->setRotation(obj->_rotateStart * rotq);
		}
	}
}

void InGameScene::updateScroll() {
	TeLayout *bg = _bgGui.layout("background");
	if (!bg)
		return;

	TeSpriteLayout *root = Game::findSpriteLayoutByName(bg, "root");
	if (!root)
		error("No root layout in the background");
	_scrollOffset = TeVector2f32();
	TeIntrusivePtr<TeTiledTexture> rootTex = root->_tiledSurfacePtr->tiledTexture();
	// During startup root texture is not yet loaded
	if (!rootTex)
		return;
	const TeVector2s32 texSize = rootTex->totalSize();
	if (texSize._x < 801) {
		if (texSize._y < 601) {
			_scrollOffset = TeVector2f32(0.5f, 0.0f);
			updateViewport(0);
		} else {
			TeVector3f32 usersz = bg->userSize();
			usersz.y() = 2.333333f;
			bg->setSize(usersz);
			//TeVector2f32 boundLayerSz = boundLayerSize();
			layerSize();

			float y1 = 300.0f / texSize._y;
			float y2 = (texSize._y - 300.0f) / texSize._y;

			if (_verticalScrollPlaying) {
				float elapsed = _verticalScrollTimer.timeFromLastTimeElapsed();
				_scrollOffset.setY(elapsed * (y2 - y1) / _verticalScrollTime + y1);
			} else if (_character && _character->_model) {
				TeIntrusivePtr<TeCamera> cam = currentCamera();
				const TeMatrix4x4 camProjMatrix = cam->projectionMatrix();
				TeMatrix4x4 camWorldMatrix = cam->worldTransformationMatrix();
				camWorldMatrix.inverse();
				const TeMatrix4x4 camProjWorld = camProjMatrix * camWorldMatrix;
				TeVector3f32 charPos = camProjWorld * _character->_model->position();
				_scrollOffset.setY(1.0f - (charPos.y() + 1.0f));
			}
			_scrollOffset.setX(0.5f);
			_scrollOffset.setY(CLIP(_scrollOffset.getY(), y1, y2));
			if (_scrollOffset.getY() >= y2 && _verticalScrollPlaying) {
				_verticalScrollTimer.stop();
				_verticalScrollPlaying = false;
			}
			root->setAnchor(TeVector3f32(0.5f, _scrollOffset.getY(), 0.5f));
			updateViewport(2);
		}
	} else {
		TeVector3f32 usersz = bg->userSize();
		usersz.x() = texSize._x / 800.0f;
		bg->setSize(usersz);
		//TeVector2f32 boundLayerSz = boundLayerSize();
		TeVector2f32 layerSz = layerSize();
		float x1, x2;
		if (g_engine->getApplication()->ratioStretched()) {
			x1 = layerSz.getX() * 2;
			const TeVector3f32 winSize = g_engine->getApplication()->getMainWindow().size();
			x2 = winSize.x();
		} else {
			x1 = layerSz.getX() * 2;
			x2 = layerSz.getX();
		}
		TeIntrusivePtr<TeCamera> cam = currentCamera();
		if (cam) {
			const TeMatrix4x4 camProjMatrix = cam->projectionMatrix();
			TeMatrix4x4 camWorldMatrix = cam->worldTransformationMatrix();
			camWorldMatrix.inverse();
			const TeMatrix4x4 camProjWorld = camProjMatrix * camWorldMatrix;
			const TeVector3f32 charPos = camProjWorld * _character->_model->position();
			_scrollOffset.setX((charPos.x() + 1.0f) / 2.0f);
			float xmin = x2 / x1;
			float xmax = 1.0f - (x2 / x1);
			_scrollOffset.setX(CLIP(_scrollOffset.getX(), xmin, xmax));
			root->setAnchor(TeVector3f32(_scrollOffset.getX(), 0.5f, 0.5f));
			TeLayout *forbg = g_engine->getGame()->forGui().layoutChecked("background");
			forbg->setAnchor(TeVector3f32(_scrollOffset.getX(), 0.5f, 0.5f));
			updateViewport(1);
			// _globalScrollingType = 1;  // This gets set but never used?
		}
	}
}

void InGameScene::updateViewport(int ival) {
	const TeVector2f32 lsize = layerSize();
	const TeVector2f32 offset((0.5f - _scrollOffset.getX()) * _scrollScale.getX(),
							_scrollOffset.getY() * _scrollScale.getY());
	const TeVector3f32 winSize = g_engine->getApplication()->getMainWindow().size();
	int x = (winSize.x() - lsize.getX()) / 2.0f + offset.getX();
	int y = (winSize.y() - lsize.getY()) / 2.0f;
	for (auto &cam : cameras()) {
		float aspectRatio = lsize.getX() / lsize.getY();
		//cam->setSomething(ival);
		cam->viewport(x, y, lsize.getX(), lsize.getY());
		if (g_engine->getApplication()->ratioStretched()) {
			aspectRatio = (aspectRatio / (winSize.x() / winSize.y())) * 1.333333f;
		}
		cam->setAspectRatio(aspectRatio);
	}
}

void InGameScene::activateMask(const Common::String &name, bool val) {
	for (auto &mask : _masks) {
		if (mask->name() == name) {
			mask->setVisible(val);
			return;
		}
	}
	warning("activateMask: Didn't find mask %s", name.c_str());
}

bool InGameScene::AnimObject::onFinished() {
	SyberiaGame *game = dynamic_cast<SyberiaGame *>(g_engine->getGame());
	assert(game);
	for (uint i = 0; i < game->yieldedCallbacks().size(); i++) {
		SyberiaGame::YieldedCallback &yc = game->yieldedCallbacks()[i];
		if (yc._luaFnName == "OnFinishedAnim" && yc._luaParam == _name) {
			TeLuaThread *thread = yc._luaThread;
			game->yieldedCallbacks().remove_at(i);
			if (thread) {
				thread->resume();
				return false;
			}
			break;
		}
	}
	game->luaScript().execute("OnFinishedAnim", _name);
	return false;
}

void InGameScene::Flamme::initFire() {
	_needsFires = true;
	_fires.resize(MAX_FIRE);
}

InGameScene::Flamme::~Flamme() {
	for (auto fire : _fires) {
		if (fire) {
			delete fire;
		}
	}
	_fires.clear();
}

} // end namespace Tetraedge
