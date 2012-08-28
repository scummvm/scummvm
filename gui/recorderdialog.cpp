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
 */

#include "common/algorithm.h"
#include "common/bufferedstream.h"
#include "common/EventRecorder.h"
#include "common/savefile.h"
#include "common/system.h"
#include "graphics/colormasks.h"
#include "graphics/palette.h"
#include "graphics/scaler.h"
#include "graphics/thumbnail.h"
#include "common/translation.h"
#include "gui/widgets/list.h"
#include "gui/message.h"
#include "gui/saveload.h"
#include "common/system.h"
#include "gui/ThemeEval.h"
#include "gui/gui-manager.h"
#include "recorderdialog.h"

#define MAX_RECORDS_NAMES 0xFF

namespace GUI {

enum {
	kRecordCmd = 'RCRD',
	kPlaybackCmd = 'PBCK',
	kDeleteCmd = 'DEL ',
	kSwitchScreenshotCmd = 'SCRN'
};

RecorderDialog::RecorderDialog() : Dialog("RecorderDialog"), _list(0), _currentScreenshot(0), _playbackFile(0) {
	_backgroundType = ThemeEngine::kDialogBackgroundSpecial;
	ButtonWidget *recordButton;
	ButtonWidget *playbackButton;
	_list = new GUI::ListWidget(this, "RecorderDialog.List");
	_list->setNumberingMode(GUI::kListNumberingOff);
	new GUI::ButtonWidget(this, "RecorderDialog.Delete", _("Delete"), 0, kDeleteCmd);
	new GUI::ButtonWidget(this, "RecorderDialog.Cancel", _("Cancel"), 0, kCloseCmd);
	recordButton = new GUI::ButtonWidget(this, "RecorderDialog.Record", _("Record"), 0, kRecordCmd);
	playbackButton = new GUI::ButtonWidget(this, "RecorderDialog.Playback", _("Playback"), 0, kPlaybackCmd);
	_gfxWidget = new GUI::GraphicsWidget(this, 0, 0, 10, 10);
	_container = new GUI::ContainerWidget(this, 0, 0, 10, 10);
	if (_gfxWidget)
		_gfxWidget->setGfx(0);
}


void RecorderDialog::reflowLayout() {
	if (g_gui.xmlEval()->getVar("Globals.RecorderDialog.ExtInfo.Visible") == 1) {
		int16 x, y;
		uint16 w, h;

		if (!g_gui.xmlEval()->getWidgetData("RecorderDialog.Thumbnail", x, y, w, h)) {
			error("Error when loading position data for Recorder Thumbnails");
		}

		int thumbW = kThumbnailWidth;
		int thumbH = kThumbnailHeight2;
		int thumbX = x + (w >> 1) - (thumbW >> 1);
		int thumbY = y + kLineHeight;

		_container->resize(x, y, w, h);
		_gfxWidget->resize(thumbX, thumbY, thumbW, thumbH);

		_container->setVisible(true);
		_gfxWidget->setVisible(true);
		updateSelection(false);
	} else {
		_container->setVisible(false);
		_gfxWidget->setVisible(false);
	}
	Dialog::reflowLayout();
}



void RecorderDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch(cmd) {
	case kDeleteCmd:
		if (_list->getSelected() >= 0) {
			MessageDialog alert(_("Do you really want to delete this record?"),
				_("Delete"), _("Cancel"));
			if (alert.runModal() == GUI::kMessageOK) {
				g_eventRec.deleteRecord(_list->getSelectedString());
				_list->setSelected(-1);
				updateList();
			}
		}
		break;
	case kRecordCmd:
		_filename = generateRecordFileName();
		setResult(kRecordDialogRecord);
		close();
		break;
	case kPlaybackCmd:
		if (_list->getSelected() >= 0) {
			_filename = _list->getSelectedString();
			setResult(kRecordDialogPlayback);
			close();
		}
		break;
	case kCloseCmd:
		setResult(kRecordDialogClose);
	default:
		Dialog::handleCommand(sender, cmd, data);
		}
	}

void RecorderDialog::updateList() {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::String pattern(_target+".r??");
	Common::StringArray files = saveFileMan->listSavefiles(pattern);
	_list->setList(files);
	_list->draw();
}

int RecorderDialog::runModal(Common::String &target) {
	_target = target;
	updateList();
	return Dialog::runModal();
}

RecorderDialog::~RecorderDialog() {
	if (_playbackFile != NULL) {
		delete _playbackFile;
		_playbackFile = NULL;
	}
}

void RecorderDialog::updateSelection(bool redraw) {
	_gfxWidget->setGfx(-1, -1, 0, 0, 0);
}

Common::String RecorderDialog::generateRecordFileName() {
	ConfMan.getActiveDomainName();
	GUI::ListWidget::StringArray recordsList = _list->getList();
	for (int i = 0; i < MAX_RECORDS_NAMES; ++i) {
		Common::String recordName = Common::String::format("%s.r%02x", _target.c_str(), i);
		if (Common::find(_list->getList().begin(), _list->getList().end(), recordName) != _list->getList().end()) {
			continue;
		}
		return recordName;
	}
	return "";
}

Graphics::Surface *RecorderDialog::getScreenShot(int number) {
	uint32 id = _playbackFile->readUint32LE();
	_playbackFile->skip(4);
	if(id != MKTAG('P','B','C','K')) {
		return NULL;
	}
	int screenCount = 0;
	while (skipToNextScreenshot()) {
		if (screenCount == number) {
			screenCount++;
			_playbackFile->seek(-4, SEEK_CUR);
			return Graphics::loadThumbnail(*_playbackFile);
		} else {
			uint32 size = _playbackFile->readUint32BE();
			_playbackFile->skip(size-8);
			screenCount++;
		}
	}
	return NULL;
}

bool RecorderDialog::skipToNextScreenshot() {
	while (true) {
		uint32 id = _playbackFile->readUint32LE();
		if (_playbackFile->eos()) {
			break;
		}
		if (id == MKTAG('B','M','H','T')) {
			return true;
		}
		else {
			uint32 size = _playbackFile->readUint32LE();
			_playbackFile->skip(size);
		}
	}
	return false;
}


} // End of namespace GUI