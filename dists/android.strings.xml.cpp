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

/*
 * This is a dummy file used for sticking strings from
 * dists/android/res/values/strings.xml into our translation system
 *
 */

#include "common/translation.h" // For catching the file during POTFILES reviews

Common::U32String app_desc = _("Graphic adventure game engine");
Common::U32String ok = _("OK");
Common::U32String quit = _("Quit");
Common::U32String no_config_file_title = _("Config File Error");
Common::U32String no_config_file = _("Unable to read ScummVM config file or create a new one!");
Common::U32String no_save_path_title = _("Save Path Error");
Common::U32String no_save_path_configured = _("Unable to create or access default save path!");
Common::U32String no_icons_path_title = _("Icons Path Error");
Common::U32String no_icons_path_configured = _("Unable to create or access default icons and shaders path!");
Common::U32String bad_explicit_save_path_configured = _("Unable to access the globally set save path! Please revert to default from ScummVM Options");
Common::U32String keyboard_toggle_btn_desc = _("Toggle virtual keyboard");

Common::U32String customkeyboardview_keycode_alt =
	// I18N: Description of the Alt button in a KeyboardView.
	_("Alt");

Common::U32String customkeyboardview_keycode_cancel =
	// I18N: Description of the Cancel button in a KeyboardView.
	_("Cancel");

Common::U32String customkeyboardview_keycode_delete =
	// I18N: Description of the Delete button in a KeyboardView.
	_("Delete");

Common::U32String customkeyboardview_keycode_done =
	// I18N: Description of the Done button in a KeyboardView.
	_("Done");

Common::U32String customkeyboardview_keycode_mode_change =
	// I18N: Description of the Mode change button in a KeyboardView.
	_("Mode change");

Common::U32String customkeyboardview_keycode_shift =
	// I18N: Description of the Shift button in a KeyboardView.
	_("Shift");

Common::U32String customkeyboardview_keycode_enter =
	// I18N: Description of the Enter button in a KeyboardView.
	_("Enter");

Common::U32String customkeyboardview_popup_close = _("Close popup");

Common::U32String saf_request_prompt = _("Please select the *root* of your external (physical) SD card. This is required for ScummVM to access this path: ");
Common::U32String saf_revoke_done = _("Storage Access Framework Permissions for ScummVM were revoked!");