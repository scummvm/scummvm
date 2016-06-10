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

#include "gui/storagewizarddialog.h"
#include "gui/widgets/list.h"
#include "gui/widget.h"
#include "gui/gui-manager.h"

#include "common/translation.h"
#include "backends/cloud/cloudmanager.h"
#include "widgets/edittext.h"

namespace GUI {

enum {
	kConnectCmd = 'Cnnt',
	kCodeBoxCmd = 'CdBx'
};

StorageWizardDialog::StorageWizardDialog(uint32 storageId): Dialog("GlobalOptions_Cloud_ConnectionWizard"), _storageId(storageId) {
	_backgroundType = GUI::ThemeEngine::kDialogBackgroundPlain;

	Common::String headline = Common::String::format(_("%s Storage Connection Wizard"), CloudMan.listStorages()[_storageId].c_str());
	new StaticTextWidget(this, "GlobalOptions_Cloud_ConnectionWizard.Headline", headline);
	
	new StaticTextWidget(this, "GlobalOptions_Cloud_ConnectionWizard.NavigateLine", _s("Navigate to the following URL:"));

	Common::String url = "https://www.scummvm.org/c/";
	switch (storageId) {
	case Cloud::kStorageDropboxId: url += "db"; break;
	case Cloud::kStorageOneDriveId: url += "od"; break;
	case Cloud::kStorageGoogleDriveId: url += "gd"; break;
	}

	new StaticTextWidget(this, "GlobalOptions_Cloud_ConnectionWizard.URLLine", url);

	new StaticTextWidget(this, "GlobalOptions_Cloud_ConnectionWizard.ReturnLine1", _s("Obtain the code from the storage, enter it"));
	new StaticTextWidget(this, "GlobalOptions_Cloud_ConnectionWizard.ReturnLine2", _s("in the following field and press 'Connect':"));
	_codeWidget = new EditTextWidget(this, "GlobalOptions_Cloud_ConnectionWizard.CodeBox", _s("Code"), 0, kCodeBoxCmd);

	// Buttons
	new ButtonWidget(this, "GlobalOptions_Cloud_ConnectionWizard.CancelButton", _("Cancel"), 0, kCloseCmd);
	new ButtonWidget(this, "GlobalOptions_Cloud_ConnectionWizard.ConnectButton", _("Connect"), 0, kConnectCmd);
}

void StorageWizardDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kCodeBoxCmd:
		break;
	case kConnectCmd:
		CloudMan.connectStorage(_storageId, _codeWidget->getEditString());
		setResult(1);
		close();
		break;
	default:
		Dialog::handleCommand(sender, cmd, data);
	}
}

} // End of namespace GUI
