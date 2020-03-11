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

#ifndef ULTIMA4_CONFIG_H
#define ULTIMA4_CONFIG_H

#include "ultima/ultima4/xml.h"
#include "ultima/shared/std/containers.h"

namespace Ultima {
namespace Ultima4 {

class ConfigElement;

/**
 * Singleton class that manages the XML configuration tree.
 */
class Config {
public:
    static const Config *getInstance();

    ConfigElement getElement(const Common::String &name) const;

    static Std::vector<Common::String> getGames();
    static void setGame(const Common::String &name);

    static char * CONFIG_XML_LOCATION_POINTER;


private:
    Config();
    static void *fileOpen(const char *filename);
    static void accumError(void *l, const char *fmt, ...);

    static Config *instance;
    xmlDocPtr doc;
};

/**
 * A single configuration element in the config tree.  Right now, a
 * thin wrapper around the XML DOM element.
 */
class ConfigElement {
public:
    ConfigElement(xmlNodePtr xmlNode);
    ConfigElement(const ConfigElement &e);
	ConfigElement();
    ~ConfigElement();

    ConfigElement &operator=(const ConfigElement &e);

    const Common::String getName() const { return _name; }

    bool exists(const Common::String &name) const;
    Common::String getString(const Common::String &name) const;
    int getInt(const Common::String &name, int defaultValue = 0) const;
    bool getBool(const Common::String &name) const;
    int getEnum(const Common::String &name, const char *enumValues[]) const;

    Std::vector<ConfigElement> getChildren() const;

    xmlNodePtr getNode() const { return _node; }

private:
    xmlNodePtr _node;
    Common::String _name;
};

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
