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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/random.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/util.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/action/puzzle/fishingpuzzle.h"

namespace Nancy {
namespace Action {

static const uint32 kMsPerTick = 20;
static const float kTicksPerSecond = 50.0f;

// Swim animations play at this rate when moving at the slowest speed
static const float kBaseAnimFPS = 15.0f;

// Path tangents are the swim speed scaled by this
static const float kTangentScale = 10.0f / 3.0f;

static const float kLineStopThreshold = 0.001f;

float FishingPuzzle::Vec2::length() const {
	return sqrtf(x * x + y * y);
}

FishingPuzzle::~FishingPuzzle() {
	for (Graphics::ManagedSurface *image : _catchableImages) {
		delete image;
	}
}

void FishingPuzzle::readEnding(Common::SeekableReadStream &stream, Ending &ending) {
	ending.scene._sceneChange.sceneID = stream.readUint16LE();
	ending.scene._sceneChange.frameID = stream.readUint16LE();
	ending.scene._sceneChange.continueSceneSound = kContinueSceneSound;
	ending.scene._flag.label = stream.readSint16LE();
	ending.scene._flag.flag = stream.readByte();
	ending.sound.readData(stream);
}

void FishingPuzzle::readData(Common::SeekableReadStream &stream) {
	_catchables.resize(MAX<int16>(stream.readSint16LE(), 0));
	for (Catchable &catchable : _catchables) {
		readFilename(stream, catchable.name);
		catchable.snagsLine = stream.readByte() == 1;
		catchable.minSpeed = stream.readFloatLE();
		catchable.maxSpeed = stream.readFloatLE();
		readFilename(stream, catchable.imageName);

		catchable.frames.resize(MAX<int16>(stream.readSint16LE(), 0));
		for (Common::Rect &frame : catchable.frames) {
			readRect(stream, frame);
		}

		if (catchable.frames.empty()) {
			error("FishingPuzzle: catchable %s has no frames", catchable.name.c_str());
		}

		readRect(stream, catchable.hitbox);
		catchable.hitbox.translate(-catchable.frames[0].left, -catchable.frames[0].top);
		readEnding(stream, catchable.ending);
	}

	_lanes.resize(MAX<int16>(stream.readSint16LE(), 0));
	for (Lane &lane : _lanes) {
		readRect(stream, lane.bounds);
		lane.minInterval = stream.readFloatLE();
		lane.maxInterval = stream.readFloatLE();

		lane.entries.resize(MAX<int16>(stream.readSint16LE(), 0));
		for (LaneEntry &entry : lane.entries) {
			Common::String name;
			readFilename(stream, name);
			entry.percent = stream.readSint32LE();

			uint i = 0;
			while (i < _catchables.size() && _catchables[i].name != name) {
				++i;
			}

			if (i == _catchables.size()) {
				error("FishingPuzzle: lane refers to missing catchable %s", name.c_str());
			}

			entry.catchable = i;
		}
	}

	_cursor = stream.readUint16LE();

	readFilename(stream, _imageName);
	_lineAcceleration = stream.readFloatLE() / kTicksPerSecond;
	readRect(stream, _lineSrc);
	_hookPoint.x = stream.readSint32LE() - _lineSrc.left;
	_hookPoint.y = stream.readSint32LE() - _lineSrc.top;
	_lineTop.x = stream.readSint32LE();
	_lineTop.y = stream.readSint32LE();
	_lineBottom.x = stream.readSint32LE();
	_lineBottom.y = stream.readSint32LE();
	readRect(stream, _lineBounds);

	Common::Rect start;
	readRect(stream, start);
	_lineStart.x = start.left;
	_lineStart.y = start.bottom;

	_clickOffset.x = stream.readSint32LE();
	_clickOffset.y = stream.readSint32LE();
	_reelSound.readData(stream);
	_exitHookY = stream.readSint32LE();
	readEnding(stream, _exitEnding);

	readExitHotspot(stream);
}

void FishingPuzzle::init() {
	initViewportSurface();
	loadImage();

	// The catchables all tend to share a single sprite sheet
	Common::Array<Common::Path> loadedNames;
	for (Catchable &catchable : _catchables) {
		uint i = 0;
		while (i < loadedNames.size() && loadedNames[i] != catchable.imageName) {
			++i;
		}

		if (i == loadedNames.size()) {
			Graphics::ManagedSurface *image = new Graphics::ManagedSurface();
			g_nancy->_resource->loadImage(catchable.imageName, *image);
			image->setTransparentColor(_drawSurface.getTransparentColor());
			_catchableImages.push_back(image);
			loadedNames.push_back(catchable.imageName);
		}

		catchable.imageID = i;
	}

	// Smaller catchables are drawn behind the larger ones
	for (uint i = 0; i < _catchables.size(); ++i) {
		const Common::Rect &frame = _catchables[i].frames[0];
		int32 area = frame.width() * frame.height();

		uint pos = 0;
		while (pos < _drawOrder.size()) {
			const Common::Rect &other = _catchables[_drawOrder[pos]].frames[0];
			if (other.width() * other.height() > area) {
				break;
			}

			++pos;
		}

		_drawOrder.insert_at(pos, i);
	}

	// Lanes start at a random point of their first interval
	for (Lane &lane : _lanes) {
		pickLaneInterval(lane);
		lane.ticks = (float)g_nancy->_randomSource->getRandomNumber(32767) / 32767.0f * lane.interval * kTicksPerSecond;
	}

	_linePos = Vec2(_lineStart.x, _lineSrc.top - _lineSrc.bottom + _lineStart.y);
	_lineTarget = Common::Point((int16)_linePos.x, (int16)_linePos.y);
	_lineVelocity = Vec2();
	_lineAccel = Vec2();
	_swimmers.clear();

	NancySceneState.setNoHeldItem();
	_lastTickTime = _lastFrameTime = g_nancy->getTotalPlayTime();
	redraw();
}

void FishingPuzzle::pickLaneInterval(Lane &lane) {
	lane.ticks = 0;
	lane.interval = lane.minInterval +
		(float)g_nancy->_randomSource->getRandomNumber(32767) / 32767.0f * (lane.maxInterval - lane.minInterval);
}

// Builds a path across the lane, from just outside one side to just outside the other,
// through up to three evenly spaced waypoints at random depths and speeds
void FishingPuzzle::spawn(const Lane &lane, uint catchable) {
	Common::RandomSource *rnd = g_nancy->_randomSource;
	const Catchable &data = _catchables[catchable];
	const Common::Rect &frame = data.frames[0];

	Swimmer swimmer;
	swimmer.catchable = catchable;
	swimmer.isMirrored = rnd->getRandomNumber(1) == 1;

	// The data rects include their right and bottom edges
	int frameWidth = frame.width() - 1;
	int laneRight = lane.bounds.right - 1;
	int laneHeight = MAX<int>(lane.bounds.height() - 1, 1);

	int numMidpoints = rnd->getRandomNumber(3);
	float step = (lane.bounds.width() - 1) / (numMidpoints + 0.00001f);

	for (int i = 0; i < numMidpoints + 2; ++i) {
		Waypoint point;
		point.pos.y = lane.bounds.top - (frame.height() - 1) / 2 + (int)rnd->getRandomNumber(laneHeight - 1);
		float speed = data.minSpeed + (float)rnd->getRandomNumber(32767) / 32767.0f * (data.maxSpeed - data.minSpeed);
		point.tangent.x = speed * kTangentScale * (swimmer.isMirrored ? -1 : 1);

		if (i == 0) {
			point.pos.x = swimmer.isMirrored ? laneRight + frameWidth + 1 : lane.bounds.left - frameWidth - 1;
		} else if (i == numMidpoints + 1) {
			point.pos.x = swimmer.isMirrored ? lane.bounds.left - 1 : laneRight + 1;
		} else {
			int index = swimmer.isMirrored ? numMidpoints - i + 1 : i - 1;
			point.pos.x = swimmer.isMirrored ? index * step - step * 0.5f : index * step + step * 0.5f;
		}

		swimmer.path.push_back(point);
	}

	for (uint i = 1; i < swimmer.path.size(); ++i) {
		Waypoint &prev = swimmer.path[i - 1];
		Waypoint &cur = swimmer.path[i];
		float speed = fabsf((prev.tangent.x + cur.tangent.x) * 0.5f);
		cur.ticks = (cur.pos - prev.pos).length() / speed * kTicksPerSecond * kTangentScale;
	}

	moveSwimmer(swimmer, 1);
	_swimmers.push_back(swimmer);
}

FishingPuzzle::Vec2 FishingPuzzle::hermite(const Vec2 &p1, const Vec2 &t1, const Vec2 &p2, const Vec2 &t2, float s) {
	float s2 = s * s;
	float s3 = s2 * s;
	return p1 * (2 * s3 - 3 * s2 + 1) + t1 * (s3 - 2 * s2 + s) + p2 * (3 * s2 - 2 * s3) + t2 * (s3 - s2);
}

// Follows the path along a spline, and returns true once its end is reached
bool FishingPuzzle::moveSwimmer(Swimmer &swimmer, float ticks) {
	Waypoint from = swimmer.path[0];
	Waypoint to = swimmer.path[1];
	Vec2 prevPos = hermite(from.pos, from.tangent, to.pos, to.tangent, swimmer.segmentTicks / MAX(to.ticks, 1.0f));

	swimmer.segmentTicks += ticks;
	while (swimmer.segmentTicks >= to.ticks) {
		swimmer.segmentTicks -= to.ticks;
		from = to;
		swimmer.path.remove_at(0);
		if (swimmer.path.size() < 2) {
			return true;
		}

		to = swimmer.path[1];
	}

	swimmer.pos = hermite(from.pos, from.tangent, to.pos, to.tangent, swimmer.segmentTicks / MAX(to.ticks, 1.0f));

	// The swim animation speeds up along with the swimmer
	const Catchable &data = _catchables[swimmer.catchable];
	float fps = (swimmer.pos - prevPos).length() / data.minSpeed * (kTicksPerSecond / ticks) * kBaseAnimFPS;
	if (fps > 0) {
		swimmer.animFPS = fps;
	}

	swimmer.animPhase += ticks / kTicksPerSecond * swimmer.animFPS / data.frames.size();
	swimmer.animPhase -= floorf(swimmer.animPhase);

	return false;
}

const Common::Rect &FishingPuzzle::getFrame(const Swimmer &swimmer) const {
	const Common::Array<Common::Rect> &frames = _catchables[swimmer.catchable].frames;
	return frames[MIN<uint>((uint)(swimmer.animPhase * frames.size()), frames.size() - 1)];
}

bool FishingPuzzle::segmentsIntersect(const Vec2 &a1, const Vec2 &a2, const Vec2 &b1, const Vec2 &b2) {
	Vec2 da = a2 - a1;
	Vec2 db = b2 - b1;
	float denom = da.x * db.y - da.y * db.x;
	if (fabsf(denom) < 0.0001f) {
		return false;
	}

	Vec2 diff = b1 - a1;
	float t = (diff.x * db.y - diff.y * db.x) / denom;
	float u = (diff.x * da.y - diff.y * da.x) / denom;
	return t >= 0 && t <= 1 && u >= 0 && u <= 1;
}

// Caught when the hook is inside the hitbox, or for some catchables when the line crosses it
bool FishingPuzzle::isCaught(const Swimmer &swimmer, const Vec2 &hook, const Vec2 &lineTop, const Vec2 &lineBottom) const {
	const Catchable &data = _catchables[swimmer.catchable];
	// The data rects include their right and bottom edges
	float left = swimmer.isMirrored ? swimmer.pos.x - (data.hitbox.right - 1) : swimmer.pos.x + data.hitbox.left;
	float right = swimmer.isMirrored ? swimmer.pos.x - data.hitbox.left : swimmer.pos.x + (data.hitbox.right - 1);
	float top = swimmer.pos.y + data.hitbox.top;
	float bottom = swimmer.pos.y + (data.hitbox.bottom - 1);

	if (hook.x >= left && hook.x <= right && hook.y >= top && hook.y <= bottom) {
		return true;
	}

	if (!data.snagsLine) {
		return false;
	}

	Vec2 corners[4] = { Vec2(left, top), Vec2(right, top), Vec2(right, bottom), Vec2(left, bottom) };
	for (uint i = 0; i < 4; ++i) {
		if (segmentsIntersect(lineTop, lineBottom, corners[i], corners[(i + 1) % 4])) {
			return true;
		}
	}

	return false;
}

// The line accelerates towards the clicked depth, keeping within its bounds
void FishingPuzzle::moveHookTo(const Common::Point &pos) {
	playSoundBlock(_reelSound);
	_lineVelocity = Vec2();

	int32 x = pos.x + _clickOffset.x - _hookPoint.x;
	int32 y = pos.y + _clickOffset.y - _hookPoint.y;
	x = MAX<int32>(x, _lineBounds.left);
	x -= MAX<int32>(_lineSrc.width() + x - _lineBounds.right, 0);
	y -= MAX<int32>(_lineSrc.height() + y - _lineBounds.bottom, 0);

	_lineTarget = Common::Point((int16)x, (int16)y);
	Vec2 direction = Vec2(x, y) - _linePos;
	float length = direction.length();
	_lineAccel = length > 0 ? direction * (_lineAcceleration / length) : Vec2();
}

// Returns true when the hook gets pulled out of the water
bool FishingPuzzle::moveLine(int32 ticks) {
	if (_lineVelocity.length() <= kLineStopThreshold && _lineAccel.length() <= kLineStopThreshold) {
		return false;
	}

	_lineVelocity = _lineVelocity + _lineAccel * ticks;
	Vec2 newPos = _linePos + _lineVelocity;
	Vec2 target(_lineTarget.x, _lineTarget.y);

	// Stop once the target has been passed
	if ((target - _linePos).length() < (target - newPos).length()) {
		_linePos = target;
		_lineVelocity = Vec2();
		_lineAccel = Vec2();
	} else {
		_linePos = newPos;
	}

	if (_hookPoint.y + _linePos.y < _exitHookY) {
		startEnding(_exitEnding);
		return true;
	}

	return false;
}

void FishingPuzzle::update(int32 lineTicks, float ticks) {
	if (lineTicks > 0 && moveLine(lineTicks)) {
		return;
	}

	Vec2 hook((int)(_hookPoint.x + _linePos.x), (int)(_hookPoint.y + _linePos.y));
	Vec2 lineTop((int)(_lineTop.x + _linePos.x), (int)(_lineTop.y + _linePos.y));
	Vec2 lineBottom((int)(_lineBottom.x + _linePos.x), (int)(_lineBottom.y + _linePos.y));

	for (uint i = 0; i < _swimmers.size(); ) {
		if (moveSwimmer(_swimmers[i], ticks)) {
			_swimmers.remove_at(i);
			continue;
		}

		if (isCaught(_swimmers[i], hook, lineTop, lineBottom)) {
			startEnding(_catchables[_swimmers[i].catchable].ending);
			return;
		}

		++i;
	}

	// Each lane sends out a random catchable at random intervals
	for (Lane &lane : _lanes) {
		lane.ticks += ticks;
		if (lane.interval >= lane.ticks / kTicksPerSecond) {
			continue;
		}

		pickLaneInterval(lane);

		int32 roll = g_nancy->_randomSource->getRandomNumber(99);
		int32 total = 0;
		for (const LaneEntry &entry : lane.entries) {
			total += entry.percent;
			if (roll < total) {
				spawn(lane, entry.catchable);
				break;
			}
		}
	}
}

void FishingPuzzle::startEnding(const Ending &ending) {
	_isEnding = true;
	_endScene = ending.scene;
	_endSound = playSoundBlock(ending.sound);
}

void FishingPuzzle::redraw() {
	_drawSurface.clear(_drawSurface.getTransparentColor());

	_drawSurface.blitFrom(_image, _lineSrc, Common::Point((int16)floorf(_linePos.x + 0.5f), (int16)floorf(_linePos.y + 0.5f)));

	for (uint catchable : _drawOrder) {
		for (const Swimmer &swimmer : _swimmers) {
			if (swimmer.catchable != catchable) {
				continue;
			}

			const Graphics::ManagedSurface &image = *_catchableImages[_catchables[catchable].imageID];
			const Common::Rect &frame = getFrame(swimmer);
			Common::Point pos((int16)floorf(swimmer.pos.x + 0.5f), (int16)floorf(swimmer.pos.y + 0.5f));
			if (swimmer.isMirrored) {
				// A flipped blit mirrors across its whole source surface, so it gets the frame on its own
				const Graphics::Surface frameSurface = image.getSubArea(frame);
				pos.x -= frame.width();
				_drawSurface.transBlitFrom(frameSurface, pos, image.getTransparentColor(), true);
			} else {
				_drawSurface.blitFrom(image, frame, pos);
			}
		}
	}

	_needsRedraw = true;
}

void FishingPuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		_state = kRun;
		// fall through
	case kRun: {
		if (_exitRequested) {
			_state = kActionTrigger;
			break;
		}

		if (_isEnding) {
			if (_endSound.name.empty() || !g_nancy->_sound->isSoundPlaying(_endSound)) {
				_state = kActionTrigger;
			}

			break;
		}

		// The line's speed depends on how many updates it gets, so it keeps to whole ticks
		uint32 now = g_nancy->getTotalPlayTime();
		int32 lineTicks = (now - _lastTickTime) / kMsPerTick;
		_lastTickTime += lineTicks * kMsPerTick;

		float ticks = (now - _lastFrameTime) / (float)kMsPerTick;
		_lastFrameTime = now;

		if (ticks > 0) {
			update(lineTicks, ticks);
		}

		redraw();
		break;
	}
	case kActionTrigger:
		if (_exitRequested) {
			_exitScene.execute();
		} else {
			_endScene.execute();
		}

		finishExecution();
		break;
	}
}

void FishingPuzzle::handleInput(NancyInput &input) {
	if (_state != kRun || _isEnding || _exitRequested) {
		return;
	}

	Common::Rect viewportBounds = NancySceneState.getViewport().getBounds();
	if (!NancySceneState.getViewport().convertViewportToScreen(viewportBounds).contains(input.mousePos)) {
		return;
	}

	if (hoverExitHotspot(input)) {
		if (input.input & NancyInput::kLeftMouseButtonUp) {
			_exitRequested = true;
			input.eatMouseInput();
		}

		return;
	}

	setDataCursor(_cursor);

	if (input.input & NancyInput::kLeftMouseButtonUp) {
		Common::Rect mouseRect = NancySceneState.getViewport().convertScreenToViewport(
			Common::Rect(input.mousePos.x, input.mousePos.y, input.mousePos.x + 1, input.mousePos.y + 1));
		moveHookTo(mouseRect.origin());
		input.eatMouseInput();
	}
}

} // End of namespace Action
} // End of namespace Nancy
