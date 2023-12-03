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
#include "gui/helpdialog.h"
#include "gui/gui-manager.h"
#include "gui/ThemeEval.h"
#include "gui/widget.h"
#include "gui/widgets/richtext.h"
#include "gui/widgets/tab.h"

namespace GUI {

HelpDialog::HelpDialog()
	: Dialog("HelpDialog") {

	TabWidget *tab = new TabWidget(this, "HelpDialog.TabWidget");

	tab->addTab(_("General"), "HelpDialog", false);
	Common::U32String helpText1 = _(
"## Where to get the games\n"
"\n"
"Visit [our Wiki](https://wiki.scummvm.org/index.php?title=Where_to_get_the_games) for a list of games and where to purchase them.\n"
"\n"
"Alternatively, download a selection of [freeware games](https://scummvm.org/games) and [demos](https://www.scummvm.org/demos/) from our website.\n"
"\n"
"For other (out of print) games, try Amazon, eBay, Game Trading Zone or other auction "
"sites. Beware of faulty games and illegal game copies.\n"
"\n"
"The ScummVM team does not recommend any individual supplier of games,"
"however the project does get a cut from every purchase on "
"[ZOOM-Platform](https://www.zoom-platform.com/?affiliate=c049516c-9c4c-42d6-8649-92ed870e8b53) "
"through affiliate referral links.\n"
"\n"
"Additionally, some games that are not available on ZOOM-Platform can be found on GOG.com.\n"
	);

	new RichTextWidget(tab, "HelpDialog.TabWidget", helpText1);


	// Now add backend-specific tabs if any
	const char * const *backendTabs = g_system->buildHelpDialogData();

	if (backendTabs) {
		while (*backendTabs) {
			Common::U32String tabName(_(*backendTabs++));
			const char *imagePack = nullptr;

			if (*backendTabs && **backendTabs)
				imagePack = *backendTabs;

			backendTabs++;

			Common::U32String tabText(_(*backendTabs++));

			tab->addTab(tabName, "HelpDialog", false);

			RichTextWidget *rt = new RichTextWidget(tab, "HelpDialog.TabWidget", tabText);

			if (imagePack)
				rt->setImageArchive(imagePack);
		}
	}

	 tab->setActiveTab(0);

	new ButtonWidget(this, "HelpDialog.Close", Common::U32String("Close"), Common::U32String(), kCloseCmd);
}

void HelpDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
		case kCloseCmd:
			close();
	}
}

} // End of namespace GUI
