
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
#include "twp/detection.h"
#include "twp/object.h"
#include "twp/resmanager.h"
#include "twp/room.h"
#include "twp/squtil.h"
#include "twp/tsv.h"

namespace Twp {

void Motor::update(float elapsed) {
	if (!isEnabled())
		return;
	onUpdate(elapsed);
}

OffsetTo::~OffsetTo() = default;

OffsetTo::OffsetTo(float duration, Common::SharedPtr<Object> obj, const Math::Vector2d &pos, InterpolationMethod im)
	: _obj(obj),
	  _tween(obj->_node->getOffset(), pos, duration, im) {
}

void OffsetTo::onUpdate(float elapsed) {
	_tween.update(elapsed);
	_obj->_node->setOffset(_tween.current());
	if (!_tween.running())
		disable();
}

MoveTo::~MoveTo() = default;

MoveTo::MoveTo(float duration, Common::SharedPtr<Object> obj, const Math::Vector2d &pos, InterpolationMethod im)
	: _obj(obj),
	  _tween(obj->_node->getPos(), pos, duration, im) {
}

void MoveTo::onUpdate(float elapsed) {
	_tween.update(elapsed);
	_obj->_node->setPos(_tween.current());
	if (!_tween.running())
		disable();
}

AlphaTo::~AlphaTo() = default;

AlphaTo::AlphaTo(float duration, Common::SharedPtr<Object> obj, float to, InterpolationMethod im)
	: _obj(obj),
	  _tween(obj->_node->getAlpha(), to, duration, im) {
}

void AlphaTo::onUpdate(float elapsed) {
	_tween.update(elapsed);
	float alpha = _tween.current();
	_obj->_node->setAlpha(alpha);
	if (!_tween.running())
		disable();
}

RotateTo::~RotateTo() = default;

RotateTo::RotateTo(float duration, Node *node, float to, InterpolationMethod im)
	: _node(node),
	  _tween(node->getRotation(), to, duration, im) {
}

void RotateTo::onUpdate(float elapsed) {
	_tween.update(elapsed);
	_node->setRotation(_tween.current());
	if (!_tween.running())
		disable();
}

RoomRotateTo::~RoomRotateTo() = default;

RoomRotateTo::RoomRotateTo(Common::SharedPtr<Room> room, float to)
	: _room(room),
	  _tween(room->_rotation, to, 0.200f, intToInterpolationMethod(0)) {
}

void RoomRotateTo::onUpdate(float elapsed) {
	_tween.update(elapsed);
	_room->_rotation = _tween.current();
	if (!_tween.running())
		disable();
}

ScaleTo::~ScaleTo() = default;

ScaleTo::ScaleTo(float duration, Node *node, float to, InterpolationMethod im)
	: _node(node),
	  _tween(node->getScale().getX(), to, duration, im) {
}

void ScaleTo::onUpdate(float elapsed) {
	_tween.update(elapsed);
	float x = _tween.current();
	_node->setScale(Math::Vector2d(x, x));
	if (!_tween.running())
		disable();
}

Shake::~Shake() = default;

Shake::Shake(Node *node, float amount)
	: _node(node),
	  _amount(amount) {
}

void Shake::onUpdate(float elapsed) {
	_shakeTime += 40.f * elapsed;
	_elapsed += elapsed;
	_node->setShakeOffset(Math::Vector2d(_amount * cos(_shakeTime + 0.3f), _amount * sin(_shakeTime)));
}

OverlayTo::OverlayTo(float duration, Common::SharedPtr<Room> room, const Color &to)
	: _room(room),
	  _to(to),
	  _tween(g_twp->_room->getOverlay(), to, duration, InterpolationMethod()) {
}

OverlayTo::~OverlayTo() = default;

void OverlayTo::onUpdate(float elapsed) {
	_tween.update(elapsed);
	_room->setOverlay(_tween.current());
	if (!_tween.running())
		disable();
}

ReachAnim::ReachAnim(Common::SharedPtr<Object> actor, Common::SharedPtr<Object> obj)
	: _actor(actor), _obj(obj) {
}

ReachAnim::~ReachAnim() = default;

void ReachAnim::playReachAnim() {
	Common::String anim = _actor->getAnimName(REACH_ANIMNAME + _obj->getReachAnim());
	_actor->play(anim);
}

void ReachAnim::onUpdate(float elapsed) {
	switch (_state) {
	case 0:
		playReachAnim();
		_state = 1;
		break;
	case 1:
		_elapsed += elapsed;
		if (_elapsed > 0.10)
			_state = 2;
		break;
	case 2:
		_actor->stand();
		Object::execVerb(_actor);
		disable();
		_state = 3;
		break;
	default:
		break;
	}
}

WalkTo::WalkTo(Common::SharedPtr<Object> obj, const Math::Vector2d &dest, int facing)
	: _obj(obj), _facing(facing) {
	if (obj->_useWalkboxes) {
		_path = obj->_room->calculatePath(obj->_node->getAbsPos(), dest);
	} else {
		_path = {obj->_node->getAbsPos(), dest};
	}

	// don't know yet why walkspeed is so slow, so I cheat
	Math::Vector2d walkSpeed = obj->_walkSpeed * 2;
	_wsd = sqrt(walkSpeed.getX() * walkSpeed.getX() + walkSpeed.getY() * walkSpeed.getY());
	if (sqrawexists(obj->_table, "preWalking"))
		sqcall(obj->_table, "preWalking");
}

void WalkTo::disable() {
	Motor::disable();
	if (!_path.empty()) {
		debugC(kDebugGame, "actor walk cancelled");
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

	debugC(kDebugGame, "actorArrived");
	_obj->play("stand");
	// the faces to the specified direction (if any)
	if (_facing) {
		debugC(kDebugGame, "actor arrived with facing %d", _facing);
		_obj->setFacing((Facing)_facing);
	}

	// call `actorArrived` callback
	if (sqrawexists(_obj->_table, "actorArrived")) {
		debugC(kDebugGame, "call actorArrived callback");
		sqcall(_obj->_table, "actorArrived");
	}

	// we need to execute a sentence when arrived ?
	if (_obj->_exec.enabled) {
		VerbId verb = _obj->_exec.verb;
		Common::SharedPtr<Object> noun1 = _obj->_exec.noun1;
		Common::SharedPtr<Object> noun2 = _obj->_exec.noun2;
		// call `postWalk`callback
		Common::String funcName = g_twp->_resManager->isActor(noun1->getId()) ? "actorPostWalk" : "objectPostWalk";
		if (sqrawexists(_obj->_table, funcName)) {
			debugC(kDebugGame, "call %s callback", funcName.c_str());
			HSQOBJECT n2Table;
			if (noun2)
				n2Table = noun2->_table;
			else
				sq_resetobject(&n2Table);
			sqcall(_obj->_table, funcName.c_str(), (SQInteger)verb.id, noun1->_table, n2Table);
		}

		if (needsReach)
			_obj->setReach(Common::SharedPtr<ReachAnim>(new ReachAnim(_obj, noun1)));
		else
			Object::execVerb(_obj);
	}
}

void WalkTo::onUpdate(float elapsed) {
	if (!_enabled)
		return;
	if (_state == kWalking && !_path.empty()) {
		Math::Vector2d dest = _path[0];
		float d = distance(dest, _obj->_node->getAbsPos());

		// arrived at destination ?
		if (d < 1.0) {
			_obj->_node->setPos((Math::Vector2d)_path[0]);
			_path.remove_at(0);
			if (_path.empty()) {
				_state = kArrived;
				actorArrived();
				return;
			}
		} else {
			Math::Vector2d delta(dest - _obj->_node->getAbsPos());
			float duration = d / _wsd;
			float factor = CLIP(elapsed / duration, 0.f, 1.f);

			Math::Vector2d dd = delta * factor;
			_obj->_node->setPos(_obj->_node->getPos() + dd);
			if (abs(delta.getX()) >= abs(delta.getY())) {
				_obj->setFacing(delta.getX() >= 0 ? Facing::FACE_RIGHT : Facing::FACE_LEFT);
			} else {
				_obj->setFacing(delta.getY() > 0 ? Facing::FACE_BACK : Facing::FACE_FRONT);
			}
		}
	}

	if (_state == kArrived) {
		Common::SharedPtr<Motor> reach = _obj->getReach();
		if (reach && reach->isEnabled()) {
			reach->update(elapsed);
			_state = kReach;
			return;
		}
	}

	if (_state == kReach) {
		Common::SharedPtr<Motor> reach = _obj->getReach();
		if (reach) {
			if (reach->isEnabled()) {
				reach->update(elapsed);
			} else {
				disable();
			}
		}
	}
}

TalkingBase::TalkingBase(Common::SharedPtr<Object> actor, float duration)
	: _actor(actor), _duration(duration) {
}

int TalkingBase::loadActorSpeech(const Common::String &name) {
	if (ConfMan.getBool("speech_mute")) {
		debugC(kDebugGame, "talking %s: speech_mute: true", _actor->_key.c_str());
		return 0;
	}

	debugC(kDebugGame, "loadActorSpeech %s.ogg", name.c_str());
	Common::String filename(name);
	filename.toUppercase();
	filename += ".ogg";
	if (g_twp->_pack->assetExists(filename.c_str())) {
		Common::SharedPtr<SoundDefinition> soundDefinition(new SoundDefinition(filename));
		if (!soundDefinition) {
			debugC(kDebugGame, "File %s.ogg not found", name.c_str());
		} else {
			g_twp->_audio->_soundDefs.push_back(soundDefinition);
			int id = g_twp->_audio->play(soundDefinition, Audio::Mixer::SoundType::kSpeechSoundType, 0, 0, 1.f);
			int duration = g_twp->_audio->getDuration(id);
			debugC(kDebugGame, "talking %s audio id: %d, dur: %d", _actor->_key.c_str(), id, duration);
			if (duration)
				_duration = static_cast<float>(duration) / 1000.f;
			return id;
		}
	}
	return 0;
}

int TalkingBase::onTalkieId(int id) {
	SQInteger result = 0;
	sqcallfunc(result, "onTalkieID", _actor->_table, id);
	if (result == 0)
		result = id;
	return result;
}

Common::String TalkingBase::talkieKey() {
	Common::String result;
	if (sqrawexists(_actor->_table, "_talkieKey") && SQ_FAILED(sqgetf(_actor->_table, "_talkieKey", result))) {
		error("Failed to get talkie key");
	}
	if (sqrawexists(_actor->_table, "_key") && SQ_FAILED(sqgetf(_actor->_table, "_key", result))) {
		error("Failed to get talkie key (2)");
	}
	return result;
}

void TalkingBase::setDuration(const Common::String &text) {
	_elapsed = 0;
	// let sayLineBaseTime = prefs(SayLineBaseTime);
	float sayLineBaseTime = 1.5f;
	//   let sayLineCharTime = prefs(SayLineCharTime);
	float sayLineCharTime = 0.025f;
	// let sayLineMinTime = prefs(SayLineMinTime);
	float sayLineMinTime = 0.2f;
	//   let sayLineSpeed = prefs(SayLineSpeed);
	float sayLineSpeed = 0.5f;
	float duration = (sayLineBaseTime + sayLineCharTime * static_cast<float>(text.size())) / (0.2f + sayLineSpeed);
	_duration = MAX(duration, sayLineMinTime);
}

float TalkingBase::getTalkSpeed() const {
	return (_actor && _actor->_sound) ? 1.f : (ConfMan.getInt("talkspeed") + 1) / 60.f;
}

Talking::Talking(Common::SharedPtr<Object> obj, const Common::StringArray &texts, const Color &color) : TalkingBase(obj, 0.f) {
	_color = color;
	_texts.assign(texts.begin() + 1, texts.end());
	say(texts[0]);
}

void Talking::append(const Common::StringArray &texts, const Color &color) {
	_color = color;
	_texts.push_back(texts);
	_enabled = !_texts.empty();
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

void Talking::onUpdate(float elapsed) {
	if (!isEnabled())
		return;

	_elapsed += elapsed * getTalkSpeed();
	if (_actor->_sound) {
		if (!g_twp->_audio->playing(_actor->_sound)) {
			debugC(kDebugGame, "talking %s audio stopped", _actor->_key.c_str());
			_actor->_sound = 0;
		} else {
			float e = static_cast<float>(g_twp->_audio->getElapsed(_actor->_sound)) / 1000.f;
			char letter = _lip.letter(e);
			_actor->setHeadIndex(letterToIndex(letter));
		}
	} else if (_elapsed < _duration) {
		char letter = _lip.letter(_elapsed);
		_actor->setHeadIndex(letterToIndex(letter));
	} else if (!_texts.empty()) {
		debugC(kDebugGame, "talking %s: %s", _actor->_key.c_str(), _texts[0].c_str());
		say(_texts[0]);
		_texts.remove_at(0);
	} else {
		debugC(kDebugGame, "talking %s: ended", _actor->_key.c_str());
		disable();
	}
}

void Talking::say(const Common::String &text) {
	if (text.empty())
		return;

	Common::String txt(text);
	if (txt[0] == '$') {
		HSQUIRRELVM v = g_twp->getVm();
		SQInteger top = sq_gettop(v);
		sq_pushroottable(v);
		Common::String code(Common::String::format("return %s", text.substr(1, text.size() - 1).c_str()));
		if (SQ_FAILED(sq_compilebuffer(v, code.c_str(), code.size(), "execCode", SQTrue))) {
			error("Error executing code %s", code.c_str());
		} else {
			sq_push(v, -2);
			// call
			if (SQ_FAILED(sq_call(v, 1, SQTrue, SQTrue))) {
				error("Error calling code %s", code.c_str());
			} else {
				if (SQ_FAILED(sqget(v, -1, txt))) {
					error("Error getting call result %s", code.c_str());
				}
				sq_settop(v, top);
			}
		}
	}
	if (txt[0] == '@') {
		int id = atoi(txt.c_str() + 1);
		txt = g_twp->_textDb->getText(id);

		id = onTalkieId(id);
		Common::String key = talkieKey();
		key.toUppercase();
		Common::String name = Common::String::format("%s_%d", key.c_str(), id);
		Common::String path = name + ".lip";

		debugC(kDebugGame, "Load lip %s", path.c_str());
		if (g_twp->_pack->assetExists(path.c_str())) {
			GGPackEntryReader entry;
			entry.open(*g_twp->_pack, path);
			_lip.load(&entry);
			debugC(kDebugGame, "Lip %s loaded", path.c_str());
		}

		if (_actor->_sound) {
			g_twp->_audio->stop(_actor->_sound);
		}

		_actor->_sound = loadActorSpeech(name);
	} else if (txt[0] == '^') {
		txt = txt.substr(1);
	}

	// remove text in parentheses
	if (txt[0] == '(') {
		uint32 i = txt.find(')');
		if (i != Common::String::npos)
			txt = txt.substr(i + 1);
	}

	debugC(kDebugGame, "sayLine '%s'", txt.c_str());

	if (sqrawexists(_actor->_table, "sayingLine")) {
		const char *anim = _actor->_animName.empty() ? nullptr : _actor->_animName.c_str();
		sqcall(_actor->_table, "sayingLine", anim, txt);
	}

	// modify state ?
	Common::String state;
	if (!txt.empty() && txt[0] == '{') {
		uint32 i = txt.find('}');
		if (i != Common::String::npos) {
			state = txt.substr(1, i - 1);
			debugC(kDebugGame, "Set state from anim '%s'", state.c_str());
			if (state != "notalk") {
				_actor->play(state);
			}
			txt = txt.substr(i + 1);
		}
	}

	if (!_actor->_sound)
		setDuration(txt);

	if (_actor->_sayNode) {
		_actor->_sayNode->remove();
	}

	if (ConfMan.getBool("subtitles")) {
		Text text2("sayline", txt, thCenter, tvTop, SCREEN_WIDTH * 3.f / 4.f, _color);
		_actor->_sayNode = Common::SharedPtr<TextNode>(new TextNode());
		_actor->_sayNode->setText(text2);
		_actor->_sayNode->setColor(_color);
		_node = _actor->_sayNode;
		Math::Vector2d pos = g_twp->roomToScreen(_actor->_node->getAbsPos() + _actor->_talkOffset);

		// clamp position to keep it on screen
		pos.setX(CLIP(pos.getX(), 10.f + text2.getBounds().getX() / 2.f, SCREEN_WIDTH - text2.getBounds().getX() / 2.f));
		pos.setY(CLIP(pos.getY(), 10.f + text2.getBounds().getY(), SCREEN_HEIGHT - text2.getBounds().getY()));

		_actor->_sayNode->setPos(pos);
		_actor->_sayNode->setAnchorNorm(Math::Vector2d(0.5f, 0.0f));
		g_twp->_screenScene->addChild(_actor->_sayNode.get());
	}

	_elapsed = 0.f;
}

void Talking::disable() {
	Motor::disable();
	if (_actor->_sound) {
		g_twp->_audio->stop(_actor->_sound);
	}
	_texts.clear();
	_actor->setHeadIndex(1);
	if (_node)
		_node->remove();
	_elapsed = 0.f;
	_duration = 0.f;
}

SayLineAt::SayLineAt(const Math::Vector2d &pos, const Color &color, Common::SharedPtr<Object> actor, float duration, const Common::String &text)
	: TalkingBase(actor, duration), _pos(pos), _color(color), _text(text) {
	say(text);
}

void SayLineAt::say(const Common::String &text) {
	Common::String txt(text);
	if (txt.size() == 0) {
		debugC(kDebugGame, "say: skipping empty line");
		return;
	}

	if (txt[0] == '$') {
		HSQUIRRELVM v = g_twp->getVm();
		SQInteger top = sq_gettop(v);
		sq_pushroottable(v);
		Common::String code(Common::String::format("return %s", text.substr(1, text.size() - 1).c_str()));
		if (SQ_FAILED(sq_compilebuffer(v, code.c_str(), code.size(), "execCode", SQTrue))) {
			error("Error executing code %s", code.c_str());
		} else {
			sq_push(v, -2);
			// call
			if (SQ_FAILED(sq_call(v, 1, SQTrue, SQTrue))) {
				error("Error calling code %s", code.c_str());
			} else {
				if (SQ_FAILED(sqget(v, -1, txt))) {
					error("Error getting call result %s", code.c_str());
				}
				sq_settop(v, top);
			}
		}
	}

	if (txt[0] == '@') {
		int id = atoi(txt.c_str() + 1);
		txt = g_twp->_textDb->getText(id);

		if (_actor) {
			id = onTalkieId(id);
			Common::String key(talkieKey());
			key.toUppercase();
			Common::String name = Common::String::format("%s_%d", key.c_str(), id);
			Common::String path(name + ".lip");

			debugC(kDebugGame, "Load lip %s", path.c_str());
			if (g_twp->_pack->assetExists(path.c_str())) {
				GGPackEntryReader entry;
				entry.open(*g_twp->_pack, path);
				Lip lip;
				lip.load(&entry);
				debugC(kDebugGame, "Lip %s loaded", path.c_str());
			}

			if (_actor->_sound) {
				g_twp->_audio->stop(_actor->_sound);
			}

			_actor->_sound = loadActorSpeech(name);
		}
	} else if (txt[0] == '^') {
		txt = txt.substr(1);
	}

	// remove text in parentheses
	if (txt[0] == '(') {
		uint32 i = txt.find(')');
		if (i != Common::String::npos)
			txt = txt.substr(i + 1);
	}

	if (_actor && !_actor->_sound)
		setDuration(txt);

	debugC(kDebugGame, "sayLine '%s'", txt.c_str());

	// transform talking position to screen pos
	Math::Vector2d talkingSize(320.f, 180.f);
	Math::Vector2d pos(Math::Vector2d(SCREEN_WIDTH, SCREEN_HEIGHT) * _pos / talkingSize);

	Text text2("sayline", txt, thCenter, tvTop, SCREEN_WIDTH * 3.f / 4.f, _color);
	_node = Common::SharedPtr<TextNode>(new TextNode());
	_node->setText(text2);
	_node->setPos(pos);
	_node->setColor(_color);
	_node->setAnchorNorm(Math::Vector2d(0.5f, 0.0f));
	g_twp->_screenScene->addChild(_node.get());

	_elapsed = 0.f;
}

void SayLineAt::onUpdate(float elapsed) {
	if (!isEnabled())
		return;

	_elapsed += elapsed * getTalkSpeed();
	if (_actor && _actor->_sound) {
		if (!g_twp->_audio->playing(_actor->_sound)) {
			debugC(kDebugGame, "talking %s audio stopped", _actor->_key.c_str());
			_actor->_sound = 0;
		}
	} else if (_elapsed >= _duration) {
		debugC(kDebugGame, "talking %s: ended", _text.c_str());
		disable();
	}
}

void SayLineAt::disable() {
	Motor::disable();
	if (_node)
		_node->remove();
}

Jiggle::Jiggle(Node *node, float amount) : _amount(amount), _node(node) {
}

Jiggle::~Jiggle() = default;

void Jiggle::onUpdate(float elapsed) {
	_jiggleTime += 20.f * elapsed;
	_node->setRotationOffset(_amount * sin(_jiggleTime));
}

MoveCursorTo::MoveCursorTo(const Math::Vector2d &pos, float time)
	: _pos(pos),
	  _tween(g_twp->_cursor.pos, pos, time, intToInterpolationMethod(IK_LINEAR)) {
}

void MoveCursorTo::onUpdate(float elapsed) {
	_tween.update(elapsed);
	g_twp->_cursor.pos = _tween.current();
	if (!_tween.running())
		disable();
}

} // namespace Twp
