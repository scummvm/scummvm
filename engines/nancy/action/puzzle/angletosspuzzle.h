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

#ifndef NANCY_ACTION_ANGLETOSSPUZZLE_H
#define NANCY_ACTION_ANGLETOSSPUZZLE_H

#include "engines/nancy/action/actionrecord.h"

namespace Nancy {
namespace Action {

// Squid toss mini-game in Nancy 8.
//
// UI layout:
//   Left panel  - 5 direct-select power buttons (Whale / Dolphin / Trout / Shrimp / Fish Fry).
//   Right panel - Ship's wheel with two arrow buttons (aim left / aim right), 5 angle positions.
//   Top-right   - "LAUNCH" button that fires the fish.
//   Top-left    - "MENU" button (handled as exit hotspot).
//
// Each AR instance represents one round of the puzzle. The player adjusts angle and power
// then clicks LAUNCH. If the selection matches the AR's _targetPower/_targetAngle, the win
// flag is set and the game transitions to _throwSquidScene to play the throw animation.
// Wrong throws transition to _throwSquidScene, showing the appropriate failure animation,
// based on which of the four fail flags is set (too strong/weak, too left/right). After the
// separate AR instances (each with their own target) implement the 3-round mechanic.
// FUN_0044a526 and FUN_0044a6be handle flag clearing and result evaluation in the original.

class AngleTossPuzzle : public RenderActionRecord {
public:
	AngleTossPuzzle() : RenderActionRecord(7) {}
	virtual ~AngleTossPuzzle() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

protected:
	Common::String getRecordTypeName() const override { return "AngleTossPuzzle"; }
	bool isViewportRelative() const override { return true; }

	Common::Path _imageName;

	// data+0x21..0x2c: 6 × uint16.
	// _initialPower/_initialAngle: starting player selection (copied to object+0x24/0x26 in original).
	// _numPowers/_numAngles: UI control bounds (always 5 in practice).
	// _targetPower/_targetAngle: the correct answer for this AR instance (compared in FUN_0044a6be).
	uint16 _initialPower = 0;
	uint16 _initialAngle = 0;
	uint16 _numPowers = 0;
	uint16 _numAngles = 0;
	uint16 _targetPower = 0;
	uint16 _targetAngle = 0;

	// The 22 rects read from the data file, in stream order.
	// Rect-to-data-offset mapping confirmed from the render callback (FUN_0044b1fa).
	//
	// Sprite rects (_fooSprite) are source areas within the loaded image.
	// Display rects (_fooDisplay) are destination areas on the viewport overlay.
	// Hotspot rects (_fooHotspot) are clickable areas in viewport-local coordinates.
	//
	//   Rect  0 — data+0x2d  _throwHotspot         (clickable area for the throw button)
	//   Rect  1 — data+0x3d  _throwDisplay         (where throw button sprite is drawn)
	//   Rect  2 — data+0x4d  _throwSprite          (throw button source in image)
	//   Rect  3 — data+0x5d  _aimDecHotspot        (aim-left arrow)
	//   Rect  4 — data+0x6d  _aimIncHotspot        (aim-right arrow)
	//   Rect  5 — data+0x7d  _angleDisplay         (single screen position for angle indicator)
	//   Rects 6-10 — data+0x8d..0xcd  _angleSprites[5]  (5 angle images in sprite sheet)
	//   Rect 11 — data+0xdd  _powerDisplay         (single screen position for power indicator)
	//   Rects 12-16 — data+0xed..0x12d  _powerHotspots[5]  (power-select click areas)
	//   Rects 17-21 — data+0x13d..0x17d  _powerSprites[5]  (5 power images in sprite sheet)

	Common::Rect _throwHotspot;						// Rect  0 — clickable area for LAUNCH
	Common::Rect _throwDisplay;						// Rect  1 — sprite dest on screen
	Common::Rect _throwSprite;						// Rect  2 — source in image
	Common::Rect _aimLeftHotspot;					// Rect  3
	Common::Rect _aimRightHotspot;					// Rect  4
	Common::Rect _angleDisplay;						// Rect  5 — single dest on screen
	Common::Array<Common::Rect> _angleSprites;		// Rects 6-10 — 5 source frames in image
	Common::Rect _powerDisplay;						// Rect 11 — single dest on screen
	Common::Array<Common::Rect> _powerHotspots;		// Rects 12-16 — click areas
	Common::Array<Common::Rect> _powerSprites;		// Rects 17-21 — 5 source frames in image

	SoundDescription _powerSound;		// Played when changing power level
	SoundDescription _squeakSound;		// Played when changing aim angle
	SoundDescription _chainSound;		// Played when LAUNCH is pressed

	SceneChangeWithFlag _throwSquidScene;	// Triggered on throw (FUN_0044a6be result)

	int16 _powerTooStrongFlag = -1;   // 0x236
	int16 _powerTooWeakFlag = -1; // 0x238

	int16 _angleTooLeftFlag = -1;	// 0x23a
	int16 _angleTooRightFlag = -1;	// 0x23c

	int16 _winFlag = -1;	// 0x23e

	SceneChangeWithFlag _exitScene;
	Common::Rect _exitHotspot;

	Graphics::ManagedSurface _image;

	uint16 _curPower = 0;
	uint16 _curAngle = 0;
	
	bool _isThrown = false;		// True while the chain sound plays after LAUNCH is pressed
	bool _exitPressed = false;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_ANGLETOSSPUZZLE_H
