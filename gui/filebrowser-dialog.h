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

#ifndef FILEBROWSER_DIALOG_H
#define FILEBROWSER_DIALOG_H

#include "gui/dialog.h"
#include "gui/widgets/edittext.h"

namespace GUI {

class ListWidget;
class EditTextWidget;
class CommandSender;

enum {
	kFBModeLoad = 0,
	kFBModeSave
};

class FileBrowserDialog : public Dialog {
public:
	FileBrowserDialog(const char *title, const char *fileExtension, int mode);

	void open() override;

	void handleCommand(CommandSender *sender, uint32 cmd, uint32 data) override;

	Common::String getResult() { return Dialog::getResult() ? _fileName->getEditString().encode() : Common::String(); }

protected:
	EditTextWidget *_fileName;
	ListWidget	   *_fileList;
	Common::String _fileMask;
	Common::String _fileExt;
	int            _mode;

	void updateListing();
	void normalieFileName();
	bool isProceedSave();
};

} // End of namespace GUI

#endif
