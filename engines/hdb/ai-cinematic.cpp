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
#include "hdb/ai.h"
#include "hdb/gfx.h"
#include "hdb/lua-script.h"
#include "hdb/map.h"
#include "hdb/sound.h"
#include "hdb/window.h"

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
	if (!_cineActive)
		return;

	// Make sure Dialogs are timing out
	g_hdb->_window->checkDialogClose(0, 0);

	// Make sure Cine Pics are drawing
	for (int i = 0; i < _numCineBlitList; i++) {
		if (_cineBlitList[i]->masked == false)
			_cineBlitList[i]->pic->draw(_cineBlitList[i]->x, _cineBlitList[i]->y);
		else
			_cineBlitList[i]->pic->drawMasked(_cineBlitList[i]->x, _cineBlitList[i]->y);
	}

	// Check for Game Pause
	if (g_hdb->getPause())
		return;

	bool bailOut = false;
	bool complete = false;

	for (uint i = 0; i < _cine.size(); i++) {
		debug(3, "processCines: [%d] %s now: %d  start: %d delay: %d", i, cineTypeStr[_cine[i]->cmdType],
				g_system->getMillis(), _cine[i]->start, _cine[i]->delay);

		switch (_cine[i]->cmdType) {
		case C_STOPCINE:
			{
				char func[64];
				memset(func, 0, 64);

				if (_cine[i]->title)
					Common::strlcpy(func, _cine[i]->title, 64);

				cineCleanup();
				if (func[0])
					g_hdb->_lua->callFunction(func, 0);
			}
			break;
		case C_STARTMAP:
			{
				const char *title = _cine[i]->title;
				// free all gfx alloc'ed during cine
				cineFreeGfx();
				_cineActive = false;
				_playerLock = false;
				_cameraLock = false;
				g_hdb->_window->setInfobarDark(0);
				g_hdb->_gfx->setPointerState(1);
				_cine.resize(0);
				_numCineFreeList = 0;
				_numCineBlitList = 0;
				// if cine is aborted and an abort function was specified, call it
				if (_cineAborted && _cineAbortFunc)
					g_hdb->_lua->callFunction(_cineAbortFunc, 0);
				g_hdb->changeMap(title);
				return;
			}
			break;
		case C_LOCKPLAYER:
			_playerLock = true;
			complete = true;
			if (_player)
				stopEntity(_player);
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
			{
				_cameraLock = false;
				int px, py;
				g_hdb->_ai->getPlayerXY(&px, &py);
				g_hdb->_map->centerMapXY(px + 16, py + 16);
				complete = true;
			}
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
			debug(3, "C_MOVECAMERA: abs(_cameraX - _cine[i]->x): %f, abs(_cameraY - _cine[i]->y): %f", fabs(_cameraX - _cine[i]->x), fabs(_cameraY - _cine[i]->y));
			if (fabs(_cameraX - _cine[i]->x) <= 1 && fabs(_cameraY - _cine[i]->y) <= 1) {
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
			} else if (_cine[i]->delay < g_system->getMillis())
				complete = true;
			else
				bailOut = true;
			break;
		case C_WAITUNTILDONE:
			if (!i)
				complete = true;
			else
				bailOut = true;

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
			} else
				warning("Can't locate '%s' in setEntity", _cine[i]->string);

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
				} else
					warning("Can't locate '%s' in moveEntity", _cine[i]->title);
			} else {
				debug(3, "C_MOVEENTITY: %d, %s tileX: %d, goalX: %d tileY %d, goalY: %d", i, AIType2Str(_cine[i]->e->type), _cine[i]->e->tileX, _cine[i]->e->goalX, _cine[i]->e->tileY, _cine[i]->e->goalY);
				if (!_cine[i]->e->goalX)
					complete = true;
			}
			break;
		case C_ANIMENTITY:
			if (!_cine[i]->start) {
				AIEntity *e = locateEntity(_cine[i]->title);
				if (e) {
					_cine[i]->e = e;
					e->state = (AIState)_cine[i]->speed;
					_cine[i]->start = 1;
					if (_cine[i]->end) // Loop ?
						complete = true;
					e->animFrame = 0;
					e->animDelay = e->animCycle;
					animEntFrames(e);
				} else {
					warning("Can't locate '%s' in animEntity", _cine[i]->title);
					complete = true;
				}
			} else {
				AIEntity *e = _cine[i]->e;
				if (!e->animFrame && e->animDelay == e->animCycle) {
					e->state = STATE_STANDDOWN;
					e->animFrame = 0;
					e->animDelay = e->animCycle;
					complete = true;
				}
			}
			break;
		case C_SETANIMFRAME:
			{
				AIEntity *e = locateEntity(_cine[i]->title);
				if (e) {
					e->state = (AIState)_cine[i]->start;
					e->animFrame = _cine[i]->end;
					e->animDelay = e->animCycle;
					animEntFrames(e);
					e->state = STATE_NONE;
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
						break;
					}
				} else
					warning("Can't find %s to ENTITYFACE", _cine[i]->title);

				complete = true;
			}
			break;
		case C_DIALOG:
			if (_cine[i]->start) {
				g_hdb->_window->openDialog(_cine[i]->title, -1, _cine[i]->string, 0, nullptr);
				g_hdb->_window->setDialogDelay(_cine[i]->delay);
				_cine[i]->start = 0;
			} else if (g_hdb->_window->getDialogDelay() < g_hdb->getTimeSlice())
				complete = true;
			break;
		case C_TEXTOUT:
			if (!_cine[i]->start) {
				g_hdb->_window->textOut(_cine[i]->title, _cine[i]->x, _cine[i]->y, _cine[i]->end);
				_cine[i]->start = 1;
			} else if (!g_hdb->_window->textOutActive())
				complete = true;
			break;
		case C_CENTERTEXTOUT:
			if (!_cine[i]->start) {
				g_hdb->_window->centerTextOut(_cine[i]->title, _cine[i]->y, _cine[i]->end);
				_cine[i]->start = 1;
			} else if (!g_hdb->_window->textOutActive())
				complete = true;
			break;
		case C_DRAWPIC:
			{
				Picture *p = cineFindInBlitList(_cine[i]->id);
				if (p == nullptr) {
					p = g_hdb->_gfx->loadPic(_cine[i]->string);
					cineAddToFreeList(p);
					cineAddToBlitList(_cine[i]->id, p, (int)_cine[i]->x, (int)_cine[i]->y, false);
				}
				_cine[i]->pic = p;
				_cine[i]->pic->draw((int)_cine[i]->x, (int)_cine[i]->y);
				complete = true;
			}
			break;
		case C_DRAWMASKEDPIC:
			{
				Picture *p = cineFindInBlitList(_cine[i]->id);
				if (p == nullptr) {
					p = g_hdb->_gfx->loadPic(_cine[i]->string);
					cineAddToFreeList(p);
					cineAddToBlitList(_cine[i]->id, p, (int)_cine[i]->x, (int)_cine[i]->y, true);
				}
				_cine[i]->pic = p;
				_cine[i]->pic->drawMasked((int)_cine[i]->x, (int)_cine[i]->y);
				complete = true;
			}
			break;

		case C_MOVEPIC:
			if (!_cine[i]->start) {
				Picture *pic = cineFindInBlitList(_cine[i]->id);
				if (!pic) {
					pic = g_hdb->_gfx->loadPic(_cine[i]->string);
					cineAddToFreeList(pic);
				} else
					cineRemoveFromBlitList(_cine[i]->id);
				_cine[i]->pic = pic;
				_cine[i]->start = 1;
			}

			cineRemoveFromBlitList(_cine[i]->id);
			_cine[i]->x += _cine[i]->xv;
			_cine[i]->y += _cine[i]->yv;
			cineAddToBlitList(_cine[i]->id, _cine[i]->pic, (int)_cine[i]->x, (int)_cine[i]->y, false);
			if (abs((int)(_cine[i]->x - _cine[i]->x2)) <= 1 && abs((int)(_cine[i]->y - _cine[i]->y2)) <= 1)
				complete = true;
			break;

		case C_MOVEMASKEDPIC:
			if (!_cine[i]->start) {
				Picture *pic = cineFindInBlitList(_cine[i]->id);
				if (!pic) {
					pic = g_hdb->_gfx->loadPic(_cine[i]->string);
					cineAddToFreeList(pic);
				} else
					cineRemoveFromBlitList(_cine[i]->id);
				_cine[i]->pic = pic;
				_cine[i]->start = 1;
			}

			cineRemoveFromBlitList(_cine[i]->id);
			_cine[i]->x += _cine[i]->xv;
			_cine[i]->y += _cine[i]->yv;
			cineAddToBlitList(_cine[i]->id, _cine[i]->pic, (int)_cine[i]->x, (int)_cine[i]->y, true);
			if (abs((int)(_cine[i]->x - _cine[i]->x2)) <= 1 && abs((int)(_cine[i]->y - _cine[i]->y2)) <= 1)
				complete = true;
			break;

		case C_USEENTITY:
			for (Common::Array<AIEntity *>::iterator it = _ents->begin(); it != _ents->end(); ++it) {
				if (Common::matchString((*it)->entityName, _cine[i]->string, true))
					g_hdb->useEntity((*it));
			}
			for (int k = 0; k < kMaxActions; k++) {
				if (Common::matchString(_actions[k].entityName, _cine[i]->string, true)) {
					checkActionList(&_dummyPlayer, _actions[k].x1, _actions[k].y1, false);
					checkActionList(&_dummyPlayer, _actions[k].x2, _actions[k].y2, false);
				}
			}
			for (int j = 0; j < kMaxAutoActions; j++) {
				if (Common::matchString(_autoActions[j].entityName, _cine[i]->string, true) && !_autoActions[j].activated)
					checkAutoList(&_dummyPlayer, _autoActions[j].x, _autoActions[j].y);
			}
			complete = true;
			break;
		case C_PLAYSOUND:
			g_hdb->_sound->playSound((int)_cine[i]->start);
			complete = true;
			break;
		case C_PLAYVOICE:
			g_hdb->_sound->playVoice((int)_cine[i]->x, (int)_cine[i]->y);
			complete = true;
			break;
		case C_FADEIN:
			if (!_cine[i]->start) {
				g_hdb->_gfx->setFade(true, (bool)_cine[i]->end, _cine[i]->speed);
				_cine[i]->start = 1;
			} else if (!g_hdb->_gfx->isFadeActive())
				complete = true;
			break;
		case C_FADEOUT:
			if (!_cine[i]->start) {
				g_hdb->_gfx->setFade(false, (bool)_cine[i]->end, _cine[i]->speed);
				_cine[i]->start = 1;
			} else if (!g_hdb->_gfx->isFadeActive())
				complete = true;
			break;
		case C_SPAWNENTITY:
			{
				int x2 = (int)_cine[i]->x2;
				int y2 = (int)_cine[i]->y2;
				spawn((AIType)x2, (AIDir)y2, (int)_cine[i]->x, (int)_cine[i]->y, _cine[i]->title, _cine[i]->string,
					_cine[i]->id, (AIDir)_cine[i]->start, (int)_cine[i]->end, (int)_cine[i]->delay, (int)_cine[i]->speed, 1);
				complete = true;
			}
			break;
		case C_REMOVEENTITY:
			{
				AIEntity *e = locateEntity(_cine[i]->string);
				if (e)
					removeEntity(e);
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
		case C_FUNCTION:
			g_hdb->_lua->callFunction(_cine[i]->title, 0);
			complete = true;
			break;
		default:
			break;
		}

		if (bailOut)
			return;

		if (complete && _cine.size()) {
			delete _cine[i];
			_cine.remove_at(i);
			i--;
			complete = false;
		}
	}
}

