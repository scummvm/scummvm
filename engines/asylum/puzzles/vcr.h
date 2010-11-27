/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef ASYLUM_VCR_H
#define ASYLUM_VCR_H

#include "asylum/puzzles/puzzle.h"

#include "asylum/shared.h"

namespace Asylum {

class AsylumEngine;
//class Screen;
//class Sound;
//class Video;
//class Cursor;
//class GraphicResource;
//struct GraphicQueueItem;

typedef struct VCRDrawInfo {
	int32 resourceId;
	Common::Point point;
} VCRDrawInfo;

const Common::Rect BlowUpPuzzleVCRPolies[10] = {
	Common::Rect(0x0F7, 0x157, 0x13A, 0x183), // rewind button region
	Common::Rect(0x14B, 0x15C, 0x17B, 0x18B), // stop button region
	Common::Rect(0x18C, 0x161, 0x1D2, 0x18F), // play button region
	Common::Rect(0x1FB, 0x156, 0x233, 0x185), // rec button region
	Common::Rect(0x154, 0x196, 0x173, 0x1AF), // red jack hole region
	Common::Rect(0x19A, 0x19B, 0x1B9, 0x1B3), // yellow jack hole region
	Common::Rect(0x1E3, 0x1A0, 0x202, 0x1BC), // black jack hole region
	Common::Rect(0x0,   0x19B, 0x3C,  0x1E0), // black jack on table region
	Common::Rect(0x4C,  0x1AC, 0x0A0, 0x1E0), // red jack on table region
	Common::Rect(0x0BB, 0x1B7, 0x0F0, 0x1E0)  // yellow jack on table region
};

class BlowUpPuzzleVCR : public BlowUpPuzzle {
public:
	BlowUpPuzzleVCR(AsylumEngine *engine);
	~BlowUpPuzzleVCR();

	void handleEvent(const AsylumEvent &event, bool doUpdate);
	void open();
	void close();
private:
	enum Jack {
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

	enum Button {
		kRewind  = 0,
		kStop    = 1,
		kPlay    = 2,
		kPower   = 3
	};

	enum ButtonState {
		kOFF     = 0,
		kON      = 1,
		kDownON  = 2,
		kDownOFF = 3
	};

	int _jacksState[3];
	int _holesState[3];
	int _buttonsState[4];
	uint32 _tvScreenAnimIdx;
	bool _isAccomplished;

	int inPolyRegion(int x, int y, int polyIdx) const;

	void update();

	void updateCursorInPolyRegion();

	GraphicQueueItem getGraphicJackItem(int32 index);
	GraphicQueueItem getGraphicShadowItem();
	void updateJack(Jack jack, const VCRDrawInfo &onTable, const VCRDrawInfo &pluggedOnRed, const VCRDrawInfo &pluggedOnYellow, const VCRDrawInfo &pluggedOnBlack, int32 resourceOnHandIndex);
	void updateBlackJack();
	void updateRedJack();
	void updateYellowJack();
	int setJackOnHole(int jackType, JackState plugged);

	void updateButton(Button button, const VCRDrawInfo &btON, const VCRDrawInfo &btDown);
	void updatePowerButton();
	void updateRewindButton();
	void updatePlayButton();
	void updateStopButton();

	void updateTVSync();

	void handleMouseDown();
	void handleMouseUp();
};

} // end of namespace Asylum

#endif // ASYLUM_VCR_H
