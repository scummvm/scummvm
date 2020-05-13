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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/base/base_viewport.h"
#include "engines/wintermute/base/base_engine.h"
#include "engines/wintermute/base/base_persistence_manager.h"
#include "engines/wintermute/base/gfx/base_renderer.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(BaseViewport, false)

//////////////////////////////////////////////////////////////////////////
BaseViewport::BaseViewport(BaseGame *inGame) : BaseClass(inGame) {
	_rect.setEmpty();
	_mainObject = nullptr;
	_offsetX = _offsetY = 0;
}


//////////////////////////////////////////////////////////////////////////
BaseViewport::~BaseViewport() {

}


//////////////////////////////////////////////////////////////////////////
bool BaseViewport::persist(BasePersistenceManager *persistMgr) {

	persistMgr->transferPtr(TMEMBER_PTR(_gameRef));

	persistMgr->transferPtr(TMEMBER_PTR(_mainObject));
	persistMgr->transferSint32(TMEMBER(_offsetX));
	persistMgr->transferSint32(TMEMBER(_offsetY));
	persistMgr->transferRect32(TMEMBER(_rect));

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool BaseViewport::setRect(int32 left, int32 top, int32 right, int32 bottom, bool noCheck) {
	if (!noCheck) {
		left = MAX<int32>(left, 0);
		top = MAX<int32>(top, 0);
		right = MIN(right, BaseEngine::instance().getRenderer()->getWidth());
		bottom = MIN(bottom, BaseEngine::instance().getRenderer()->getHeight());
	}

	_rect.setRect(left, top, right, bottom);
	_offsetX = left;
	_offsetY = top;
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
Rect32 *BaseViewport::getRect() {
	return &_rect;
}


//////////////////////////////////////////////////////////////////////////
int BaseViewport::getWidth() const {
	return _rect.right - _rect.left;
}


//////////////////////////////////////////////////////////////////////////
int BaseViewport::getHeight() const {
	return _rect.bottom - _rect.top;
}

Common::String BaseViewport::debuggerToString() const {
	return Common::String::format("%p: BaseViewport: (top, right, bottom, left): (%d, %d, %d, %d)", (const void *)this, _rect.top, _rect.right, _rect.bottom, _rect.left);
}
} // End of namespace Wintermute
