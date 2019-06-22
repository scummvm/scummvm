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

#include "hdb/hdb.h"

namespace HDB {

static const char *cineTypeStr[] = {
	"C_NO_COMMAND",
	"C_STOPCINE",
	"C_LOCKPLAYER",
	"C_UNLOCKPLAYER",
	"C_SETCAMERA",
	"C_MOVECAMERA",
	"C_WAIT",
	"C_WAITUNTILDONE",
	"C_MOVEENTITY",
	"C_DIALOG",
	"C_ANIMENTITY",
	"C_RESETCAMERA",
	"C_SETENTITY",
	"C_STARTMAP",
	"C_MOVEPIC",
	"C_MOVEMASKEDPIC",
	"C_DRAWPIC",
	"C_DRAWMASKEDPIC",
	"C_FADEIN",
	"C_FADEOUT",
	"C_SPAWNENTITY",
	"C_PLAYSOUND",
	"C_CLEAR_FG",
	"C_SET_FG",
	"C_SET_BG",
	"C_FUNCTION",
	"C_ENTITYFACE",
	"C_USEENTITY",
	"C_REMOVEENTITY",
	"C_SETANIMFRAME",
	"C_TEXTOUT",
	"C_CENTERTEXTOUT",
	"C_PLAYVOICE",

	"C_ENDLIST"
};

void AI::processCines() {

	bool complete, bailOut;

	if (!_cineActive) {
		return;
	}

	bailOut = complete = false;

	// TODO: Make sure Dialogs are timing out

	// TODO: Make sure Cine Pics are drawing

	// TODO: Check for Game Pause

	for (uint i = 0; i < _cine.size();i++) {
		debug(3, "processCines: [%d] %s now: %d  start: %d delay: %d", i, cineTypeStr[_cine[i]->cmdType],
				g_system->getMillis(), _cine[i]->start, _cine[i]->delay);

		switch (_cine[i]->cmdType) {
		case C_LOCKPLAYER:
			_playerLock = true;
			complete = true;
			if (_player) {
				stopEntity(_player);
			}
			clearWaypoints();
			break;
		case C_UNLOCKPLAYER:
			_playerLock = false;
			complete = true;
			break;
		case C_SETCAMERA:
			_cameraX = _cine[i]->x;
			_cameraY = _cine[i]->y;
			g_hdb->_map->centerMapXY((int)_cameraX + 16, (int)_cameraY + 16);
			_cameraLock = true;
			complete = true;
			break;
		case C_RESETCAMERA:
			int px, py;
			_cameraLock = false;
			g_hdb->_ai->getPlayerXY(&px, &py);
			g_hdb->_map->centerMapXY(px + 16, py + 16);
			complete = true;
			break;
		case C_MOVECAMERA:
			_cameraLock = true;
			if (!(_cine[i]->start)) {
				debug(3, "C_MOVECAMERA: [%d] now: x: %f, y: %f, speed: %d", i, _cine[i]->x, _cine[i]->y, _cine[i]->speed);
				_cine[i]->xv = (((double)_cine[i]->x) - _cameraX) / (double)_cine[i]->speed;
				_cine[i]->yv = (((double)_cine[i]->y) - _cameraY) / (double)_cine[i]->speed;
				_cine[i]->start = 1;
			}
			_cameraX += _cine[i]->xv;
			_cameraY += _cine[i]->yv;
			debug(3, "C_MOVECAMERA: _cine[%d]->xv: %f, _cine[%d]->yv: %f", i, _cine[i]->xv, i, _cine[i]->yv);
			debug(3, "C_MOVECAMERA: abs(_cameraX - _cine[i]->x): %f, abs(_cameraY - _cine[i]->y): %f", abs(_cameraX - _cine[i]->x), abs(_cameraY - _cine[i]->y));
			if (abs(_cameraX - _cine[i]->x) <= 1 && abs(_cameraY - _cine[i]->y) <= 1) {
				_cameraX = _cine[i]->x;
				_cameraY = _cine[i]->y;
				complete = true;
			}
			g_hdb->_map->centerMapXY((int)_cameraX + 16, (int)_cameraY + 16);
			break;
		case C_WAIT:
			if (!(_cine[i]->start)) {
				_cine[i]->start = 1;
				_cine[i]->delay = g_system->getMillis() + _cine[i]->delay * 1000;
			} else {
				if (_cine[i]->delay < g_system->getMillis()) {
					complete = true;
				} else {
					bailOut = true;
				}
			}
			break;
		case C_WAITUNTILDONE:
			if (!i) {
				complete = true;
			} else {
				bailOut = true;
			}
			break;
		case C_SETENTITY:
			_cine[i]->e = locateEntity(_cine[i]->string);
			if (_cine[i]->e) {
				_cine[i]->e->tileX = (int)_cine[i]->x / kTileWidth;
				_cine[i]->e->x = (int)_cine[i]->x;
				_cine[i]->e->tileY = (int)_cine[i]->y / kTileHeight;
				_cine[i]->e->y = (int)_cine[i]->y;
				_cine[i]->e->level = (int)_cine[i]->x2;
				debug(2, "Found '%s' in setEntity", _cine[i]->string);
			} else {
				warning("Can't locate '%s' in setEntity", _cine[i]->string);
			}
			complete = true;
			break;
		case C_MOVEENTITY:
			if (!_cine[i]->start) {
				AIEntity *e = locateEntity(_cine[i]->title);
				if (e) {
					_cine[i]->e = e;
					_cine[i]->e->moveSpeed = _cine[i]->speed;
					_cine[i]->e->level = (int)_cine[i]->x2;
					setEntityGoal(_cine[i]->e, (int)_cine[i]->x, (int)_cine[i]->y);
					_cine[i]->start = 1;
				} else {
					warning("Can't locate '%s' in moveEntity", _cine[i]->title);
				}
			} else {
				if (!_cine[i]->e->goalX) {
					complete = true;
				}
			}
			break;
		case C_ENTITYFACE:
		{
			AIEntity *e = locateEntity(_cine[i]->title);

			if (e) {
				int d = (int)_cine[i]->x;
				e->dir = (AIDir)d;
				switch (e->dir) {
				case DIR_UP:
					e->state = STATE_STANDUP;
					break;
				case DIR_DOWN:
					e->state = STATE_STANDDOWN;
					break;
				case DIR_LEFT:
					e->state = STATE_STANDLEFT;
					break;
				case DIR_RIGHT:
					e->state = STATE_STANDRIGHT;
					break;
				default:
					warning("AI-CINEMATIC: processCines: DIR_NONE");
				}
			} else {
				warning("Can't find %s to ENTITYFACE", _cine[i]->title);
			}
			complete = true;
			break;
		}
		case C_USEENTITY:
			for (Common::Array<AIEntity *>::iterator it = _ents->begin(); it != _ents->end(); it++) {
				if (Common::matchString((*it)->entityName, _cine[i]->string)) {
					g_hdb->useEntity((*it));
				}
			}
			warning("STUB: PROCESSCINES: USEENTITY: CheckActionList required;");
#if 0
			for (int i = 0;i < kMaxAutoActions;i++) {
				if (Common::matchString(_autoActions[i].entityName, _cine[i]->string) && !_autoActions[i].activated)
					checkAutoList(&_dummyPlayer, _autoActions[i].x, _autoActions[i].y);
			}
#endif
			complete = true;
			break;
		case C_FADEIN:
			if (!_cine[i]->start) {
				g_hdb->_drawMan->setFade(true, (bool)_cine[i]->end, _cine[i]->speed);
				_cine[i]->start = 1;
			} else if (!g_hdb->_drawMan->isFadeActive()) {
				complete = true;
			}
			break;
		case C_FADEOUT:
			if (!_cine[i]->start) {
				g_hdb->_drawMan->setFade(false, (bool)_cine[i]->end, _cine[i]->speed);
				_cine[i]->start = 1;
			} else if (!g_hdb->_drawMan->isFadeActive()) {
				complete = true;
			}
			break;
		case C_CLEAR_FG:
			g_hdb->_map->setMapFGTileIndex((int)_cine[i]->x, (int)_cine[i]->y, -1);
			g_hdb->_map->removeFGTileAnimation((int)_cine[i]->x, (int)_cine[i]->y);
			complete = true;
			break;
		case C_SET_BG:
			g_hdb->_map->setMapBGTileIndex((int)_cine[i]->x, (int)_cine[i]->y, (int)_cine[i]->start);
			g_hdb->_map->addBGTileAnimation((int)_cine[i]->x, (int)_cine[i]->y);
			complete = true;
			break;
		case C_SET_FG:
			g_hdb->_map->setMapFGTileIndex((int)_cine[i]->x, (int)_cine[i]->y, (int)_cine[i]->start);
			g_hdb->_map->addFGTileAnimation((int)_cine[i]->x, (int)_cine[i]->y);
			complete = true;
			break;
		default:
			warning("STUB: AI::PROCESSCINES incomplete for %d", _cine[i]->cmdType);
			break;
		}

		if (bailOut) {
			return;
		}

		if (complete) {
			if (_cine.size()) {
				_cine.remove_at(i);
				i--;
				complete = false;
			}
		}
	}
}

void AI::cineStart(bool abortable, const char *abortFunc) {
	_cineAbortable = abortable;
	_cineAborted = false;
	_cineAbortFunc = abortFunc;
	_cineActive = true;
	_playerLock = false;
	_cameraLock = false;
}

void AI::cineLockPlayer() {
	CineCommand *cmd = new CineCommand;
	cmd->cmdType = C_LOCKPLAYER;
	_cine.push_back(cmd);
}

void AI::cineUnlockPlayer() {
	CineCommand *cmd = new CineCommand;
	cmd->cmdType = C_UNLOCKPLAYER;
	_cine.push_back(cmd);
}

void AI::cineSetCamera(int x, int y) {
	CineCommand *cmd = new CineCommand;
	cmd->x = x * kTileWidth;
	cmd->y = y * kTileHeight;
	cmd->cmdType = C_SETCAMERA;
	_cine.push_back(cmd);
}

void AI::cineResetCamera() {
	CineCommand *cmd = new CineCommand;
	cmd->cmdType = C_RESETCAMERA;
	_cine.push_back(cmd);
}

void AI::cineMoveCamera(int x, int y, int speed) {
	CineCommand *cmd = new CineCommand;
	cmd->start = 0;
	cmd->x = x * kTileWidth;
	cmd->y = y * kTileHeight;
	cmd->speed = speed;
	debug(2, "Setting up C_MOVECAMERA: x: %f, y: %f", cmd->x, cmd->y);
	cmd->cmdType = C_MOVECAMERA;
	_cine.push_back(cmd);
}

void AI::cineWait(int seconds) {
	CineCommand *cmd = new CineCommand;
	cmd->start = 0;
	cmd->cmdType = C_WAIT;
	cmd->delay = seconds;
	_cine.push_back(cmd);
}

void AI::cineWaitUntilDone() {
	CineCommand *cmd = new CineCommand;
	cmd->cmdType = C_WAITUNTILDONE;
	_cine.push_back(cmd);
}

void AI::cineSetEntity(const char *entName, int x, int y, int level) {
	CineCommand *cmd = new CineCommand;
	cmd->string = entName;
	cmd->x = x * kTileWidth;
	cmd->y = y * kTileHeight;
	cmd->x2 = level;
	cmd->cmdType = C_SETENTITY;
	_cine.push_back(cmd);
}

void AI::cineMoveEntity(const char *entName, int x, int y, int level, int speed) {
	CineCommand *cmd = new CineCommand;
	cmd->x = x;
	cmd->y = y;
	cmd->x2 = level;
	cmd->start = 0;
	cmd->speed = speed;
	cmd->title = entName;
	cmd->cmdType = C_MOVEENTITY;
	_cine.push_back(cmd);
}

void AI::cineEntityFace(const char *luaName, double dir) {
	CineCommand *cmd = new CineCommand;
	cmd->title = luaName;
	cmd->x = dir;
	cmd->cmdType = C_ENTITYFACE;
	_cine.push_back(cmd);
}

void AI::cineUse(const char *entName) {
	CineCommand *cmd = new CineCommand;
	cmd->string = entName;
	cmd->cmdType = C_USEENTITY;
	_cine.push_back(cmd);
}

void AI::cineFadeIn(bool isBlack, int steps) {
	CineCommand *cmd = new CineCommand;
	cmd->speed = steps;
	cmd->end = (int) isBlack;
	cmd->start = 0;
	cmd->cmdType = C_FADEIN;
	_cine.push_back(cmd);
}

void AI::cineFadeOut(bool isBlack, int steps) {
	CineCommand *cmd = new CineCommand;
	cmd->speed = steps;
	cmd->end = (int) isBlack;
	cmd->start = 0;
	cmd->cmdType = C_FADEOUT;
	_cine.push_back(cmd);
}

void AI::cineClearForeground(int x, int y) {
	CineCommand *cmd = new CineCommand;
	cmd->x = x;
	cmd->y = y;
	cmd->cmdType = C_CLEAR_FG;
	_cine.push_back(cmd);
}

void AI::cineSetBackground(int x, int y, int index) {
	CineCommand *cmd = new CineCommand;
	cmd->x = x;
	cmd->y = y;
	cmd->start = index;
	cmd->cmdType = C_SET_BG;
	_cine.push_back(cmd);
}

void AI::cineSetForeground(int x, int y, int index) {
	CineCommand *cmd = new CineCommand;
	cmd->x = x;
	cmd->y = y;
	cmd->start = index;
	cmd->cmdType = C_SET_FG;
	_cine.push_back(cmd);
}

} // End of Namespace
