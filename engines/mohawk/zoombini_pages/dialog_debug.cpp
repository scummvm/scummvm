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

#include "mohawk/console.h"
#include "mohawk/cursors.h"
#include "mohawk/mohawk.h"
#include "mohawk/sound.h"
#include "mohawk/video.h"

#include "mohawk/zoombini.h"
#include "mohawk/zoombini_graphics.h"
#include "mohawk/zoombini_pages/dialog_debug.h"
#include "mohawk/zoombini_random.h"
#include "mohawk/zoombini_sound.h"
#include "mohawk/zoombini_state.h"

namespace Mohawk {

ZoombiniDialogDebug::ZoombiniDialogDebug(MohawkEngine_Zoombini *vm, const ZoombiniDebugCommand &cmd) : ZoombiniDialog(vm, ZoombiniPageType::kDialogDebug),
																									   _cmd(cmd) {
}

ZoombiniDialogDebug::~ZoombiniDialogDebug() {
}

void ZoombiniDialogDebug::setBackgroundBitmap() {
	if (_cmd._type == ZoombiniDebugCommand::kDrawTerrain) {
		ZoombiniPage *activePage = _vm->getActivePage();
		if (!activePage || !activePage->applyTerrainMaskToShapeScreen()) {
			_vm->_gfx->fillArea(ZoombiniGraphics::kBackScreen, ZoombiniGraphics::kColor2D_Black);
			_vm->_gfx->fillArea(ZoombiniGraphics::kShapeScreen, ZoombiniGraphics::kColor2D_Black);
		} else {
			_vm->_gfx->copyToScreen(ZoombiniGraphics::kBackScreen, *_vm->_gfx->getShapeScreen());
		}
		scheduleForceRedraw();
		return;
	}

	_vm->_gfx->fillArea(ZoombiniGraphics::kBackScreen, ZoombiniGraphics::kColor0A_White);
	_vm->_gfx->fillArea(ZoombiniGraphics::kShapeScreen, ZoombiniGraphics::kColor0A_White);
	scheduleForceRedraw();
}

void ZoombiniDialogDebug::loadFeatures() {
	switch (_cmd._type) {
	case ZoombiniDebugCommand::kDrawCursor: { // [*] Virtual Feature - draw cursor
		ZmbFeature::EventHooks hooksDrawCursor;
		hooksDrawCursor.setRenderFunc(static_cast<ZmbFeature::OnRenderFunc>(&ZoombiniDialogDebug::drawCursor_render));

		loadScrbFeature(ZmbResource(ZmbResource::kPage, 0), 0, 0, ZmbFeature::FLAG_04000000_OVERLAY | ZmbFeature::FLAG_00001000_TOPMOST, hooksDrawCursor);
		break;
	}
	case ZoombiniDebugCommand::kDrawImage: { // [*] Callback-only runner - draw image
		ZmbFeature::EventHooks hooksDrawImage;
		hooksDrawImage.setRenderFunc(static_cast<ZmbFeature::OnRenderFunc>(&ZoombiniDialogDebug::drawImage_render));
		hooksDrawImage.setKeyDownFunc(static_cast<ZmbFeature::OnKeyDownFunc>(&ZoombiniDialogDebug::common_onKeyDown));

		loadScrbFeature(ZmbResource(ZmbResource::kPage, 0), 0, 0, ZmbFeature::FLAG_04000000_OVERLAY | ZmbFeature::FLAG_00001000_TOPMOST, hooksDrawImage);
		break;
	}
	case ZoombiniDebugCommand::kDrawShape: { // [*] Callback-only runner - draw shape
		ZmbFeature::EventHooks hooksDrawShape;
		hooksDrawShape.setRenderFunc(static_cast<ZmbFeature::OnRenderFunc>(&ZoombiniDialogDebug::drawShape_render));
		hooksDrawShape.setKeyDownFunc(static_cast<ZmbFeature::OnKeyDownFunc>(&ZoombiniDialogDebug::common_onKeyDown));

		loadScrbFeature(ZmbResource(ZmbResource::kPage, 0), 0, 0, ZmbFeature::FLAG_04000000_OVERLAY | ZmbFeature::FLAG_00001000_TOPMOST, hooksDrawShape);
		break;
	}
	case ZoombiniDebugCommand::kDrawShapes: { // [*] Callback-only runner - draw shapes
		ZmbFeature::EventHooks hooksDrawShapes;
		hooksDrawShapes.setRenderFunc(static_cast<ZmbFeature::OnRenderFunc>(&ZoombiniDialogDebug::drawShapes_render));
		hooksDrawShapes.setKeyDownFunc(static_cast<ZmbFeature::OnKeyDownFunc>(&ZoombiniDialogDebug::drawShapes_onKeyDown));

		loadScrbFeature(ZmbResource(ZmbResource::kPage, 0), 0, 0, ZmbFeature::FLAG_04000000_OVERLAY | ZmbFeature::FLAG_00001000_TOPMOST, hooksDrawShapes);

		if (1 < _cmd._shapeIdx)
			_drawShapesPrevShapeIdxStack.push(_cmd._shapeIdx);
		break;
	}
	case ZoombiniDebugCommand::kDrawFeature: { // [*] Virtual Feature - draw feature
		ZmbFeature::EventHooks hooksDrawFeature;
		hooksDrawFeature.setRenderFunc(static_cast<ZmbFeature::OnRenderFunc>(&ZoombiniDialogDebug::drawFeature_render));
		hooksDrawFeature.setKeyDownFunc(static_cast<ZmbFeature::OnKeyDownFunc>(&ZoombiniDialogDebug::drawFeature_onKeyDown));
		loadScrbFeature(_cmd._resource, _cmd._scrbId, 0, ZmbFeature::FLAG_04000000_OVERLAY | ZmbFeature::FLAG_00001000_TOPMOST, hooksDrawFeature);
		break;
	}
	case ZoombiniDebugCommand::kDrawTerrain: { // [*] Active page terrain mask
		ZmbFeature::EventHooks hooksDrawTerrain;
		hooksDrawTerrain.setRenderFunc(static_cast<ZmbFeature::OnRenderFunc>(&ZoombiniDialogDebug::drawTerrain_render));
		hooksDrawTerrain.setKeyDownFunc(static_cast<ZmbFeature::OnKeyDownFunc>(&ZoombiniDialogDebug::common_onKeyDown));

		loadScrbFeature(ZmbResource(ZmbResource::kPage, 0), 0, 0, ZmbFeature::FLAG_04000000_OVERLAY | ZmbFeature::FLAG_00001000_TOPMOST, hooksDrawTerrain);
		break;
	}
	case ZoombiniDebugCommand::kPlotPoint: { // [*] Virtual Feature - plot point
		ZmbFeature::EventHooks hooksPlotPoint;
		hooksPlotPoint.setRenderFunc(static_cast<ZmbFeature::OnRenderFunc>(&ZoombiniDialogDebug::plotPoint_render));
		hooksPlotPoint.setKeyDownFunc(static_cast<ZmbFeature::OnKeyDownFunc>(&ZoombiniDialogDebug::common_onKeyDown));

		loadScrbFeature(ZmbResource(ZmbResource::kPage, 0), 0, 0, ZmbFeature::FLAG_04000000_OVERLAY | ZmbFeature::FLAG_00001000_TOPMOST, hooksPlotPoint);
		break;
	}
	case ZoombiniDebugCommand::kPlotLine: { // [*] Callback-only runner - plot line
		ZmbFeature::EventHooks hooksPlotLine;
		hooksPlotLine.setRenderFunc(static_cast<ZmbFeature::OnRenderFunc>(&ZoombiniDialogDebug::plotLine_render));
		hooksPlotLine.setKeyDownFunc(static_cast<ZmbFeature::OnKeyDownFunc>(&ZoombiniDialogDebug::common_onKeyDown));

		loadScrbFeature(ZmbResource(ZmbResource::kPage, 0), 0, 0, ZmbFeature::FLAG_04000000_OVERLAY | ZmbFeature::FLAG_00001000_TOPMOST, hooksPlotLine);
		break;
	}
	case ZoombiniDebugCommand::kPlotRect: { // [*] Callback-only runner - plot rect
		ZmbFeature::EventHooks hooksPlotRect;
		hooksPlotRect.setRenderFunc(static_cast<ZmbFeature::OnRenderFunc>(&ZoombiniDialogDebug::plotRect_render));
		hooksPlotRect.setKeyDownFunc(static_cast<ZmbFeature::OnKeyDownFunc>(&ZoombiniDialogDebug::common_onKeyDown));

		loadScrbFeature(ZmbResource(ZmbResource::kPage, 0), 0, 0, ZmbFeature::FLAG_04000000_OVERLAY | ZmbFeature::FLAG_00001000_TOPMOST, hooksPlotRect);
		break;
	}
	default:
		error("debug: invalid debug command: %u", _cmd._type);
		break;
	}
}

void ZoombiniDialogDebug::close() {
	_vm->_cursor->setDefaultCursor();
	ZoombiniDialog::close();
}

ZmbEventHandleResult ZoombiniDialogDebug::onLButtonDown(const Common::Point &absPos, const Common::Point &relPos) {
	(void)absPos;
	(void)relPos;

	close();
	return ZmbEventHandleResult::kConsumed;
}

ZmbEventHandleResult ZoombiniDialogDebug::common_onKeyDown(ZmbFeature *feature, const Common::KeyState &kbd, bool kbdRepeat) {
	(void)feature;
	(void)kbdRepeat;

	if (classifyDialogKey(kbd) == kDialogKeyNone)
		return ZmbEventHandleResult::kPassthrough;

	close();
	return ZmbEventHandleResult::kConsumed;
}

void ZoombiniDialogDebug::drawTitleText(ZmbFeature *feature, const Common::U32String &titleText) {
	(void)feature;

	ZoombiniGraphics::ScreenKind screenKind = ZoombiniGraphics::kShapeScreen;

	ZoombiniGraphics::TextConf tc;
	tc._fontUsage = ZoombiniFontUsage::kFontDebugTitle;
	tc._wordWrap = false;
	_vm->_gfx->fillArea(screenKind, _titleRect, ZoombiniGraphics::kColor0A_White);
	_vm->_gfx->drawText(screenKind, titleText, _titleRect, tc);
}

void ZoombiniDialogDebug::drawEscText(ZmbFeature *feature, const Common::U32String &keyLegendText) {
	(void)feature;

	ZoombiniGraphics::ScreenKind screenKind = ZoombiniGraphics::kShapeScreen;

	Common::U32String text;
	if (keyLegendText.empty()) {
		text = _escText;
	} else {
		text = keyLegendText;
		if (keyLegendText.lastChar() != U' ')
			text += U" ";
		text.append(_escText);
	}

	ZoombiniGraphics::TextConf tc;
	tc._fontUsage = ZoombiniFontUsage::kFontDebugText;
	tc._hAlign = Graphics::kTextAlignRight;
	tc._fillBackground = true;
	tc._fillBackgroundKey = ZoombiniGraphics::kColor0A_White;
	tc._wordWrap = false;
	_vm->_gfx->drawText(screenKind, text, _titleRect, tc);
}

void ZoombiniDialogDebug::commitDisplayToBackground() {
	// The dirty renderer restores shapeScreen from backScreen before a forced redraw.
	// Preserve the static debug result there so it remains visible until this dialog is explicitly closed.
	_vm->_gfx->copyToScreen(ZoombiniGraphics::kBackScreen, *_vm->_gfx->getScreen(ZoombiniGraphics::kShapeScreen));
}

ZmbRenderResult ZoombiniDialogDebug::drawCursor_render(ZmbFeature *feature) {
	if (!_updateScreen)
		return ZmbRenderResult::kSkipped;
	_updateScreen = false;

	_vm->_cursor->setCursor(_cmd._resource._id);

	drawEscText(feature);
	commitDisplayToBackground();

	return ZmbRenderResult::kRendered;
}

ZmbRenderResult ZoombiniDialogDebug::drawImage_render(ZmbFeature *feature) {
	if (!_updateScreen)
		return ZmbRenderResult::kSkipped;
	_updateScreen = false;

	ZoombiniGraphics::ScreenKind screenKind = ZoombiniGraphics::kShapeScreen;

	// Setup palette if it has one
	if (_vm->hasResource(ID_SHPL, _cmd._resource))
		_vm->_gfx->setPalette(_cmd._resource._id);

	_vm->_gfx->drawBackground(screenKind, _cmd._resource._id);

	drawEscText(feature);
	commitDisplayToBackground();

	return ZmbRenderResult::kRendered;
}

ZmbRenderResult ZoombiniDialogDebug::drawShape_render(ZmbFeature *feature) {
	if (!_updateScreen)
		return ZmbRenderResult::kSkipped;
	_updateScreen = false;

	ZoombiniGraphics::ScreenKind screenKind = ZoombiniGraphics::kShapeScreen;
	_vm->_gfx->clearScreen(screenKind);

	uint32 shapeCount = _vm->_gfx->getShapeCount(_cmd._resource);
	Common::Rect shapeSize = _vm->_gfx->getShapeSize(_cmd._resource, _cmd._shapeIdx);
	shapeSize.moveTo(0, _titleRect.height());

	drawTitleText(feature, Common::String::format("Shape (%03u/%03u) of tBMP %s (w=%u h=%u)", _cmd._shapeIdx, shapeCount, _cmd._resource.toString().c_str(), shapeSize.width(), shapeSize.height()));
	drawEscText(feature);

	_vm->_gfx->fillArea(screenKind, shapeSize, ZoombiniGraphics::kColor0A_White);
	_vm->_gfx->drawShape(screenKind, _cmd._resource, _cmd._shapeIdx, Common::Point(0, _titleRect.height()));
	commitDisplayToBackground();

	return ZmbRenderResult::kRendered;
}

ZmbRenderResult ZoombiniDialogDebug::drawShapes_render(ZmbFeature *feature) {
	// Skip @ref ZoombiniDialogDebug::_updateScreen.
	// Use @ref ZoombiniDialogDebug::_multiScreenNextOp instead.
	_updateScreen = false;

	ZoombiniGraphics::ScreenKind screenKind = ZoombiniGraphics::kShapeScreen;

	uint16 startShapeIdx = 0;
	uint32 shapeCount = _vm->_gfx->getShapeCount(_cmd._resource);
	MultiScreenOperation multiScreenOp = _multiScreenNextOp;
	_multiScreenNextOp = kMultiScreenOpNone;

	switch (multiScreenOp) {
	case kMultiScreenOpNone: // Do nothing
		return ZmbRenderResult::kSkipped;
	case kMultiScreenOpInit:
		startShapeIdx = _cmd._shapeIdx;
		break;
	case kMultiScreenOpPrev:
		if (_drawShapesPrevShapeIdxStack.empty())
			startShapeIdx = 1;
		else
			startShapeIdx = _drawShapesPrevShapeIdxStack.pop();
		assert(startShapeIdx <= shapeCount);
		break;
	case kMultiScreenOpNext:
		if (_drawShapesNextShapeIdx == 0)
			return ZmbRenderResult::kSkipped;
		startShapeIdx = _drawShapesNextShapeIdx;
		break;
	default:
		error("debug: invalid drawShapesNextOp: %u", multiScreenOp);
		break;
	};

	_vm->_gfx->clearScreen(screenKind);
	_vm->_gfx->fillArea(screenKind, ZoombiniGraphics::kColor0A_White);

	ZoombiniGraphics::TextConf tc;
	tc._fontUsage = ZoombiniFontUsage::kFontDebugText;
	tc._wordWrap = false;

	const uint16 textWidth = 24;
	uint16 maxWidth = 0;
	_drawShapesNextShapeIdx = 0;

	uint32 endShapeIdx = 0;

	Common::Point pos = Common::Point(0, _titleRect.height());
	for (uint32 shapeIdx = startShapeIdx; shapeIdx <= shapeCount && pos.x < ZoombiniGraphics::kScreenWidth; shapeIdx++) {
		Common::Rect shapeSize = _vm->_gfx->getShapeSize(_cmd._resource, shapeIdx);
		if (ZoombiniGraphics::kScreenHeight <= pos.y + shapeSize.height()) {
			pos.x += textWidth + maxWidth;
			pos.y = _titleRect.height();
			maxWidth = 0;
		}
		maxWidth = MAX(maxWidth, static_cast<uint16>(shapeSize.width()));

		if (ZoombiniGraphics::kScreenWidth < pos.x + textWidth + maxWidth)
			_drawShapesNextShapeIdx = shapeIdx;

		Common::Rect textRect = Common::Rect(pos, textWidth, shapeSize.height());
		_vm->_gfx->drawText(screenKind, Common::String::format("%u", shapeIdx), textRect, tc);

		Common::Point shapePos = Common::Point(pos.x + textWidth, pos.y);
		_vm->_gfx->drawShape(screenKind, _cmd._resource, shapeIdx, shapePos);

		pos.y += shapeSize.height();
		endShapeIdx = MAX(endShapeIdx, shapeIdx);
	}

	if (multiScreenOp != kMultiScreenOpPrev && 0 < _drawShapesNextShapeIdx)
		_drawShapesPrevShapeIdxStack.push(startShapeIdx);

	drawTitleText(feature, Common::String::format("[Shapes] tBMP(%s) shape(%u-%u/%u)", _cmd._resource.toString().c_str(), startShapeIdx, endShapeIdx, shapeCount));

	Common::U32String keyLegendText;
	if (endShapeIdx < shapeCount)
		keyLegendText = Common::U32String(U"[<-/->] shape");
	drawEscText(feature, keyLegendText);
	commitDisplayToBackground();

	return ZmbRenderResult::kRendered;
}

ZmbEventHandleResult ZoombiniDialogDebug::drawShapes_onKeyDown(ZmbFeature *feature, const Common::KeyState &kbd, bool kbdRepeat) {
	(void)feature;
	(void)kbdRepeat;

	ZmbEventHandleResult result = ZmbEventHandleResult::kPassthrough;

	switch (classifyDialogKey(kbd)) {
	case kDialogKeyAccept:
	case kDialogKeyCancel:
		close();
		result = ZmbEventHandleResult::kConsumed;
		break;
	case kDialogKeyNone:
		switch (getKeyboardNavDirection(kbd)) {
		case KBD_NAV_LEFT:
			_multiScreenNextOp = kMultiScreenOpPrev;
			result = ZmbEventHandleResult::kConsumed;
			break;
		case KBD_NAV_RIGHT:
			_multiScreenNextOp = kMultiScreenOpNext;
			result = ZmbEventHandleResult::kConsumed;
			break;
		default:
			break;
		}
		break;
	}

	return result;
}

ZmbRenderResult ZoombiniDialogDebug::drawFeature_render(ZmbFeature *feature) {
	// Skip @ref ZoombiniDialogDebug::_updateScreen.
	// Use @ref ZoombiniDialogDebug::_multiScreenNextOp instead.
	_updateScreen = false;

	uint16 startHsId = 0;
	MultiScreenOperation multiScreenOp = _multiScreenNextOp;
	_multiScreenNextOp = kMultiScreenOpNone;

	switch (multiScreenOp) {
	case kMultiScreenOpNone: // Do nothing
		return ZmbRenderResult::kSkipped;
	case kMultiScreenOpInit:
		startHsId = 0;
		break;
	case kMultiScreenOpPrev:
		if (_drawFeaturePrevHsIdxStack.empty())
			startHsId = 0;
		else
			startHsId = _drawFeaturePrevHsIdxStack.pop();
		break;
	case kMultiScreenOpNext:
		if (_drawFeatureNextHsIdx == 0)
			return ZmbRenderResult::kSkipped;
		startHsId = _drawFeatureNextHsIdx;
		break;
	case kMultiScreenOpUp:
		if (_drawFeatureFrame == 0)
			return ZmbRenderResult::kSkipped;
		do {
			_drawFeatureFrame -= 1u;
			ZmbHotspotGroup *nextGroup = feature->getHotspotGroup(_drawFeatureFrame);
			if (nextGroup && nextGroup->getHotspotCount() != 0)
				break; // Found non-empty previous frame
		} while (0 < _drawFeatureFrame);
		break;
	case kMultiScreenOpDown:
		if (feature->getFrameCount() <= _drawFeatureFrame + 1u)
			return ZmbRenderResult::kSkipped;
		do {
			_drawFeatureFrame += 1u;
			ZmbHotspotGroup *nextGroup = feature->getHotspotGroup(_drawFeatureFrame);
			if (nextGroup && nextGroup->getHotspotCount() != 0)
				break; // Found non-empty next frame
		} while (_drawFeatureFrame + 1u < feature->getFrameCount());
		break;
	default:
		error("debug: invalid drawFeatureNextOp: %u", multiScreenOp);
		break;
	};

	ZoombiniGraphics::ScreenKind screenKind = ZoombiniGraphics::kShapeScreen;

	_vm->_gfx->clearScreen(screenKind);
	_vm->_gfx->fillArea(screenKind, ZoombiniGraphics::kColor0A_White);

	ZmbHotspotGroup *hsGroup = feature->getHotspotGroup(_drawFeatureFrame);
	uint32 hsCount = hsGroup->getHotspotCount();
	assert(startHsId < hsCount);

	ZoombiniGraphics::TextConf tc;
	tc._fontUsage = ZoombiniFontUsage::kFontDebugText;
	tc._wordWrap = false;

	const uint16 textWidth = 80;
	uint16 maxWidth = 0;
	uint32 endHsId = 0;
	_drawFeatureNextHsIdx = 0;

	Common::Point pos = Common::Point(0, _titleRect.height());
	for (uint32 hsId = startHsId; hsId < hsCount && pos.x < ZoombiniGraphics::kScreenWidth; hsId++) {
		ZmbHotspot &hs = hsGroup->getHotspot(hsId);
		if (hs._shapeIdx == 0)
			continue; // Skip empty hotspot
		Common::Rect shapeSize = _vm->_gfx->getShapeSize(_cmd._resource, hs._shapeIdx);
		if (ZoombiniGraphics::kScreenHeight <= pos.y + shapeSize.height()) {
			pos.x += textWidth + maxWidth;
			pos.y = _titleRect.height();
			maxWidth = 0;
		}
		maxWidth = MAX(maxWidth, static_cast<uint16>(shapeSize.width()));

		if (ZoombiniGraphics::kScreenWidth < pos.x + textWidth + maxWidth)
			_drawFeatureNextHsIdx = hsId;

		Common::Rect textRect = Common::Rect(pos, textWidth, shapeSize.height());
		_vm->_gfx->drawText(screenKind, Common::String::format("%2u (%3u, %3u)", hs._hsId, hs._x, hs._y), textRect, tc);

		Common::Point shapePos = Common::Point(pos.x + textWidth, pos.y);
		_vm->_gfx->drawShape(screenKind, _cmd._resource, hs._shapeIdx, shapePos);

		pos.y += shapeSize.height();

		endHsId = MAX(endHsId, hsId);
	}

	drawTitleText(feature, Common::String::format("[Feature] tBMP(%s) SCRB(%03d) hotspot(%u-%u/%u) frame(%u/%u)",
												  _cmd._resource.toString().c_str(), _cmd._scrbId,
												  startHsId, endHsId, hsCount,
												  _drawFeatureFrame, feature->getFrameCount()));
	Common::U32String keyLegendText;
	if (endHsId + 1 < hsCount)
		keyLegendText = Common::U32String(U"[<-/->] hs ");
	if (1 < feature->getFrameCount())
		keyLegendText += Common::U32String(U"[UP/DN] frame ");
	drawEscText(feature, keyLegendText);
	commitDisplayToBackground();

	return ZmbRenderResult::kRendered;
}

ZmbRenderResult ZoombiniDialogDebug::drawTerrain_render(ZmbFeature *feature) {
	if (!_updateScreen)
		return ZmbRenderResult::kSkipped;
	_updateScreen = false;

	ZoombiniPage *activePage = _vm->getActivePage();
	if (!activePage || !activePage->hasTerrainBitmap()) {
		ZoombiniGraphics::TextConf textConf;
		textConf._fontUsage = ZoombiniFontUsage::kFontDebugTitle;
		textConf._textPalette = ZoombiniGraphics::kColor0A_White;
		textConf._fillBackground = true;
		textConf._fillBackgroundKey = ZoombiniGraphics::kColor2D_Black;
		textConf._wordWrap = false;
		const Common::Rect messageRect(0, _titleRect.height(), ZoombiniGraphics::kScreenWidth, _titleRect.height() + 14);
		_vm->_gfx->drawText(ZoombiniGraphics::kShapeScreen, Common::U32String(U"No loaded terrain bitmap"), messageRect, textConf);
	} else {
		const uint32 pageType = static_cast<uint32>(activePage->getPageType());
		drawTitleText(feature, Common::String::format("[Terrain] page(%u)", pageType));
		drawEscText(feature);
	}
	commitDisplayToBackground();

	return ZmbRenderResult::kRendered;
}

ZmbEventHandleResult ZoombiniDialogDebug::drawFeature_onKeyDown(ZmbFeature *feature, const Common::KeyState &kbd, bool kbdRepeat) {
	(void)feature;
	(void)kbdRepeat;

	ZmbEventHandleResult result = ZmbEventHandleResult::kPassthrough;

	switch (classifyDialogKey(kbd)) {
	case kDialogKeyAccept:
	case kDialogKeyCancel:
		close();
		result = ZmbEventHandleResult::kConsumed;
		break;
	case kDialogKeyNone:
		switch (getKeyboardNavDirection(kbd)) {
		case KBD_NAV_LEFT:
			_multiScreenNextOp = kMultiScreenOpPrev;
			result = ZmbEventHandleResult::kConsumed;
			break;
		case KBD_NAV_RIGHT:
			_multiScreenNextOp = kMultiScreenOpNext;
			result = ZmbEventHandleResult::kConsumed;
			break;
		case KBD_NAV_UP:
			_multiScreenNextOp = kMultiScreenOpUp;
			result = ZmbEventHandleResult::kConsumed;
			break;
		case KBD_NAV_DOWN:
			_multiScreenNextOp = kMultiScreenOpDown;
			result = ZmbEventHandleResult::kConsumed;
			break;
		default:
			break;
		}
		break;
	}

	return result;
}

ZmbRenderResult ZoombiniDialogDebug::plotPoint_render(ZmbFeature *feature) {
	if (!_updateScreen)
		return ZmbRenderResult::kSkipped;
	_updateScreen = false;

	ZoombiniGraphics::ScreenKind screenKind = ZoombiniGraphics::kShapeScreen;

	// Draw through @ref ZoombiniGraphics so the point participates in dirty tracking.
	Common::Rect pointRect(_cmd._x1, _cmd._y1, _cmd._x1 + 1, _cmd._y1 + 1);
	_vm->_gfx->fillArea(screenKind, pointRect, _cmd._color);

	// Draw title
	drawTitleText(feature, Common::String::format("[Plot Point] at (%d, %d) with color %u", _cmd._x1, _cmd._y1, _cmd._color));
	drawEscText(feature);
	commitDisplayToBackground();

	return ZmbRenderResult::kRendered;
}

ZmbRenderResult ZoombiniDialogDebug::plotLine_render(ZmbFeature *feature) {
	if (!_updateScreen)
		return ZmbRenderResult::kSkipped;
	_updateScreen = false;

	ZoombiniGraphics::ScreenKind screenKind = ZoombiniGraphics::kShapeScreen;

	// Draw the line between the two stored endpoints.
	_vm->_gfx->drawLine(screenKind, Common::Point(_cmd._x1, _cmd._y1), Common::Point(_cmd._x2, _cmd._y2), _cmd._color);

	// Draw title
	drawTitleText(feature, Common::String::format("[Plot Line] from (%d, %d) to (%d, %d) with color %u", _cmd._x1, _cmd._y1, _cmd._x2, _cmd._y2, _cmd._color));
	drawEscText(feature);
	commitDisplayToBackground();

	return ZmbRenderResult::kRendered;
}

ZmbRenderResult ZoombiniDialogDebug::plotRect_render(ZmbFeature *feature) {
	if (!_updateScreen)
		return ZmbRenderResult::kSkipped;
	_updateScreen = false;

	ZoombiniGraphics::ScreenKind screenKind = ZoombiniGraphics::kShapeScreen;

	// Draw through @ref ZoombiniGraphics so each edge participates in dirty tracking.
	Common::Rect rect(_cmd._x1, _cmd._y1, _cmd._x2, _cmd._y2);
	_vm->_gfx->drawLine(screenKind, Common::Point(rect.left, rect.top), Common::Point(rect.right - 1, rect.top), _cmd._color);
	_vm->_gfx->drawLine(screenKind, Common::Point(rect.left, rect.bottom - 1), Common::Point(rect.right - 1, rect.bottom - 1), _cmd._color);
	_vm->_gfx->drawLine(screenKind, Common::Point(rect.left, rect.top), Common::Point(rect.left, rect.bottom - 1), _cmd._color);
	_vm->_gfx->drawLine(screenKind, Common::Point(rect.right - 1, rect.top), Common::Point(rect.right - 1, rect.bottom - 1), _cmd._color);

	// Draw title
	drawTitleText(feature, Common::String::format("[Plot Rect] from (%d, %d) to (%d, %d) with color %u", _cmd._x1, _cmd._y1, _cmd._x2, _cmd._y2, _cmd._color));
	drawEscText(feature);
	commitDisplayToBackground();

	return ZmbRenderResult::kRendered;
}

} // End of namespace Mohawk
