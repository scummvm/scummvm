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
#include "backends/cloud/cloudmanager.h"
#ifdef USE_SDL_NET
#include "backends/networking/sdl_net/localwebserver.h"
#endif
#include "common/translation.h"
#include "widgets/edittext.h"

namespace GUI {

enum {
	kConnectCmd = 'Cnnt',
	kCodeBoxCmd = 'CdBx'
};

StorageWizardDialog::StorageWizardDialog(uint32 storageId):
	Dialog("GlobalOptions_Cloud_ConnectionWizard"), _storageId(storageId), _close(false) {
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
	for (uint32 i = 0; i < CODE_FIELDS; ++i)
		_codeWidget[i] = new EditTextWidget(this, "GlobalOptions_Cloud_ConnectionWizard.CodeBox" + Common::String::format("%d", i+1), "", 0, kCodeBoxCmd);
	_messageWidget = new StaticTextWidget(this, "GlobalOptions_Cloud_ConnectionWizard.MessageLine", "");

	// Buttons
	new ButtonWidget(this, "GlobalOptions_Cloud_ConnectionWizard.CancelButton", _("Cancel"), 0, kCloseCmd);
	_connectWidget = new ButtonWidget(this, "GlobalOptions_Cloud_ConnectionWizard.ConnectButton", _("Connect"), 0, kConnectCmd);
}

void StorageWizardDialog::open() {
	Dialog::open();
#ifdef USE_SDL_NET
	LocalServer.start();
	LocalServer.indexPageHandler().setTarget(this);
#endif
}

void StorageWizardDialog::close() {
#ifdef USE_SDL_NET
	LocalServer.stopOnIdle();
	LocalServer.indexPageHandler().setTarget(nullptr);
#endif
	Dialog::close();
}

void StorageWizardDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kCodeBoxCmd: {		
		Common::String code, message;
		int correctFields = 0;
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
			if (CODE_FIELDS - correctFields == 1) messageTemplate = _("Field %s has a mistake in it.");
			else messageTemplate = _("Fields %s have mistakes in them.");
			message = Common::String::format(messageTemplate.c_str(), message.c_str());
		}

		bool ok = false;
		if (correctFields == CODE_FIELDS && code.size() > 0) {
			//the last 3 chars must be an encoded crc16
			if (code.size() > 3) {
				uint32 size = code.size();
				uint32 gotcrc = decodeHashchar(code[size-3]) | (decodeHashchar(code[size-2]) << 6) | (decodeHashchar(code[size-1]) << 12);
				code.erase(size - 3);
				uint32 crc = crc16(code);
				ok = (crc == gotcrc);
			}
			if (ok) message = _("All OK!");
			else message = _("Invalid code");
		}
		_connectWidget->setEnabled(ok);
		_messageWidget->setLabel(message);
		break;
	}
	case kConnectCmd: {
		Common::String code;
		for (uint32 i = 0; i < CODE_FIELDS; ++i) {
			Common::String subcode = _codeWidget[i]->getEditString();
			if (subcode.size() == 0) continue;
			code += subcode;
			code.deleteLastChar();
		}
		code.erase(code.size() - 3);
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

int StorageWizardDialog::decodeHashchar(char c) {
	const char HASHCHARS[65] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ?!";	
	for (uint32 i = 0; i < 64; ++i)
		if (c == HASHCHARS[i])
			return i;
	return -1;
}

bool StorageWizardDialog::correctChecksum(Common::String s) {
	if (s.size() == 0) return false; //no last char
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
