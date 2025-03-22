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
}

void SceneInfo::loadScnFile(const Common::Path &path) {
	debug("loading scene script: %s", path.toString().c_str());
	if (!_scnFile.open(path)) {
		error("Can't open scene file '%s'", path.toString().c_str());
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
		uint32 startFrame, endFrame = 0;
		Common::String sceneName, zoneName = nullptr;
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
			error("MSG Not implemented: %s", line.c_str());
			break;
		case 4: // START
			_startscene = tokenizer.nextToken();
			break;
		case 5: // GLOBAL
			error("GLOBAL Not implemented: %s", line.c_str());
			break;
		case 6: // END
			done = true;
			break;
		default:
			error("Unknown script section encountered: %s", line.c_str());
			break;
		}
	}
	_scnFile.close();
	addZonesToScenes();
}

void SceneInfo::parseScene(Common::String sceneName, uint32 startFrame, uint32 endFrame) {
	Scene *scene = new Scene();
	scene->preop = "DEFAULT";
	scene->insop = "DEFAULT";
	scene->scnmsg = "DEFAULT";
	scene->wepdwn = "DEFAULT";
	scene->scnscr = "DEFAULT";
	scene->nxtfrm = "DEFAULT";
	scene->nxtscn = "DEFAULT";
	scene->missedRects = "DEFAULT";
	scene->missedRects = "DEFAULT";
	scene->scnscrParam = 0;
	scene->dataParam1 = 0;
	scene->dataParam2 = 0;
	scene->dataParam3 = 0;
	scene->dataParam4 = 0;
	scene->dataParam5 = "";
	scene->name = sceneName;
	scene->startFrame = startFrame;
	scene->endFrame = endFrame;
	scene->difficultyMod = 0;
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
			scene->next = tokenizer.nextToken();
			break;
		case 2: // ZONES
			scene->zonesStart = tokenizer.nextToken();
			scene->zonesStart2 = tokenizer.nextToken();
			scene->zonesStart3 = tokenizer.nextToken();
			break;
		case 3: // PREOP
			scene->preop = tokenizer.nextToken();
			scene->preopParam = tokenizer.nextToken();
			break;
		case 4: // SHOWMSG / SCNMSG
			scene->scnmsg = tokenizer.nextToken();
			scene->scnmsgParam = tokenizer.nextToken();
			break;
		case 5: // INSOP
			scene->insop = tokenizer.nextToken();
			scene->insopParam = tokenizer.nextToken();
			break;
		case 6: // WEPDWN
			scene->wepdwn = tokenizer.nextToken();
			break;
		case 7: // SCNSCR
			scene->scnscr = tokenizer.nextToken();
			scene->scnscrParam = atoi(tokenizer.nextToken().c_str());
			break;
		case 8: // NXTFRM
			scene->nxtfrm = tokenizer.nextToken();
			break;
		case 9: // NXTSCN
			scene->nxtscn = tokenizer.nextToken();
			// ignore next token if existing
			tokenizer.nextToken();
			break;
		case 10: // DATA
			scene->dataParam1 = atoi(tokenizer.nextToken().c_str());
			scene->dataParam2 = atoi(tokenizer.nextToken().c_str());
			scene->dataParam3 = atoi(tokenizer.nextToken().c_str());
			scene->dataParam4 = atoi(tokenizer.nextToken().c_str());
			scene->dataParam5 = atoi(tokenizer.nextToken().c_str());
			scene->dataParam6 = atoi(tokenizer.nextToken().c_str());
			break;
		case 11: // DIFF
			scene->diff = atoi(tokenizer.nextToken().c_str());
			break;
		case 12: // MISSEDRECTS
			scene->missedRects = tokenizer.nextToken();
			// ignore next token if existing
			tokenizer.nextToken();
			break;
		case 13: // DIFFICULTY_MOD
			scene->difficultyMod = atoi(tokenizer.nextToken().c_str());
			break;
		case 14: // ;
			done = true;
			break;
		default:
			error("Unknown scene token found: %s", line.c_str());
			break;
		}
		Common::String nextToken = tokenizer.nextToken();
		if (!nextToken.empty()) {
			error("missed token %s in line %s", nextToken.c_str(), line.c_str());
		}
	}
	_scenes.push_back(scene);
}

