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

#include "tetraedge/tetraedge.h"
#include "tetraedge/game/character.h"
#include "tetraedge/game/game.h"
#include "tetraedge/game/character_settings_xml_parser.h"
#include "tetraedge/te/te_model_animation.h"

namespace Tetraedge {

/*static*/ Common::HashMap<Common::String, Character::CharacterSettings> *Character::_globalCharacterSettings = nullptr;

/*static*/ Common::Array<Character::AnimCacheElement> Character::_animCache;
uint Character::_animCacheSize = 0;

void Character::CharacterSettings::clear() {
	_name.clear();
	_modelFileName.clear();
	_defaultScale = TeVector3f32();
	_walkFileName.clear();
	_walkSettings.clear();
	_walkSpeed = 0.0f;
	_cutSceneCurveDemiPosition = TeVector3f32();
	_defaultEyes.clear();
	_defaultMouth.clear();
	_defaultBody.clear();
}

void Character::WalkSettings::clear() {
	for (int i = 0; i < 4; i++) {
		_walkParts[i] = AnimSettings();
	}
}

Character::Character() : _curveOffset(0), _lastFrame(-1), _callbacksChanged(false),
_notWalkAnim(false), _someRepeatFlag(false), _walkModeStr("Walk"),
_needsSomeUpdate(false), _positionFlag(false), _lookingAtTallThing(false),
_stepSound1("sounds/SFX/PAS_H_BOIS1.ogg"), _stepSound2("sounds/SFX/PAS_H_BOIS2.ogg"),
_freeMoveZone(nullptr), _animSoundOffset(0), _lastAnimFrame(0), _charLookingAt(nullptr),
_recallageY(true) {
	_curModelAnim.setDeleteFn(&TeModelAnimation::deleteLater);
}

Character::~Character() {
	_model->setVisible(false);
	_model->bonesUpdatedSignal().remove(this, &Character::onBonesUpdate);
	deleteAnim();
	Game *game = g_engine->getGame();
	Common::Array<TeIntrusivePtr<TeModel>> &models = game->scene().models();
	for (unsigned int i = 0; i < models.size(); i++) {
		if (models[i] == _model) {
			models.remove_at(i);
			break;
		}
	}
	removeAnim();
	for (unsigned int s = 0; s < 2; s++) {
		for (unsigned int i = 0; i < models.size(); i++) {
			if (models[i] == _shadowModel[s]) {
				models.remove_at(i);
				break;
			}
		}
	}
}

void Character::addCallback(const Common::String &key, const Common::String &s2, float f1, float f2) {
	/*Callback *c = new Callback();
	c->x = (int)f1;
	c->y = (int)f2;
	c->f = (f2 == -1.0 ? -NAN : 0.0f;*/
	error("TODO: Implement Character::addCallback");
}

/*static*/ void Character::animCacheFreeAll() {
	for (const auto &entry : _animCache)
		_animCacheSize -= entry._size;
	_animCache.clear();
}

/*static*/ void Character::animCacheFreeOldest() {
	_animCacheSize -= _animCache[_animCache.size() - 1]._size;
	_animCache.pop_back();
}

/*static*/ TeIntrusivePtr<TeModelAnimation> Character::animCacheLoad(const Common::Path &path) {
	static Common::HashMap<Common::String, TeIntrusivePtr<TeModelAnimation>> _cache;

	const Common::String pathStr = path.toString();
	if (_cache.contains(pathStr))
		return _cache.getVal(pathStr);

	TeIntrusivePtr<TeModelAnimation> modelAnim = new TeModelAnimation();
	if (!modelAnim->load(path)) {
		warning("Failed to load anim %s", path.toString().c_str());
	}

	_cache.setVal(pathStr, modelAnim);
	return modelAnim;
}

float Character::animLength(const TeModelAnimation &modelanim, long bone, long lastframe) {
	int last = modelanim.lastFrame();
	if (lastframe > last)
		lastframe = last;
	int first = modelanim.firstFrame();
	const TeVector3f32 starttrans = translationVectorFromAnim(modelanim, bone, first);
	const TeVector3f32 endtrans = translationVectorFromAnim(modelanim, bone, last);
	const TeVector3f32 secondtrans = translationVectorFromAnim(modelanim, bone, first + 1);
	return ((endtrans.z() - starttrans.z()) + secondtrans.z()) - starttrans.z();
}

float Character::animLengthFromFile(const Common::String &animname, uint *pframeCount, uint lastframe) {
	if (animname.empty()) {
		*pframeCount = 0;
		return 0.0f;
	}
	TeIntrusivePtr<TeModelAnimation> anim = _model->anim();
	if (!anim->_loadedPath.toString().contains(animname)) {
		Common::Path animpath("models/Anims");
		animpath.joinInPlace(animname);
		anim = animCacheLoad(animpath);
		if (!anim)
			error("Character::animLengthFromFile couldn't load anim %s", animname.c_str());
	}

	// The "Pere" or "father" bone is the root.
	float animLen = animLength(*anim, anim->findBone("Pere"), lastframe);
	int frameCount = anim->lastFrame() + 1 - anim->firstFrame();
	*pframeCount = frameCount;

	return animLen * _model->scale().z();
}

bool Character::blendAnimation(const Common::String &animname, float amount, bool repeat, bool param_4) {
	Common::Path animpath("models/Anims");
	animpath.joinInPlace(animname);

	_notWalkAnim = !(animname.contains(_characterSettings._walkFileName)
			|| animname.contains(walkAnim(WalkPart_Start))
			|| animname.contains(walkAnim(WalkPart_Loop))
			|| animname.contains(walkAnim(WalkPart_EndG))
			|| animname.contains(walkAnim(WalkPart_EndD)));

	if (_curModelAnim) {
		_curModelAnim->onFinished().remove(this, &Character::onModelAnimationFinished);
		_curModelAnim->unbind();
	}

	_curModelAnim = animCacheLoad(animpath);
	_curModelAnim->onFinished().add(this, &Character::onModelAnimationFinished);

	_curModelAnim->bind(_model);
	_model->blendAnim(_curModelAnim, amount, repeat);
	_lastFrame = -1;
	_curModelAnim->play();
	_curAnimName = animname;
	warning("TODO: Set field 0x2d1 in Character::blendAnimation");
	return true;
}

TeVector3f32 Character::correctPosition(const TeVector3f32 &pos) {
	bool flag;
	TeVector3f32 result = _freeMoveZone->correctCharacterPosition(pos, &flag, true);
	if (!flag)
		result = _model->position();
	return result;
}

float Character::curveOffset() {
	return _curveOffset;
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
	}
	_model->removeAnim();
	_curModelAnim.release();
}

