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

#define FORBIDDEN_SYMBOL_EXCEPTION_FILE
#define FORBIDDEN_SYMBOL_EXCEPTION_unistd_h
#define FORBIDDEN_SYMBOL_EXCEPTION_time_h
#include "common/scummsys.h"

#if defined(POSIX) && defined(USE_SYSDIALOGS) && defined(USE_GTK)

#include "backends/dialogs/gtk/gtk-dialogs.h"

#include "common/config-manager.h"
#include "common/translation.h"

#include <gtk/gtk.h>

Common::DialogManager::DialogResult GtkDialogManager::showFileBrowser(const Common::U32String &title, Common::FSNode &choice, bool isDirBrowser) {
	if (!gtk_init_check(NULL, NULL))
		return kDialogError;

	DialogResult result = kDialogCancel;

	// Convert labels to UTF-8
	Common::String utf8Title = title.encode();
	Common::String utf8Choose = _("Choose").encode();
	Common::String utf8Cancel = _("Cancel").encode();

	GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
	if (isDirBrowser) {
		action = GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER;
	}
#if GTK_CHECK_VERSION(3,20,0)
	GtkFileChooserNative *native = gtk_file_chooser_native_new(utf8Title.c_str(), NULL, action, utf8Choose.c_str(), utf8Cancel.c_str());
	GtkFileChooser *chooser = GTK_FILE_CHOOSER(native);
#else
	GtkWidget *dialog = gtk_file_chooser_dialog_new(utf8Title.c_str(), NULL, action, utf8Choose.c_str(), GTK_RESPONSE_ACCEPT, utf8Cancel.c_str(), GTK_RESPONSE_CANCEL, NULL);
	GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
#endif

	// Customize dialog
	gtk_file_chooser_set_show_hidden(chooser, ConfMan.getBool("gui_browser_show_hidden", Common::ConfigManager::kApplicationDomain));
	if (ConfMan.hasKey("browser_lastpath")) {
		gtk_file_chooser_set_current_folder(chooser, ConfMan.get("browser_lastpath").c_str());
	}

	// Show dialog
	beginDialog();
#if GTK_CHECK_VERSION(3,20,0)
	int res = gtk_native_dialog_run(GTK_NATIVE_DIALOG(native));
#else
	int res = gtk_dialog_run(GTK_DIALOG(dialog));
#endif
	if (res == GTK_RESPONSE_ACCEPT) {
		// Get the selection from the user
		char *path = gtk_file_chooser_get_filename(chooser);
		choice = Common::FSNode(path);
		ConfMan.set("browser_lastpath", path);
		result = kDialogOk;
		g_free(path);
	}

#if GTK_CHECK_VERSION(3,20,0)
	g_object_unref(native);
#else
	gtk_widget_destroy(dialog);
#endif

	while (gtk_events_pending())
		gtk_main_iteration();
	endDialog();
	return result;
}

#endif
