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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef HOPKINS_EVENTS_H
#define HOPKINS_EVENTS_H

#include "common/scummsys.h"
#include "common/events.h"
#include "common/str.h"

namespace Hopkins {

#define GAME_FRAME_RATE 50
#define GAME_FRAME_TIME (1000 / GAME_FRAME_RATE)

class HopkinsEngine;

class EventsManager {
private:
	HopkinsEngine *_vm;

	void pollEvents();
	void handleKey(Common::Event &event);
	void checkForNextFrameCounter();
public:
	bool souris_flag;
	bool mouse_linux;
	int souris_sizex, souris_sizey;
	int ofset_souris_x, ofset_souris_y;
	int start_x, start_y;
	int souris_x, souris_y;
	bool CASSE;
	int OLD_ICONE;
	int souris_n;
	int souris_bb;
	int souris_b;
	byte *pointeur_souris;
	uint32 _gameCounter;
	uint32 lItCounter;
	uint32 _priorCounterTime;
	uint32 _priorFrameTime;
	bool ESC_KEY;
	bool NOESC;
	bool KEY_INVENT;
	int btsouris;
public:
	EventsManager();
	void setParent(HopkinsEngine *vm);

	void INSTALL_SOURIS();
	void souris_on();
	void souris_xy(int xp, int yp);
	void souris_max();
	int XMOUSE();
	int YMOUSE();
	int BMOUSE();
	void MOUSE_ON();
	void MOUSE_OFF();
	void CHANGE_MOUSE(int id);

	void CONTROLE_MES();
	void delay(int delay);
	void VBL();
};

} // End of namespace Hopkins

#endif /* HOPKINS_EVENTS_H */
