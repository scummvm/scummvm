/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2006 The ScummVM project
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
 * $Header$
 *
 */

#ifndef SKYMAIN_H
#define SKYMAIN_H

#include "common/stdafx.h"
#include "base/engine.h"

class GameDetector;

namespace Sky {

struct SystemVars {
	uint32 systemFlags;
	uint32 gameVersion;
	uint32 mouseFlag;
	uint16 language;
	uint32 currentPalette;
	uint16 gameSpeed;
	uint16 currentMusic;
	bool pastIntro;
	bool quitGame;
	bool paused;
};

struct Compact;
class Sound;
class Disk;
class Text;
class Logic;
class Mouse;
class Screen;
class Control;
class MusicBase;
class Intro;
class Debugger;
class SkyCompact;

class SkyEngine : public Engine {
	void errorString(const char *buf_input, char *buf_output);
protected:
	byte _keyPressed, _keyFlags;
	bool _floppyIntro;

	int _mouseX, _mouseY;

	Sound *_skySound;
	Disk *_skyDisk;
	Text *_skyText;
	Logic *_skyLogic;
	Mouse *_skyMouse;
	Screen *_skyScreen;
	Control *_skyControl;
	SkyCompact *_skyCompact;
	Debugger *_debugger;

	MusicBase *_skyMusic;
	Intro *_skyIntro;

public:
	SkyEngine(GameDetector *detector, OSystem *syst);
	virtual ~SkyEngine();

	static bool isDemo(void);
	static bool isCDVersion(void);

	static void *fetchItem(uint32 num);
	static void *_itemList[300];

	static SystemVars _systemVars;

protected:
	byte _fastMode;

	void delay(int32 amount);
	int go();
	void handleKey(void);

	uint32 _lastSaveTime;

	int init(GameDetector &detector);
	void initItemList();

	void initVirgin();
	static void timerHandler(void *ptr);
	void gotTimerTick();
	void loadFixedItems();
};

} // End of namespace Sky

#endif
