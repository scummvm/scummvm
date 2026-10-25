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

#ifndef SCOOBY_ROOM_MOUSE_CONTROLLER_H
#define SCOOBY_ROOM_MOUSE_CONTROLLER_H

#include "common/rect.h"
#include "common/scummsys.h"

#include "controller_buttons.h"

// MANAGED-ONLY MOUSE SUPPORT (see mouse_input.h). Nothing in this file exists in the original game.
//
// Drives the original room cursor from the mouse while the room loop is idle. It only writes the recovered
// cursor variables and produces the controller presses a pad player would make, so every selection still
// runs through the original logic:
//   - Moving the mouse opens the original cursor interface (as A/B would) and places the crosshair or the
//     verb-bar highlight under the pointer, so hovering shows object names as the original cursor does.
//   - Left click on a verb, an inventory item or a page arrow presses B with the highlight on that entry; the
//     C arrow presses C (verbs/inventory toggle).
//   - Left click on an object presses B (performing the chosen verb, else the object's default verb). The
//     press waits for the double-click window first.
//   - Double-click on an object walks Shaggy into it, so an object with an original walk-in interaction (a
//     doorway, stairs) takes him through. A single click on an object whose default verb is SHUT (an open
//     door) does the same when no verb was chosen in the bar; choosing SHUT first still shuts it.
//   - Left click on room space with no object closes the cursor (A) and walks Shaggy towards the point by
//     holding the d-pad, using the original movement and collision code. Walking stops at the point, when no
//     progress is made (blocked: Shaggy ends as near as the original movement allows), or on any new input.
//   - While "use with" waits for its second target, any room click presses B, as the pad would.
//   - Right click cancels: it stops a walk, otherwise it presses A (closes the cursor interface).
//
// A click is decided one retrace after the cursor is placed on it, so the original main loop has resolved
// the object (CurrentInteraction) or bar entry (MenuCommand) under the click before any press.

namespace Scooby {

class MouseInput;
class ScoobyDooRom;
class RuntimeState;

class RoomMouseController {
public:
	RoomMouseController();

	// Binds the ROM used to look up object shapes for walk-in targets.
	void attach(const ScoobyDooRom &rom) { _rom = &rom; }

	// Idle room retrace sample: returns the d-pad directions to hold for a walk (pulses go through mouse).
	ControllerButtons updateIdle(RuntimeState &state, MouseInput &mouse);

	// Any other room retrace sample (scripts, dialogue, transitions): a pending click is dropped, walking
	// pauses, and a walk interrupted for more than a couple of samples is abandoned.
	void updateBusy(RuntimeState &state);

	// Forgets pending clicks and walks (keyboard or pad took over).
	void reset();

private:
	enum class ClickStage {
		None,
		Placing,          // Cursor pinned on the click until the main loop has resolved it.
		DoubleClickWindow // Object click waiting to see whether a second click follows.
	};

	// Recovered room layout constants (content coordinates of the H32 room screen).
	static const int16 kInterfaceTopY = 0xA8;       // CursorY at or below this targets the verb bar.
	static const int16 kInterfaceSecondRowY = 0xC0; // Bar rows split here (RoomSpriteRenderer).
	static const int16 kCursorMinimumX = -8;        // RoomActorUpdater::updateInteractionCursor clamps.
	static const int16 kCursorMaximumX = 0xF7;
	static const int kCrosshairHotspot = 7;         // Crosshair centre within the 16x16 cursor sprite.
	static const int kBarColumnWidth = 0x28;
	static const int kBarColumnCount = 6;
	static const uint8 kDisplayInterfaceMask = 0x08;
	static const uint8 kDisplayRightInterfaceMask = 0x10;
	static const uint8 kAlternateInteractionMask = 0x01;
	static const uint8 kForegroundPolarityMask = 0x01;
	static const uint8 kFixedRoomObjectMask = 0x02;
	static const int16 kRoomObjectFirstInteraction = 3;

	// Managed-only tuning, in retrace samples and pixels.
	static const int kDoubleClickWindow = 18; // About 0.3 s.
	static const int kDoubleClickDistance = 4;
	static const int kArrivalToleranceX = 2;
	static const int kArrivalToleranceY = 1;
	static const int kWalkStallLimit = 30;
	static const int kInterfaceWaitLimit = 6;
	static const int kBusyLimit = 2;

	void placeCursor(RuntimeState &state, const Common::Point &target) const;
	void resolveClick(RuntimeState &state, MouseInput &mouse);
	void walkIntoClickedObject(RuntimeState &state, MouseInput &mouse);
	bool findWalkInTarget(RuntimeState &state, int16 interaction, int16 &worldX, int16 &worldY) const;
	void startWalk(int16 worldX, int16 worldY);
	ControllerButtons stepWalk(RuntimeState &state);
	bool isBarArrow(const RuntimeState &state, const Common::Point &target) const;

	const ScoobyDooRom *_rom;
	int16 _roomId;
	bool _suppressAutoOpen;

	ClickStage _clickStage;
	bool _clickPlaced;
	int _clickWait;
	Common::Point _clickPosition;
	int16 _clickInteraction;

	bool _walking;
	int16 _walkTargetX;
	int16 _walkTargetY;
	int _walkBestDistance;
	int _walkStall;
	int _walkInterfaceWait;
	int _busySamples;
};

} // namespace Scooby

#endif // SCOOBY_ROOM_MOUSE_CONTROLLER_H
