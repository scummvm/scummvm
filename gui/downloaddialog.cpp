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

#include "gui/downloaddialog.h"
#include "backends/cloud/cloudmanager.h"
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
	kDownloadDialogButtonCmd = 'Dldb'
};

DownloadDialog::DownloadDialog(uint32 storageId, LauncherDialog *launcher) :
	Dialog("GlobalOptions_Cloud_DownloadDialog"), _launcher(launcher), _close(false) {
	_backgroundType = GUI::ThemeEngine::kDialogBackgroundPlain;

	_browser = new BrowserDialog(_("Select directory where to download game data"), true);
	_remoteBrowser = new RemoteBrowserDialog(_("Select directory with game data"));

	_remoteDirectoryLabel = new StaticTextWidget(this, "GlobalOptions_Cloud_DownloadDialog.RemoteDirectory", _("From: "));
	_localDirectoryLabel = new StaticTextWidget(this, "GlobalOptions_Cloud_DownloadDialog.LocalDirectory", _("To: "));
	uint32 progress = (uint32)(100 * CloudMan.getDownloadingProgress());
	_progressBar = new SliderWidget(this, "GlobalOptions_Cloud_DownloadDialog.ProgressBar");
	_progressBar->setMinValue(0);
	_progressBar->setMaxValue(100);
	_progressBar->setValue(progress);
	_progressBar->setEnabled(false);
	_percentLabel = new StaticTextWidget(this, "GlobalOptions_Cloud_DownloadDialog.PercentText", Common::String::format("%u %%", progress));
	_downloadSizeLabel = new StaticTextWidget(this, "GlobalOptions_Cloud_DownloadDialog.DownloadSize", Common::U32String());
	_downloadSpeedLabel = new StaticTextWidget(this, "GlobalOptions_Cloud_DownloadDialog.DownloadSpeed", Common::U32String());
	if (!g_gui.useLowResGUI())
		_cancelButton = new ButtonWidget(this, "GlobalOptions_Cloud_DownloadDialog.MainButton", _("Cancel download"), Common::U32String(), kDownloadDialogButtonCmd);
	else
		_cancelButton = new ButtonWidget(this, "GlobalOptions_Cloud_DownloadDialog.MainButton", _c("Cancel download", "lowres"), Common::U32String(), kDownloadDialogButtonCmd);

	_closeButton = new ButtonWidget(this, "GlobalOptions_Cloud_DownloadDialog.CloseButton", _("Hide"), Common::U32String(), kCloseCmd);
	refreshWidgets();

	CloudMan.setDownloadTarget(this);
}

DownloadDialog::~DownloadDialog() {
	CloudMan.setDownloadTarget(nullptr);
}

void DownloadDialog::open() {
	Dialog::open();
	CloudMan.setDownloadTarget(this);
	if (!CloudMan.isDownloading())
		if (!selectDirectories())
			close();
	reflowLayout();
	g_gui.scheduleTopDialogRedraw();
}

void DownloadDialog::close() {
	CloudMan.setDownloadTarget(nullptr);
	Dialog::close();
}

void DownloadDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kDownloadDialogButtonCmd:
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
	default:
		Dialog::handleCommand(sender, cmd, data);
	}
}

bool DownloadDialog::selectDirectories() {
	if (g_system->isConnectionLimited()) {
		MessageDialog alert(_("It looks like your connection is limited. "
			"Do you really want to download files with it?"), _("Yes"), _("No"));
		if (alert.runModal() != GUI::kMessageOK)
			return false;
	}

	// First, the user should select the remote directory to download
	if (_remoteBrowser->runModal() <= 0)
		return false;

	Cloud::StorageFile remoteDirectory = _remoteBrowser->getResult();

	// Now, the user should select the local directory to download into
	if (_browser->runModal() <= 0)
		return false;

	Common::FSNode dir(_browser->getResult());
	Common::FSList files;
	if (!dir.getChildren(files, Common::FSNode::kListAll)) {
		MessageDialog alert(_("ScummVM couldn't open the specified directory!"));
		alert.runModal();
		return false;
	}

	// Check that there is no file with the remote directory's name in the local one
	for (auto &file : files) {
		if (file.getName().equalsIgnoreCase(remoteDirectory.name())) {
			// If there is, ask user whether it's OK
			if (!file.isDirectory()) {
				GUI::MessageDialog alert(_("Cannot create a directory to download - the specified directory has a file with the same name."), _("OK"));
				alert.runModal();
				return false;
			}
			GUI::MessageDialog alert(
				Common::U32String::format(_("The \"%s\" already exists in the specified directory.\nDo you really want to download files into that directory?"), remoteDirectory.name().c_str()),
				_("Yes"),
				_("No")
				);
			if (alert.runModal() != GUI::kMessageOK)
				return false;
			break;
		}
	}

	// Make a local path
	Common::Path localPath = dir.getPath();
	localPath = localPath.appendComponent(remoteDirectory.name());

	CloudMan.startDownload(remoteDirectory.path(), localPath);
	CloudMan.setDownloadTarget(this);
	_localDirectory = localPath;
	return true;
}

void DownloadDialog::handleTickle() {
	if (_close) {
		if (_launcher)
			_launcher->doGameDetection(_localDirectory);
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

void DownloadDialog::reflowLayout() {
	Dialog::reflowLayout();
	refreshWidgets();
}

Common::U32String DownloadDialog::getSizeLabelText() {
	const char *downloadedUnits, *totalUnits;
	Common::String downloaded = Common::getHumanReadableBytes(CloudMan.getDownloadBytesNumber(), downloadedUnits);
	Common::String total = Common::getHumanReadableBytes(CloudMan.getDownloadTotalBytesNumber(), totalUnits);
	return Common::U32String::format(_("Downloaded %s %S / %s %S"), downloaded.c_str(), _(downloadedUnits).c_str(), total.c_str(), _(totalUnits).c_str());
}

Common::U32String DownloadDialog::getSpeedLabelText() {
	const char *speedUnits;
	Common::String speed = Common::getHumanReadableBytes(CloudMan.getDownloadSpeed(), speedUnits);
	return Common::U32String::format(_("Download speed: %s %S/s"), speed.c_str(), _(speedUnits).c_str());
}

void DownloadDialog::refreshWidgets() {
	_localDirectory = CloudMan.getDownloadLocalDirectory();
	_remoteDirectoryLabel->setLabel(_("From: ") + Common::U32String(CloudMan.getDownloadRemoteDirectory()));
	_localDirectoryLabel->setLabel(_("To: ") + Common::U32String(_localDirectory.toString(Common::Path::kNativeSeparator)));
	uint32 progress = (uint32)(100 * CloudMan.getDownloadingProgress());
	_percentLabel->setLabel(Common::String::format("%u %%", progress));
	_downloadSizeLabel->setLabel(getSizeLabelText());
	_downloadSpeedLabel->setLabel(getSpeedLabelText());
	_progressBar->setValue(progress);
}

} // End of namespace GUI
