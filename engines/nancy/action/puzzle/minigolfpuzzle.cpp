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
#include "engines/nancy/graphics.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/cursor.h"
#include "engines/nancy/util.h"
#include "engines/nancy/puzzledata.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/action/puzzle/minigolfpuzzle.h"

namespace Nancy {
namespace Action {

// TODO - open items:
//  - Wall restitution and the sink-speed threshold (kSinkSpeed) are approximations.
//  - Leaving an unsolved hole is a scene-level hotspot (not part of this record);
//    input is gated to the viewport so that hotspot still works, but it isn't
//    driven from here.

// The struck "speed" = clamp(dragLen, maxSpeed) * kPowerScale * maxSpeed; the ball
// then decelerates linearly by _decel each step. But the actual per-step movement
// is that speed times kMoveScale (the original scales the displacement by
// _DAT_005350a0/_DAT_005352d0 = 1/50), so travel distance = kMoveScale * speed^2 /
// (2*_decel) - quadratic in drag, but 50x gentler than the raw speed. A full-course
// shot is thus a comfortable ~170px drag. The physics advance in fixed 30Hz steps.
static const double kPowerScale = 0.005;
static const double kMoveScale = 1.0 / 50.0;	// displacement = speed * kMoveScale per step
static const double kFixedStep = 1.0 / 30.0;
static const double kRestSpeed = 0.5;		// stop the ball below this per-step speed
static const double kSinkSpeed = 130.0;		// ball sinks only if it reaches the cup at or below this speed; faster rolls over
static const double kRestitution = 0.8;		// wall-bounce energy retained
static const double kDefaultAimDrag = 80.0;	// default aim-cursor distance from the ball (mask px)

// Isometric projection (mask space -> screen): rotate 45 degrees, foreshorten Y by
// half. cos45 == sin45; the Y component is additionally scaled by kIsoYScale.
static const double kIsoCos = 0.70710678118654752;	// cos(pi/4) == sin(pi/4)
static const double kIsoYScale = 0.5;

static const int kGhostAlpha = 110;			// opacity (0-255) of the preview "virtual balls"

void MinigolfPuzzle::readData(Common::SeekableReadStream &stream) {
	// 106-byte PuzzleBase header.
	readFilename(stream, _ballImageName);		// 0x00
	readFilename(stream, _holeBoundaryName);	// 0x21
	_maxSpeed = stream.readSint32LE();			// 0x42
	_decel = stream.readDoubleLE();				// 0x46
	_launchMode = stream.readByte();			// 0x4e
	readRect(stream, _teeRect);					// 0x4f - the tee / ball-start square
	_initialPower = stream.readSint16LE();		// 0x5f
	_initialAngle = stream.readSint16LE();		// 0x61
	_strokeCountIndex = stream.readSint16LE();	// 0x63
	stream.skip(4);								// 0x65
	_mirrorFlag = stream.readByte();			// 0x69

	// Ball roll animation frames (one sprite per evenly-spaced rotation).
	uint16 numFrames = stream.readUint16LE();
	_ballFrames.resize(numFrames);
	for (uint i = 0; i < numFrames; ++i) {
		readRect(stream, _ballFrames[i]);
	}

	// Two random-sound blocks: the putt and the wall-bounce cues.
	_puttSound.readData(stream);
	_wallSound.readData(stream);

	// The hole/sink/overlay zones.
	readActionZoneArray(stream, _zones);
}

void MinigolfPuzzle::init() {
	Common::Rect vpBounds = NancySceneState.getViewport().getBounds();
	_drawSurface.create(vpBounds.width(), vpBounds.height(),
		g_nancy->_graphics->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	setTransparent(true);
	setVisible(true);
	moveTo(vpBounds);

	g_nancy->_resource->loadImage(_ballImageName, _image);
	_image.setTransparentColor(_drawSurface.getTransparentColor());
	g_nancy->_resource->loadImage(_holeBoundaryName, _boundaryMask);
	if (_boundaryMask.empty()) {
		warning("MinigolfPuzzle: boundary overlay '%s' did not load - walls will not work",
			_holeBoundaryName.toString().c_str());
	}

	// Isometric projection anchors: the mask centre maps to the viewport centre.
	_vpCenterX = vpBounds.width() / 2.0;
	_vpCenterY = vpBounds.height() / 2.0;
	if (!_boundaryMask.empty()) {
		_maskCenterX = _boundaryMask.w / 2.0;
		_maskCenterY = _boundaryMask.h / 2.0;
	}

	// The cups are the zones carrying a target scene (their special effect id), in
	// mask/course space. A hole can have several, each with its own scene/flag/sound
	// (e.g. hole 6a's special "shot sky high" cup plays Ball_Short, not GOL_Sink). A
	// separate zone over each cup carries Nancy's reaction voice line.
	_inSlope.resize(_zones.size(), false);
	for (uint i = 0; i < _zones.size(); ++i) {
		const ActionZone &z = _zones[i];
		if (z.specialEffectId >= 1000 && !z.rect.isEmpty()) {
			_sinkZones.push_back(i);
		} else if (_reactionSound.names.empty() && !z._sound.names.empty()) {
			_reactionSound = z._sound;
		}
	}

	// A cosmetic overlay sprite (e.g. hole 6a's broken wall), drawn at its dest rect.
	for (const ActionZone &z : _zones) {
		if (z.type == kZoneOverlay && !z.overlayName.empty() &&
				!z.overlaySrcRects.empty() && !z.overlayDestRect.isEmpty()) {
			g_nancy->_resource->loadImage(Common::Path(z.overlayName), _overlayImage);
			_overlayImage.setTransparentColor(_drawSurface.getTransparentColor());
			_overlaySrc = z.overlaySrcRects[0];
			_overlayDest = z.overlayDestRect;
			break;
		}
	}

	// Start the ball on the tee. In the mask the fairway is WHITE and walls/out-of-
	// bounds are BLACK, so the ball start sits on white - calibrate "open" there.
	if (!_teeRect.isEmpty()) {
		_ballX = (_teeRect.left + _teeRect.right) / 2.0;
		_ballY = (_teeRect.top + _teeRect.bottom) / 2.0;
	} else if (!_boundaryMask.empty()) {
		_ballX = _boundaryMask.w / 2.0;
		_ballY = _boundaryMask.h * 0.75;
	} else {
		_ballX = vpBounds.width() / 2.0;
		_ballY = vpBounds.height() * 0.75;
	}

	if (!_boundaryMask.empty() && _ballX >= 0 && _ballY >= 0 &&
			_ballX < _boundaryMask.w && _ballY < _boundaryMask.h) {
		_openColor = _boundaryMask.getPixel((int)_ballX, (int)_ballY);
	}

	// Seed the slope enter/leave state from the ball's start, so a tee that sits in
	// a slope doesn't fire a spurious kick on the first step.
	for (uint i = 0; i < _zones.size(); ++i) {
		_inSlope[i] = _zones[i].type == kZoneSlope &&
			_zones[i].rect.contains(Common::Point((int16)_ballX, (int16)_ballY));
	}

	// A default aim in the level's preset direction, so the preview shows at once.
	// The cursor sits behind the ball (opposite the launch direction).
	double a = (double)_initialAngle * (M_PI / 180.0);
	_aimCursor = Common::Point((int16)(_ballX - cos(a) * kDefaultAimDrag), (int16)(_ballY + sin(a) * kDefaultAimDrag));

	// Every hole starts with the player placing the ball on the tee, and its stroke
	// count reset to zero.
	_mgState = kPlacing;
	_lastUpdate = g_nancy->getTotalPlayTime();
	_strokes = 0;
	writeStrokeCount();

	redraw();
}

Common::Point MinigolfPuzzle::projectToScreen(double mx, double my) const {
	double px = mx - _maskCenterX;
	double py = my - _maskCenterY;
	double sx = _vpCenterX + kIsoCos * (px - py);
	double sy = _vpCenterY + kIsoCos * kIsoYScale * (px + py);
	return Common::Point((int16)(sx + 0.5), (int16)(sy + 0.5));
}

void MinigolfPuzzle::unprojectToMask(int sx, int sy, double &mx, double &my) const {
	double dx = (sx - _vpCenterX) / kIsoCos;
	double dy = (sy - _vpCenterY) / (kIsoCos * kIsoYScale);
	// Inverse of the (px-py, px+py) rotation.
	double px = (dx + dy) / 2.0;
	double py = (dy - dx) / 2.0;
	mx = px + _maskCenterX;
	my = py + _maskCenterY;
}

bool MinigolfPuzzle::isWall(int px, int py) const {
	if (_boundaryMask.empty()) {
		return false;	// no mask - fall back to open play
	}
	if (px < 0 || py < 0 || px >= (int)_boundaryMask.w || py >= (int)_boundaryMask.h) {
		return true;	// off the course
	}
	return _boundaryMask.getPixel(px, py) != _openColor;
}

void MinigolfPuzzle::drawBall() {
	if (_ballFrames.empty()) {
		return;
	}

	const Common::Rect &src = _ballFrames[_ballFrame % _ballFrames.size()];
	Common::Point c = projectToScreen(_ballX, _ballY);
	Common::Point pos(c.x - src.width() / 2, c.y - src.height() / 2);
	_drawSurface.blitFrom(_image, src, pos);
}

void MinigolfPuzzle::drawAimPreview() {
	// Velocity the current aim would produce (mirrors launchBall: away from cursor).
	double vx, vy;
	aimToVelocity(_ballX - _aimCursor.x, _ballY - _aimCursor.y, vx, vy);
	if (vx == 0.0 && vy == 0.0) {
		return;
	}
	const Common::Rect ghostSrc = _ballFrames.empty() ? Common::Rect() : _ballFrames[0];
	if (ghostSrc.isEmpty()) {
		return;
	}

	// March the shot forward and collect the predicted path.
	Common::Array<Common::Point> path;
	double x = _ballX, y = _ballY;
	const int kSteps = 240;
	for (int i = 0; i < kSteps; ++i) {
		bool reachedHole = stepBall(x, y, vx, vy, false);
		path.push_back(Common::Point((int16)(x + 0.5), (int16)(y + 0.5)));
		if (reachedHole || sqrt(vx * vx + vy * vy) < kRestSpeed) {
			break;
		}
	}

	// Show exactly 3 "virtual balls" spaced evenly along the path: bunched near the
	// ball for a soft hit, spread out for a hard one.
	for (int k = 1; k <= 3; ++k) {
		uint idx = (path.size() * k) / 3;
		if (idx > 0) {
			--idx;
		}
		Common::Point gc = projectToScreen(path[idx].x, path[idx].y);
		Common::Point gp(gc.x - ghostSrc.width() / 2, gc.y - ghostSrc.height() / 2);
		drawGhostBall(ghostSrc, gp);
	}
}

void MinigolfPuzzle::drawGhostBall(const Common::Rect &src, const Common::Point &dest) {
	// The preview balls are translucent: blend the ball sprite with the course
	// background so they read as fainter than the real ball.
	const Graphics::ManagedSurface &bg = NancySceneState.getViewport().getBackground();
	const Graphics::PixelFormat &fmt = _drawSurface.format;
	uint32 transColor = _image.getTransparentColor();

	for (int sy = 0; sy < src.height(); ++sy) {
		int dy = dest.y + sy;
		if (dy < 0 || dy >= _drawSurface.h) {
			continue;
		}
		for (int sx = 0; sx < src.width(); ++sx) {
			int dx = dest.x + sx;
			if (dx < 0 || dx >= _drawSurface.w) {
				continue;
			}

			uint32 px = _image.getPixel(src.left + sx, src.top + sy);
			if (px == transColor) {
				continue;	// transparent part of the sprite
			}

			byte ballR, ballG, ballB;
			_image.format.colorToRGB(px, ballR, ballG, ballB);

			byte bgR = 0, bgG = 0, bgB = 0;
			if (dx < (int)bg.w && dy < (int)bg.h) {
				bg.format.colorToRGB(bg.getPixel(dx, dy), bgR, bgG, bgB);
			}

			byte r = (byte)((ballR * kGhostAlpha + bgR * (255 - kGhostAlpha)) / 255);
			byte g = (byte)((ballG * kGhostAlpha + bgG * (255 - kGhostAlpha)) / 255);
			byte b = (byte)((ballB * kGhostAlpha + bgB * (255 - kGhostAlpha)) / 255);
			_drawSurface.setPixel(dx, dy, fmt.RGBToColor(r, g, b));
		}
	}
}

void MinigolfPuzzle::redraw() {
	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	if (!_overlayImage.empty() && !_overlayDest.isEmpty()) {
		_drawSurface.blitFrom(_overlayImage, _overlaySrc, Common::Point(_overlayDest.left, _overlayDest.top));
	}
	if (_mgState == kAiming) {
		drawAimPreview();
	}
	if (!_ballHidden) {
		drawBall();
	}
	_needsRedraw = true;
}

void MinigolfPuzzle::playSoundBlock(const RandomSoundBlock &block) {
	if (block.names.empty()) {
		return;
	}

	uint idx = block.names.size() == 1 ? 0 : g_nancy->_randomSource->getRandomNumber(block.names.size() - 1);
	const Common::String &name = block.names[idx];
	if (name.empty() || name == "NO SOUND") {
		return;
	}

	SoundDescription desc;
	desc.name = name;
	desc.channelID = block.channel;
	desc.numLoops = block.numLoops > 0 ? block.numLoops : 1;
	desc.volume = block.volume;

	g_nancy->_sound->loadSound(desc);
	g_nancy->_sound->playSound(desc);
}

void MinigolfPuzzle::aimToVelocity(double aimX, double aimY, double &vx, double &vy) const {
	// Struck speed is proportional to the drag length (clamped to maxSpeed), aimed
	// along the drag vector: speed = drag * kPowerScale * maxSpeed.
	double len = sqrt(aimX * aimX + aimY * aimY);
	if (len < 1.0) {
		vx = vy = 0.0;
		return;
	}
	double drag = MIN(len, (double)_maxSpeed);
	double speed = drag * kPowerScale * (double)_maxSpeed;
	vx = (aimX / len) * speed;
	vy = (aimY / len) * speed;
}

void MinigolfPuzzle::pipeExitVelocity(const ActionZone &zone, double inVx, double inVy, double &outVx, double &outVy) const {
	// A negative exitSpeed/exitAngle means "keep the ball's incoming value".
	double inSpeed = sqrt(inVx * inVx + inVy * inVy);
	double speed = zone.exitSpeed < 0 ? inSpeed : (double)zone.exitSpeed;
	double angle = zone.exitAngle < 0 ? atan2(-inVy, inVx) : (double)zone.exitAngle * (M_PI / 180.0);
	outVx = speed * cos(angle);
	outVy = -speed * sin(angle);
}

void MinigolfPuzzle::launchBall(const Common::Point &maskCursor) {
	// The club is pulled back behind the ball: the shot travels away from the
	// cursor (ball - cursor), golf-backswing style, not toward it.
	aimToVelocity(_ballX - maskCursor.x, _ballY - maskCursor.y, _velX, _velY);
	if (_velX == 0.0 && _velY == 0.0) {
		return;
	}

	// Count the stroke - the scorecard scores the hole by this value.
	++_strokes;
	writeStrokeCount();

	playSoundBlock(_puttSound);
	_mgState = kMoving;
	_lastUpdate = g_nancy->getTotalPlayTime();
	_stepAccum = 0.0;
}

void MinigolfPuzzle::writeStrokeCount() {
	if (_strokeCountIndex < 0) {
		return;
	}

	TableData *table = (TableData *)NancySceneState.getPuzzleData(TableData::getTag());
	if (!table) {
		return;
	}

	// The stroke-count slot is a combined table index (single values first, then
	// combos); the scorecard's text overlays read it back the same way.
	uint boundary = table->getNumSingleValues();
	uint index = (uint)_strokeCountIndex;
	if (index < boundary) {
		table->setSingleValue(index, _strokes);
	} else {
		table->setComboValue(index - boundary, (float)_strokes);
	}
}

bool MinigolfPuzzle::stepBall(double &x, double &y, double &vx, double &vy, bool playSounds) {
	// One fixed physics step. The actual displacement is the ball's speed scaled by
	// kMoveScale (vx/vy carry the raw speed, which the friction below decays).
	double dispX = vx * kMoveScale;
	double dispY = vy * kMoveScale;

	// Walk the displacement in ~1px sub-steps so a fast ball can't tunnel through a
	// thin wall, reflecting off each axis independently so it can slide along an
	// angled wall.
	int steps = (int)ceil(MAX(ABS(dispX), ABS(dispY)));
	if (steps < 1) {
		steps = 1;
	}
	double sx = dispX / steps;
	double sy = dispY / steps;
	bool bounced = false;

	for (int i = 0; i < steps; ++i) {
		double nx = x + sx;
		double ny = y + sy;

		// The cups: a ball slow enough drops into whichever cup it's over (potting),
		// a faster one rolls over it and Nancy reacts. A cup reads as non-fairway in
		// the mask, so an overshooting ball must skip the wall reflection here (via
		// continue) or it would bounce off the hole. Checked per sub-step so a fast
		// ball can't skip it.
		Common::Point ballPt((int16)(nx + 0.5), (int16)(ny + 0.5));
		bool overCup = false;
		for (uint si = 0; si < _sinkZones.size(); ++si) {
			if (!_zones[_sinkZones[si]].rect.contains(ballPt)) {
				continue;
			}
			if (sqrt(vx * vx + vy * vy) <= kSinkSpeed) {
				_sunkZone = (int)_sinkZones[si];
				x = nx;
				y = ny;
				return true;
			}
			overCup = true;
			break;
		}
		if (overCup) {
			if (playSounds) {
				if (!_wasOverHole) {
					playSoundBlock(_reactionSound);
				}
				_wasOverHole = true;
			}
			x = nx;
			y = ny;
			continue;
		}
		if (playSounds) {
			_wasOverHole = false;
		}

		// Teleport / pipe zones: entering one whisks the ball to its exit instead of
		// bouncing off the wall-hole.
		for (uint zi = 0; zi < _zones.size(); ++zi) {
			const ActionZone &z = _zones[zi];
			if (z.type != kZoneTeleport || z.exitRect.isEmpty() ||
					!z.rect.contains(Common::Point((int16)(nx + 0.5), (int16)(ny + 0.5)))) {
				continue;
			}

			if (playSounds) {
				// Real ball: enter the pipe. updateBall plays the warp sound, holds
				// the ball out of sight, then emerges it at the exit.
				_pipeZone = (int)zi;
				_pipeInVx = vx;
				_pipeInVy = vy;
				x = nx;
				y = ny;
				vx = vy = 0.0;
			} else {
				// Preview: emerge at once so the ghost path continues through the pipe.
				x = (z.exitRect.left + z.exitRect.right) / 2.0;
				y = (z.exitRect.top + z.exitRect.bottom) / 2.0;
				pipeExitVelocity(z, vx, vy, vx, vy);
			}
			return false;
		}

		if (isWall((int)(nx + 0.5), (int)(y + 0.5))) {
			sx = -sx;
			vx = -vx * kRestitution;
			nx = x;
			bounced = true;
		}
		if (isWall((int)(x + 0.5), (int)(ny + 0.5))) {
			sy = -sy;
			vy = -vy * kRestitution;
			ny = y;
			bounced = true;
		}
		if (isWall((int)(nx + 0.5), (int)(ny + 0.5))) {
			sx = -sx;
			sy = -sy;
			vx = -vx * kRestitution;
			vy = -vy * kRestitution;
			nx = x;
			ny = y;
			bounced = true;
		}

		x = nx;
		y = ny;
	}

	// Linear friction: shave a fixed amount off the speed each step, keeping the
	// direction, so the ball decelerates to a stop. A sand trap (terrain zone) the
	// ball is currently inside adds extra deceleration.
	double decel = _decel;
	for (uint zi = 0; zi < _zones.size(); ++zi) {
		const ActionZone &z = _zones[zi];
		if (z.type == kZoneTerrain && z.terrainDecel != 0.0 &&
				z.rect.contains(Common::Point((int16)(x + 0.5), (int16)(y + 0.5)))) {
			decel += z.terrainDecel;
		}
	}

	double speed = sqrt(vx * vx + vy * vy);
	if (speed > 0.0) {
		double newSpeed = speed - decel;
		if (newSpeed < 0.0) {
			newSpeed = 0.0;
		}
		vx = vx / speed * newSpeed;
		vy = vy / speed * newSpeed;
	}

	if (playSounds && bounced) {
		playSoundBlock(_wallSound);
	}
	return false;
}

void MinigolfPuzzle::updateBall() {
	uint32 now = g_nancy->getTotalPlayTime();
	double elapsed = (now - _lastUpdate) / 1000.0;
	_lastUpdate = now;
	if (elapsed <= 0.0) {
		return;
	}
	if (elapsed > 0.25) {
		elapsed = 0.25;	// don't try to catch up huge gaps (e.g. after a pause)
	}

	// Advance the physics in fixed 30Hz steps so travel distance is frame-rate
	// independent.
	_stepAccum += elapsed;
	while (_stepAccum >= kFixedStep) {
		_stepAccum -= kFixedStep;

		// Held inside a pipe: play the warp sound, keep the ball out of sight, then
		// emerge it at the exit once the hold time (or the sound) is done.
		if (_pipeZone != -1) {
			const ActionZone &z = _zones[_pipeZone];
			if (!_ballHidden) {
				playSoundBlock(z._sound);
				_ballHidden = true;
				_needsRedraw = true;
				_pipeReleaseTime = z.teleportDelay > 0 ? now + (uint32)z.teleportDelay : 0;
			}

			bool release = z.teleportDelay > 0 ? now >= _pipeReleaseTime
				: !g_nancy->_sound->isSoundPlaying((uint16)z._sound.channel);
			if (release) {
				_ballX = (z.exitRect.left + z.exitRect.right) / 2.0;
				_ballY = (z.exitRect.top + z.exitRect.bottom) / 2.0;
				pipeExitVelocity(z, _pipeInVx, _pipeInVy, _velX, _velY);
				_pipeZone = -1;
				_ballHidden = false;
				_needsRedraw = true;
			}
			continue;
		}

		bool reachedHole = stepBall(_ballX, _ballY, _velX, _velY, true);

		if (_pipeZone != -1) {
			continue;	// ball just entered a pipe - handle the hold next step
		}

		// Slopes: kick the ball's velocity toward the slope's angle on entering the
		// zone, and undo the kick on leaving, so it drifts downhill while inside.
		for (uint zi = 0; zi < _zones.size(); ++zi) {
			const ActionZone &z = _zones[zi];
			if (z.type != kZoneSlope) {
				continue;
			}
			bool inside = z.rect.contains(Common::Point((int16)(_ballX + 0.5), (int16)(_ballY + 0.5)));
			if (inside == _inSlope[zi]) {
				continue;
			}
			double a = (double)z.slopeAngle * (M_PI / 180.0);
			double fx = z.slopeForce * cos(a);
			double fy = -z.slopeForce * sin(a);
			_velX += inside ? fx : -fx;
			_velY += inside ? fy : -fy;
			_inSlope[zi] = inside;
		}

		if (!_ballFrames.empty()) {
			_ballFrame = (_ballFrame + 1) % _ballFrames.size();
		}

		// Potting the ball wins. Resolve the target scene / flag / fade from the cup
		// the ball actually dropped into (a hole can have several with different
		// outcomes, e.g. hole 4a's middle cup plays a cutscene).
		if (reachedHole && _sunkZone >= 0 && _sunkZone < (int)_zones.size()) {
			const ActionZone &cup = _zones[_sunkZone];
			_velX = _velY = 0.0;
			_ballX = (cup.rect.left + cup.rect.right) / 2.0;
			_ballY = (cup.rect.top + cup.rect.bottom) / 2.0;
			_mgState = kSunk;
			_solved = true;
			_sunkTime = now;
			playSoundBlock(cup._sound);

			_winScene.sceneID = cup.specialEffectId;
			if (cup.type == kZoneSceneChange && cup.tailId != -1) {
				NancySceneState.setEventFlag(cup.tailId, cup.tailFlag ? g_nancy->_true : g_nancy->_false);
			}
			if (cup.hasSpecialEffect) {
				_winHasFade = true;
				_winFadeType = cup.seType;
				_winFadeTotalTime = cup.seTotalTime;
				_winFadeToBlackTime = cup.seFadeToBlackTime;
				_winFadeRect = cup.seRect;
			}
			return;
		}

		// Coming to rest without sinking readies the next stroke from where it stopped.
		if (sqrt(_velX * _velX + _velY * _velY) < kRestSpeed) {
			_velX = _velY = 0.0;
			_mgState = kAiming;
			return;
		}
	}
}

void MinigolfPuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		_state = kRun;
		// fall through
	case kRun:
		if (_mgState == kMoving) {
			updateBall();
			redraw();
		} else if (_mgState == kSunk) {
			// Hold on the sunk ball briefly, then release the record.
			if (g_nancy->getTotalPlayTime() - _sunkTime > 1500) {
				_state = kActionTrigger;
			}
		}
		break;
	case kActionTrigger:
		// Potting shows the hole's win scene (and sets the win event flag). The sink
		// zone's special effect is the fade that covers the change, so start it just
		// before the scene change (it captures the current frame, then dissolves to
		// the new scene).
		if (_solved && _winScene.sceneID >= 1000 && _winScene.sceneID != kNoScene) {
			if (_winHasFade) {
				NancySceneState.specialEffect(_winFadeType, _winFadeTotalTime, _winFadeToBlackTime, _winFadeRect);
			}
			NancySceneState.changeScene(_winScene);
		}
		finishExecution();
		break;
	}
}

void MinigolfPuzzle::handleInput(NancyInput &input) {
	if (_state != kRun) {
		return;
	}

	// Keep the golf-club cursor while the ball is rolling / sunk (no input taken).
	if (_mgState == kMoving || _mgState == kSunk) {
		g_nancy->_cursor->setCursorType((CursorManager::CursorType)34, true);
		return;
	}

	// Only take input over the play area. Clicks outside the viewport (e.g. a
	// leave / give-up hotspot in the game frame) are left for other records, so we
	// don't strike the ball when the player is trying to quit the hole.
	if (!NancySceneState.getViewport().getScreenPosition().contains(input.mousePos)) {
		return;
	}

	// Cursor position, converted screen -> viewport -> mask (course) space.
	Common::Rect screenPt(input.mousePos.x, input.mousePos.y, input.mousePos.x + 1, input.mousePos.y + 1);
	Common::Rect vpPt = NancySceneState.getViewport().convertScreenToViewport(screenPt);
	double cmx, cmy;
	unprojectToMask(vpPt.left, vpPt.top, cmx, cmy);
	Common::Point cursor((int16)cmx, (int16)cmy);

	if (_mgState == kPlacing) {
		// The ball can only be dropped on the tee square; it tracks the cursor
		// there until a click sets it down. Uses the drag-and-drop hand cursor.
		g_nancy->_cursor->setCursorType(CursorManager::kDragHand);
		if (!_teeRect.isEmpty()) {
			_ballX = CLIP<int>(cursor.x, _teeRect.left, _teeRect.right - 1);
			_ballY = CLIP<int>(cursor.y, _teeRect.top, _teeRect.bottom - 1);
		} else {
			_ballX = cursor.x;
			_ballY = cursor.y;
		}
		redraw();
		if (input.input & NancyInput::kLeftMouseButtonUp) {
			_mgState = kAiming;
		}
		return;
	}

	// kAiming: the golf-club cursor aims (angle) and its distance from the ball
	// sets the power; a click strikes the ball. The club is CURS cursor type 34.
	g_nancy->_cursor->setCursorType((CursorManager::CursorType)34, true);
	_aimCursor = cursor;
	redraw();
	if (input.input & NancyInput::kLeftMouseButtonUp) {
		launchBall(cursor);
	}
}

} // End of namespace Action
} // End of namespace Nancy
