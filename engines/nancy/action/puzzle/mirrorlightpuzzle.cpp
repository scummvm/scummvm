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

// TODO - open items for this puzzle (mostly cosmetic):
//  - Win presentation: the original lights the bulb via a per-zone movie
//    animation (loaded by the movie loader at init); we draw a single static
//    overlay frame with no sound instead.
//  - The player's quit/cancel path (leaving an unsolved puzzle) is unknown.
//  - Mirror rotation step (2 deg/click) approximates the original's exact
//    per-click amount.

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
			// Bounce at the mirror's centre (not the edge where the beam entered),
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

		// Reaching the detector zone (the SpecialEffect zone at the bulb) solves
		// the puzzle.
		if (!_detectorRect.isEmpty() && _detectorRect.contains(p)) {
			_solved = true;
			_beamPath.push_back(p);
			return;
		}
	}

	_beamPath.push_back(Common::Point((int16)(px + 0.5), (int16)(py + 0.5)));
}

void MirrorLightPuzzle::drawBeamGlow() {
	if (_beamPath.size() < 2) {
		return;
	}

	const int w = _drawSurface.w;
	const int h = _drawSurface.h;
	// A soft Gaussian profile reads as a fuzzy, translucent light ray. peak is the
	// additive intensity at the beam centre (well under 255 so the room shows
	// through); sigma controls how quickly it fades; R clips the faint tail.
	const int R = MAX<int>(4, _glowRadius + _glowRadius / 2);
	const int R2 = R * R;
	const double sigma = MAX<double>(1.5, _glowRadius / 2.0);
	const double twoSigma2 = 2.0 * sigma * sigma;
	const int peak = 110;

	// Accumulate the max glow intensity per pixel (so overlapping stamps don't
	// over-brighten), then composite once.
	Common::Array<byte> intensity(w * h, 0);
	for (uint s = 1; s < _beamPath.size(); ++s) {
		const Common::Point &p0 = _beamPath[s - 1];
		const Common::Point &p1 = _beamPath[s];
		int dx = p1.x - p0.x;
		int dy = p1.y - p0.y;
		int len = MAX<int>(1, (int)sqrt((double)(dx * dx + dy * dy)));
		for (int i = 0; i <= len; ++i) {
			int cx = p0.x + dx * i / len;
			int cy = p0.y + dy * i / len;
			for (int oy = -R; oy <= R; ++oy) {
				for (int ox = -R; ox <= R; ++ox) {
					int px = cx + ox;
					int py = cy + oy;
					if (px < 0 || py < 0 || px >= w || py >= h) {
						continue;
					}
					int d2 = ox * ox + oy * oy;
					if (d2 > R2) {
						continue;
					}
					int val = (int)(peak * exp(-(double)d2 / twoSigma2));
					if (val <= 0) {
						continue;
					}
					byte &acc = intensity[py * w + px];
					if (val > acc) {
						acc = (byte)val;
					}
				}
			}
		}
	}

	const Graphics::ManagedSurface &bg = NancySceneState.getViewport().getBackground();
	const Graphics::PixelFormat &fmt = _drawSurface.format;
	uint32 transColor = _drawSurface.getTransparentColor();

	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < w; ++x) {
			int inten = intensity[y * w + x];
			if (inten == 0) {
				continue;
			}

			// Skip actual mirror-sprite pixels (the mirror faces stay clean), but
			// still glow over the transparent parts of a mirror's bounding box.
			// Mirrors are the only thing drawn before the glow, so any opaque
			// pixel here is a mirror pixel.
			uint32 under = _drawSurface.getPixel(x, y);
			if (under != transColor) {
				continue;
			}

			// Glow over the scene background behind the beam.
			byte r, g, b;
			if (x < bg.w && y < bg.h) {
				bg.format.colorToRGB(bg.getPixel(x, y), r, g, b);
			} else {
				r = g = b = 0;
			}

			r = (byte)MIN<int>(255, r + inten);
			g = (byte)MIN<int>(255, g + inten);
			b = (byte)MIN<int>(255, b + inten);
			_drawSurface.setPixel(x, y, fmt.RGBToColor(r, g, b));
		}
	}
}

void MirrorLightPuzzle::redraw() {
	_drawSurface.clear(g_nancy->_graphics->getTransColor());

	// Mirror sprites first, then the beam on top of them.
	for (uint i = 0; i < _mirrors.size(); ++i) {
		drawMirror(i);
	}

	// The beam, composited additively over the mirrors and scene background so it
	// reads as glowing light rather than flat lines.
	drawBeamGlow();

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

	// The detector zone carries a SpecialEffect whose id is the win scene (same
	// semantics as MinigolfPuzzle's sink zone). Its rect is the beam target.
	for (uint i = 0; i < _zones.size(); ++i) {
		if (_zones[i].specialEffectId >= 1000) {
			_detectorRect = _zones[i].rect;
			_winScene.sceneID = _zones[i].specialEffectId;
			break;
		}
	}

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
		// The detector zone's SpecialEffect id is the win scene.
		if (_solved && _winScene.sceneID >= 1000 && _winScene.sceneID != kNoScene) {
			NancySceneState.changeScene(_winScene);
		}
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
