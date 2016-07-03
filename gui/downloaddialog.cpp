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

namespace GUI {

enum {
	kDownloadDialogButtonCmd = 'Dldb'	
};

DownloadDialog::DownloadDialog(uint32 storageId):
	Dialog("GlobalOptions_Cloud_DownloadDialog"), _wasInProgress(true), _inProgress(false), _close(false) {
	_backgroundType = GUI::ThemeEngine::kDialogBackgroundPlain;

	_messageText = new StaticTextWidget(this, "GlobalOptions_Cloud_DownloadDialog.DialogDesc", _("Press the button to download a directory"));
	_mainButton = new ButtonWidget(this, "GlobalOptions_Cloud_DownloadDialog.MainButton", _("Start download"), 0, kDownloadDialogButtonCmd);
	_closeButton = new ButtonWidget(this, "GlobalOptions_Cloud_DownloadDialog.CloseButton", _("OK"), 0, kCloseCmd);
	updateButtons();
}

void DownloadDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kDownloadDialogButtonCmd: {
		_inProgress = !_inProgress;
		reflowLayout();
		break;
	}
	default:
		Dialog::handleCommand(sender, cmd, data);
	}
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
	if (_wasInProgress == _inProgress) return;

	if (_inProgress) {		
		_messageText->setLabel(_("Press the button to cancel the download"));
		_mainButton->setLabel(_("Cancel the download"));
	} else {
		_messageText->setLabel(_("Press the button to download a directory"));
		_mainButton->setLabel(_("Start download"));
	}	
	
	_wasInProgress = _inProgress;	
}


} // End of namespace GUI
