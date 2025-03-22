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

#ifndef ALG_SCENE_H
#define ALG_SCENE_H

#include "common/array.h"
#include "common/file.h"
#include "common/path.h"
#include "common/rect.h"
#include "common/str.h"

namespace Alg {

struct TokenEntry {
	const char *name;
	uint8 value;
};

const TokenEntry _mainTokens[] = {
	{"ZONE", 1},
	{"SCENE", 2},
	{"MSG", 3},
	{"START", 4},
	{"GLOBAL", 5},
	{"END", 6},
	{nullptr, 0}};

const TokenEntry _zoneTokens[] = {
	{"NEXT", 1},
	{"PTRFB", 2},
	{"RECT", 3},
	{";", 4},
	{nullptr, 0}};

const TokenEntry _sceneTokens[] = {
	{"NEXT", 1},
	{"ZONES", 2},
	{"PREOP", 3},
	{"SHOWMSG", 4},
	{"SCNMSG", 4},
	{"INSOP", 5},
	{"WEPDWN", 6},
	{"SCNSCR", 7},
	{"NXTFRM", 8},
	{"NXTSCN", 9},
	{"DATA", 10},
	{"DIFF", 11},
	{"MISSEDRECTS", 12},
	{"DIFFICULTY_MOD", 13},
	{";", 14},
	{nullptr, 0}};

class Rect : public Common::Rect {
public:
	Common::String scene;
	uint32 score;
	Common::String rectHit;
	Common::String unknown;
	bool isMoving = false;
	Common::Rect dest;
	void center(int16 cx, int16 cy, int16 w, int16 h) {
		right = cx + (w / 2);
		left = cx - (w / 2);
		top = cy - (h / 2);
		bottom = cy + (h / 2);
	}
};

class Zone {
public:
	Common::String name;
	uint32 startFrame;
	uint32 endFrame;
	Common::String ptrfb;
	Common::Array<Rect> rects;
	Common::String next;
	void addRect(int16 left, int16 top, int16 right, int16 bottom, Common::String scene, uint32 score, Common::String rectHit, Common::String unknown);
};

class Scene {
public:
	Common::String name;
	uint32 startFrame;
	uint32 endFrame;
	Common::String next;
	Common::String zonesStart;
	Common::String zonesStart2;
	Common::String zonesStart3;
	Common::String preop;
	Common::String preopParam;
	Common::String insop;
	Common::String insopParam;
	Common::String scnmsg;
	Common::String scnmsgParam;
	Common::String wepdwn;
	Common::String scnscr;
	int32 scnscrParam;
	Common::String nxtfrm;
	Common::String nxtscn;
	int32 dataParam1;
	int32 dataParam2;
	int32 dataParam3;
	int32 dataParam4;
	Common::String dataParam5;
	Common::String dataParam6;
	uint32 diff;
	Common::String missedRects;
	uint32 difficultyMod;
	Common::Array<Zone *> zones;
};

class SceneInfo {

public:
	SceneInfo();
	~SceneInfo();
	void loadScnFile(const Common::Path &path);
	Common::String getStartScene() { return _startscene; }
	Common::Array<Scene *> *getScenes() { return &_scenes; }
	Scene *findScene(Common::String sceneName);
	void addScene(Scene *scene);

private:
	Common::File _scnFile;
	Common::String _startscene;
	Common::Array<Scene *> _scenes;
	Common::Array<Zone *> _zones;

	void parseStart(Common::String sceneName);
	void parseScene(Common::String sceneName, uint32 startFrame, uint32 endFrame);
	void parseZone(Common::String zoneName, uint32 startFrame, uint32 endFrame);
	void addZonesToScenes();
	Zone *findZone(Common::String zoneName);
	int8 getToken(const struct TokenEntry *tokenList, Common::String token);
	bool ignoreScriptLine(Common::String line);
};

} // End of namespace Alg

#endif