void SceneInfo::parseZone(Common::String zoneName, uint32 startFrame, uint32 endFrame) {
	Zone *zone = new Zone();
	zone->name = zoneName;
	zone->startFrame = startFrame;
	zone->endFrame = endFrame;
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
			zone->next = tokenizer.nextToken();
			break;
		case 2: // PTRFB
			zone->ptrfb = tokenizer.nextToken();
			break;
		case 3: // RECT
		{
			Common::String temp = tokenizer.nextToken();
			if (temp == "MOVING") {
				rect = new Rect();
				rect->isMoving = true;
				rect->left = atoi(tokenizer.nextToken().c_str());
				rect->top = atoi(tokenizer.nextToken().c_str());
				rect->right = atoi(tokenizer.nextToken().c_str());
				rect->bottom = atoi(tokenizer.nextToken().c_str());
				rect->dest.left = atoi(tokenizer.nextToken().c_str());
				rect->dest.top = atoi(tokenizer.nextToken().c_str());
				rect->dest.right = atoi(tokenizer.nextToken().c_str());
				rect->dest.bottom = atoi(tokenizer.nextToken().c_str());
				rect->scene = tokenizer.nextToken();
				rect->score = atoi(tokenizer.nextToken().c_str());
				rect->rectHit = tokenizer.nextToken();
				rect->unknown = tokenizer.nextToken();
				zone->rects.push_back(*rect);
			} else {
				rect = new Rect();
				rect->isMoving = false;
				rect->left = atoi(temp.c_str());
				rect->top = atoi(tokenizer.nextToken().c_str());
				rect->right = atoi(tokenizer.nextToken().c_str());
				rect->bottom = atoi(tokenizer.nextToken().c_str());
				rect->scene = tokenizer.nextToken();
				rect->score = atoi(tokenizer.nextToken().c_str());
				rect->rectHit = tokenizer.nextToken();
				rect->unknown = tokenizer.nextToken();
				zone->rects.push_back(*rect);
			}
		} break;
		case 4: // ;
			done = true;
			break;
		default:
			error("Unknown zone token found: %s", line.c_str());
			break;
		}
		Common::String nextToken = tokenizer.nextToken();
		if (!nextToken.empty()) {
			error("missed token %s in line %s", nextToken.c_str(), line.c_str());
		}
	}
	_zones.push_back(zone);
}

void SceneInfo::addZonesToScenes() {
	for (uint32 i = 0; i < _scenes.size(); i++) {
		Scene *scene = _scenes[i];
		if (!scene->zonesStart.empty()) {
			Zone *zone = findZone(scene->zonesStart);
			scene->zones.push_back(zone);
			while (!zone->next.empty()) {
				zone = findZone(zone->next);
				if (zone == nullptr) {
					break;
				}
				scene->zones.push_back(zone);
			}
		}
		if (!scene->zonesStart2.empty() && scene->zonesStart2 != scene->zonesStart) {
			Zone *zone = findZone(scene->zonesStart2);
			scene->zones.push_back(zone);
			while (!zone->next.empty()) {
				zone = findZone(zone->next);
				if (zone == nullptr) {
					break;
				}
				scene->zones.push_back(zone);
			}
		}
		if (!scene->zonesStart3.empty() && scene->zonesStart3 != scene->zonesStart2) {
			Zone *zone = findZone(scene->zonesStart3);
			scene->zones.push_back(zone);
			while (!zone->next.empty()) {
				zone = findZone(zone->next);
				if (zone == nullptr) {
					break;
				}
				scene->zones.push_back(zone);
			}
		}
	}
}

Zone *SceneInfo::findZone(Common::String zoneName) {
	for (uint32 i = 0; i < _zones.size(); i++) {
		if (_zones[i]->name.equalsIgnoreCase(zoneName)) {
			return _zones[i];
		}
	}
	warning("Cannot find zone %s", zoneName.c_str());
	return nullptr;
}

Scene *SceneInfo::findScene(Common::String sceneName) {
	for (uint32 i = 0; i < _scenes.size(); i++) {
		if (_scenes[i]->name.equalsIgnoreCase(sceneName)) {
			return _scenes[i];
		}
	}
	error("Cannot find scene %s", sceneName.c_str());
}

void SceneInfo::addScene(Scene *scene) {
	_scenes.push_back(scene);
}

void Zone::addRect(int16 left, int16 top, int16 right, int16 bottom, Common::String scene, uint32 score, Common::String rectHit, Common::String unknown) {
	Rect *rect = new Rect();
	rect->left = left;
	rect->top = top;
	rect->right = right;
	rect->bottom = bottom;
	rect->scene = scene;
	rect->score = score;
	rect->rectHit = rectHit;
	rect->unknown = unknown;
	rects.push_back(*rect);
}

int8 SceneInfo::getToken(const struct TokenEntry *tokenList, Common::String token) {
	for (int i = 0; tokenList[i].name != nullptr; i++) {
		if (token == tokenList[i].name) {
			return tokenList[i].value;
		}
	}
	return -1;
}

bool SceneInfo::ignoreScriptLine(Common::String line) {
	if (line.size() == 0) {
		return true; // empty line
	} else if (line.substr(0, 2) == "//") {
		return true; // comment
	} else if (line.substr(0, 1) == "*") {
		return true; // doc comment
	} else if (line.substr(0, 4) == "NXET") {
		return true; // typo in Maddog2
	} else if (line.substr(0, 5) == "DATA$") {
		return true; // typo in DrugWars
	} else if (line.substr(0, 10) == "NUMBER_OF_") {
		return true; // unnecessary numbers
	}
	return false;
}

} // End of namespace Alg
