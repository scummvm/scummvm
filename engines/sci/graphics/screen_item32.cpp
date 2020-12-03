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
#include "sci/resource/resource.h"
#include "sci/engine/features.h"
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
uint32 ScreenItem::_nextCreationId = 0;

ScreenItem::ScreenItem(const reg_t object) :
_creationId(_nextCreationId++),
_object(object),
_pictureId(-1),
_created(g_sci->_gfxFrameout->getScreenCount()),
_updated(0),
_deleted(0),
_mirrorX(false),
_drawBlackLines(false) {
	SegManager *segMan = g_sci->getEngineState()->_segMan;

	setFromObject(segMan, object, true, true);
	_plane = readSelector(segMan, object, SELECTOR(plane));
}

ScreenItem::ScreenItem(const reg_t plane, const CelInfo32 &celInfo) :
_creationId(_nextCreationId++),
_plane(plane),
_useInsetRect(false),
_z(0),
_celInfo(celInfo),
_fixedPriority(false),
_position(0, 0),
_object(make_reg(0, _nextObjectId++)),
_pictureId(-1),
_created(g_sci->_gfxFrameout->getScreenCount()),
_updated(0),
_deleted(0),
_mirrorX(false),
_drawBlackLines(false) {}

ScreenItem::ScreenItem(const reg_t plane, const CelInfo32 &celInfo, const Common::Rect &rect) :
_creationId(_nextCreationId++),
_plane(plane),
_useInsetRect(false),
_z(0),
_celInfo(celInfo),
_fixedPriority(false),
_position(rect.left, rect.top),
_object(make_reg(0, _nextObjectId++)),
_pictureId(-1),
_created(g_sci->_gfxFrameout->getScreenCount()),
_updated(0),
_deleted(0),
_mirrorX(false),
_drawBlackLines(false) {
	if (celInfo.type == kCelTypeColor) {
		_insetRect = rect;
	}
}

ScreenItem::ScreenItem(const reg_t plane, const CelInfo32 &celInfo, const Common::Point &position, const ScaleInfo &scaleInfo) :
_creationId(_nextCreationId++),
_plane(plane),
_scale(scaleInfo),
_useInsetRect(false),
_z(0),
_celInfo(celInfo),
_fixedPriority(false),
_position(position),
_object(make_reg(0, _nextObjectId++)),
_pictureId(-1),
_created(g_sci->_gfxFrameout->getScreenCount()),
_updated(0),
_deleted(0),
_mirrorX(false),
_drawBlackLines(false) {}

ScreenItem::ScreenItem(const ScreenItem &other) :
_creationId(other._creationId),
_plane(other._plane),
_scale(other._scale),
_useInsetRect(other._useInsetRect),
_celInfo(other._celInfo),
_object(other._object),
_mirrorX(other._mirrorX),
_scaledPosition(other._scaledPosition),
_screenRect(other._screenRect),
_drawBlackLines(other._drawBlackLines) {
	if (other._useInsetRect) {
		_insetRect = other._insetRect;
	}
}

void ScreenItem::operator=(const ScreenItem &other) {
	// SSCI did not check for differences in `_celInfo` to clear `_celObj` here;
	// instead, it unconditionally set `_celInfo`, didn't clear `_celObj`, and
	// did hacky stuff in `kIsOnMe` to avoid testing a mismatched `_celObj`. See
	// `GfxFrameout::kernelIsOnMe` for more detail.
	//
	// kCelTypeMem types are unconditionally invalidated because the properties
	// of a CelObjMem can "change" when a game deletes a bitmap and then creates
	// a new one that reuses the old bitmap's offset in BitmapTable (as happens
	// in the LSL7 About screen when hovering names).
	if (_celInfo.type == kCelTypeMem || _celInfo != other._celInfo) {
		_celInfo = other._celInfo;
		_celObj.reset();
	}

	_creationId = other._creationId;
	_screenRect = other._screenRect;
	_mirrorX = other._mirrorX;
	_useInsetRect = other._useInsetRect;
	if (other._useInsetRect) {
		_insetRect = other._insetRect;
	}
	_scale = other._scale;
	_scaledPosition = other._scaledPosition;
	_drawBlackLines = other._drawBlackLines;
}

