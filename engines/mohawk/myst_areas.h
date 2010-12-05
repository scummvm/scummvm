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

// Myst Resource Types
enum ResourceType {
	kMystForwardArea = 0,
	kMystLeftArea = 1,
	kMystRightArea = 2,
	kMystDownArea = 3,
	kMystUpArea = 4,
	kMystAction = 5,
	kMystVideo = 6,
	kMystSwitch = 7,
	kMystConditionalImage = 8,
	kMystSlider = 10,
	kMystDragArea = 11,
	kMystVideoInfo = 12,
	kMystHoverArea = 13
};

// Myst Resource Flags
// TODO: Figure out other flags
enum {
	kMystSubimageEnableFlag = (1 << 0),
	kMystHotspotEnableFlag  = (1 << 1),
	kMystUnknownFlag        = (1 << 2),
	kMystZipModeEnableFlag  = (1 << 3)
};

class MystResource {
public:
	MystResource(MohawkEngine_Myst *vm, Common::SeekableReadStream *rlstStream, MystResource *parent);
	virtual ~MystResource();

	MystResource *_parent;
	ResourceType type;

	bool contains(Common::Point point) { return _rect.contains(point); }
	virtual void drawDataToScreen() {}
	virtual void handleAnimation() {}
	virtual Common::Rect getRect() { return _rect; }
	bool isEnabled();
	void setEnabled(bool enabled);
	bool isDrawSubimages() { return _flags & kMystSubimageEnableFlag; }
	uint16 getDest() { return _dest; }
	virtual uint16 getType8Var() { return 0xFFFF; }
	bool unreachableZipDest();
	bool canBecomeActive();

	// Mouse interface
	virtual void handleMouseUp(const Common::Point &mouse);
	virtual void handleMouseDown(const Common::Point &mouse) {}
	virtual void handleMouseDrag(const Common::Point &mouse) {}
	virtual void handleMouseEnter() {}
	virtual void handleMouseLeave() {}

protected:
	MohawkEngine_Myst *_vm;

	uint16 _flags;
	Common::Rect _rect;
	uint16 _dest;
};

class MystResourceType5 : public MystResource {
public:
	MystResourceType5(MohawkEngine_Myst *vm, Common::SeekableReadStream *rlstStream, MystResource *parent);
	void handleMouseUp(const Common::Point &mouse);

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
	uint16 _direction; // 1 => forward, -1 => backwards
	uint16 _playBlocking;
	uint16 _playOnCardChange;
	uint16 _u3;

private:
	bool _videoRunning;
};

class MystResourceType7 : public MystResource {
public:
	MystResourceType7(MohawkEngine_Myst *vm, Common::SeekableReadStream *rlstStream, MystResource *parent);
	virtual ~MystResourceType7();

	virtual void drawDataToScreen();
	virtual void handleAnimation();

	virtual void handleMouseUp(const Common::Point &mouse);
	virtual void handleMouseDown(const Common::Point &mouse);
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
	virtual void drawDataToScreen();
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

class MystResourceType11 : public MystResourceType8 {
public:
	MystResourceType11(MohawkEngine_Myst *vm, Common::SeekableReadStream *rlstStream, MystResource *parent);
	virtual ~MystResourceType11();
	void handleMouseDown(const Common::Point &mouse);
	void handleMouseUp(const Common::Point &mouse);
	void handleMouseDrag(const Common::Point &mouse);

	uint16 getList1(uint16 index);
	uint16 getList2(uint16 index);
	uint16 getList3(uint16 index);

	uint16 getStepsV() { return _stepsV; }

	Common::Point _pos;
protected:
	void setPositionClipping(const Common::Point &mouse, Common::Point &dest);

	uint16 _flagHV;
	uint16 _minH;
	uint16 _maxH;
	uint16 _minV;
	uint16 _maxV;
	uint16 _stepsH;
	uint16 _stepsV;
	uint16 _stepH;
	uint16 _stepV;
	uint16 _mouseDownOpcode;
	uint16 _mouseDragOpcode;
	uint16 _mouseUpOpcode;
	struct {
		uint16 listCount;
		uint16 *list;
	} _lists[3];

};

class MystResourceType10 : public MystResourceType11 {
public:
	MystResourceType10(MohawkEngine_Myst *vm, Common::SeekableReadStream *rlstStream, MystResource *parent);
	virtual ~MystResourceType10();

	void drawDataToScreen();
	void handleMouseDown(const Common::Point &mousee);
	void handleMouseUp(const Common::Point &mouse);
	void handleMouseDrag(const Common::Point &mouse);
	void setStep(uint16 step);
	void setPosition(uint16 pos);

protected:
	Common::Rect boundingBox();
	void updatePosition(const Common::Point &mouse);
    void restoreBackground();

	uint16 _dragSound;
	uint16 _sliderWidth;
	uint16 _sliderHeigth;
};

class MystResourceType12 : public MystResourceType11 {
public:
	MystResourceType12(MohawkEngine_Myst *vm, Common::SeekableReadStream *rlstStream, MystResource *parent);
	virtual ~MystResourceType12();
	void drawFrame(uint16 frame);

protected:
	uint16 _numFrames;
	uint16 _firstFrame;
	Common::Rect _frameRect;

private:
	uint16 _currentFrame;
};

class MystResourceType13 : public MystResource {
public:
	MystResourceType13(MohawkEngine_Myst *vm, Common::SeekableReadStream *rlstStream, MystResource *parent);
	void handleMouseUp(const Common::Point &mouse);
	void handleMouseEnter();
	void handleMouseLeave();

protected:
	uint16 _enterOpcode;
	uint16 _leaveOpcode;
};

} // End of namespace Mohawk

#endif
