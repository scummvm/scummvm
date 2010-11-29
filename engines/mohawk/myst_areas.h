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
 * $URL$
 * $Id$
 *
 */

#include "mohawk/myst.h"

#ifndef MYST_AREAS_H
#define MYST_AREAS_H

namespace Mohawk {


class MystResource {
public:
	MystResource(MohawkEngine_Myst *vm, Common::SeekableReadStream *rlstStream, MystResource *parent);
	virtual ~MystResource();

	MystResource *_parent;
	uint16 type;

	bool contains(Common::Point point) { return _rect.contains(point); }
	virtual void drawDataToScreen() {}
	virtual void drawConditionalDataToScreen(uint16 state) {}
	virtual void handleAnimation() {}
	virtual Common::Rect getRect() { return _rect; }
	bool isEnabled() { return _enabled; }
	void setEnabled(bool enabled) { _enabled = enabled; }
	uint16 getDest() { return _dest; }
	virtual uint16 getType8Var() { return 0xFFFF; }

	// Mouse interface
	virtual void handleMouseUp();
	virtual void handleMouseDown() {}
	virtual void handleMouseMove() {}
	virtual void handleMouseEnter() {}
	virtual void handleMouseLeave() {}

protected:
	MohawkEngine_Myst *_vm;

	uint16 _flags;
	Common::Rect _rect;
	uint16 _dest;
	bool _enabled;
};

class MystResourceType5 : public MystResource {
public:
	MystResourceType5(MohawkEngine_Myst *vm, Common::SeekableReadStream *rlstStream, MystResource *parent);
	void handleMouseUp();

protected:
	MystScript _script;
};

class MystResourceType6 : public MystResourceType5 {
public:
	MystResourceType6(MohawkEngine_Myst *vm, Common::SeekableReadStream *rlstStream, MystResource *parent);
	void handleAnimation();

protected:
	static Common::String convertMystVideoName(Common::String name);
	Common::String _videoFile;
	uint16 _left;
	uint16 _top;
	uint16 _loop;
	uint16 _u0;
	uint16 _playBlocking;
	uint16 _playOnCardChange;
	uint16 _u3;

private:
	bool _videoRunning;
};

struct MystResourceType7 : public MystResource {
public:
	MystResourceType7(MohawkEngine_Myst *vm, Common::SeekableReadStream *rlstStream, MystResource *parent);
	virtual ~MystResourceType7();

	virtual void drawDataToScreen();
	virtual void handleAnimation();

	virtual void handleMouseUp();
	virtual void handleMouseDown();
	virtual void handleMouseEnter();
	virtual void handleMouseLeave();

protected:
	uint16 _var7;
	uint16 _numSubResources;
	Common::Array<MystResource*> _subResources;
};

class MystResourceType8 : public MystResourceType7 {
public:
	MystResourceType8(MohawkEngine_Myst *vm, Common::SeekableReadStream *rlstStream, MystResource *parent);
	virtual ~MystResourceType8();
	void drawDataToScreen();
	void drawConditionalDataToScreen(uint16 state);
	uint16 getType8Var();

	struct SubImage {
		uint16 wdib;
		Common::Rect rect;
	} *_subImages;

protected:
	uint16 _var8;
	uint16 _numSubImages;
};

// No MystResourceType9!

class MystResourceType10 : public MystResourceType8 {
public:
	MystResourceType10(MohawkEngine_Myst *vm, Common::SeekableReadStream *rlstStream, MystResource *parent);
	virtual ~MystResourceType10();
	void handleMouseDown();
	void handleMouseUp();
	void handleMouseMove();

protected:
	uint16 _kind;
	Common::Rect _rect10;
	uint16 _u0;
	uint16 _u1;
	uint16 _mouseDownOpcode;
	uint16 _mouseDragOpcode;
	uint16 _mouseUpOpcode;
	struct {
		uint16 listCount;
		uint16 *list;
	} _lists[4];

	bool _mouseDown;
};

class MystResourceType11 : public MystResourceType8 {
public:
	MystResourceType11(MohawkEngine_Myst *vm, Common::SeekableReadStream *rlstStream, MystResource *parent);
	virtual ~MystResourceType11();
	void handleMouseDown();
	void handleMouseUp();
	void handleMouseMove();

protected:
	uint16 _kind;
	Common::Rect _rect11;
	uint16 _u0;
	uint16 _u1;
	uint16 _mouseDownOpcode;
	uint16 _mouseDragOpcode;
	uint16 _mouseUpOpcode;
	struct {
		uint16 listCount;
		uint16 *list;
	} _lists[3];

	bool _mouseDown;
};

class MystResourceType12 : public MystResourceType8 {
public:
	MystResourceType12(MohawkEngine_Myst *vm, Common::SeekableReadStream *rlstStream, MystResource *parent);
	virtual ~MystResourceType12();
	void handleAnimation();
	void handleMouseUp();

protected:
	uint16 _kind;
	Common::Rect _rect11;
	uint16 _state0Frame;
	uint16 _state1Frame;
	uint16 _mouseDownOpcode;
	uint16 _mouseDragOpcode;
	uint16 _mouseUpOpcode;
	struct {
		uint16 listCount;
		uint16 *list;
	} _lists[3];

	uint16 _numFrames;
	uint16 _firstFrame;
	Common::Rect _frameRect;

private:
	bool _doAnimation;
	uint16 _currentFrame;
};

class MystResourceType13 : public MystResource {
public:
	MystResourceType13(MohawkEngine_Myst *vm, Common::SeekableReadStream *rlstStream, MystResource *parent);
	void handleMouseUp();
	void handleMouseEnter();
	void handleMouseLeave();

protected:
	uint16 _enterOpcode;
	uint16 _leaveOpcode;
};

} // End of namespace Mohawk

#endif
