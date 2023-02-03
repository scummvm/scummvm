/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "backends/networking/curl/request.h"
#include "gui/downloadpacksdialog.h"
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
	kDownloadProceedCmd = 'Dlpr',
	kListDownloadFinishedCmd = 'DlLE',
	kCleanupCmd = 'DlCL',
	kClearCacheCmd = 'DlCC'
};

struct DialogState {
	DownloadPacksDialog *dialog;
	Networking::Session session;
	Common::HashMap<Common::String, uint32> fileHash;
	IconProcessState state;
	uint32 downloadedSize;
	uint32 totalSize;
	uint32 totalFiles;
	uint32 startTime;
	const char *listfname;
	uint32 lastUpdate;

	DialogState(const char *fname) {
		listfname = fname;
		state = kDownloadStateNone;
		downloadedSize = totalSize = totalFiles = startTime = lastUpdate = 0;
		dialog = nullptr;
	}

	void downloadList();
	void proceedDownload();

	void downloadListCallback(Networking::DataResponse response);
	void downloadFileCallback(Networking::DataResponse response);
	void errorCallback(Networking::ErrorResponse error);

private:
	bool takeOneFile();
} static *g_state;


void DialogState::downloadList() {
	Networking::SessionRequest *rq = session.get(Common::String::format("https://downloads.scummvm.org/frs/icons/%s", listfname), "",
		new Common::Callback<DialogState, Networking::DataResponse>(this, &DialogState::downloadListCallback),
		new Common::Callback<DialogState, Networking::ErrorResponse>(this, &DialogState::errorCallback),
		true);

	rq->start();
}

void DialogState::proceedDownload() {
	startTime = lastUpdate = g_system->getMillis();
	takeOneFile();
}

bool DialogState::takeOneFile() {
	auto f = fileHash.begin();
	if (f == fileHash.end())
		return false;

	Common::String fname = f->_key;
	fileHash.erase(fname);

	Common::String url = Common::String::format("https://downloads.scummvm.org/frs/icons/%s", fname.c_str());
	Common::String localFile = normalizePath(ConfMan.get("iconspath") + "/" + fname, '/');

	Networking::SessionRequest *rq = session.get(url, localFile,
		new Common::Callback<DialogState, Networking::DataResponse>(this, &DialogState::downloadFileCallback),
		new Common::Callback<DialogState, Networking::ErrorResponse>(this, &DialogState::errorCallback));

	rq->start();
	return true;
}

void DialogState::downloadListCallback(Networking::DataResponse r) {
	Networking::SessionFileResponse *response = static_cast<Networking::SessionFileResponse *>(r.value);
	Common::MemoryReadStream stream(response->buffer, response->len);

	int nline = 0;
	while (!stream.eos()) {
		Common::String s = stream.readString('\n');
		nline++;
		if (s.empty())
			continue;

		size_t pos = s.findFirstOf(',');
		if (pos == Common::String::npos) {
			warning("DownloadPacksDialog: wrong string format at line %d: <%s>", nline, s.c_str());
			continue;
		}

		fileHash.setVal(s.substr(0, pos), atol(s.substr(pos + 1).c_str()));
	}

	state = kDownloadStateListDownloaded;
	if (dialog)
		dialog->sendCommand(kListDownloadFinishedCmd, 0);
}

void DialogState::downloadFileCallback(Networking::DataResponse r) {
	Networking::SessionFileResponse *response = static_cast<Networking::SessionFileResponse *>(r.value);

	downloadedSize += response->len;
	if (response->eos) {
		if (!takeOneFile()) {
			state = kDownloadComplete;
			if (dialog)
				dialog->sendCommand(kDownloadEndedCmd, 0);
			return;
		}
	}

	if (dialog && g_system->getMillis() > lastUpdate + 500) {
		lastUpdate = g_system->getMillis();
		dialog->sendCommand(kDownloadProgressCmd, 0);
	}
}

