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

#include "common/algorithm.h"
#include "common/bufferedstream.h"
#include "common/savefile.h"
#include "common/system.h"
#include "graphics/palette.h"
#include "graphics/scaler.h"
#include "graphics/thumbnail.h"
#include "common/translation.h"
#include "gui/widgets/list.h"
#include "gui/editrecorddialog.h"
#include "gui/EventRecorder.h"
#include "gui/message.h"
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
	kNextScreenshotCmd = 'NEXT',
	kPrevScreenshotCmd = 'PREV',
	kEditRecordCmd = 'EDIT'
};

RecorderDialog::RecorderDialog() : Dialog("RecorderDialog"), _list(nullptr), _currentScreenshot(0) {
	_firstScreenshotUpdate = false;
	_screenShotsCount = 0;
	_currentScreenshotText = nullptr;
	_authorText = nullptr;
	_notesText = nullptr;

	_backgroundType = ThemeEngine::kDialogBackgroundSpecial;

	new StaticTextWidget(this, "RecorderDialog.Title", _("Recorder or Playback Gameplay"));

	_list = new GUI::ListWidget(this, "RecorderDialog.List");
	_list->setNumberingMode(GUI::kListNumberingOff);

	_deleteButton = new GUI::ButtonWidget(this, "RecorderDialog.Delete", _("Delete"), Common::U32String(), kDeleteCmd);
	new GUI::ButtonWidget(this, "RecorderDialog.Cancel", _("Cancel"), Common::U32String(), kCloseCmd);
	new GUI::ButtonWidget(this, "RecorderDialog.Record", _("Record"), Common::U32String(), kRecordCmd);
	_playbackButton = new GUI::ButtonWidget(this, "RecorderDialog.Playback", _("Playback"), Common::U32String(), kPlaybackCmd);

	_editButton = new GUI::ButtonWidget(this, "RecorderDialog.Edit", _("Edit"), Common::U32String(), kEditRecordCmd);

	_editButton->setEnabled(false);
	_deleteButton->setEnabled(false);
	_playbackButton->setEnabled(false);

	_gfxWidget = new GUI::GraphicsWidget(this, 0, 0, 10, 10);
	_container = new GUI::ContainerWidget(this, "RecorderDialog.Thumbnail");
	if (g_gui.xmlEval()->getVar("Globals.RecorderDialog.ExtInfo.Visible") == 1) {
		new GUI::ButtonWidget(this,"RecorderDialog.NextScreenShotButton", Common::U32String("<"), Common::U32String(), kPrevScreenshotCmd);
		new GUI::ButtonWidget(this, "RecorderDialog.PreviousScreenShotButton", Common::U32String(">"), Common::U32String(), kNextScreenshotCmd);
		_currentScreenshotText = new StaticTextWidget(this, "RecorderDialog.currentScreenshot", Common::U32String("0/0"));
		_authorText = new StaticTextWidget(this, "RecorderDialog.Author", _("Author: "));
		_notesText = new StaticTextWidget(this, "RecorderDialog.Notes", _("Notes: "));
	}
	if (_gfxWidget)
		_gfxWidget->setGfx(nullptr);
}


void RecorderDialog::reflowLayout() {
	Dialog::reflowLayout();

	if (g_gui.xmlEval()->getVar("Globals.RecorderDialog.ExtInfo.Visible") == 1) {
		int16 x, y;
		int16 w, h;

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
}



void RecorderDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch(cmd) {
	case kEditRecordCmd: {
		if (_list->getSelected() >= 0) {
			EditRecordDialog editDlg(_fileHeaders[_list->getSelected()].author, _fileHeaders[_list->getSelected()].name, _fileHeaders[_list->getSelected()].notes);
			if (editDlg.runModal() != kOKCmd) {
				return;
			}
			_playbackFile.openRead(_fileHeaders[_list->getSelected()].fileName);
			_playbackFile.getHeader().author = editDlg.getAuthor();
			_playbackFile.getHeader().name = editDlg.getName();
			_playbackFile.getHeader().notes = editDlg.getNotes();
			_playbackFile.updateHeader();
			_fileHeaders[_list->getSelected()] = _playbackFile.getHeader();
			int oldselection = _list->getSelected();
			updateList();
			_list->setSelected(oldselection);
			updateSelection(true);
			_playbackFile.close();
		}
	}
		break;
	case kNextScreenshotCmd:
		++_currentScreenshot;
		updateScreenshot();
		break;
	case kPrevScreenshotCmd:
		--_currentScreenshot;
		updateScreenshot();
		break;
	case kDeleteCmd:
		if (_list->getSelected() >= 0) {
			MessageDialog alert(_("Do you really want to delete this record?"),
				_("Delete"), _("Cancel"));
			if (alert.runModal() == GUI::kMessageOK) {
				_playbackFile.close();
				g_eventRec.deleteRecord(_fileHeaders[_list->getSelected()].fileName);
				_list->setSelected(-1);
				updateList();
			}
		}
		break;
	case GUI::kListSelectionChangedCmd:
		updateSelection(true);
		break;
	case kRecordCmd: {
		TimeDate t;
		QualifiedGameDescriptor desc = EngineMan.findTarget(_target);
		g_system->getTimeAndDate(t);
		EditRecordDialog editDlg(_("Unknown Author"), Common::String::format("%.2d.%.2d.%.4d ", t.tm_mday, t.tm_mon, 1900 + t.tm_year) + desc.description, "");
		if (editDlg.runModal() != kOKCmd) {
			return;
		}
		_author = editDlg.getAuthor();
		_name = editDlg.getName();
		_notes = editDlg.getNotes();
		_filename = g_eventRec.generateRecordFileName(_target);
		setResult(kRecordDialogRecord);
		close();
		}
		break;
	case kPlaybackCmd:
		if (_list->getSelected() >= 0) {
			_filename = _fileHeaders[_list->getSelected()].fileName;
			setResult(kRecordDialogPlayback);
			close();
		}
		break;
	case kCloseCmd:
		setResult(kRecordDialogClose);
		// Fall through
	default:
		Dialog::handleCommand(sender, cmd, data);
	}
}

