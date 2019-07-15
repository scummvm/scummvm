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
#include "gui/gui-manager.h"
#include "gui/message.h"
#include "gui/widget.h"
#include "gui/widgets/edittext.h"
#include "gui/widgets/scrollcontainer.h"
#include "backends/cloud/cloudmanager.h"
#ifdef USE_SDL_NET
#include "backends/networking/sdl_net/localwebserver.h"
#endif
#include "common/translation.h"

namespace GUI {

enum {
	kConnectCmd = 'Cnnt',
	kCodeBoxCmd = 'CdBx',
	kOpenUrlCmd = 'OpUr',
	kPasteCodeCmd = 'PsCd',
	kStorageWizardContainerReflowCmd = 'SWCr'
};

StorageWizardDialog::StorageWizardDialog(uint32 storageId):
	Dialog("GlobalOptions_Cloud_ConnectionWizard"), _storageId(storageId), _close(false) {
#ifdef USE_SDL_NET
	_stopServerOnClose = false;
#endif
	_backgroundType = GUI::ThemeEngine::kDialogBackgroundPlain;

	ScrollContainerWidget *container = new ScrollContainerWidget(this, "GlobalOptions_Cloud_ConnectionWizard.Container", kStorageWizardContainerReflowCmd);
	container->setTarget(this);

	Common::String headline = Common::String::format(_("%s Storage Connection Wizard"), CloudMan.listStorages()[_storageId].c_str());
	_headlineWidget = new StaticTextWidget(container, "GlobalOptions_Cloud_ConnectionWizard_Container.Headline", headline);

	_navigateLineWidget = new StaticTextWidget(container, "GlobalOptions_Cloud_ConnectionWizard_Container.NavigateLine", _("Navigate to the following URL:"));
	_urlLineWidget = new StaticTextWidget(container, "GlobalOptions_Cloud_ConnectionWizard_Container.URLLine", getUrl());

	_returnLine1 = new StaticTextWidget(container, "GlobalOptions_Cloud_ConnectionWizard_Container.ReturnLine1", _("Obtain the code from the storage, enter it"));
	_returnLine2 = new StaticTextWidget(container, "GlobalOptions_Cloud_ConnectionWizard_Container.ReturnLine2", _("in the following field and press 'Connect':"));
	_codeWidget = new EditTextWidget(container, "GlobalOptions_Cloud_ConnectionWizard_Container.CodeBox1", "", 0, kCodeBoxCmd);
	_messageWidget = new StaticTextWidget(container, "GlobalOptions_Cloud_ConnectionWizard_Container.MessageLine", "");

	// Buttons
	_cancelWidget = new ButtonWidget(container, "GlobalOptions_Cloud_ConnectionWizard_Container.CancelButton", _("Cancel"), 0, kCloseCmd);
	_openUrlWidget = new ButtonWidget(container, "GlobalOptions_Cloud_ConnectionWizard_Container.OpenUrlButton", _("Open URL"), 0, kOpenUrlCmd);
	_pasteCodeWidget = new ButtonWidget(container, "GlobalOptions_Cloud_ConnectionWizard_Container.PasteCodeButton", _("Paste"), _("Pastes clipboard contents into fields"), kPasteCodeCmd);
	_connectWidget = new ButtonWidget(container, "GlobalOptions_Cloud_ConnectionWizard_Container.ConnectButton", _("Connect"), 0, kConnectCmd);

	// Initialy the code is empty, so disable the connect button
	_connectWidget->setEnabled(false);

	_picture = new GraphicsWidget(container, "GlobalOptions_Cloud_ConnectionWizard_Container.Picture");
#ifndef DISABLE_FANCY_THEMES
	if (g_gui.theme()->supportsImages()) {
		_picture->useThemeTransparency(true);
		switch (_storageId) {
		case Cloud::kStorageDropboxId:
			_picture->setGfx(g_gui.theme()->getImageSurface(ThemeEngine::kImageDropboxLogo));
			break;
		case Cloud::kStorageOneDriveId:
			_picture->setGfx(g_gui.theme()->getImageSurface(ThemeEngine::kImageOneDriveLogo));
			break;
		case Cloud::kStorageGoogleDriveId:
			_picture->setGfx(g_gui.theme()->getImageSurface(ThemeEngine::kImageGoogleDriveLogo));
			break;
		case Cloud::kStorageBoxId:
			_picture->setGfx(g_gui.theme()->getImageSurface(ThemeEngine::kImageBoxLogo));
			break;
		}
	}
#endif

	containerWidgetsReflow();
}

void StorageWizardDialog::open() {
	Dialog::open();

	if (CloudMan.isWorking()) {
		bool doClose = true;

		MessageDialog alert(_("Another Storage is active. Do you want to interrupt it?"), _("Yes"), _("No"));
		if (alert.runModal() == GUI::kMessageOK) {
			if (CloudMan.isDownloading())
				CloudMan.cancelDownload();
			if (CloudMan.isSyncing())
				CloudMan.cancelSync();

			// I believe it still would return `true` here, but just in case
			if (CloudMan.isWorking()) {
				MessageDialog alert2(_("Wait until current Storage finishes up and try again."));
				alert2.runModal();
			} else {
				doClose = false;
			}
		}

		if (doClose) {
			close();
		}
	}
}

void StorageWizardDialog::close() {
	Dialog::close();
}

void StorageWizardDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kCodeBoxCmd: {
		Common::String code = _codeWidget->getEditString();
		bool ok = (code.size() > 0);
		_connectWidget->setEnabled(ok);
		_messageWidget->setLabel("");
		break;
	}
	case kOpenUrlCmd: {
		if (!g_system->openUrl(getUrl())) {
			MessageDialog alert(_("Failed to open URL!\nPlease navigate to this page manually."));
			alert.runModal();
		}
		break;
	}
	case kPasteCodeCmd: {
		if (g_system->hasTextInClipboard()) {
			Common::String message = g_system->getTextFromClipboard();
			if (!message.empty()) {
				_codeWidget->setEditString(message);
			}
			handleCommand(sender, kCodeBoxCmd, data);
			g_gui.scheduleTopDialogRedraw();
		}
		break;
	}
	case kConnectCmd: {
		Common::String code = _codeWidget->getEditString();
		if (code.size() == 0)
			return;

		CloudMan.connectStorage(_storageId, code);
		setResult(1);
		close();
		break;
	}
#ifdef USE_SDL_NET
	case kStorageCodePassedCmd:
		CloudMan.connectStorage(_storageId, LocalServer.indexPageHandler().code());
		_close = true;
		break;
#endif
	case kStorageWizardContainerReflowCmd:
		containerWidgetsReflow();
		break;
	default:
		Dialog::handleCommand(sender, cmd, data);
	}
}

