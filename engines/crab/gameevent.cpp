#include "stdafx.h"
#include "gameevent.h"

using namespace pyrodactyl::event;
using namespace pyrodactyl::people;

GameEvent::GameEvent()
{
	id = 0;
	type = EVENT_DIALOG;
	special = 0;
	state = PST_NORMAL;
}

void GameEvent::Load(rapidxml::xml_node<char> *node)
{
	if (!LoadEventID(id, "id", node))
		id = 0;

	LoadStr(title, "title", node);
	LoadStr(dialog, "dialog", node);
	LoadEnum(state, "state", node, false);

	std::string Type;
	LoadStr(Type, "type", node);

	if (Type == "dlg"){ type = EVENT_DIALOG; special = 0; }
	else if (Type == "reply"){ type = EVENT_REPLY; gEventStore.AddConv(node, special); }
	else if (Type == "animation"){ type = EVENT_ANIM; LoadNum(special, "anim", node); }
	else if (Type == "silent") { type = EVENT_SILENT; special = 0; }
	else if (Type == "text") { type = EVENT_TEXT; special = 0; }
	else { type = EVENT_SPLASH; special = 0; }

	trig.Load(node);

	next.clear();
	for (rapidxml::xml_node<char> *i = node->first_node("next"); i != NULL; i = i->next_sibling("next"))
		if (i->first_attribute("id") != NULL)
			next.push_back(StringToNumber<EventID>(i->first_attribute("id")->value()));

	effect.clear();
	for (rapidxml::xml_node<char> *it = node->first_node("effect"); it != NULL; it = it->next_sibling("effect"))
	{
		Effect e;
		e.Load(it);
		effect.push_back(e);
	}
}