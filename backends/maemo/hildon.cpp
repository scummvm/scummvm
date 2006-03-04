/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
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
 
#include <hildon-widgets/hildon-app.h>
#include <gtk/gtk.h>
#include <libosso.h>

#define OSSO_APP_NAME    "scummvm"
#define OSSO_APP_VERSION "0.9.0CVS"
#define OSSO_APP_SERVICE "org.scummvm."OSSO_APP_NAME
#define OSSO_APP_OBJECT  "/org/scummvm/"OSSO_APP_NAME
#define OSSO_APP_IFACE   "org.scummvm."OSSO_APP_NAME

// Application UI data struct
typedef struct _AppData AppData;
struct _AppData {
    HildonApp *app;
    HildonAppView *appview;
    osso_context_t *osso_context;
};

// Callback for exit D-BUS event
void exit_event_handler(gboolean die_now, gpointer data) {
    AppData *appdata;
    appdata = (AppData *)data;
    g_print("exit_event_handler called\n");
    /* Do whatever application needs to do before exiting */
    gtk_infoprint(GTK_WINDOW(appdata->app), "Exiting...");
}

// Callback for normal D-BUS messages
gint dbus_req_handler(const gchar *interface, const gchar *method,
                      GArray *arguments, gpointer data,
                      osso_rpc_t *retval) {
    AppData *appdata;
    appdata = (AppData *)data;
    osso_system_note_infoprint(appdata->osso_context, method, retval);
    return OSSO_OK;
}


// Main application
int main(int argc, char *argv[]) {
    // Create needed variables
    HildonApp *app;
    HildonAppView *appview;
    osso_context_t *osso_context;
    osso_return_t result;
    GtkWidget *main_vbox;
    GtkWidget *label;

    // Initialize the GTK.
    gtk_init(&argc, &argv);

    // Initialize maemo application
    osso_context = osso_initialize(OSSO_APP_NAME, OSSO_APP_VERSION, TRUE, NULL);

    // Check that initialization was ok
    if (osso_context == NULL) {
        return OSSO_ERROR;
    }

    // Create the hildon application and setup the title
    app = HILDON_APP(hildon_app_new());
    hildon_app_set_title(app, "ScummVM");
    hildon_app_set_two_part_title(app, TRUE);

    // Create HildonAppView and set it to HildonApp
    appview = HILDON_APPVIEW(hildon_appview_new("AppView Title"));
    hildon_app_set_appview(app, appview);

    // Create AppData
    AppData *appdata;
    appdata = g_new0(AppData, 1);
    appdata->app = app;
    appdata->appview = appview;
    appdata->osso_context = osso_context;

    // Add vbox to appview
    main_vbox = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(appview), main_vbox);

    // Add button to vbox
    label = gtk_label_new("Waiting for DBUS message...");
    gtk_box_pack_start(GTK_BOX(main_vbox), label, FALSE, TRUE, 0);

    // Add handler for hello D-BUS messages
    result = osso_rpc_set_cb_f(appdata->osso_context, 
                               OSSO_APP_SERVICE, 
                               OSSO_APP_OBJECT, 
                               OSSO_APP_IFACE,
                               dbus_req_handler, appdata);
    if (result != OSSO_OK) {
        g_print("Error setting D-BUS callback (%d)\n", result);
        return OSSO_ERROR;
    }

    // Add handler for Exit D-BUS messages
    result = osso_application_set_exit_cb(appdata->osso_context,
                                          exit_event_handler,
                                          (gpointer) appdata);
    if (result != OSSO_OK) {
        g_print("Error setting exit callback (%d)\n", result);
        return OSSO_ERROR;
    }

    // Begin the main application
    gtk_widget_show_all(GTK_WIDGET(app));
    gtk_main();

    // Deinitialize OSSO
    osso_deinitialize(osso_context);

    return 0;
}
