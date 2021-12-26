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

#ifndef GUI_DOWNLOADICONSDIALOG_H
#define GUI_DOWNLOADICONSDIALOG_H

#include "gui/dialog.h"
#include "common/str.h"
#include "common/ustr.h"

namespace Networking {
class Session;
}

namespace GUI {
class CommandSender;
class StaticTextWidget;
class ButtonWidget;
class SliderWidget;

enum IconProcessState {
	kDownloadStateNone,
	kDownloadStateList,
	kDownloadStateListDownloaded,
	kDownloadStateListCalculated,
	kDownloadStateDownloading,
	kDownloadComplete
};

class DownloadIconsDialog : public Dialog, public CommandSender {
	StaticTextWidget *_statusText;
	StaticTextWidget *_errorText;
	StaticTextWidget *_percentLabel;
	StaticTextWidget *_downloadSizeLabel;
	StaticTextWidget *_downloadSpeedLabel;
	SliderWidget *_progressBar;
	ButtonWidget *_cancelButton;
	ButtonWidget *_closeButton;

	Common::String _localDirectory;
	bool _close;

	Common::U32String getSizeLabelText();
	Common::U32String getSpeedLabelText();

	void refreshWidgets();

public:
	DownloadIconsDialog();
	~DownloadIconsDialog() override;

	void open() override;
	void close() override;
	void handleCommand(CommandSender *sender, uint32 cmd, uint32 data) override;
	void handleTickle() override;
	void reflowLayout() override;

	void downloadListCallback(Networking::DataResponse response);
	void downloadFileCallback(Networking::DataResponse response);
	void errorCallback(Networking::ErrorResponse error);

	void setError(Common::U32String &msg);

private:
	void downloadList();
	void calculateList();
	void proceedDownload();
	void setState(IconProcessState state);
	bool takeOneFile();
};

} // End of namespace GUI

#endif
