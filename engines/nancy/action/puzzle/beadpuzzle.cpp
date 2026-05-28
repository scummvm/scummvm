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
#include "engines/nancy/puzzledata.h"
#include "engines/nancy/action/puzzle/beadpuzzle.h"

namespace Nancy {
namespace Action {

static BeadPuzzleData *getBeadData() {
	return (BeadPuzzleData *)NancySceneState.getPuzzleData(BeadPuzzleData::getTag());
}

static const uint32 kDropTickMs = 40;

void BeadPuzzle::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _imageName);

	_numSlots     = stream.readUint16LE();
	_numBeadTypes = stream.readUint16LE();

	for (int i = 0; i < kMaxBeadTypes; ++i)
		readRect(stream, _beadSrcRects[i]);

	readRect(stream, _threadSrc);
	readRect(stream, _threadDest);
	readRect(stream, _removeHotspot);
	readRect(stream, _resultHotspot);

	for (int i = 0; i < kMaxBeadTypes; ++i)
		readRect(stream, _pickupHotspots[i]);

	for (int i = 0; i < kMaxSlots; ++i)
		readRect(stream, _slotDestRects[i]);

	for (int i = 0; i < kMaxSlots; ++i)
		readRect(stream, _resultDestRects[i]);

	// Solution stores beadType+1 per slot; 0 marks unused tail entries.
	for (int i = 0; i < kMaxSlots; ++i)
		_solution[i] = stream.readUint16LE();

	stream.skip(2);

	_pickupSound.readNormal(stream);
	_placeSound.readNormal(stream);
	_removeSound.readNormal(stream);

	_mistakeThreshold = stream.readSint16LE();

	_partialSound.readNormal(stream);
	_wrongSound.readNormal(stream);

	_partialFlag.label = stream.readSint16LE();
	_partialFlag.flag  = stream.readByte();

	_perfectSound.readNormal(stream);

	_perfectFlag.label = stream.readSint16LE();
	_perfectFlag.flag  = stream.readByte();

	_defaultScene.readData(stream);
	stream.skip(2);
	_solvedScene.readData(stream);
	stream.skip(2);

	readRect(stream, _exitHotspot);

	if (_numSlots > kMaxSlots)
		_numSlots = kMaxSlots;
	if (_numBeadTypes > kMaxBeadTypes)
		_numBeadTypes = kMaxBeadTypes;
}

void BeadPuzzle::init() {
	Common::Rect vpBounds = NancySceneState.getViewport().getBounds();
	_drawSurface.create(vpBounds.width(), vpBounds.height(),
		g_nancy->_graphics->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	setTransparent(true);
	setVisible(true);
	moveTo(vpBounds);

	g_nancy->_resource->loadImage(_imageName, _image);
	_image.setTransparentColor(_drawSurface.getTransparentColor());

	_placed.clear();
	BeadPuzzleData *bpd = getBeadData();
	if (bpd) {
		for (uint i = 0; i < bpd->placedBeads.size() && i < (uint)_numSlots; ++i)
			_placed.push_back(bpd->placedBeads[i]);
	}

	_heldBead = -1;
	_subState = kPlaying;
	_resultKind = kNoResult;
	_resultSoundPlayed = false;

	redraw();
}

void BeadPuzzle::persistState() {
	BeadPuzzleData *bpd = getBeadData();
	if (!bpd)
		return;
	bpd->placedBeads.clear();
	for (uint i = 0; i < _placed.size(); ++i)
		bpd->placedBeads.push_back(_placed[i]);
}

void BeadPuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		_state = kRun;
		// fall through

	case kRun:
		switch (_subState) {
		case kPlaying:
			if ((int)_placed.size() >= (int)_numSlots && _heldBead == -1)
				evaluate();
			break;

		case kDroppingBead:
			if (g_system->getMillis() >= _dropNextTick) {
				if (_dropCurrentSlot <= (int)_placed.size()) {
					_placed.push_back(_heldBead);
					_heldBead = -1;
					_subState = kPlaying;
					persistState();
				} else {
					--_dropCurrentSlot;
					_dropNextTick = g_system->getMillis() + kDropTickMs;
				}
				redraw();
			}
			break;

		case kShowingResult:
			if (!_resultSoundPlayed) {
				const SoundDescription *snd = nullptr;
				if (_resultKind == kPartial)      snd = &_partialSound;
				else if (_resultKind == kWrong)   snd = &_wrongSound;
				if (snd && snd->name != "NO SOUND") {
					g_nancy->_sound->loadSound(*snd);
					g_nancy->_sound->playSound(*snd);
				}
				_resultSoundPlayed = true;
			}
			break;

		case kPerfectWaitSound:
			if (g_system->getMillis() >= _perfectExitTime &&
			    !g_nancy->_sound->isSoundPlaying(_perfectSound)) {
				g_nancy->_sound->stopSound(_perfectSound);
				_subState = kExitToSolved;
			}
			break;

		case kExitToSolved:
		case kExitToDefault:
			_state = kActionTrigger;
			break;
		}
		break;

	case kActionTrigger:
		g_nancy->_sound->stopSound(_pickupSound);
		g_nancy->_sound->stopSound(_placeSound);
		g_nancy->_sound->stopSound(_removeSound);
		g_nancy->_sound->stopSound(_partialSound);
		g_nancy->_sound->stopSound(_wrongSound);
		g_nancy->_sound->stopSound(_perfectSound);
		{
			const SceneChangeDescription &dest = (_subState == kExitToSolved) ? _solvedScene : _defaultScene;
			if (dest.sceneID != kNoScene)
				NancySceneState.changeScene(dest);
		}
		finishExecution();
		break;
	}
}

