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

#include "gui/remotebrowser.h"
#include "gui/gui-manager.h"
#include "gui/widgets/list.h"

#include "common/config-manager.h"
#include "common/system.h"
#include "common/algorithm.h"

#include "common/translation.h"
#include "backends/networking/curl/request.h"
#include "backends/cloud/storage.h"
#include "backends/cloud/cloudmanager.h"
#include "message.h"

namespace GUI {

enum {
	kChooseCmd = 'Chos',
	kGoUpCmd = 'GoUp'
};

RemoteBrowserDialog::RemoteBrowserDialog(const Common::U32String &title):
	Dialog("Browser"), _navigationLocked(false), _updateList(false), _showError(false),
	_workingRequest(nullptr), _ignoreCallback(false) {
	_backgroundType = GUI::ThemeEngine::kDialogBackgroundPlain;

	new StaticTextWidget(this, "Browser.Headline", title);
	_currentPath = new StaticTextWidget(this, "Browser.Path", Common::U32String("DUMMY"));

	_fileList = new ListWidget(this, "Browser.List");
	_fileList->setNumberingMode(kListNumberingOff);
	_fileList->setEditable(false);

	if (g_system->getOverlayWidth() > 320)
		new ButtonWidget(this, "Browser.Up", _("Go up"), _("Go to previous directory level"), kGoUpCmd);
	else
		new ButtonWidget(this, "Browser.Up", _c("Go up", "lowres"), _("Go to previous directory level"), kGoUpCmd);
	new ButtonWidget(this, "Browser.Cancel", _("Cancel"), Common::U32String(), kCloseCmd);
	new ButtonWidget(this, "Browser.Choose", _("Choose"), Common::U32String(), kChooseCmd);
}

RemoteBrowserDialog::~RemoteBrowserDialog() {
	if (_workingRequest) {
		_ignoreCallback = true;
		_workingRequest->finish();
	}
}

void RemoteBrowserDialog::open() {
	Dialog::open();
	listDirectory(Cloud::StorageFile());
}

void RemoteBrowserDialog::close() {
	Dialog::close();
	if (_workingRequest) {
		_ignoreCallback = true;
		_workingRequest->finish();
		_ignoreCallback = false;
	}
}

void RemoteBrowserDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kChooseCmd: {
		// If nothing is selected in the list widget, choose the current dir.
		// Else, choose the dir that is selected.
		int selection = _fileList->getSelected();
		if (selection >= 0)
			_choice = _nodeContent[selection];
		else
			_choice = _node;
		setResult(1);
		close();
		break;
	}
	case kGoUpCmd:
		goUp();
		break;
	case kListItemActivatedCmd:
	case kListItemDoubleClickedCmd:
		if (_nodeContent[data].isDirectory()) {
			_rememberedNodeContents[_node.path()] = _nodeContent;
			listDirectory(_nodeContent[data]);
		}
		break;
	case kListSelectionChangedCmd:
		// We do not allow selecting directories,
		// thus we will invalidate the selection
		// when the user selects a directory over here.
		if (data != (uint32)-1 && !_nodeContent[data].isDirectory())
			_fileList->setSelected(-1);
		break;
	default:
		Dialog::handleCommand(sender, cmd, data);
	}
}

void RemoteBrowserDialog::handleTickle() {
	if (_updateList) {
		updateListing();
		_updateList = false;
	}

	if (_showError) {
		_showError = false;
		MessageDialog alert(_("ScummVM could not access the directory!"));
		alert.runModal();
	}

	Dialog::handleTickle();
}

void RemoteBrowserDialog::updateListing() {
	// Update the path display
	Common::String path = _node.path();
	if (path.empty())
		path = "/"; //root
	if (_navigationLocked)
		path = "Loading... " + path;
	_currentPath->setLabel(path);

	if (!_navigationLocked) {
		// Populate the ListWidget
		ListWidget::U32StringArray list;
		ListWidget::ColorList colors;
		for (Common::Array<Cloud::StorageFile>::iterator i = _nodeContent.begin(); i != _nodeContent.end(); ++i) {
			if (i->isDirectory()) {
				list.push_back(i->name() + "/");
				colors.push_back(ThemeEngine::kFontColorNormal);
			} else {
				list.push_back(i->name());
				colors.push_back(ThemeEngine::kFontColorAlternate);
			}
		}

		_fileList->setList(list, &colors);
		_fileList->scrollTo(0);
	}

	_fileList->setEnabled(!_navigationLocked);

	// Finally, redraw
	g_gui.scheduleTopDialogRedraw();
}

void RemoteBrowserDialog::goUp() {
	if (_rememberedNodeContents.contains(_node.path()))
		_rememberedNodeContents.erase(_node.path());

	Common::String path = _node.path();
	if (path.size() && (path.lastChar() == '/' || path.lastChar() == '\\'))
		path.deleteLastChar();
	if (path.empty()) {
		_rememberedNodeContents.erase("");
	} else {
		for (int i = path.size() - 1; i >= 0; --i)
			if (i == 0 || path[i] == '/' || path[i] == '\\') {
				path.erase(i);
				break;
			}
	}

	listDirectory(Cloud::StorageFile(path, 0, 0, true));
}

void RemoteBrowserDialog::listDirectory(const Cloud::StorageFile &node) {
	if (_navigationLocked || _workingRequest)
		return;

	if (_rememberedNodeContents.contains(node.path())) {
		_nodeContent = _rememberedNodeContents[node.path()];
	} else {
		_navigationLocked = true;

		_workingRequest = CloudMan.listDirectory(
			node.path(),
			new Common::Callback<RemoteBrowserDialog, Cloud::Storage::ListDirectoryResponse>(this, &RemoteBrowserDialog::directoryListedCallback),
			new Common::Callback<RemoteBrowserDialog, Networking::ErrorResponse>(this, &RemoteBrowserDialog::directoryListedErrorCallback),
			false
		);
	}

	_backupNode = _node;
	_node = node;
	updateListing();
}

void RemoteBrowserDialog::directoryListedCallback(Cloud::Storage::ListDirectoryResponse response) {
	_workingRequest = nullptr;
	if (_ignoreCallback)
		return;

	_navigationLocked = false;
	_nodeContent = response.value;
	Common::sort(_nodeContent.begin(), _nodeContent.end(), FileListOrder());
	_updateList = true;
}

void RemoteBrowserDialog::directoryListedErrorCallback(Networking::ErrorResponse error) {
	_workingRequest = nullptr;
	if (_ignoreCallback)
		return;

	_navigationLocked = false;
	_node = _backupNode;
	_updateList = true;
	_showError = true;
}

} // End of namespace GUI
