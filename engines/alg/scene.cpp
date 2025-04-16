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

#include "common/debug.h"
#include "common/file.h"
#include "common/tokenizer.h"

#include "alg/scene.h"

namespace Alg {

SceneInfo::SceneInfo() {
}

SceneInfo::~SceneInfo() {
	for (auto scene : _scenes) {
		delete scene;
	}
	for (auto zone : _zones) {
		delete zone;
	}
	_scenes.clear();
	_zones.clear();
}

void SceneInfo::loadScnFile(const Common::Path &path) {
	debug("loading scene script: %s", path.toString().c_str());
	if (!_scnFile.open(path)) {
		error("SceneInfo::loadScnFile(): Can't open scene file '%s'", path.toString().c_str());
	}
	bool done = false;
	while (_scnFile.pos() < _scnFile.size() && !done) {
		Common::String line = _scnFile.readLine();
		line.trim();
		if (ignoreScriptLine(line)) {
			continue;
		}
		Common::StringTokenizer tokenizer(line, " ");
		int8 token = getToken(_mainTokens, tokenizer.nextToken());
		uint32 startFrame = 0, endFrame = 0;
		Common::String sceneName = nullptr, zoneName = nullptr;
		switch (token) {
		case 0: // ;
			break;
		case 1: // ZONE
			zoneName = tokenizer.nextToken();
			startFrame = atoi(tokenizer.nextToken().c_str());
			endFrame = atoi(tokenizer.nextToken().c_str());
			parseZone(zoneName, startFrame, endFrame);
			break;
		case 2: // SCENE
			sceneName = tokenizer.nextToken();
			startFrame = atoi(tokenizer.nextToken().c_str());
			endFrame = atoi(tokenizer.nextToken().c_str());
			sceneName.toLowercase();
			parseScene(sceneName, startFrame, endFrame);
			break;
		case 3: // MSG
			error("SceneInfo::loadScnFile(): MSG Not implemented: %s", line.c_str());
			break;
		case 4: // START
			_startScene = tokenizer.nextToken();
			break;
		case 5: // GLOBAL
			error("SceneInfo::loadScnFile(): GLOBAL Not implemented: %s", line.c_str());
			break;
		case 6: // END
			done = true;
			break;
		default:
			error("SceneInfo::loadScnFile(): Unknown script section encountered: %s", line.c_str());
			break;
		}
	}
	_scnFile.close();
	addZonesToScenes();
}

void SceneInfo::parseScene(const Common::String &sceneName, uint32 startFrame, uint32 endFrame) {
	Scene *scene = new Scene(sceneName, startFrame, endFrame);
	bool done = false;
	while (_scnFile.pos() < _scnFile.size() && !done) {
		Common::String line = _scnFile.readLine();
		line.trim();
		if (ignoreScriptLine(line)) {
			continue;
		}
		Common::StringTokenizer tokenizer(line, " ");
		int8 token = getToken(_sceneTokens, tokenizer.nextToken());
		switch (token) {
		case 0: // EOF
		case 1: // NEXT
			scene->_next = tokenizer.nextToken();
			break;
		case 2: // ZONES
			scene->_zonesStart = tokenizer.nextToken();
			scene->_zonesStart2 = tokenizer.nextToken();
			scene->_zonesStart3 = tokenizer.nextToken();
			break;
		case 3: // PREOP
			scene->_preop = tokenizer.nextToken();
			scene->_preopParam = tokenizer.nextToken();
			break;
		case 4: // SHOWMSG / SCNMSG
			scene->_scnmsg = tokenizer.nextToken();
			scene->_scnmsgParam = tokenizer.nextToken();
			break;
		case 5: // INSOP
			scene->_insop = tokenizer.nextToken();
			scene->_insopParam = tokenizer.nextToken();
			break;
		case 6: // WEPDWN
			scene->_wepdwn = tokenizer.nextToken();
			break;
		case 7: // SCNSCR
			scene->_scnscr = tokenizer.nextToken();
			scene->_scnscrParam = atoi(tokenizer.nextToken().c_str());
			break;
		case 8: // NXTFRM
			scene->_nxtfrm = tokenizer.nextToken();
			break;
		case 9: // NXTSCN
			scene->_nxtscn = tokenizer.nextToken();
			// ignore next token if existing
			tokenizer.nextToken();
			break;
		case 10: // DATA
			scene->_dataParam1 = atoi(tokenizer.nextToken().c_str());
			scene->_dataParam2 = atoi(tokenizer.nextToken().c_str());
			scene->_dataParam3 = atoi(tokenizer.nextToken().c_str());
			scene->_dataParam4 = atoi(tokenizer.nextToken().c_str());
			scene->_dataParam5 = atoi(tokenizer.nextToken().c_str());
			scene->_dataParam6 = atoi(tokenizer.nextToken().c_str());
			break;
		case 11: // DIFF
			scene->_diff = atoi(tokenizer.nextToken().c_str());
			break;
		case 12: // MISSEDRECTS
			scene->_missedRects = tokenizer.nextToken();
			// ignore next token if existing
			tokenizer.nextToken();
			break;
		case 13: // DIFFICULTY_MOD
			scene->_difficultyMod = atoi(tokenizer.nextToken().c_str());
			break;
		case 14: // ;
			done = true;
			break;
		default:
			error("SceneInfo::parseScene(): Unknown scene token found: %s", line.c_str());
			break;
		}
		Common::String nextToken = tokenizer.nextToken();
		if (!nextToken.empty()) {
			error("SceneInfo::parseScene(): missed token %s in line %s", nextToken.c_str(), line.c_str());
		}
	}
	_scenes.push_back(scene);
}

void SceneInfo::parseZone(const Common::String &zoneName, uint32 startFrame, uint32 endFrame) {
	Zone *zone = new Zone(zoneName, startFrame, endFrame);
	bool done = false;
	while (_scnFile.pos() < _scnFile.size() && !done) {
		Common::String line = _scnFile.readLine();
		line.trim();
		if (ignoreScriptLine(line)) {
			continue;
		}
		Common::StringTokenizer tokenizer(line, " ");
		int8 token = getToken(_zoneTokens, tokenizer.nextToken());
		Rect *rect = nullptr;
		switch (token) {
		case 0: // EOF
			break;
		case 1: // NEXT
			zone->_next = tokenizer.nextToken();
			break;
		case 2: // PTRFB
			zone->_ptrfb = tokenizer.nextToken();
			break;
		case 3: // RECT
		{
			Common::String temp = tokenizer.nextToken();
			if (temp == "MOVING") {
				rect = new Rect();
				rect->_isMoving = true;
				rect->left = atoi(tokenizer.nextToken().c_str());
				rect->top = atoi(tokenizer.nextToken().c_str());
				rect->right = atoi(tokenizer.nextToken().c_str());
				rect->bottom = atoi(tokenizer.nextToken().c_str());
				rect->_dest.left = atoi(tokenizer.nextToken().c_str());
				rect->_dest.top = atoi(tokenizer.nextToken().c_str());
				rect->_dest.right = atoi(tokenizer.nextToken().c_str());
				rect->_dest.bottom = atoi(tokenizer.nextToken().c_str());
				rect->_scene = tokenizer.nextToken();
				rect->_score = atoi(tokenizer.nextToken().c_str());
				rect->_rectHit = tokenizer.nextToken();
				rect->_unknown = tokenizer.nextToken();
				zone->_rects.push_back(rect);
			} else {
				rect = new Rect();
				rect->_isMoving = false;
				rect->left = atoi(temp.c_str());
				rect->top = atoi(tokenizer.nextToken().c_str());
				rect->right = atoi(tokenizer.nextToken().c_str());
				rect->bottom = atoi(tokenizer.nextToken().c_str());
				rect->_scene = tokenizer.nextToken();
				rect->_score = atoi(tokenizer.nextToken().c_str());
				rect->_rectHit = tokenizer.nextToken();
				rect->_unknown = tokenizer.nextToken();
				zone->_rects.push_back(rect);
			}
		} break;
		case 4: // ;
			done = true;
			break;
		default:
			error("SceneInfo::parseZone(): Unknown zone token found: %s", line.c_str());
			break;
		}
		Common::String nextToken = tokenizer.nextToken();
		if (!nextToken.empty()) {
			error("SceneInfo::parseZone(): missed token %s in line %s", nextToken.c_str(), line.c_str());
		}
	}
	_zones.push_back(zone);
}

void SceneInfo::addZonesToScenes() {
	for (auto &scene : _scenes) {
		if (!scene->_zonesStart.empty()) {
			Zone *zone = findZone(scene->_zonesStart);
			scene->_zones.push_back(zone);
			while (!zone->_next.empty()) {
				zone = findZone(zone->_next);
				if (zone == nullptr) {
					break;
				}
				scene->_zones.push_back(zone);
			}
		}
		if (!scene->_zonesStart2.empty() && scene->_zonesStart2 != scene->_zonesStart) {
			Zone *zone = findZone(scene->_zonesStart2);
			scene->_zones.push_back(zone);
			while (!zone->_next.empty()) {
				zone = findZone(zone->_next);
				if (zone == nullptr) {
					break;
				}
				scene->_zones.push_back(zone);
			}
		}
		if (!scene->_zonesStart3.empty() && scene->_zonesStart3 != scene->_zonesStart2) {
			Zone *zone = findZone(scene->_zonesStart3);
			scene->_zones.push_back(zone);
			while (!zone->_next.empty()) {
				zone = findZone(zone->_next);
				if (zone == nullptr) {
					break;
				}
				scene->_zones.push_back(zone);
			}
		}
	}
}

void SceneInfo::addScene(Scene *scene) {
	_scenes.push_back(scene);
}

Zone *SceneInfo::findZone(const Common::String &zoneName) {
	for (auto &zone : _zones) {
		if (zone->_name.equalsIgnoreCase(zoneName)) {
			return zone;
		}
	}
	warning("SceneInfo::findZone(): Cannot find zone %s", zoneName.c_str());
	return nullptr;
}

Scene *SceneInfo::findScene(const Common::String &sceneName) {
	for (auto &scene : _scenes) {
		if (scene->_name.equalsIgnoreCase(sceneName)) {
			return scene;
		}
	}
	error("SceneInfo::findScene(): Cannot find scene %s", sceneName.c_str());
}

int8 SceneInfo::getToken(const TokenEntry *tokenList, const Common::String &token) {
	for (int i = 0; tokenList[i].name != nullptr; i++) {
		if (token == tokenList[i].name) {
			return tokenList[i].value;
		}
	}
	return -1;
}

bool SceneInfo::ignoreScriptLine(const Common::String &line) {
	if (line.empty()) {
		return true; // empty line
	} else if (line.hasPrefix("//")) {
		return true; // comment
	} else if (line.hasPrefix("*")) {
		return true; // doc comment
	} else if (line.hasPrefix("NXET")) {
		return true; // typo in Maddog2
	} else if (line.hasPrefix("DATA$")) {
		return true; // typo in DrugWars
	} else if (line.hasPrefix("NUMBER_OF_")) {
		return true; // unnecessary numbers
	}
	return false;
}

Scene::Scene(const Common::String &name, uint32 startFrame, uint32 endFrame) {
	_preop = "DEFAULT";
	_insop = "DEFAULT";
	_scnmsg = "DEFAULT";
	_wepdwn = "DEFAULT";
	_scnscr = "DEFAULT";
	_nxtfrm = "DEFAULT";
	_nxtscn = "DEFAULT";
	_missedRects = "DEFAULT";
	_missedRects = "DEFAULT";
	_scnscrParam = 0;
	_dataParam1 = 0;
	_dataParam2 = 0;
	_dataParam3 = 0;
	_dataParam4 = 0;
	_dataParam5 = "";
	_name = name;
	_startFrame = startFrame;
	_endFrame = endFrame;
	_difficultyMod = 0;
}

Zone::Zone(const Common::String &name, const Common::String &ptrfb) {
	_name = name;
	_ptrfb = ptrfb;
}

Zone::Zone(const Common::String &name, uint32 startFrame, uint32 endFrame) {
	_name = name;
	_startFrame = startFrame;
	_endFrame = endFrame;
}

Zone::~Zone() {
	for (auto rect : _rects) {
		delete rect;
	}
}

void Zone::addRect(int16 left, int16 top, int16 right, int16 bottom, const Common::String &scene, uint32 score, const Common::String &rectHit, const Common::String &unknown) {
	Rect *rect = new Rect();
	rect->left = left;
	rect->top = top;
	rect->right = right;
	rect->bottom = bottom;
	rect->_scene = scene;
	rect->_score = score;
	rect->_rectHit = rectHit;
	rect->_unknown = unknown;
	_rects.push_back(rect);
}

} // End of namespace Alg
