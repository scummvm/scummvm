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

enum DebuggerDrawnObjectType {
	debuggerObjTypeUndefined     = 99,
	debuggerObjTypeActor 	     =  0,
	debuggerObjType3dObject      =  1,
	debuggerObjTypeItem          =  2,
	debuggerObjTypeRegionNormal  =  3,
	debuggerObjTypeRegionExit    =  4,
	debuggerObjTypeWaypointNorm  =  5,
	debuggerObjTypeWaypoingFlee  =  6,
	debuggerObjTypeWaypointCover =  7,
	debuggerObjTypeWalkbox       =  8,
	debuggerObjTypeEffect        =  9,
	debuggerObjTypeLight         = 10,
	debuggerObjTypeFog           = 11
};

class Debugger : public GUI::Debugger{
	BladeRunnerEngine *_vm;

	static const uint kMaxSpecificObjectsDrawnCount = 100;

	struct DebuggerDrawnObject {
		int                     sceneId;
		int                     setId;
		int                     objId;
		DebuggerDrawnObjectType type;
	};

public:
	bool _isDebuggerOverlay;

	bool _viewActorsToggle;
	bool _view3dObjectsToggle;
	bool _viewItemsToggle;
	bool _viewFogs;
	bool _viewLights;
	bool _viewScreenEffects;
	bool _viewObstacles;
	bool _viewRegionsNormalToggle;
	bool _viewRegionsExitsToggle;
	bool _viewUI;
	bool _viewWaypointsNormalToggle;
	bool _viewWaypointsFleeToggle;
	bool _viewWaypointsCoverToggle;
	bool _viewWalkboxes;
	bool _viewZBuffer;
	bool _playFullVk;
	bool _showStatsVk;
	bool _showMazeScore;
	bool _showMouseClickInfo;

	Debugger(BladeRunnerEngine *vm);
	~Debugger() override;

	bool cmdAnimation(int argc, const char **argv);
	bool cmdHealth(int argc, const char **argv);
//	bool cmdChapter(int argc, const char **argv);
	bool cmdDraw(int argc, const char **argv);
	bool cmdFlag(int argc, const char **argv);
	bool cmdGoal(int argc, const char **argv);
	bool cmdLoop(int argc, const char **argv);
	bool cmdPosition(int argc, const char **argv);
	bool cmdMusic(int argc, const char** argv);
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
	bool cmdMazeScore(int argc, const char **argv);
	bool cmdObject(int argc, const char **argv);
	bool cmdItem(int argc, const char **argv);
	bool cmdRegion(int argc, const char **argv);
	bool cmdClick(int argc, const char **argv);
	bool cmdDifficulty(int argc, const char **argv);
#if BLADERUNNER_ORIGINAL_BUGS
#else
	bool cmdEffect(int argc, const char **argv);
#endif // BLADERUNNER_ORIGINAL_BUGS
	bool cmdList(int argc, const char **argv);
	bool cmdVk(int argc, const char **argv);

	Common::String getDifficultyDescription(int difficultyValue);
	void drawDebuggerOverlay();

	void drawBBox(Vector3 start, Vector3 end, View *view, Graphics::Surface *surface, int color);
	void drawSceneObjects();
	void drawLights();
	void drawFogs();
	void drawRegions();
	void drawWaypoints();
	void drawWalkboxes();
	void drawScreenEffects();

	bool dbgAttemptToLoadChapterSetScene(int chapterId, int setId, int sceneId);

private:
	Common::Array<DebuggerDrawnObject> _specificDrawnObjectsList;
	bool _specificActorsDrawn;
	bool _specific3dObjectsDrawn;
	bool _specificItemsDrawn;
	bool _specificEffectsDrawn;
	bool _specificLightsDrawn;
	bool _specificFogsDrawn;
	bool _specificRegionNormalDrawn;
	bool _specificRegionExitsDrawn;
	bool _specificWaypointNormalDrawn;
	bool _specificWaypointFleeDrawn;
	bool _specificWaypointCoverDrawn;
	bool _specificWalkboxesDrawn;

	void toggleObjectInDbgDrawList(DebuggerDrawnObject &drObj);
	int findInDbgDrawList(DebuggerDrawnObjectType objType, int objId, int setId, int sceneId);
	void updateTogglesForDbgDrawListInCurrentSetAndScene();
};

} // End of namespace BladeRunner

#endif
