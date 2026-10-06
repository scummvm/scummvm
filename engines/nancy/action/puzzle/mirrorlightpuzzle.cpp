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
#include "engines/nancy/util.h"
#include "engines/nancy/puzzledata.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/action/puzzle/mirrorlightpuzzle.h"

namespace Nancy {
namespace Action {

static const double kTwoPi = 2.0 * M_PI;
static const double kRotateStep = kTwoPi * 0.1 * 0.02;	// 0.72 degrees per tick
static const uint32 kRotateDelay = 200;					// ms from pressing a button to the first turn
static const uint32 kRotateTickTime = 20;				// ms between turns while the button is held
static const uint32 kOverlayFrameTime = 66;				// ms per overlay animation frame
static const double kUnsetMirrorAngle = -1.0;

void MirrorLightPuzzle::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _imageName);
	_beamAngle = stream.readSint16LE();
	_beamOriginX = stream.readSint32LE();
	_beamOriginY = stream.readSint32LE();
	stream.skip(1);							// designer debug flag (shows the hovered mirror's angle)
	_beamColor[2] = stream.readByte();		// stored as b, g, r
	_beamColor[1] = stream.readByte();
	_beamColor[0] = stream.readByte();
	_beamHalfWidth = stream.readSint16LE();
	_beamCenterOpacity = stream.readDoubleLE();
	_beamEdgeOpacity = stream.readDoubleLE();

	// Mirror sprite frames - one per evenly-spaced angle around a full turn.
	uint16 numFrames = stream.readUint16LE();
	_frameSrcRects.resize(numFrames);
	for (uint i = 0; i < numFrames; ++i) {
		readRect(stream, _frameSrcRects[i]);
	}

	// Mirrors. The original reads every record then ignores any past kMaxMirrors.
	uint16 numMirrors = stream.readUint16LE();
	_mirrors.resize(numMirrors);
	for (uint i = 0; i < numMirrors; ++i) {
		Mirror &m = _mirrors[i];
		readRect(stream, m.destRect);
		m.angle = (double)stream.readSint16LE() * (M_PI / 180.0);
		stream.skip(16);	// bounds for dragging the mirror around (unused; empty in the data)
		m.minAngle = (double)stream.readSint16LE() * (M_PI / 180.0);
		m.maxAngle = (double)stream.readSint16LE() * (M_PI / 180.0);
		m.step = kRotateStep;

		if (m.isRotatable() && !isAngleWithinLimits(m, m.angle)) {
			m.angle = m.minAngle;
		}
	}
	if (_mirrors.size() > kMaxMirrors) {
		_mirrors.resize(kMaxMirrors);
	}

	readActionZoneArray(stream, _zones);
}

bool MirrorLightPuzzle::isAngleWithinLimits(const Mirror &m, double angle) const {
	if (m.minAngle <= m.maxAngle) {
		return angle >= m.minAngle && angle <= m.maxAngle;
	}

	// The range wraps past 0 (min > max)
	if (angle > m.minAngle && angle < kTwoPi) {
		return true;
	}
	if (angle >= m.maxAngle) {
		return false;
	}
	return angle > 0.0;
}

uint MirrorLightPuzzle::frameForAngle(double angle) const {
	if (_frameSrcRects.empty()) {
		return 0;
	}

	double a = fmod(angle, kTwoPi);
	if (a < 0.0) {
		a += kTwoPi;
	}

	uint frame = (uint)((a / kTwoPi) * _frameSrcRects.size() + 0.5);
	return frame % _frameSrcRects.size();
}

void MirrorLightPuzzle::drawMirror(uint index) {
	const Mirror &m = _mirrors[index];
	if (m.destRect.isEmpty()) {
		return;
	}

	const Common::Rect &src = _frameSrcRects[frameForAngle(m.angle)];
	_drawSurface.blitFrom(_image, src, m.destRect);
}

