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

#include "tetraedge/game/character.h"
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
_missingCurrentAnim(false), _someRepeatFlag(false), _walkModeStr("Walk"), _needsSomeUpdate(false),
_stepSound1("sounds/SFX/PAS_H_BOIS1.ogg"), _stepSound2("sounds/SFX/PAS_H_BOIS2.ogg"),
_freeMoveZone(nullptr) {
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
	modelAnim->load(path);

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

	// TODO: Check this maths.. is this really what it does?
	return animLen * _model->scale().z();
}

bool Character::blendAnimation(const Common::String &animname, float param_2, bool param_3, bool param_4) {
	error("TODO: Implement Character::blendAnimation");
}

TeVector3f32 Character::correctPosition(const TeVector3f32 &pos) {
	error("TODO: Implement Character::correctPosition");
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
	error("TODO: Implement Character::deleteAnim");
}

void Character::deleteCallback(const Common::String &str1, const Common::String &str2, float f) {
	error("TODO: Implement Character::deleteCallback");
}

//static bool deserialize(TiXmlElement *param_1, Walk *param_2);
void Character::endMove() {
	error("TODO: Implement Character::endMove.");
}

const Character::WalkSettings *Character::getCurrentWalkFiles() {
	for (const auto & walkSettings : _characterSettings._walkSettings) {
		if (walkSettings._key == _walkModeStr)
			return &walkSettings._value;
	}
	return nullptr;
}

bool Character::isFramePassed(uint frameno) {
	error("TODO: Implement Character::isFramePassed.");
}

bool Character::isWalkEnd() {
	error("TODO: Implement Character::isWalkEnd.");
	return false;
}

bool Character::leftStepFrame(enum Character::WalkPart walkpart) {
	error("TODO: Implement Character::leftStepFrame.");
	return false;
}

bool Character::rightStepFrame(enum Character::WalkPart walkpart) {
	error("TODO: Implement Character::rightStepFrame.");
	return false;
}

bool Character::loadModel(const Common::String &name, bool param_2) {
	assert(_globalCharacterSettings);
	if (_model) {
		//_model->bonesUpdateSignal().remove(this, &Character::onBonesUpdate);
	}
	_model = new TeModel();
	//_model->bonesUpdateSignal().add(this, &Character::onBonesUpdate);

	if (!_globalCharacterSettings->contains(name))
		return false;

	_characterSettings = _globalCharacterSettings->getVal(name);
	_model->_texturePath = Common::Path("models/Textures");
	_model->_enableLights = true;
	Common::Path modelPath("models");
	modelPath.joinInPlace(_characterSettings._modelFileName);
	if (!_model->load(modelPath))
		return false;

	_model->setName(name);
	_model->setScale(_characterSettings._defaultScale);

	for (unsigned int i = 0; i < _model->_meshes.size(); i++)
		_model->_meshes[i].setVisible(true);

	_model->setVisibleByName("_B_", false);
	_model->setVisibleByName("_Y_", false);

	_model->setVisibleByName(_characterSettings._defaultEyes, true);
	_model->setVisibleByName(_characterSettings._defaultMouth, true);

	setAnimation(_characterSettings._walkFileName, true, false, false, -1, 9999);

	_walkPart0AnimLen = animLengthFromFile(walkAnim(WalkPart_Start), &_walkPart0AnimFrameCount, 9999);
	_walkPart3AnimLen = animLengthFromFile(walkAnim(WalkPart_EndG), &_walkPart3AnimFrameCount, 9999);
	_walkPart1AnimLen = animLengthFromFile(walkAnim(WalkPart_Loop), &_walkPart1AnimFrameCount, 9999);

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
	const uint32 bufsize = xmlFile.size();
	char *buf = new char[bufsize+1];
	buf[bufsize] = '\0';
	xmlFile.read(buf, bufsize);
	Common::String fixedbuf(buf);
	uint32 offset = fixedbuf.find("------------");
	while (offset != Common::String::npos) {
		fixedbuf.replace(offset, 12, "--");
		offset = fixedbuf.find("------------");
	}

	// Big HACK: Remove the embedded comment in this config.
	offset = fixedbuf.find("<!--<walk>");
	if (offset != Common::String::npos) {
		uint32 endOffset = fixedbuf.find(" -->", offset);
		if (endOffset != Common::String::npos) {
			uint32 realEndOffset = fixedbuf.find("walk>-->", endOffset);
			if (realEndOffset  != Common::String::npos && realEndOffset > endOffset) {
				fixedbuf.replace(offset, endOffset - offset, "<!-- ");
			}
		}
	}

	if (!parser.loadBuffer((unsigned char *)fixedbuf.c_str(), bufsize))
		error("Character::loadSettings: Can't open %s", path.c_str());

	if (!parser.parse())
		error("Character::loadSettings: Can't parse %s", path.c_str());

	return false;
}