void BeadPuzzle::handleInput(NancyInput &input) {
	if (_state != kRun)
		return;
	if (_subState == kDroppingBead ||
	    _subState == kPerfectWaitSound ||
	    _subState == kExitToSolved ||
	    _subState == kExitToDefault)
		return;

	Common::Rect vpScreen = NancySceneState.getViewport().getScreenPosition();
	Common::Point mouseVP = input.mousePos - Common::Point(vpScreen.left, vpScreen.top);

	if (_heldBead != -1 && _heldDrawPos != mouseVP) {
		_heldDrawPos = mouseVP;
		redraw();
	}

	if (_subState == kShowingResult) {
		if (!_resultHotspot.isEmpty() && _resultHotspot.contains(mouseVP)) {
			g_nancy->_cursor->setCursorType(CursorManager::kHotspot);
			if (input.input & NancyInput::kLeftMouseButtonUp) {
				g_nancy->_sound->stopSound(_partialSound);
				g_nancy->_sound->stopSound(_wrongSound);
				if (!_placed.empty())
					_placed.pop_back();
				_heldBead = -1;
				_subState = kPlaying;
				_resultKind = kNoResult;
				_resultSoundPlayed = false;
				persistState();
				redraw();
			}
			return;
		}
	} else if (_heldBead == -1) {
		for (int i = 0; i < (int)_numBeadTypes; ++i) {
			if (_pickupHotspots[i].isEmpty() || !_pickupHotspots[i].contains(mouseVP))
				continue;
			g_nancy->_cursor->setCursorType(CursorManager::kHotspot);
			if (input.input & NancyInput::kLeftMouseButtonUp) {
				if ((int)_placed.size() < (int)_numSlots) {
					_heldBead = (int16)i;
					_heldDrawPos = mouseVP;
					if (_pickupSound.name != "NO SOUND") {
						g_nancy->_sound->loadSound(_pickupSound);
						g_nancy->_sound->playSound(_pickupSound);
					}
					redraw();
				}
			}
			return;
		}

		if (!_removeHotspot.isEmpty() && _removeHotspot.contains(mouseVP) && !_placed.empty()) {
			g_nancy->_cursor->setCursorType(CursorManager::kHotspot);
			if (input.input & NancyInput::kLeftMouseButtonUp) {
				_placed.pop_back();
				if (_removeSound.name != "NO SOUND") {
					g_nancy->_sound->loadSound(_removeSound);
					g_nancy->_sound->playSound(_removeSound);
				}
				persistState();
				redraw();
			}
			return;
		}
	} else {
		if ((int)_placed.size() < (int)_numSlots &&
		    !_removeHotspot.isEmpty() &&
		    _removeHotspot.contains(mouseVP)) {
			g_nancy->_cursor->setCursorType(CursorManager::kDragHand);
			if (input.input & NancyInput::kLeftMouseButtonUp) {
				g_nancy->_sound->stopSound(_pickupSound);
				if (_placeSound.name != "NO SOUND") {
					g_nancy->_sound->loadSound(_placeSound);
					g_nancy->_sound->playSound(_placeSound);
				}
				// Animate the bead sliding from the bottom of the thread up
				// to the next free slot before committing it.
				_subState = kDroppingBead;
				_dropCurrentSlot = (int16)((int)_numSlots - 1);
				_dropNextTick = g_system->getMillis() + kDropTickMs;
				redraw();
			}
			return;
		}

		if (_heldBead >= 0 && _heldBead < (int)_numBeadTypes &&
		    !_pickupHotspots[_heldBead].isEmpty() &&
		    _pickupHotspots[_heldBead].contains(mouseVP)) {
			g_nancy->_cursor->setCursorType(CursorManager::kDragHand);
			if (input.input & NancyInput::kLeftMouseButtonUp) {
				_heldBead = -1;
				g_nancy->_sound->stopSound(_pickupSound);
				redraw();
			}
			return;
		}
	}

	if (!_exitHotspot.isEmpty() && _exitHotspot.contains(mouseVP)) {
		g_nancy->_cursor->setCursorType(g_nancy->_cursor->_puzzleExitCursor);
		if (input.input & NancyInput::kLeftMouseButtonUp)
			_subState = kExitToDefault;
	}
}