void MirrorLightPuzzle::traceBeam() {
	_beamPath.clear();

	// The original marches the beam a fixed step at a time, reflecting off each
	// mirror it enters. It works in a y-up frame (it negates dy when measuring
	// angles), so in screen space (y-down) a beam at angle a advances along
	// (cos a, -sin a): the level's 270deg source points straight down. Each
	// mirror's stored angle is its surface NORMAL, so reflection is the standard
	// r = d - 2(d.n)n (verified against the original: the 270deg beam off the
	// fixed 75deg mirror heads to the top-right mirror).
	const double kStep = 1.0;
	const int kMaxSteps = 20000;

	double a = (double)_beamAngle * (M_PI / 180.0);
	double dx = cos(a);
	double dy = -sin(a);

	double px = _beamOriginX;
	double py = _beamOriginY;
	Common::Point origin((int16)px, (int16)py);
	_beamPath.push_back(origin);

	Common::Rect vp = NancySceneState.getViewport().getBounds();
	int lastMirror = -1;

	for (int steps = 0; steps < kMaxSteps; ++steps) {
		px += dx * kStep;
		py += dy * kStep;
		Common::Point p((int16)(px + 0.5), (int16)(py + 0.5));

		if (p.x < 0 || p.y < 0 || p.x >= vp.width() || p.y >= vp.height()) {
			break;	// beam left the play area without solving
		}

		// Stepping clear of the last mirror re-arms it for another bounce.
		if (lastMirror != -1 && !_mirrors[lastMirror].destRect.contains(p)) {
			lastMirror = -1;
		}

		int hit = -1;
		for (uint i = 0; i < _mirrors.size(); ++i) {
			if ((int)i != lastMirror && !_mirrors[i].destRect.isEmpty() &&
					_mirrors[i].destRect.contains(p)) {
				hit = (int)i;
				break;
			}
		}

		if (hit != -1) {
			// Bounce at the mirror's center (not the edge where the beam entered),
			// so the beam visually meets each mirror at its middle.
			const Common::Rect &mr = _mirrors[hit].destRect;
			px = (mr.left + mr.right) / 2.0;
			py = (mr.top + mr.bottom) / 2.0;
			_beamPath.push_back(Common::Point((int16)px, (int16)py));

			// Reflect about the mirror's surface normal (its stored angle).
			double nx = cos(_mirrors[hit].angle);
			double ny = -sin(_mirrors[hit].angle);
			double dot = dx * nx + dy * ny;
			dx -= 2.0 * dot * nx;
			dy -= 2.0 * dot * ny;
			lastMirror = hit;
			continue;
		}

		// The beam ends on hitting a boundary zone (the one inside the bulb). The
		// scene-change zone around it only reacts to where the beam ends.
		for (uint i = 0; i < _zones.size(); ++i) {
			if (_zones[i].type == kZoneBoundary && _zones[i].rect.contains(p)) {
				_beamPath.push_back(p);
				return;
			}
		}
	}

	_beamPath.push_back(Common::Point((int16)(px + 0.5), (int16)(py + 0.5)));
}

void MirrorLightPuzzle::updateZones() {
	// Zones react to the beam's end point. Entering a zone plays its sound; the
	// scene-change zone solves the puzzle, and an overlay zone animates while the
	// beam stays in it.
	const Common::Point &end = _beamPath.back();
	for (uint i = 0; i < _zones.size(); ++i) {
		const ActionZone &z = _zones[i];
		Common::Rect r = z.rect;
		if (z.type == kZoneOverlay) {
			if (r.isEmpty()) {
				r = z.overlayDestRect;
			}
		} else if (z.type != kZoneUnknown15) {
			continue;
		}

		bool inside = r.contains(end);
		if (inside && !_zoneOccupied[i]) {
			playSoundBlock(z._sound);
			if (z.type == kZoneUnknown15) {
				_winZone = (int)i;
				_solved = true;
			}
		}
		_zoneOccupied[i] = inside;
	}
}

void MirrorLightPuzzle::blendBeamPixel(int x, int y, double opacity) {
	if (x < 0 || y < 0 || x >= _drawSurface.w || y >= _drawSurface.h) {
		return;
	}

	// Blend over whatever is already drawn here (a mirror or an earlier beam line),
	// or over the scene background where the surface is still transparent.
	const Graphics::PixelFormat &fmt = _drawSurface.format;
	uint32 pixel = _drawSurface.getPixel(x, y);
	byte r, g, b;
	if (pixel == _drawSurface.getTransparentColor()) {
		const Graphics::ManagedSurface &bg = NancySceneState.getViewport().getBackground();
		if (x < bg.w && y < bg.h) {
			bg.format.colorToRGB(bg.getPixel(x, y), r, g, b);
		} else {
			r = g = b = 0;
		}
	} else {
		fmt.colorToRGB(pixel, r, g, b);
	}

	r = (byte)(r * (1.0 - opacity) + _beamColor[0] * opacity);
	g = (byte)(g * (1.0 - opacity) + _beamColor[1] * opacity);
	b = (byte)(b * (1.0 - opacity) + _beamColor[2] * opacity);
	_drawSurface.setPixel(x, y, fmt.RGBToColor(r, g, b));
}

