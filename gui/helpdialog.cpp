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

	_tab = new TabWidget(this, "HelpDialog.TabWidget");

static const char * const helpTabs[] = {
_s("General"),
"",
_s(
"## ScummVM at a Glance\n"
"\n"
"ScummVM is a modern reimplementation of various game engines. Once you transfer the original game data to your device, it endeavors to use it to faithfully recreate the original gaming experience. \n"
"\n"
"ScummVM isn't your typical emulator of DOS, Windows, or some console. Rather than a one-size-fits-all approach, it takes a meticulous route, implementing the precise game logic for each specific title or engine it supports. ScummVM will not work with game engines it does not support.\n"
"\n"
"ScummVM is developed by a team of volunteers and is free software. We lack an extensive testing team, possess only a limited range of devices, and cannot always address every request. We also do not run advertisements or sell you anything. Please be mindful of this when you submit a complaint or a bug report.\n"
"\n"
"## Where to get the games\n"
"\n"
"Visit [our Wiki](https://wiki.scummvm.org/index.php?title=Where_to_get_the_games) for a detailed list of supported games and where to purchase them.\n"
"\n"
"Alternatively, you can download a variety of [freeware games](https://scummvm.org/games) and [demos](https://www.scummvm.org/demos/) directly from our website.\n"
"\n"
"The ScummVM team does not endorse any specific game supplier. "
"However, the project receives a commission from every purchase made on "
"[ZOOM-Platform](https://www.zoom-platform.com/?affiliate=c049516c-9c4c-42d6-8649-92ed870e8b53) "
"through affiliate referral links.\n"
"\n"
"Additionally, games not available on ZOOM-Platform can be found on other suppliers such as GOG.com and Steam.\n"
"\n"
"For other (out-of-print) games, consider checking platforms like Amazon, eBay, Game Trading Zone, or other auction "
"sites. Be cautious of faulty games and illegal game copies.\n"
),

#ifdef USE_CLOUD

_s("Cloud"),
"helpdialog.zip",
_s(
"## Connecting a cloud service - Quick mode\n"
"\n"
"1. From the Launcher, select **Global Options** and then select the **Cloud** tab.\n"
"\n"
"2. Select your preferred cloud storage service from the **Active storage** dropdown, then select **Connect**.\n"
"\n	"
"  ![Select cloud service](choose_storage.png \"Select cloud service\"){w=70%}\n"
"\n"
"3. Select **Quick mode**.\n"
"\n	"
"  ![Quick mode](quick_mode.png \"Quick mode\"){w=70%}\n"
"\n"
"4. Select **Run server** and then select **Next** \n"
"\n	"
"  ![Run server](run_server.png \"Run server\"){w=70%}\n"
"\n"
"  ![Next step](server_next.png \"Next step\"){w=70%}\n"
"\n"
"5. Open the link.\n"
"\n	"
"  ![Open the link](open_link.png \"Open the link\"){w=70%}\n"
"\n"
"6. In the browser window that opens, select the cloud service to connect. \n"
"\n	"
"  ![Choose the cloud service](cloud_browser.png \"Choose the cloud service\"){w=70%}\n"
"\n"
"7. Sign in to the chosen cloud service. Once completed, return to ScummVM.\n"
"\n"
"8. On the success screen, select **Finish** to exit. \n"
"\n	"
"  ![Success](cloud_success.png \"Success\"){w=70%}\n"
"9. Back on the main Cloud tab, select **Enable storage**.\n"
"\n	"
"  ![Enable storage](enable_storage.png \"Enable storage\"){w=70%}\n"
"\n"
"10. You're ready to go! Use the cloud functionality to sync saved games or game files between your devices.\n"
"\n	"
"  ![Cloud functionality](cloud_functions.png \"Cloud functionality\"){w=70%}\n"
"\n"
"   For more information, including how to use the manual connection wizard, see our [Cloud documentation](https://docs.scummvm.org/en/latest/use_scummvm/connect_cloud.html) "
),

#endif

0,
	};


	addTabs(helpTabs);
	// Now add backend-specific tabs if any
	const char * const *backendTabs = g_system->buildHelpDialogData();

	if (backendTabs)
		addTabs(backendTabs);

	_tab->setActiveTab(0);

	new ButtonWidget(this, "HelpDialog.Close", Common::U32String("Close"), Common::U32String(), kCloseCmd);
}

void HelpDialog::addTabs(const char * const *tabData) {
	while (*tabData) {
		Common::U32String tabName(*tabData++);
		const char *imagePack = nullptr;

		if (*tabData && **tabData)
			imagePack = *tabData;

		tabData++;

		Common::U32String tabText(*tabData++);

		_tab->addTab(tabName, "HelpContentDialog", false);

		RichTextWidget *rt = new RichTextWidget(_tab, "HelpContentDialog.RichTextWidget", tabText);

		if (imagePack)
			rt->setImageArchive(imagePack);
	}
}

void HelpDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
		case kCloseCmd:
			close();
	}
}

} // End of namespace GUI