void ScreenItem::init() {
	_nextObjectId = 20000;
	_nextCreationId = 0;
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
			Resource *view = g_sci->getResMan()->findResource(ResourceId(kResourceTypeView, _celInfo.resourceId), false);
			if (!view) {
				error("Failed to load %s", _celInfo.toString().c_str());
			}

			const uint16 headerSize = view->getUint16SEAt(0) + /* header size field */ sizeof(uint16);
			const uint8 loopCount = view->getUint8At(2);
			const uint8 loopSize = view->getUint8At(12);

			// loopNo is set to be an unsigned integer in SSCI, so if it's a
			// negative value, it'll be fixed accordingly
			if ((uint16)_celInfo.loopNo >= loopCount) {
				const int maxLoopNo = loopCount - 1;
				_celInfo.loopNo = maxLoopNo;
				writeSelectorValue(segMan, object, SELECTOR(loop), maxLoopNo);
			}

			SciSpan<const byte> loopData = view->subspan(headerSize + (_celInfo.loopNo * loopSize));
			const int8 seekEntry = loopData[0];
			if (seekEntry != -1) {
				loopData = view->subspan(headerSize + (seekEntry * loopSize));
			}

			// celNo is set to be an unsigned integer in SSCI, so if it's a
			// negative value, it'll be fixed accordingly
			const uint8 celCount = loopData[2];
			if ((uint16)_celInfo.celNo >= celCount) {
				const int maxCelNo = celCount - 1;
				_celInfo.celNo = maxCelNo;
				writeSelectorValue(segMan, object, SELECTOR(cel), maxCelNo);
			}
		}
	}

	const reg_t bitmap = readSelector(segMan, object, SELECTOR(bitmap));
	if (updateBitmap && !bitmap.isNull()) {
		_celInfo.bitmap = bitmap;
		_celInfo.type = kCelTypeMem;
	} else {
		_celInfo.bitmap = NULL_REG;
		_celInfo.type = kCelTypeView;
	}

	if (updateCel || updateBitmap) {
		_celObj.reset();
	}

	if (readSelectorValue(segMan, object, SELECTOR(fixPriority))) {
		_fixedPriority = true;
		_priority = readSelectorValue(segMan, object, SELECTOR(priority));
	} else {
		_fixedPriority = false;
		writeSelectorValue(segMan, object, SELECTOR(priority), _position.y);
	}

	_z = (int16)readSelectorValue(segMan, object, SELECTOR(z));
	_position.y -= _z;

	if (g_sci->_features->usesAlternateSelectors()) {
		if (readSelectorValue(segMan, object, SELECTOR(seenRect))) {
			_useInsetRect = true;
			_insetRect.left = readSelectorValue(segMan, object, SELECTOR(left));
			_insetRect.top = readSelectorValue(segMan, object, SELECTOR(top));
			_insetRect.right = readSelectorValue(segMan, object, SELECTOR(right)) + 1;
			_insetRect.bottom = readSelectorValue(segMan, object, SELECTOR(bottom)) + 1;
		} else {
			_useInsetRect = false;
		}
	} else {
		if (readSelectorValue(segMan, object, SELECTOR(useInsetRect))) {
			_useInsetRect = true;
			_insetRect.left = readSelectorValue(segMan, object, SELECTOR(inLeft));
			_insetRect.top = readSelectorValue(segMan, object, SELECTOR(inTop));
			_insetRect.right = readSelectorValue(segMan, object, SELECTOR(inRight)) + 1;
			_insetRect.bottom = readSelectorValue(segMan, object, SELECTOR(inBottom)) + 1;
		} else {
			_useInsetRect = false;
		}
	}

	segMan->getObject(object)->clearInfoSelectorFlag(kInfoFlagViewVisible);
}

