/* Copyright (C) 2024 Giovanni Cascione <ing.cascione@gmail.com>
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
#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include <streams/file_stream.h>
#include "backends/platform/libretro/include/libretro-options-widget.h"
#include "backends/platform/libretro/include/libretro-core.h"
#include "backends/platform/libretro/include/libretro-fs.h"
#include "backends/platform/libretro/include/libretro-os.h"
#include "gui/launcher.h"

#include "common/translation.h"

enum {
	kPlaylistPathCmd = 'pchp',
	kPlaylistPathClearCmd = 'pclp',
	kPlaylistGenerateCmd = 'pgen'
};

enum {
	kPlaylistFormatJSON = 0,
	kPlaylistFormat6lines,
};

enum {
	kHooksLocationSave = 0,
	kHooksLocationGame,
};

LibretroOptionsWidget::LibretroOptionsWidget(GuiObject *boss, const Common::String &name, const Common::String &domain) :
	OptionsContainerWidget(boss, name, "LibretroOptionsDialog", domain) {

	new GUI::StaticTextWidget(widgetsBoss(), "LibretroOptionsDialog.PlaylistHeader", _("LIBRETRO PLAYLIST GENERATOR"));
	new GUI::StaticTextWidget(widgetsBoss(), "LibretroOptionsDialog.PlaylistSubheader", _("(check '? > Libretro playlist' for detailed info)"));
	(new GUI::ButtonWidget(widgetsBoss(), "LibretroOptionsDialog.PlaylistPathButton", _("Playlists Path"), Common::U32String(), kPlaylistPathCmd))->setTarget(this);
	_playlistPath = new GUI::StaticTextWidget(widgetsBoss(), "LibretroOptionsDialog.PlaylistPath", _("Specifies where your playlist will be saved."));
	GUI::addClearButton(widgetsBoss(), "LibretroOptionsDialog.PlaylistPathButtonClear", kPlaylistPathClearCmd);

	new GUI::StaticTextWidget(widgetsBoss(), "LibretroOptionsDialog.PlaylistVersionText", _("Playlist format:"));
	_playlistVersion = new GUI::PopUpWidget(widgetsBoss(), "LibretroOptionsDialog.PlaylistVersion");
	_playlistVersion->appendEntry(Common::U32String("JSON "), kPlaylistFormatJSON);
	_playlistVersion->appendEntry(Common::U32String("6-lines"), kPlaylistFormat6lines);

	new GUI::StaticTextWidget(widgetsBoss(), "LibretroOptionsDialog.HooksLocationText", _("Hooks location:"));
	_hooksLocation = new GUI::PopUpWidget(widgetsBoss(), "LibretroOptionsDialog.HooksLocation");
	_hooksLocation->appendEntry(_("All in save folder"), kHooksLocationSave);
	_hooksLocation->appendEntry(_("One in each game folder"), kHooksLocationGame);

	_hooksClear = new GUI::CheckboxWidget(widgetsBoss(), "LibretroOptionsDialog.HooksClear", _("Clear existing hooks"));

	(new GUI::ButtonWidget(widgetsBoss(), "LibretroOptionsDialog.PlaylistGenerateButton", _("Generate playlist"), Common::U32String(), kPlaylistGenerateCmd))->setTarget(this);

	new GUI::StaticTextWidget(widgetsBoss(), "LibretroOptionsDialog.PlaylistStatusText", _("Status: "));
	_playlistStatus = new GUI::StaticTextWidget(widgetsBoss(), "LibretroOptionsDialog.PlaylistStatus", Common::String("-"));
}

LibretroOptionsWidget::~LibretroOptionsWidget() {
}

void LibretroOptionsWidget::defineLayout(GUI::ThemeEval &layouts, const Common::String &layoutName, const Common::String &overlayedLayout) const {
	layouts.addDialog(layoutName, overlayedLayout)
	.addLayout(GUI::ThemeLayout::kLayoutVertical, 8)
	.addPadding(0, 0, 0, 0)
	.addWidget("PlaylistHeader", "", -1, layouts.getVar("Globals.Line.Height"))
	.addWidget("PlaylistSubheader", "", -1, layouts.getVar("Globals.Line.Height"))
	.addLayout(GUI::ThemeLayout::kLayoutHorizontal, 10, GUI::ThemeLayout::kItemAlignCenter)
	.addPadding(0, 0, 0, 0)
	.addWidget("PlaylistPathButton", "Button")
	.addWidget("PlaylistPath", "", -1, layouts.getVar("Globals.Line.Height"))
	.addWidget("PlaylistPathButtonClear", "SearchClearButton", layouts.getVar("Globals.Line.Height"), layouts.getVar("Globals.Line.Height"))
	.closeLayout()

	.addLayout(GUI::ThemeLayout::kLayoutHorizontal)
	.addPadding(0, 0, 0, 0)
	.addWidget("PlaylistVersionText", "OptionsLabel")
	.addWidget("PlaylistVersion", "PopUp")
	.closeLayout()

	.addLayout(GUI::ThemeLayout::kLayoutHorizontal)
	.addPadding(0, 0, 0, 0)
	.addWidget("HooksLocationText", "OptionsLabel")
	.addWidget("HooksLocation", "PopUp")
	.closeLayout()

	.addLayout(GUI::ThemeLayout::kLayoutHorizontal)
	.addPadding(0, 0, 0, 0)
	.addWidget("HooksClear", "Checkbox")
	.closeLayout()

	.addLayout(GUI::ThemeLayout::kLayoutHorizontal)
	.addPadding(0, 0, 0, 0)
	.addWidget("PlaylistGenerateButton", "WideButton")
	.addSpace(layouts.getVar("Globals.Line.Height") * 2)
	.addWidget("PlaylistStatusText", "", -1, layouts.getVar("Globals.Line.Height"))
	.addWidget("PlaylistStatus", "", -1, layouts.getVar("Globals.Line.Height"))
	.closeLayout()

	.closeLayout()
	.closeDialog();
}

bool LibretroOptionsWidget::cleanFolder(Common::String &path) {
	bool res = true;
	Common::ArchiveMemberList listHooks;
	Common::FSNode(Common::Path(path)).listChildren(listHooks, "*." CORE_EXTENSIONS);
	for (Common::ArchiveMemberPtr hook : listHooks) {
		Common::FSNode *fshook = dynamic_cast<Common::FSNode *>(hook.get());
		if (fshook->isDirectory())
			continue;
		if (remove(fshook->getPath().toString().c_str()) == 0)
			retro_log_cb(RETRO_LOG_INFO, "Hook file deleted in '%s'.\n", fshook->getPath().toString().c_str());
		else {
			res = false;
			retro_log_cb(RETRO_LOG_WARN, "Failed to delete hook file in '%s'.\n", fshook->getPath().toString().c_str());
		}
	}
	return res;
}

bool LibretroOptionsWidget::generatePlaylist(Common::String playlistPath) {
	bool isFirstEntry = true;
	bool success = true;
	bool cleanSuccess = true;
	Common::String playlistElement;
	Common::String playlistFooter;
	Common::String path;
	char separator[2] = {0};
	Common::String hookPath;
	Common::String hookFilePath;
	Common::String title;

	/* Create playlist file */
	RFILE *playlistFile = filestream_open(Common::String(playlistPath + "/" + CORE_NAME + ".lpl").c_str(), RETRO_VFS_FILE_ACCESS_WRITE, RETRO_VFS_FILE_ACCESS_HINT_NONE);
	if (!playlistFile) {
		_playlistStatus->setLabel(_("Failed, can't access playlist file"));
		retro_log_cb(RETRO_LOG_ERROR, "Failed to access playlst file at '%s'.\n", Common::String(playlistPath + CORE_NAME + ".lpl").c_str());
		return false;
	} else
		retro_log_cb(RETRO_LOG_INFO, "Playlist file created in '%s'.\n", Common::String(playlistPath + CORE_NAME + ".lpl").c_str());

	/* Create common hook folder */
	if (ConfMan.getInt("libretro_hooks_location", _domain) != kHooksLocationGame) {
		hookPath = LibRetroFilesystemNode(Common::String(LIBRETRO_G_SYSTEM->getSaveDir())).getPath() + "/" + COMMON_HOOKS_FOLDER;
		LibRetroFilesystemNode(hookPath).createDirectory();
		if (ConfMan.getBool("libretro_hooks_clear", _domain)) {
			cleanSuccess = cleanFolder(hookPath);
		}
	}

	/* Setup playlist template */
	if (ConfMan.getInt("libretro_playlist_version", _domain) != kPlaylistFormat6lines) {
		const char *cCorePath = retro_get_core_dir();
		LibRetroFilesystemNode corePath(".");
		if (cCorePath)
			corePath = LibRetroFilesystemNode(Common::String(cCorePath));
		filestream_printf(playlistFile, "{\n  \"version\": \"1.5\",\n  \"default_core_path\": \"%s\",\n  \"default_core_name\": \"ScummVM\",\n  \"label_display_mode\": 0,\n  \"right_thumbnail_mode\": 2,\n  \"left_thumbnail_mode\": 3,\n  \"sort_mode\": 0,\n  \"items\": [", corePath.exists() ? corePath.getPath().c_str() : "");
		playlistElement = "%s\n    {\n      \"path\": \"%s\",\n      \"label\": \"%s\",\n      \"core_path\": \"DETECT\",\n      \"core_name\": \"DETECT\",\n      \"crc32\": \"DETECT\",\n      \"db_name\": \"" CORE_NAME ".lpl\"\n    }";
		playlistFooter = "\n  ]\n}";
	} else
		playlistElement = "%s%s\n%s\nDETECT\nDETECT\nDETECT\nScummVM.lpl\n";

	/* Crawl ScummVM game list */
	Common::ConfigManager::DomainMap::iterator iter = ConfMan.beginGameDomains();
	for (; iter != ConfMan.endGameDomains(); ++iter) {

		if (!iter->_value.tryGetVal("path", path) || iter->_value.contains("id_came_from_command_line"))
			continue;

		Common::Path cleanPath = Common::Path::fromConfig(path);
		if (!Common::FSNode(cleanPath).isDirectory())
			continue;

		if (ConfMan.getInt("libretro_hooks_location", _domain) == kHooksLocationGame) {
			hookPath = LibRetroFilesystemNode(cleanPath.toString()).getPath();
			if (ConfMan.getBool("libretro_hooks_clear", _domain))
				if (!cleanFolder(hookPath)) cleanSuccess = false;
		}

		title = iter->_key;
		iter->_value.tryGetVal("description", title);
		hookFilePath = hookPath + + "/" + iter->_key.c_str() + "." + CORE_EXTENSIONS;

		filestream_printf(playlistFile, playlistElement.c_str(), separator, hookFilePath.c_str(), title.c_str());

		if (isFirstEntry && ConfMan.getInt("libretro_playlist_version", _domain) != kPlaylistFormat6lines) {
			*separator = ',';
			isFirstEntry = false;
		}

		/* Create hook file */
		RFILE *hookFile = filestream_open(hookFilePath.c_str(), RETRO_VFS_FILE_ACCESS_WRITE, RETRO_VFS_FILE_ACCESS_HINT_NONE);
		if (!hookFile) {
			retro_log_cb(RETRO_LOG_ERROR, "Failed to access/create hook file at '%s'.\n", hookFilePath.c_str());
			success = false;
			break;
		} else
			retro_log_cb(RETRO_LOG_INFO, "Hook file created in '%s'.\n", hookFilePath.c_str());


		filestream_printf(hookFile, "%s", iter->_key.c_str());
		filestream_close(hookFile);
	}

	filestream_printf(playlistFile, playlistFooter.c_str());
	filestream_close(playlistFile);

	Common::String response;
	if (success) {
		response = _("Done");
		if (!cleanSuccess)
			response += " (" +  _("cleaning failed") + ")";
	} else
		response = _("Failed, can't create hook files");

	_playlistStatus->setLabel(response);

	return success;
}