void Character::deleteCallback(const Common::String &str1, const Common::String &str2, float f) {
	error("TODO: Implement Character::deleteCallback");
}

//static bool deserialize(TiXmlElement *param_1, Walk *param_2);
void Character::endMove() {
	if (_model->name() == "Kate")
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
	const Common::String animFile = _model->anim()->_loadedPath.getLastComponent().toString();
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
	_model->_texturePath = Common::Path("models/Textures");
	_model->_enableLights = true;
	Common::Path modelPath("models");
	modelPath.joinInPlace(_characterSettings._modelFileName);
	if (!_model->load(modelPath))
		return false;

	_model->setName(mname);
	_model->setScale(_characterSettings._defaultScale);

	for (auto &mesh : _model->_meshes)
		mesh.setVisible(true);

	_model->setVisibleByName("_B_", false);
	_model->setVisibleByName("_Y_", false);

	// Note: game loops through "faces" here, but it only ever uses the default ones.
	_model->setVisibleByName(_characterSettings._defaultEyes, true);
	_model->setVisibleByName(_characterSettings._defaultMouth, true);
	_model->setVisibleByName(_characterSettings._defaultBody, true);

	setAnimation(_characterSettings._walkFileName, true);

	_walkPart0AnimLen = animLengthFromFile(walkAnim(WalkPart_Start), &_walkPart0AnimFrameCount);
	_walkPart3AnimLen = animLengthFromFile(walkAnim(WalkPart_EndG), &_walkPart3AnimFrameCount);
	_walkPart1AnimLen = animLengthFromFile(walkAnim(WalkPart_Loop), &_walkPart1AnimFrameCount);

	TeIntrusivePtr<Te3DTexture> shadow = new Te3DTexture();
	shadow->load("models/Textures/simple_shadow_alpha.tga");

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
		pmodel->setQuad(shadow, arr, TeColor(0xff,0xff,0xff,0x50));
	}
	return true;
}