void MirrorLightPuzzle::drawBeamLine(Common::Point p0, Common::Point p1, double opacity) {
	int dx = ABS(p1.x - p0.x);
	int dy = -ABS(p1.y - p0.y);
	int sx = p0.x < p1.x ? 1 : -1;
	int sy = p0.y < p1.y ? 1 : -1;
	int err = dx + dy;
	int x = p0.x;
	int y = p0.y;

	while (true) {
		blendBeamPixel(x, y, opacity);
		if (x == p1.x && y == p1.y) {
			break;
		}

		int e2 = 2 * err;
		if (e2 >= dy) {
			err += dy;
			x += sx;
		}
		if (e2 <= dx) {
			err += dx;
			y += sy;
		}
	}
}

void MirrorLightPuzzle::drawBeam() {
	// Each segment is drawn as a band of parallel lines, offset vertically for a
	// mostly horizontal segment and horizontally otherwise. The opacity falls off
	// linearly from the center line to the edges of the band.
	const int halfWidth = MAX<int>(0, _beamHalfWidth);
	for (uint s = 1; s < _beamPath.size(); ++s) {
		const Common::Point &p0 = _beamPath[s - 1];
		const Common::Point &p1 = _beamPath[s];
		bool horizontal = ABS(p1.y - p0.y) < ABS(p1.x - p0.x);

		for (int o = -halfWidth; o <= halfWidth; ++o) {
			double t = halfWidth ? (double)(halfWidth - ABS(o)) / halfWidth : 1.0;
			double opacity = _beamEdgeOpacity + (_beamCenterOpacity - _beamEdgeOpacity) * t;
			Common::Point offset = horizontal ? Common::Point(0, o) : Common::Point(o, 0);
			drawBeamLine(p0 + offset, p1 + offset, opacity);
		}
	}
}

void MirrorLightPuzzle::drawOverlays() {
	for (const ZoneOverlay &overlay : _overlays) {
		if (!_zoneOccupied[overlay.zoneIndex]) {
			continue;
		}

		const ActionZone &z = _zones[overlay.zoneIndex];
		_drawSurface.blitFrom(overlay.image, z.overlaySrcRects[overlay.frame],
			Common::Point(z.overlayDestRect.left, z.overlayDestRect.top));
	}
}

void MirrorLightPuzzle::redraw() {
	_drawSurface.clear(g_nancy->_graphics->getTransColor());

	// Mirror sprites first, then the beam over them, then the lit target.
	for (uint i = 0; i < _mirrors.size(); ++i) {
		drawMirror(i);
	}

	drawBeam();
	drawOverlays();

	_needsRedraw = true;
}

void MirrorLightPuzzle::rotateMirror(uint index, int dir) {
	Mirror &m = _mirrors[index];

	// A turn that would cross a limit reverses the mirror, so it swings back.
	if (!isAngleWithinLimits(m, m.angle + dir * m.step)) {
		m.step = -m.step;
	}
	m.angle += dir * m.step;

	saveMirrorAngles();
	traceBeam();
	updateZones();
	redraw();
}

void MirrorLightPuzzle::saveMirrorAngles() {
	MirrorLightData *data = (MirrorLightData *)NancySceneState.getPuzzleData(MirrorLightData::getTag());
	if (!data) {
		return;
	}

	data->angles.resize(_mirrors.size());
	for (uint i = 0; i < _mirrors.size(); ++i) {
		data->angles[i] = _mirrors[i].angle;
	}
}