void StorageWizardDialog::handleTickle() {
	if (_close) {
		setResult(1);
		close();
	}

	Dialog::handleTickle();
}

void StorageWizardDialog::containerWidgetsReflow() {
	// contents
	if (_headlineWidget) _headlineWidget->setVisible(true);
	if (_navigateLineWidget) _navigateLineWidget->setVisible(true);
	if (_urlLineWidget) _urlLineWidget->setVisible(true);
	if (_returnLine1) _returnLine1->setVisible(true);
	if (_returnLine2) _returnLine2->setVisible(true);
	
	_codeWidget->setVisible(true);
	_messageWidget->setVisible(true);

	// left column / first bottom row
	if (_picture) {
		_picture->setVisible(g_system->getOverlayWidth() > 320);
	}
	if (_openUrlWidget) {
		bool visible = g_system->hasFeature(OSystem::kFeatureOpenUrl);
		_openUrlWidget->setVisible(visible);
	}
	if (_pasteCodeWidget) {
		bool visible = g_system->hasFeature(OSystem::kFeatureClipboardSupport);
		_pasteCodeWidget->setVisible(visible);
	}

	// bottom row
	if (_cancelWidget) _cancelWidget->setVisible(true);
	if (_connectWidget) _connectWidget->setVisible(true);
}

Common::String StorageWizardDialog::getUrl() const {
	return "https://cloud.scummvm.org/";
}

} // End of namespace GUI
