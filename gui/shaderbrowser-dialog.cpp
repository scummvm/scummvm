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

#include "gui/shaderbrowser-dialog.h"

#include "common/system.h"
#include "common/algorithm.h"
#include "common/str-array.h"
#include "common/zip-set.h"

#include "common/translation.h"

#include "gui/ThemeEval.h"
#include "gui/widgets/list.h"
#include "gui/browser.h"
#include "gui/gui-manager.h"
#include "gui/message.h"

namespace GUI {

enum {
	kChooseCmd = 'Chos',
	kChooseFileCmd = 'File',
	kSearchCmd = 'SRCH',
	kListSearchCmd = 'LSSR',
	kSearchClearCmd = 'SRCL',
};

const char *const kFileMask = "*.glslp";
const char *const kFileExt  = "glslp";

ShaderBrowserDialog::ShaderBrowserDialog(const Common::Path &initialSelection) : Dialog("ShaderBrowser") {

	new StaticTextWidget(this, "ShaderBrowser.Headline", _("Choose shader from the list below (or pick a file instead)"));

	_fileName = new PathWidget(this, "ShaderBrowser.Filename", initialSelection);

	// Search box
	_searchDesc = nullptr;
#ifndef DISABLE_FANCY_THEMES
	_searchPic = nullptr;
	if (g_gui.xmlEval()->getVar("Globals.ShowSearchPic") == 1 && g_gui.theme()->supportsImages()) {
		_searchPic = new GraphicsWidget(this, "ShaderBrowser.SearchPic", _("Search in game list"));
		_searchPic->setGfxFromTheme(ThemeEngine::kImageSearch);
	} else
#endif
		_searchDesc = new StaticTextWidget(this, "ShaderBrowser.SearchDesc", _("Search:"));

	_searchWidget = new EditTextWidget(this, "ShaderBrowser.Search", _search, Common::U32String(), kSearchCmd);
	_searchClearButton = addClearButton(this, "ShaderBrowser.SearchClearButton", kSearchClearCmd);

	new ButtonWidget(this, "ShaderBrowser.BrowseFile", _("Pick file instead..."), _("Pick shader from file system"), kChooseFileCmd);

	// Add file list
	_fileList = new ListWidget(this, "ShaderBrowser.List");
	_fileList->setNumberingMode(kListNumberingOff);
	_fileList->setEditable(false);

	_backgroundType = GUI::ThemeEngine::kDialogBackgroundPlain;

	// Buttons
	new ButtonWidget(this, "ShaderBrowser.Cancel", _("Cancel"), Common::U32String(), kCloseCmd);
	new ButtonWidget(this, "ShaderBrowser.Choose", _("Choose"), Common::U32String(), kChooseCmd);
}

void ShaderBrowserDialog::open() {
	// Call super implementation
	Dialog::open();

	Common::generateZipSet(_shaderSet, "shaders.dat", "shaders*.dat");

	updateListing();
}

void ShaderBrowserDialog::reflowLayout() {
#ifndef DISABLE_FANCY_THEMES
	if (g_gui.xmlEval()->getVar("Globals.ShowSearchPic") == 1 && g_gui.theme()->supportsImages()) {
		if (!_searchPic)
			_searchPic = new GraphicsWidget(this, "ShaderBrowser.SearchPic", _("Search in game list"));
		_searchPic->setGfxFromTheme(ThemeEngine::kImageSearch);

		if (_searchDesc) {
			removeWidget(_searchDesc);
			g_gui.addToTrash(_searchDesc, this);
			_searchDesc = nullptr;
		}
	} else {
		if (!_searchDesc)
			_searchDesc = new StaticTextWidget(this, "ShaderBrowser.SearchDesc", _("Search:"));

		if (_searchPic) {
			removeWidget(_searchPic);
			g_gui.addToTrash(_searchPic, this);
			_searchPic = nullptr;
		}
	}
#endif

	removeWidget(_searchClearButton);
	g_gui.addToTrash(_searchClearButton, this);
	_searchClearButton = addClearButton(this, "ShaderBrowser.SearchClearButton", kSearchClearCmd);

	Dialog::reflowLayout();
}

void ShaderBrowserDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kChooseCmd:
		if (_fileName->getLabel().empty())
			break;

		normalizeFileName();

		setResult(1);
		close();
		break;
	case kChooseFileCmd: {
		BrowserDialog browser(_("Select shader"), false);
		if (browser.runModal() > 0) {
			// User made his choice...
			Common::FSNode file(browser.getResult());
			_fileName->setLabel(file.getPath());

			setResult(1);
			close();
		}
		break;
	}
	case kSearchCmd:
		// Update the active search filter.
		_fileList->setFilter(_searchWidget->getEditString());
		break;
	case kSearchClearCmd:
		// Reset the active search filter, thus showing all games again
		_searchWidget->setEditString(Common::U32String());
		_fileList->setFilter(Common::U32String());
		break;
	case kListSelectionChangedCmd:
		_fileName->setLabel(Common::Path(_fileList->getList().operator[](_fileList->getSelected()).encode()));
		break;
	case kListItemActivatedCmd:
	case kListItemDoubleClickedCmd:
		normalizeFileName();
		setResult(1);
		close();
		break;
	default:
		Dialog::handleCommand(sender, cmd, data);
	}
}

void ShaderBrowserDialog::normalizeFileName() {
	Common::Path filename = _fileName->getLabel();

	Common::String basename = filename.baseName();
	if (basename.matchString(kFileMask, true))
		return;

	filename.appendInPlace(".").appendInPlace(kFileExt);
	_fileName->setLabel(filename);
}


void ShaderBrowserDialog::updateListing() {
	Common::U32StringArray list;
	Common::ArchiveMemberList files;

	_shaderSet.listMatchingMembers(files, kFileMask, true);

	Common::sort(files.begin(), files.end(), Common::ArchiveMemberListComparator());

	for (auto &file : files) {
		list.push_back(file->getPathInArchive().toString().decode());
	}

	_fileList->setList(list);
	_fileList->scrollTo(0);

	// Finally, redraw
	g_gui.scheduleTopDialogRedraw();
}

} // End of namespace GUI
