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

#include "backends/cloud/cloudmanager.h"
#include "gui/downloadiconsdialog.h"
#include "gui/downloaddialog.h"
#include "backends/networking/curl/session.h"
#include "common/config-manager.h"
#include "common/translation.h"
#include "common/util.h"
#include "engines/metaengine.h"
#include "gui/browser.h"
#include "gui/chooser.h"
#include "gui/editgamedialog.h"
#include "gui/gui-manager.h"
#include "gui/launcher.h"
#include "gui/message.h"
#include "gui/remotebrowser.h"
#include "gui/widgets/edittext.h"
#include "gui/widgets/list.h"

namespace GUI {

enum {
	kDownloadIconsDialogButtonCmd = 'Dldb',
	kListEndedCmd = 'DLLE'
};

static DownloadIconsDialog *g_dialog;

DownloadIconsDialog::DownloadIconsDialog() :
	Dialog("GlobalOptions_DownloadIconsDialog"), CommandSender(this), _close(false) {
	g_dialog = this;

	_backgroundType = GUI::ThemeEngine::kDialogBackgroundPlain;

	_statusText = new StaticTextWidget(this, "GlobalOptions_DownloadIconsDialog.StatusText", _("Downloading icons list..."));
	_errorText = new StaticTextWidget(this, "GlobalOptions_DownloadIconsDialog.ErrorText", Common::U32String(""));

	uint32 progress = (uint32)(100 * CloudMan.getDownloadingProgress());
	_progressBar = new SliderWidget(this, "GlobalOptions_DownloadIconsDialog.ProgressBar");
	_progressBar->setMinValue(0);
	_progressBar->setMaxValue(100);
	_progressBar->setValue(progress);
	_progressBar->setEnabled(false);
	_percentLabel = new StaticTextWidget(this, "GlobalOptions_DownloadIconsDialog.PercentText", Common::String::format("%u %%", progress));
	_downloadSizeLabel = new StaticTextWidget(this, "GlobalOptions_DownloadIconsDialog.DownloadSize", Common::U32String());
	_downloadSpeedLabel = new StaticTextWidget(this, "GlobalOptions_DownloadIconsDialog.DownloadSpeed", Common::U32String());
	if (g_system->getOverlayWidth() > 320)
		_cancelButton = new ButtonWidget(this, "GlobalOptions_DownloadIconsDialog.MainButton", _("Cancel download"), Common::U32String(), kDownloadIconsDialogButtonCmd);
	else
		_cancelButton = new ButtonWidget(this, "GlobalOptions_DownloadIconsDialog.MainButton", _c("Cancel download", "lowres"), Common::U32String(), kDownloadIconsDialogButtonCmd);

	_closeButton = new ButtonWidget(this, "GlobalOptions_DownloadIconsDialog.CloseButton", _("Hide"), Common::U32String(), kCloseCmd);
	refreshWidgets();

	CloudMan.setDownloadTarget(this);

	_session = new Networking::Session();

	downloadList();
}

DownloadIconsDialog::~DownloadIconsDialog() {
	CloudMan.setDownloadTarget(nullptr);

	_session->close();
	delete _session;
}

void DownloadIconsDialog::open() {
	Dialog::open();
	reflowLayout();
	g_gui.scheduleTopDialogRedraw();
}

void DownloadIconsDialog::close() {
	CloudMan.setDownloadTarget(nullptr);
	Dialog::close();
}

void DownloadIconsDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kDownloadIconsDialogButtonCmd:
		{
			CloudMan.setDownloadTarget(nullptr);
			CloudMan.cancelDownload();
			close();
			break;
		}
	case kDownloadProgressCmd:
		if (!_close) {
			refreshWidgets();
			g_gui.scheduleTopDialogRedraw();
		}
		break;
	case kDownloadEndedCmd:
		_close = true;
		break;
	case kListEndedCmd:
		_statusText->setLabel(Common::U32String::format(_("Downloading icons list... %d entries"), _fileHash.size()));
		calculateList();
		break;
	default:
		Dialog::handleCommand(sender, cmd, data);
	}
}

void DownloadIconsDialog::handleTickle() {
	if (_close) {
		close();
		_close = false;
		return;
	}

	int32 progress = (int32)(100 * CloudMan.getDownloadingProgress());
	if (_progressBar->getValue() != progress) {
		refreshWidgets();
		g_gui.scheduleTopDialogRedraw();
	}

	Dialog::handleTickle();
}

