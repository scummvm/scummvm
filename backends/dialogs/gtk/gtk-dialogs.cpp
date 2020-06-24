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
#include "common/encoding.h"
#include "common/translation.h"

#include <gtk/gtk.h>

Common::DialogManager::DialogResult GtkDialogManager::showFileBrowser(const char *title, Common::FSNode &choice, bool isDirBrowser) {
	if (!gtk_init_check(NULL, NULL))
		return kDialogError;

	DialogResult result = kDialogCancel;

	// Get current encoding
	Common::String guiEncoding = "ASCII";
#ifdef USE_TRANSLATION
	guiEncoding = TransMan.getCurrentCharset();
#endif
	Common::Encoding utf8("utf-8", guiEncoding);

	// Convert labels to UTF-8
	char *utf8Title = utf8.convert(title, strlen(title));
	Common::String choose = _("Choose");
	char *utf8Choose = utf8.convert(choose.c_str(), choose.size());
	Common::String cancel = _("Cancel");
	char* utf8Cancel = utf8.convert(cancel.c_str(), cancel.size());

	GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
	if (isDirBrowser) {
		action = GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER;
	}
#if GTK_CHECK_VERSION(3,20,0)
	GtkFileChooserNative *native = gtk_file_chooser_native_new(utf8Title, NULL, action, utf8Choose, utf8Cancel);
	GtkFileChooser *chooser = GTK_FILE_CHOOSER(native);
#else
	GtkWidget *dialog = gtk_file_chooser_dialog_new(utf8Title, NULL, action, utf8Choose, GTK_RESPONSE_ACCEPT, utf8Cancel, GTK_RESPONSE_CANCEL, NULL);
	GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
#endif
	free(utf8Cancel);
	free(utf8Choose);
	free(utf8Title);

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
		result = kDialogOk;
		g_free(path);

		// Save last path
		char *last = gtk_file_chooser_get_current_folder(chooser);
		ConfMan.set("browser_lastpath", last);
		g_free(last);
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
