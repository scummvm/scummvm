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

#include "gui/downloaddialog.h"
#include "gui/widgets/list.h"
#include "gui/widget.h"
#include "gui/gui-manager.h"
#include "backends/cloud/cloudmanager.h"
#include "common/translation.h"
#include "widgets/edittext.h"
#include "message.h"
#include "browser.h"
#include "remotebrowser.h"

namespace GUI {

enum {
	kDownloadDialogButtonCmd = 'Dldb'	
};

DownloadDialog::DownloadDialog(uint32 storageId):
	Dialog("GlobalOptions_Cloud_DownloadDialog"), _close(false),
	_workingRequest(nullptr), _ignoreCallback(false) {
	_backgroundType = GUI::ThemeEngine::kDialogBackgroundPlain;

	_browser = new BrowserDialog(_("Select directory where to download game data"), true);
	_remoteBrowser = new RemoteBrowserDialog(_("Select directory with game data"));

	_messageText = new StaticTextWidget(this, "GlobalOptions_Cloud_DownloadDialog.DialogDesc", _("Press the button to download a directory"));
	_mainButton = new ButtonWidget(this, "GlobalOptions_Cloud_DownloadDialog.MainButton", _("Start download"), 0, kDownloadDialogButtonCmd);
	_closeButton = new ButtonWidget(this, "GlobalOptions_Cloud_DownloadDialog.CloseButton", _("OK"), 0, kCloseCmd);
	updateButtons();
}

DownloadDialog::~DownloadDialog() {
	if (_workingRequest) {
		_ignoreCallback = true;
		_workingRequest->finish();
	}
}

void DownloadDialog::close() {
	if (_workingRequest) {
		_ignoreCallback = true;
		_workingRequest->finish();
		_ignoreCallback = false;
	}
	Dialog::close();
}

void DownloadDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kDownloadDialogButtonCmd: {
		if (_workingRequest == nullptr) {
			selectDirectories();
		} else {
			_ignoreCallback = true;
			_workingRequest->finish();
			_ignoreCallback = false;
		}

		reflowLayout();
		draw();
		break;
	}
	default:
		Dialog::handleCommand(sender, cmd, data);
	}
}

void DownloadDialog::selectDirectories() {
	//first user should select remote directory to download	
	if (_remoteBrowser->runModal() <= 0) return;

	Cloud::StorageFile remoteDirectory = _remoteBrowser->getResult();

	//now user should select local directory to download into
	if (_browser->runModal() <= 0) return;

	Common::FSNode dir(_browser->getResult());
	Common::FSList files;
	if (!dir.getChildren(files, Common::FSNode::kListAll)) {
		MessageDialog alert(_("ScummVM couldn't open the specified directory!"));
		alert.runModal();
		return;
	}

	//check that there is no file with the remote directory's name in the local one
	for (Common::FSList::iterator i = files.begin(); i != files.end(); ++i) {
		if (i->getName().equalsIgnoreCase(remoteDirectory.name())) {
			//if there is, ask user whether it's OK
			if (!i->isDirectory()) {
				GUI::MessageDialog alert(_("Cannot create a directory to download - the specified directory has a file with the same name."), _("OK"));
				alert.runModal();
				return;
			}
			GUI::MessageDialog alert(
				Common::String::format(_("The \"%s\" already exists in the specified directory.\nDo you really want to download files into that directory?"), remoteDirectory.name().c_str()),
				_("Yes"),
				_("No")
			);
			if (alert.runModal() != GUI::kMessageOK) return;
			break;
		}
	}

	//make a local path
	Common::String localPath = dir.getPath();

	//simple heuristic to determine which path separator to use
	int backslashes = 0;
	for (uint32 i = 0; i < localPath.size(); ++i)
		if (localPath[i] == '/') --backslashes;
		else if (localPath[i] == '\\') ++backslashes;

	if (backslashes > 0) localPath += '\\' + remoteDirectory.name();
	else localPath += '/' + remoteDirectory.name();

	_workingRequest = CloudMan.downloadFolder(
		remoteDirectory.path(), localPath,
		new Common::Callback<DownloadDialog, Cloud::Storage::FileArrayResponse>(this, &DownloadDialog::directoryDownloadedCallback),
		new Common::Callback<DownloadDialog, Networking::ErrorResponse>(this, &DownloadDialog::directoryDownloadedErrorCallback),
		true
	);
}

void DownloadDialog::directoryDownloadedCallback(Cloud::Storage::FileArrayResponse response) {
	_workingRequest = nullptr;
	if (_ignoreCallback) return;

	//TODO: show response.value (if not empty), show message on OSD
}

void DownloadDialog::directoryDownloadedErrorCallback(Networking::ErrorResponse error) {
	_workingRequest = nullptr;
	if (_ignoreCallback) return;

	//TODO: _showError = true;
}

void DownloadDialog::handleTickle() {
	if (_close) {
		setResult(1);
		close();
	}

	Dialog::handleTickle();
}

void DownloadDialog::reflowLayout() {
	Dialog::reflowLayout();
	updateButtons();
}

void DownloadDialog::updateButtons() {	
	if (_workingRequest != nullptr) {		
		_messageText->setLabel(_("Press the button to cancel the download"));
		_mainButton->setLabel(_("Cancel the download"));
	} else {
		_messageText->setLabel(_("Press the button to download a directory"));
		_mainButton->setLabel(_("Start download"));
	}
}

} // End of namespace GUI