/*static*/ bool Character::loadSettings(const Common::String &path) {
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

	return false;
}

bool Character::onBonesUpdate(const Common::String &boneName, TeMatrix4x4 &boneMatrix) {
	if (!_model || !_model->anim())
		return false;

	Game *game = g_engine->getGame();
	if (boneName == "Pere") {
		const Common::String animfile = _model->anim()->_loadedPath.getLastComponent().toString();
		bool resetX = false;
		if (game->scene()._character == this) {
			for (const auto &walkSettings : _characterSettings._walkSettings) {
				resetX |= (walkSettings._key.contains("Walk") || walkSettings._key.contains("Jog"));
				resetX |= (walkSettings._value._walkParts[0]._file == animfile ||
						walkSettings._value._walkParts[1]._file == animfile ||
						walkSettings._value._walkParts[2]._file == animfile ||
						walkSettings._value._walkParts[3]._file == animfile);
			}
			resetX |= animfile.contains(_characterSettings._walkFileName);
		} else {
			resetX = (animfile.contains(_characterSettings._walkFileName) ||
					  animfile.contains(walkAnim(WalkPart_Start)) ||
					  animfile.contains(walkAnim(WalkPart_Loop)) ||
					  animfile.contains(walkAnim(WalkPart_EndD)) ||
					  animfile.contains(walkAnim(WalkPart_EndG)));
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

			_headRotation.setX(_lastHeadRotation.getX());
			_headRotation.setY(_lastHeadRotation.getY());

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
		_shadowModel[1]->setPosition(pos);
		_shadowModel[1]->setRotation(_model->rotation());
		_shadowModel[1]->setScale(_model->scale());
	}

	// Move any objects attached to the bone
	for (Object3D *obj : game->scene().object3Ds()) {
		if (obj->_onCharName == _model->name() && boneName == obj->_onCharBone) {
			obj->model()->setVisible(true);
			TeMatrix4x4 objmatrix = boneMatrix;
			objmatrix.scale(obj->_objScale);
			objmatrix.rotate(obj->_objRotation);
			objmatrix.scale(obj->_objScale);
			objmatrix.translate(obj->_objTranslation);
			obj->model()->forceMatrix(objmatrix);
			obj->model()->setPosition(_model->position());
			obj->model()->setRotation(_model->rotation());
			obj->model()->setScale(_model->scale());
		}
	}

	return true;
}

bool Character::onModelAnimationFinished() {
	const Common::Path &loadedPath = _model->anim()->_loadedPath;
	const Common::String animfile = loadedPath.getLastComponent().toString();

	// TODO: Do something with _unrecalAnims here.

	Game *game = g_engine->getGame();
	bool isWalkAnim = false;
	if (game->scene()._character == this) {
		// TODO: check if this logic matches..
		for (const auto &walkSettings : _characterSettings._walkSettings) {
			isWalkAnim |= (walkSettings._key.contains("Walk") || walkSettings._key.contains("Jog"));
			isWalkAnim |= (walkSettings._value._walkParts[0]._file == animfile ||
					walkSettings._value._walkParts[1]._file == animfile ||
					walkSettings._value._walkParts[2]._file == animfile ||
					walkSettings._value._walkParts[3]._file == animfile);
		}
		isWalkAnim |= animfile.contains(_characterSettings._walkFileName);
	} else {
		isWalkAnim = (animfile.contains(_characterSettings._walkFileName) ||
				  animfile.contains(walkAnim(WalkPart_Start)) ||
				  animfile.contains(walkAnim(WalkPart_Loop)) ||
				  animfile.contains(walkAnim(WalkPart_EndD)) ||
				  animfile.contains(walkAnim(WalkPart_EndG)));
	}

	if (isWalkAnim) {
		int pereBone = _curModelAnim->findBone("Pere");
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

	if (_someRepeatFlag && loadedPath.toString().contains(_setAnimName)) {
		_notWalkAnim = false;
		_someRepeatFlag = false;
		setAnimation(_characterSettings._walkFileName, true);
	}

	return false;
}

void Character::permanentUpdate() {
	error("TODO: Implement Character::permanentUpdate.");
}

void Character::placeOnCurve(TeIntrusivePtr<TeBezierCurve> &curve) {
	_curve = curve;
	updatePosition(_curveOffset);
}

void Character::removeAnim() {
	if (_curModelAnim) {
		_curModelAnim->onFinished().remove(this, &Character::onModelAnimationFinished);
		_curModelAnim->unbind();
	}
	_model->removeAnim();
	if (_curModelAnim) {
		_curModelAnim.release();
	}
}

void Character::removeFromCurve() {
	_curve.release();
}

bool Character::setAnimation(const Common::String &aname, bool repeat, bool param_3, bool unused, int startFrame, int endFrame) {
	if (aname.empty())
		return false;

	Common::Path animPath("models/Anims");
	animPath.joinInPlace(aname);
	bool isWalkAnim = (aname.contains(_characterSettings._walkFileName) ||
					  aname.contains(walkAnim(WalkPart_Start)) ||
					  aname.contains(walkAnim(WalkPart_Loop)) ||
					  aname.contains(walkAnim(WalkPart_EndD)) ||
					  aname.contains(walkAnim(WalkPart_EndG)));
	_notWalkAnim = !isWalkAnim;

	if (_curModelAnim) {
		_curModelAnim->onFinished().remove(this, &Character::onModelAnimationFinished);
		_curModelAnim->unbind();
	}

	_curModelAnim = animCacheLoad(animPath);
	_curModelAnim->onFinished().add(this, &Character::onModelAnimationFinished);
	_curModelAnim->bind(_model);
	_curModelAnim->setFrameLimits(startFrame, endFrame);
	_model->setAnim(_curModelAnim, repeat);
	_lastFrame = -1;
	_curModelAnim->play();
	_setAnimName = aname;
	_curAnimName = aname;
	_someRepeatFlag = !(repeat | !param_3);

	return true;
}

void Character::setAnimationSound(const Common::String &sname, uint offset) {
	warning("TODO: Set field 0x2f8 to 0 in Character::setAnimationSound.");
	_animSound = sname;
	_animSoundOffset = offset;
}

void Character::setCurveOffset(float offset) {
	_curveOffset = offset;
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
	_shadowModel[0]->setVisible(visible);
	_shadowModel[1]->setVisible(visible);
	return false;
}

float Character::speedFromAnim(double movepercent) {
	error("TODO: Implement Character::speedFromAnim");
	return 0;
}

float Character::translationFromAnim(const TeModelAnimation &anim, long bone, long param_3) {
	return translationVectorFromAnim(anim, bone, param_3).z();
}

TeVector3f32 Character::translationVectorFromAnim(const TeModelAnimation &anim, long bone, long frame) {
	const TeTRS trs = trsFromAnim(anim, bone, frame);
	return trs.getTranslation();
}

TeTRS Character::trsFromAnim(const TeModelAnimation &anim, long bone, long frame) {
	if (bone == -1)
		return TeTRS();

	return anim.getTRS(bone, frame, false);
}

void Character::update(double percentval) {
	if (!_curve || !_runTimer.running())
		return;
	//float speed = speedFromAnim(percentval);

	error("TODO: Implement Character::update");
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
	_walkPart0AnimLen = animLengthFromFile(walkAnim(WalkPart_Start), &_walkPart0AnimFrameCount, 9999);
	_walkPart3AnimLen = animLengthFromFile(walkAnim(WalkPart_EndG), &_walkPart3AnimFrameCount, 9999);
	_walkPart1AnimLen = animLengthFromFile(walkAnim(WalkPart_Loop), &_walkPart1AnimFrameCount, 9999);
}

void Character::walkTo(float curveEnd, bool walkFlag) {
	error("TODO: Implement Character::walkTo");
}

} // end namespace Tetraedge
