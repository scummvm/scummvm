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

#ifndef ASYLUM_PUZZLES_VCR_H
#define ASYLUM_PUZZLES_VCR_H

#include "asylum/puzzles/puzzle.h"

#include "asylum/shared.h"

namespace Asylum {

class AsylumEngine;

typedef struct VCRDrawInfo {
	int32 resourceId;
	Common::Point point;
} VCRDrawInfo;

const int16 puzzleVCRPolygons[10][4] = {
	{0x0F7, 0x157, 0x13A, 0x183}, // rewind button region
	{0x14B, 0x15C, 0x17B, 0x18B}, // stop button region
	{0x18C, 0x161, 0x1D2, 0x18F}, // play button region
	{0x1FB, 0x156, 0x233, 0x185}, // rec button region
	{0x154, 0x196, 0x173, 0x1AF}, // red jack hole region
	{0x19A, 0x19B, 0x1B9, 0x1B3}, // yellow jack hole region
	{0x1E3, 0x1A0, 0x202, 0x1BC}, // black jack hole region
	{0x0,   0x19B, 0x3C,  0x1E0}, // black jack on table region
	{0x4C,  0x1AC, 0x0A0, 0x1E0}, // red jack on table region
	{0x0BB, 0x1B7, 0x0F0, 0x1E0}  // yellow jack on table region
};

//////////////////////////////////////////////////////////////////////////
// Puzzle 1
//////////////////////////////////////////////////////////////////////////
class PuzzleVCR : public Puzzle {
public:
	PuzzleVCR(AsylumEngine *engine);
	~PuzzleVCR();

	// Serializable
	virtual void saveLoadWithSerializer(Common::Serializer &s);

private:
	enum Color {
		kNone   = -1,
		kBlack  = 0,
		kRed    = 1,
		kYellow = 2
	};

	enum JackState {
		kOnTable            = 0,
		kPluggedOnRed       = 1,
		kPluggedOnYellow    = 2,
		kPluggedOnBlack     = 3,
		kOnHand             = 4
	};

	enum VCRRegions {
		kRewindButton   = 0,
		kStopButton     = 1,
		kPlayButton     = 2,
		kPowerButton    = 3,
		kRedHole        = 4,
		kYellowHole     = 5,
		kBlackHole      = 6,
		kBlackJack      = 7,
		kRedJack        = 8,
		kYellowJack     = 9
	};

	enum ButtonState {
		kOFF     = 0,
		kON      = 1,
		kDownON  = 2,
		kDownOFF = 3
	};

	JackState _jacksState[3];
	JackState _holesState[3];
	ButtonState _buttonsState[4];
	uint32 _tvScreenFrameIndex;
	bool _isAccomplished;

	//////////////////////////////////////////////////////////////////////////
	// Event Handling
	//////////////////////////////////////////////////////////////////////////
	bool init(const AsylumEvent &evt);
	bool key(const AsylumEvent &evt);
	bool mouseLeftDown(const AsylumEvent &evt);
	bool mouseLeftUp(const AsylumEvent &evt);
	bool mouseRightDown(const AsylumEvent &evt);

	//////////////////////////////////////////////////////////////////////////
	// Drawing
	//////////////////////////////////////////////////////////////////////////
	void updateScreen();
	void updateCursor();

	//////////////////////////////////////////////////////////////////////////
	// Updates
	//////////////////////////////////////////////////////////////////////////
	void updateJack(Color jack, const VCRDrawInfo &onTable, const VCRDrawInfo &pluggedOnRed, const VCRDrawInfo &pluggedOnYellow, const VCRDrawInfo &pluggedOnBlack, int32 resourceOnHandIndex);
	void updateBlackJack();
	void updateRedJack();
	void updateYellowJack();

	void updateButton(VCRRegions button, const VCRDrawInfo &btON, const VCRDrawInfo &btDown);
	void updatePowerButton();
	void updateRewindButton();
	void updatePlayButton();
	void updateStopButton();

	void updateTVSync();

	//////////////////////////////////////////////////////////////////////////
	// Helpers
	//////////////////////////////////////////////////////////////////////////
	int inPolygon(const Common::Point &point, int polyIdx) const;
	Color getJackOnHand() const;
	void setJackOnHole(Color hole, JackState state, JackState newState);
	void pickJack(Color jack);

};

} // end of namespace Asylum

#endif // ASYLUM_PUZZLES_VCR_H
