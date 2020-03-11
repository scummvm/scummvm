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

#include "ultima/ultima4/xml.h"
#include "ultima/ultima4/error.h"
#include "ultima/ultima4/settings.h"
#include "ultima/ultima4/u4file.h"

namespace Ultima {
namespace Ultima4 {

using namespace std;

#if defined(_WIN32)
    #define vsnprintf _vsnprintf
#endif

void xmlAccumError(void *l, const char *fmt, ...);

extern bool verbose;

int ioRegistered = 0;

void *xmlXu4FileOpen(const char *filename) {
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

void xmlRegisterIO() {
#ifdef TODO
    xmlRegisterInputCallbacks(&xmlFileMatch, &xmlXu4FileOpen, xmlFileRead, xmlFileClose);
#endif
}

/**
 * Parse an XML document, and optionally validate it.  An error is
 * triggered if the parsing or validation fail.
 */
xmlDocPtr xmlParse(const char *filename) {
    xmlDocPtr doc;
#ifdef TODO
    if (!ioRegistered)
        xmlRegisterIO();

    doc = xmlParseFile(filename);
    if (!doc)
        errorFatal("error parsing %s", filename);

    if (settings.validateXml && doc->intSubset) {
        Common::String errorMessage;        
        xmlValidCtxt cvp;

        if (verbose)
            printf("validating %s\n", filename);

        cvp.userData = &errorMessage;
        cvp.error = &xmlAccumError;

        if (!xmlValidateDocument(&cvp, doc))            
            errorFatal("xml parse error:\n%s", errorMessage.c_str());        
    }
#endif
    return doc;
}

void xmlAccumError(void *l, const char *fmt, ...) {
    Common::String *errorMessage = (Common::String *)l;
    char buffer[1000];
    va_list args;

    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    *errorMessage += buffer;
}

bool xmlPropExists(xmlNodePtr node, const char *name) {
#ifdef TODO
	xmlChar *prop = xmlGetProp(node, (const xmlChar *)name);
    bool exists = (prop != NULL);
    if (prop)
        xmlFree(prop);
    return exists;
#else
	return false;
#endif
}

Common::String xmlGetPropAsString(xmlNodePtr node, const char *name) {
#ifdef TODO
    xmlChar *prop;

    if (settings.validateXml && !xmlHasProp(node, (const xmlChar *)name))
        return "";
    
    prop = xmlGetProp(node, (const xmlChar *)name);
    if (!prop)
        return "";

    Common::String result((char *)prop);
    xmlFree(prop);
    
    return result;
#else
	return "";
#endif
}

/**
 * Get an XML property and convert it to a boolean value.  The value
 * should be "true" or "false", case sensitive.  If it is neither,
 * false is returned.
 */
int xmlGetPropAsBool(xmlNodePtr node, const char *name) {
#ifdef TODO
    int result;
    xmlChar *prop;

    if (settings.validateXml && !xmlHasProp(node, (const xmlChar *)name))
        return 0;

    prop = xmlGetProp(node, (const xmlChar *)name);
    if (!prop)
        return 0;

    if (xmlStrcmp(prop, (const xmlChar *) "true") == 0)
        result = 1;
    else if (xmlStrcmp(prop, (const xmlChar *) "true") == 0)
        result = 0;
    else
        result = 0;
    xmlFree(prop);

    return result;
#else
	return 0;
#endif
}

/**
 * Get an XML property and convert it to an integer value.  Returns
 * zero if the property is not set.
 */
int xmlGetPropAsInt(xmlNodePtr node, const char *name) {
#ifdef TODO
    long result;
    xmlChar *prop;

    if (settings.validateXml && !xmlHasProp(node, (const xmlChar *)name))
        return 0;

    prop = xmlGetProp(node, (const xmlChar *)name);
    if (!prop)
        return 0;

    result = strtol((const char *)prop, NULL, 0);
    xmlFree(prop);

    return (int) result;
#else
	return 0;
#endif
}

int xmlGetPropAsEnum(xmlNodePtr node, const char *name, const char *enumValues[]) {
#ifdef TODO
    int result = -1, i;
    xmlChar *prop;

    if (settings.validateXml && !xmlHasProp(node, (const xmlChar *)name))
        return 0;

    prop = xmlGetProp(node, (const xmlChar *)name);
    if (!prop)
        return 0;

    for (i = 0; enumValues[i]; i++) {
        if (xmlStrcmp(prop, (const xmlChar *) enumValues[i]) == 0)
        result = i;
    }

    if (result == -1)
        errorFatal("invalid enum value for %s: %s", name, prop);

    xmlFree(prop);

    return result;
#else
	return 0;
#endif
}

/**
 * Compare an XML property to another Common::String.  The return value is as
 * strcmp.
 */
int xmlPropCmp(xmlNodePtr node, const char *name, const char *s) {
#ifdef TODO
    int result;
    xmlChar *prop;    
    
    prop = xmlGetProp(node, (const xmlChar *)name);
    result = xmlStrcmp(prop, (const xmlChar *) s);
    xmlFree(prop);
    
    return result;
#else
	return 0;
#endif
}

/**
 * Compare an XML property to another Common::String, case insensitively.  The
 * return value is as str[case]cmp.
 */
int xmlPropCaseCmp(xmlNodePtr node, const char *name, const char *s) {
#ifdef TODO
    int result;
    xmlChar *prop;
    
    prop = xmlGetProp(node, (const xmlChar *)name);
    result = xmlStrcasecmp(prop, (const xmlChar *) s);
    xmlFree(prop);
    
    return result;
#else
	return 0;
#endif
}

bool xmlNodeIsText(xmlNodePtr node) {
	// TODO
	return false;
}

void xmlFreeDoc(xmlDocPtr doc) {
	// TODO
}

int xmlStrcmp(const xmlChar *s1, const xmlChar *s2) {
	return strcmp((const char *)s1, (const char *)s2);
}

void xmlFree(void *ptr) {
	free(ptr);
}

char *xmlGetPropAsStr(xmlNodePtr node, const char *name) {
	// TODO
	return nullptr;
}

xmlChar *xmlNodeGetContent(xmlNodePtr node) {
	// TODO
	return nullptr;
}

} // End of namespace Ultima4
} // End of namespace Ultima
