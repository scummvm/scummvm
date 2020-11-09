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

#include "common/system.h"
#include "common/translation.h"

#include "gui/gui-manager.h"
#include "gui/ThemeEval.h"
#include "gui/widget.h"

#include "engines/grim/md5checkdialog.h"
#include "engines/grim/md5check.h"

namespace Grim {

MD5CheckDialog::MD5CheckDialog() :
		GUI::Dialog(30, 20, 260, 124) {

	const int screenW = g_system->getOverlayWidth();
	const int screenH = g_system->getOverlayHeight();

	Common::U32String message = _(
		"ScummVM will now verify the game data files, to make sure you have the best gaming experience.\n"
		"This may take a while, please wait.\nSuccessive runs will not check them again.");

	// First, determine the size the dialog needs. For this we have to break
	// down the string into lines, and taking the maximum of their widths.
	// Using this, and accounting for the space the button(s) need, we can set
	// the real size of the dialog
	Common::Array<Common::U32String> lines;
	g_gui.getFont().wordWrapText(message, screenW - 2 * 20, lines);
	_w = screenW - 20;
	int maxlineWidth = _w - 20;

	int lineCount = lines.size();

	_h = 60 + kLineHeight;

	// Limit the number of lines so that the dialog still fits on the screen.
	if (lineCount > (screenH - 20 - _h) / kLineHeight) {
		lineCount = (screenH - 20 - _h) / kLineHeight;
	}
	_h += lineCount * kLineHeight;

	// Center the dialog
	_x = (screenW - _w) / 2;
	_y = (screenH - _h) / 2;

	// Each line is represented by one static text item.
	int height = 10;
	for (int i = 0; i < lineCount; i++) {
		new GUI::StaticTextWidget(this, 10, height, maxlineWidth, kLineHeight,
								  lines[i], Graphics::kTextAlignCenter);
		height += kLineHeight;
	}
	height += 20;

	_progressSliderWidget = new GUI::SliderWidget(this, 20, height + 10, _w - 40, 10);

	check();
}

void MD5CheckDialog::check() {
	_checkOk = true;
	MD5Check::startCheckFiles();
}

void MD5CheckDialog::handleTickle() {
	int p, t;
	Common::String filename;
	if (!MD5Check::advanceCheck(&p, &t)) {
		_checkOk = false;
	}
	_progressSliderWidget->setValue(p * 100 / t);
	_progressSliderWidget->markAsDirty();

	if (p == t) {
		setResult(_checkOk);
		close();
	}
}

}
