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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/base/base_active_rect.h"
#include "engines/wintermute/base/gfx/base_renderer.h"
#include "engines/wintermute/base/gfx/base_surface.h"
#include "engines/wintermute/base/gfx/base_image.h"
#include "engines/wintermute/base/base_sub_frame.h"
#include "engines/wintermute/base/base_region.h"
#include "engines/wintermute/base/base_engine.h"
#include "engines/wintermute/platform_osystem.h"
#include "engines/wintermute/base/base_persistence_manager.h"
#include "engines/wintermute/dcgf.h"

#ifdef ENABLE_WME3D
#include "engines/wintermute/base/gfx/xmodel.h"
#endif

namespace Wintermute {

//////////////////////////////////////////////////////////////////////
BaseRenderer::BaseRenderer(BaseGame *inGame) : BaseClass(inGame) {
	_window = 0;
	_clipperWindow = 0;
	_active = false;
	_ready = false;
	_windowed = true;
	_forceAlphaColor = 0x00;

	_indicatorDisplay = false;
	_indicatorColor = BYTETORGBA(255, 0, 0, 128);
	_indicatorProgress = 0;
	_indicatorX = -1;
	_indicatorY = -1;
	_indicatorWidth = -1;
	_indicatorHeight = 8;
	_indicatorWidthDrawn = 0;

	_loadImageName = "";
	_saveImageName = "";
	_saveLoadImage = nullptr;
	_loadInProgress = false;
	_hasDrawnSaveLoadImage = false;

	_saveImageX = _saveImageY = 0;
	_loadImageX = _loadImageY = 0;

	_width = _height = _bPP = 0;
	BasePlatform::setRectEmpty(&_monitorRect);

	_realWidth = _realHeight = 0;
	_drawOffsetX = _drawOffsetY = 0;
}

//////////////////////////////////////////////////////////////////////
BaseRenderer::~BaseRenderer() {
	deleteRectList();
	unclipCursor();
	SAFE_DELETE(_saveLoadImage);
}

//////////////////////////////////////////////////////////////////////
void BaseRenderer::initLoop() {
	deleteRectList();
}

//////////////////////////////////////////////////////////////////////
BaseObject *BaseRenderer::getObjectAt(int x, int y) {
	Common::Point32 point;
	point.x = x;
	point.y = y;

	for (int32 i = (int32)_rectList.getSize() - 1; i >= 0; i--) {
		if (BasePlatform::ptInRect(&_rectList[i]->_rect, point)) {
			if (_rectList[i]->_precise) {
				// frame
				if (_rectList[i]->_frame) {
					int xx = (int)((_rectList[i]->_frame->getRect().left + x - _rectList[i]->_rect.left + _rectList[i]->_offsetX) / (float)((float)_rectList[i]->_zoomX / (float)100));
					int yy = (int)((_rectList[i]->_frame->getRect().top  + y - _rectList[i]->_rect.top  + _rectList[i]->_offsetY) / (float)((float)_rectList[i]->_zoomY / (float)100));

					if (_rectList[i]->_frame->_mirrorX) {
						int width = _rectList[i]->_frame->getRect().right - _rectList[i]->_frame->getRect().left;
						xx = width - xx;
					}

					if (_rectList[i]->_frame->_mirrorY) {
						int height = _rectList[i]->_frame->getRect().bottom - _rectList[i]->_frame->getRect().top;
						yy = height - yy;
					}

					if (!_rectList[i]->_frame->_surface->isTransparentAt(xx, yy)) {
						return _rectList[i]->_owner;
					}
				}

#ifdef ENABLE_WME3D
				else if (_rectList[i]->_xmodel) {
					if (!_rectList[i]->_xmodel->isTransparentAt(x, y)) {
						return _rectList[i]->_owner;
					}
				}
#endif
				// region
				else if (_rectList[i]->_region) {
					if (_rectList[i]->_region->pointInRegion(x + _rectList[i]->_offsetX, y + _rectList[i]->_offsetY)) {
						return _rectList[i]->_owner;
					}
				}
			} else {
				return _rectList[i]->_owner;
			}
		}
	}

	return (BaseObject *)nullptr;
}

//////////////////////////////////////////////////////////////////////////
void BaseRenderer::deleteRectList() {
	for (int32 i = 0; i < _rectList.getSize(); i++) {
		delete _rectList[i];
	}
	_rectList.removeAll();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
bool BaseRenderer::initRenderer(int width, int height, bool windowed) {
	return STATUS_FAILED;
}

//////////////////////////////////////////////////////////////////////
void BaseRenderer::onWindowChange() {
}

//////////////////////////////////////////////////////////////////////////
bool BaseRenderer::windowedBlt() {
	return STATUS_FAILED;
}

//////////////////////////////////////////////////////////////////////////
bool BaseRenderer::setup2D(bool Force) {
	return STATUS_FAILED;
}

#ifdef ENABLE_WME3D
//////////////////////////////////////////////////////////////////////////
bool BaseRenderer::setup3D(Camera3D *camera, bool force) {
	return STATUS_FAILED;
}
#endif

//////////////////////////////////////////////////////////////////////////
bool BaseRenderer::fillRect(int x, int y, int w, int h, uint32 color) {
	return STATUS_FAILED;
}

//////////////////////////////////////////////////////////////////////////
void BaseRenderer::initIndicator() {
	if (_indicatorY == -1) {
		_indicatorY = _height - _indicatorHeight;
	}
	if (_indicatorX == -1) {
		_indicatorX = 0;
	}
	if (_indicatorWidth == -1) {
		_indicatorWidth = _width;
	}
}

void BaseRenderer::setIndicator(int width, int height, int x, int y, uint32 color) {
	_indicatorWidth = width;
	_indicatorHeight = height;
	_indicatorX = x;
	_indicatorY = y;
	_indicatorColor = color;
}

void BaseRenderer::setIndicatorVal(int value) {
	bool redisplay = (_indicatorProgress != value);
	_indicatorProgress = value;
	if (redisplay)
		displayIndicator();
}

void BaseRenderer::setLoadingScreen(const char *filename, int x, int y) {
	if (filename == nullptr) {
		_saveImageName = "";
	} else {
		_loadImageName = filename;
	}
	_loadImageX = x;
	_loadImageY = y;
}

void BaseRenderer::setSaveImage(const char *filename, int x, int y) {
	if (filename == nullptr) {
		_saveImageName = "";
	} else {
		_saveImageName = filename;
	}
	_saveImageX = x;
	_saveImageY = y;
}

void BaseRenderer::initSaveLoad(bool isSaving, bool quickSave) {
	_indicatorDisplay = true;
	_indicatorProgress = 0;
	_hasDrawnSaveLoadImage = false;

	if (isSaving && !quickSave) {
		SAFE_DELETE(_saveLoadImage);
		if (_saveImageName.size()) {
			_saveLoadImage = createSurface();

			if (!_saveLoadImage || DID_FAIL(_saveLoadImage->create(_saveImageName, true, 0, 0, 0))) {
				SAFE_DELETE(_saveLoadImage);
			}
		}
	} else {
		SAFE_DELETE(_saveLoadImage);
		if (_loadImageName.size()) {
			_saveLoadImage = createSurface();

			if (!_saveLoadImage || DID_FAIL(_saveLoadImage->create(_loadImageName, true, 0, 0, 0))) {
				SAFE_DELETE(_saveLoadImage);
			}
		}
		_loadInProgress = true;
	}
}

void BaseRenderer::endSaveLoad() {
	_loadInProgress = false;
	_indicatorDisplay = false;
	_indicatorWidthDrawn = 0;

	SAFE_DELETE(_saveLoadImage);
}

void BaseRenderer::persistSaveLoadImages(BasePersistenceManager *persistMgr) {
	persistMgr->transferString(TMEMBER(_loadImageName));
	persistMgr->transferString(TMEMBER(_saveImageName));
	persistMgr->transferSint32(TMEMBER(_saveImageX));
	persistMgr->transferSint32(TMEMBER(_saveImageY));
	persistMgr->transferSint32(TMEMBER(_loadImageX));
	persistMgr->transferSint32(TMEMBER(_loadImageY));
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
bool BaseRenderer::setViewport(int left, int top, int right, int bottom) {
	return STATUS_FAILED;
}

//////////////////////////////////////////////////////////////////////////
bool BaseRenderer::setScreenViewport() {
	return setViewport(_drawOffsetX, _drawOffsetY, _width + _drawOffsetX, _height + _drawOffsetY);
}

//////////////////////////////////////////////////////////////////////////
bool BaseRenderer::setViewport(Common::Rect32 *rect) {
	return setViewport(rect->left + _drawOffsetX,
	                   rect->top + _drawOffsetY,
	                   rect->right + _drawOffsetX,
	                   rect->bottom + _drawOffsetY);
}

//////////////////////////////////////////////////////////////////////////
bool BaseRenderer::clipCursor() {
	// TODO: Reimplement this. (Currently aspect-indpendence isn't quite finished)
	/*
	if (!_windowed) {
	    Common::Rect32 rc;
	    GetWindowRect(_window, &rc);

	    // if "maintain aspect ratio" is in effect, lock mouse to visible area
	    rc.left = _drawOffsetX;
	    rc.top = _drawOffsetY;
	    rc.right = rc.left + _width;
	    rc.bottom = rc.top + _height;

	    ::ClipCursor(&rc);
	}
	 */
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BaseRenderer::unclipCursor() {
	/*
	if (!_windowed) ::ClipCursor(nullptr);
	*/
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BaseRenderer::pointInViewport(Common::Point32 *p) {
	if (p->x < _drawOffsetX) {
		return false;
	}
	if (p->y < _drawOffsetY) {
		return false;
	}
	if (p->x > _drawOffsetX + _width) {
		return false;
	}
	if (p->y > _drawOffsetY + _height) {
		return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool BaseRenderer::displayIndicator() {
	if (!_indicatorDisplay || !_indicatorProgress) {
		return STATUS_OK;
	}

#ifdef ENABLE_FOXTAIL
	if (BaseEngine::instance().isFoxTail()) {
		_hasDrawnSaveLoadImage = false;
		clear();
		displaySaveloadRect();
		displaySaveloadImage();
		forcedFlip();
		return STATUS_OK;
	}
#endif

	displaySaveloadImage();
	displaySaveloadRect();
	indicatorFlip();
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BaseRenderer::displaySaveloadImage() {
	if (_saveLoadImage && !_hasDrawnSaveLoadImage) {
		Common::Rect32 rc;
		BasePlatform::setRect(&rc, 0, 0, _saveLoadImage->getWidth(), _saveLoadImage->getHeight());
		if (_loadInProgress) {
			_saveLoadImage->displayTrans(_loadImageX, _loadImageY, rc);
		} else {
			_saveLoadImage->displayTrans(_saveImageX, _saveImageY, rc);
		}
		flip();
		_hasDrawnSaveLoadImage = true;
	}

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BaseRenderer::displaySaveloadRect() {
	if ((!_indicatorDisplay && _indicatorWidth <= 0) || _indicatorHeight <= 0) {
		return STATUS_OK;
	}
	int curWidth = (int)(_indicatorWidth * (float)((float)_indicatorProgress / 100.0f));
	fillRect(_indicatorX, _indicatorY, curWidth, _indicatorHeight, _indicatorColor);
	_indicatorWidthDrawn = curWidth;
	return STATUS_OK;
}

} // End of namespace Wintermute
