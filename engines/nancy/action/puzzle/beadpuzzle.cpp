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
#include "engines/nancy/ui/textbox.h"
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

	_beadCursorType = stream.readSint16LE();

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

	_solveScene._sceneChange.readData(stream);
	stream.skip(2);
	_exitScene._sceneChange.readData(stream);
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

	loadImage();

	_placed.clear();
	BeadPuzzleData *bpd = getBeadData();
	if (bpd) {
		for (uint i = 0; i < bpd->placedBeads.size() && i < (uint)_numSlots; ++i)
			_placed.push_back(bpd->placedBeads[i]);
	}

	_heldBead = -1;
	_movingBead = -1;
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
		_heldBeadObject.registerGraphics();
		_state = kRun;
		// fall through

	case kRun:
		switch (_subState) {
		case kPlaying:
			if ((int)_placed.size() >= (int)_numSlots && _heldBead == -1)
				evaluate();
			break;

		case kDroppingBead:
			// The place sound loops until the bead reaches its slot
			if (_placeSound.name != "NO SOUND" && !g_nancy->_sound->isSoundPlaying(_placeSound))
				g_nancy->_sound->playSound(_placeSound);

			if (g_system->getMillis() >= _moveNextTick) {
				if (_movingSlot <= (int)_placed.size()) {
					_placed.push_back(_movingBead);
					_movingBead = -1;
					g_nancy->_sound->stopSound(_placeSound);
					_subState = kPlaying;
					persistState();
				} else {
					--_movingSlot;
					_moveNextTick = g_system->getMillis() + kDropTickMs;
				}
				redraw();
			}
			break;

		case kRemovingBead:
			// The removed bead slides off the far end of the thread
			if (g_system->getMillis() >= _moveNextTick) {
				if (_movingSlot >= (int)_numSlots - 1) {
					_movingBead = -1;
					g_nancy->_sound->stopSound(_removeSound);
					_subState = kPlaying;
				} else {
					++_movingSlot;
					_moveNextTick = g_system->getMillis() + kDropTickMs;
				}
				redraw();
			}
			break;

		case kShowingResult:
			if (!_resultSoundPlayed) {
				playResultSound(_resultKind == kPartial ? _partialSound : _wrongSound);
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
		NancySceneState.changeScene(_subState == kExitToSolved ? _solveScene._sceneChange : _exitScene._sceneChange);
		finishExecution();
		break;
	}
}

void BeadPuzzle::handleInput(NancyInput &input) {
	if (_state != kRun)
		return;
	if (_subState == kDroppingBead ||
	    _subState == kRemovingBead ||
	    _subState == kPerfectWaitSound ||
	    _subState == kExitToSolved ||
	    _subState == kExitToDefault)
		return;

	Common::Rect vpScreen = NancySceneState.getViewport().getScreenPosition();
	Common::Point mouseVP = input.mousePos - Common::Point(vpScreen.left, vpScreen.top);

	_heldBeadObject.handleInput(input);

	if (_subState == kShowingResult) {
		if (!_resultHotspot.isEmpty() && _resultHotspot.contains(mouseVP)) {
			g_nancy->_cursor->setCursorType(CursorManager::kHotspot);
			if (input.input & NancyInput::kLeftMouseButtonUp) {
				g_nancy->_sound->stopSound(_partialSound);
				g_nancy->_sound->stopSound(_wrongSound);
				NancySceneState.getTextbox().clear();
				if (!_placed.empty())
					_placed.pop_back();
				holdBead(-1, nullptr);
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
			g_nancy->_cursor->setCursorType((CursorManager::CursorType)_beadCursorType, true);
			if (input.input & NancyInput::kLeftMouseButtonUp) {
				if ((int)_placed.size() < (int)_numSlots) {
					holdBead((int16)i, &input);
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
			g_nancy->_cursor->setCursorType((CursorManager::CursorType)_beadCursorType, true);
			if (input.input & NancyInput::kLeftMouseButtonUp) {
				_movingBead = _placed.back();
				_placed.pop_back();
				_movingSlot = (int16)_placed.size();
				_moveNextTick = g_system->getMillis() + kDropTickMs;
				_subState = kRemovingBead;
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
			g_nancy->_cursor->setCursorType((CursorManager::CursorType)_beadCursorType, true);
			if (input.input & NancyInput::kLeftMouseButtonUp) {
				g_nancy->_sound->stopSound(_pickupSound);
				if (_placeSound.name != "NO SOUND") {
					g_nancy->_sound->loadSound(_placeSound);
					g_nancy->_sound->playSound(_placeSound);
				}
				// The bead leaves the cursor and slides from the bottom of the
				// thread up to the next free slot before it is committed.
				_subState = kDroppingBead;
				_movingBead = _heldBead;
				_movingSlot = (int16)((int)_numSlots - 1);
				_moveNextTick = g_system->getMillis() + kDropTickMs;
				holdBead(-1, nullptr);
			}
			return;
		}

		if (_heldBead >= 0 && _heldBead < (int)_numBeadTypes &&
		    !_pickupHotspots[_heldBead].isEmpty() &&
		    _pickupHotspots[_heldBead].contains(mouseVP)) {
			g_nancy->_cursor->setCursorType((CursorManager::CursorType)_beadCursorType, true);
			if (input.input & NancyInput::kLeftMouseButtonUp) {
				holdBead(-1, nullptr);
				g_nancy->_sound->stopSound(_pickupSound);
				redraw();
			}
			return;
		}
	}

	if (hoverExitHotspot(input)) {
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
		playResultSound(_perfectSound);
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

// Plays a result remark and shows its caption, looked up by the sound name.
void BeadPuzzle::playResultSound(const SoundDescription &sound) {
	if (sound.name == "NO SOUND")
		return;

	g_nancy->_sound->loadSound(sound);
	g_nancy->_sound->playSound(sound);

	Common::String caption = resolveSubtitleText(sound.name);
	if (caption.empty())
		caption = resolveSubtitleText(sound.name, Common::String(), "CONVO");
	showSubtitle(caption);
}

// Puts a bead on the cursor, or takes the held one off it for a bead of -1. The
// input is only needed when picking one up, to place it under the cursor at once.
void BeadPuzzle::holdBead(int16 bead, NancyInput *input) {
	_heldBead = bead;

	if (_heldBead >= 0 && _heldBead < (int16)_numBeadTypes &&
			_image.getBounds().contains(_beadSrcRects[_heldBead])) {
		_heldBeadObject._drawSurface.create(_image, _beadSrcRects[_heldBead]);
		_heldBeadObject.setTransparent(true);
		_heldBeadObject.setVisible(true);
		_heldBeadObject.pickUp();
		if (input) {
			_heldBeadObject.handleInput(*input);
		}
	} else {
		_heldBeadObject.setVisible(false);
		_heldBeadObject.putDown();
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

	if ((_subState == kDroppingBead || _subState == kRemovingBead) &&
	    _movingBead >= 0 && _movingBead < (int)_numBeadTypes &&
	    _movingSlot >= 0 && _movingSlot < (int)_numSlots) {
		const Common::Rect &src = _beadSrcRects[_movingBead];
		const Common::Rect &dst = _slotDestRects[_movingSlot];
		if (!src.isEmpty() && !dst.isEmpty())
			_drawSurface.blitFrom(_image, src, Common::Point(dst.left, dst.top));
	}

	_needsRedraw = true;
}

} // End of namespace Action
} // End of namespace Nancy
