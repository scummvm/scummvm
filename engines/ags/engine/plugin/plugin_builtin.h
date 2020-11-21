//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
//=============================================================================
//
// Plugin system functions.
//
//=============================================================================
#ifndef __AGS_EE_PLUGIN__PLUGINBUILTIN_H
#define __AGS_EE_PLUGIN__PLUGINBUILTIN_H

#define PLUGIN_FILENAME_MAX (49)

class IAGSEngine;

using namespace AGS; // FIXME later

//  Initial implementation for apps to register their own inbuilt plugins

struct InbuiltPluginDetails {
    char      filename[PLUGIN_FILENAME_MAX+1];
    void      (*engineStartup) (IAGSEngine *);
    void      (*engineShutdown) ();
    int       (*onEvent) (int, int);
    void      (*initGfxHook) (const char *driverName, void *data);
    int       (*debugHook) (const char * whichscript, int lineNumber, int reserved);
};

// Register a builtin plugin.
int pl_register_builtin_plugin(InbuiltPluginDetails const &details);

#endif // __AGS_EE_PLUGIN__PLUGINBUILTIN_H