void AI::cineCleanup() {
	cineFreeGfx();
	_cineActive = false;
	// If aborted and abort function specified, call it
	if (_cineAborted && _cineAbortFunc)
		g_hdb->_lua->callFunction(_cineAbortFunc, 0);

	_cameraLock = false;
	_playerLock = false;
	g_hdb->_window->setInfobarDark(0);
	g_hdb->_gfx->setPointerState(1);

	int px, py;
	getPlayerXY(&px, &py);
	g_hdb->_map->centerMapXY(px + 16, py + 16);
}

void AI::cineAbort() {
	for (uint i = 0; i < _cine.size(); i++) {
		if (_cine[i]->cmdType == C_STARTMAP || _cine[i]->cmdType == C_STOPCINE)
			_cine[0] = _cine[i];
	}

	_cine.resize(1);

	g_hdb->_window->closeAll();

	if (_player)
		stopEntity(_player);
	_cineAborted = true;
}

void AI::cineAddToBlitList(const char *id, Picture *pic, int x, int y, bool masked) {
	_cineBlitList[_numCineBlitList] = new CineBlit;
	_cineBlitList[_numCineBlitList]->id = id;
	_cineBlitList[_numCineBlitList]->pic = pic;
	_cineBlitList[_numCineBlitList]->x = x;
	_cineBlitList[_numCineBlitList]->y = y;
	_cineBlitList[_numCineBlitList]->masked = masked;
	_numCineBlitList++;
}

