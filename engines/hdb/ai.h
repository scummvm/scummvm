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

#ifndef HDB_AI_H
#define HDB_AI_H

#include "common/system.h"

namespace HDB {

enum CineType {
	C_NO_COMMAND,
	C_STOPCINE,
	C_LOCKPLAYER,
	C_UNLOCKPLAYER,
	C_SETCAMERA,
	C_MOVECAMERA,
	C_WAIT,
	C_WAITUNTILDONE,
	C_MOVEENTITY,
	C_DIALOG,
	C_ANIMENTITY,
	C_RESETCAMERA,
	C_SETENTITY,
	C_STARTMAP,
	C_MOVEPIC,
	C_MOVEMASKEDPIC,
	C_DRAWPIC,
	C_DRAWMASKEDPIC,
	C_FADEIN,
	C_FADEOUT,
	C_SPAWNENTITY,
	C_PLAYSOUND,
	C_CLEAR_FG,
	C_SET_FG,
	C_SET_BG,
	C_FUNCTION,
	C_ENTITYFACE,
	C_USEENTITY,
	C_REMOVEENTITY,
	C_SETANIMFRAME,
	C_TEXTOUT,
	C_CENTERTEXTOUT,
	C_PLAYVOICE,

	C_ENDLIST
};

struct CineCommand {
	CineType cmdType;
	double x, y;
	double x2, y2;
	double xv, yv;
	int start, end;
	uint32	delay;
	int	speed;
	char *title;
	char *string;
	char *id;
	// AIEntity *entity
};

class AI {
public:

	AI();
	~AI();

	// Cinematic Functions
	void processCines();
	void cineStart(bool abortable, char *abortFunc);
	void cineSetCamera(int x, int y);
	void cineResetCamera();
	void cineMoveCamera(int x, int y, int speed);

	Common::Array<CineCommand *> *_cine;

private:
	// Cinematics Variables
	bool _cineAbortable;
	bool _cineAborted;
	char *_cineAbortFunc;
	bool _cineActive;
	bool _playerLock;
	bool _cameraLock;
	double _cameraX, _cameraY;

};

} // End of Namespace

#endif // !HDB_AI_H
