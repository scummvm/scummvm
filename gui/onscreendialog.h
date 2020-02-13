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

#ifndef GUI_ONSCREENDIALOG_H
#define GUI_ONSCREENDIALOG_H

#include "gui/dialog.h"
#include "gui/widget.h"

namespace GUI {

class OnScreenDialog : public Dialog {
private:
	uint32 _lastTime;
	bool _enableDrag;
	bool _mouseOver;
	bool _editDlgShown;
	Common::Point _dragPoint;
	GUI::StaticTextWidget *_text;
	Dialog *_dlg;

	bool isMouseOver(int x, int y);

public:
	OnScreenDialog(bool recordingMode);
	~OnScreenDialog() override;
	void close() override;
	bool isVisible() const override;
	void reflowLayout() override;

	void setReplayedTime(uint32 newTime);

	void handleMouseMoved(int x, int y, int button) override;
	void handleMouseDown(int x, int y, int button, int clickCount) override;
	void handleMouseUp(int x, int y, int button, int clickCount) override;
	void handleCommand(CommandSender *sender, uint32 cmd, uint32 data) override;

	bool isMouseOver();
	bool isEditDlgVisible();
	Dialog *getActiveDlg();
protected:
	void releaseFocus() override;
};

} // End of namespace GUI

#endif