void ScreenItem::calcRects(const Plane &plane) {
	const int16 scriptWidth = g_sci->_gfxFrameout->getScriptWidth();
	const int16 scriptHeight = g_sci->_gfxFrameout->getScriptHeight();
	const int16 screenWidth = g_sci->_gfxFrameout->getScreenWidth();
	const int16 screenHeight = g_sci->_gfxFrameout->getScreenHeight();

	const CelObj &celObj = getCelObj();

	Common::Rect celRect(celObj._width, celObj._height);
	if (_useInsetRect) {
		if (_insetRect.intersects(celRect)) {
			_insetRect.clip(celRect);
		} else {
			_insetRect = Common::Rect();
		}
	} else {
		_insetRect = celRect;
	}

	Ratio scaleX, scaleY;
	if (_scale.signal == kScaleSignalManual) {
		scaleX = Ratio(_scale.x, 128);
		scaleY = Ratio(_scale.y, 128);
	} else if (_scale.signal == kScaleSignalVanishingPoint) {
		int num = _scale.max * (_position.y - plane._vanishingPoint.y) / (scriptWidth - plane._vanishingPoint.y);
		scaleX = Ratio(num, 128);
		scaleY = Ratio(num, 128);
	}

	if (scaleX.getNumerator() && scaleY.getNumerator()) {
		_screenItemRect = _insetRect;

		Ratio celToScreenX;
		Ratio celToScreenY;
		if (getSciVersion() < SCI_VERSION_2_1_LATE) {
			celToScreenX = Ratio(screenWidth, celObj._xResolution);
			celToScreenY = Ratio(screenHeight, celObj._yResolution);
		}

		// Cel may use a coordinate system that is not the same size as the
		// script coordinate system (usually this means high-resolution pictures
		// with low-resolution scripts)
		if (celObj._xResolution != kLowResX || celObj._yResolution != kLowResY) {
			// high resolution coordinates

			if (_useInsetRect) {
				if (getSciVersion() < SCI_VERSION_2_1_LATE) {
					const Ratio scriptToCelX(celObj._xResolution, scriptWidth);
					const Ratio scriptToCelY(celObj._yResolution, scriptHeight);
					mulru(_screenItemRect, scriptToCelX, scriptToCelY, 0);
				}

				if (_screenItemRect.intersects(celRect)) {
					_screenItemRect.clip(celRect);
				} else {
					_screenItemRect = Common::Rect();
				}
			}

			int originX = celObj._origin.x;
			int originY = celObj._origin.y;

			if (_mirrorX != celObj._mirrorX && _celInfo.type != kCelTypePic) {
				originX = celObj._width - celObj._origin.x - 1;
			}

			if (!scaleX.isOne() || !scaleY.isOne()) {
				// Different games use a different cel scaling mode, but the
				// difference isn't consistent across SCI versions; instead,
				// it seems to be related to an update that happened during
				// SCI2.1mid where games started using hi-resolution game
				// scripts
				if (scriptWidth == kLowResX) {
					mulinc(_screenItemRect, scaleX, scaleY);
				} else {
					_screenItemRect.left = (_screenItemRect.left * scaleX).toInt();
					_screenItemRect.top = (_screenItemRect.top * scaleY).toInt();

					if (scaleX.getNumerator() > scaleX.getDenominator()) {
						_screenItemRect.right = (_screenItemRect.right * scaleX).toInt();
					} else {
						_screenItemRect.right = ((_screenItemRect.right - 1) * scaleX).toInt() + 1;
					}

					if (scaleY.getNumerator() > scaleY.getDenominator()) {
						_screenItemRect.bottom = (_screenItemRect.bottom * scaleY).toInt();
					} else {
						_screenItemRect.bottom = ((_screenItemRect.bottom - 1) * scaleY).toInt() + 1;
					}
				}

				originX = (originX * scaleX).toInt();
				originY = (originY * scaleY).toInt();
			}

			mulinc(_screenItemRect, celToScreenX, celToScreenY);
			originX = (originX * celToScreenX).toInt();
			originY = (originY * celToScreenY).toInt();

			const Ratio scriptToScreenX = Ratio(screenWidth, scriptWidth);
			const Ratio scriptToScreenY = Ratio(screenHeight, scriptHeight);

			if (/* TODO: dword_C6288 */ (false) && _celInfo.type == kCelTypePic) {
				_scaledPosition.x = _position.x;
				_scaledPosition.y = _position.y;
			} else {
				_scaledPosition.x = (_position.x * scriptToScreenX).toInt() - originX;
				_scaledPosition.y = (_position.y * scriptToScreenY).toInt() - originY;
			}

			_screenItemRect.translate(_scaledPosition.x, _scaledPosition.y);

			if (_mirrorX != celObj._mirrorX && _celInfo.type == kCelTypePic) {
				Common::Rect temp(_insetRect);

				if (!scaleX.isOne()) {
					mulinc(temp, scaleX, Ratio());
				}

				mulinc(temp, celToScreenX, Ratio());

				CelObjPic *celObjPic = dynamic_cast<CelObjPic *>(_celObj.get());
				if (celObjPic == nullptr) {
					error("Expected a CelObjPic");
				}
				temp.translate((celObjPic->_relativePosition.x * scriptToScreenX).toInt() - originX, 0);

				int deltaX = plane._planeRect.width() - temp.right - temp.left;

				_scaledPosition.x += deltaX;
				_screenItemRect.translate(deltaX, 0);
			}

			_scaledPosition.x += plane._planeRect.left;
			_scaledPosition.y += plane._planeRect.top;
			_screenItemRect.translate(plane._planeRect.left, plane._planeRect.top);

			_ratioX = scaleX * celToScreenX;
			_ratioY = scaleY * celToScreenY;
		} else {
			// low resolution coordinates

			int originX = celObj._origin.x;
			if (_mirrorX != celObj._mirrorX && _celInfo.type != kCelTypePic) {
				originX = celObj._width - celObj._origin.x - 1;
			}

			if (!scaleX.isOne() || !scaleY.isOne()) {
				mulinc(_screenItemRect, scaleX, scaleY);
				// TODO: This was in SSCI, baked into the multiplication. It is
				// not clear why this is the only one that reduces the BR corner
				_screenItemRect.right -= 1;
				_screenItemRect.bottom -= 1;
			}

			_scaledPosition.x = _position.x - (originX * scaleX).toInt();
			_scaledPosition.y = _position.y - (celObj._origin.y * scaleY).toInt();
			_screenItemRect.translate(_scaledPosition.x, _scaledPosition.y);

			if (_mirrorX != celObj._mirrorX && _celInfo.type == kCelTypePic) {
				Common::Rect temp(_insetRect);

				if (!scaleX.isOne()) {
					mulinc(temp, scaleX, Ratio());
					temp.right -= 1;
				}

				CelObjPic *celObjPic = dynamic_cast<CelObjPic *>(_celObj.get());
				if (celObjPic == nullptr) {
					error("Expected a CelObjPic");
				}
				temp.translate(celObjPic->_relativePosition.x - (originX * scaleX).toInt(), celObjPic->_relativePosition.y - (celObj._origin.y * scaleY).toInt());

				int deltaX = plane._gameRect.width() - temp.right - temp.left;

				_scaledPosition.x += deltaX;
				_screenItemRect.translate(deltaX, 0);
			}

			_scaledPosition.x += plane._gameRect.left;
			_scaledPosition.y += plane._gameRect.top;
			_screenItemRect.translate(plane._gameRect.left, plane._gameRect.top);

			if (!celToScreenX.isOne() || !celToScreenY.isOne()) {
				mulru(_scaledPosition, celToScreenX, celToScreenY);
				mulru(_screenItemRect, celToScreenX, celToScreenY, 1);
			}

			_ratioX = scaleX * celToScreenX;
			_ratioY = scaleY * celToScreenY;
		}

		_screenRect = _screenItemRect;

		// PQ4CD creates screen items with invalid rects; SSCI does not care
		// about this, but `Common::Rect::clip` does, so we need to check
		// whether or not the rect is actually valid before clipping and only
		// clip valid rects
		if (_screenRect.intersects(plane._screenRect) && _screenRect.isValidRect()) {
			_screenRect.clip(plane._screenRect);
		} else {
			_screenRect.right = 0;
			_screenRect.bottom = 0;
			_screenRect.left = 0;
			_screenRect.top = 0;
		}

		if (!_fixedPriority) {
			_priority = _z + _position.y;
		}
	} else {
		_screenRect.left = 0;
		_screenRect.top = 0;
		_screenRect.right = 0;
		_screenRect.bottom = 0;
	}
}