void LibretroOptionsWidget::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kPlaylistPathCmd: {
		GUI::BrowserDialog browser(_("Select Playlist directory"), true);
		if (browser.runModal() > 0) {
			Common::FSNode dir(browser.getResult());
			_playlistPath->setLabel(dir.getPath().toString());
		}
		break;
	}
	case kPlaylistPathClearCmd: {
		_playlistPath->setLabel(_c("None", "path"));
		break;
	}
	case kPlaylistGenerateCmd: {
		save();
		_playlistStatus->setLabel(Common::String("-"));
		generatePlaylist(ConfMan.get("libretro_playlist_path"));
		break;
	}
	default:
		GUI::OptionsContainerWidget::handleCommand(sender, cmd, data);
	}

}

void LibretroOptionsWidget::load() {
	_playlistPath->setLabel(ConfMan.get("libretro_playlist_path"));

	if (ConfMan.getInt("libretro_playlist_version", _domain) == kPlaylistFormat6lines)
		_playlistVersion->setSelected(kPlaylistFormat6lines);
	else
		_playlistVersion->setSelected(kPlaylistFormatJSON);

	if (ConfMan.getInt("libretro_hooks_location", _domain) == kHooksLocationGame)
		_hooksLocation->setSelected(kHooksLocationGame);
	else
		_hooksLocation->setSelected(kHooksLocationSave);

	_hooksClear->setState(ConfMan.getBool("libretro_hooks_clear", _domain));
}

bool LibretroOptionsWidget::save() {
	Common::String data = _playlistPath->getLabel();
	if (data.empty())
		ConfMan.set("libretro_playlist_path", "", _domain);
	else
		ConfMan.set("libretro_playlist_path", data, _domain);

	if (_playlistVersion->getSelected() == kPlaylistFormat6lines)
		ConfMan.setInt("libretro_playlist_version", kPlaylistFormat6lines, _domain);
	else
		ConfMan.setInt("libretro_playlist_version", kPlaylistFormatJSON, _domain);

	if (_hooksLocation->getSelected() == kHooksLocationGame)
		ConfMan.setInt("libretro_hooks_location", kHooksLocationGame, _domain);
	else
		ConfMan.setInt("libretro_hooks_location", kHooksLocationSave, _domain);

	ConfMan.setBool("libretro_hooks_clear", _hooksClear->getState(), _domain);

	/* Always return true to call applyBackendSettings every time settings are applied */
	return true;
}
