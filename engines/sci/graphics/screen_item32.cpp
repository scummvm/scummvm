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

#include "sci/console.h"
#include "sci/resource.h"
#include "sci/engine/kernel.h"
#include "sci/engine/selector.h"
#include "sci/engine/state.h"
#include "sci/graphics/celobj32.h"
#include "sci/graphics/frameout.h"
#include "sci/graphics/screen_item32.h"
#include "sci/graphics/view.h"

namespace Sci {
#pragma mark ScreenItem

uint16 ScreenItem::_nextObjectId = 20000;

ScreenItem::ScreenItem(const reg_t object) :
_celObj(nullptr),
_object(object),
_pictureId(-1),
_created(g_sci->_gfxFrameout->getScreenCount()),
_updated(0),
_deleted(0),
_mirrorX(false) {
	SegManager *segMan = g_sci->getEngineState()->_segMan;

	setFromObject(segMan, object, true, true);
	_plane = readSelector(segMan, object, SELECTOR(plane));
}

ScreenItem::ScreenItem(const reg_t plane, const CelInfo32 &celInfo) :
_plane(plane),
_useInsetRect(false),
_z(0),
_celInfo(celInfo),
_celObj(nullptr),
_fixPriority(false),
_position(0, 0),
_object(make_reg(0, _nextObjectId++)),
_pictureId(-1),
_created(g_sci->_gfxFrameout->getScreenCount()),
_updated(0),
_deleted(0),
_mirrorX(false) {}

ScreenItem::ScreenItem(const reg_t plane, const CelInfo32 &celInfo, const Common::Rect &rect) :
_plane(plane),
_useInsetRect(false),
_z(0),
_celInfo(celInfo),
_celObj(nullptr),
_fixPriority(false),
_position(rect.left, rect.top),
_object(make_reg(0, _nextObjectId++)),
_pictureId(-1),
_created(g_sci->_gfxFrameout->getScreenCount()),
_updated(0),
_deleted(0),
_mirrorX(false) {
	if (celInfo.type == kCelTypeColor) {
		_insetRect = rect;
	}
}

ScreenItem::ScreenItem(const reg_t plane, const CelInfo32 &celInfo, const Common::Rect &rect, const ScaleInfo &scaleInfo) :
_plane(plane),
_scale(scaleInfo),
_useInsetRect(false),
_z(0),
_celInfo(celInfo),
_celObj(nullptr),
_fixPriority(false),
_position(rect.left, rect.top),
_object(make_reg(0, _nextObjectId++)),
_pictureId(-1),
_created(g_sci->_gfxFrameout->getScreenCount()),
_updated(0),
_deleted(0),
_mirrorX(false) {}

ScreenItem::ScreenItem(const ScreenItem &other) :
_plane(other._plane),
_scale(other._scale),
_useInsetRect(other._useInsetRect),
_celInfo(other._celInfo),
_celObj(nullptr),
_object(other._object),
_mirrorX(other._mirrorX),
_scaledPosition(other._scaledPosition),
_screenRect(other._screenRect) {
	if (other._useInsetRect) {
		_insetRect = other._insetRect;
	}
}

void ScreenItem::operator=(const ScreenItem &other) {
	_celInfo = other._celInfo;
	_screenRect = other._screenRect;
	_mirrorX = other._mirrorX;
	_useInsetRect = other._useInsetRect;
	if (other._useInsetRect) {
		_insetRect = other._insetRect;
	}
	_scale = other._scale;
	_scaledPosition = other._scaledPosition;
}

void ScreenItem::init() {
	_nextObjectId = 20000;
}

void ScreenItem::setFromObject(SegManager *segMan, const reg_t object, const bool updateCel, const bool updateBitmap) {
	_position.x = readSelectorValue(segMan, object, SELECTOR(x));
	_position.y = readSelectorValue(segMan, object, SELECTOR(y));
	_scale.x = readSelectorValue(segMan, object, SELECTOR(scaleX));
	_scale.y = readSelectorValue(segMan, object, SELECTOR(scaleY));
	_scale.max = readSelectorValue(segMan, object, SELECTOR(maxScale));
	_scale.signal = (ScaleSignals32)(readSelectorValue(segMan, object, SELECTOR(scaleSignal)) & 3);

	if (updateCel) {
		_celInfo.resourceId = (GuiResourceId)readSelectorValue(segMan, object, SELECTOR(view));
		_celInfo.loopNo = readSelectorValue(segMan, object, SELECTOR(loop));
		_celInfo.celNo = readSelectorValue(segMan, object, SELECTOR(cel));

		if (_celInfo.resourceId <= kPlanePic) {
			// TODO: Enhance GfxView or ResourceManager to allow
			// metadata for resources to be retrieved once, from a
			// single location
			Resource *view = g_sci->getResMan()->findResource(ResourceId(kResourceTypeView, _celInfo.resourceId), false);
			if (!view) {
				error("Failed to load resource %d", _celInfo.resourceId);
			}

			// NOTE: +2 because the header size field itself is excluded from
			// the header size in the data
			const uint16 headerSize = READ_SCI11ENDIAN_UINT16(view->data) + 2;
			const uint8 loopCount = view->data[2];
			const uint8 loopSize = view->data[12];

			if (_celInfo.loopNo >= loopCount) {
				const int maxLoopNo = loopCount - 1;
				_celInfo.loopNo = maxLoopNo;
				writeSelectorValue(segMan, object, SELECTOR(loop), maxLoopNo);
			}

			byte *loopData = view->data + headerSize + (_celInfo.loopNo * loopSize);
			const int8 seekEntry = loopData[0];
			if (seekEntry != -1) {
				loopData = view->data + headerSize + (seekEntry * loopSize);
			}
			const uint8 celCount = loopData[2];
			if (_celInfo.celNo >= celCount) {
				const int maxCelNo = celCount - 1;
				_celInfo.celNo = maxCelNo;
				writeSelectorValue(segMan, object, SELECTOR(cel), maxCelNo);
			}
		}
	}

	if (updateBitmap) {
		const reg_t bitmap = readSelector(segMan, object, SELECTOR(bitmap));
		if (!bitmap.isNull()) {
			_celInfo.bitmap = bitmap;
			_celInfo.type = kCelTypeMem;
		} else {
			_celInfo.bitmap = NULL_REG;
			_celInfo.type = kCelTypeView;
		}
	}

	if (updateCel || updateBitmap) {
		delete _celObj;
		_celObj = nullptr;
	}

	if (readSelectorValue(segMan, object, SELECTOR(fixPriority))) {
		_fixPriority = true;
		_priority = readSelectorValue(segMan, object, SELECTOR(priority));
	} else {
		_fixPriority = false;
		writeSelectorValue(segMan, object, SELECTOR(priority), _position.y);
	}

	// TODO: At this point (needs checking), GK1 seems to check for the "visible"
	// selector of a plane object. If the object has such a selector, and it's set
	// to 0, then the object should be hidden.
	//
	// This is needed for the inventory in GK1, and seemed to be used only for that
	// game - the "visible" selector isn't present in any other SCI32 game.
	// Possible disabled and unverified code that checks for this follows. This fixes
	// the inventory in GK1. Verify against disassembly!
#if 0
	if (lookupSelector(segMan, object, SELECTOR(visible), NULL, NULL) != kSelectorNone) {
		if (readSelectorValue(segMan, object, SELECTOR(visible)) == 0) {
			_fixPriority = true;
			_priority = -1;
		}
	}
#endif

	_z = readSelectorValue(segMan, object, SELECTOR(z));
	_position.y -= _z;

	if (readSelectorValue(segMan, object, SELECTOR(useInsetRect))) {
		_useInsetRect = true;
		_insetRect.left = readSelectorValue(segMan, object, SELECTOR(inLeft));
		_insetRect.top = readSelectorValue(segMan, object, SELECTOR(inTop));
		_insetRect.right = readSelectorValue(segMan, object, SELECTOR(inRight)) + 1;
		_insetRect.bottom = readSelectorValue(segMan, object, SELECTOR(inBottom)) + 1;
	} else {
		_useInsetRect = false;
	}

	// TODO: SCI2.1/SQ6 engine clears this flag any time ScreenItem::Update(MemID)
	// or ScreenItem::ScreenItem(MemID) are called, but doing this breaks
	// view cycling because the flag isn't being set again later. There are over
	// 100 places in the engine code where this flag is set, so it is probably
	// a matter of figuring out what all of those calls are that re-set it. For
	// now, since these are the *only* calls that clear this flag, we can just
	// leave it set all the time.
	// segMan->getObject(object)->clearInfoSelectorFlag(kInfoFlagViewVisible);
}

void ScreenItem::calcRects(const Plane &plane) {
	const int16 scriptWidth = g_sci->_gfxFrameout->getCurrentBuffer().scriptWidth;
	const int16 scriptHeight = g_sci->_gfxFrameout->getCurrentBuffer().scriptHeight;
	const int16 screenWidth = g_sci->_gfxFrameout->getCurrentBuffer().screenWidth;
	const int16 screenHeight = g_sci->_gfxFrameout->getCurrentBuffer().screenHeight;

	Common::Rect celRect(_celObj->_width, _celObj->_height);
	if (_useInsetRect) {
		if (_insetRect.intersects(celRect)) {
			_insetRect.clip(celRect);
		} else {
			_insetRect = Common::Rect();
		}
	} else {
		_insetRect = celRect;
	}

	Ratio newRatioX;
	Ratio newRatioY;

	if (_scale.signal & kScaleSignalDoScaling32) {
		if (_scale.signal & kScaleSignalUseVanishingPoint) {
			int num = _scale.max * (_position.y - plane._vanishingPoint.y) / (scriptWidth - plane._vanishingPoint.y);
			newRatioX = Ratio(num, 128);
			newRatioY = Ratio(num, 128);
		} else {
			newRatioX = Ratio(_scale.x, 128);
			newRatioY = Ratio(_scale.y, 128);
		}
	}

	if (newRatioX.getNumerator() && newRatioY.getNumerator()) {
		_screenItemRect = _insetRect;

		if (_celObj->_scaledWidth != scriptWidth || _celObj->_scaledHeight != scriptHeight) {
			if (_useInsetRect) {
				Ratio celScriptXRatio(_celObj->_scaledWidth, scriptWidth);
				Ratio celScriptYRatio(_celObj->_scaledHeight, scriptHeight);
				mulru(_screenItemRect, celScriptXRatio, celScriptYRatio);

				if (_screenItemRect.intersects(celRect)) {
					_screenItemRect.clip(celRect);
				} else {
					_screenItemRect = Common::Rect();
				}
			}

			int displaceX = _celObj->_displace.x;
			int displaceY = _celObj->_displace.y;

			if (_mirrorX != _celObj->_mirrorX && _celInfo.type != kCelTypePic) {
				displaceX = _celObj->_width - _celObj->_displace.x - 1;
			}

			if (!newRatioX.isOne() || !newRatioY.isOne()) {
				mulru(_screenItemRect, newRatioX, newRatioY);
				displaceX = (displaceX * newRatioX).toInt();
				displaceY = (displaceY * newRatioY).toInt();
			}

			Ratio celXRatio(screenWidth, _celObj->_scaledWidth);
			Ratio celYRatio(screenHeight, _celObj->_scaledHeight);

			displaceX = (displaceX * celXRatio).toInt();
			displaceY = (displaceY * celYRatio).toInt();

			mulru(_screenItemRect, celXRatio, celYRatio);

			if (/* TODO: dword_C6288 */ false && _celInfo.type == kCelTypePic) {
				_scaledPosition.x = _position.x;
				_scaledPosition.y = _position.y;
			} else {
				_scaledPosition.x = (_position.x * screenWidth / scriptWidth) - displaceX;
				_scaledPosition.y = (_position.y * screenHeight / scriptHeight) - displaceY;
			}

			_screenItemRect.translate(_scaledPosition.x, _scaledPosition.y);

			if (_mirrorX != _celObj->_mirrorX && _celInfo.type == kCelTypePic) {
				Common::Rect temp(_insetRect);

				if (!newRatioX.isOne()) {
					mulru(temp, newRatioX, Ratio());
				}

				mulru(temp, celXRatio, Ratio());

				CelObjPic *celObjPic = dynamic_cast<CelObjPic *>(_celObj);

				temp.translate(celObjPic->_relativePosition.x * screenWidth / scriptWidth - displaceX, 0);

				// TODO: This is weird, and probably wrong calculation of widths
				// due to BR-inclusion
				int deltaX = plane._planeRect.right - plane._planeRect.left + 1 - temp.right - 1 - temp.left;

				_scaledPosition.x += deltaX;
				_screenItemRect.translate(deltaX, 0);
			}

			_scaledPosition.x += plane._planeRect.left;
			_scaledPosition.y += plane._planeRect.top;
			_screenItemRect.translate(plane._planeRect.left, plane._planeRect.top);

			_ratioX = newRatioX * Ratio(screenWidth, _celObj->_scaledWidth);
			_ratioY = newRatioY * Ratio(screenHeight, _celObj->_scaledHeight);
		} else {
			int displaceX = _celObj->_displace.x;
			if (_mirrorX != _celObj->_mirrorX && _celInfo.type != kCelTypePic) {
				displaceX = _celObj->_width - _celObj->_displace.x - 1;
			}

			if (!newRatioX.isOne() || !newRatioY.isOne()) {
				mulru(_screenItemRect, newRatioX, newRatioY);
				// TODO: This was in the original code, baked into the
				// multiplication though it is  not immediately clear
				// why this is the only one that reduces the BR corner
				_screenItemRect.right -= 1;
				_screenItemRect.bottom -= 1;
			}

			_scaledPosition.x = _position.x - (displaceX * newRatioX).toInt();
			_scaledPosition.y = _position.y - (_celObj->_displace.y * newRatioY).toInt();
			_screenItemRect.translate(_scaledPosition.x, _scaledPosition.y);

			if (_mirrorX != _celObj->_mirrorX && _celInfo.type == kCelTypePic) {
				Common::Rect temp(_insetRect);

				if (!newRatioX.isOne()) {
					mulru(temp, newRatioX, Ratio());
					temp.right -= 1;
				}

				CelObjPic *celObjPic = dynamic_cast<CelObjPic *>(_celObj);
				temp.translate(celObjPic->_relativePosition.x - (displaceX * newRatioX).toInt(), celObjPic->_relativePosition.y - (_celObj->_displace.y * newRatioY).toInt());

				// TODO: This is weird, and probably wrong calculation of widths
				// due to BR-inclusion
				int deltaX = plane._gameRect.right - plane._gameRect.left + 1 - temp.right - 1 - temp.left;

				_scaledPosition.x += deltaX;
				_screenItemRect.translate(deltaX, 0);
			}

			_scaledPosition.x += plane._gameRect.left;
			_scaledPosition.y += plane._gameRect.top;
			_screenItemRect.translate(plane._gameRect.left, plane._gameRect.top);

			if (screenWidth != _celObj->_scaledWidth || _celObj->_scaledHeight != screenHeight) {
				Ratio celXRatio(screenWidth, _celObj->_scaledWidth);
				Ratio celYRatio(screenHeight, _celObj->_scaledHeight);
				mulru(_scaledPosition, celXRatio, celYRatio);
				mulru(_screenItemRect, celXRatio, celYRatio);
			}

			_ratioX = newRatioX * Ratio(screenWidth, _celObj->_scaledWidth);
			_ratioY = newRatioY * Ratio(screenHeight, _celObj->_scaledHeight);
		}

		_screenRect = _screenItemRect;

		if (_screenRect.intersects(plane._screenRect)) {
			_screenRect.clip(plane._screenRect);
		} else {
			_screenRect.right = 0;
			_screenRect.bottom = 0;
			_screenRect.left = 0;
			_screenRect.top = 0;
		}

		if (!_fixPriority) {
			_priority = _z + _position.y;
		}
	} else {
		_screenRect.left = 0;
		_screenRect.top = 0;
		_screenRect.right = 0;
		_screenRect.bottom = 0;
	}
}

CelObj &ScreenItem::getCelObj() {
	if (_celObj == nullptr) {
		switch (_celInfo.type) {
			case kCelTypeView:
				_celObj = new CelObjView(_celInfo.resourceId, _celInfo.loopNo, _celInfo.celNo);
			break;
			case kCelTypePic:
				error("Internal error, pic screen item with no cel.");
			break;
			case kCelTypeMem:
				_celObj = new CelObjMem(_celInfo.bitmap);
			break;
			case kCelTypeColor:
				_celObj = new CelObjColor(_celInfo.color, _insetRect.width(), _insetRect.height());
			break;
		}
	}

	return *_celObj;
}

void ScreenItem::printDebugInfo(Console *con) const {
	con->debugPrintf("%x:%x (%s), prio %d, x %d, y %d, z: %d, scaledX: %d, scaledY: %d flags: %d\n",
		_object.getSegment(), _object.getOffset(),
		g_sci->getEngineState()->_segMan->getObjectName(_object),
		_priority,
		_position.x,
		_position.y,
		_z,
		_scaledPosition.x,
		_scaledPosition.y,
		_created | (_updated << 1) | (_deleted << 2)
	);
	con->debugPrintf("    screen rect (%d, %d, %d, %d)\n", PRINT_RECT(_screenRect));
	if (_useInsetRect) {
		con->debugPrintf("    inset rect: (%d, %d, %d, %d)\n", PRINT_RECT(_insetRect));
	}

	Common::String celType;
	switch (_celInfo.type) {
		case kCelTypePic:
			celType = "pic";
			break;
		case kCelTypeView:
			celType = "view";
			break;
		case kCelTypeColor:
			celType = "color";
			break;
		case kCelTypeMem:
			celType = "mem";
			break;
	}

	con->debugPrintf("    type: %s, res %d, loop %d, cel %d, bitmap %04x:%04x, color: %d\n",
		celType.c_str(),
		_celInfo.resourceId,
		_celInfo.loopNo,
		_celInfo.celNo,
		PRINT_REG(_celInfo.bitmap),
		_celInfo.color
	);
	if (_celObj != nullptr) {
		con->debugPrintf("    width %d, height %d, scaledWidth %d, scaledHeight %d\n",
			_celObj->_width,
			_celObj->_height,
			_celObj->_scaledWidth,
			_celObj->_scaledHeight
		);
	}
}

void ScreenItem::update(const reg_t object) {
	SegManager *segMan = g_sci->getEngineState()->_segMan;

	const GuiResourceId view = readSelectorValue(segMan, object, SELECTOR(view));
	const int16 loopNo = readSelectorValue(segMan, object, SELECTOR(loop));
	const int16 celNo = readSelectorValue(segMan, object, SELECTOR(cel));

	const bool updateCel = (
		_celInfo.resourceId != view ||
		_celInfo.loopNo != loopNo ||
		_celInfo.celNo != celNo
	);

	const bool updateBitmap = !readSelector(segMan, object, SELECTOR(bitmap)).isNull();

	setFromObject(segMan, object, updateCel, updateBitmap);

	if (!_created) {
		_updated = g_sci->_gfxFrameout->getScreenCount();
	}

	_deleted = 0;
}

#pragma mark -
#pragma mark ScreenItemList
ScreenItem *ScreenItemList::findByObject(const reg_t object) const {
	const_iterator screenItemIt = Common::find_if(begin(), end(), FindByObject<ScreenItem *>(object));

	if (screenItemIt == end()) {
		return nullptr;
	}

	return *screenItemIt;
}
void ScreenItemList::sort() {
	// TODO: SCI engine used _unsorted as an array of indexes into the
	// list itself and then performed the same swap operations on the
	// _unsorted array as the _storage array during sorting, but the
	// only reason to do this would be if some of the pointers in the
	// list were replaced so the pointer values themselves couldn’t
	// simply be recorded and then restored later. It is not yet
	// verified whether this simplification of the sort/unsort is
	// safe.
	for (size_type i = 0; i < size(); ++i) {
		_unsorted[i] = (*this)[i];
	}

	Common::sort(begin(), end(), sortHelper);
}
void ScreenItemList::unsort() {
	for (size_type i = 0; i < size(); ++i) {
		(*this)[i] = _unsorted[i];
	}
}

}
