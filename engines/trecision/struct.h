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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef TRECISION_STRUCT_H
#define TRECISION_STRUCT_H

#include "common/stream.h"
#include "common/rect.h"
#include "common/scummsys.h"
#include "common/serializer.h"
#include "graphics/surface.h"
#include "trecision/defines.h"

namespace Trecision {

struct SRoom {
	char  _baseName[4];                     // Room name
	uint16 _bkgAnim;                        // Background animation
	uint16 _object[MAXOBJINROOM];           // Objects in the room
	uint16 _sounds[MAXSOUNDSINROOM];        // Sounds of the room
	uint16 _actions[MAXACTIONINROOM];       // Character actions in the room

	bool hasExtra() { return _flag & kObjFlagExtra; }
	bool isDone() { return _flag & kObjFlagDone; }
	void setExtra(bool on) { if (on) _flag |= kObjFlagExtra; else _flag &= ~kObjFlagExtra; }
	void setDone(bool on) { if (on) _flag |= kObjFlagDone; else _flag &= ~kObjFlagDone; }

	void syncGameStream(Common::Serializer &ser);
	void loadRoom(Common::SeekableReadStreamEndian *stream);

private:
	uint8 _flag; // Room visited or not, extra or not
};

struct SObject {
	Common::Rect _rect;
	Common::Rect _area;
	int8  _position;						// -1 if no position
	uint16 _name;
	uint16 _examine;
	uint16 _action;
	uint8 _goRoom;                          // If direction room num - if person num dialog
	uint8 _nbox;                            // Which 3d box the object is associated with
	uint8 _ninv;                            // ptr inventory
	uint16 _anim;
	
	void readRect(Common::SeekableReadStream *stream);
	void setFlagDone(bool on) { if (on) _flag |= kObjFlagDone; else _flag &= ~kObjFlagDone; }
	void setFlagExamine(bool on) { if (on) _flag |= kObjFlagExamine; else _flag &= ~kObjFlagExamine; }
	void setFlagExtra(bool on) { if (on) _flag |= kObjFlagExtra; else _flag &= ~kObjFlagExtra; }
	void setFlagPerson(bool on) { if (on) _flag |= kObjFlagPerson; else _flag &= ~kObjFlagPerson; }
	void setFlagRoomOut(bool on) { if (on) _flag |= kObjFlagRoomOut; else _flag &= ~kObjFlagRoomOut; }
	void setFlagRoomIn(bool on) { if (on) _flag |= kObjFlagRoomIn; else _flag &= ~kObjFlagRoomIn; }
	void setFlagTake(bool on) { if (on) _flag |= kObjFlagTake; else _flag &= ~kObjFlagTake; }

	bool isFlagDone() { return _flag & kObjFlagDone; }
	bool isFlagExamine() { return _flag & kObjFlagExamine; }
	bool isFlagExtra() { return _flag & kObjFlagExtra; }
	bool isFlagPerson() { return _flag & kObjFlagPerson; }
	bool isFlagRoomIn() { return _flag & kObjFlagRoomIn; }
	bool isFlagRoomOut() { return _flag & kObjFlagRoomOut; }
	bool isFlagTake() { return _flag & kObjFlagTake; }
	bool isFlagUseWith() { return _flag & kObjFlagUseWith; }

	bool isModeHidden() { return _mode & OBJMODE_HIDDEN; }
	bool isModeFull() { return _mode & OBJMODE_FULL; }
	bool isModeMask() { return _mode & OBJMODE_MASK; }
	bool isModeLim() { return _mode & OBJMODE_LIM; }
	bool isModeStatus() { return _mode & OBJMODE_OBJSTATUS; }

