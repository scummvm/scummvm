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

#include "tetraedge/te/te_extended_text_layout.h"

namespace Tetraedge {

TeExtendedTextLayout::TeExtendedTextLayout() {
	_textLayout.setSizeType(RELATIVE_TO_PARENT);
	_textLayout.setAnchor(TeVector3f32(0.5, 0.0, 0.0));
	_textLayout.setPosition(TeVector3f32(0.5, 0.0, 0.0));
	const TeVector3f32 usersz = userSize();
	_textLayout.setSize(TeVector3f32(1.0, 1.0, usersz.z()));
	_scrollingLayout.setContentLayout(&_textLayout);
	_scrollingLayout.setSizeType(RELATIVE_TO_PARENT);
	_scrollingLayout.setSize(TeVector3f32(1.0, 1.0, usersz.z()));
	_scrollingLayout.setDirection(TeVector3f32(0.0, 1.0, 0.0));
	_scrollingLayout.setMouseControl(false);
	_scrollingLayout.setEnclose(true);
	_scrollingLayout.setAutoScrollLoop(1);
	_scrollingLayout.setAutoScrollDelay(4000);
	_scrollingLayout.setAutoScrollAnimation1Enabled(true);
	_scrollingLayout.setAutoScrollAnimation1Delay(0);
	_scrollingLayout.setAutoScrollAnimation1Speed(0.1f);
	_scrollingLayout.setAutoScrollAnimation2Enabled(false);
	_scrollingLayout.setAutoScrollAnimation2Delay(0);
	_scrollingLayout.setAutoScrollAnimation2Speed(0.1f);
	addChild(&_scrollingLayout);
}

void TeExtendedTextLayout::setAutoScrollDelay(int val) {
	_scrollingLayout.setAutoScrollDelay(val);
}

void TeExtendedTextLayout::setAutoScrollSpeed(float val) {
	_scrollingLayout.setAutoScrollAnimation1Speed(val);
	_scrollingLayout.setAutoScrollAnimation2Speed(val);
}

void TeExtendedTextLayout::setText(const Common::String &val) {
	_textLayout.setText(val);
	_scrollingLayout.resetScrollPosition();
	_scrollingLayout.playAutoScroll();
}

void TeExtendedTextLayout::setInterLine(float val) {
	_textLayout.setInterLine(val);
}

void TeExtendedTextLayout::setWrapMode(TeTextBase2::WrapMode mode) {
	if (mode == TeTextBase2::WrapModeFixed) {
		_textLayout.setAnchor(TeVector3f32(0.5f, 0.0f, 0.0f));
		_textLayout.setPosition(TeVector3f32(0.5f, 0.0f, 0.0f));
		_scrollingLayout.setDirection(TeVector3f32(0.0, 1.0, 0.0));
	} else {
		_textLayout.setAnchor(TeVector3f32(0.0f, 0.5f, 0.0f));
		_textLayout.setPosition(TeVector3f32(0.0f, 0.5f, 0.0f));
		_scrollingLayout.setDirection(TeVector3f32(1.0, 0.0, 0.0));
	}
	_scrollingLayout.setContentLayout(nullptr);
	_scrollingLayout.setContentLayout(&_textLayout);
	_textLayout.setWrapMode(mode);
}

void TeExtendedTextLayout::setTextSizeType(int type) {
	_textLayout.setTextSizeType(type);
}

void TeExtendedTextLayout::setTextSizeProportionalToWidth(int val) {
	_textLayout.setTextSizeProportionalToWidth(val);
}


} // end namespace Tetraedge