bool Character::onBonesUpdate(const Common::String &param_1, const TeMatrix4x4 *param_2) {
	error("TODO: Implement Character::onBonesUpdate");
	return false;
}

bool Character::onModelAnimationFinished() {
	error("TODO: Implement Character::onModelAnimationFinished");
	return false;
}

void Character::permanentUpdate() {
	error("TODO: Implement Character::permanentUpdate.");
}

void Character::placeOnCurve(const TeBezierCurve &curve) {
	_curve = curve;
	updatePosition(_curveOffset);
}

void Character::removeAnim() {
	if (_curModelAnim) {
		_curModelAnim->onFinished().remove(this, &Character::onModelAnimationFinished);
	}
	_model->removeAnim();
	if (_curModelAnim) {
		_curModelAnim.release();
	}
}

void Character::removeFromCurve() {
	error("TODO: Implement Character::removeFromCurve.");
}

bool Character::setAnimation(const Common::String &name, bool repeat, bool param_3, bool unused, int startFrame, int endFrame)  {
	if (name.empty())
		return false;

	Common::Path animPath("models/Anims");
	animPath.joinInPlace(name);
	bool validAnim = (name.contains(_characterSettings._walkFileName) ||
					  name.contains(walkAnim(WalkPart_Start)) ||
					  name.contains(walkAnim(WalkPart_Loop)) ||
					  name.contains(walkAnim(WalkPart_EndD)) ||
					  name.contains(walkAnim(WalkPart_EndG)));
	_missingCurrentAnim = !validAnim;

	if (_curModelAnim) {
		_curModelAnim->onFinished().remove(this, &Character::onModelAnimationFinished);
		_curModelAnim->unbind();
	}

	_curModelAnim = animCacheLoad(animPath);
	_curModelAnim->bind(_model);
	_curModelAnim->setFrameLimits(startFrame, endFrame);
	_model->setAnim(_curModelAnim, repeat);
	_lastFrame = -1;
	_curModelAnim->play();
	_setAnimName = name;
	_curAnimName = name;
	_someRepeatFlag = !(repeat | !param_3);

	return true;
}

void Character::setAnimationSound(const Common::String &name, uint param_2)  {
	error("TODO: Implement Character::setAnimationSound.");
}

void Character::setCurveOffset(float offset) {
	_curveOffset = offset;
	updatePosition(offset);
}

void Character::setFreeMoveZone(const Common::SharedPtr<TeFreeMoveZone> &zone) {
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

TeVector3f32 Character::translationVectorFromAnim(const TeModelAnimation &anim, long bone, long frame)  {
	const TeTRS trs = trsFromAnim(anim, bone, frame);
	return trs.getTranslation();
}

TeTRS Character::trsFromAnim(const TeModelAnimation &anim, long bone, long frame)  {
	if (bone == -1)
		return TeTRS();

	return anim.getTRS(bone, frame, false);
}

void Character::update(double percentval)  {
	error("TODO: Implement Character::update");
}

void Character::updateAnimFrame()  {
	error("TODO: Implement Character::updateAnimFrame");
}

void Character::updatePosition(float curveOffset) {
	error("TODO: Implement Character::updatePosition");
}

Common::String Character::walkAnim(Character::WalkPart part)  {
	Common::String result;
	const Character::WalkSettings *settings = getCurrentWalkFiles();
	if (settings) {
		return settings->_walkParts[(int)part]._file;
	}
	return result;
}

void Character::walkMode(const Common::String &mode) {
	error("TODO: Implement Character::walkMode");
}

void Character::walkTo(float param_1, bool param_2) {
	error("TODO: Implement Character::walkTo");
}

} // end namespace Tetraedge
