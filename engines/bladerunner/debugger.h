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

#ifndef BLADERUNNER_DEBUGGER_H
#define BLADERUNNER_DEBUGGER_H

#include "bladerunner/vector.h"

#include "gui/debugger.h"

namespace Graphics {
struct Surface;
}

namespace BladeRunner {

class BladeRunnerEngine;
class View;

class Debugger : public GUI::Debugger{
	BladeRunnerEngine *_vm;

public:
	bool _isDebuggerOverlay;

	bool _viewActorsOnly;
	bool _viewFogs;
	bool _viewLights;
	bool _viewScreenEffects;
	bool _viewSceneObjects;
	bool _viewObstacles;
	bool _viewRegions;
	bool _viewUI;
	bool _viewWaypoints;
	bool _viewWalkboxes;
	bool _viewZBuffer;


	Debugger(BladeRunnerEngine *vm);
	~Debugger();

	bool cmdAnimation(int argc, const char **argv);
	bool cmdChapter(int argc, const char **argv);
	bool cmdDraw(int argc, const char **argv);
	bool cmdFlag(int argc, const char **argv);
	bool cmdGoal(int argc, const char **argv);
	bool cmdLoop(int argc, const char **argv);
	bool cmdPosition(int argc, const char **argv);
	bool cmdSay(int argc, const char **argv);
	bool cmdScene(int argc, const char **argv);
	bool cmdVariable(int argc, const char **argv);
	bool cmdClue(int argc, const char **argv);
	bool cmdTimer(int argc, const char **argv);
	bool cmdFriend(int argc, const char **argv);
	bool cmdLoad(int argc, const char **argv);
	bool cmdSave(int argc, const char **argv);
	bool cmdOverlay(int argc, const char **argv);
	bool cmdSubtitle(int argc, const char **argv);

	void drawDebuggerOverlay();

	void drawBBox(Vector3 start, Vector3 end, View *view, Graphics::Surface *surface, int color);
	void drawSceneObjects();
	void drawActors();
	void drawLights();
	void drawFogs();
	void drawRegions();
	void drawWaypoints();
	void drawWalkboxes();
	void drawScreenEffects();
};

} // End of namespace BladeRunner

#endif
