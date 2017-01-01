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

#include "titanic/support/screen_manager.h"
#include "titanic/star_control/star_view.h"
#include "titanic/star_control/star_control.h"
#include "titanic/star_control/star_field.h"
#include "titanic/core/game_object.h"

namespace Titanic {

CStarView::CStarView() : _sub12(nullptr, nullptr), _sub13((void *)nullptr),
		_owner(nullptr), _starField(nullptr), _videoSurface(nullptr), _field118(0),
		_videoSurface2(nullptr), _field210(0), _homePhotoMask(nullptr),
		_field218(0), _field21C(0) {
	CStar20Data data = { 0, 0, 0x47C35000, 0, 0x41A00000,
		0x3F800000, 0x3F800000, 0x3F800000 };

	_sub12.proc3(&data);
}

void CStarView::load(SimpleFile *file, int param) {
	if (!param) {
		_sub12.load(file, param);

		_field118 = file->readNumber();
		if (_field118)
			_sub13.load(file, 0);

		_field218 = file->readNumber();
		_field21C = file->readNumber();
	}
}

void CStarView::save(SimpleFile *file, int indent) {
	_sub12.save(file, indent);

	file->writeNumberLine(_field118, indent);
	if (_field118)
		_sub13.save(file, indent);

	file->writeNumberLine(_field218, indent);
	file->writeNumberLine(_field21C, indent);
}

void CStarView::setup(CScreenManager *screenManager, CStarField *starField, CStarControl *starControl) {
	_starField = starField;
	_owner = starControl;
}

void CStarView::reset() {
	// TODO
}

void CStarView::draw(CScreenManager *screenManager) {
	if (!screenManager || !_videoSurface || !_starField)
		return;

	if (_fader.isActive()) {
		CVideoSurface *surface = _field21C ? _videoSurface2 : _videoSurface;
		surface = _fader.fade(screenManager, surface);
		screenManager->blitFrom(SURFACE_PRIMARY, surface);
	} else {
		Point destPos(20, 10);

		if (_field21C) {
			screenManager->blitFrom(SURFACE_PRIMARY, _videoSurface2, &destPos);

			if (!_homePhotoMask && _owner) {
				_homePhotoMask = _owner->getHiddenObject("HomePhotoMask");
			}

			if (_homePhotoMask)
				_homePhotoMask->draw(screenManager, Point(20, 187));
		} else {
			fn1();

			// Render the display
			_videoSurface->clear();
			_videoSurface->lock();
			_starField->render(_videoSurface, &_sub12);
			_videoSurface->unlock();

			// Blit the resulting surface to the screen
			screenManager->blitFrom(SURFACE_PRIMARY, _videoSurface, &destPos);
		}
	}
}

void CStarView::MouseButtonDownMsg(int unused, const Point &pt) {
	// TODO
}

void CStarView::MouseMoveMsg(int unused, const Point &pt) {
	// TODO
}

CErrorCode CStarView::KeyCharMsg(int key) {
	// TODO
	return CErrorCode();
}

bool CStarView::canSetStarDestination() const {
	// TODO
	return false;
}

void CStarView::starDestinationSet() {
	// TODO
}

void CStarView::fn1() {
	// TODO
}

} // End of namespace Titanic
