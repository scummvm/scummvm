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

#ifndef ASYLUM_PUZZLES_HIVECONTROL_H
#define ASYLUM_PUZZLES_HIVECONTROL_H

#include "common/hashmap.h"

#include "asylum/puzzles/puzzle.h"

namespace Asylum {

class AsylumEngine;

enum Control {
	kControlNone          = -1,
	kControlWingsButton1  = 34,
	kControlWingsButton2  = 35,
	kControlWingsButton3  = 36,
	kControlReset         = 38,
	kControlWheelLeft     = 39,
	kControlWheelRight    = 40,
	kControlButtonRight   = 41,
	kControlButtonLeft    = 42,
	kControlGlyph1        = 49,
	kControlGlyph2        = 50,
	kControlGlyph3        = 51,
	kControlGlyph4        = 52,
	kControlGlyph5        = 53,
	kControlGlyph6        = 54
};

static const uint32 puzzleHiveControlHieroglyphs[2][6] = {
	{5, 12, 22, 13, 20, 6},
	{21, 4, 14, 13, 20, 6}
};

class PuzzleHiveControl : public Puzzle {
public:
	PuzzleHiveControl(AsylumEngine *engine);
	~PuzzleHiveControl();

	// Serializable
	virtual void saveLoadWithSerializer(Common::Serializer &s);

private:
	enum Element {
		kElementSwirl		= 31,
		kElementFlyHead		= 32,
		kElementResetDynamic	= 33,
		kElementLever		= 37,
		kElementWingLeft1	= 43,
		kElementWingRight1	= 44,
		kElementWingLeft2	= 45,
		kElementWingRight2	= 46,
		kElementWingLeft3	= 47,
		kElementWingRight3	= 48,
		kElementLensLeft	= 55,
		kElementLensRight	= 56,
		kElementIndicator	= 57,
		kElementSwirlRim	= 71,
		kElementResetStatic	= 72
	};

	int32 _soundVolume;
	int32 _rectIndex;
	uint32 _counter;

	int32 _data_457260;
	int32 _data_457264;

	uint32 _frameIndexes[73];
	Common::HashMap<uint32, Common::Point> _controlPoints;

	bool _resetFlag;
	bool _wingsState[3];
	bool _glyphFlags[2][6];
	Control _currentControl;
	uint32 _colorL, _colorR;
	uint32 _frameIndexOffset;
	uint32 _leverPosition, _prevLeverPosition, _leverDelta;

	//////////////////////////////////////////////////////////////////////////
	// Event Handling
	//////////////////////////////////////////////////////////////////////////
	bool init(const AsylumEvent &evt);
	void updateScreen();
	bool mouseLeftDown(const AsylumEvent &evt);
	bool mouseRightDown(const AsylumEvent &evt);

	//////////////////////////////////////////////////////////////////////////
	// Helpers
	//////////////////////////////////////////////////////////////////////////
	void updateCursor();
	Control findControl();
	void playSound();
	void reset();
	bool hitTest1(Control control, const Common::Point &point, const Common::Point &location);
};

} // End of namespace Asylum

#endif // ASYLUM_PUZZLES_HIVECONTROL_H
