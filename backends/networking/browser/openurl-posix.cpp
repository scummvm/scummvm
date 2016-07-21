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

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "backends/networking/browser/openurl.h"
#include "common/textconsole.h"
#include <stdlib.h>

namespace Networking {
namespace Browser {

namespace {
bool launch(const Common::String client, const Common::String &url) {
    // FIXME: system's input must be heavily escaped
    // well, when url's specified by user
    // it's OK now (urls are hardcoded somewhere in GUI)
    Common::String cmd = client + " " + url;
    return (system(cmd.c_str()) != -1);
}
}

bool openUrl(const Common::String &url) {
    // inspired by Qt's "qdesktopservices_x11.cpp"

    // try "standards"
    if (launch("xdg-open", url))
        return true;
    if (launch(getenv("DEFAULT_BROWSER"), url))
        return true;
    if (launch(getenv("BROWSER"), url))
        return true;

    // try desktop environment specific tools
    if (launch("gnome-open", url)) // gnome
        return true;
    if (launch("kfmclient openURL", url)) // kde
        return true;
    if (launch("exo-open", url)) // xfce
        return true;

    // try browser names
    if (launch("firefox", url))
        return true;
    if (launch("mozilla", url))
        return true;
    if (launch("netscape", url))
        return true;
    if (launch("opera", url))
        return true;

    warning("Networking::Browser::openUrl() (POSIX) failed to open URL");
    return false;
}

} // End of namespace Browser
} // End of namespace Networking