Picture *AI::cineFindInBlitList(const char *name) {
	for (int i = 0; i < _numCineBlitList; i++) {
		if (Common::matchString(_cineBlitList[i]->id, name, true))
			return _cineBlitList[i]->pic;
	}
	return nullptr;
}

void AI::cineRemoveFromBlitList(const char *name) {
	for (int i = 0; i < _numCineBlitList; i++) {
		if (Common::matchString(_cineBlitList[i]->id, name, true)) {
			delete _cineBlitList[i];
			for (; i < _numCineBlitList - 1; i++)
				_cineBlitList[i] = _cineBlitList[i + 1];
			_numCineBlitList--;
			_cineBlitList[_numCineBlitList] = nullptr;
			return;
		}
	}
}

void AI::cineAddToFreeList(Picture *pic) {
	if (_numCineFreeList >= kMaxCineGfx) {
		warning("cineAddToFreeList: Too many gfx in Cinematic!");
		return;
	}
	_cineFreeList[_numCineFreeList] = pic;
	_numCineFreeList++;
}

void AI::cineFreeGfx() {
	for (int i = 0; i < _numCineFreeList; i++)
		delete _cineFreeList[i];

	_numCineFreeList = 0;
}

void AI::cineStart(bool abortable, const char *abortFunc) {
	_cineAbortable = abortable;
	_cineAborted = false;
	_cineAbortFunc = abortFunc;
	_numCineBlitList = 0;
	_numCineFreeList = 0;
	_cineActive = true;
	_playerLock = false;
	_cameraLock = false;

	_cine.clear();
}