void RecorderDialog::updateList() {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::String pattern(_target + ".r??");
	Common::StringArray files = saveFileMan->listSavefiles(pattern);
	Common::PlaybackFile file;
	Common::U32StringArray namesList;
	_fileHeaders.clear();
	for (Common::StringArray::iterator i = files.begin(); i != files.end(); ++i) {
		if (file.openRead(*i)) {
			namesList.push_back(file.getHeader().name);
			_fileHeaders.push_back(file.getHeader());
		}
		file.close();
	}
	_list->setList(namesList);
	_list->markAsDirty();
}

int RecorderDialog::runModal(Common::String &target) {
	_target = target;
	updateList();
	return Dialog::runModal();
}

RecorderDialog::~RecorderDialog() {
}

void RecorderDialog::updateSelection(bool redraw) {
	if (_list->getSelected() >= 0) {
		_editButton->setEnabled(true);
		_deleteButton->setEnabled(true);
		_playbackButton->setEnabled(true);
	}

	if (g_gui.xmlEval()->getVar("Globals.RecorderDialog.ExtInfo.Visible") != 1)
		return;

	_gfxWidget->setGfx(-1, -1, 0, 0, 0);
	_screenShotsCount = 0;
	_currentScreenshot = 0;
	updateScreenShotsText();
	if (_list->getSelected() >= 0) {
		_authorText->setLabel(_("Author: ") + Common::U32String(_fileHeaders[_list->getSelected()].author));
		_notesText->setLabel(_("Notes: ") + Common::U32String(_fileHeaders[_list->getSelected()].notes));

		_firstScreenshotUpdate = true;
		updateScreenshot();
		if ((_screenShotsCount) > 0) {
			_currentScreenshot = 1;
		}
		updateScreenshot();
	} else {
		_authorText->setLabel(_("Author: "));
		_notesText->setLabel(_("Notes: "));
		_screenShotsCount = -1;
		_currentScreenshot = 0;
		_gfxWidget->setGfx(-1, -1, 0, 0, 0);
		_gfxWidget->markAsDirty();
		updateScreenShotsText();
	}
}

void RecorderDialog::updateScreenshot() {
	if (_list->getSelected() == -1) {
		return;
	}
	if (_currentScreenshot < 1) {
		_currentScreenshot = _screenShotsCount;
	}
	if (_currentScreenshot > _screenShotsCount) {
		_currentScreenshot = 1;
	}
	if (_firstScreenshotUpdate) {
		_playbackFile.openRead(_fileHeaders[_list->getSelected()].fileName);
		_screenShotsCount = _playbackFile.getScreensCount();
		_firstScreenshotUpdate = false;
	}
	Graphics::Surface *srcsf = _playbackFile.getScreenShot(_currentScreenshot);
	if (srcsf != nullptr) {
		Graphics::Surface *destsf = Graphics::scale(*srcsf, _gfxWidget->getWidth(), _gfxWidget->getHeight());
		_gfxWidget->setGfx(destsf);
		updateScreenShotsText();
		delete destsf;
		delete srcsf;
	} else {
		_gfxWidget->setGfx(-1, -1, 0, 0, 0);
	}
	_gfxWidget->markAsDirty();
}

void RecorderDialog::updateScreenShotsText() {
	if (_screenShotsCount == -1) {
		_currentScreenshotText->setLabel(Common::String::format("%d / ?", _currentScreenshot));
	} else {
		_currentScreenshotText->setLabel(Common::String::format("%d / %d", _currentScreenshot, _screenShotsCount));
	}
}

} // End of namespace GUI
