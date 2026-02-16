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

#ifndef XPLIB_XPLIB_H
#define XPLIB_XPLIB_H

#include "bolt/bolt.h"

namespace Bolt {

struct DisplaySpecs;
class BoltEngine;

class XpLib {
public:
	XpLib(BoltEngine *bolt);
	~XpLib();

	void swapWord();
	void swapLong();
	void blit();
	void maskBlit();
	void setMem();
	void terminate();
	void initialize();
	void getPalette();
	void setPalette();
	void startCycle();
	void stopCycle();
	void setScreenBrightness();
	void readCursor();
	void setCursorPos();
	void setCursorImage();
	void setCursorColor();
	void showCursor();
	void hideCursor();
	void getEvent();
	void peekEvent();
	void postEvent();
	void enableController();
	void disableController();
	void setInactivityTimer();
	void setScreenSaverTimer(int32 time);
	bool chooseDisplaySpec(int *outMode, int numSpecs, DisplaySpecs *specs);
	void setCoordSpec(int32 x, int32 y, int32 width, int32 height);
	void displayPic();
	void updateDisplay();
	void setFrameRate();
	void setTransparency(bool toggle);
	void fillDisplay();
	void getRandom();
	void randomize();
	void openFile();
	void closeFile();
	void readFile();
	void setFilePos();
	byte *allocMem(int32 size);
	void tryAllocMem();
	void freeMem(void *mem);
	void playSound();
	void stopSound();
	void startTimer();
	void killTimer();

protected:
	BoltEngine *_bolt;
};

} // End of namespace Bolt

#endif // XPLIB_XPLIB_H
