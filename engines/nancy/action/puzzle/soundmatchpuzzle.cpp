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
#include "engines/nancy/action/puzzle/soundmatchpuzzle.h"

namespace Nancy {
namespace Action {

void SoundMatchPuzzle::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _imageNameLitButtons);	// 0x000: overlay image (lit buttons)

	// 0x021: feedback sounds (loaded at init, played on whale button clicks)
	_feedbackSoundWrong.readNormal(stream);	// 0x021: played on incorrect whale click
	_feedbackSoundRight.readNormal(stream); // 0x052: played on correct whale click

	stream.skip(1);	// 0x083

	_winScene.readData(stream);	// 0x084
	_winSound.readNormal(stream);	// 0x09d
	_exitScene.readData(stream);	// 0x0ce

	readRect(stream, _exitHotspot);	// 0x0e7

	_requiredPairs = stream.readUint16LE();	// 0x0f7

	// 0x119: per-button entries (kNumButtons x 0x15c bytes each)
	// NOTE: The original tangled the sound button and whale button
	// data together, but we read them into separate structures for clarity.
	for (int i = 0; i < kNumButtons; ++i) {
		SoundButtonEntry &soundButton = _soundButtons[i];
		WhaleButtonEntry &whaleButton = _whaleButtons[i];

		readRect(stream, whaleButton.whaleSrcRect);  // 0x13c
		readRect(stream, whaleButton.whaleDestRect); // 0x14c

		soundButton.sound.readNormal(stream);    // 0x000

		stream.skip(33);               // 0x031: empty name field

		soundButton.text = stream.readString('\0', 200); // 0x052: whale sound subtitle

		whaleButton.correctSound = stream.readUint16LE(); // 0x11a

		readRect(stream, soundButton.numSrcRect);    // 0x11c
		readRect(stream, soundButton.numDestRect);   // 0x12c
	}
}

void SoundMatchPuzzle::init() {
	Common::Rect vpBounds = NancySceneState.getViewport().getBounds();
	_drawSurface.create(vpBounds.width(), vpBounds.height(),
	                    g_nancy->_graphics->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	setTransparent(true);
	setVisible(true);
	moveTo(vpBounds);

	g_nancy->_resource->loadImage(_imageNameLitButtons, _imageLitButtons);
	_imageLitButtons.setTransparentColor(_drawSurface.getTransparentColor());

	_selectedSoundButton = -1;

	for (int i = 0; i < kNumButtons; ++i) {
		_soundButtons[i].matched = false;
		_whaleButtons[i].matched = false;
	}

	_matchedPairs  = 0;
	_isExiting     = false;
	_solveSubState = kIdle;

	redraw();
}

void SoundMatchPuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();

		// Load feedback sounds so they are ready for playback in handleInput
		if (_feedbackSoundWrong.name != "NO SOUND")
			g_nancy->_sound->loadSound(_feedbackSoundWrong);
		if (_feedbackSoundRight.name != "NO SOUND")
			g_nancy->_sound->loadSound(_feedbackSoundRight);

		_state = kRun;
		// fall through

	case kRun:
		switch (_solveSubState) {
		case kIdle:
			break;

		case kSoundPlaying:
			// Per-button (whale call) sound is playing. If it stops naturally
			// before the player picks a whale, deselect and return to idle.
			if (!g_nancy->_sound->isSoundPlaying(_soundButtons[_selectedSoundButton].sound)) {
				g_nancy->_sound->stopSound(_soundButtons[_selectedSoundButton].sound);
				NancySceneState.getTextbox().clear();
				_selectedSoundButton = -1;
				redraw();
				_solveSubState = kIdle;
			}
			break;

		case kCheckMatch:
			// A whale button was clicked.
			// Correct: wait for _feedbackSoundRight to stop, then check win.
			// Wrong:   wait for _feedbackSoundWrong to stop, then back to idle.
			if (_soundButtons[_selectedSoundButton].matched) {
				if (_feedbackSoundRight.name == "NO SOUND" ||
				    !g_nancy->_sound->isSoundPlaying(_feedbackSoundRight)) {
					if (_matchedPairs >= _requiredPairs) {
						if (_winSound.name != "NO SOUND") {
							g_nancy->_sound->loadSound(_winSound);
							g_nancy->_sound->playSound(_winSound);
						}
						_solveSubState = kWinSound;
					} else {
						_selectedSoundButton = -1;
						_solveSubState  = kIdle;
					}
				}
			} else {
				if (_feedbackSoundWrong.name == "NO SOUND" ||
				    !g_nancy->_sound->isSoundPlaying(_feedbackSoundWrong)) {
					_selectedSoundButton = -1;
					_solveSubState  = kIdle;
				}
			}
			redraw();
			break;

		case kWinSound:
			if (_winSound.name == "NO SOUND" ||
			    !g_nancy->_sound->isSoundPlaying(_winSound)) {
				g_nancy->_sound->stopSound(_winSound);
				_state = kActionTrigger;
			}
			break;
		}
		break;

	case kActionTrigger:
		g_nancy->_sound->stopSound(_feedbackSoundWrong);
		g_nancy->_sound->stopSound(_feedbackSoundRight);
		if (_selectedSoundButton >= 0)
			g_nancy->_sound->stopSound(_soundButtons[_selectedSoundButton].sound);
		g_nancy->_sound->stopSound(_winSound);
		if (_isExiting)
			_exitScene.execute();
		else
			_winScene.execute();
		finishExecution();
		break;
	}
}

