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

#include "gui/download-games-dialog.h"
#include "gui/widget.h"
#include "gui/widgets/list.h"
#include "common/translation.h"

namespace GUI {

enum {
	kDownloadSelectedCmd = 'DWNS',
};

DownloadGamesDialog::DownloadGamesDialog()
	: Dialog("DownloadGames") {

	new StaticTextWidget(this, "DownloadGames.Headline", _("Download Freeware Games"));

	// Add list with downloadable game titles
	_gamesList = new ListWidget(this, "DownloadGames.List");
	_gamesList->setNumberingMode(kListNumberingOff);
	_gamesList->setEditable(false);

	// Populate the ListWidget
	Common::U32StringArray games = {
		_("Beneath a Steel Sky - Freeware CD Version"),
		_("Beneath a Steel Sky - Freeware Floppy Version"),
		_("Broken Sword 2.5: The Return of the Templars - Freeware Version"),
		_("Broken Sword 2.5: The Return of the Templars - Hebrew translation AddOn"),
		_("Dráscula: The Vampire Strikes Back - Freeware Version (English)"),
		_("Dráscula: The Vampire Strikes Back - Freeware Version (Music AddOn, MP3 format)"),
		_("Dráscula: The Vampire Strikes Back - Freeware Version (Music AddOn, FLAC format)"),
		_("Dráscula: The Vampire Strikes Back - Freeware Version (Music AddOn, OGG format)"),
		_("Dráscula: The Vampire Strikes Back - Freeware Version (Spanish, German, French and Italian AddOn)"),
		_("Dráscula: The Vampire Strikes Back - Freeware Version (Updated Spanish, German, French and Italian AddOn) - requires ScummVM 1.3.0 or more"),
	};
	_gamesList->setList(games);

	new ButtonWidget(this, "DownloadGames.Back", _("Back"), Common::U32String(), kCloseCmd);
	new ButtonWidget(this, "DownloadGames.Download", _("Download"), Common::U32String(), kDownloadSelectedCmd);
}

} // End of namespace GUI