void DialogState::errorCallback(Networking::ErrorResponse error) {
	Common::U32String message = Common::U32String::format(_("ERROR %d: %s"), error.httpResponseCode, error.response.c_str());

	if (dialog)
		dialog->setError(message);
}

static uint32 getDownloadingProgress() {
	if (!g_state || g_state->totalSize == 0)
		return 0;

	uint32 progress = (uint32)(100 * ((double)g_state->downloadedSize / (double)g_state->totalSize));

	return progress;
}

static uint32 getDownloadSpeed() {
	uint32 speed = 1000 * ((double)g_state->downloadedSize / (g_system->getMillis() - g_state->startTime));

	return speed;
}

DownloadPacksDialog::DownloadPacksDialog(Common::U32String packname, const char *listfname, const char *packsglob) :
	Dialog("GlobalOptions_DownloadPacksDialog"), CommandSender(this), _close(false), _packname(packname),
	_packsglob(packsglob) {

	_backgroundType = GUI::ThemeEngine::kDialogBackgroundPlain;

	// I18N: String like "Downloading icon packs list..."
	_statusText = new StaticTextWidget(this, "GlobalOptions_DownloadPacksDialog.StatusText", Common::U32String::format(_("Downloading %S list..."), _packname.c_str()));
	_errorText = new StaticTextWidget(this, "GlobalOptions_DownloadPacksDialog.ErrorText", Common::U32String(""));

	uint32 progress = getDownloadingProgress();
	_progressBar = new SliderWidget(this, "GlobalOptions_DownloadPacksDialog.ProgressBar");
	_progressBar->setMinValue(0);
	_progressBar->setMaxValue(100);
	_progressBar->setValue(progress);
	_progressBar->setEnabled(false);
	_percentLabel = new StaticTextWidget(this, "GlobalOptions_DownloadPacksDialog.PercentText", Common::String::format("%u %%", progress));
	_downloadSizeLabel = new StaticTextWidget(this, "GlobalOptions_DownloadPacksDialog.DownloadSize", Common::U32String());
	_downloadSpeedLabel = new StaticTextWidget(this, "GlobalOptions_DownloadPacksDialog.DownloadSpeed", Common::U32String());
	_cancelButton = new ButtonWidget(this, "GlobalOptions_DownloadPacksDialog.MainButton", _("Cancel download"), Common::U32String(), kCleanupCmd);
	_closeButton = new ButtonWidget(this, "GlobalOptions_DownloadPacksDialog.CloseButton", _("Hide"), Common::U32String(), kCloseCmd);
	_clearCacheButton = new ButtonWidget(this, "GlobalOptions_DownloadPacksDialog.ResetButton", _("Clear Cache"), Common::U32String(), kClearCacheCmd);

	if (!g_state) {
		g_state = new DialogState(listfname);

		g_state->dialog = this;

		setState(kDownloadStateList);
		refreshWidgets();

		g_state->downloadList();
	} else {
		g_state->dialog = this;

		setState(g_state->state);
		refreshWidgets();
	}
}

DownloadPacksDialog::~DownloadPacksDialog() {
}

void DownloadPacksDialog::open() {
	Dialog::open();
	reflowLayout();
	g_gui.scheduleTopDialogRedraw();
}

void DownloadPacksDialog::close() {
	if (g_state)
		g_state->dialog = nullptr;

	Dialog::close();
}

