#include "MapData.h"
#include "stdafx.h"

using namespace pyrodactyl::ui;

void MapData::Load(rapidxml::xml_node<char> *node) {
	LoadStr(path_bg, "bg", node);
	LoadStr(path_overlay, "overlay", node);
}

void MapData::DestAdd(const std::string &name, const int &x, const int &y) {
	MarkerData md;
	md.name = name;
	md.pos.x = x;
	md.pos.y = y;
	dest.push_back(md);
}

void MapData::SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root) {
	rapidxml::xml_node<char> *child_clip = doc.allocate_node(rapidxml::node_element, "clip");
	for (auto c = reveal.begin(); c != reveal.end(); ++c)
		c->SaveState(doc, child_clip, "rect");
	root->append_node(child_clip);

	rapidxml::xml_node<char> *child_dest = doc.allocate_node(rapidxml::node_element, "dest");
	for (auto d = dest.begin(); d != dest.end(); ++d) {
		rapidxml::xml_node<char> *child = doc.allocate_node(rapidxml::node_element, "pos");
		child->append_attribute(doc.allocate_attribute("name", d->name.c_str()));
		child->append_attribute(doc.allocate_attribute("x", gStrPool.Get(d->pos.x)));
		child->append_attribute(doc.allocate_attribute("y", gStrPool.Get(d->pos.y)));
		child_dest->append_node(child);
	}
	root->append_node(child_dest);
}

void MapData::LoadState(rapidxml::xml_node<char> *node) {
	reveal.clear();
	if (NodeValid("clip", node)) {
		rapidxml::xml_node<char> *clipnode = node->first_node("clip");
		for (rapidxml::xml_node<char> *n = clipnode->first_node("rect"); n != NULL; n = n->next_sibling("rect")) {
			Rect r;
			r.Load(n);
			reveal.push_back(r);
		}
	}

	dest.clear();
	if (NodeValid("dest", node)) {
		rapidxml::xml_node<char> *destnode = node->first_node("dest");
		for (rapidxml::xml_node<char> *n = destnode->first_node("pos"); n != NULL; n = n->next_sibling("pos")) {
			MarkerData md;
			LoadStr(md.name, "name", n);
			md.pos.Load(n);
			dest.push_back(md);
		}
	}
}