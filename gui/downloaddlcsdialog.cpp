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

#include "common/translation.h"

#include "gui/downloaddlcsdialog.h"
#include "gui/widget.h"
#include "gui/gui-manager.h"
#include "gui/widgets/list.h"

#include "backends/dlc/dlcmanager.h"
#include "backends/dlc/dlcdesc.h"

enum {
	kCancelSelectedCmd = 'CANS',
};

namespace GUI {

DownloadDLCsDialog::DownloadDLCsDialog()
	: Dialog("DLCDownloads") {

	new StaticTextWidget(this, "DLCDownloads.Headline", _("Currently Downloading Games"));

	_currentDownloadLabel = new StaticTextWidget(this, "DLCDownloads.CurrentDownload", Common::U32String());
	_errorLabel = new StaticTextWidget(this, "DLCDownloads.ErrorText", Common::U32String(DLCMan._errorText));

	_progressBar = new SliderWidget(this, "DLCDownloads.ProgressBar");
	_progressBar->setMinValue(0);
	_progressBar->setMaxValue(100);
	_progressBar->setValue(0);
	_progressBar->setEnabled(false);

	_percentLabel = new StaticTextWidget(this, "DLCDownloads.PercentText", Common::String());
	_downloadedSizeLabel = new StaticTextWidget(this, "DLCDownloads.DownloadedSize", Common::U32String());

	new StaticTextWidget(this, "DLCDownloads.Pending", _("Pending Downloads"));
	_pendingDownloadsList = new ListWidget(this, "DLCDownloads.List");
	_pendingDownloadsList->setNumberingMode(kListNumberingOff);
	_pendingDownloadsList->setEditable(false);

	new ButtonWidget(this, "DLCDownloads.Back", _("Back"), Common::U32String(), kCloseCmd);
	_cancelButton = new ButtonWidget(this, "DLCDownloads.Cancel", _("Cancel"), Common::U32String(), kCancelSelectedCmd);

	refreshWidgets();
}

Common::U32String DownloadDLCsDialog::getSizeLabelText() {
	const char *downloadedUnits, *totalUnits;
	Common::String downloaded = Common::getHumanReadableBytes(DLCMan._currentDownloadedSize, downloadedUnits);
	Common::String total = Common::getHumanReadableBytes(DLCMan._queuedDownloadTasks.front()->size, totalUnits);
	return Common::U32String::format(_("Downloaded %s %S / %s %S"), downloaded.c_str(), _(downloadedUnits).c_str(), total.c_str(), _(totalUnits).c_str());
}

uint32 DownloadDLCsDialog::getDownloadingProgress() {
	if (DLCMan._queuedDownloadTasks.empty()) {
		// no DLC is currently downloading
		return 0;
	}
	uint32 progress = (uint32)(100 * ((double)DLCMan._currentDownloadedSize / (double)DLCMan._queuedDownloadTasks.front()->size));
	return progress;
}

void DownloadDLCsDialog::refreshWidgets() {
	Common::U32StringArray pendingList;
	if (DLCMan._queuedDownloadTasks.empty()) {
		// no DLC is currently downloading
		_currentDownloadLabel->setLabel(Common::U32String("No downloads in progress"));
		_downloadedSizeLabel->setLabel(Common::U32String());
		_pendingDownloadsList->setList(pendingList);
	} else {
		_currentDownloadLabel->setLabel(DLCMan._queuedDownloadTasks.front()->name);
		_downloadedSizeLabel->setLabel(getSizeLabelText());

		for (const auto &it : DLCMan._dlcsInProgress) {
			if (it->state == DLC::DLCDesc::kInProgress) {
				pendingList.push_back(it->name);
			} else {
				pendingList.push_back("[Cancelled] " + it->name);
			}
		}
		_pendingDownloadsList->setList(pendingList);
		if (_progressBar->getValue() >= 100) {
			// if a game is downloaded i.e. the first item is removed from _dlcsInProgress
			_selectedIdx--;
		}
		_pendingDownloadsList->setSelected(_selectedIdx);
	}

	uint32 progress = getDownloadingProgress();
	_percentLabel->setLabel(Common::String::format("%u %%", progress));
	_progressBar->setValue(progress);

	g_gui.scheduleTopDialogRedraw();
}

void DownloadDLCsDialog::handleTickle() {
	int32 progress = getDownloadingProgress();

	if (_progressBar->getValue() != progress) {
		_selectedIdx = _pendingDownloadsList->getSelected();
		refreshWidgets();
	}

	// enable cancel button only when a list item is selected
	if (_pendingDownloadsList->getSelected() == -1) {
		if (_cancelButton->isEnabled()) {
			_cancelButton->setEnabled(false);
			g_gui.scheduleTopDialogRedraw();
		}
	} else {
		if (!_cancelButton->isEnabled()) {
			_cancelButton->setEnabled(true);
			g_gui.scheduleTopDialogRedraw();
		}
	}

	if (_errorLabel->getLabel() != DLCMan._errorText) {
		_errorLabel->setLabel(DLCMan._errorText);
		g_gui.scheduleTopDialogRedraw();
	}

	Dialog::handleTickle();
}

void DownloadDLCsDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kCancelSelectedCmd: {
		uint32 idx = DLCMan._dlcsInProgress[_pendingDownloadsList->getSelected()]->idx;
		DLCMan.cancelDownload(idx);
		}
		break;
	default:
		Dialog::handleCommand(sender, cmd, data);
	}
}

} // End of namespace GUI
