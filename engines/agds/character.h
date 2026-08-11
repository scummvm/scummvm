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

#ifndef CHARACTER_H
#define CHARACTER_H

#include "common/hashmap.h"
#include "common/ptr.h"
#include "common/rect.h"
#include "common/scummsys.h"

namespace Common {
class SeekableReadStream;
class ReadStream;
class WriteStream;
} // namespace Common
namespace Graphics {
struct Surface;
class ManagedSurface;
} // namespace Graphics

namespace AGDS {

class AGDSEngine;
class Object;
using ObjectPtr = Common::SharedPtr<Object>;
class Animation;
using AnimationPtr = Common::SharedPtr<Animation>;

class Character {
	using FogPtr = Common::ScopedPtr<Graphics::ManagedSurface>;
	AGDSEngine *_engine;
	ObjectPtr _object;
	AnimationPtr _animation;
	FogPtr _fog;
	bool _jokes;
	Common::String _name;
	Common::String _processName;
	Common::Point _pos;
	Common::Point _animationPos;
	bool _enabled;
	bool _visible;
	bool _stopped;
	bool _shown;
	int _phase;
	int _frames;
	int _direction;
	int _jokesDirection;
	int _movementDirections;
	int _fogMinZ, _fogMaxZ;

	struct AnimationDescription {
		struct Frame {
			int x, y;             // cumulative walk displacement from animation start
			uint w, h;            // cropped bitmap size of this frame
			uint canvasW, canvasH; // logical canvas size (anchor and scale reference)
			int offX, offY;       // anchor tweak of the crop inside the logical canvas
			int cropX, cropY;     // position of the crop inside the full FLIC frame
		};

		Common::String filename;
		Common::Array<Frame> frames;
	};
	Common::HashMap<uint, AnimationDescription> _animations;
	const AnimationDescription *_description;
	bool _standing;

	// Phased characters (Black Mirror main cast) store 5 animations per
	// facing direction: start(0), cycle A(1), cycle B(2), stop from A(3),
	// stop from B(4). Their animation index is dir * 5 + phase.
	static const int kAnimsPerDirection = 5;

	bool phased() const {
		return _movementDirections != 0 &&
		       _animations.size() >= (uint)(_movementDirections * kAnimsPerDirection);
	}

	int animIndexFor(int dir, int phase) const {
		return phased() ? dir * kAnimsPerDirection + phase : dir;
	}

	const AnimationDescription::Frame *frameAt(int dir, int phase, int frame) const {
		const AnimationDescription *desc = animationDescription(animIndexFor(dir, phase));
		if (!desc || desc->frames.empty())
			return nullptr;
		if (frame < 0)
			frame = 0;
		if (frame >= (int)desc->frames.size())
			frame = desc->frames.size() - 1;
		return &desc->frames[frame];
	}

	bool animate(int animIndex, int speed, bool jokes);
	float perspectiveScale() const;
	Common::Point animationPosition() const;

	// --- Walk state machine (mirrors the original engine) ---
	enum WalkState {
		kWalkIdle = 0,
		kWalkWalking = 1,          // normal walk along the path
		kWalkLeaving = 2,          // walking off-screen
		kWalkTurning = 3,          // turning in place only
		kWalkRemovePending = 5,    // finish current cycle, then re-issue moveTo
		kWalkLeavePending = 6,     // finish current cycle, then leave
		kWalkStopPending = 9       // finish current cycle, play stop anim, idle
	};

	int _walkState;
	int _targetDir;
	Common::Array<Common::Point> _path;
	uint _pathIndex;
	Common::Point _segStart, _segTarget, _finalTarget;
	int _finalDir;
	bool _exactMove;
	int _walkPhase;                // 0..4 walk phase, -1 = finished
	int _cyclesA, _cyclesB;
	bool _playStopAnim;
	Common::Array<int> _cycleScales; // permyriad scale per planned cycle
	int _scaleIdx;
	int _errX, _errY, _errStepX, _errStepY;
	int _errPeriodX, _errPeriodY, _errPhaseX, _errPhaseY;
	int _walkTickCnt;
	int _cycleErrX, _cycleErrY, _corrX, _corrY;
	int _decodedFrame; // FLIC frame currently decoded into _animation

	void walkUpdateTick();
	void turnOneStep();
	void startNextSegment();
	void setCycleScale(uint idx, int scale);
	void setupSegmentPhased(int dx, int dy);
	int advanceWalkPhase(bool cont);
	void advanceWalkFrame();
	bool walkTickAdvance();
	void stepErrorCorrection();
	void walkingTick();
	void arrive(int prevState);
	void stopMoving(bool force);
	bool loadWalkAnim();
	Common::Point totalDisp(int dir, int phase) const;
	int perspectiveScalePermyriad(int y) const;
	int scaleForSegmentY(int simY, int targetY) const;
	static double scalePermyriad(int v, int scale);
	void checkTrapRegions();

public:
	Character(AGDSEngine *engine, const Common::String &name);
	~Character();

	void associate(const Common::String &name);

	const AnimationDescription *animationDescription(uint index) const {
		auto it = _animations.find(index);
		return it != _animations.end() ? &it->_value : nullptr;
	}

	const Common::String &name() const {
		return _name;
	}

	const ObjectPtr &object() const {
		return _object;
	}

	void load(Common::SeekableReadStream &stream);
	void loadState(Common::ReadStream &stream);
	void saveState(Common::WriteStream &stream) const;

	void enable(bool enabled = true) {
		_enabled = enabled;
	}

	void visible(bool visible);
	bool visible() const {
		return _visible && _shown;
	}

	bool active() const {
		return _enabled && _visible;
	}

	bool animate(Common::Point pos, int direction, int speed);

	void stop();
	void leave(const Common::String &processName);

	int phase() const {
		return _jokes ? _phase : -1;
	}
	void phase(int phase) {
		_phase = phase;
	}

	void position(Common::Point pos) {
		// placing the character cancels any walk in progress
		cancelWalk();
		_pos = pos;
	}

	void cancelWalk() {
		_walkState = kWalkIdle;
		_path.clear();
		_pathIndex = 0;
		_targetDir = _direction;
	}

	Common::Point position() const {
		return _pos;
	}
	bool pointIn(Common::Point pos) const;

	void notifyProcess(const Common::String &processName);
	// leave: walk off-screen instead of stopping at the destination
	bool moveTo(const Common::String &processName, Common::Point dst, int direction,
	            bool userMove = false, bool leave = false);
	void pointTo(const Common::String &processName, Common::Point dst);
	// Graceful stop: finish the current walk cycle, play the stop
	// animation, then face dir (-1 keeps the current facing).
	void requestStop(int dir);

	bool walking() const {
		return _walkState != kWalkIdle;
	}

	bool direction(int dir);

	int direction() const {
		return _jokes ? _jokesDirection : _direction;
	}

	// base facing direction (0-15), ignoring any jokes gesture in progress
	int baseDirection() const {
		return _direction;
	}

	void tick(bool reactivate);
	void paint(Graphics::Surface &backbuffer, Common::Point pos) const;

	int getDirectionForMovement(Common::Point delta);

	int z() const;

	void reset();
	void setFog(Graphics::ManagedSurface *surface, int minZ, int maxZ);
};

} // End of namespace AGDS

#endif /* AGDS_CHARACTER_H */
