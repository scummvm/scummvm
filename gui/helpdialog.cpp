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
	: Dialog(30, 20, 260, 124) {

	const int screenW = g_system->getOverlayWidth();
	const int screenH = g_system->getOverlayHeight();

	int buttonWidth = g_gui.xmlEval()->getVar("Globals.Button.Width", 0);
	int buttonHeight = g_gui.xmlEval()->getVar("Globals.Button.Height", 0);

	_w = screenW * 8 / 10;
	_h = screenH * 8 / 10;

	// Center the dialog
	_x = (screenW - _w) / 2;
	_y = (screenH - _h) / 2;


	int tabHeight = _h - (buttonHeight + 10) * 5 / 2;

	TabWidget *tab = new TabWidget(this, 10, 10, _w - 10, tabHeight);

	tab->addTab(_("General"), "GlobalOptions_Graphics", false);
	Common::U32String helpText1 = _(
"#### Where to get the games\n"
"\n"
"Many games supported by ScummVM can still be bought from companies at the links below. Not all games on this list are supported by ScummVM, please check the compatibility page beforehand.\n"
"\n"
"Several games have been released for free legal download by their respective copyright holders. You can download them from [our website](https://scummvm.org/games).\n"
"\n"
"For other (out of print) games try Amazon, eBay, Game Trading Zone or other auction sites but beware of faulty games (e.g., scratched discs) and illegal game copies (e.g., from Butterfly Media).\n"
"\n"
"The ScummVM team does not recommend any individual supplier of games and this list is for reference purposes only. The ScummVM project does get a cut from every purchase on [GOG.com](https://www.gog.com/?pp=22d200f8670dbdb3e253a90eee5098477c95c23d) and [ZOOM-Platform](https://www.zoom-platform.com/?affiliate=c049516c-9c4c-42d6-8649-92ed870e8b53) through one of the links with the added affiliate referrer though.\n"
	);

	new RichTextWidget(tab, 10, 10, _w - 10, tabHeight - buttonHeight - 10, helpText1);

	tab->addTab(_("Controls"), "GlobalOptions_Graphics", false);

	Common::U32String helpText2 = _(
"#### Touch controls\n"
"\n"
"The touch control scheme can be configured in the global settings. From the Launcher, go to **Options > Backend > Choose the preferred touch mode**.\n"
"It's possible to configure the touch mode for three situations (ScummVM menus, 2D games and 3D games) and choose one of the three possible modes:\n"
"\n"
" * Direct mouse, the touch controls are direct. The pointer jumps to where the finger touches the screen (default for menus).\n"
" * Touchpad emulation, the touch controls are indirect. The finger can be far away from the pointer and still move it, like on a laptop touchpad.\n"
" * Gamepad emulation, the touch controls don't move any mouse. The fingers must be placed on lower left and right of the screen and respectively emulate a directional pad and action buttons.\n"
" * The pointer speed setting in the **Controls tab** affects how far the pointer moves in response to a finger movement.\n"
"\n"
"The touch mode can be switched at anytime by tapping on the controller icon, next to the menu icon at the top right of the screen.\n"
"\n"
"To display or hide the small controller icon, from the Launcher select **Options** and then the **Backend** tab. Tick the **Show on-screen control** box to enable the controller icon.\n"
"\n"
"#### Two finger tap\n"
"\n"
"To do a two finger tap, hold one finger down and then tap with a second finger.\n"
"\n"
"#### Three finger tap\n"
"\n"
"To do a three finger tap, start with holding down one finger and progressively touch down the other two fingers, one at a time, while still holding down the previous fingers. Imagine you are impatiently tapping your fingers on a surface, but then slow down that movement so it is rhythmic, but not too slow.\n"
"\n"
"#### Immersive Sticky fullscreen mode\n"
"\n"
"ScummVM for Android uses the Immersive Sticky fullscreen mode, which means that the Android system bar is hidden until the user swipes from an edge with a system bar. Swipe from the edge to reveal the system bars.  They remain semi-transparent and disappear after a few seconds unless you interact with them. Your swipe also registers in the game, so if you need to swipe from an edge with system bars, your game play is not interrupted.\n"
"\n"
"#### Global Main Menu\n"
"\n"
"To open the Global Main Menu, tap on the small menu icon at the top right of the screen.\n"
"\n"
"To display or hide the small menu icon, from the Launcher select **Options** and then the **Backend** tab. Tick the **Show on-screen control** box to enable the menu icon.\n"
"\n"
"#### Virtual keyboard\n"
"\n"
"To open the virtual keyboard, long press on the small controller icon at the top right of the screen, or tap on any editable text field. To hide the virtual keyboard, tap the small controller icon (which became a keyboard one) again, or tap outside the text field.\n"
"\n"
	);

	new RichTextWidget(tab, 10, 10, _w - 10, tabHeight - buttonHeight - 10, helpText2);

	tab->addTab(_("Adding Games"), "GlobalOptions_Graphics", false);

	Common::U32String helpText3 = _(
"#### Adding SAF paths to ScummVM directory list\n"
"\n"
"Starting with version 2.7.0 of ScummVM for Android, significant changes were made to the file access system to allow support for modern versions of the Android Operating System.\n"
"\n"
"If you find that your existing added games or custom paths no longer work, please edit those paths and this time use the SAF system to browse to the desired locations.\n"
"\n"
"To do that:\n"
"\n"
"  1. For each game whose data is not found, go to the \"Paths\" tab in the \"Game Options\" and change the \"Game path\"\n"
"\n"
"  2. Inside the ScummVM file browser, use \"Go Up\" until you reach the \"root\" folder where you will see the \"<Add a new folder>\" option.\n"
"\n"
"  ![File browser](browser-root.png \"Android browser\")\n"
"\n"
"    File Browser root with <Add a new folder> item\n"
"\n"
"  3. Choose that, then browse and select the \"parent\" folder for your games subfolders, e.g. \"SD Card > ScummVMgames\". Click on \"Use this folder\".\n"
"\n"
"  ![OS file browser root](fs-root.png \"OS file browser root\")\n"
"\n"
"    OS file browser root\n"
"\n"
"  ![OS selectable folder](fs-folder.png \"OS selectable folder\")\n"
"\n"
"    OS file browser selectable folder with \"Use this folder\" button\n"
"\n"
"  ![OS access permission dialog](fs-permission.png \"OS access permission\")\n"
"\n"
"    OS file browser ask to grant ScummVM directory access permission\n"
"\n"
"  4. Then, a new folder \"ScummVMgames\" will appear on the \"root\" folder of the ScummVM browser.\n"
"\n"
"  ![SAF folder added](browser-folder-in-list.png \"SAF folder added\")\n"
"\n"
"    File browser with added SAF folder in root\n"
"\n"
"  5. Browse through this folder to your game data.\n"
"\n"
"Steps 2 and 3 need to be done only once for all of your games.\n"
"\n"
"\n"
"#### Removing SAF path authorizations\n"
"\n"
"In case you would like to revoke any of the granted SAF authorizations, there is an option for this in the \"Global Options > Backend\" tab as shown on the screenshot below:\n"
"\n"
"![\"Remove folder authorizations...\" button](gui-remove-permissions.png \"'Remove folder authorizations...' button\")\n"
"\n"
"    GUI tab with \"Remove folder authorizations...\" button\n"
"\n"
"![List of authorizations to revoked](gui-remove-list.png \"List of authorizations to revoke\")\n"
"\n"
"    GUI dialog with list of authorizations to revoke\n"
"\n"
"In case you revoke authorization to a path, still used for specific games/titles, please follow the procedure of fixing them outlined in the previous subheading.\n"
	);

	new RichTextWidget(tab, 10, 10, _w - 10, tabHeight - buttonHeight - 10, helpText3);

	tab->addTab(_("Paths"), "GlobalOptions_Graphics", false);

	new ButtonWidget(this, _w - buttonWidth - 10, _h - buttonHeight - 10, buttonWidth, buttonHeight, Common::U32String("Close"), Common::U32String(), kCloseCmd);
}

void HelpDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
		case kCloseCmd:
			close();
	}
}

} // End of namespace GUI