	void setModeHidden(bool on) { if (on) _mode |= OBJMODE_HIDDEN; else _mode &= ~OBJMODE_HIDDEN; }
	void setModeFull(bool on) { if (on) _mode |= OBJMODE_FULL; else _mode &= ~OBJMODE_FULL; }
	void setModeMask(bool on) { if (on) _mode |= OBJMODE_MASK; else _mode &= ~OBJMODE_MASK; }
	void setModeLim(bool on) { if (on) _mode |= OBJMODE_LIM; else _mode &= ~OBJMODE_LIM; }
	void setModeStatus(bool on) { if (on) _mode |= OBJMODE_OBJSTATUS; else _mode &= ~OBJMODE_OBJSTATUS; }
	
	void syncGameStream(Common::Serializer &ser);
	void loadObj(Common::SeekableReadStreamEndian *stream);

private:
	uint8 _flag;
	uint8 _mode;
};

struct SInvObject {
	uint16 _name;                            // Object name in the inventory
	uint16 _examine;                         // Sentence if examined
	uint16 _action;
	uint16 _anim;

	void setFlagExtra(bool on) { if (on) _flag |= kObjFlagExtra; else _flag &= ~kObjFlagExtra; }

	bool isFlagExtra() { return _flag & kObjFlagExtra; }
	bool isUseWith() { return _flag & kObjFlagUseWith; }

	void syncGameStream(Common::Serializer &ser);
	void loadObj(Common::SeekableReadStreamEndian *stream);

private:
	uint8 _flag;
};

struct SAtFrame {
	uint8 _type;	   //ATFTEXT, ATFSND, ATFEVENT
	uint8 _area;	   // 0 1 2 3 4
	uint16 _numFrame;
	uint16 _index;
};

// Shifted left by 1 - 4, depending on the subarea
#define SMKANIM_OFF_BASE 16

struct SAnim {
	char _name[14];
	uint16 _flag;		// 1- background 2- icon 3- action 4- active  -  4bits per child
	Common::Rect _area[MAXAREA];
	uint8 _nbox;
	SAtFrame _atFrame[MAXATFRAME];

	/**
	 * Toggle the animation of a subarea
	 * @param area: 1 - 4
	 * @param show: show or hide the animation area
	*/
	void toggleAnimArea(uint8 area, bool show) {
		assert(area >= 1 && area <= 4);
		if (show)
			_flag &= ~(SMKANIM_OFF_BASE << area);
		else
			_flag |= (SMKANIM_OFF_BASE << area);
	}

	/**
	 * Checks if an animation subarea is shown
	 * @param area: 1 - 4
	 * @return true if the subarea is shown 
	*/
	bool isAnimAreaShown(uint8 area) {
		assert(area >= 1 && area <= 4);
		return !(_flag & (SMKANIM_OFF_BASE << area));
	}
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

	void clear();
	void sendFrame(Scheduler *scheduler);
	bool isEmptyEvent() const { return _class == 0 && _event == 0;  }
};

class TrecisionEngine;

struct SDText {
	Common::Rect _rect;
	Common::Rect _subtitleRect;
	uint16 _textColor;
	Common::String _text;
	Common::String _drawTextLines[MAXDTEXTLINES];

	void set(SDText *org);
	void set(Common::Rect rect, Common::Rect subtitleRect, uint16 textCol, const Common::String &text);

	void draw(TrecisionEngine *vm, bool hideLastChar = false, Graphics::Surface *externalSurface = nullptr);
	uint16 calcHeight(TrecisionEngine *vm);
};

struct STexture {
	int16 _dx, _dy, _angle;
	uint8 *_texture;

	void clear();
	void set(int16 x, int16 y, uint8 *buffer);
	bool isActive() { return _active; };

private:
	bool _active;
};

struct SVertex {
	float _x, _y, _z;
	float _nx, _ny, _nz;

	void clear();
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

	void clear();
};

struct SCamera {
	float _ex, _ey, _ez;
	float _e1[3];
	float _e2[3];
	float _e3[3];
	float _fovX, _fovY;

	void clear();
};

} // End of namespace Trecision
#endif
