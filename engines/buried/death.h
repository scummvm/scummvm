/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995 Presto Studios, Inc.
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

#ifndef BURIED_DEATH_H
#define BURIED_DEATH_H

#include "buried/global_flags.h"
#include "buried/window.h"

namespace Graphics {
class Font;
struct Surface;
}

namespace Buried {

class AVIFrames;
class AgentEvaluation;

class DeathWindow : public Window {
public:
	DeathWindow(BuriedEngine *vm, Window *parent, int deathSceneIndex, GlobalFlags globalFlags, Common::Array<int> itemArray);
	~DeathWindow();

	void onPaint();
	bool onEraseBackground();
	void onTimer(uint timer);
	void onLButtonUp(const Common::Point &point, uint flags);
	void onLButtonDown(const Common::Point &point, uint flags);
	void onMouseMove(const Common::Point &point, uint flags);

private:
	Common::Rect _quit;
	Common::Rect _restoreGame;
	Common::Rect _mainMenu;
	int _curButton;
	uint _timer;
	AVIFrames *_deathSceneFrames;
	int _deathSceneIndex;
	GlobalFlags _globalFlags;
	Common::Array<int> _itemArray;
	int32 _deathFrameIndex;
	bool _lightOn;
	Graphics::Font *_textFontA;
	Graphics::Font *_textFontB;
	int _fontHeightA, _fontHeightB;
	bool _walkthroughMode;
	AgentEvaluation *_agentEvaluation;
};

} // End of namespace Buried

#endif
