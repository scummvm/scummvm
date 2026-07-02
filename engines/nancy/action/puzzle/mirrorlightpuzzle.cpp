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


#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/util.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/action/puzzle/mirrorlightpuzzle.h"

namespace Nancy {
namespace Action {

// TODO - open items for this puzzle:
//  - Win/exit scene progression: the "solved" event flag is set outside the
//    puzzle data (not embedded here) and is still unidentified, so the scene
//    may not advance after a win.
//  - No exit/cancel path: how the player quits an unsolved puzzle is unknown.
//  - Win presentation: only a static lit-bulb overlay is drawn; the original
//    plays a bulb-lights-up animation with a sound cue - neither is wired up.
//  - Detector zone is chosen heuristically (any non-boundary zone away from the
//    source); the true detector zone is unconfirmed.
//  - Mirror rotation step (2 deg/click) and the glowing light-ray rendering are
//    approximations, not the original's exact behaviour.

void MirrorLightPuzzle::readData(Common::SeekableReadStream &stream) {
	// 65-byte base header.
	readFilename(stream, _imageName);
	_beamAngle = stream.readSint16LE();
	_beamOriginX = stream.readSint32LE();
	_beamOriginY = stream.readSint32LE();
	stream.skip(4);
	_glowRadius = stream.readSint16LE();
	stream.skip(8);							// step size (double)
	stream.skip(8);

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
		stream.skip(16);	// secondary rect (unused; empty in the data)
		m.minAngle = (double)stream.readSint16LE() * (M_PI / 180.0);
		m.maxAngle = (double)stream.readSint16LE() * (M_PI / 180.0);
	}
	if (_mirrors.size() > kMaxMirrors) {
		_mirrors.resize(kMaxMirrors);
	}

	readActionZoneArray(stream, _zones);
}

uint MirrorLightPuzzle::frameForAngle(double angle) const {
	if (_frameSrcRects.empty()) {
		return 0;
	}

	const double twoPi = 2.0 * M_PI;
	double a = fmod(angle, twoPi);
	if (a < 0.0) {
		a += twoPi;
	}

	uint frame = (uint)((a / twoPi) * _frameSrcRects.size() + 0.5);
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
	_solved = false;

	// The original marches the beam a fixed step at a time, reflecting off each
	// mirror it enters. It works in a y-up frame (it negates dy when measuring
	// angles), so in screen space (y-down) a beam at angle a advances along
	// (cos a, -sin a): the level's 270deg source points straight down. Each
	// mirror's stored angle is its surface NORMAL, so reflection is the standard
	// r = d - 2(d.n)n (verified against the original: the 270deg beam off the
	// fixed 75deg mirror heads to the top-right mirror).
	const double kStep = 1.0;
	const int kMaxSteps = 20000;
	const byte kZoneBoundary = 0x14;

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
			_beamPath.push_back(p);
			// Reflect about the mirror's surface normal (its stored angle).
			double nx = cos(_mirrors[hit].angle);
			double ny = -sin(_mirrors[hit].angle);
			double dot = dx * nx + dy * ny;
			dx -= 2.0 * dot * nx;
			dy -= 2.0 * dot * ny;
			lastMirror = hit;
			continue;
		}

		// Reaching a detector zone (any non-boundary zone away from the source)
		// solves the puzzle. TODO: confirm which zone is the true detector.
		for (uint i = 0; i < _zones.size(); ++i) {
			const ActionZone &z = _zones[i];
			if (z.type != kZoneBoundary && !z.rect.isEmpty() &&
					!z.rect.contains(origin) && z.rect.contains(p)) {
				_solved = true;
				break;
			}
		}
		if (_solved) {
			_beamPath.push_back(p);
			return;
		}
	}

	_beamPath.push_back(Common::Point((int16)(px + 0.5), (int16)(py + 0.5)));
}

