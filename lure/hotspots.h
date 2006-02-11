/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef __lure_hotspots_h__
#define __lure_hotspots_h__

#include "lure/luredefs.h"
#include "lure/screen.h"
#include "lure/disk.h"
#include "lure/res_struct.h"

namespace Lure {

class Hotspot;

typedef void(*HandlerMethodPtr)(Hotspot &h);

class HotspotTickHandlers {
private:
	static void defaultHandler(Hotspot &h);
	static void standardAnimHandler(Hotspot &h);
	static void roomExitAnimHandler(Hotspot &h);
	static void playerAnimHandler(Hotspot &h);
	static void droppingTorchAnimHandler(Hotspot &h);
	static void fireAnimHandler(Hotspot &h);
	static void headAnimationHandler(Hotspot &h);

public:
	static HandlerMethodPtr getHandler(uint16 procOffset);
};


class Hotspot {
private:
	HotspotData *_data;
	HotspotAnimData *_anim;
	HandlerMethodPtr _tickHandler;
	Surface *_frames;
	int16 _startX, _startY;
	uint16 _height, _width;
	uint16 _numFrames;
	uint16 _frameNumber;
	uint16 _tickCtr;
	bool _persistant;

	int16 _destX, _destY;
	uint16 _destHotspotId;
public:
	Hotspot(HotspotData *res);
	~Hotspot();

	void setAnimation(uint16 newAnimId);
	void setAnimation(HotspotAnimData *newRecord);
	uint16 hotspotId() { return _data->hotspotId; }
	Surface &frames() { return *_frames; }
	HotspotAnimData &anim() { return *_anim; }
	HotspotData &resource() { return *_data; }
	uint16 numFrames() { return _numFrames; }
	uint16 frameNumber() { return _frameNumber; }
	void setFrameNumber(uint16 v) { _frameNumber = v; }
	void incFrameNumber();
	uint16 frameWidth() { return _width; }
	int16 x() { return _startX; }
	int16 y() { return _startY; }
	int16 destX() { return _destX; }
	int16 destY() { return _destY; }
	uint16 destHotspotId() { return _destHotspotId; }
	uint16 width() { return _width; }
	uint16 height() { return _height; }
	uint16 roomNumber() { return _data->roomNumber; }
	uint16 script() { return _data->sequenceOffset; }
	uint8 layer() { return _data->layer; }
	uint16 tickCtr() { return _tickCtr; }
	void setTickCtr(uint16 newVal) { _tickCtr = newVal; }
	void setTickProc(uint16 newVal);
	bool persistant() { return _persistant; }
	void setPersistant(bool value) { _persistant = value; }
	void setRoomNumber(uint16 roomNum) { _data->roomNumber = roomNum; }
	bool isActiveAnimation();
	void setPosition(int16 newX, int16 newY);
	void setDestPosition(int16 newX, int16 newY) { _destX = newX; _destY = newY; }
	void setSize(uint16 newWidth, uint16 newHeight);
	void setScript(uint16 offset) { _data->sequenceOffset = offset; }
	void setActions(uint32 newActions) { _data->actions = newActions; }

	void copyTo(Surface *dest);
	bool executeScript();
	void tick();
	void walkTo(int16 endPosX, int16 endPosY, uint16 destHotspot = 0, bool immediate = false);
	void setDirection(Direction dir);

	// Action set
	void doAction(Action action, HotspotData *hotspot);
	bool isRoomExit(uint16 id);
	void doGet(HotspotData *hotspot);
	void doOperate(HotspotData *hotspot, Action action);
	void doOpen(HotspotData *hotspot);
	void doClose(HotspotData *hotspot);
	void doLockUnlock(HotspotData *hotspot);
	void doUse(HotspotData *hotspot);
	void doGive(HotspotData *hotspot);
	void doTalkTo(HotspotData *hotspot);
	void doTell(HotspotData *hotspot);
	void doLook();
	void doLookAt(HotspotData *hotspot);
	void doAsk(HotspotData *hotspot);
	void doDrink();
	void doStatus();
	void doBribe(HotspotData *hotspot);
	void doExamine();
	void doSimple(HotspotData *hotspot, Action action);
};

typedef ManagedList<Hotspot *> HotspotList;

} // End of namespace Lure

#endif