void SoundMatchPuzzle::handleInput(NancyInput &input) {
	if (_state != kRun || _matchedPairs >= _requiredPairs)
		return;

	Common::Rect vpScreen = NancySceneState.getViewport().getScreenPosition();
	Common::Point mouseVP = input.mousePos - Common::Point(vpScreen.left, vpScreen.top);

	// Numbered button clicks — accepted in idle or while a sound is playing
	if (_solveSubState == kIdle || _solveSubState == kSoundPlaying) {
		for (int i = 0; i < kNumButtons; ++i) {
			if (_soundButtons[i].matched)
				continue; // already matched; numbered button is inactive
			if (!_soundButtons[i].numDestRect.contains(mouseVP))
				continue;

			g_nancy->_cursor->setCursorType(CursorManager::kHotspot);

			if (input.input & NancyInput::kLeftMouseButtonUp) {
				// Stop any currently playing per-button sound
				if (_selectedSoundButton >= 0)
					g_nancy->_sound->stopSound(_soundButtons[_selectedSoundButton].sound);

				_selectedSoundButton = i;

				if (_soundButtons[i].sound.name != "NO SOUND") {
					g_nancy->_sound->loadSound(_soundButtons[i].sound);
					g_nancy->_sound->playSound(_soundButtons[i].sound);
				}

				NancySceneState.getTextbox().clear();
				if (!_soundButtons[i].text.empty())
					NancySceneState.getTextbox().addTextLine(_soundButtons[i].text);

				redraw();
				_solveSubState = kSoundPlaying;
			}
			return;
		}
	}

	// Whale button clicks — only while a numbered button is selected and its sound plays
	if (_solveSubState == kSoundPlaying) {
		for (uint16 whaleButton = 0; whaleButton < kNumButtons; ++whaleButton) {
			if (!_whaleButtons[whaleButton].whaleDestRect.contains(mouseVP))
				continue;

			g_nancy->_cursor->setCursorType(CursorManager::kHotspot);

			if (input.input & NancyInput::kLeftMouseButtonUp) {
				// Stop the per-button sound now that the player has made a choice
				g_nancy->_sound->stopSound(_soundButtons[_selectedSoundButton].sound);
				NancySceneState.getTextbox().clear();

				uint16 soundButton = _soundButtonIndex[_selectedSoundButton] + 1;

				if (soundButton == _whaleButtons[whaleButton].correctSound && !_whaleButtons[whaleButton].matched) {
					// Correct whale - match!
					_soundButtons[_selectedSoundButton].matched = true;
					_whaleButtons[whaleButton].matched = true;
					++_matchedPairs;
					if (_feedbackSoundRight.name != "NO SOUND")
						g_nancy->_sound->playSound(_feedbackSoundRight);
				} else {
					// Wrong whale
					if (_feedbackSoundWrong.name != "NO SOUND")
						g_nancy->_sound->playSound(_feedbackSoundWrong);
				}

				redraw();
				_solveSubState = kCheckMatch;
			}
			return;
		}
	}

	if (_exitHotspot.contains(mouseVP)) {
		g_nancy->_cursor->setCursorType(g_nancy->_cursor->_puzzleExitCursor);
		if (input.input & NancyInput::kLeftMouseButtonUp) {
			_isExiting = true;
			_state = kActionTrigger;
		}
	}
}

void SoundMatchPuzzle::redraw() {
	_drawSurface.clear(_drawSurface.getTransparentColor());

	for (int i = 0; i < kNumButtons; ++i) {
		if (_soundButtons[i].matched || i == _selectedSoundButton) {
			const Common::Rect &dest = _soundButtons[i].numDestRect;
			_drawSurface.blitFrom(_imageLitButtons, _soundButtons[i].numSrcRect,
			                      Common::Point(dest.left, dest.top));
		}

		if (_whaleButtons[i].matched) {
			const Common::Rect &dest = _whaleButtons[i].whaleDestRect;
			_drawSurface.blitFrom(_imageLitButtons, _whaleButtons[i].whaleSrcRect,
			                      Common::Point(dest.left, dest.top));
		}
	}

	_needsRedraw = true;
}

} // End of namespace Action
} // End of namespace Nancy
