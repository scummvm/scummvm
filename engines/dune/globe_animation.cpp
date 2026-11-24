/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "dune/dune.h"
#include "common/stream.h"
#include "common/system.h"

namespace Dune {

/** A host repaint must not publish the partly drawn next globe frame. */
void DuneEngine::redrawGlobe() {
	if (_globePresented.size() != kScreenWidth * kScreenHeight) {
		drawFresco();
		_globeSpin = (_globeSpin + 1) % Map::kEquatorWidth; // B98B/B9A8
	} else {
		memcpy(_framebuffer1, _globePresented.begin(), _globePresented.size());
		_graphics.setFrameBuffer(_framebuffer1);
		setCompass(kCompassGlobe); // Rebuild controls without advancing the saved globe.
		drawPanel();
		drawMenu();
		drawGlobePlayerMarker(_globePresentedSpin);
	}
	// A fresh load has no displayed page. B98E's later partial globe copies
	// cannot expose the doors and borders reconstructed by this repaint.
	_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0, kScreenWidth, kScreenHeight);
	_system->updateScreen();
}

/** B8E1 registers B9AE with period one: at most one 148h row per poll. */
bool DuneEngine::pollGlobeAnimation(uint16 ticks) {
	if (!_onGlobe || !ticks || _globeBackground.size() != kScreenWidth * kScreenHeight ||
			_globePresented.size() != kScreenWidth * kScreenHeight)
		return false;
	if (!_map.stepGlobe(_globeBackground.begin(), _globeSpin, _globeTilt, _frescoDoors != 0, _globeDrawing))
		return false;
	// B98E copies only 2440's dirty inset; panel, gauges and borders stay put.
	const Common::Rect dirty(96, 15, 234, 134);
	for (int y = dirty.top; y < dirty.bottom; y++) {
		const uint at = y * kScreenWidth + dirty.left;
		memcpy(_framebuffer1 + at, _globeBackground.begin() + at, dirty.width());
		memcpy(_globePresented.begin() + at, _globeBackground.begin() + at, dirty.width());
	}
	_globePresentedSpin = _globeSpin;
	_graphics.setFrameBuffer(_framebuffer1);
	drawGlobePlayerMarker(); // BC0C belongs only to the displayed copy.
	_system->copyRectToScreen(_framebuffer1 + dirty.top * kScreenWidth + dirty.left,
			kScreenWidth, dirty.left, dirty.top, dirty.width(), dirty.height());
	_system->updateScreen();
	_globeSpin = (_globeSpin + 1) % Map::kEquatorWidth; // B9A8 follows presentation.
	return true;
}

void DuneEngine::updateGlobeAnimation() {
	const uint32 now = animationClockTicks();
	if (!_onGlobe) {
		_globeBackground.clear();
		_globePresented.clear();
		_globeDrawing = GlobeRenderState();
		_globeAnimationTime = now;
		return;
	}
	const uint16 ticks = animationElapsedTicks(_globeAnimationTime, now);
	_globeAnimationTime = now;
	pollGlobeAnimation(ticks);
}

void DuneEngine::saveGlobeAnimation(Common::WriteStream *stream) const {
	const bool active = _onGlobe && _globeBackground.size() == kScreenWidth * kScreenHeight &&
			_globePresented.size() == kScreenWidth * kScreenHeight;
	stream->writeByte(active);
	if (!active)
		return;
	stream->writeUint16LE(_globeSpin);
	stream->writeSint16LE(_globeTilt);
	stream->writeUint16LE(_globePresentedSpin);
	stream->writeUint16LE(_globeDrawing.position);
	stream->writeSint16LE(_globeDrawing.row);
	stream->writeByte(_globeDrawing.lowerHalf);
	stream->write(_globeBackground.begin(), _globeBackground.size());
	stream->write(_globePresented.begin(), _globePresented.size());
}

bool DuneEngine::loadGlobeAnimation(Common::ReadStream *stream) {
	_globeBackground.clear();
	_globePresented.clear();
	_globeDrawing = GlobeRenderState();
	_globeSpin = _globeTilt = _globePresentedSpin = 0;
	_globeAnimationTime = _system->getMillis() / 5;
	const byte active = stream->readByte();
	if (active > 1 || stream->err() || stream->eos() || (active && !_onGlobe))
		return false;
	if (!active)
		return true;
	_globeSpin = stream->readUint16LE();
	_globeTilt = stream->readSint16LE();
	_globePresentedSpin = stream->readUint16LE();
	_globeDrawing.position = stream->readUint16LE();
	_globeDrawing.row = stream->readSint16LE();
	const byte lower = stream->readByte();
	_globeDrawing.lowerHalf = lower != 0;
	if (_globeSpin >= Map::kEquatorWidth || _globePresentedSpin >= Map::kEquatorWidth ||
			_globeTilt <= -(int)Map::kLatitudes || _globeTilt >= (int)Map::kLatitudes || lower > 1 ||
			!_map.validGlobeProgress(_globeDrawing) || stream->err() || stream->eos())
		return false;
	_globeBackground.resize(kScreenWidth * kScreenHeight);
	_globePresented.resize(kScreenWidth * kScreenHeight);
	return stream->read(_globeBackground.begin(), _globeBackground.size()) == _globeBackground.size() &&
			stream->read(_globePresented.begin(), _globePresented.size()) == _globePresented.size() && !stream->err();
}

/** B400: successful menu Load restores the calling globe, then glides to Paul. */
void DuneEngine::restoreCallingGlobe(int spin, int tilt, int doors, byte vision) {
	if (!_onGlobe)
		teardownStrategicMap(false);
	_onGlobe = true; // Do not run B8C6/BA75's initial player centring again.
	_onMixer = false;
	_globeSpin = spin;
	_globeTilt = tilt;
	_frescoDoors = doors;
	_state[0xdd02] = vision;
	WRITE_LE_UINT16(_state.begin() + 0xdd0f, (uint16)doors);
	_resultsPage.clear();
	_resultsGaugeTime = 0;
	if (vision) {
		slideFrescoDoors(false); // B915, not B961: keep the calling vision byte.
		_state[0xdd02] = vision;
	}
	initialiseResultsGauges(); // B8F3/BE1D starts the newly loaded totals.
	slideFrescoDoors(true);
	_state[0xdd02] = vision - 1; // B96E decrements, including ff -> fe.
	showGlobe(); // B972/B941: full draw and Results command, without BA75.
	_globeAnimationTime = _system->getMillis() / 5; // B8EA reinstalls B9AE.
	glideToPlayerOnGlobe(); // BA9E uses the loaded physical position.
}

} // namespace Dune
