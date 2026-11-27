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

#include "mohawk/zoombini_pages/dialog_version.h"
#include "mohawk/zoombini.h"
#include "mohawk/zoombini_graphics.h"

namespace Mohawk {

ZoombiniDialogVersion::ZoombiniDialogVersion(MohawkEngine_Zoombini *vm)
	: ZoombiniDialog(vm, ZoombiniPageType::kDialogVersion) {
}

void ZoombiniDialogVersion::setBackgroundBitmap() {
	ZoombiniDialog::setBackgroundBitmap();

	// Center the version panel on the screen.
	const Common::Point screenCenter = _vm->_gfx->getScreenRect().center();
	const Common::Rect panelRect = Common::Rect::center(screenCenter.x, screenCenter.y, 340, 152);

	// Draw a white panel with a one-pixel black border.
	const ZoombiniGraphics::ScreenKind screenKind = ZoombiniGraphics::kBackScreen;
	_vm->_gfx->fillArea(screenKind, panelRect, ZoombiniGraphics::kColor0A_White);
	_vm->_gfx->frameArea(screenKind, panelRect, ZoombiniGraphics::kColor2D_Black);

	// Center and wrap the version text inside the panel's margins.
	Common::Rect textRect = panelRect;
	textRect.grow(-9);
	ZoombiniGraphics::TextConf textConf;
	textConf._hAlign = Graphics::kTextAlignCenter;
	textConf._vAlign = Graphics::kTextAlignCenter;
	textConf._wordWrap = true;
	_vm->_gfx->drawText(screenKind, ZoombiniText::kDialogBodyVersionInfo, textRect, textConf);
}

ZmbEventHandleResult ZoombiniDialogVersion::onKeyDown(const Common::KeyState &kbd, bool kbdRepeat) {
	(void)kbd;
	(void)kbdRepeat;
	close();
	return ZmbEventHandleResult::kConsumed;
}

ZmbEventHandleResult ZoombiniDialogVersion::onLButtonDown(const Common::Point &absPos, const Common::Point &relPos) {
	(void)absPos;
	(void)relPos;
	close();
	return ZmbEventHandleResult::kConsumed;
}

ZmbEventHandleResult ZoombiniDialogVersion::onMButtonDown(const Common::Point &absPos, const Common::Point &relPos) {
	// The version dialog reacts on mouse middle button click, unlike other dialogs.
	// Reuse the left-click handler to dismiss the panel on wheel click.
	return onLButtonDown(absPos, relPos);
}

ZmbEventHandleResult ZoombiniDialogVersion::onRButtonDown(const Common::Point &absPos, const Common::Point &relPos) {
	return onLButtonDown(absPos, relPos);
}

} // End of namespace Mohawk
