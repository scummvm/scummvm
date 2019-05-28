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
	for (uint32 i = 0; i < CODE_FIELDS; ++i)
		_codeWidget[i] = new EditTextWidget(container, "GlobalOptions_Cloud_ConnectionWizard_Container.CodeBox" + Common::String::format("%d", i+1), "", 0, kCodeBoxCmd);
	_messageWidget = new StaticTextWidget(container, "GlobalOptions_Cloud_ConnectionWizard_Container.MessageLine", "");

	// Buttons
	_cancelWidget = new ButtonWidget(container, "GlobalOptions_Cloud_ConnectionWizard_Container.CancelButton", _("Cancel"), 0, kCloseCmd);
	_openUrlWidget = new ButtonWidget(container, "GlobalOptions_Cloud_ConnectionWizard_Container.OpenUrlButton", _("Open URL"), 0, kOpenUrlCmd);
	_pasteCodeWidget = new ButtonWidget(container, "GlobalOptions_Cloud_ConnectionWizard_Container.PasteCodeButton", _("Paste"), _("Pastes clipboard contents into fields"), kPasteCodeCmd);
	_connectWidget = new ButtonWidget(container, "GlobalOptions_Cloud_ConnectionWizard_Container.ConnectButton", _("Connect"), 0, kConnectCmd);

	// Initialy the code is empty, so disable the connect button
	_connectWidget->setEnabled(false);

	if (Cloud::CloudManager::couldUseLocalServer()) {
		// hide fields and even the button if local webserver is on
		_returnLine1->setLabel(_("You will be directed to ScummVM's page where"));
		_returnLine2->setLabel(_("you should allow it to access your storage."));
	}

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
			return;
		}
	}

#ifdef USE_SDL_NET
	if (Cloud::CloudManager::couldUseLocalServer()) {
		_stopServerOnClose = !LocalServer.isRunning();
		LocalServer.start(true); // using "minimal mode" (no "/files", "/download", etc available)
		LocalServer.indexPageHandler().setTarget(this);
	}
#endif
}

void StorageWizardDialog::close() {
#ifdef USE_SDL_NET
	if (Cloud::CloudManager::couldUseLocalServer()) {
		if (_stopServerOnClose)
			LocalServer.stopOnIdle();
		LocalServer.indexPageHandler().setTarget(nullptr);
	}
#endif
	Dialog::close();
}

void StorageWizardDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kCodeBoxCmd: {
		Common::String code, message;
		uint32 correctFields = 0;
		for (uint32 i = 0; i < CODE_FIELDS; ++i) {
			Common::String subcode = _codeWidget[i]->getEditString();
			if (subcode.size() == 0) {
				++correctFields;
				continue;
			}
			bool correct = correctChecksum(subcode);
			if (correct) {
				code += subcode;
				code.deleteLastChar();
				++correctFields;
			} else {
				if (i == correctFields) { //first incorrect field
					message += Common::String::format("#%d", i + 1);
				} else {
					message += Common::String::format(", #%d", i + 1);
				}
			}
		}

		if (message.size() > 0) {
			Common::String messageTemplate;
			if (CODE_FIELDS - correctFields == 1)
				messageTemplate = _("Field %s has a mistake in it.");
			else
				messageTemplate = _("Fields %s have mistakes in them.");
			message = Common::String::format(messageTemplate.c_str(), message.c_str());
		}

		bool ok = false;
		if (correctFields == CODE_FIELDS && code.size() > 0) {
			//the last 3 chars must be an encoded crc16
			if (code.size() > 3) {
				uint32 size = code.size();
				uint32 gotcrc = decodeHashchar(code[size - 3]) | (decodeHashchar(code[size - 2]) << 6) | (decodeHashchar(code[size - 1]) << 12);
				code.erase(size - 3);
				uint32 crc = crc16(code);
				ok = (crc == gotcrc);
			}
			if (ok)
				message = _("All OK!");
			else
				message = _("Invalid code");
		}
		_connectWidget->setEnabled(ok);
		_messageWidget->setLabel(message);
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
			for (uint32 i = 0; i < CODE_FIELDS; ++i) {
				if (message.empty()) break;
				Common::String subcode = "";
				for (uint32 j = 0; j < message.size(); ++j) {
					if (message[j] == ' ') {
						message.erase(0, j+1);
						break;
					}
					subcode += message[j];
					if (j+1 == message.size()) {
						message = "";
						break;
					}
				}
				_codeWidget[i]->setEditString(subcode);
			}
			handleCommand(sender, kCodeBoxCmd, data);
			g_gui.scheduleTopDialogRedraw();
		}
		break;
	}
	case kConnectCmd: {
		Common::String code;
		for (uint32 i = 0; i < CODE_FIELDS; ++i) {
			Common::String subcode = _codeWidget[i]->getEditString();
			if (subcode.size() == 0)
				continue;
			code += subcode;
			code.deleteLastChar();
		}
		if (code.size() > 3) {
			code.erase(code.size() - 3);
			CloudMan.connectStorage(_storageId, code);
			setResult(1);
			close();
		}
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

	bool showFields = (!Cloud::CloudManager::couldUseLocalServer());
	for (uint32 i = 0; i < CODE_FIELDS; ++i)
		_codeWidget[i]->setVisible(showFields);
	_messageWidget->setVisible(showFields);

	// left column / first bottom row
	if (_picture) {
		_picture->setVisible(g_system->getOverlayWidth() > 320);
	}
	if (_openUrlWidget) {
		bool visible = g_system->hasFeature(OSystem::kFeatureOpenUrl);
		_openUrlWidget->setVisible(visible);
	}
	if (_pasteCodeWidget) {
		bool visible = showFields && g_system->hasFeature(OSystem::kFeatureClipboardSupport);
		_pasteCodeWidget->setVisible(visible);
	}

	// bottom row
	if (_cancelWidget) _cancelWidget->setVisible(true);
	if (_connectWidget) {
		_connectWidget->setVisible(showFields);
	}
}

Common::String StorageWizardDialog::getUrl() const {
	Common::String url = "https://www.scummvm.org/c/";
	switch (_storageId) {
	case Cloud::kStorageDropboxId:
		url += "db";
		break;
	case Cloud::kStorageOneDriveId:
		url += "od";
		break;
	case Cloud::kStorageGoogleDriveId:
		url += "gd";
		break;
	case Cloud::kStorageBoxId:
		url += "bx";
		break;
	}

	if (Cloud::CloudManager::couldUseLocalServer())
		url += "s";

	return url;
}

int StorageWizardDialog::decodeHashchar(char c) {
	const char HASHCHARS[65] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ?!";
	for (uint32 i = 0; i < 64; ++i)
		if (c == HASHCHARS[i])
			return i;
	return -1;
}

bool StorageWizardDialog::correctChecksum(Common::String s) {
	if (s.size() == 0)
		return false; //no last char
	int providedChecksum = decodeHashchar(s.lastChar());
	int calculatedChecksum = 0x2A; //any initial value would do, but it must equal to the one used on the page where these checksums were generated
	for (uint32 i = 0; i < s.size()-1; ++i) {
		calculatedChecksum = calculatedChecksum ^ s[i];
	}
	return providedChecksum == (calculatedChecksum % 64);
}

uint32 StorageWizardDialog::crc16(Common::String s) { //"CRC16_CCITT_FALSE"
	uint32 crc = 0xFFFF, x;
	for (uint32 i = 0; i < s.size(); ++i) {
		x = ((crc >> 8) ^ s[i]) & 0xFF;
		x ^= x >> 4;
		crc = ((crc << 8) ^ (x << 12) ^ (x << 5) ^ x) & 0xFFFF;
	}
	return crc;
}

} // End of namespace GUI
