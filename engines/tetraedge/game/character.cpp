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

#include "common/hashmap.h"
#include "common/path.h"
#include "common/file.h"
#include "common/debug.h"
#include "common/util.h"

#include "tetraedge/tetraedge.h"
#include "tetraedge/game/character.h"
#include "tetraedge/game/application.h"
#include "tetraedge/game/game.h"
#include "tetraedge/game/character_settings_xml_parser.h"
#include "tetraedge/te/te_model_animation.h"
#include "tetraedge/te/te_core.h"

namespace Tetraedge {

/*static*/ Common::HashMap<Common::String, Character::CharacterSettings> *Character::_globalCharacterSettings = nullptr;
/*static*/ Common::HashMap<Common::String, TeIntrusivePtr<TeModelAnimation>> *Character::_animCacheMap = nullptr;

// /*static*/ Common::Array<Character::AnimCacheElement> *Character::_animCache = nullptr;
// /*static*/ uint Character::_animCacheSize = 0;

void Character::CharacterSettings::clear() {
	_name.clear();
	_modelFileName.clear();
	_defaultScale = TeVector3f32();
	_idleAnimFileName.clear();
	_walkSettings.clear();
	_walkSpeed = 0.0f;
	_cutSceneCurveDemiPosition = TeVector3f32();
	_defaultEyes.clear();
	_defaultMouth.clear();
	_defaultBody.clear();
	_invertNormals = false;
}

void Character::WalkSettings::clear() {
	for (int i = 0; i < 4; i++) {
		_walkParts[i] = AnimSettings();
	}
}

Character::Character() : _walkCurveStart(0), _lastFrame(-1), _callbacksChanged(false),
_notWalkAnim(false), _returnToIdleAnim(false), _walkModeStr("Walk"),
_needsSomeUpdate(false), _positionFlag(false), _lookingAtTallThing(false),
_stepSound1("sounds/SFX/PAS_H_BOIS1.ogg"), _stepSound2("sounds/SFX/PAS_H_BOIS2.ogg"),
_freeMoveZone(nullptr), _animSoundOffset(0), _lastAnimFrame(0), _charLookingAt(nullptr),
_recallageY(true), _walkToFlag(false), _walkCurveEnd(0.0f), _walkCurveLast(0.0f),
_walkCurveLen(0.0f), _walkCurveIncrement(0.0f), _walkEndAnimG(false), _walkTotalFrames(0),
_walkCurveNextLength(0.0f), _walkedLength(0.0f), _walkLoopAnimLen(0.0f), _walkEndGAnimLen(0.0f),
_walkStartAnimLen(0.0f), _walkStartAnimFrameCount(0), _walkLoopAnimFrameCount(0),
_walkEndGAnimFrameCount(0), _hasAnchor(false), _charLookingAtOffset(0.0f) {
	_curModelAnim.setDeleteFn(&TeModelAnimation::deleteLaterStatic);
}

Character::~Character() {
	_model->setVisible(false);
	_model->bonesUpdatedSignal().remove(this, &Character::onBonesUpdate);
	deleteAnim();
	Game *game = g_engine->getGame();
	Common::Array<TeIntrusivePtr<TeModel>> &models = game->scene().models();
	for (uint i = 0; i < models.size(); i++) {
		if (models[i] == _model) {
			models.remove_at(i);
			break;
		}
	}
	removeAnim();
	for (uint s = 0; s < 2; s++) {
		if (!_shadowModel[s])
			continue;
		for (uint i = 0; i < models.size(); i++) {
			if (models[i] == _shadowModel[s]) {
				models.remove_at(i);
				break;
			}
		}
	}
}

/*static*/
void Character::cleanup() {
	if (_globalCharacterSettings)
		delete _globalCharacterSettings;
	_globalCharacterSettings = nullptr;
	animCacheFreeAll();
}

void Character::addCallback(const Common::String &animKey, const Common::String &fnName, float triggerFrame, float maxCalls) {
	Callback *c = new Callback();
	c->_luaFn = fnName;
	c->_lastCheckFrame = 0;
	c->_triggerFrame = (int)triggerFrame;
	c->_maxCalls = (int)maxCalls;
	// Slight difference here to orig (that sets -NAN) because of
	// the way this gets used later, setting large negative is more correct.
	c->_callsMade = (maxCalls == -1.0 ? -1e9 : 0.0f);

	if (g_engine->gameType() == TetraedgeEngine::kSyberia) {
		// Syberia 1 has slightly weird logic to decide what key to use.

		//
		// WORKAROUND: This callback seems to be set too late.. frame 31, but it
		// only gets to 15?  Some bug in the way anim blends hand off?
		// for scenes/CitSpace2/34230/Logic34230.lua
		//
		if (fnName == "ChangeClef" && c->_triggerFrame == 31)
			c->_triggerFrame = 15;

		const Common::Path animPath = _model->anim()->loadedPath();

		// Another difference.. the original messes with paths a bit - just
		// use the file name, since it's already limited by character.
		Common::String animName = animPath.getLastComponent().toString();
		if (animName.empty())
			animName = animPath.toString();

		if (_callbacks.contains(animName)) {
			_callbacks[animName].push_back(c);
		} else {
			Common::Path animKeyPath(animKey);
			Common::Array<Callback *> callbacks;
			callbacks.push_back(c);

			_callbacks.setVal(animKeyPath.getLastComponent().toString(), callbacks);
		}
	} else if (g_engine->gameType() == TetraedgeEngine::kSyberia2){
		// Syberia 2 is simpler, it always uses a lower-case version of the anim
		// file in the passed key.
		Common::String key = Common::Path(animKey).getLastComponent().toString();
		key.toLowercase();
		if (_callbacks.contains(key)) {
			_callbacks[key].push_back(c);
		} else {
			Common::Array<Callback *> callbacks;
			callbacks.push_back(c);
			_callbacks.setVal(key, callbacks);
		}
	} else {
		error("addCallback: Unsupported game type.");
	}
}

/*static*/
void Character::animCacheFreeAll() {
	/*
	if (_animCache) {
		for (const auto &entry : (*_animCache))
			_animCacheSize -= entry._size;
		delete _animCache;
		_animCache = nullptr;
	} */
	if (_animCacheMap) {
		delete _animCacheMap;
		_animCacheMap = nullptr;
	}
}

/*static*/
void Character::animCacheFreeOldest() {
	// Unused?
	//_animCacheSize -= _animCache[_animCache.size() - 1]._size;
	//_animCache.pop_back();
}

/*static*/
TeIntrusivePtr<TeModelAnimation> Character::animCacheLoad(const Common::Path &path) {
	const Common::String pathStr = path.toString();
	if (!_animCacheMap) {
		_animCacheMap = new Common::HashMap<Common::String, TeIntrusivePtr<TeModelAnimation>>();
	}
	if (_animCacheMap->contains(pathStr)) {
		// Copy from the cache (keep the cached instance clean)
		return new TeModelAnimation(*_animCacheMap->getVal(pathStr));
	}

	TeIntrusivePtr<TeModelAnimation> modelAnim = new TeModelAnimation();
	if (!modelAnim->load(path)) {
		warning("Failed to load anim %s", path.toString().c_str());
	}

	_animCacheMap->setVal(pathStr, modelAnim);
	return modelAnim;
}

float Character::animLength(const TeModelAnimation &modelanim, int bone, int lastframe) {
	int last = modelanim.lastFrame();
	if (lastframe > last)
		lastframe = last;
	int first = modelanim.firstFrame();
	const TeVector3f32 starttrans = translationVectorFromAnim(modelanim, bone, first);
	const TeVector3f32 endtrans = translationVectorFromAnim(modelanim, bone, lastframe);
	const TeVector3f32 secondtrans = translationVectorFromAnim(modelanim, bone, first + 1);
	return ((endtrans.z() - starttrans.z()) + secondtrans.z()) - starttrans.z();
}

float Character::animLengthFromFile(const Common::String &animname, uint32 *pframeCount, uint lastframe /* = 9999 */) {
	if (animname.empty()) {
		*pframeCount = 0;
		return 0.0f;
	}
	TeIntrusivePtr<TeModelAnimation> anim = _model->anim();
	if (!anim->loadedPath().toString().contains(animname)) {
		Common::Path animpath("models/Anims");
		animpath.joinInPlace(animname);
		anim = animCacheLoad(animpath);
		if (!anim)
			error("Character::animLengthFromFile couldn't load anim %s", animname.c_str());
	}

	// The "Pere" or "father" bone is the root.
	float animLen = animLength(*anim, anim->findBone(rootBone()), lastframe);
	int frameCount = anim->lastFrame() + 1 - anim->firstFrame();
	*pframeCount = frameCount;

	return animLen * _model->scale().z();
}

bool Character::blendAnimation(const Common::String &animname, float amount, bool repeat, bool returnToIdle) {
	Common::Path animpath("models/Anims");
	animpath.joinInPlace(animname);

	_notWalkAnim = !(animname.contains(_characterSettings._idleAnimFileName)
			|| animname.contains(walkAnim(WalkPart_Start))
			|| animname.contains(walkAnim(WalkPart_Loop))
			|| animname.contains(walkAnim(WalkPart_EndG))
			|| animname.contains(walkAnim(WalkPart_EndD)));

	if (_curModelAnim) {
		_curModelAnim->onFinished().remove(this, &Character::onModelAnimationFinished);
		_curModelAnim->unbind();
		_curModelAnim->reset();
	}

	_curModelAnim = animCacheLoad(animpath);
	assert(_curModelAnim);
	_curModelAnim->reset();
	_curModelAnim->onFinished().add(this, &Character::onModelAnimationFinished);

	_curModelAnim->bind(_model);
	_model->blendAnim(_curModelAnim, amount, repeat);
	_lastFrame = -1;
	_curModelAnim->play();
	_curAnimName = animname;
	_returnToIdleAnim = !repeat && returnToIdle;
	return true;
}

TeVector3f32 Character::correctPosition(const TeVector3f32 &pos) {
	bool flag;
	TeVector3f32 result = _freeMoveZone->correctCharacterPosition(pos, &flag, true);
	if (!flag)
		result.y() = _model->position().y();
	return result;
}

float Character::curveOffset() {
	return _walkCurveStart;
}

void Character::deleteAllCallback() {
	_callbacksChanged = true;
	for (auto &pair : _callbacks) {
		for (Callback *c : pair._value) {
			delete c;
		}
	}
	_callbacks.clear();
}

void Character::deleteAnim() {
	if (_curModelAnim) {
		_curModelAnim->onFinished().remove(this, &Character::onModelAnimationFinished);
		_curModelAnim->unbind();
		_curModelAnim->reset();
	}
	_model->removeAnim();
	_curModelAnim.release();
}

void Character::deleteCallback(const Common::String &key, const Common::String &fnName, float f) {
	_callbacksChanged = true;
	assert(_model->anim());
	Common::String animFile = _model->anim()->loadedPath().getLastComponent().toString();
	if (!_callbacks.contains(animFile))
		return;

	Common::Array<Callback *> &cbs = _callbacks.getVal(animFile);
	for (uint i = 0; i < cbs.size(); i++) {
		if (fnName.empty()) {
			delete cbs[i];
			// don't remove from array, clear at the end.
		} else if (cbs[i]->_luaFn == fnName) {
			if (f == -1 || cbs[i]->_triggerFrame == f) {
				delete cbs[i];
				cbs.remove_at(i);
				i--;
			}
		}
	}
	if (fnName.empty())
		cbs.clear();

	if (cbs.empty())
		_callbacks.erase(animFile);
}

void Character::endMove() {
	if (g_engine->getGame()->scene()._character == this)
		walkMode("Walk");

	_onFinishedSignal.call();
	stop();
}

const Character::WalkSettings *Character::getCurrentWalkFiles() {
	for (const auto & walkSettings : _characterSettings._walkSettings) {
		if (walkSettings._key == _walkModeStr)
			return &walkSettings._value;
	}
	return nullptr;
}

bool Character::isFramePassed(int frameno) {
	return (frameno > _lastAnimFrame && _model->anim()->curFrame2() >= frameno);
}

bool Character::isWalkEnd() {
	const Common::String animFile = _model->anim()->loadedPath().getLastComponent().toString();
	for (const auto & walkSettings : _characterSettings._walkSettings) {
		if (walkSettings._value._walkParts[WalkPart_EndD]._file.contains(animFile)
				|| walkSettings._value._walkParts[WalkPart_EndG]._file.contains(animFile))
			return true;
	}
	return false;
}

int Character::leftStepFrame(enum Character::WalkPart walkpart) {
	const Character::WalkSettings *settings = getCurrentWalkFiles();
	if (settings) {
		return settings->_walkParts[(int)walkpart]._stepLeft;
	}
	return -1;
}

int Character::rightStepFrame(enum Character::WalkPart walkpart) {
	const Character::WalkSettings *settings = getCurrentWalkFiles();
	if (settings) {
		return settings->_walkParts[(int)walkpart]._stepRight;
	}
	return -1;
}

bool Character::loadModel(const Common::String &mname, bool unused) {
	assert(_globalCharacterSettings);
	if (_model) {
		_model->bonesUpdatedSignal().remove(this, &Character::onBonesUpdate);
	}
	_model = new TeModel();
	_model->bonesUpdatedSignal().add(this, &Character::onBonesUpdate);

	if (!_globalCharacterSettings->contains(mname))
		return false;

	_characterSettings = _globalCharacterSettings->getVal(mname);
	_model->setTexturePath("models/Textures");
	_model->setEnableLights(true);
	if (!_model->load(Common::Path("models").join(_characterSettings._modelFileName))) {
		warning("Failed to load character model %s", _characterSettings._modelFileName.c_str());
		return false;
	}

	_model->setName(mname);
	_model->setScale(_characterSettings._defaultScale);
	if (_characterSettings._invertNormals)
		_model->invertNormals();

	for (auto &mesh : _model->meshes())
		mesh->setVisible(true);

	// Set all mouthes, eyes, etc not visible by default
	_model->setVisibleByName("_B_", false);
	_model->setVisibleByName("_Y_", false);
	_model->setVisibleByName("_M_", false);
	_model->setVisibleByName("_E_", false);

	// Note: game loops through "faces" here, but it only ever uses the default ones.
	_model->setVisibleByName(_characterSettings._defaultEyes, true);
	_model->setVisibleByName(_characterSettings._defaultMouth, true);
	_model->setVisibleByName(_characterSettings._defaultBody, true);

	setAnimation(_characterSettings._idleAnimFileName, true);

	_walkStartAnimLen = animLengthFromFile(walkAnim(WalkPart_Start), &_walkStartAnimFrameCount);
	_walkEndGAnimLen = animLengthFromFile(walkAnim(WalkPart_EndG), &_walkEndGAnimFrameCount);
	_walkLoopAnimLen = animLengthFromFile(walkAnim(WalkPart_Loop), &_walkLoopAnimFrameCount);

	if (g_engine->gameType() == TetraedgeEngine::kSyberia) {
		// Only Syberia 1 has the simple shadow.
		TeIntrusivePtr<Te3DTexture> shadow = Te3DTexture::makeInstance();
		TeCore *core = g_engine->getCore();
		shadow->load(core->findFile("models/Textures/simple_shadow_alpha.tga"));

		for (int i = 0; i < 2; i++) {
			TeModel *pmodel = new TeModel();
			_shadowModel[i] = pmodel;
			pmodel->setName("Shadow");
			Common::Array<TeVector3f32> arr;
			arr.resize(4);
			arr[0] = TeVector3f32(-60.0, 0.0, -60.0);
			arr[1] = TeVector3f32(-60.0, 0.0, 60.0);
			arr[2] = TeVector3f32(60.0, 0.0, -60.0);
			arr[3] = TeVector3f32(60.0, 0.0, 60.0);
			pmodel->setQuad(shadow, arr, TeColor(0xff, 0xff, 0xff, 0x50));
		}
	}
	return true;
}

/*static*/
bool Character::loadSettings(const Common::String &path) {
	CharacterSettingsXmlParser parser;
	parser.setAllowText();
	if (_globalCharacterSettings)
		delete _globalCharacterSettings;
	_globalCharacterSettings = new Common::HashMap<Common::String, CharacterSettings>();
	parser.setCharacterSettings(_globalCharacterSettings);

	// WORKAROUND: This file contains invalid comments
	// eg, <!--------- and a comment-inside-a-comment.
	// patch them before parsing.
	Common::File xmlFile;
	if (!xmlFile.open(path))
		error("Character::loadSettings: Can't open %s", path.c_str());
	const int64 bufsize = xmlFile.size();
	char *buf = new char[bufsize+1];
	buf[bufsize] = '\0';
	xmlFile.read(buf, bufsize);
	Common::String fixedbuf(buf);
	delete [] buf;
	size_t offset = fixedbuf.find("------------");
	while (offset != Common::String::npos) {
		fixedbuf.replace(offset, 12, "--");
		offset = fixedbuf.find("------------");
	}

	// Big HACK: Remove the embedded comment in this config.
	offset = fixedbuf.find("<!--<walk>");
	if (offset != Common::String::npos) {
		size_t endOffset = fixedbuf.find(" -->", offset);
		if (endOffset != Common::String::npos) {
			size_t realEndOffset = fixedbuf.find("walk>-->", endOffset);
			if (realEndOffset  != Common::String::npos && realEndOffset > endOffset) {
				fixedbuf.replace(offset, endOffset - offset, "<!-- ");
			}
		}
	}

	if (!parser.loadBuffer((const byte *)fixedbuf.c_str(), bufsize))
		error("Character::loadSettings: Can't open %s", path.c_str());

	if (!parser.parse())
		error("Character::loadSettings: Can't parse %s", path.c_str());

	return true;
}

bool Character::onBonesUpdate(const Common::String &boneName, TeMatrix4x4 &boneMatrix) {
	if (!_model || !_model->anim())
		return false;

	Game *game = g_engine->getGame();
	if (boneName == rootBone()) {
		const Common::String animfile = _model->anim()->loadedPath().getLastComponent().toString();
		bool resetX = false;
		if (game->scene()._character == this) {
			for (const auto &walkSettings : _characterSettings._walkSettings) {
				if (walkSettings._key.contains("Walk") || walkSettings._key.contains("Jog")) {
					resetX |= (walkSettings._value._walkParts[0]._file.contains(animfile)
								|| walkSettings._value._walkParts[1]._file.contains(animfile)
								|| walkSettings._value._walkParts[2]._file.contains(animfile)
								|| walkSettings._value._walkParts[3]._file.contains(animfile));
				}
			}
			resetX |= _characterSettings._idleAnimFileName.contains(animfile);
		} else {
			resetX = (_characterSettings._idleAnimFileName.contains(animfile) ||
					  walkAnim(WalkPart_Start).contains(animfile) ||
					  walkAnim(WalkPart_Loop).contains(animfile) ||
					  walkAnim(WalkPart_EndD).contains(animfile) ||
					  walkAnim(WalkPart_EndG).contains(animfile));
		}
		if (resetX) {
			boneMatrix.setValue(0, 3, 0.0f);
			boneMatrix.setValue(2, 3, 0.0f);
		}
	}

	if (boneName.contains("Bip01 Head")) {
		if (_hasAnchor) {
			game->scene().currentCamera()->apply();
			_lastHeadRotation = _headRotation;
			TeQuaternion rot1 = TeQuaternion::fromAxisAndAngle(TeVector3f32(-1, 0, 0), _lastHeadRotation.getX());
			TeQuaternion rot2 = TeQuaternion::fromAxisAndAngle(TeVector3f32(0, 0, 1), _lastHeadRotation.getY());
			boneMatrix.rotate(rot1);
			boneMatrix.rotate(rot2);
		} else {
			float lastHeadX = _lastHeadRotation.getX();
			float minX = (lastHeadX > 0) ? -0.1 : 0.1;
			float newX = (fabs(minX) > fabs(lastHeadX)) ? 0.0 : minX + lastHeadX;
			_lastHeadRotation.setX(newX);

			float lastHeadY = _lastHeadRotation.getY();
			float minY = (lastHeadY > 0) ? -0.1 : 0.1;
			float newY = (fabs(minY) > fabs(lastHeadY)) ? 0.0 : minY + lastHeadY;
			_lastHeadRotation.setY(newY);

			_headRotation = _lastHeadRotation;

			TeQuaternion rot1 = TeQuaternion::fromAxisAndAngle(TeVector3f32(-1, 0, 0), _lastHeadRotation.getX());
			TeQuaternion rot2 = TeQuaternion::fromAxisAndAngle(TeVector3f32(0, 0, 1), _lastHeadRotation.getY());
			boneMatrix.rotate(rot1);
			boneMatrix.rotate(rot2);
			_lastHeadBoneTrans = boneMatrix.translation();
		}
	}

	if (boneName.contains("Bip01 L Foot") || boneName.contains("Bip01 R Foot")) {
		TeVector3f32 trans = boneMatrix.translation();
		trans.rotate(_model->rotation());
		const TeVector3f32 modelScale = _model->scale();
		trans.x() *= modelScale.x();
		trans.y() = 0.0;
		trans.z() *= modelScale.z();
		TeVector3f32 pos = _model->position() + trans;
		if (_freeMoveZone) {
			bool flag;
			pos = _freeMoveZone->correctCharacterPosition(pos, &flag, true);
		}
		int shadowNo = boneName.contains("Bip01 L Foot") ? 0 : 1;
		if (_shadowModel[shadowNo]) {
			_shadowModel[shadowNo]->setPosition(pos);
			_shadowModel[shadowNo]->setRotation(_model->rotation());
			_shadowModel[shadowNo]->setScale(_model->scale());
		}
	}

	// Move any objects attached to the bone
	for (Object3D *obj : game->scene().object3Ds()) {
		if (obj->_onCharName == _model->name() && boneName == obj->_onCharBone) {
			if (_model->anim()->curFrame2() >= obj->_startFrame
					&& _model->anim()->curFrame2() <= obj->_endFrame) {
				obj->model()->setVisible(true);

				if (!obj->_moveAnim._runTimer.running()) {
					obj->_lastMatrix = boneMatrix;
					obj->_lastMatrix.scale(obj->_objScale);
					obj->_lastMatrix.rotate(obj->_objRotation);
					obj->_lastMatrix.translate(obj->_objTranslation);
					obj->model()->forceMatrix(obj->_lastMatrix);
					obj->model()->setRotation(_model->rotation());
					obj->model()->setPosition(_model->position());
					obj->model()->setScale(_model->scale());
				} else {
					obj->model()->forceMatrix(obj->_lastMatrix);
					obj->model()->setRotation(_model->rotation());
					obj->model()->setPosition(_model->position() + obj->_curMovePos);
					obj->model()->setScale(_model->scale());
				}
			} else {
				obj->model()->setVisible(false);
			}
		}
	}

	return true;
}

bool Character::onModelAnimationFinished() {
	// this shouldn't happen but check to be sure..
	if (!_model || !_model->anim())
		return false;

	const Common::Path loadedPath = _model->anim()->loadedPath();
	const Common::String animfile = loadedPath.getLastComponent().toString();

	bool shouldAdjust = true;
	for (const auto &unrecal : g_engine->getApplication()->unrecalAnims()) {
		if (animfile.contains(unrecal))
			shouldAdjust = false;
	}

	Game *game = g_engine->getGame();
	bool isWalkAnim = false;
	if (game->scene()._character == this) {
		for (const auto &walkSettings : _characterSettings._walkSettings) {
			if (walkSettings._key.contains("Walk") || walkSettings._key.contains("Jog")) {
				isWalkAnim |= (walkSettings._value._walkParts[0]._file.contains(animfile)
							 || walkSettings._value._walkParts[1]._file.contains(animfile)
							 || walkSettings._value._walkParts[2]._file.contains(animfile)
							 || walkSettings._value._walkParts[3]._file.contains(animfile));
			}
		}
		isWalkAnim |= animfile.contains(_characterSettings._idleAnimFileName);
	} else {
		isWalkAnim = (_characterSettings._idleAnimFileName.contains(animfile)
					|| walkAnim(WalkPart_Start).contains(animfile)
					|| walkAnim(WalkPart_Loop).contains(animfile)
					|| walkAnim(WalkPart_EndD).contains(animfile)
					|| walkAnim(WalkPart_EndG).contains(animfile));
	}

	if (!isWalkAnim && shouldAdjust) {
		int pereBone = _curModelAnim->findBone(rootBone());
		const TeTRS endTRS = trsFromAnim(*_curModelAnim, pereBone, _curModelAnim->lastFrame());
		TeVector3f32 trans = endTRS.getTranslation();
		trans.x() = -trans.x();

		TeVector3f32 newpos;
		if (!_recallageY) {
			const TeTRS startTRS = trsFromAnim(*_curModelAnim, pereBone, _curModelAnim->firstFrame());
			trans = trans - startTRS.getTranslation();
			const TeTRS nearEndTRS = trsFromAnim(*_curModelAnim, pereBone, _curModelAnim->lastFrame() - 1);
			trans  = trans + (endTRS.getTranslation() - nearEndTRS.getTranslation());
			newpos = _model->worldTransformationMatrix() * trans;
		} else if (!_freeMoveZone) {
			trans.x() = -trans.x();
			trans.y() = 0.0;
			newpos = _model->worldTransformationMatrix() * trans;
		} else {
			newpos = correctPosition(_model->worldTransformationMatrix() * trans);
		}
		_model->setPosition(newpos);
	}

	if (game->scene()._character == this) {
		_characterAnimPlayerFinishedSignal.call(loadedPath.toString());
	} else {
		_onCharacterAnimFinishedSignal.call(_model->name());
	}

	Common::Path setAnimNamePath = _setAnimName;
	Common::String setAnimNameFile = setAnimNamePath.getLastComponent().toString();
	Common::String loadedPathFile = loadedPath.getLastComponent().toString();
	if (_returnToIdleAnim && loadedPathFile.contains(setAnimNameFile)) {
		_notWalkAnim = false;
		_returnToIdleAnim = false;
		setAnimation(_characterSettings._idleAnimFileName, true);
	}

	return false;
}

void Character::permanentUpdate() {
	assert(_model->anim());
	const Common::Path &animPath = _model->anim()->loadedPath();
	int curFrame = _model->anim()->curFrame2();
	Game *game = g_engine->getGame();
	_callbacksChanged = false;
	// Diverge from original - just use filename for anim callbacks as the
	// original does werid things with paths.
	Common::String animFile = animPath.getLastComponent().toString();
	if (g_engine->gameType() == TetraedgeEngine::kSyberia2)
		animFile.toLowercase();

	//if (!_callbacks.empty())
	//	debug("%s: check cbs for %s frame %d speed %.02f", _model->name().c_str(),
	//			animFile.c_str(), curFrame, _model->anim()->speed());

	if (_callbacks.contains(animFile)) {
		Common::Array<Callback *> &cbs = _callbacks.getVal(animFile);
		for (Callback *cb : cbs) {
			//debug("%s: check cb for %s frame %d against %d. speed %.02f", _model->name().c_str(),
			//		animFile.c_str(), curFrame, cb->_triggerFrame, _model->anim()->speed());
			if (cb->_triggerFrame > cb->_lastCheckFrame && curFrame >= cb->_triggerFrame){
				int callsMade = cb->_callsMade;
				cb->_callsMade++;
				if (callsMade >= cb->_maxCalls)
					continue;
				cb->_lastCheckFrame = curFrame;
				game->luaScript().execute(cb->_luaFn);
				if (_callbacksChanged)
					break;
			}
			cb->_lastCheckFrame = curFrame;
		}
	}

	if (animFile.contains("ka_esc_h")) {
		if (_lastAnimFrame < 7 && _model->anim()->curFrame2() > 6) {
			game->playSound("sounds/SFX/PAS_F_PAVE1.ogg", 1, 1.0f);
		} else if (_lastAnimFrame < 22 && _model->anim()->curFrame2() > 21) {
			game->playSound("sounds/SFX/PAS_F_PAVE2.ogg", 1, 1.0f);
		}
	} else if (animFile.contains("ka_esc_b")) {
		if (_lastAnimFrame < 12 && _model->anim()->curFrame2() > 11) {
			game->playSound("sounds/SFX/PAS_F_PAVE1.ogg", 1, 1.0f);
		} else if (_lastAnimFrame < 27 && _model->anim()->curFrame2() > 26) {
			game->playSound("sounds/SFX/PAS_F_PAVE2.ogg", 1, 1.0f);
		}
	}
	updateAnimFrame();
}

void Character::placeOnCurve(TeIntrusivePtr<TeBezierCurve> &curve) {
	_curve = curve;
	updatePosition(_walkCurveStart);
}

void Character::removeAnim() {
	if (_curModelAnim) {
		_curModelAnim->onFinished().remove(this, &Character::onModelAnimationFinished);
		_curModelAnim->unbind();
		_curModelAnim->reset();
	}
	_model->removeAnim();
	if (_curModelAnim) {
		_curModelAnim.release();
	}
}

void Character::removeFromCurve() {
	_curve.release();
}

Common::String Character::rootBone() const {
	if (g_engine->gameType() != TetraedgeEngine::kSyberia2 || _model->name() != "Youki")
		return "Pere";
	else
		return "Bip01";
}

bool Character::setAnimation(const Common::String &aname, bool repeat, bool returnToIdle, bool unused, int startFrame, int endFrame) {
	if (aname.empty())
		return false;

	Common::Path animPath("models/Anims");
	animPath.joinInPlace(aname);
	bool isWalkAnim = (aname.contains(_characterSettings._idleAnimFileName) ||
					  aname.contains(walkAnim(WalkPart_Start)) ||
					  aname.contains(walkAnim(WalkPart_Loop)) ||
					  aname.contains(walkAnim(WalkPart_EndD)) ||
					  aname.contains(walkAnim(WalkPart_EndG)));
	_notWalkAnim = !isWalkAnim;

	if (_curModelAnim) {
		_curModelAnim->onFinished().remove(this, &Character::onModelAnimationFinished);
		_curModelAnim->unbind();
		_curModelAnim->reset();
	}

	_curModelAnim = animCacheLoad(animPath);
	_curModelAnim->reset();
	_curModelAnim->onFinished().add(this, &Character::onModelAnimationFinished);
	_curModelAnim->bind(_model);
	_curModelAnim->setFrameLimits(startFrame, endFrame);
	_model->setAnim(_curModelAnim, repeat);
	_lastFrame = -1;
	_curModelAnim->play();
	_setAnimName = aname;
	_curAnimName = aname;
	_returnToIdleAnim = !repeat && returnToIdle;

	return true;
}

void Character::setAnimationSound(const Common::String &sname, uint offset) {
	warning("TODO: Set field 0x2f8 to 0 in Character::setAnimationSound.");
	_animSound = sname;
	_animSoundOffset = offset;
}

void Character::setCurveOffset(float offset) {
	_walkCurveStart = offset;
	updatePosition(offset);
}

void Character::setFreeMoveZone(TeFreeMoveZone *zone) {
	_freeMoveZone = zone;
}

void Character::setStepSound(const Common::String &stepSound1, const Common::String &stepSound2) {
	_stepSound1 = stepSound1;
	_stepSound2	= stepSound2;
}

bool Character::setShadowVisible(bool visible) {
	if (_shadowModel[0]) {
		_shadowModel[0]->setVisible(visible);
		_shadowModel[1]->setVisible(visible);
	}
	return false;
}

float Character::speedFromAnim(double msFromStart) {
	if (!_model)
		return 0.0f;

	TeIntrusivePtr<TeModelAnimation> modelAnim;
	if (_model->boneBlenders().empty()) {
		modelAnim = _model->anim();
	} else {
		modelAnim = _model->boneBlenders().back()->_anim;
	}

	if (!modelAnim)
		return 0.0f;

	const int pereBone = modelAnim->findBone(rootBone());
	int curFrame = modelAnim->calcCurrentFrame(msFromStart);

	float result;
	if (_lastFrame == -1) {
		const TeVector3f32 nowvec = translationVectorFromAnim(*modelAnim, pereBone, 0);
		const TeVector3f32 lastvec = translationVectorFromAnim(*modelAnim, pereBone, 1);
		result = lastvec.z() - nowvec.z();
	} else {
		const TeVector3f32 nowvec = translationVectorFromAnim(*modelAnim, pereBone, curFrame);
		const TeVector3f32 lastvec = translationVectorFromAnim(*modelAnim, pereBone, _lastFrame);
		result = nowvec.z() - lastvec.z();
		if (curFrame < _lastFrame) {
			result += animLength(*modelAnim, pereBone, 9999);
		}
	}
	_lastFrame = curFrame;
	result *= _model->scale().z();
	return result;
}

float Character::translationFromAnim(const TeModelAnimation &anim, int bone, int frame) {
	return translationVectorFromAnim(anim, bone, frame).z();
}

TeVector3f32 Character::translationVectorFromAnim(const TeModelAnimation &anim, int bone, int frame) {
	const TeTRS trs = trsFromAnim(anim, bone, frame);
	return trs.getTranslation();
}

TeTRS Character::trsFromAnim(const TeModelAnimation &anim, int bone, int frame) {
	if (bone == -1)
		return TeTRS();

	return anim.getTRS(bone, frame, false);
}

void Character::update(double msFromStart) {
	if (!_curve || !_runTimer.running())
		return;

	_walkCurveNextLength = speedFromAnim(msFromStart) * _walkCurveIncrement + _walkCurveNextLength;

	if (_curve->controlPoints().size() < 2) {
		blendAnimation(_characterSettings._idleAnimFileName, 0.0667f, true, false);
		endMove();
		return;
	}

	const float baseAngle = (_walkCurveStart > _walkCurveEnd ? M_PI : 0);
	const float sign = (_walkCurveStart > _walkCurveEnd ? -1 : 1);

	updatePosition(_walkCurveLast);

	float lastWalkedLength = _walkedLength;
	TeVector3f32 lastNextPos = _model->position();
	TeVector3f32 nextPos = _model->position();
	TeVector3f32 newPos = _model->position();
	float lastOffset = _walkCurveLast;

	// First do a coarse search for the position, then back up 1 step and do a finer
	// search.

	const float coarseStep = (4.0 / _curve->numIterations()) * sign;
	const float fineStep = (1.0 / _curve->numIterations()) * sign;

	float offset = _walkCurveLast;
	while (_walkedLength < _walkCurveNextLength) {
		lastOffset = offset;
		lastWalkedLength = _walkedLength;
		lastNextPos = nextPos;

		offset = CLIP(lastOffset + coarseStep, 0.0f, 1.0f);

		newPos = _curve->retrievePoint(offset) + _curveStartLocation;
		const TeVector2f32 dist = TeVector2f32(nextPos.x(), nextPos.z()) - TeVector2f32(newPos.x(), newPos.z());
		_walkedLength += dist.length();

		nextPos = newPos;
		if (offset == 1.0 || offset == 0.0)
			break;
	}

	_walkedLength = lastWalkedLength;
	nextPos = lastNextPos;
	offset = lastOffset;

	while (_walkedLength < _walkCurveNextLength) {
		offset = CLIP(offset + fineStep, 0.0f, 1.0f);

		newPos = _curve->retrievePoint(offset) + _curveStartLocation;
		const TeVector2f32 dist = TeVector2f32(nextPos.x(), nextPos.z()) - TeVector2f32(newPos.x(), newPos.z());
		_walkedLength += dist.length();

		nextPos = newPos;
		if (offset == 1.0 || offset == 0.0)
			break;
	}

	if (_freeMoveZone) {
		bool correctflag;
		newPos = _freeMoveZone->correctCharacterPosition(newPos, &correctflag, true);
	}

	//debug("Character::update %4d %.04f %s -> %s %.4f", (int)msFromStart, offset, _model->position().dump().c_str(),
	//		newPos.dump().c_str(), (newPos - _model->position()).length());

	_walkCurveLast = offset;
	_model->setPosition(newPos);

	TeVector3f32 t1;
	TeVector3f32 t2;
	_curve->pseudoTangent(offset, t1, t2);
	const TeVector3f32 normalizedTangent = (t2 - t1).getNormalized();
	float angle = TeVector3f32(0.0, 0.0, 1.0).dotProduct(normalizedTangent);
	angle = acos(angle);
	TeVector3f32 crossprod = TeVector3f32::crossProduct(TeVector3f32(0.0, 0.0, 1.0), normalizedTangent);
	if (crossprod.y() >= 0.0f) {
		angle = -angle;
	}
	//debug("update: curve offset %f - angle %f (base %f)", offset, angle, baseAngle);
	TeQuaternion rot = TeQuaternion::fromAxisAndAngle(TeVector3f32(0.0, 1.0, 0.0), baseAngle + angle);
	_model->setRotation(rot);

	const Common::String endGAnim = walkAnim(WalkPart_EndG);
	if (_walkCurveLast == _walkCurveEnd || fabs(_walkCurveEnd - _walkCurveStart) < fabs(_walkCurveLast - _walkCurveStart)) {
		if (_walkToFlag) {
			_walkToFlag = false;
			endMove();
		}
		if (endGAnim.empty()) {
			blendAnimation(_characterSettings._idleAnimFileName, 0.0667f, true, false);
			endMove();
		}
	}

	if (!endGAnim.empty() && _curAnimName == walkAnim(WalkPart_Loop) &&
		   ((_curModelAnim->speed() * (msFromStart / 1000.0)) >= _walkTotalFrames)) {
		if (_walkToFlag) {
			_walkToFlag = false;
			endMove();
		} else {
			if (_walkEndAnimG)
				setAnimation(walkAnim(WalkPart_EndG), false);
			else
				setAnimation(walkAnim(WalkPart_EndD), false);
		}
	}

	// Note:
	// The game does a bunch of extra things here (line 252 on)
	// that seem to have no actual effect??

	updateAnimFrame();
}

void Character::updateAnimFrame() {
	if (_model->anim()) {
		_lastAnimFrame = _model->anim()->curFrame2();
	}
}

void Character::updatePosition(float curveOffset) {
	assert(_curve);
	if (!_curve->controlPoints().empty()) {
		TeVector3f32 pt = _curve->retrievePoint(curveOffset) + _curveStartLocation;
		if (_freeMoveZone) {
			bool flag;
			pt = _freeMoveZone->correctCharacterPosition(pt, &flag, true);
		}
		_model->setPosition(pt);
	}
}

Common::String Character::walkAnim(Character::WalkPart part) {
	Common::String result;
	const Character::WalkSettings *settings = getCurrentWalkFiles();
	if (settings) {
		return settings->_walkParts[(int)part]._file;
	}
	return result;
}

void Character::walkMode(const Common::String &mode) {
	if (_walkModeStr != mode)
		_walkModeStr = mode;
	_walkStartAnimLen = animLengthFromFile(walkAnim(WalkPart_Start), &_walkStartAnimFrameCount);
	_walkEndGAnimLen = animLengthFromFile(walkAnim(WalkPart_EndG), &_walkEndGAnimFrameCount);
	_walkLoopAnimLen = animLengthFromFile(walkAnim(WalkPart_Loop), &_walkLoopAnimFrameCount);
}

void Character::walkTo(float curveEnd, bool walkFlag) {
	_walkToFlag = walkFlag;
	stop();
	_walkCurveEnd = curveEnd;
	_walkCurveLast = _walkCurveStart;
	_walkCurveNextLength = 0.0f;
	_walkedLength = 0.0f;
	if (!_curve)
		warning("_curve not set in Character::walkTo");
	if (_curve && _curve->controlPoints().size()) {
		const float walkEndLen = (walkFlag ? 0 : _walkEndGAnimLen);
		_walkCurveLen = _curve->length();
		_walkEndAnimG = false;
		const float nloops = (_walkCurveLen - (walkEndLen + _walkStartAnimLen)) / _walkLoopAnimLen;
		float animLen;
		if (nloops >= 0) {
			Game *game = g_engine->getGame();
			if (game->scene()._character == this && _walkModeStr == "Walk") {
				int looplen = (int)(nloops * _walkLoopAnimFrameCount);
				int repeats = looplen / _walkLoopAnimFrameCount;
				uint32 remainder = looplen % _walkLoopAnimFrameCount;

				uint framecounts[4];

				if (repeats == 0)
					framecounts[0] = INT_MAX;
				else
					framecounts[0] = (repeats - 1) * _walkLoopAnimFrameCount + 29;

				framecounts[1] = _walkLoopAnimFrameCount * repeats + 13;
				framecounts[2] = _walkLoopAnimFrameCount * repeats + 29;
				framecounts[3] = _walkLoopAnimFrameCount * (repeats + 1) + 13;

				for (int i = 0; i < 4; i++) {
					framecounts[i] = abs((int)(framecounts[i] - (int)(nloops * _walkLoopAnimFrameCount)));
				}

				int minoffset = 0;
				for (int i = 0; i < 4; i++) {
					if (framecounts[i] < framecounts[minoffset])
						minoffset = i;
				}

				switch(minoffset) {
				case 0:
					remainder = 29;
					_walkEndAnimG = true;
					repeats--;
					break;
				case 1:
					remainder = 13;
					break;
				case 2:
					remainder = 29;
					_walkEndAnimG = true;
					break;
				case 3:
					remainder = 13;
					repeats++;
					break;
				}
				_walkTotalFrames = _walkLoopAnimFrameCount * repeats + _walkStartAnimFrameCount + remainder;
				const float loopAnimLen = animLengthFromFile(walkAnim(WalkPart_Loop), &remainder, remainder);
				_walkCurveIncrement = _walkCurveLen / (repeats * _walkLoopAnimLen + walkEndLen + _walkStartAnimLen + loopAnimLen);
				play();
				return; // NOTE: early return here.
			} else {
				// Run or NPC walk
				double intpart;
				double remainder = modf(nloops, &intpart);
				if (remainder >= 0.5) {
					_walkEndAnimG = true;
					intpart += 0.75;
				} else {
					intpart += 0.25;
				}
				_walkTotalFrames = (int)(_walkLoopAnimFrameCount * intpart) + _walkStartAnimFrameCount;
				animLen = walkEndLen + (float)_walkStartAnimLen + intpart * _walkLoopAnimLen;
			}
		} else {
			_walkTotalFrames = _walkStartAnimFrameCount;
			animLen = (float)(_walkStartAnimLen + _walkEndGAnimLen);
		}
		_walkCurveIncrement = _walkCurveLen / animLen;
	}
	play();
}

Character::Water::Water() {
	_model = new TeModel();
	_model->setName("Water");
	TeIntrusivePtr<TeCamera> cam = g_engine->getGame()->scene().currentCamera();
	if (!cam)
		error("No active camera when constructing water");
	TeMatrix4x4 camMatrix = cam->worldTransformationMatrix();
	Common::Array<TeVector3f32> quad;
	quad.resize(4);
	quad[0] = camMatrix.mult3x3(TeVector3f32(-0.1f, 0.0f,  0.1f));
	quad[1] = camMatrix.mult3x3(TeVector3f32( 0.1f, 0.0f,  0.1f));
	quad[2] = camMatrix.mult3x3(TeVector3f32(-0.1f, 0.0f, -0.1f));
	quad[3] = camMatrix.mult3x3(TeVector3f32( 0.1f, 0.0f, -0.1f));
	const TeQuaternion noRot = TeQuaternion::fromEuler(TeVector3f32(0, 0, 0));
	TeIntrusivePtr<Te3DTexture> tex = Te3DTexture::makeInstance();
	tex->load(g_engine->getCore()->findFile("texturesIngame/EauOndine1.tga"));
	_model->setQuad(tex, quad, TeColor(255, 0, 0, 0));
	_model->setRotation(noRot);
	_model->setScale(TeVector3f32(0.5, 0.5, 0.5));
	_colorAnim._duration = 2000.0f;
	TeColor col = _model->color();
	col.a() = 100;
	_colorAnim._startVal = col;
	col.a() = 0;
	_colorAnim._endVal = col;
	Common::Array<float> curve;
	curve.push_back(0);
	curve.push_back(1);
	_colorAnim.setCurve(curve);
	_colorAnim._callbackObj = _model.get();
	_colorAnim._callbackMethod = &TeModel::setColor;
	_colorAnim.play();
	_scaleAnim._duration = 2000.0f;
	_scaleAnim._startVal = _model->scale();
	_scaleAnim._endVal = TeVector3f32(3.0f, 3.0f, 3.0f);
	_scaleAnim.setCurve(curve);
	_scaleAnim._callbackObj = _model.get();
	_scaleAnim._callbackMethod = &TeModel::setScale;
}

} // end namespace Tetraedge
