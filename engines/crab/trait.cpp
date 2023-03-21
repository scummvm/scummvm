#include "stdafx.h"
#include "trait.h"

using namespace pyrodactyl::people;

//------------------------------------------------------------------------
// Purpose: Load
//------------------------------------------------------------------------
void Trait::Load(rapidxml::xml_node<char> *node)
{
	LoadStr(id_str, "id", node);
	id = StringToNumber<int>(id_str);

	LoadStr(name, "name", node);
	LoadStr(desc, "desc", node);
	LoadImgKey(img, "img", node);
	LoadBool(unread, "unread", node);
}

void Trait::Clear()
{
	id = -1;
	id_str = "";
	name = "";
	desc = "";
	img = 0;
	unread = false;
}

//------------------------------------------------------------------------
// Purpose: Save and load state
//------------------------------------------------------------------------
void Trait::SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root, const char* rootname)
{
	rapidxml::xml_node<char> *child = doc.allocate_node(rapidxml::node_element, rootname);
	child->append_attribute(doc.allocate_attribute("id", gStrPool.Get(id)));
	child->append_attribute(doc.allocate_attribute("name", name.c_str()));
	child->append_attribute(doc.allocate_attribute("desc", desc.c_str()));
	child->append_attribute(doc.allocate_attribute("img", gStrPool.Get(img)));

	SaveBool(unread, "unread", doc, child);

	root->append_node(child);
}