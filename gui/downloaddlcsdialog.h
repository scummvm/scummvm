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

#ifndef DOWNLOAD_DLCS_DIALOG_H
#define DOWNLOAD_DLCS_DIALOG_H

#include "gui/dialog.h"
#include "gui/widgets/list.h"

namespace GUI {

enum {
	kCancelSelectedCmd = 'CANS',
};

class DownloadDLCsDialog : public Dialog {
public:
	DownloadDLCsDialog();

	void handleCommand(CommandSender *sender, uint32 cmd, uint32 data) override;
	void handleTickle() override;
	uint32 getDownloadingProgress();

private:
	StaticTextWidget *_currentDownloadLabel;
	StaticTextWidget *_errorLabel;
	StaticTextWidget *_downloadedSizeLabel;
	StaticTextWidget *_percentLabel;
	SliderWidget *_progressBar;
	ListWidget *_pendingDownloadsList;
	ButtonWidget *_cancelButton;

	int _selectedIdx = -1;

	Common::U32String getSizeLabelText();

	void refreshWidgets();
};

} // End of namespace GUI

#endif
