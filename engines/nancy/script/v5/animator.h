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

#ifndef NANCY_SCRIPT_V5_ANIMATOR_H
#define NANCY_SCRIPT_V5_ANIMATOR_H

#include "common/array.h"

#include "engines/nancy/script/mathtypes.h"

struct lua_State;

namespace Nancy {
namespace Script {

class Record;
class AnimatorStage;

// Nancy29 moves, rotates, scales and fades every record with its animator,
// which replaces the blend stages of the earlier games:
//
//   ar.animator[1].t:AddBlendSlerp(0.5, Point:New(500, 500))
//   ar.animator[1].r:AddBlendLerp(1, 2, math.pi)  -- waits 1s, rotates for 2s
//
// An animator is a list of stages, numbered from 1, and starts with one stage.
// Each stage has four tracks: a (alpha), s (scale), r (rotation) and t
// (translation). Nancy32 adds p (polar), a translation by a radius and an
// angle. A track plays the blends added to it one after the other, each one
// moving it to a target value. A lerp blend moves at a constant speed, while a
// "slerp" blend starts and ends slowly. When all tracks of a stage have
// finished blending, the record's "Stage<n>Done" function is called. Nancy30
// calls the stage's own OnDone function instead, which scripts store in the
// stage like any other field (ar.animator[1].OnDone).
//
// The record's transform is the product of the transforms of the stages, in
// order, and each stage scales, then rotates, then translates (then moves by
// its polar track). The record's alpha is the product of the alphas of the
// stages. Both are updated whenever a stage changes or blends.
class AnimatorTrack {
public:
	enum Kind {
		kAlpha,
		kScale,
		kRotation,
		kTranslation,
		kPolar,		// Nancy32: the radius in x, the angle in radians in y
		kNumKinds
	};

	enum BlendKind {
		kLerp,
		kSlerp
	};

	AnimatorTrack(AnimatorStage &stage, Kind kind, double x, double y);

	Kind getKind() const { return _kind; }
	AnimatorStage &getStage() const { return _stage; }

	bool isBlending() const { return !_blends.empty(); }

	// The total duration of the blends, counting the current one in full
	double getDuration() const;

	// The current value. Alpha and rotation tracks have a single value, in x.
	double getX() const { return _current[0]; }
	double getY() const { return _current[1]; }
	const ScriptMatrix &getMatrix() const { return _matrix; }

	// Advances the current blend. Returns whether the track is blending.
	bool advance(double seconds);

	// Sets the track to a value. A blend that is underway continues from it.
	void set(const double value[2]);

	// Adds a blend after the others. A delay is added before it as a blend
	// that keeps the value the track will have by then.
	void addBlend(BlendKind kind, double delay, double duration, const double target[2]);

	// Ends all blends, leaving the track at its current value (stop) or at
	// the target of the last blend (finish)
	void stopBlending();
	void finishBlending();

	// Reads a value for this kind of track from the stack: a number, a point,
	// a vector, a table with x and y, or two numbers
	void readValue(lua_State *L, int index, double value[2]) const;
	void pushValue(lua_State *L) const;

	static const char *getTypeName(Kind kind);

private:
	struct Blend {
		BlendKind kind = kLerp;
		double duration = 0.0;
		double time = 0.0;
		double target[2] = { 0.0, 0.0 };
	};

	// Sets the current value, which is also where the next blend starts from
	void setValue(const double value[2]);
	void setCurrent(const double value[2]);
	void finishFirstBlend();

	AnimatorStage &_stage;
	Kind _kind;
	double _start[2];
	double _current[2];
	ScriptMatrix _matrix;
	Common::Array<Blend> _blends;
};

class Animator;

class AnimatorStage {
public:
	AnimatorStage(Animator &animator, uint index, double alpha);
	~AnimatorStage();

	Animator &getAnimator() const { return _animator; }

	// Numbered from 1
	uint getIndex() const { return _index; }

	AnimatorTrack &getTrack(AnimatorTrack::Kind kind) const { return *_tracks[kind]; }

	const ScriptMatrix &getMatrix() const { return _matrix; }
	double getAlpha() const { return _tracks[AnimatorTrack::kAlpha]->getX(); }

	bool isBlending() const;
	double getDuration() const;
	bool advance(double seconds);
	void stopBlending();
	void finishBlending();

	// Scales, then rotates, then translates, then moves by the polar track
	void updateMatrix();

	// Called when a track was set to a value, when a script stopped or
	// finished its blends, or when its last blend ended
	void onTrackSet(AnimatorTrack::Kind kind);
	void onTrackStopped();
	void onTrackFinished();

	// Nancy30: pushes the table of what scripts stored in the stage
	void pushFields(lua_State *L);

private:
	Animator &_animator;
	uint _index;
	AnimatorTrack *_tracks[AnimatorTrack::kNumKinds];
	ScriptMatrix _matrix;
};

class Animator {
public:
	// The alpha track of each new stage starts at the record's own localAlpha
	Animator(Record &owner);
	~Animator();

	Record &getOwner() const { return _owner; }

	uint getNumStages() const { return _stages.size(); }
	AnimatorStage *getStage(uint index) const;	// numbered from 1; null if missing

	void createStage();
	void removeAllStages();

	bool isBlending() const;
	double getDuration() const;
	void stopBlending();
	void finishBlending();

	// Advances the blends, and passes the results to the record
	void advance(double seconds);

	// Sets the record's transform or alpha to the product of the stages
	void updateTransform();
	void updateAlpha();

	// A stage finished its blends. Its Done function is called later, once
	// the stages aren't being iterated, as it can change them.
	void onStageDone(uint index);
	void callDoneFunctions();

	// The Lua objects through which the scripts access the animator, its
	// stages and their tracks
	void push(lua_State *L);
	static void registerTypes(lua_State *L);

private:
	Record &_owner;
	Common::Array<AnimatorStage *> _stages;
	Common::Array<uint> _doneStages;
};

} // End of namespace Script
} // End of namespace Nancy

#endif // NANCY_SCRIPT_V5_ANIMATOR_H
