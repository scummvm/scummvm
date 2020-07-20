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

#ifndef GUI_RECORDER_DIALOG_H
#define GUI_RECORDER_DIALOG_H
#include "common/stream.h"
#include "common/recorderfile.h"
#include "gui/dialog.h"
namespace GUI {

class ListWidget;
class GraphicsWidget;
class ButtonWidget;
class CommandSender;
class ContainerWidget;
class StaticTextWidget;

class RecorderDialog : public GUI::Dialog {
	using GUI::Dialog::runModal;

private:
	bool _firstScreenshotUpdate;
	Common::PlaybackFile _playbackFile;
	Common::String _target;
	Common::String _filename;
	int _currentScreenshot;
	int _screenShotsCount;
	Common::Array<Common::PlaybackFile::PlaybackFileHeader> _fileHeaders;
	GUI::ListWidget *_list;
	GUI::ContainerWidget *_container;
	GUI::GraphicsWidget *_gfxWidget;
	GUI::StaticTextWidget *_currentScreenshotText;
	GUI::StaticTextWidget *_authorText;
	GUI::StaticTextWidget *_notesText;
	GUI::ButtonWidget *_editButton;
	GUI::ButtonWidget *_deleteButton;
	GUI::ButtonWidget *_playbackButton;

	void updateList();
	void updateScreenShotsText();
	void updateSelection(bool redraw);
	void updateScreenshot();
public:
	Common::U32String _author;
	Common::String    _name;
	Common::String    _notes;
	enum DialogResult {
		kRecordDialogClose,
		kRecordDialogRecord,
		kRecordDialogPlayback
	};
	RecorderDialog();
	~RecorderDialog() override;

	void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) override;
	void reflowLayout() override;

	int runModal(Common::String &target);
	const Common::String getFileName() {return _filename;}
};

}  // End of namespace GUI


#endif
