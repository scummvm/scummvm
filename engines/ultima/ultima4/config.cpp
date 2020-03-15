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

#include "ultima/ultima4/config.h"
#include "ultima/ultima4/error.h"
#include "ultima/ultima4/settings.h"
#include "ultima/ultima4/u4file.h"

namespace Ultima {
namespace Ultima4 {

using namespace std;

extern bool verbose;
Config *Config::_instance;

Config::Config() {
	_instance = this;

#ifdef TODO
	doc = xmlParseFile(Config::CONFIG_XML_LOCATION_POINTER);
	if (!doc) {
		printf("Failed to read core config.xml. Assuming it is located at '%s'", Config::CONFIG_XML_LOCATION_POINTER);
		errorFatal("error parsing config.xml");
}

	xmlXIncludeProcess(doc);

	if (settings.validateXml && doc->intSubset) {
		Common::String errorMessage;
		xmlValidCtxt cvp;

		if (verbose)
			printf("validating config.xml\n");

		cvp.userData = &errorMessage;
		cvp.error = &accumError;

		// Error changed to not fatal due to regression in libxml2
		if (!xmlValidateDocument(&cvp, doc))
			errorWarning("xml validation error:\n%s", errorMessage.c_str());
	}
#endif
}

Config::~Config() {
	_instance = nullptr;
}

const Config *Config::getInstance() {
#ifdef TODO
    if (!instance) {
        xmlRegisterInputCallbacks(&xmlFileMatch, &fileOpen, xmlFileRead, xmlFileClose);
        instance = new Config;
    }
#endif
	return _instance;
}

ConfigElement Config::getElement(const Common::String &name) const {
#ifdef TODO
	xmlXPathContextPtr context;
    xmlXPathObjectPtr result;

    Common::String path = "/config/" + name;
    context = xmlXPathNewContext(doc);
    result = xmlXPathEvalExpression(reinterpret_cast<const xmlChar *>(path.c_str()), context);
    if(xmlXPathNodeSetIsEmpty(result->nodesetval))
        errorFatal("no match for xpath %s\n", path.c_str());

    xmlXPathFreeContext(context);

    if (result->nodesetval->nodeNr > 1)
        errorWarning("more than one match for xpath %s\n", path.c_str());

    xmlNodePtr node = result->nodesetval->nodeTab[0];
    xmlXPathFreeObject(result);

    return ConfigElement(node);
#else
		return ConfigElement();
#endif
}

char DEFAULT_CONFIG_XML_LOCATION[] = "config.xml";
char * Config::CONFIG_XML_LOCATION_POINTER = &DEFAULT_CONFIG_XML_LOCATION[0];


Std::vector<Common::String> Config::getGames() {
    Std::vector<Common::String> result;
    result.push_back("Ultima IV");
    return result;
}

void Config::setGame(const Common::String &name) {
}

void *Config::fileOpen(const char *filename) {
#ifdef TODO
    void *result;
    Common::String pathname(u4find_conf(filename));

    if (pathname.empty())
        return NULL;
    result = xmlFileOpen(pathname.c_str());

    if (verbose)
        printf("xml parser opened %s: %s\n", pathname.c_str(), result ? "success" : "failed");

    return result;
#else
	return nullptr;
#endif
}

void Config::accumError(void *l, const char *fmt, ...) {
#ifdef TODO
    Common::String* errorMessage = static_cast<Common::String *>(l);
    char buffer[1000];
    va_list args;

    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    errorMessage->append(buffer);
#endif
}

ConfigElement::ConfigElement(xmlNodePtr xmlNode) : _node(xmlNode), _name(reinterpret_cast<const char *>(xmlNode->name)) {
}

ConfigElement::ConfigElement(const ConfigElement &e) : _node(e._node), _name(e._name) {
}

ConfigElement::ConfigElement() : _node(nullptr) {
}

ConfigElement::~ConfigElement() {
}

ConfigElement &ConfigElement::operator=(const ConfigElement &e) {
    if (&e != this) {
        _node = e._node;
        _name = e._name;
    }
    return *this;
}

/**
 * Returns true if the property exists in the current config element
 */
bool ConfigElement::exists(const Common::String &name) const {
#ifdef TODO
    xmlChar *prop = xmlGetProp(node, reinterpret_cast<const xmlChar *>(name.c_str()));
    bool exists = prop != NULL;
    xmlFree(prop);

    return exists;
#else
	return false;
#endif
}

Common::String ConfigElement::getString(const Common::String &name) const {
#ifdef TODO
    xmlChar *prop = xmlGetProp(node, reinterpret_cast<const xmlChar *>(name.c_str()));
    if (!prop)
        return "";

    Common::String result(reinterpret_cast<const char *>(prop));
    xmlFree(prop);
    
    return result;
#else
	return "";
#endif
}

int ConfigElement::getInt(const Common::String &name, int defaultValue) const {
#ifdef TODO
	long result;
    xmlChar *prop;

    prop = xmlGetProp(node, reinterpret_cast<const xmlChar *>(name.c_str()));
    if (!prop)
        return defaultValue;

    result = strtol(reinterpret_cast<const char *>(prop), NULL, 0);
    xmlFree(prop);

    return static_cast<int>(result);
#else
	return 0;
#endif
}

bool ConfigElement::getBool(const Common::String &name) const {
#ifdef TODO
	int result;

    xmlChar *prop = xmlGetProp(node, reinterpret_cast<const xmlChar *>(name.c_str()));
    if (!prop)
        return false;

    if (xmlStrcmp(prop, reinterpret_cast<const xmlChar *>("true")) == 0)
        result = true;
    else
        result = false;

    xmlFree(prop);

    return result;
#else
	return false;
#endif
}

int ConfigElement::getEnum(const Common::String &name, const char *enumValues[]) const {
#ifdef TODO
    int result = -1, i;
    xmlChar *prop;

    prop = xmlGetProp(node, reinterpret_cast<const xmlChar *>(name.c_str()));
    if (!prop)
        return 0;

    for (i = 0; enumValues[i]; i++) {
        if (xmlStrcmp(prop, reinterpret_cast<const xmlChar *>(enumValues[i])) == 0)
        result = i;
    }

    if (result == -1)
        errorFatal("invalid enum value for %s: %s", name.c_str(), prop);

    xmlFree(prop);

    return result;
#else
	return 0;
#endif
}

Std::vector<ConfigElement> ConfigElement::getChildren() const {
	Std::vector<ConfigElement> result;
#ifdef TODO

    for (xmlNodePtr child = node->children; child; child = child->next) {
        if (child->type == XML_ELEMENT_NODE)
            result.push_back(ConfigElement(child));
    }
#endif
    return result;
}

} // End of namespace Ultima4
} // End of namespace Ultima
