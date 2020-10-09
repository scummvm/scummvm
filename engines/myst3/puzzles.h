/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef PUZZLES_H_
#define PUZZLES_H_

#include "common/scummsys.h"

#include "graphics/surface.h"

namespace Myst3 {

class Myst3Engine;

class Puzzles {
public:
	Puzzles(Myst3Engine *vm);
	virtual ~Puzzles();

	void run(uint16 id, uint16 arg0 = 0, uint16 arg1 = 0, uint16 arg2 = 0);

private:
	Myst3Engine *_vm;

	typedef int32 SymbolCodeSolution[4];

	struct PegCombination {
		uint16 movie;
		bool pegs[5];
		uint16 pegFrames[3];
		uint16 expireFrame;
	};

	void leversBall(int16 var);

	void tesla(int16 movie, int16 var, int16 move);

	void resonanceRingControl();
	void resonanceRingsLaunchBall();
	void resonanceRingsLights();

	void pinball(int16 var);
	const PegCombination *_pinballFindCombination(uint16 var, const PegCombination pegs[], uint16 size);

	void weightDrag(uint16 var, uint16 movie);

	void journalSaavedro(int16 move);
	int16 _journalSaavedroLastPageLastChapterValue();
	uint16 _journalSaavedroGetNode(uint16 chapter);
	uint16 _journalSaavedroPageCount(uint16 chapter);
	bool _journalSaavedroHasChapter(uint16 chapter);
	uint16 _journalSaavedroNextChapter(uint16 chapter, bool forward);

	void journalAtrus(uint16 node, uint16 var);
	void mainMenu(uint16 action);
	void projectorLoadBitmap(uint16 bitmap);
	void projectorAddSpotItem(uint16 bitmap, uint16 x, uint16 y);
	void projectorUpdateCoordinates();

	void symbolCodesInit(uint16 var, uint16 posX, uint16 posY);
	void symbolCodesClick(int16 var);
	bool _symbolCodesCheckSolution(uint16 var, const SymbolCodeSolution &solution);
	int32 _symbolCodesFound();

	void railRoadSwitchs();

	void rollercoaster();

	void settingsSave();

	void updateSoundScriptTimer();

	void checkCanSave();

	void _drawForVarHelper(int16 var, int32 startValue, int32 endValue);
	void _drawXTicks(uint16 ticks);
};

} // End of namespace Myst3

#endif // PUZZLES_H_
