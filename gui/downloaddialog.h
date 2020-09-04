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

#ifndef GUI_DOWNLOADDIALOG_H
#define GUI_DOWNLOADDIALOG_H

#include "gui/dialog.h"
#include "common/str.h"
#include "common/ustr.h"

namespace GUI {
class LauncherDialog;

class CommandSender;
class EditTextWidget;
class StaticTextWidget;
class ButtonWidget;
class SliderWidget;
class BrowserDialog;
class RemoteBrowserDialog;

enum DownloadProgress {
	kDownloadProgressCmd = 'DLPR',
	kDownloadEndedCmd = 'DLEN'
};

class DownloadDialog : public Dialog {
	LauncherDialog *_launcher;
	BrowserDialog *_browser;
	RemoteBrowserDialog *_remoteBrowser;

	StaticTextWidget *_remoteDirectoryLabel;
	StaticTextWidget *_localDirectoryLabel;
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
	bool selectDirectories();

public:
	DownloadDialog(uint32 storageId, LauncherDialog *launcher);
	~DownloadDialog() override;

	void open() override;
	void close() override;
	void handleCommand(CommandSender *sender, uint32 cmd, uint32 data) override;
	void handleTickle() override;
	void reflowLayout() override;
};

} // End of namespace GUI

#endif
