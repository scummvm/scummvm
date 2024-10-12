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

#ifndef DARKSEED_CUTSCENE_H
#define DARKSEED_CUTSCENE_H

#include "common/rect.h"
#include "common/str.h"
#include "graphics/managed_surface.h"
#include "darkseed/pal.h"
#include "darkseed/titlefont.h"
#include "darkseed/morph.h"

namespace Darkseed {

class Cutscene {
private:
	char _cutsceneId;
	uint16 _movieStep = 9999;
	TitleFont *_titleFont = nullptr;
	Pal _palette;
	Anm _animation;
	int _animIdx;
	int _animCount;
	int _animDelayCount;
	int _animDirection;
	uint32 _startTime = 0;
	Morph *_morph = nullptr;

public:
	virtual ~Cutscene();
	void play(char cutsceneId);
	bool isPlaying() {
		return _movieStep != 9999;
	}
	void update();

private:
	bool introScene();
	bool embryoInsertedScene();
	bool shipLaunchScene();
	bool alienBornScene();
	bool babyDollScene();
	bool bookScene();
	bool nightmare2Scene();
	bool nightmare3Scene();

	void runAnim(int direction = 1);
	bool stepAnim(int drawMode = 1);

	void putHouse();

	void registTime();
	bool waitTime(int16 duration);

	void freeMorph();
};

}; // End of namespace Darkseed

#endif // DARKSEED_CUTSCENE_H