void DownloadPacksDialog::setState(IconProcessState state) {
	g_state->state = state;

	switch (state) {
	case kDownloadStateNone:
	case kDownloadStateList:
		_statusText->setLabel(Common::U32String::format(_("Downloading %S list..."), _packname.c_str()));
		_cancelButton->setLabel(_("Cancel download"));
		_cancelButton->setCmd(kCleanupCmd);
		_closeButton->setVisible(true);

		g_state->totalSize = 0;
		g_state->fileHash.clear();
		break;

	case kDownloadStateListDownloaded:
		_statusText->setLabel(Common::U32String::format(_("Downloading %S list... %d entries"), _packname.c_str(), g_state->fileHash.size()));
		_cancelButton->setLabel(_("Cancel download"));
		_cancelButton->setCmd(kCleanupCmd);
		_closeButton->setVisible(true);
		break;

	case kDownloadStateListCalculated: {
			const char *sizeUnits;
			Common::String size = Common::getHumanReadableBytes(g_state->totalSize, sizeUnits);

			_statusText->setLabel(Common::U32String::format(_("Detected %d new packs, %s %S"), g_state->fileHash.size(), size.c_str(), _(sizeUnits).c_str()));

			_cancelButton->setLabel(_("Download"));
			_cancelButton->setCmd(kDownloadProceedCmd);

			_closeButton->setVisible(true);
			_closeButton->setLabel(_("Cancel"));
			_closeButton->setCmd(kCleanupCmd);
			_closeButton->setEnabled(true);
			break;
		}

	case kDownloadStateDownloading:
		_cancelButton->setLabel(_("Cancel download"));
		_cancelButton->setCmd(kCleanupCmd);

		_closeButton->setVisible(true);
		_closeButton->setLabel(_("Hide"));
		_closeButton->setCmd(kCloseCmd);
		_closeButton->setEnabled(true);
		_clearCacheButton->setEnabled(false);
		break;

	case kDownloadComplete: {
			const char *sizeUnits;
			Common::String size = Common::getHumanReadableBytes(g_state->totalSize, sizeUnits);
			_statusText->setLabel(Common::U32String::format(_("Download complete, downloaded %d packs, %s %S"), g_state->totalFiles, size.c_str(), _(sizeUnits).c_str()));
			_cancelButton->setVisible(false);
			_cancelButton->setLabel(_("Cancel download"));
			_cancelButton->setCmd(kCleanupCmd);

			_closeButton->setVisible(true);
			_closeButton->setLabel(_("Close"));
			_closeButton->setCmd(kCleanupCmd);
			_closeButton->setEnabled(true);
			_clearCacheButton->setEnabled(true);
			break;
		}
	}
}

void DownloadPacksDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kCleanupCmd:
		{
			g_state->session.abortRequest();
			delete g_state;
			g_state = nullptr;

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
		setState(kDownloadComplete);
		break;
	case kListDownloadFinishedCmd:
		setState(kDownloadStateListDownloaded);
		calculateList();
		break;
	case kDownloadProceedCmd:
		setState(kDownloadStateDownloading);
		g_state->proceedDownload();
		break;
	case kClearCacheCmd:
		_clearCacheButton->setEnabled(false);
		clearCache();
		break;
	default:
		Dialog::handleCommand(sender, cmd, data);
	}
}

void DownloadPacksDialog::handleTickle() {
	if (_close) {
		close();
		_close = false;
		return;
	}

	int32 progress = getDownloadingProgress();
	if (_progressBar->getValue() != progress) {
		refreshWidgets();
		g_gui.scheduleTopDialogRedraw();
	}

	Dialog::handleTickle();
}

void DownloadPacksDialog::reflowLayout() {
	Dialog::reflowLayout();
	refreshWidgets();
}

Common::U32String DownloadPacksDialog::getSizeLabelText() {
	const char *downloadedUnits, *totalUnits;
	Common::String downloaded = Common::getHumanReadableBytes(g_state->downloadedSize, downloadedUnits);
	Common::String total = Common::getHumanReadableBytes(g_state->totalSize, totalUnits);
	return Common::U32String::format(_("Downloaded %s %S / %s %S"), downloaded.c_str(), _(downloadedUnits).c_str(), total.c_str(), _(totalUnits).c_str());
}

Common::U32String DownloadPacksDialog::getSpeedLabelText() {
	const char *speedUnits;
	Common::String speed = Common::getHumanReadableBytes(getDownloadSpeed(), speedUnits);
	return Common::U32String::format(_("Download speed: %s %S/s"), speed.c_str(), _(speedUnits).c_str());
}

