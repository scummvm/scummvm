
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

#include "common/config-manager.h"

#include "twp/twp.h"
#include "twp/motor.h"
#include "twp/object.h"
#include "twp/scenegraph.h"
#include "twp/squtil.h"

namespace Twp {

OffsetTo::~OffsetTo() {}

OffsetTo::OffsetTo(float duration, Object *obj, Math::Vector2d pos, InterpolationMethod im)
	: _obj(obj),
	  _tween(obj->_node->getOffset(), pos, duration, im) {
}

void OffsetTo::update(float elapsed) {
	_tween.update(elapsed);
	_obj->_node->setOffset(_tween.current());
	if (!_tween.running())
		disable();
}

MoveTo::~MoveTo() {}

MoveTo::MoveTo(float duration, Object *obj, Math::Vector2d pos, InterpolationMethod im)
	: _obj(obj),
	  _tween(obj->_node->getPos(), pos, duration, im) {
}

void MoveTo::update(float elapsed) {
	_tween.update(elapsed);
	_obj->_node->setPos(_tween.current());
	if (!_tween.running())
		disable();
}

AlphaTo::~AlphaTo() {}

AlphaTo::AlphaTo(float duration, Object *obj, float to, InterpolationMethod im)
	: _obj(obj),
	  _tween(obj->_node->getAlpha(), to, duration, im) {
}

void AlphaTo::update(float elapsed) {
	_tween.update(elapsed);
	float alpha = _tween.current();
	_obj->_node->setAlpha(alpha);
	if (!_tween.running())
		disable();
}

RotateTo::~RotateTo() {}

RotateTo::RotateTo(float duration, Node *node, float to, InterpolationMethod im)
	: _node(node),
	  _tween(node->getRotation(), to, duration, im) {
}

void RotateTo::update(float elapsed) {
	_tween.update(elapsed);
	_node->setRotation(_tween.current());
	if (!_tween.running())
		disable();
}

ScaleTo::~ScaleTo() {}

ScaleTo::ScaleTo(float duration, Node *node, float to, InterpolationMethod im)
	: _node(node),
	  _tween(node->getScale().getX(), to, duration, im) {
}

void ScaleTo::update(float elapsed) {
	_tween.update(elapsed);
	float x = _tween.current();
	_node->setScale(Math::Vector2d(x, x));
	if (!_tween.running())
		disable();
}

Shake::~Shake() {}

Shake::Shake(Node *node, float amount)
	: _node(node),
	  _amount(amount) {
}

void Shake::update(float elapsed) {
	_shakeTime += 40.f * elapsed;
	_elapsed += elapsed;
	_node->setShakeOffset(Math::Vector2d(_amount * cos(_shakeTime + 0.3f), _amount * sin(_shakeTime)));
}

OverlayTo::OverlayTo(float duration, Room *room, Color to)
	: _room(room),
	  _to(to),
	  _tween(g_engine->_room->getOverlay(), to, duration, InterpolationMethod()) {
}

OverlayTo::~OverlayTo() {}

void OverlayTo::update(float elapsed) {
	_tween.update(elapsed);
	_room->setOverlay(_tween.current());
	if (!_tween.running())
		disable();
}

ReachAnim::ReachAnim(Object *actor, Object *obj)
	: _actor(actor), _obj(obj) {
}

ReachAnim::~ReachAnim() {
}

void ReachAnim::playReachAnim() {
	Common::String anim = _actor->getAnimName(REACH_ANIMNAME + _obj->getReachAnim());
	_actor->play(anim);
}

void ReachAnim::update(float elapsed) {
	switch (_state) {
	case 0:
		playReachAnim();
		_state = 1;
		break;
	case 1:
		_elapsed += elapsed;
		if (_elapsed > 0.5)
			_state = 2;
		break;
	case 2:
		_actor->stand();
		_actor->execVerb();
		disable();
		_state = 3;
		break;
	default:
		break;
	}
}

WalkTo::WalkTo(Object *obj, Math::Vector2d dest, int facing)
	: _obj(obj), _facing(facing) {
	if (obj->_useWalkboxes) {
		_path = obj->_room->calculatePath(obj->_node->getAbsPos(), dest);
	} else {
		_path = {obj->_node->getAbsPos(), dest};
	}
	_wsd = sqrt(obj->_walkSpeed.getX() * obj->_walkSpeed.getX() + obj->_walkSpeed.getY() * obj->_walkSpeed.getY());
	if (sqrawexists(obj->_table, "preWalking"))
		sqcall(obj->_table, "preWalking");
}

void WalkTo::disable() {
	Motor::disable();
	if (_path.size() != 0) {
		debug("actor walk cancelled");
	}
	if (_obj->isWalking())
		_obj->play("stand");
}

static bool needsReachAnim(int verbId) {
	return (verbId == VERB_PICKUP) || (verbId == VERB_OPEN) || (verbId == VERB_CLOSE) || (verbId == VERB_PUSH) || (verbId == VERB_PULL) || (verbId == VERB_USE);
}

void WalkTo::actorArrived() {
	bool needsReach = _obj->_exec.enabled && needsReachAnim(_obj->_exec.verb.id);
	if (!needsReach)
		disable();

	debug("actorArrived");
	_obj->play("stand");
	// the faces to the specified direction (if any)
	if (_facing) {
		debug("actor arrived with facing %d", _facing);
		_obj->setFacing((Facing)_facing);
	}

	// call `actorArrived` callback
	if (sqrawexists(_obj->_table, "actorArrived")) {
		debug("call actorArrived callback");
		sqcall(_obj->_table, "actorArrived");
	}

	// we need to execute a sentence when arrived ?
	if (_obj->_exec.enabled) {
		VerbId verb = _obj->_exec.verb;
		Object *noun1 = _obj->_exec.noun1;
		Object *noun2 = _obj->_exec.noun2;
		// call `postWalk`callback
		Common::String funcName = isActor(noun1->getId()) ? "actorPostWalk" : "objectPostWalk";
		if (sqrawexists(_obj->_table, funcName)) {
			debug("call %s callback", funcName.c_str());
			HSQOBJECT n2Table;
			if (noun2)
				n2Table = noun2->_table;
			else
				sq_resetobject(&n2Table);
			sqcall(_obj->_table, funcName.c_str(), verb.id, noun1->_table, n2Table);
		}

		if (needsReach)
			_obj->setReach(new ReachAnim(_obj, noun1));
		else
			_obj->execVerb();
	}
}

void WalkTo::update(float elapsed) {
	if (_path.size() != 0) {
		Math::Vector2d dest = _path[0];
		float d = distance(dest, _obj->_node->getAbsPos());

		// arrived at destination ?
		if (d < 1.0) {
			_obj->_node->setPos(_path[0]);
			_path.remove_at(0);
			if (_path.size() == 0) {
				actorArrived();
			}
		} else {
			Math::Vector2d delta = dest - _obj->_node->getAbsPos();
			float duration = d / _wsd;
			float factor = Twp::clamp(elapsed / duration, 0.f, 1.f);

			Math::Vector2d dd = delta * factor;
			_obj->_node->setPos(_obj->_node->getPos() + dd);
			if (abs(delta.getX()) >= abs(delta.getY())) {
				_obj->setFacing(delta.getX() >= 0 ? FACE_RIGHT : FACE_LEFT);
			} else {
				_obj->setFacing(delta.getY() > 0 ? FACE_BACK : FACE_FRONT);
			}
		}
	}

	Motor *reach = _obj->getReach();
	if (reach && reach->isEnabled()) {
		reach->update(elapsed);
		if (!reach->isEnabled())
			disable();
	}
}

Talking::Talking(Object *obj, const Common::StringArray &texts, Color color) {
	_obj = obj;
	_color = color;
	_texts.assign(texts.begin() + 1, texts.end());
	say(texts[0]);
}

static int letterToIndex(char c) {
	switch (c) {
	case 'A':
		return 1;
	case 'B':
		return 2;
	case 'C':
		return 3;
	case 'D':
		return 4;
	case 'E':
		return 5;
	case 'F':
		return 6;
	case 'G':
		return 1;
	case 'H':
		return 4;
	case 'X':
		return 1;
	default:
		error("unknown letter %c", c);
	}
}

void Talking::update(float elapsed) {
	if (isEnabled()) {
		_elapsed += elapsed;
		if (_elapsed < _duration) {
			char letter = _lip.letter(_elapsed);
			_obj->setHeadIndex(letterToIndex(letter));
		} else {
			if (_texts.size() > 0) {
				say(_texts[0]);
				_texts.remove_at(0);
			} else {
				disable();
			}
		}
	}
}

int Talking::loadActorSpeech(const Common::String &name) {
	if(!ConfMan.getBool("talkiesHearVoice")) return 0;

	debug("loadActorSpeech %s.ogg", name.c_str());
	Common::String filename(name);
	filename.toUppercase();
	filename += ".ogg";
	if (g_engine->_pack.assetExists(filename.c_str())) {
		SoundDefinition *soundDefinition = new SoundDefinition(filename);
		if (!soundDefinition) {
			debug("File %s.ogg not found", name.c_str());
		} else {
			g_engine->_audio._soundDefs.push_back(soundDefinition);
			return g_engine->_audio.play(soundDefinition, Audio::Mixer::SoundType::kSpeechSoundType, 0, 0, 1.f, _obj->getId());
		}
	}
	return 0;
}

void Talking::say(const Common::String &text) {
	Common::String txt(text);
	if (text[0] == '@') {
		int id = atoi(text.c_str() + 1);
		txt = g_engine->_textDb.getText(id);

		id = onTalkieId(id);
		Common::String key = talkieKey();
		key.toUppercase();
		Common::String name = Common::String::format("%s_%d", key.c_str(), id);
		Common::String path = name + ".lip";

		debug("Load lip %s", path.c_str());
		if (g_engine->_pack.assetExists(path.c_str())) {
			GGPackEntryReader entry;
			entry.open(g_engine->_pack, path);
			_lip.load(&entry);
			debug("Lip %s loaded", path.c_str());
		}

		// TODO: call sayingLine
		if(_obj->_sound) {
			g_engine->_audio.stop(_obj->_sound);
		}
		_obj->_sound = loadActorSpeech(name);
	} else if (text[0] == '^') {
		txt = text.substr(1);
	}

	// remove text in parenthesis
	if (txt[0] == '(') {
		int i = txt.find(')');
		if (i != -1)
			txt = txt.substr(i + 1);
	}

	debug("sayLine '%s'", txt.c_str());

	// modify state ?
	Common::String state;
	if (txt[0] == '{') {
		int i = txt.find('}');
		if (i != -1) {
			state = txt.substr(1, txt.size() - 2);
			debug("Set state from anim '%s'", state.c_str());
			if (state != "notalk") {
				_obj->play(state);
			}
			txt = txt.substr(i + 1);
		}
	}

	setDuration(txt);

	if (_obj->_sayNode) {
		_obj->_sayNode->remove();
	}
	Text text2("sayline", txt, thCenter, tvCenter, SCREEN_WIDTH * 3.f / 4.f, _color);
	_obj->_sayNode = new TextNode();
	((TextNode *)_obj->_sayNode)->setText(text2);
	_obj->_sayNode->setColor(_color);
	_node = _obj->_sayNode;
	Math::Vector2d pos = g_engine->roomToScreen(_obj->_node->getAbsPos() + Math::Vector2d(_obj->_talkOffset.getX(), _obj->_talkOffset.getY()));

	// clamp position to keep it on screen
	pos.setX(Twp::clamp(pos.getX(), 10.f + text2.getBounds().getX() / 2.f, SCREEN_WIDTH - text2.getBounds().getX() / 2.f));
	pos.setY(Twp::clamp(pos.getY(), 10.f + text2.getBounds().getY(), SCREEN_HEIGHT - text2.getBounds().getY()));

	_obj->_sayNode->setPos(pos);
	_obj->_sayNode->setAnchorNorm(Math::Vector2d(0.5f, 0.5f));
	g_engine->_screenScene.addChild(_obj->_sayNode);
}

void Talking::disable() {
	Motor::disable();
	_texts.clear();
	_obj->setHeadIndex(1);
	_node->remove();
}

int Talking::onTalkieId(int id) {
	int result = 0;
	sqcallfunc(result, "onTalkieID", _obj->_table, id);
	if (result == 0)
		result = id;
	return result;
}

void Talking::setDuration(const Common::String &text) {
	_elapsed = 0;
	// let sayLineBaseTime = prefs(SayLineBaseTime);
	float sayLineBaseTime = 1.5f;
	//   let sayLineCharTime = prefs(SayLineCharTime);
	float sayLineCharTime = 0.025f;
	// let sayLineMinTime = prefs(SayLineMinTime);
	float sayLineMinTime = 0.2f;
	//   let sayLineSpeed = prefs(SayLineSpeed);
	float sayLineSpeed = 0.5f;
	float duration = (sayLineBaseTime + sayLineCharTime * text.size()) / (0.2f + sayLineSpeed);
	_duration = MAX(duration, sayLineMinTime);
}

Common::String Talking::talkieKey() {
	Common::String result;
	if (sqrawexists(_obj->_table, "_talkieKey"))
		sqgetf(_obj->_table, "_talkieKey", result);
	else
		sqgetf(_obj->_table, "_key", result);
	return result;
}

} // namespace Twp