void BeadPuzzle::evaluate() {
	int mismatches = 0;
	for (int i = 0; i < (int)_numSlots; ++i) {
		int expected = (int)_solution[i] - 1;
		int actual   = _placed[i];
		if (actual != expected)
			++mismatches;
	}

	if (mismatches == 0) {
		_resultKind = kPerfect;
		if (_perfectFlag.label != -1)
			NancySceneState.setEventFlag(_perfectFlag);
		if (_perfectSound.name != "NO SOUND") {
			g_nancy->_sound->loadSound(_perfectSound);
			g_nancy->_sound->playSound(_perfectSound);
		}
		_perfectExitTime = g_system->getMillis() + 4000;
		_subState = kPerfectWaitSound;
	} else if (mismatches > _mistakeThreshold) {
		_resultKind = kWrong;
		_subState = kShowingResult;
	} else {
		_resultKind = kPartial;
		if (_partialFlag.label != -1)
			NancySceneState.setEventFlag(_partialFlag);
		_subState = kShowingResult;
	}

	redraw();
}

void BeadPuzzle::redraw() {
	_drawSurface.clear(_drawSurface.getTransparentColor());

	if (!_threadSrc.isEmpty() && !_threadDest.isEmpty())
		_drawSurface.blitFrom(_image, _threadSrc, Common::Point(_threadDest.left, _threadDest.top));

	const bool resultMode = (_subState == kShowingResult ||
	                         _subState == kPerfectWaitSound);
	const Common::Rect *dests = resultMode ? _resultDestRects : _slotDestRects;

	for (int i = 0; i < (int)_placed.size(); ++i) {
		int t = _placed[i];
		if (t < 0 || t >= (int)_numBeadTypes)
			continue;
		const Common::Rect &src = _beadSrcRects[t];
		const Common::Rect &dst = dests[i];
		if (src.isEmpty() || dst.isEmpty())
			continue;
		_drawSurface.blitFrom(_image, src, Common::Point(dst.left, dst.top));
	}

	if (_subState == kDroppingBead && _heldBead >= 0 && _heldBead < (int)_numBeadTypes &&
	    _dropCurrentSlot >= 0 && _dropCurrentSlot < (int)_numSlots) {
		const Common::Rect &src = _beadSrcRects[_heldBead];
		const Common::Rect &dst = _slotDestRects[_dropCurrentSlot];
		if (!src.isEmpty() && !dst.isEmpty())
			_drawSurface.blitFrom(_image, src, Common::Point(dst.left, dst.top));
	} else if (_subState == kPlaying && _heldBead >= 0 && _heldBead < (int)_numBeadTypes) {
		const Common::Rect &src = _beadSrcRects[_heldBead];
		if (!src.isEmpty()) {
			int x = _heldDrawPos.x - src.width()  / 2;
			int y = _heldDrawPos.y - src.height() / 2;
			_drawSurface.blitFrom(_image, src, Common::Point(x, y));
		}
	}

	_needsRedraw = true;
}

} // End of namespace Action
} // End of namespace Nancy
