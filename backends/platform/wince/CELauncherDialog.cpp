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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "backends/platform/wince/wince-sdl.h"

#include "CELauncherDialog.h"

#include "engines/metaengine.h"

#include "gui/gui-manager.h"
#include "gui/widget.h"
#include "gui/browser.h"
#include "gui/message.h"
#include "gui/ThemeEval.h"

#include "common/config-manager.h"

#include "common/translation.h"

using namespace GUI;
using namespace Common;

class CEAboutDialog : public Dialog {
public:
	CEAboutDialog()
		: Dialog(10, 60, 300, 77) {
		char tempo[100];

		// FIXME: Fingolfin asks: why is there a FIXME here? Please either clarify what
		// needs fixing, or remove it!
		const int buttonWidth = g_gui.xmlEval()->getVar("Globals.Button.Width", 0);
		const int buttonHeight = g_gui.xmlEval()->getVar("Globals.Button.Height", 0);
		new ButtonWidget(this, (_w - buttonWidth) / 2, 45, buttonWidth, buttonHeight, _("OK"), 0, kCloseCmd, '\r'); // Close dialog - FIXME

		Common::String videoDriver(_("Using SDL driver "));
		SDL_VideoDriverName(tempo, sizeof(tempo));
		videoDriver += tempo;
		new StaticTextWidget(this, 0, 10, _w, kLineHeight, videoDriver, Graphics::kTextAlignCenter);
		Common::String displayInfos(_("Display "));
		sprintf(tempo, "%dx%d (real %dx%d)", GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), OSystem_WINCE3::getScreenWidth(), OSystem_WINCE3::getScreenHeight());
		displayInfos += tempo;
		new StaticTextWidget(this, 0, 20, _w, kLineHeight, displayInfos, Graphics::kTextAlignCenter);
	}
};

CELauncherDialog::CELauncherDialog() : GUI::LauncherDialog() {
}

void CELauncherDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	LauncherDialog::handleCommand(sender, cmd, data);
	if (cmd == 'ABOU') {
		CEAboutDialog about;
		about.runModal();
	}
}

void CELauncherDialog::automaticScanDirectory(const Common::FSNode &node) {
	// First check if we have a recognized game in the current directory
	Common::FSList files;
	node.getChildren(files, Common::FSNode::kListFilesOnly);
	// detect
	GameList candidates(EngineMan.detectGames(files));
	// insert
	if (candidates.size() >= 1) {
		GameDescriptor result = candidates[0];
		result["path"] = node.getPath();
		addGameToConf(result);
	}
	// Then recurse on the subdirectories
	Common::FSList dirs;
	node.getChildren(dirs, Common::FSNode::kListDirectoriesOnly);
	for (Common::FSList::const_iterator currentDir = dirs.begin(); currentDir != dirs.end(); ++currentDir)
		automaticScanDirectory(*currentDir);

}

/* FIXME: We do this here, replicating code of the launcher, because findfirst/next
 * returns some illegal paths atm.
 */
void CELauncherDialog::addGame() {
	MessageDialog alert(_("Do you want to perform an automatic scan ?"), _("Yes"), _("No"));
	if (alert.runModal() == kMessageOK && _browser->runModal() > 0) {
		// Clear existing domains
		ConfigManager::DomainMap &domains = (ConfigManager::DomainMap &)ConfMan.getGameDomains();
		domains.clear();
		ConfMan.flushToDisk();
		automaticScanDirectory(_browser->getResult());
		ConfMan.flushToDisk();
		updateListing();
		draw();
	} else
		GUILauncherDialog::addGame();
}
