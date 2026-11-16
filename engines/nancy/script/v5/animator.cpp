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
#include "common/util.h"

#include "engines/nancy/nancy.h"

#include "engines/nancy/script/luabind.h"
#include "engines/nancy/script/luastate.h"
#include "engines/nancy/script/record.h"
#include "engines/nancy/script/scenecontext.h"

#include "engines/nancy/script/v5/animator.h"

namespace Nancy {
namespace Script {

// A blend takes at least this long
static const double kMinBlendTime = 0.001;

// AnimatorTrack

AnimatorTrack::AnimatorTrack(AnimatorStage &stage, Kind kind, double x, double y) :
		_stage(stage),
		_kind(kind) {
	const double value[2] = { x, y };
	setCurrent(value);
}

const char *AnimatorTrack::getTypeName(Kind kind) {
	switch (kind) {
	case kAlpha:
		return "ND.AnimatorAlphaTrack";
	case kScale:
		return "ND.AnimatorScaleTrack";
	case kRotation:
		return "ND.AnimatorRotationTrack";
	case kPolar:
		return "ND.AnimatorPolarTrack";
	default:
		return "ND.AnimatorTranslationTrack";
	}
}

void AnimatorTrack::setCurrent(const double value[2]) {
	_start[0] = value[0];
	_start[1] = value[1];
	setValue(value);
}

void AnimatorTrack::setValue(const double value[2]) {
	_current[0] = value[0];
	_current[1] = value[1];

	_matrix = ScriptMatrix();
	switch (_kind) {
	case kScale:
		_matrix = ScriptMatrix::makeSRT(value[0], value[1], 0.0, 0.0, 0.0);
		break;
	case kRotation:
		// D3DXMatrixRotationZ
		_matrix = ScriptMatrix::makeSRT(1.0, 1.0, value[0], 0.0, 0.0);
		break;
	case kTranslation:
		_matrix = ScriptMatrix::makeSRT(1.0, 1.0, 0.0, value[0], value[1]);
		break;
	case kPolar:
		_matrix = ScriptMatrix::makeSRT(1.0, 1.0, 0.0, value[0] * cos(value[1]), value[0] * sin(value[1]));
		break;
	default:
		break;
	}
}

double AnimatorTrack::getDuration() const {
	double duration = 0.0;
	for (const Blend &blend : _blends) {
		duration += blend.duration;
	}

	return duration;
}

void AnimatorTrack::finishFirstBlend() {
	const Blend blend = _blends.front();
	_blends.remove_at(0);
	setCurrent(blend.target);

	if (_blends.empty()) {
		_stage.onTrackFinished();
	}
}

// Only one blend advances per frame. Time left over when it ends isn't passed
// on to the next one.
bool AnimatorTrack::advance(double seconds) {
	if (_blends.empty()) {
		return false;
	}

	Blend &blend = _blends.front();
	const double time = blend.time + seconds;
	if (blend.duration <= time) {
		finishFirstBlend();
		return true;
	}

	double progress = CLIP(time / blend.duration, 0.0, 1.0);
	if (blend.kind == kSlerp) {
		progress = sin((progress - 0.5) * M_PI) * 0.5 + 0.5;
	}

	double value[2];
	for (int i = 0; i < 2; ++i) {
		value[i] = _start[i] + (blend.target[i] - _start[i]) * progress;
	}

	setValue(value);
	blend.time = time;
	return true;
}

void AnimatorTrack::set(const double value[2]) {
	setCurrent(value);
	_stage.onTrackSet(_kind);
}

void AnimatorTrack::addBlend(BlendKind kind, double delay, double duration, const double target[2]) {
	if (delay != 0.0) {
		Blend wait;
		wait.duration = MAX(delay, kMinBlendTime);
		for (int i = 0; i < 2; ++i) {
			wait.target[i] = _blends.empty() ? _current[i] : _blends.back().target[i];
		}

		_blends.push_back(wait);
	}

	Blend blend;
	blend.kind = kind;
	blend.duration = MAX(duration, kMinBlendTime);
	blend.target[0] = target[0];
	blend.target[1] = target[1];
	_blends.push_back(blend);
}

void AnimatorTrack::stopBlending() {
	if (_blends.empty()) {
		return;
	}

	_blends.clear();
	setCurrent(_current);
	_stage.onTrackStopped();
}

void AnimatorTrack::finishBlending() {
	if (_blends.empty()) {
		return;
	}

	const Blend last = _blends.back();
	_blends.clear();
	setCurrent(last.target);
	_stage.onTrackStopped();
}

void AnimatorTrack::readValue(lua_State *L, int index, double value[2]) const {
	if (_kind == kAlpha || _kind == kRotation) {
		value[0] = luaL_checknumber(L, index);
		value[1] = 0.0;
		return;
	}

	if (lua_isnumber(L, index)) {
		value[0] = lua_tonumber(L, index);
		value[1] = luaL_checknumber(L, index + 1);
		return;
	}

	if (!lua_istable(L, index) && !lua_isuserdata(L, index)) {
		luaL_argerror(L, index, "expected a point, a vector or two numbers");
	}

	lua_getfield(L, index, "x");
	lua_getfield(L, index, "y");
	if (!lua_isnumber(L, -2) || !lua_isnumber(L, -1)) {
		luaL_argerror(L, index, "Table must have 'x' and 'y' fields");
	}

	value[0] = lua_tonumber(L, -2);
	value[1] = lua_tonumber(L, -1);
	lua_pop(L, 2);
}

// Scales and translations are returned as vectors, and as points from
// Nancy32 on, which has no vectors
void AnimatorTrack::pushValue(lua_State *L) const {
	if (_kind == kAlpha || _kind == kRotation) {
		lua_pushnumber(L, _current[0]);
	} else if (g_nancy->getGameType() >= kGameTypeNancy32) {
		pushPointCoordinates(L, _current[0], _current[1]);
	} else {
		pushVector(L, ScriptVector(_current[0], _current[1], 0.0));
	}
}

// AnimatorStage

AnimatorStage::AnimatorStage(Animator &animator, uint index, double alpha) :
		_animator(animator),
		_index(index) {
	_tracks[AnimatorTrack::kAlpha] = new AnimatorTrack(*this, AnimatorTrack::kAlpha, alpha, 0.0);
	_tracks[AnimatorTrack::kScale] = new AnimatorTrack(*this, AnimatorTrack::kScale, 1.0, 1.0);
	_tracks[AnimatorTrack::kRotation] = new AnimatorTrack(*this, AnimatorTrack::kRotation, 0.0, 0.0);
	_tracks[AnimatorTrack::kTranslation] = new AnimatorTrack(*this, AnimatorTrack::kTranslation, 0.0, 0.0);
	_tracks[AnimatorTrack::kPolar] = new AnimatorTrack(*this, AnimatorTrack::kPolar, 0.0, 0.0);
}

AnimatorStage::~AnimatorStage() {
	for (AnimatorTrack *track : _tracks) {
		delete track;
	}

	// Drops what scripts stored in the stage, the way the exe keeps it: in the
	// registry, under the stage
	lua_State *L = _animator.getOwner().getContext().getLua();
	lua_pushlightuserdata(L, this);
	lua_pushnil(L);
	lua_rawset(L, LUA_REGISTRYINDEX);
}

void AnimatorStage::pushFields(lua_State *L) {
	lua_pushlightuserdata(L, this);
	lua_rawget(L, LUA_REGISTRYINDEX);
	if (lua_istable(L, -1)) {
		return;
	}

	lua_pop(L, 1);
	lua_newtable(L);
	lua_pushlightuserdata(L, this);
	lua_pushvalue(L, -2);
	lua_rawset(L, LUA_REGISTRYINDEX);
}

bool AnimatorStage::isBlending() const {
	for (const AnimatorTrack *track : _tracks) {
		if (track->isBlending()) {
			return true;
		}
	}

	return false;
}

double AnimatorStage::getDuration() const {
	double duration = 0.0;
	for (const AnimatorTrack *track : _tracks) {
		duration = MAX(duration, track->getDuration());
	}

	return duration;
}

bool AnimatorStage::advance(double seconds) {
	bool isBlending = false;
	for (AnimatorTrack *track : _tracks) {
		isBlending |= track->advance(seconds);
	}

	return isBlending;
}

void AnimatorStage::stopBlending() {
	for (AnimatorTrack *track : _tracks) {
		track->stopBlending();
	}
}

void AnimatorStage::finishBlending() {
	for (AnimatorTrack *track : _tracks) {
		track->finishBlending();
	}
}

void AnimatorStage::updateMatrix() {
	_matrix = _tracks[AnimatorTrack::kScale]->getMatrix() *
		_tracks[AnimatorTrack::kRotation]->getMatrix() *
		_tracks[AnimatorTrack::kTranslation]->getMatrix() *
		_tracks[AnimatorTrack::kPolar]->getMatrix();
}

// The alpha track only changes the record's alpha, the others only its
// transform
void AnimatorStage::onTrackSet(AnimatorTrack::Kind kind) {
	if (kind == AnimatorTrack::kAlpha) {
		_animator.updateAlpha();
	} else {
		_animator.updateTransform();
	}
}

// Updates both the transform and the alpha of the record to where the track
// ended up
void AnimatorStage::onTrackStopped() {
	_animator.updateTransform();
	_animator.updateAlpha();
	onTrackFinished();
}

void AnimatorStage::onTrackFinished() {
	if (!isBlending()) {
		_animator.onStageDone(_index);
	}
}

// Animator

Animator::Animator(Record &owner) : _owner(owner) {
	createStage();
}

Animator::~Animator() {
	for (AnimatorStage *stage : _stages) {
		delete stage;
	}
}

AnimatorStage *Animator::getStage(uint index) const {
	return index >= 1 && index <= _stages.size() ? _stages[index - 1] : nullptr;
}

void Animator::createStage() {
	_stages.push_back(new AnimatorStage(*this, _stages.size() + 1, _owner.getLocalAlpha()));
}

// The record keeps its current transform and alpha until something changes
// them
void Animator::removeAllStages() {
	for (AnimatorStage *stage : _stages) {
		delete stage;
	}

	_stages.clear();
}

bool Animator::isBlending() const {
	for (const AnimatorStage *stage : _stages) {
		if (stage->isBlending()) {
			return true;
		}
	}

	return false;
}

double Animator::getDuration() const {
	double duration = 0.0;
	for (const AnimatorStage *stage : _stages) {
		duration = MAX(duration, stage->getDuration());
	}

	return duration;
}

void Animator::stopBlending() {
	for (uint i = 0; i < _stages.size(); ++i) {
		_stages[i]->stopBlending();
	}
}

void Animator::finishBlending() {
	for (uint i = 0; i < _stages.size(); ++i) {
		_stages[i]->finishBlending();
	}
}

// While any track is blending, the record gets both the transform and the
// alpha of the stages
void Animator::advance(double seconds) {
	bool isBlending = false;
	for (AnimatorStage *stage : _stages) {
		isBlending |= stage->advance(seconds);
	}

	if (isBlending) {
		updateTransform();
		updateAlpha();
	}

	callDoneFunctions();
}

void Animator::updateTransform() {
	ScriptMatrix transform;
	for (AnimatorStage *stage : _stages) {
		stage->updateMatrix();
		transform = transform * stage->getMatrix();
	}

	_owner.setAnimatedTransform(transform);
}

void Animator::updateAlpha() {
	double alpha = 1.0;
	for (const AnimatorStage *stage : _stages) {
		alpha *= stage->getAlpha();
	}

	_owner.setAnimatedAlpha(CLIP(alpha, 0.0, 1.0));
}

void Animator::onStageDone(uint index) {
	_doneStages.push_back(index);
}

static void pushStageRef(lua_State *L, Record &record, uint stage);

void Animator::callDoneFunctions() {
	const Common::Array<uint> doneStages = _doneStages;
	_doneStages.clear();

	for (uint index : doneStages) {
		if (g_nancy->getGameType() < kGameTypeNancy30) {
			_owner.callFunction(Common::String::format("Stage%uDone", index).c_str());
			continue;
		}

		// Nancy30: stage.OnDone(stage)
		AnimatorStage *stage = getStage(index);
		if (!stage) {
			continue;
		}

		lua_State *L = _owner.getContext().getLua();
		stage->pushFields(L);
		lua_getfield(L, -1, "OnDone");
		lua_remove(L, -2);
		if (!lua_isfunction(L, -1)) {
			lua_pop(L, 1);
			continue;
		}

		pushStageRef(L, _owner, index);
		LuaState::call(L, 1, 0, "Error running the OnDone of an animator stage");
	}
}

// The Lua objects of the animator, its stages and their tracks. Each one
// identifies the record, the stage and the track, and keeps the record alive
// while the scripts hold a reference to it.

struct AnimatorRef {
	Record *record;
	uint stage;		// 0 for the animator
	int track;		// -1 for the animator or a stage
};

static const char *const kAnimatorType = "ND.Animator";
static const char *const kAnimatorStageType = "ND.AnimatorStage";

static void pushRef(lua_State *L, Record &record, const char *typeName, uint stage, int track) {
	AnimatorRef *ref = (AnimatorRef *)pushUserData(L, sizeof(AnimatorRef), typeName);
	ref->record = &record;
	ref->stage = stage;
	ref->track = track;

	lua_createtable(L, 1, 0);
	record.push();
	lua_rawseti(L, -2, 1);
	lua_setfenv(L, -2);
}

static void pushStageRef(lua_State *L, Record &record, uint stage) {
	pushRef(L, record, kAnimatorStageType, stage, -1);
}

void Animator::push(lua_State *L) {
	pushRef(L, _owner, kAnimatorType, 0, -1);
}

static Animator &getAnimator(lua_State *L, const AnimatorRef &ref) {
	Animator *animator = ref.record->getAnimator();
	if (!animator) {
		luaL_error(L, "The AR has no animator");
	}

	return *animator;
}

static Animator &checkAnimator(lua_State *L) {
	return getAnimator(L, *(AnimatorRef *)checkUserData(L, 1, kAnimatorType));
}

// A stage object always refers to the same stage number. It can't be used
// after RemoveAllStages() has removed that stage.
static AnimatorStage &getStage(lua_State *L, const AnimatorRef &ref) {
	AnimatorStage *stage = getAnimator(L, ref).getStage(ref.stage);
	if (!stage) {
		luaL_error(L, "Stage %u of the animator was removed", ref.stage);
	}

	return *stage;
}

static AnimatorStage &checkStage(lua_State *L) {
	return getStage(L, *(AnimatorRef *)checkUserData(L, 1, kAnimatorStageType));
}

static AnimatorTrack &checkTrack(lua_State *L) {
	const AnimatorRef *ref = nullptr;
	for (int kind = AnimatorTrack::kAlpha; !ref && kind < AnimatorTrack::kNumKinds; ++kind) {
		ref = (const AnimatorRef *)testUserData(L, 1, AnimatorTrack::getTypeName((AnimatorTrack::Kind)kind));
	}

	if (!ref) {
		luaL_typerror(L, 1, "ND.AnimatorTrack");
	}

	return getStage(L, *ref).getTrack((AnimatorTrack::Kind)ref->track);
}

// The functions that the animator, its stages and their tracks all have.
// Stopping or finishing the blends calls the Done functions of the stages.

static int animatorGetDuration(lua_State *L) {
	lua_pushnumber(L, checkAnimator(L).getDuration());
	return 1;
}

static int animatorIsBlending(lua_State *L) {
	lua_pushboolean(L, checkAnimator(L).isBlending());
	return 1;
}

static int animatorStopBlending(lua_State *L) {
	Animator &animator = checkAnimator(L);
	animator.stopBlending();
	animator.callDoneFunctions();
	return 0;
}

static int animatorFinishBlending(lua_State *L) {
	Animator &animator = checkAnimator(L);
	animator.finishBlending();
	animator.callDoneFunctions();
	return 0;
}

static int animatorCreateStage(lua_State *L) {
	checkAnimator(L).createStage();
	return 0;
}

static int animatorRemoveAllStages(lua_State *L) {
	checkAnimator(L).removeAllStages();
	return 0;
}

// animator[n] is stage n, or nil
static int animatorIndex(lua_State *L) {
	if (lua_type(L, 2) != LUA_TNUMBER) {
		return indexFunctions(L);
	}

	const AnimatorRef &ref = *(AnimatorRef *)checkUserData(L, 1, kAnimatorType);
	const int index = (int)lua_tointeger(L, 2);
	if (index < 1 || !getAnimator(L, ref).getStage(index)) {
		lua_pushnil(L);
	} else {
		pushRef(L, *ref.record, kAnimatorStageType, index, -1);
	}

	return 1;
}

static int stageGetDuration(lua_State *L) {
	lua_pushnumber(L, checkStage(L).getDuration());
	return 1;
}

static int stageIsBlending(lua_State *L) {
	lua_pushboolean(L, checkStage(L).isBlending());
	return 1;
}

static int stageStopBlending(lua_State *L) {
	AnimatorStage &stage = checkStage(L);
	Animator &animator = stage.getAnimator();
	stage.stopBlending();
	animator.callDoneFunctions();
	return 0;
}

static int stageFinishBlending(lua_State *L) {
	AnimatorStage &stage = checkStage(L);
	Animator &animator = stage.getAnimator();
	stage.finishBlending();
	animator.callDoneFunctions();
	return 0;
}

static bool findTrackKind(const char *name, AnimatorTrack::Kind &kind) {
	static const char *const kTrackNames[] = { "a", "s", "r", "t", "p" };
	for (uint i = 0; i < ARRAYSIZE(kTrackNames); ++i) {
		if (strcmp(name, kTrackNames[i]) == 0) {
			kind = (AnimatorTrack::Kind)i;
			return true;
		}
	}

	return false;
}

// stage.a, stage.s, stage.r and stage.t are its tracks, and in Nancy32 stage.p.
// In Nancy30, other names give what scripts stored in the stage.
static int stageIndex(lua_State *L) {
	AnimatorTrack::Kind kind;
	if (lua_type(L, 2) != LUA_TSTRING || !findTrackKind(lua_tostring(L, 2), kind)) {
		indexFunctions(L);
		if (lua_isnil(L, -1) && g_nancy->getGameType() >= kGameTypeNancy30) {
			lua_pop(L, 1);
			checkStage(L).pushFields(L);
			lua_pushvalue(L, 2);
			lua_rawget(L, -2);
			lua_remove(L, -2);
		}

		return 1;
	}

	const AnimatorRef &ref = *(AnimatorRef *)checkUserData(L, 1, kAnimatorStageType);
	getStage(L, ref);
	pushRef(L, *ref.record, AnimatorTrack::getTypeName(kind), ref.stage, kind);
	return 1;
}

// stage.t = value sets a track to a value. Nancy30 stores anything else in
// the stage, like its OnDone function.
static int stageNewIndex(lua_State *L) {
	static const char *const kTrackDescriptions[] = { "Alpha", "Scale", "Rotation", "Translation", "Polar" };

	AnimatorStage &stage = checkStage(L);
	AnimatorTrack::Kind kind;
	if (!findTrackKind(luaL_checkstring(L, 2), kind)) {
		if (g_nancy->getGameType() < kGameTypeNancy30) {
			return luaL_error(L, "Cannot set field '%s' of an animator stage", lua_tostring(L, 2));
		}

		stage.pushFields(L);
		lua_pushvalue(L, 2);
		lua_pushvalue(L, 3);
		lua_rawset(L, -3);
		return 0;
	}

	AnimatorTrack &track = stage.getTrack(kind);
	if (track.isBlending()) {
		return luaL_error(L, "%s Track on Stage %u is in the midst of blending. It cannot be overridden.",
			kTrackDescriptions[kind], stage.getIndex());
	}

	double value[2];
	track.readValue(L, 3, value);
	track.set(value);
	return 0;
}

// track:AddBlendLerp([delay, ]duration, value), and the same for AddBlendSlerp.
// The first number is a delay if another number follows the duration (alpha
// and rotation tracks), or follows the first number (scale and translation
// tracks).
static int addBlend(lua_State *L, AnimatorTrack::BlendKind kind) {
	AnimatorTrack &track = checkTrack(L);
	const bool hasOneNumber = track.getKind() == AnimatorTrack::kAlpha || track.getKind() == AnimatorTrack::kRotation;
	const bool hasDelay = lua_type(L, hasOneNumber ? 4 : 3) == LUA_TNUMBER;

	double delay = 0.0;
	double duration = 0.0;
	double target[2];
	if (hasDelay) {
		delay = luaL_checknumber(L, 2);
		duration = luaL_checknumber(L, 3);
		track.readValue(L, 4, target);
	} else {
		duration = luaL_checknumber(L, 2);
		track.readValue(L, 3, target);
	}

	track.addBlend(kind, delay, duration, target);
	return 0;
}

static int trackAddBlendLerp(lua_State *L) {
	return addBlend(L, AnimatorTrack::kLerp);
}

static int trackAddBlendSlerp(lua_State *L) {
	return addBlend(L, AnimatorTrack::kSlerp);
}

static int trackGet(lua_State *L) {
	checkTrack(L).pushValue(L);
	return 1;
}

static int trackSet(lua_State *L) {
	AnimatorTrack &track = checkTrack(L);
	if (track.isBlending()) {
		return luaL_error(L, "Cannot set a track's value while it's blending");
	}

	double value[2];
	track.readValue(L, 2, value);
	track.set(value);
	return 0;
}

static int trackGetDuration(lua_State *L) {
	lua_pushnumber(L, checkTrack(L).getDuration());
	return 1;
}

static int trackIsBlending(lua_State *L) {
	lua_pushboolean(L, checkTrack(L).isBlending());
	return 1;
}

static int trackStopBlending(lua_State *L) {
	AnimatorTrack &track = checkTrack(L);
	Animator &animator = track.getStage().getAnimator();
	track.stopBlending();
	animator.callDoneFunctions();
	return 0;
}

static int trackFinishBlending(lua_State *L) {
	AnimatorTrack &track = checkTrack(L);
	Animator &animator = track.getStage().getAnimator();
	track.finishBlending();
	animator.callDoneFunctions();
	return 0;
}

void Animator::registerTypes(lua_State *L) {
	static const luaL_Reg kAnimatorFunctions[] = {
		{ "__index", animatorIndex },
		{ "CreateStage", animatorCreateStage },
		{ "RemoveAllStages", animatorRemoveAllStages },
		{ "GetDuration", animatorGetDuration },
		{ "IsBlending", animatorIsBlending },
		{ "StopBlending", animatorStopBlending },
		{ "FinishBlending", animatorFinishBlending },
		{ nullptr, nullptr }
	};

	static const luaL_Reg kStageFunctions[] = {
		{ "__index", stageIndex },
		{ "__newindex", stageNewIndex },
		{ "GetDuration", stageGetDuration },
		{ "IsBlending", stageIsBlending },
		{ "StopBlending", stageStopBlending },
		{ "FinishBlending", stageFinishBlending },
		{ nullptr, nullptr }
	};

	static const luaL_Reg kTrackFunctions[] = {
		{ "__index", indexFunctions },
		{ "AddBlendLerp", trackAddBlendLerp },
		{ "AddBlendSlerp", trackAddBlendSlerp },
		{ "Get", trackGet },
		{ "Set", trackSet },
		{ "GetDuration", trackGetDuration },
		{ "IsBlending", trackIsBlending },
		{ "StopBlending", trackStopBlending },
		{ "FinishBlending", trackFinishBlending },
		{ nullptr, nullptr }
	};

	registerType(L, kAnimatorType, kAnimatorFunctions);
	registerType(L, kAnimatorStageType, kStageFunctions);
	for (int kind = AnimatorTrack::kAlpha; kind < AnimatorTrack::kNumKinds; ++kind) {
		registerType(L, AnimatorTrack::getTypeName((AnimatorTrack::Kind)kind), kTrackFunctions);
	}
}

} // End of namespace Script
} // End of namespace Nancy
