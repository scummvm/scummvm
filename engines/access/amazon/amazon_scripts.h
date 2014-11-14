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

#ifndef ACCESS_AMAZON_SCRIPTS_H
#define ACCESS_AMAZON_SCRIPTS_H

#include "common/scummsys.h"
#include "access/scripts.h"

namespace Access {

namespace Amazon {

class AmazonEngine;

class AmazonScripts: public Scripts {
private:
	AmazonEngine *_game;

	int _xTrack;
	int _yTrack;
	int _zTrack;
	int _xCam;
	int _yCam;
	int _zCam;

	int _pNumObj;
	int _pImgNum[32];
	SpriteResource *_pObject[32];

	int _pObjX[32];
	int _pObjY[32];
	int _pObjZ[32];
	int _pObjXl[16];
	int _pObjYl[16];

	void pan();

protected:
	virtual void executeSpecial(int commandIndex, int param1, int param2);
	virtual void executeCommand(int commandIndex);

	void doFlyCell();
	void doFallCell();
	void scrollFly();
	void scrollFall();
	void scrollJWalk();
	void cLoop();
	void mWhile1();
	void mWhile2();
	void mWhileFly();
	void mWhileFall();
	void mWhileJWalk();
	void mWhileDoOpen();
	void mWhile(int param1);
	void setVerticalCode(Common::Rect bounds);
	void setHorizontalCode(Common::Rect bounds);
	void chkVLine();
	void chkHLine();
	void guardSee();
	void setGuardFrame();
	void guard();
	void plotTorchSpear(int indx, const int *&buf);
	void plotPit(int indx, const int *&buf);
	int antHandleRight(int indx, const int *&buf);
	int antHandleLeft(int indx, const int *&buf);
	int antHandleStab(int indx, const int *&buf);
	void ANT();
	void loadBackground(int param1, int param2);
	void plotInactive();
	void setInactive();
	void boatWalls(int param1, int param2);

	void cmdHelp();
	void cmdCycleBack();
	void cmdChapter();
	void cmdSetHelp();
	void cmdCenterPanel();
	void cmdMainPanel();
	void CMDRETFLASH();
public:
	AmazonScripts(AccessEngine *vm);
};

} // End of namespace Amazon

} // End of namespace Access

#endif /* ACCESS_AMAZON_SCRIPTS_H */
