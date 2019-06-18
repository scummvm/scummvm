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

bool AI::init() {
	warning("STUB: AI::init required");
	return true;
}

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
		switch (_cine[i]->cmdType) {
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
			warning("STUB: AI::GetPlayerXY required");
			warning("STUB: MAP::CenterMapXY required");
			complete = true;
			break;
		case C_MOVECAMERA:
			_cameraLock = true;
			if (!(_cine[i]->start)) {
				_cine[i]->xv = (((double)_cine[i]->x) - _cameraX) / (double)_cine[i]->speed;
				_cine[i]->yv = (((double)_cine[i]->y) - _cameraY) / (double)_cine[i]->speed;
				_cine[i]->start = 1;
			}
			_cameraX += _cine[i]->xv;
			_cameraY += _cine[i]->yv;
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

} // End of Namespace
