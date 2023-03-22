#include "quest.h"
#include "stdafx.h"

using namespace pyrodactyl::event;

Quest::Quest(const std::string &Title, const std::string &Text, const bool &Unread, const bool &Marker) : title(Title) {
	text.push_front(Text);
	unread = Unread;
	marker = Marker;
}

void Quest::LoadState(rapidxml::xml_node<char> *node) {
	LoadStr(title, "title", node);
	LoadBool(unread, "unread", node);
	LoadBool(marker, "marker", node);

	for (rapidxml::xml_node<char> *n = node->first_node("info"); n != NULL; n = n->next_sibling("info"))
		text.push_back(n->value());
}

void Quest::SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root) {
	rapidxml::xml_node<char> *child = doc.allocate_node(rapidxml::node_element, "quest");
	child->append_attribute(doc.allocate_attribute("title", title.c_str()));

	SaveBool(unread, "unread", doc, child);
	SaveBool(marker, "marker", doc, child);

	for (auto i = text.begin(); i != text.end(); ++i) {
		rapidxml::xml_node<char> *grandchild = doc.allocate_node(rapidxml::node_element, "info");
		grandchild->value(i->c_str());
		child->append_node(grandchild);
	}

	root->append_node(child);
}