void AI::cineStop(const char *funcNext) {
	CineCommand *cmd = new CineCommand;
	cmd->cmdType = C_STOPCINE;
	cmd->title = funcNext;
	_cine.push_back(cmd);
}

void AI::cineStartMap(const char *mapName) {
	CineCommand *cmd = new CineCommand;
	cmd->cmdType = C_STARTMAP;
	cmd->title = mapName;
	_cine.push_back(cmd);
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

void AI::cineAnimEntity(const char *entName, AIState state, int loop) {
	CineCommand *cmd = new CineCommand;
	cmd->start = 0;
	cmd->title = entName;
	cmd->speed = state;
	cmd->end = loop;
	cmd->cmdType = C_ANIMENTITY;
	_cine.push_back(cmd);
}

void AI::cineSetAnimFrame(const char *entName, AIState state, int frame) {
	CineCommand *cmd = new CineCommand;
	cmd->start = state;
	cmd->title = entName;
	cmd->end = frame;
	cmd->cmdType = C_SETANIMFRAME;
	_cine.push_back(cmd);
}

void AI::cineEntityFace(const char *luaName, double dir) {
	CineCommand *cmd = new CineCommand;
	cmd->title = luaName;
	cmd->x = dir;
	cmd->cmdType = C_ENTITYFACE;
	_cine.push_back(cmd);
}

void AI::cineSpawnEntity(AIType t, AIDir d, int x, int y, const char *func_init, const char *func_action,
				const char *func_use, AIDir d2, int level, int value1, int value2) {
	CineCommand *cmd = new CineCommand;
	cmd->cmdType = C_SPAWNENTITY;
	cmd->x2 = (double)t;
	cmd->y2 = (double)d;
	cmd->x = (double)x;
	cmd->y = (double)y;
	cmd->title = func_init;
	cmd->string = func_action;
	cmd->id = func_use;
	cmd->start = (int)d2;
	cmd->end = level;
	cmd->delay = value1;
	cmd->speed = value2;

	_cine.push_back(cmd);
}

void AI::cineRemoveEntity(const char *entName) {
	CineCommand *cmd = new CineCommand;
	cmd->string = entName;
	cmd->cmdType = C_REMOVEENTITY;
	_cine.push_back(cmd);
}

void AI::cineDialog(const char *title, const char *string, int seconds) {
	CineCommand *cmd = new CineCommand;
	cmd->title = title;
	cmd->string = string;
	cmd->delay = seconds;
	cmd->start = 1;
	if (!title || !string)
		warning("cineDialog: Missing Title or Text");
	cmd->cmdType = C_DIALOG;
	debug(6, "In cineDialog: C_DIALOG created. cmd->start: %d, cmd->title: %s", cmd->start, cmd->title);
	_cine.push_back(cmd);
}

void AI::cineTextOut(const char *text, int x, int y, int timer) {
	CineCommand *cmd = new CineCommand;
	cmd->title = text;
	cmd->x = x;
	cmd->y = y;
	cmd->end = timer;
	cmd->start = 0;
	cmd->cmdType = C_TEXTOUT;
	_cine.push_back(cmd);
}

void AI::cineCenterTextOut(const char *text, int y, int timer) {
	CineCommand *cmd = new CineCommand;
	cmd->title = text;
	cmd->y = y;
	cmd->end = timer;
	cmd->start = 0;
	cmd->cmdType = C_CENTERTEXTOUT;
	_cine.push_back(cmd);
}

void AI::cineDrawPic(const char *id, const char *pic, int x, int y) {
	if (!pic || !id) {
		warning("cineDrawPic: Missing ID or PIC");
		return;
	}

	CineCommand *cmd = new CineCommand;
	cmd->x = x;
	cmd->y = y;
	cmd->string = pic;
	cmd->id = id;
	cmd->cmdType = C_DRAWPIC;
	_cine.push_back(cmd);
}

void AI::cineDrawMaskedPic(const char *id, const char *pic, int x, int y) {
	if (!pic || !id) {
		warning("cineDrawMaskedPic: Missing ID or PIC");
		return;
	}

	CineCommand *cmd = new CineCommand;
	cmd->x = x;
	cmd->y = y;
	cmd->string = pic;
	cmd->id = id;
	cmd->cmdType = C_DRAWMASKEDPIC;
	_cine.push_back(cmd);
}

void AI::cineMovePic(const char *id, const char *pic, int x1, int y1, int x2, int y2, int speed) {
	if (!pic || !id) {
		warning("cineMovePic: Missing ID or PIC");
		return;
	}

	CineCommand *cmd = new CineCommand;
	cmd->x = x1;
	cmd->y = y1;
	cmd->x2 = x2;
	cmd->y2 = y2;
	cmd->speed = speed;
	cmd->xv = ((double)(x2-x1)) / (double)speed;
	cmd->yv = ((double)(y2-y1)) / (double)speed;
	cmd->start = 0;
	cmd->string = pic;
	cmd->id = id;
	cmd->cmdType = C_MOVEPIC;
	_cine.push_back(cmd);
}

void AI::cineMoveMaskedPic(const char *id, const char *pic, int x1, int y1, int x2, int y2, int speed) {
	if (!pic || !id) {
		warning("cineMoveMaskedPic: Missing ID or PIC");
		return;
	}

	CineCommand *cmd = new CineCommand;
	cmd->x = x1;
	cmd->y = y1;
	cmd->x2 = x2;
	cmd->y2 = y2;
	cmd->speed = speed;
	cmd->xv = ((double)(x2-x1)) / (double)speed;
	cmd->yv = ((double)(y2-y1)) / (double)speed;
	cmd->start = 0;
	cmd->string = pic;
	cmd->id = id;
	cmd->cmdType = C_MOVEMASKEDPIC;
	_cine.push_back(cmd);
}

void AI::cineUse(const char *entName) {
	CineCommand *cmd = new CineCommand;
	cmd->string = entName;
	cmd->cmdType = C_USEENTITY;
	_cine.push_back(cmd);
}

void AI::cinePlaySound(int index) {
	CineCommand *cmd = new CineCommand;
	cmd->start = index;
	cmd->cmdType = C_PLAYSOUND;
	_cine.push_back(cmd);
}

void AI::cinePlayVoice(int index, int actor) {
	CineCommand *cmd = new CineCommand;
	cmd->x = index;
	cmd->y = actor;
	cmd->cmdType = C_PLAYVOICE;
	_cine.push_back(cmd);
}

void AI::cineFadeIn(bool isBlack, int steps) {
	CineCommand *cmd = new CineCommand;
	cmd->speed = steps;
	cmd->end = (int)isBlack;
	cmd->start = 0;
	cmd->cmdType = C_FADEIN;
	_cine.push_back(cmd);
}

void AI::cineFadeOut(bool isBlack, int steps) {
	CineCommand *cmd = new CineCommand;
	cmd->speed = steps;
	cmd->end = (int)isBlack;
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

void AI::cineFunction(const char *func) {
	CineCommand *cmd = new CineCommand;
	cmd->title = func;
	cmd->cmdType = C_FUNCTION;
	_cine.push_back(cmd);
}

} // End of Namespace
