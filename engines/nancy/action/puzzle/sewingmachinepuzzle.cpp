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

#include "engines/nancy/state/scene.h"
#include "engines/nancy/action/puzzle/sewingmachinepuzzle.h"

namespace Nancy {
namespace Action {

// Size of a single drawn stitch, in pixels.
static const int kStitchSize = 2;

// Fallback seam x used if the mask scan fails.
static const int kSeamStartX = 490;

// Scene event flag that switches the needle between its animated and static
// overlays (true while the cloth is being fed).
static const int16 kNeedleAnimFlag = 1004;

void SewingMachinePuzzle::readData(Common::SeekableReadStream &stream) {
	// 87-byte PuzzleBase header blob.
	readFilename(stream, _imageName);	// blob 0x00
	readRect(stream, _rects[0]);		// blob 0x21
	readRect(stream, _rects[1]);		// blob 0x31
	_directionVector.x = stream.readSint32LE();	// blob 0x41
	_directionVector.y = stream.readSint32LE();	// blob 0x45
	_extentVector.x = stream.readSint32LE();	// blob 0x49
	_extentVector.y = stream.readSint32LE();	// blob 0x4d
	for (int i = 0; i < 3; ++i) {
		_params[i] = stream.readSint16LE();		// blob 0x51 / 0x53 / 0x55
	}

	_soundBlock.readData(stream);

	readActionZoneArray(stream, _zones);
}

void SewingMachinePuzzle::classifyZones() {
	for (uint i = 0; i < _zones.size(); ++i) {
		const ActionZone &z = _zones[i];
		switch (z.type) {
		case kZoneEventFlag:	// seam mask + mistake lines
			_collisionZone = i;
			break;
		case kZoneSceneChange:	// bottom completion trigger
			_triggerZones.push_back(i);
			break;
		case kZoneBoundary:	// play-area boundary
			_boundaryZone = i;
			break;
		default:
			break;
		}
	}
}

void SewingMachinePuzzle::playSoundBlock(const RandomSoundBlock &block) {
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

	// The mistake lines carry no inline caption; look the subtitle up by sound name,
	// first in the Autotext table, then in the conversation table.
	Common::String text = resolveSubtitleText(name);
	if (text.empty())
		text = resolveSubtitleText(name, Common::String(), "CONVO");
	showSubtitle(text);
}

Common::Point SewingMachinePuzzle::needleInStrip() const {
	// The needle is fixed on screen; map it back into cloth space.
	return _needleScreen - _offset;
}

void SewingMachinePuzzle::drawCloth() {
	// Draw the cloth at native size wherever it has been dragged; only the part
	// within the viewport is blitted, so the scene shows where the cloth is not.
	Common::Rect src(-_offset.x, -_offset.y, -_offset.x + _drawSurface.w, -_offset.y + _drawSurface.h);
	src.clip(Common::Rect(_image.w, _image.h));

	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	if (!src.isEmpty()) {
		Common::Point dest(src.left + _offset.x, src.top + _offset.y);
		_drawSurface.blitFrom(_image, src, dest);
	}

	// Stitches sewn so far, as a dark dashed thread.
	uint32 stitchColor = _drawSurface.format.RGBToColor(0, 0, 0);
	Common::Rect surfaceBounds(_drawSurface.w, _drawSurface.h);
	for (uint i = 0; i < _stitches.size(); ++i) {
		int sx = _stitches[i].x + _offset.x;
		int sy = _stitches[i].y + _offset.y;
		Common::Rect dot(sx, sy, sx + kStitchSize, sy + kStitchSize);
		dot.clip(surfaceBounds);
		if (!dot.isEmpty()) {
			_drawSurface.fillRect(dot, stitchColor);
		}
	}

	_needsRedraw = true;
}

void SewingMachinePuzzle::feedCloth(const Common::Point &delta) {
	// Dragging moves the cloth: vertical feeds it, horizontal steers it.
	Common::Point newOffset(CLIP<int>(_offset.x + delta.x, _minOffsetX, _maxOffsetX),
		CLIP<int>(_offset.y + delta.y, _minOffsetY, _maxOffsetY));

	int moved = ABS(newOffset.x - _offset.x) + ABS(newOffset.y - _offset.y);
	_offset = newOffset;

	if (moved == 0) {
		// The machine's needle only runs while the cloth is moving.
		NancySceneState.setEventFlag(kNeedleAnimFlag, g_nancy->_false);
		return;
	}

	// Run the needle animation (the scene swaps in its animated overlay).
	NancySceneState.setEventFlag(kNeedleAnimFlag, g_nancy->_true);

	// Lay down a stitch each time the cloth advances past the spacing.
	Common::Point needle = needleInStrip();
	int spacing = MAX<int>(1, _params[1]);
	if (_stitches.empty() ||
		ABS(needle.x - _stitches.back().x) + ABS(needle.y - _stitches.back().y) >= spacing) {
		_stitches.push_back(needle);
	}

	drawCloth();
	checkSeam();

	// Reaching a bottom trigger whose mistake-flag gate is satisfied finishes the seam.
	if (!_solved) {
		Common::Point end = needleInStrip();
		for (uint i = 0; i < _triggerZones.size(); ++i) {
			if (triggerFires(_zones[_triggerZones[i]], end)) {
				_solved = true;
				_state = kActionTrigger;
				break;
			}
		}
	}
}

bool SewingMachinePuzzle::triggerFires(const ActionZone &z, const Common::Point &needle) const {
	if (!z.rect.contains(needle)) {
		return false;
	}

	// Both triggers gate on the mistake flag (val49): the narrow zone fires only on a
	// clean run (flag clear, val4b 0 -> EV_Solved_Dress), the wide zone only when a
	// mistake was made (flag set, val4b 1 -> EV_Tried_Dress), so they are exclusive.
	return z.val49 == kFlagNoLabel || NancySceneState.getEventFlag(z.val49, z.val4b);
}

void SewingMachinePuzzle::checkSeam() {
	if (!_hasSeamMask || _collisionZone < 0) {
		return;
	}

	Common::Point needle = needleInStrip();
	int mx = (int)((needle.x - _maskOrigin.x) * _maskScaleX);
	int my = (int)((needle.y - _maskOrigin.y) * _maskScaleY);

	// The needle strays off the seam when, while inside the collision region, it lands
	// on the mask's background instead of the marked corridor.
	bool off = mx >= 0 && my >= 0 && mx < _seamMask.w && my < _seamMask.h &&
		_seamMask.getPixel(mx, my) == _offSeamColor;

	// Edge-triggered: comment once on leaving the seam, re-armed only once the needle
	// returns to it.
	if (off && !_offSeam) {
		const ActionZone &seam = _zones[_collisionZone];
		playSoundBlock(seam._sound);
		NancySceneState.setEventFlag(seam.tailId, seam.tailFlag ? g_nancy->_true : g_nancy->_false);
	}

	_offSeam = off;
}

void SewingMachinePuzzle::init() {
	Common::Rect vpBounds = NancySceneState.getViewport().getBounds();
	_drawSurface.create(vpBounds.width(), vpBounds.height(),
		g_nancy->_graphics->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	setTransparent(true);
	setVisible(true);
	moveTo(vpBounds);

	g_nancy->_resource->loadImage(_imageName, _image);
	_image.setTransparentColor(_drawSurface.getTransparentColor());

	// The needle's fixed sewing point = the bottom-center of the needle overlay's
	// dest rect (167,0,285,170).
	_needleScreen = Common::Point((167 + 285) / 2, 170);

	// Clamp so the needle can reach any point on the freely dragged cloth.
	_maxOffsetX = _needleScreen.x;
	_minOffsetX = _needleScreen.x - _image.w;
	_maxOffsetY = _needleScreen.y;
	_minOffsetY = _needleScreen.y - _image.h;

	// Fill from the top; the seam x under the needle is refined by the mask scan below.
	_offset.y = CLIP<int>(0, _minOffsetY, _maxOffsetY);
	_offset.x = CLIP<int>(_needleScreen.x - kSeamStartX, _minOffsetX, _maxOffsetX);

	// The collision zone's overlay is a per-pixel mask of the seam path.
	if (_collisionZone < 0 || _zones[_collisionZone].ovlName.empty()) {
		return;
	}

	g_nancy->_resource->loadImage(Common::Path(_zones[_collisionZone].ovlName), _seamMask);

	// The mask covers only the cloth's seam region = the collision zone's rect, so
	// cloth pixels map to mask pixels by that origin + scale.
	Common::Rect region = _zones[_collisionZone].rect;
	if (region.width() <= 0 || region.height() <= 0) {
		return;
	}
	_maskOrigin = Common::Point(region.left, region.top);
	_maskScaleX = (double)_seamMask.w / region.width();
	_maskScaleY = (double)_seamMask.h / region.height();

	// Dark corridor on a light background: sample the background at a corner, then
	// center the needle on the corridor at its row.
	_offSeamColor = _seamMask.getPixel(0, 0);
	int maskRow = CLIP<int>((int)((needleInStrip().y - _maskOrigin.y) * _maskScaleY), 0, _seamMask.h - 1);
	int first = -1, last = -1;
	for (int x = 0; x < _seamMask.w; ++x) {
		if (_seamMask.getPixel(x, maskRow) != _offSeamColor) {
			if (first < 0) {
				first = x;
			}
			last = x;
		}
	}

	if (first >= 0) {
		int seamClothX = _maskOrigin.x + (int)(((first + last) / 2) / _maskScaleX);
		_offset.x = CLIP<int>(_needleScreen.x - seamClothX, _minOffsetX, _maxOffsetX);
		_hasSeamMask = true;
	} else {
		warning("SewingMachinePuzzle: seam mask '%s' has no seam on the needle row; off-seam detection disabled",
			_zones[_collisionZone].ovlName.c_str());
	}
}

void SewingMachinePuzzle::execute() {
	switch (_state) {
	case kBegin:
		classifyZones();	// init() needs the seam zone's mask
		init();
		registerGraphics();
		playSoundBlock(_soundBlock);	// start the sewing-machine ambience
		drawCloth();
		_state = kRun;
		break;
	case kRun:
		break;
	case kActionTrigger: {
		// Raise the flag for whichever bottom trigger fires: a clean run finishes in the
		// narrow zone (EV_Solved_Dress), a mistaken one in the wide zone (EV_Tried_Dress).
		// They share the win scene, so cross-dissolve to it once.
		Common::Point end = needleInStrip();
		const ActionZone *sceneZone = nullptr;
		for (uint i = 0; i < _triggerZones.size(); ++i) {
			const ActionZone &tz = _zones[_triggerZones[i]];
			if (!triggerFires(tz, end)) {
				continue;
			}
			if (tz.tailId != -1) {
				NancySceneState.setEventFlag(tz.tailId, tz.tailFlag ? g_nancy->_true : g_nancy->_false);
			}
			if (!sceneZone) {
				sceneZone = &tz;
			}
		}

		if (sceneZone && sceneZone->specialEffectId >= 1000) {
			if (sceneZone->hasSpecialEffect) {
				NancySceneState.specialEffect(sceneZone->seType, sceneZone->seTotalTime, sceneZone->seFadeToBlackTime, sceneZone->seRect);
			}
			SceneChangeDescription scene;
			scene.sceneID = sceneZone->specialEffectId;
			NancySceneState.changeScene(scene);
		}
		finishExecution();
		break;
	}
	}
}

void SewingMachinePuzzle::handleInput(NancyInput &input) {
	if (_state != kRun) {
		return;
	}

	// Cheat: Ctrl+Shift+C forgives any mistakes, clearing the collision flag so a
	// finished seam still counts as a clean solve. The off-seam latch is left as-is so
	// clearing while still off the line doesn't instantly re-flag; the next fresh stray
	// does.
	for (uint i = 0; i < input.otherKbdInput.size(); ++i) {
		const Common::KeyState &key = input.otherKbdInput[i];
		if ((key.flags & Common::KBD_CTRL) && (key.flags & Common::KBD_SHIFT) &&
				key.keycode == Common::KEYCODE_c && _collisionZone >= 0) {
			NancySceneState.setEventFlag(_zones[_collisionZone].tailId, g_nancy->_false);
			debug("Sewing cheat: mistakes cleared");
		}
	}

	g_nancy->_cursor->setCursorType(CursorManager::kHotspot);

	if (input.input & NancyInput::kLeftMouseButtonDown) {
		_dragging = true;
		_lastDragPos = input.mousePos;
	}

	if (_dragging && (input.input & NancyInput::kLeftMouseButtonHeld)) {
		feedCloth(input.mousePos - _lastDragPos);
		_lastDragPos = input.mousePos;
	}

	if (input.input & NancyInput::kLeftMouseButtonUp) {
		_dragging = false;
		NancySceneState.setEventFlag(kNeedleAnimFlag, g_nancy->_false);
	}
}

} // End of namespace Action
} // End of namespace Nancy