CelObj &ScreenItem::getCelObj() const {
	if (!_celObj) {
		switch (_celInfo.type) {
			case kCelTypeView:
				_celObj.reset(new CelObjView(_celInfo.resourceId, _celInfo.loopNo, _celInfo.celNo));
				break;
			case kCelTypePic:
				error("Internal error, pic screen item with no cel.");
				break;
			case kCelTypeMem:
				_celObj.reset(new CelObjMem(_celInfo.bitmap));
				break;
			case kCelTypeColor:
				_celObj.reset(new CelObjColor(_celInfo.color, _insetRect.width(), _insetRect.height()));
				break;
			default:
				break;
		}
	}

	return *_celObj;
}

void ScreenItem::printDebugInfo(Console *con) const {
	const char *name;
	if (_object.isNumber()) {
		name = "-scummvm-";
	} else {
		name = g_sci->getEngineState()->_segMan->getObjectName(_object);
	}

	con->debugPrintf("%04x:%04x (%s), prio %d, ins %u, x %d, y %d, z: %d, scaledX: %d, scaledY: %d flags: %d\n",
		PRINT_REG(_object),
		name,
		_priority,
		_creationId,
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

	con->debugPrintf("    %s\n", _celInfo.toString().c_str());

	if (_celObj) {
		con->debugPrintf("    width %d, height %d, x-resolution %d, y-resolution %d\n",
			_celObj->_width,
			_celObj->_height,
			_celObj->_xResolution,
			_celObj->_yResolution
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

void ScreenItem::update() {
	Plane *plane = g_sci->_gfxFrameout->getPlanes().findByObject(_plane);
	if (plane == nullptr) {
		error("ScreenItem::update: Invalid plane %04x:%04x", PRINT_REG(_plane));
	}

	if (plane->_screenItemList.findByObject(_object) == nullptr) {
		error("ScreenItem::update: %04x:%04x not in plane %04x:%04x", PRINT_REG(_object), PRINT_REG(_plane));
	}

	if (!_created) {
		_updated = g_sci->_gfxFrameout->getScreenCount();
	}
	_deleted = 0;

	_celObj.reset();
}

Common::Rect ScreenItem::getNowSeenRect(const Plane &plane) const {
	CelObj &celObj = getCelObj();

	Common::Rect celObjRect(celObj._width, celObj._height);
	Common::Rect nsRect;

	if (_useInsetRect) {
		if (_insetRect.intersects(celObjRect)) {
			nsRect = _insetRect;
			nsRect.clip(celObjRect);
		} else {
			nsRect = Common::Rect();
		}
	} else {
		nsRect = celObjRect;
	}

	const uint16 scriptWidth = g_sci->_gfxFrameout->getScriptWidth();
	const uint16 scriptHeight = g_sci->_gfxFrameout->getScriptHeight();

	Ratio scaleX, scaleY;
	if (_scale.signal == kScaleSignalManual) {
		scaleX = Ratio(_scale.x, 128);
		scaleY = Ratio(_scale.y, 128);
	} else if (_scale.signal == kScaleSignalVanishingPoint) {
		int num = _scale.max * (_position.y - plane._vanishingPoint.y) / (scriptWidth - plane._vanishingPoint.y);
		scaleX = Ratio(num, 128);
		scaleY = Ratio(num, 128);
	}

	if (scaleX.getNumerator() == 0 || scaleY.getNumerator() == 0) {
		return Common::Rect();
	}

	int16 originX = celObj._origin.x;
	int16 originY = celObj._origin.y;

	if (_mirrorX != celObj._mirrorX && _celInfo.type != kCelTypePic) {
		originX = celObj._width - originX - 1;
	}

	if (celObj._xResolution != kLowResX || celObj._yResolution != kLowResY) {
		// high resolution coordinates

		if (_useInsetRect) {
			if (getSciVersion() < SCI_VERSION_2_1_LATE) {
				const Ratio scriptToCelX(celObj._xResolution, scriptWidth);
				const Ratio scriptToCelY(celObj._yResolution, scriptHeight);
				mulru(nsRect, scriptToCelX, scriptToCelY, 0);
			}

			if (nsRect.intersects(celObjRect)) {
				nsRect.clip(celObjRect);
			} else {
				nsRect = Common::Rect();
			}
		}

		if (!scaleX.isOne() || !scaleY.isOne()) {
			// Different games use a different cel scaling mode, but the
			// difference isn't consistent across SCI versions; instead, it
			// seems to be related to an update that happened during SCI2.1mid
			// where games started using high-resolution game scripts
			if (scriptWidth == kLowResX) {
				mulinc(nsRect, scaleX, scaleY);
				// TODO: This was in SSCI, baked into the multiplication. It is
				// not clear why this is the only one that reduces the BR corner
				nsRect.right -= 1;
				nsRect.bottom -= 1;
			} else {
				nsRect.left = (nsRect.left * scaleX).toInt();
				nsRect.top = (nsRect.top * scaleY).toInt();

				if (scaleX.getNumerator() > scaleX.getDenominator()) {
					nsRect.right = (nsRect.right * scaleX).toInt();
				} else {
					nsRect.right = ((nsRect.right - 1) * scaleX).toInt() + 1;
				}

				if (scaleY.getNumerator() > scaleY.getDenominator()) {
					nsRect.bottom = (nsRect.bottom * scaleY).toInt();
				} else {
					nsRect.bottom = ((nsRect.bottom - 1) * scaleY).toInt() + 1;
				}
			}
		}

		Ratio celToScriptX;
		Ratio celToScriptY;
		if (getSciVersion() < SCI_VERSION_2_1_LATE) {
			celToScriptX = Ratio(scriptWidth, celObj._xResolution);
			celToScriptY = Ratio(scriptHeight, celObj._yResolution);
		}

		originX = (originX * scaleX * celToScriptX).toInt();
		originY = (originY * scaleY * celToScriptY).toInt();

		mulinc(nsRect, celToScriptX, celToScriptY);
		nsRect.translate(_position.x - originX, _position.y - originY);
	} else {
		// low resolution coordinates

		if (!scaleX.isOne() || !scaleY.isOne()) {
			mulinc(nsRect, scaleX, scaleY);
			// TODO: This was in SSCI, baked into the multiplication. It is not
			// clear why this is the only one that reduces the BR corner
			nsRect.right -= 1;
			nsRect.bottom -= 1;
		}

		originX = (originX * scaleX).toInt();
		originY = (originY * scaleY).toInt();
		nsRect.translate(_position.x - originX, _position.y - originY);

		if (_mirrorX != celObj._mirrorX && _celInfo.type != kCelTypePic) {
			nsRect.translate(plane._gameRect.width() - nsRect.width(), 0);
		}
	}

	return nsRect;
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
	if (size() < 2) {
		return;
	}

	for (size_type i = 0; i < size(); ++i) {
		_unsorted[i] = i;
	}

	for (size_type i = size() - 1; i > 0; --i) {
		bool swap = false;

		for (size_type j = 0; j < i; ++j)  {
			value_type &a = operator[](j);
			value_type &b = operator[](j + 1);

			if (a == nullptr || *a > *b) {
				SWAP(a, b);
				SWAP(_unsorted[j], _unsorted[j + 1]);
				swap = true;
			}
		}

		if (!swap) {
			break;
		}
	}
}
void ScreenItemList::unsort() {
	if (size() < 2) {
		return;
	}

	for (size_type i = 0; i < size(); ++i) {
		while (_unsorted[i] != i) {
			SWAP(operator[](_unsorted[i]), operator[](i));
			SWAP(_unsorted[_unsorted[i]], _unsorted[i]);
		}
	}
}

} // End of namespace Sci
