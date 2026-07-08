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

#ifndef NANCY_ACTION_SEWINGMACHINEPUZZLE_H
#define NANCY_ACTION_SEWINGMACHINEPUZZLE_H

#include "engines/nancy/action/actionrecord.h"
#include "engines/nancy/commontypes.h"
#include "engines/nancy/action/navigationrecords.h"
#include "engines/nancy/action/actionzone.h"

namespace Nancy {
namespace Action {

// Nancy12 sewing-machine puzzle (action record 162): the player feeds a piece of
// cloth through a sewing machine needle.
//
// The cloth is a tall strip that the player drags to feed under the fixed needle;
// stitch/needle sounds play as it slides, and feeding it all the way through
// cross-dissolves to the trigger zone's win scene.
// TODO: the horizontal steer range and the drag-to-feed scale are tunable guesses.
class SewingMachinePuzzle : public RenderActionRecord {
public:
	SewingMachinePuzzle() : RenderActionRecord(7) {}
	virtual ~SewingMachinePuzzle() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

protected:
	Common::String getRecordTypeName() const override { return "SewingMachinePuzzle"; }

	// Sorts the ActionZone array into its gameplay roles.
	void classifyZones();
	// Plays one entry of a random-sound block (needle/stitch cues).
	void playSoundBlock(const RandomSoundBlock &block);
	// Draws the sewing view (the "BED_Sewing_OVL" overlay).
	void drawCloth();
	// Advances the sewing by a drag delta, firing stitch/needle cues and marking
	// the puzzle solved once enough cloth has been fed through.
	void feedCloth(const Common::Point &delta);

	// Background image ("BED_Sewing_OVL").
	Common::Path _imageName;

	// 87-byte PuzzleBase header blob: image name + two cloth regions + a direction
	// vector + an extent vector (its magnitude = the seam length) + three params
	// (_params[1] = stitch distance threshold, _params[2] = its hysteresis).
	Common::Rect _rects[2];
	Common::Point _directionVector;
	Common::Point _extentVector;
	int16 _params[3] = {};

	RandomSoundBlock _soundBlock;
	Common::Array<ActionZone> _zones;

	// ActionZone roles (indices into _zones; -1 == none).
	int _collisionZone = -1;			// type 0x0b: the needle line + its sounds/flag
	int _boundaryZone = -1;				// type 0x14
	Common::Array<uint> _triggerZones;	// type 0x0c: completion triggers

	// Win transition from a trigger zone's SpecialEffect: its leading id is the win
	// scene, the effect is the fade played over the change.
	SceneChangeDescription _winScene;
	int16 _winEventFlag = -1;
	bool _winHasFade = false;
	byte _winFadeType = 0;
	uint16 _winFadeTotalTime = 0;
	uint16 _winFadeToBlackTime = 0;
	Common::Rect _winFadeRect;

	// Runtime state. The cloth is a tall strip drawn at (_clothPos); dragging scrolls
	// it under the fixed needle (feed = vertical, steer = horizontal).
	Common::Point _clothPos;			// top-left of the cloth on screen
	int _maxFeed = 0;					// vertical scroll range (image height - viewport)
	int _maxSteer = 0;					// horizontal steer range each way
	double _strokeDistance = 0.0;		// movement accumulated toward the next stitch
	bool _dragging = false;
	Common::Point _lastDragPos;
	bool _solved = false;

	Graphics::ManagedSurface _image;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_SEWINGMACHINEPUZZLE_H
