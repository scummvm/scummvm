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

#ifndef KYRA_DEBUGGER_H
#define KYRA_DEBUGGER_H

#include "gui/debugger.h"

namespace Kyra {

class KyraEngine_v1;
class KyraEngine_LoK;
class KyraEngine_v2;
class KyraEngine_HoF;

class Debugger : public ::GUI::Debugger {
public:
	Debugger(KyraEngine_v1 *vm);
	~Debugger() override {}  // we need this for __SYMBIAN32__ archaic gcc/UIQ

	virtual void initialize();

protected:
	KyraEngine_v1 *_vm;

	bool cmdSetScreenDebug(int argc, const char **argv);
	bool cmdLoadPalette(int argc, const char **argv);
	bool cmdShowFacings(int argc, const char **argv);
	bool cmdGameSpeed(int argc, const char **argv);
	bool cmdListFlags(int argc, const char **argv);
	bool cmdToggleFlag(int argc, const char **argv);
	bool cmdQueryFlag(int argc, const char **argv);
	bool cmdListTimers(int argc, const char **argv);
	bool cmdSetTimerCountdown(int argc, const char **argv);
};

class Debugger_LoK : public Debugger {
public:
	Debugger_LoK(KyraEngine_LoK *vm);
	~Debugger_LoK() override {}  // we need this for __SYMBIAN32__ archaic gcc/UIQ

	void initialize() override;
protected:
	KyraEngine_LoK *_vm;

	bool cmdEnterRoom(int argc, const char **argv);
	bool cmdListScenes(int argc, const char **argv);
	bool cmdGiveItem(int argc, const char **argv);
	bool cmdListBirthstones(int argc, const char **argv);
};

class Debugger_v2 : public Debugger {
public:
	Debugger_v2(KyraEngine_v2 *vm);
	~Debugger_v2() override {}

	void initialize() override;
protected:
	KyraEngine_v2 *_vm;

	bool cmdEnterScene(int argc, const char **argv);
	bool cmdListScenes(int argc, const char **argv);
	bool cmdSceneInfo(int argc, const char **argv);
	bool cmdCharacterInfo(int argc, const char **argv);
	bool cmdSceneToFacing(int argc, const char **argv);
	bool cmdGiveItem(int argc, const char **argv);
};

class Debugger_HoF : public Debugger_v2 {
public:
	Debugger_HoF(KyraEngine_HoF *vm);

	void initialize() override;
protected:
	KyraEngine_HoF *_vm;

	bool cmdPasscodes(int argc, const char **argv);
};

#ifdef ENABLE_LOL
class LoLEngine;

class Debugger_LoL : public Debugger {
public:
	Debugger_LoL(LoLEngine *vm);

protected:
	LoLEngine *_vm;
};
#endif // ENABLE_LOL

#ifdef ENABLE_EOB
class EoBCoreEngine;

class Debugger_EoB : public Debugger {
public:
	Debugger_EoB(EoBCoreEngine *vm);

	void initialize() override;
protected:
	EoBCoreEngine *_vm;

	bool cmdImportSaveFile(int argc, const char **argv);
	bool cmdSaveOriginal(int argc, const char **argv);
	bool cmdListMonsters(int argc, const char **argv);
	bool cmdShowPosition(int argc, const char **argv);
	bool cmdSetPosition(int argc, const char **argv);
	bool cmdPrintMap(int argc, const char **argv);
	bool cmdOpenDoor(int argc, const char **argv);
	bool cmdCloseDoor(int argc, const char **argv);
	bool cmdListFlags(int argc, const char **argv);
	bool cmdSetFlag(int argc, const char **argv);
	bool cmdClearFlag(int argc, const char **argv);
};
#endif // ENABLE_EOB

} // End of namespace Kyra

#endif
