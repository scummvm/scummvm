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

#ifndef TRECISION_STRUCT_H
#define TRECISION_STRUCT_H

#include "common/rect.h"
#include "common/scummsys.h"
#include "common/stream.h"
#include "graphics/surface.h"
#include "trecision/defines.h"

namespace Trecision {

struct SRoom {
	char  _baseName[4];                     // Room name
	uint8 _flag;							// Room visited or not
	uint16 _bkgAnim;                        // Background animation
	uint16 _object[MAXOBJINROOM];           // Objects in the room
	uint16 _sounds[MAXSOUNDSINROOM];        // Sounds of the room
	uint16 _actions[MAXACTIONINROOM];       // Character actions in the room
};

struct SObject {
	Common::Rect _rect;
	Common::Rect _lim;
	int8  _position;						// -1 if no position
	uint16 _name;
	uint16 _examine;
	uint16 _action;
	uint8 _goRoom;                          // If direction room num - if person num dialog
	uint8 _nbox;                            // Which 3d box the object is associated with
	uint8 _ninv;                            // ptr inventory
	uint8 _mode;                              /* 0- full
												1- mask
												2- limits
												3- objStatus
												6- center when click !
												7- inventory referenceable */
	uint8 _flag;							/*  Examine = 0
												Direction = 1
												Person = 2
												Carried = 3
												2- Take
												3- Open
												4- Close
												5- Use
												6- Extra
												7- Operated     */
	uint16 _anim;

	void readRect(Common::SeekableReadStream *stream) {
		_rect.left = stream->readUint16LE();
		_rect.top = stream->readUint16LE();
		_rect.setWidth(stream->readUint16LE());
		_rect.setHeight(stream->readUint16LE());
	}
};

struct SInvObject {
	uint16 _name;                            // Object name in the inventory
	uint16 _examine;                         // Sentence if examined
	uint16 _action;
	uint8 _flag;                              /* 0-
												1-
												2-
												3- Open
												4- Close
												5- Use      (SET=use OFF & notSET open and close = use with)
												6- Extra
												7- Operated  */
	uint16 _anim;
};

struct SAtFrame {
	uint8 _type;	   //ATFTEXT, ATFSND, ATFEVENT
	uint8 _child;	   // 0 1 2 3 4
	uint16 _numFrame;
	uint16 _index;
};

struct SAnim {
	char _name[14];
	uint16 _flag;		// 1- background 2- icon 3- action 4- active  -  4bits per child
	Common::Rect _lim[MAXCHILD];
	uint8 _nbox;
	SAtFrame _atFrame[MAXATFRAME];
};

struct SSortTable {
	uint16 _objectId;                        // Object ID
	bool  _remove;                           // Whether to copy or remove
};

class Scheduler;

struct SScriptFrame {
	uint8 _class;
	uint8 _event;

	uint8 _u8Param;

	uint16 _u16Param1;
	uint16 _u16Param2;

	uint16 _u32Param;

	bool  _noWait;

	void sendFrame(Scheduler *scheduler);
	bool isEmptyEvent() const { return _class == 0 && _event == 0;  }
};

struct SScript {
	uint16 _firstFrame;
	uint8 _flag;         // 0 - BREAKABLE - If it may be stopped by a mouse click
};                       // 1 - DIALOGEXITNOANIM - If the script is launched from a dialogue at the last choice it exits the dialogue without link anim and by default

struct StackText {
	uint16 x;
	uint16 y;
	uint16 tcol, scol;
	char text[256];
	bool clear;
};

class TrecisionEngine;

struct SDText {
	Common::Rect _rect;
	Common::Rect _subtitleRect;
	uint16 tcol;
	uint16 scol;
	Common::String text;
	char _drawTextLines[MAXDTEXTLINES][MAXDTEXTCHARS];

	void set(SDText org);
	void set(Common::Rect rect, Common::Rect subtitleRect, uint16 tcol, uint16 scol, const Common::String &pText);

	void draw(TrecisionEngine *vm, Graphics::Surface *externalSurface = nullptr);
	uint16 calcHeight(TrecisionEngine *vm);
};

struct ElevatorAction {
	uint16 dialog;
	uint16 choice;
	uint16 action;
	uint16 newRoom;
};

struct Message {
	uint8 _class;    // message class
	uint8 _event;    // message name
	uint8 _priority; // message priority

	uint8 _u8Param;
	uint16 _u16Param1; // byte parameter 1
	uint16 _u16Param2; // byte parameter 2
	uint32 _u32Param;  // int parameter
	uint32 _timestamp;

	void set(Message *src) {
		_class = src->_class;
		_event = src->_event;
		_priority = src->_priority;
		_u8Param = src->_u8Param;
		_u16Param1 = src->_u16Param1;
		_u16Param2 = src->_u16Param2;
		_u32Param = src->_u32Param;
		_timestamp = src->_timestamp;
	}
};

struct MessageQueue {
	uint8 _head, _tail, _len;
	Message *_event[MAXMESSAGE];

	void orderEvents();
	bool testEmptyQueue(uint8 cls);
	bool testEmptyCharacterQueue4Script();
	bool getMessage(Message **msg);
	void initQueue();

private:
	uint8 predEvent(uint8 i);
};

struct STexture {
	int16 _dx, _dy, _angle;
	uint8 *_texture;
	uint8 _flag;
};

struct SVertex {
	float _x, _y, _z;
	float _nx, _ny, _nz;

	void clear() {
		_x = _y = _z = 0.0f;
		_nx = _ny = _nz = 0.0f;
	}
};

struct SFace {
	uint16 _a, _b, _c;
	uint16 _mat;
};

struct SLight {
	float _x, _y, _z;
	float _dx, _dy, _dz;
	float _inr, _outr;
	uint8 _hotspot;
	uint8 _fallOff;
	int8 _inten;
	int8 _position;

	void clear() {
		_x = _y = _z = 0.0f;
		_dx = _dy = _dz = 0.0f;
		_inr = _outr = 0.0f;
		_hotspot = 0;
		_fallOff = 0;
		_inten = 0;
		_position = 0;
	}
};

struct SCamera {
	float _ex, _ey, _ez;
	float _e1[3];
	float _e2[3];
	float _e3[3];
	float _fovX, _fovY;

	void clear() {
		_ex = _ey = _ez = 0.0f;
		_fovX = _fovY = 0.0f;
		for (uint8 i = 0; i < 3; ++i)
			_e1[i] = _e2[i] = _e3[i] = 0.0f;
	}
};

} // End of namespace Trecision
#endif