void DownloadIconsDialog::reflowLayout() {
	Dialog::reflowLayout();
	refreshWidgets();
}

Common::U32String DownloadIconsDialog::getSizeLabelText() {
	Common::String downloaded, downloadedUnits, total, totalUnits;
	downloaded = getHumanReadableBytes(CloudMan.getDownloadBytesNumber(), downloadedUnits);
	total = getHumanReadableBytes(CloudMan.getDownloadTotalBytesNumber(), totalUnits);
	return Common::U32String::format(_("Downloaded %s %S / %s %S"), downloaded.c_str(), _(downloadedUnits).c_str(), total.c_str(), _(totalUnits).c_str());
}

Common::U32String DownloadIconsDialog::getSpeedLabelText() {
	Common::String speed, speedUnits;
	speed = getHumanReadableBytes(CloudMan.getDownloadSpeed(), speedUnits);
	speedUnits += "/s";
	return Common::U32String::format(_("Download speed: %s %S"), speed.c_str(), _(speedUnits).c_str());
}

void DownloadIconsDialog::refreshWidgets() {
	uint32 progress = (uint32)(100 * CloudMan.getDownloadingProgress());
	_percentLabel->setLabel(Common::String::format("%u %%", progress));
	_downloadSizeLabel->setLabel(getSizeLabelText());
	_downloadSpeedLabel->setLabel(getSpeedLabelText());
	_progressBar->setValue(progress);
}

void DownloadIconsDialog::downloadListCallback(Networking::DataResponse response) {
	Networking::SessionRequest *req = dynamic_cast<Networking::SessionRequest *>(response.request);

	Common::MemoryReadStream stream(req->getData(), req->getSize());

	int nline = 0;

	while (!stream.eos()) {
		Common::String s = stream.readString('\n');

		nline++;

		if (s.empty())
			continue;

		size_t pos = s.findFirstOf(',');

		if (pos == Common::String::npos) {
			warning("DownloadIconsDialog: wrong string format at line %d: <%s>", nline, s.c_str());
			continue;
		}

		g_dialog->_fileHash.setVal(s.substr(0, pos), atol(s.substr(pos + 1).c_str()));
	}

	sendCommand(kListEndedCmd, 0);
}

void DownloadIconsDialog::errorCallback(Networking::ErrorResponse error) {
	warning("Error %ld: %s", error.httpResponseCode, error.response.c_str());
}

void DownloadIconsDialog::downloadList() {
	Networking::SessionRequest *rq = _session->get("https://downloads.scummvm.org/frs/icons/LIST",
		new Common::Callback<DownloadIconsDialog, Networking::DataResponse>(this, &DownloadIconsDialog::downloadListCallback),
		new Common::Callback<DownloadIconsDialog, Networking::ErrorResponse>(this, &DownloadIconsDialog::errorCallback),
		true);

	rq->start();
}

void DownloadIconsDialog::calculateList() {
	if (!ConfMan.hasKey("iconspath")) {
		_errorText->setLabel(_("ERROR: No icons path set"));
		return;
	}

	// Scan all files in iconspath and remove present ones from the
	// donwloaded files list
	Common::FSDirectory *iconDir = new Common::FSDirectory(ConfMan.get("iconspath"));
	Common::ArchiveMemberList iconFiles;

	iconDir->listMatchingMembers(iconFiles, "gui-icons*.dat");

	for (auto ic = iconFiles.begin(); ic != iconFiles.end(); ++ic) {
		Common::String fname = (*ic)->getName();

		if (_fileHash.contains(fname))
			_fileHash.erase(fname);
	}

	delete iconDir;

	// Now calculate the size of the missing files
	uint32 totalsize = 0;
	for (auto f = _fileHash.begin(); f != _fileHash.end(); ++f) {
		totalsize += f->_value;
	}

	if (totalsize == 0) {
		_statusText->setLabel(_("No new icons packs available"));
		return;
	}

	Common::String size, sizeUnits;
	size = getHumanReadableBytes(totalsize, sizeUnits);

	_statusText->setLabel(Common::U32String::format(_("Need to download %d files, %s %S"), _fileHash.size(), size.c_str(), _(sizeUnits).c_str()));

}

} // End of namespace GUI