void MirrorLightPuzzle::init() {
	Common::Rect vpBounds = NancySceneState.getViewport().getBounds();
	_drawSurface.create(vpBounds.width(), vpBounds.height(),
		g_nancy->_graphics->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	setTransparent(true);
	setVisible(true);
	moveTo(vpBounds);

	loadImage();

	// Mirrors keep the angle the player last left them at.
	MirrorLightData *data = (MirrorLightData *)NancySceneState.getPuzzleData(MirrorLightData::getTag());
	if (data) {
		for (uint i = 0; i < _mirrors.size() && i < data->angles.size(); ++i) {
			Mirror &m = _mirrors[i];
			double saved = data->angles[i];
			if (saved != kUnsetMirrorAngle && (!m.isRotatable() || isAngleWithinLimits(m, saved))) {
				m.angle = saved;
			}
		}
	}

	// Each overlay zone draws from its own image.
	uint numOverlays = 0;
	for (const ActionZone &z : _zones) {
		if (z.type == kZoneOverlay && !z.overlayName.empty() && !z.overlaySrcRects.empty()) {
			++numOverlays;
		}
	}

	_overlays.resize(numOverlays);
	uint32 now = g_nancy->getTotalPlayTime();
	uint overlayIndex = 0;
	for (uint i = 0; i < _zones.size(); ++i) {
		const ActionZone &z = _zones[i];
		if (z.type != kZoneOverlay || z.overlayName.empty() || z.overlaySrcRects.empty()) {
			continue;
		}

		ZoneOverlay &overlay = _overlays[overlayIndex++];
		overlay.zoneIndex = i;
		overlay.nextFrameTime = now + kOverlayFrameTime;
		g_nancy->_resource->loadImage(Common::Path(z.overlayName), overlay.image);
		if (z.overlayPlayMode != 1) {
			overlay.image.setTransparentColor(_drawSurface.getTransparentColor());
		}
	}

	_zoneOccupied.resize(_zones.size(), false);

	traceBeam();
	updateZones();
	redraw();
}

void MirrorLightPuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		_state = kRun;
		// fall through
	case kRun: {
		// Solving shows one frame with the target lit, then triggers.
		if (_solved) {
			_state = kActionTrigger;
			break;
		}

		uint32 now = g_nancy->getTotalPlayTime();
		if (_rotateDir != 0 && _hoveredMirror != -1 && now >= _nextRotateTime) {
			rotateMirror(_hoveredMirror, _rotateDir);
			_nextRotateTime = now + kRotateTickTime;
		}

		bool advanced = false;
		for (ZoneOverlay &overlay : _overlays) {
			if (_zoneOccupied[overlay.zoneIndex] && now >= overlay.nextFrameTime) {
				overlay.frame = (overlay.frame + 1) % _zones[overlay.zoneIndex].overlaySrcRects.size();
				overlay.nextFrameTime = now + kOverlayFrameTime;
				advanced = true;
			}
		}
		if (advanced) {
			redraw();
		}
		break;
	}
	case kActionTrigger: {
		// The scene-change zone carries the target scene as its special effect id,
		// plus the special effect that covers the change.
		const ActionZone &z = _zones[_winZone];
		if (z.specialEffectId != kNoScene) {
			if (z.hasSpecialEffect) {
				NancySceneState.specialEffect(z.seType, z.seTotalTime, z.seFadeToBlackTime, z.seRect);
			}
			SceneChangeDescription sceneChange;
			sceneChange.sceneID = z.specialEffectId;
			NancySceneState.changeScene(sceneChange);
		}
		finishExecution();
		break;
	}
	}
}

void MirrorLightPuzzle::handleInput(NancyInput &input) {
	if (_state != kRun || _solved) {
		return;
	}

	// A held button keeps turning its mirror, even once the pointer leaves it.
	if (_rotateDir != 0) {
		uint16 held = _rotateDir > 0 ? NancyInput::kLeftMouseButtonHeld : NancyInput::kRightMouseButtonHeld;
		if (input.input & held) {
			g_nancy->_cursor->setCursorType(CursorManager::kRotateRight);
			return;
		}
		_rotateDir = 0;
	}

	_hoveredMirror = -1;
	for (uint i = 0; i < _mirrors.size(); ++i) {
		if (!_mirrors[i].isRotatable()) {
			continue;
		}

		Common::Rect screenRect = NancySceneState.getViewport().convertViewportToScreen(_mirrors[i].destRect);
		if (screenRect.contains(input.mousePos)) {
			_hoveredMirror = (int16)i;
			break;
		}
	}

	if (_hoveredMirror == -1) {
		return;
	}

	g_nancy->_cursor->setCursorType(CursorManager::kRotateRight);

	// Holding the left button turns the mirror one way, the right button the other.
	// The first turn comes after a short delay, so a quick click does nothing.
	if (input.input & NancyInput::kLeftMouseButtonDown) {
		_rotateDir = 1;
	} else if (input.input & NancyInput::kRightMouseButtonDown) {
		_rotateDir = -1;
	} else {
		return;
	}
	_nextRotateTime = g_nancy->getTotalPlayTime() + kRotateDelay;
}

} // End of namespace Action
} // End of namespace Nancy
