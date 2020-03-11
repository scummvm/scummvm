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

#ifndef ULTIMA4_XML_H
#define ULTIMA4_XML_H

#include "common/list.h"
#include "common/str.h"

namespace Ultima {
namespace Ultima4 {

enum xmlNodeType {
	XML_COMMENT_NODE
};

typedef char xmlChar;

struct xmlNode {
	const xmlChar *name;
	xmlNodeType type;
	xmlNode *parent;
	xmlNode *children;
	xmlNode *next;
};

typedef void *xmlDocPtr;
typedef xmlNode *xmlNodePtr;

xmlDocPtr xmlParse(const char *filename);
bool xmlPropExists(xmlNodePtr node, const char *name);
Common::String xmlGetPropAsString(xmlNodePtr node, const char *name);
int xmlGetPropAsBool(xmlNodePtr node, const char *name);
int xmlGetPropAsInt(xmlNodePtr node, const char *name);
int xmlGetPropAsEnum(xmlNodePtr node, const char *name, const char *enumValues[]);
int xmlPropCmp(xmlNodePtr node, const char *name, const char *s);
int xmlPropCaseCmp(xmlNodePtr node, const char *name, const char *s);

bool xmlNodeIsText(xmlNodePtr node);
void xmlFreeDoc(xmlDocPtr doc);
int xmlStrcmp(const xmlChar *s1, const xmlChar *s2);
void xmlFree(void *ptr);
char *xmlGetPropAsStr(xmlNodePtr node, const char *name);
xmlChar *xmlNodeGetContent(xmlNodePtr node);

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
