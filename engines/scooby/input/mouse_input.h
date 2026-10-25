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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef SCOOBY_MOUSE_INPUT_H
#define SCOOBY_MOUSE_INPUT_H

#include "common/events.h"
#include "common/rect.h"
#include "common/scummsys.h"

#include "controller_buttons.h"
#include "room_mouse_controller.h"

// MANAGED-ONLY MOUSE SUPPORT. Nothing in this file exists in the original game.
//
// The recovered game logic reads only the active-low controller byte. Mouse support is layered on top of it:
// mouse activity is turned into the same controller presses the original reads (a short "pulse"), or,
// on screens that opt in, into a directly hovered selection. With the mouse unused, every recovered input
// path behaves exactly as before.
//
// The device used last owns the pointer: any keyboard or pad action hides the mouse cursor and stops
// mouse-driven behaviour until the mouse moves or clicks again.
//
// Hook points in the recovered code are all marked "Managed-only mouse support":
//   - ControllerInput::pollControllers / processEvents / isIntroSkipPressed (event intake and pulses),
//   - ApplicationLoop::run and StudioLogoSequence::run (the startup intros, see Mode::Intro),
//   - SessionController::runGame (marks the idle room loop, see Mode::RoomIdle),
//   - RoomActorUpdater::tryOpenInteractionInterface (RuntimeState::MouseOpenInterfaceRequest),
//   - RoomSpriteRenderer::appendCursorOrLowerInterfaceSprite (RuntimeState::MouseCursorActive),
//   - MainMenuController::selectMainMenuOption, EpisodePasswordMenuController, SoundTestController and
//     InteractionActionMenuExecutor (hover and click selection).
//
// Not mouse-driven (keyboard or pad only): the d-pad mini-games, room-script actions 0x21, 0x27 and the
// duel (0x32). Clicks there still press B (left) and A (right).

namespace Scooby {

class ScoobyDooRom;
class TileScene;
class RuntimeState;

// The screen family sampling input; it selects the cursor image.
enum class InputScreen {
	Intro, // Startup logos and title intro: no pointer.
	Menu,  // Main menu, password entry and sound test: system arrow.
	Room   // Rooms, room scripts and dialogue: the game's targeting crosshair.
};

class MouseInput {
public:
	// How clicks are interpreted while a screen samples input.
	enum class Mode {
		// Left click presses B, right click presses A, the wheel presses Up (away) or Down (towards).
		Generic,
		// The running loop reads hover and clicks itself through contentPosition() and the take*() calls.
		Handled,
		// Sound test: left click plays (B), right click leaves (C), the wheel steps the track (Left/Right).
		SoundTest,
		// The idle room loop: cursor targeting, the verb bar and click-to-walk (RoomMouseController).
		RoomIdle,
		// Startup intros (title intro and studio logos): the pointer stays hidden and clicks never become
		// controller presses; a left click is left for isIntroSkipPressed to skip with. The studio logos poll
		// through the room retrace handler, so this mode also stops that poll treating the click as a room
		// click or showing the crosshair.
		Intro
	};

	MouseInput();

	// Loads the targeting crosshair from the room cursor sprite tiles and binds the scene used to convert
	// screen positions into content positions (H32 screens are centered in the 320-pixel frame).
	void attach(const ScoobyDooRom &rom, const TileScene &scene);

	// Event intake from ControllerInput::processEvents.
	void handleEvent(const Common::Event &event);
	void handleKeyboardOrPadInput();

	// Called by ControllerInput::pollControllers once per retrace sample, before and after event intake.
	void beginPoll();
	ControllerButtons sampleControllerButtons(RuntimeState &state, InputScreen screen);

	// Called by ControllerInput::isIntroSkipPressed: a left click skips like Escape. Mouse use during the
	// intros does not count as activity, so the pointer first appears once the mouse moves on the main menu.
	bool takeIntroSkipClick();

	// True while the mouse (rather than the keyboard or a pad) is the active device.
	bool isActive() const { return _active; }

	// Pointer position in the current scene's content coordinates (0-255 on H32 screens), the coordinate
	// space of the recovered sprites and cursor variables.
	Common::Point contentPosition() const { return toContent(_position); }

	// Consumes "the pointer moved" since the last call.
	bool takeMoved();

	// Consumes a left click (content position of the press) or a right click. A click is kept only for the
	// retrace sample it arrived in and the loop pass after it, so it never leaks into a later screen.
	bool takeLeftClick(Common::Point &contentPos);
	bool takeRightClick();

	// Queues a controller press held for kPulseSamples retrace samples, followed by one released sample.
	void pulse(ControllerButtons buttons);

	Mode mode() const { return _mode; }
	void setMode(Mode mode) { _mode = mode; }

private:
	enum class CursorStyle {
		Hidden,
		Arrow,
		Crosshair
	};

	static const int kCrosshairSize = 16;
	static const int kCrosshairHotspot = 7;
	static const int kMaxQueuedPulses = 4;
	// Some recovered loops sample input only every other retrace, so a press spans a few samples (a key
	// press lasts many frames); the original reacts to press edges, so the length is otherwise invisible.
	static const int kPulseSamples = 3;

	Common::Point toContent(const Common::Point &screenPos) const;
	void updateCursor(InputScreen screen);
	ControllerButtons mapGenericClicks();
	ControllerButtons mapSoundTestClicks();
	ControllerButtons emitPulse();

	const TileScene *_scene;
	RoomMouseController _room;
	Mode _mode;
	bool _active;
	bool _moved;
	Common::Point _position;
	bool _leftClick;
	Common::Point _leftClickPosition;
	uint32 _leftClickPoll;
	bool _rightClick;
	uint32 _rightClickPoll;
	int _wheel;
	uint32 _wheelPoll;
	uint32 _pollCount;
	CursorStyle _cursorStyle;
	byte _crosshairPixels[kCrosshairSize * kCrosshairSize];

	ControllerButtons _pulseQueue[kMaxQueuedPulses];
	int _pulseCount;
	int _pulseSamplesLeft;
	bool _pulseReleasePending;
};

// Selects a click interpretation for the lifetime of a recovered loop and restores the previous one.
class MouseModeScope {
public:
	MouseModeScope(MouseInput &mouse, MouseInput::Mode mode) : _mouse(mouse), _previous(mouse.mode()) {
		_mouse.setMode(mode);
	}
	~MouseModeScope() { _mouse.setMode(_previous); }

private:
	MouseInput &_mouse;
	MouseInput::Mode _previous;
};

} // namespace Scooby

#endif // SCOOBY_MOUSE_INPUT_H
