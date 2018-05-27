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

#ifndef MYST_AREAS_H
#define MYST_AREAS_H

#include "mohawk/myst.h"
#include "mohawk/myst_scripts.h"
#include "mohawk/video.h"

#include "common/rect.h"

namespace Mohawk {

// Myst Resource Types
enum ResourceType {
	kMystAreaForward = 0,
	kMystAreaLeft = 1,
	kMystAreaRight = 2,
	kMystAreaDown = 3,
	kMystAreaUp = 4,
	kMystAreaAction = 5,
	kMystAreaVideo = 6,
	kMystAreaActionSwitch = 7,
	kMystAreaImageSwitch = 8,
	kMystAreaSlider = 10,
	kMystAreaDrag = 11,
	kMystVideoInfo = 12,
	kMystAreaHover = 13
};

// Myst Resource Flags
// TODO: Figure out other flags
enum {
	kMystSubimageEnableFlag = (1 << 0),
	kMystHotspotEnableFlag  = (1 << 1),
	kMystUnknownFlag        = (1 << 2),
	kMystZipModeEnableFlag  = (1 << 3)
};

class MystArea {
public:
	MystArea(MohawkEngine_Myst *vm, ResourceType type, Common::SeekableReadStream *rlstStream, MystArea *parent);
	virtual ~MystArea();

	virtual const Common::String describe();
	void drawBoundingRect();

	bool hasType(ResourceType type) const { return _type == type; }
	bool contains(Common::Point point) { return _rect.contains(point); }
	virtual void drawDataToScreen() {}
	virtual void handleCardChange() {}
	Common::Rect getRect() { return _rect; }
	void setRect(const Common::Rect &rect) { _rect = rect; }
	bool isEnabled();
	void setEnabled(bool enabled);
	bool isDrawSubimages() { return _flags & kMystSubimageEnableFlag; }
	uint16 getDest() { return _dest; }
	virtual uint16 getImageSwitchVar() { return 0xFFFF; }
	bool unreachableZipDest();
	bool canBecomeActive();

	// Mouse interface
	virtual void handleMouseUp();
	virtual void handleMouseDown() {}
	virtual void handleMouseDrag() {}

	MystArea *_parent;
protected:
	MohawkEngine_Myst *_vm;

	ResourceType _type;
	uint16 _flags;
	Common::Rect _rect;
	uint16 _dest;
};

class MystAreaAction : public MystArea {
public:
	MystAreaAction(MohawkEngine_Myst *vm, ResourceType type, Common::SeekableReadStream *rlstStream, MystArea *parent);

	void handleMouseUp() override;
	const Common::String describe() override;

protected:
	MystScript _script;
};

class MystAreaVideo : public MystAreaAction {
public:
	MystAreaVideo(MohawkEngine_Myst *vm, ResourceType type, Common::SeekableReadStream *rlstStream, MystArea *parent);

	VideoEntryPtr playMovie();
	VideoEntryPtr getVideo();

	void handleCardChange() override;
	bool isPlaying();
	void setDirection(int16 direction) { _direction = direction; }
	void setBlocking(bool blocking) { _playBlocking = blocking; }
	void pauseMovie(bool pause);

protected:
	static Common::String convertMystVideoName(const Common::String &name);

	Common::String _videoFile;
	int16 _left;
	int16 _top;
	uint16 _loop;
	int16 _direction; // 1 => forward, -1 => backwards
	uint16 _playBlocking;
	bool _playOnCardChange;
	uint16 _playRate; // percents
};

class MystAreaActionSwitch : public MystArea {
public:
	MystAreaActionSwitch(MohawkEngine_Myst *vm, ResourceType type, Common::SeekableReadStream *rlstStream, MystArea *parent);
	~MystAreaActionSwitch() override;

	void drawDataToScreen() override;
	void handleCardChange() override;

	void handleMouseUp() override;
	void handleMouseDown() override;

	MystArea *getSubResource(uint16 index) { return _subResources[index]; }
protected:
	typedef void (MystArea::*AreaHandler)();

	void doSwitch(AreaHandler handler);

	uint16 _actionSwitchVar;
	Common::Array<MystArea *> _subResources;
};

class MystAreaImageSwitch : public MystAreaActionSwitch {
public:
	MystAreaImageSwitch(MohawkEngine_Myst *vm, ResourceType type, Common::SeekableReadStream *rlstStream, MystArea *parent);
	~MystAreaImageSwitch() override;

	struct SubImage {
		uint16 wdib;
		Common::Rect rect;
	};

	const Common::String describe() override;
	void drawDataToScreen() override;
	void drawConditionalDataToScreen(uint16 state, bool update = true);
	uint16 getImageSwitchVar() override;

	SubImage getSubImage(uint index) const;
	void setSubImageRect(uint index, const Common::Rect &rect);

protected:
	uint16 _imageSwitchVar;
	Common::Array<SubImage> _subImages;
};

class MystAreaDrag : public MystAreaImageSwitch {
public:
	MystAreaDrag(MohawkEngine_Myst *vm, ResourceType type, Common::SeekableReadStream *rlstStream, MystArea *parent);
	~MystAreaDrag() override;

	const Common::String describe() override;

	void handleMouseDown() override;
	void handleMouseUp() override;
	void handleMouseDrag() override;

	uint16 getList1(uint16 index);
	uint16 getList2(uint16 index);
	uint16 getList3(uint16 index);

	uint16 getStepsH() { return _stepsH; }
	uint16 getStepsV() { return _stepsV; }

	Common::Point _pos;
protected:
	typedef Common::Array<uint16> ValueList;

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
	ValueList _lists[3];
};

class MystAreaSlider : public MystAreaDrag {
public:
	MystAreaSlider(MohawkEngine_Myst *vm, ResourceType type, Common::SeekableReadStream *rlstStream, MystArea *parent);
	~MystAreaSlider() override;

	void handleMouseDown() override;
	void handleMouseUp() override;
	void handleMouseDrag() override;
	void setStep(uint16 step);
	void setPosition(uint16 pos);
    void restoreBackground();

protected:
	Common::Rect boundingBox();
	void updatePosition(const Common::Point &mouse);

	uint16 _dragSound;
	uint16 _sliderWidth;
	uint16 _sliderHeight;
};

class MystVideoInfo : public MystAreaDrag {
public:
	MystVideoInfo(MohawkEngine_Myst *vm, ResourceType type, Common::SeekableReadStream *rlstStream, MystArea *parent);
	~MystVideoInfo() override;

	void drawFrame(uint16 frame);
	bool pullLeverV();
	void releaseLeverV();
	uint16 getNumFrames() { return _numFrames; }

protected:
	uint16 _numFrames;
	uint16 _firstFrame;
	Common::Rect _frameRect;
};

class MystAreaHover : public MystArea {
public:
	MystAreaHover(MohawkEngine_Myst *vm, ResourceType type, Common::SeekableReadStream *rlstStream, MystArea *parent);

	const Common::String describe() override;

	void handleMouseUp() override;
	void handleMouseEnter();
	void handleMouseLeave();

protected:
	uint16 _enterOpcode;
	uint16 _leaveOpcode;
};

} // End of namespace Mohawk

#endif
