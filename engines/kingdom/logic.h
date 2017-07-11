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

#ifndef KINGDOM_LOGIC_H
#define KINGDOM_LOGIC_H

#include "gui/debugger.h"

namespace Kingdom {

class KingdomGame;

class Logic {
private:
	KingdomGame *_vm;

public:
	int _statPlay;
	int _oldStatPlay;
	int _nodeNum;
	int _oldNode; // CHECKME: Useless? If so, to be removed
	int _nextNode;
	int _robberyNode;
	int16 _nodes[99];
	int _currMap;
	bool _oldEye;
	bool _eye;
	bool _oldHelp;
	bool _help;
	int _lastObstacle;
	bool _lastObs;
	int _inventory[19];
	bool _replay;
	bool _pouch;
	bool _oldPouch;
	bool _resurrect;
	bool _tideCntl;
	int _rtnNode;
	int _health;
	int _healthOld;
	bool _fstFwd;
	bool _spell1;
	bool _spell2;
	bool _spell3;

	int _mapStat; // CHECKME: Useless? If so, to be removed

	void enAll(); // TODO: Rename later as enableAll
	void dsAll(); // TODO: Rename later as disableAll
	bool wound();
	void GPLogic1_SubSP10();
	void GPLogic1_SubSP121();
	void GPLogic2_SubSP361();
	void GPLogic4_SP901();
	bool chkDesertObstacles();
	void increaseHealth();
	void endCredits();
	void gameHelp_Sub43C();

public:
	explicit Logic(Kingdom::KingdomGame *vm);
	virtual ~Logic(void) {}

	void GPLogic1();
	void GPLogic2();
	void GPLogic3();
	void GPLogic4();
	void gameHelp();
	void inventoryDel(int item);
	void inventoryAdd(int item);
	void initVariables();
	void initPlay();
	void switchAS();
	void synchronize(Common::Serializer &s);
};
}

#endif
