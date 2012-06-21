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
 */

#ifndef GUI_RECORDER_DIALOG_H
#define GUI_RECORDER_DIALOG_H
#include "common/stream.h"
#include "gui/dialog.h"
namespace GUI {

class ListWidget;
class GraphicsWidget;
class ButtonWidget;
class CommandSender;
class ContainerWidget;
class StaticTextWidget;


class RecorderDialog : public GUI::Dialog {
private:
	Common::SeekableReadStream *_playbackFile;
	Common::String _target;
	Common::String _filename;
	int _currentScreenshot;
	int _screenShotsCount;
	GUI::ListWidget *_list;
	GUI::ContainerWidget *_container;
	GUI::GraphicsWidget *_gfxWidget;
	GUI::StaticTextWidget *_currentScreenshotText;
	void updateList();
	void updateScreenShotsText();
	void updateSelection(bool redraw);
	void updateScreenshot();
	int calculateScreenshotsCount();
	Common::String generateRecordFileName();
	Graphics::Surface *getScreenShot(int number);
	bool skipToNextScreenshot();
public:
	enum DialogResult {
		kRecordDialogClose,
		kRecordDialogRecord,
		kRecordDialogPlayback
	};
	RecorderDialog();
	~RecorderDialog();
	virtual void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data);
	int runModal(Common::String &target);
	virtual void reflowLayout();
	const Common::String getFileName() {return _filename;}
};

}  // End of namespace GUI


#endif
