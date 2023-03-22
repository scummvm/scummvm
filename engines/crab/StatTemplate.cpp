#include "stdafx.h"

#include "StatTemplate.h"
#include "XMLDoc.h"

using namespace pyrodactyl::stat;

void StatTemplates::Load(const std::string &filename) {
	XMLDoc conf(filename);
	if (conf.ready()) {
		rapidxml::xml_node<char> *node = conf.Doc()->first_node("templates");
		for (auto n = node->first_node("stats"); n != NULL; n = n->next_sibling("stats"))
			collection.push_back(n);
	}
}