void DownloadPacksDialog::refreshWidgets() {
	uint32 progress = getDownloadingProgress();
	_percentLabel->setLabel(Common::String::format("%u %%", progress));
	_downloadSizeLabel->setLabel(getSizeLabelText());
	_downloadSpeedLabel->setLabel(getSpeedLabelText());
	_progressBar->setValue(progress);
}

void DownloadPacksDialog::setError(Common::U32String &msg) {
	_errorText->setLabel(msg);

	_cancelButton->setLabel(_("Close"));
	_cancelButton->setCmd(kCleanupCmd);
}

void DownloadPacksDialog::calculateList() {
	Common::String iconsPath = ConfMan.get("iconspath");
	if (iconsPath.empty()) {
		Common::U32String str(_("ERROR: No icons path set"));
		setError(str);
		return;
	}

	// Scan all files in iconspath and remove present and incomplete ones from the
	// donwloaded files list
	Common::FSDirectory *iconDir = new Common::FSDirectory(iconsPath);

	Common::ArchiveMemberList iconFiles;

	iconDir->listMatchingMembers(iconFiles, _packsglob);

	for (auto ic = iconFiles.begin(); ic != iconFiles.end(); ++ic) {
		Common::String fname = (*ic)->getName();
		Common::SeekableReadStream *str = (*ic)->createReadStream();
		uint32 size = str->size();
		delete str;

		if (g_state->fileHash.contains(fname) && size == g_state->fileHash[fname])
			g_state->fileHash.erase(fname);
	}

	delete iconDir;

	// Now calculate the size of the missing files
	g_state->totalSize = 0;
	for (auto f = g_state->fileHash.begin(); f != g_state->fileHash.end(); ++f) {
		g_state->totalSize += f->_value;
	}

	g_state->totalFiles = g_state->fileHash.size();

	if (g_state->totalSize == 0) {
		// I18N: String like "No new icon packs available"
		Common::U32String error(Common::U32String::format(_("No new %S available"), _packname.c_str()));
		_closeButton->setEnabled(false);
		setError(error);
		return;
	}

	setState(kDownloadStateListCalculated);
}

void DownloadPacksDialog::clearCache() {
	Common::String iconsPath = ConfMan.get("iconspath");
	if (iconsPath.empty()) {
		Common::U32String str(_("ERROR: No icons path set"));
		setError(str);
		return;
	}

	Common::FSDirectory *iconDir = new Common::FSDirectory(iconsPath);

	Common::ArchiveMemberList iconFiles;

	iconDir->listMatchingMembers(iconFiles, _packsglob);
	int totalSize = 0;

	for (auto ic = iconFiles.begin(); ic != iconFiles.end(); ++ic) {
		Common::String fname = (*ic)->getName();
		Common::SeekableReadStream *str = (*ic)->createReadStream();
		uint32 size = str->size();
		delete str;

		totalSize += size;
	}

	const char *sizeUnits;
	Common::String size = Common::getHumanReadableBytes(totalSize, sizeUnits);

	GUI::MessageDialog dialog(Common::U32String::format(_("You are about to remove %s %S of data, deleting all previously downloaded %S. Do you want to proceed?"), size.c_str(), _(sizeUnits).c_str(), _packname.c_str()), _("Proceed"), _("Cancel"));
	if (dialog.runModal() == ::GUI::kMessageOK) {

		// Build list of previously downloaded icon files
		for (auto ic = iconFiles.begin(); ic != iconFiles.end(); ++ic) {
			Common::String fname = (*ic)->getName();
			Common::FSNode fs(Common::Path(iconsPath).join(fname));
			Common::WriteStream *str = fs.createWriteStream();

			// Overwrite previously downloaded pack files with dummy data
			str->writeByte(0);
			str->finalize();
		}
		g_state->fileHash.clear();

		// Fetch current packs list file and re-trigger downloads
		g_state->downloadList();
		calculateList();
		_cancelButton->setVisible(true);
	} else {
		_clearCacheButton->setEnabled(true);
	}
}

} // End of namespace GUI