void MirrorLightPuzzle::redraw() {
	_drawSurface.clear(g_nancy->_graphics->getTransColor());

	// Mirror sprites first, then the beam on top of them.
	for (uint i = 0; i < _mirrors.size(); ++i) {
		drawMirror(i);
	}

	// The beam as a round radial falloff (dim green-white outer -> bright white
	// core) to read as glowing light. TODO: the original composites actual
	// glowing light-ray sprites; a soft/additive glow is a follow-up.
	if (_beamPath.size() >= 2) {
		int t = MAX<int>(2, _glowRadius);
		uint32 colors[3] = {
			_drawSurface.format.RGBToColor(90, 120, 90),	// outer haze
			_drawSurface.format.RGBToColor(180, 210, 180),	// mid glow
			_drawSurface.format.RGBToColor(255, 255, 255)	// core
		};
		int bands[3] = { t, t / 2, MAX<int>(1, t / 4) };
		// Outer band first so brighter bands land on top.
		for (int band = 0; band < 3; ++band) {
			int r = bands[band];
			for (uint i = 1; i < _beamPath.size(); ++i) {
				const Common::Point &p0 = _beamPath[i - 1];
				const Common::Point &p1 = _beamPath[i];
				for (int oy = -r; oy <= r; ++oy) {
					for (int ox = -r; ox <= r; ++ox) {
						if (ox * ox + oy * oy > r * r) {
							continue;
						}
						_drawSurface.drawLine(p0.x + ox, p0.y + oy, p1.x + ox, p1.y + oy, colors[band]);
					}
				}
			}
		}
	}

	// Once solved, light up the detector overlay (the bulb graphic).
	if (_solved) {
		for (uint i = 0; i < _zones.size(); ++i) {
			const ActionZone &z = _zones[i];
			if (!z.overlaySrcRects.empty() && !z.overlayDestRect.isEmpty()) {
				_drawSurface.blitFrom(_image, z.overlaySrcRects[0],
					Common::Point(z.overlayDestRect.left, z.overlayDestRect.top));
			}
		}
	}

	_needsRedraw = true;
}

void MirrorLightPuzzle::rotateMirror(uint index, bool clockwise) {
	Mirror &m = _mirrors[index];
	if (m.minAngle == m.maxAngle) {
		return;	// fixed mirror
	}

	// Fine step so the beam can be aimed precisely; clamped to the mirror's
	// rotation range. The exact per-click amount in the original is unconfirmed.
	const double step = 2.0 * (M_PI / 180.0);
	m.angle += clockwise ? step : -step;
	if (m.angle < m.minAngle) {
		m.angle = m.minAngle;
	} else if (m.angle > m.maxAngle) {
		m.angle = m.maxAngle;
	}

	traceBeam();
	redraw();
}

void MirrorLightPuzzle::init() {
	Common::Rect vpBounds = NancySceneState.getViewport().getBounds();
	_drawSurface.create(vpBounds.width(), vpBounds.height(),
		g_nancy->_graphics->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	setTransparent(true);
	setVisible(true);
	moveTo(vpBounds);

	g_nancy->_resource->loadImage(_imageName, _image);
	_image.setTransparentColor(_drawSurface.getTransparentColor());

	traceBeam();
	redraw();
}

void MirrorLightPuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		_state = kRun;
		// fall through
	case kRun:
		if (_solved) {
			// Hold on the lit-bulb win state briefly before releasing.
			if (_solvedTime == 0) {
				_solvedTime = g_nancy->getTotalPlayTime();
			} else if (g_nancy->getTotalPlayTime() - _solvedTime > 1500) {
				_state = kActionTrigger;
			}
		}
		break;
	case kActionTrigger:
		// TODO: the win/exit scene transitions are driven externally (an event
		// flag the puzzle sets on solve + separate scene ARs), not embedded here.
		// The solved event flag is still unidentified, so the scene may not
		// advance yet; for now just release the record.
		finishExecution();
		break;
	}
}

void MirrorLightPuzzle::handleInput(NancyInput &input) {
	if (_state != kRun || _solved) {
		return;
	}

	for (uint i = 0; i < _mirrors.size(); ++i) {
		Mirror &m = _mirrors[i];
		if (m.minAngle == m.maxAngle) {
			continue;	// fixed mirror
		}

		Common::Rect screenRect = NancySceneState.getViewport().convertViewportToScreen(m.destRect);
		if (!screenRect.contains(input.mousePos)) {
			continue;
		}

		// Clicking the right half rotates one way, the left half the other.
		bool clockwise = input.mousePos.x >= (screenRect.left + screenRect.right) / 2;
		g_nancy->_cursor->setCursorType(clockwise ? CursorManager::kRotateRight : CursorManager::kRotateLeft);
		if (input.input & NancyInput::kLeftMouseButtonUp) {
			rotateMirror(i, clockwise);
		}
		return;
	}
}

} // End of namespace Action
} // End of namespace Nancy
