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

#include "gui/filebrowser-dialog.h"

#include "common/system.h"
#include "common/algorithm.h"
#include "common/savefile.h"
#include "common/str-array.h"

#include "common/translation.h"

#include "gui/widgets/list.h"
#include "gui/gui-manager.h"
#include "gui/message.h"

namespace GUI {

enum {
	kChooseCmd = 'Chos'
};

FileBrowserDialog::FileBrowserDialog(const char *title, const char *fileExtension, int mode)
	: Dialog("FileBrowser"), _mode(mode), _fileExt(fileExtension) {

	_fileMask = "*.";
	_fileMask += fileExtension;
	_fileList = NULL;

	new StaticTextWidget(this, "FileBrowser.Headline", title ? title :
					mode == kFBModeLoad ? _("Choose file for loading") : _("Enter filename for saving"));

	_fileName = new EditTextWidget(this, "FileBrowser.Filename", "");

	if (mode == kFBModeLoad)
		_fileName->setEnabled(false);

	// Add file list
	_fileList = new ListWidget(this, "FileBrowser.List");
	_fileList->setNumberingMode(kListNumberingOff);
	_fileList->setEditable(false);

	_backgroundType = GUI::ThemeEngine::kDialogBackgroundPlain;

	// Buttons
	new ButtonWidget(this, "FileBrowser.Cancel", _("Cancel"), 0, kCloseCmd);
	new ButtonWidget(this, "FileBrowser.Choose", _("Choose"), 0, kChooseCmd);
}

void FileBrowserDialog::open() {
	// Call super implementation
	Dialog::open();

	updateListing();
}

void FileBrowserDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kChooseCmd:
		if (_fileName->getEditString().empty())
			break;

		normalieFileName();

		if (!isProceedSave())
			break;

		setResult(1);
		close();
		break;
	case kListSelectionChangedCmd:
		_fileName->setEditString(_fileList->getList().operator[](_fileList->getSelected()).c_str());
		_fileName->markAsDirty();
		break;
	case kListItemActivatedCmd:
	case kListItemDoubleClickedCmd:
		normalieFileName();

		if (!isProceedSave())
			break;

		setResult(1);
		close();
		break;
	default:
		Dialog::handleCommand(sender, cmd, data);
	}
}

void FileBrowserDialog::normalieFileName() {
	Common::String filename = _fileName->getEditString();

	if (filename.matchString(_fileMask))
		return;

	_fileName->setEditString(filename + "." + _fileExt);
}


bool FileBrowserDialog::isProceedSave() {
	bool matched = false;

	if (_mode == kFBModeLoad)
		return true;

	for (ListWidget::StringArray::const_iterator file = _fileList->getList().begin(); file != _fileList->getList().end(); ++file) {
		if (*file == _fileName->getEditString()) {
			matched = true;
			break;
		}
	}

	if (matched) {
		GUI::MessageDialog alert(_("Do you really want to overwrite the file?"), _("Yes"), _("No"));

		if (alert.runModal() != GUI::kMessageOK)
			return false;
	}

	return true;
}

void FileBrowserDialog::updateListing() {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();

	ListWidget::StringArray list;

	Common::StringArray filenames = saveFileMan->listSavefiles(_fileMask);
	Common::sort(filenames.begin(), filenames.end());

	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		list.push_back(file->c_str());
	}

	_fileList->setList(list);
	_fileList->scrollTo(0);

	// Finally, redraw
	g_gui.scheduleTopDialogRedraw();
}

} // End of namespace GUI
