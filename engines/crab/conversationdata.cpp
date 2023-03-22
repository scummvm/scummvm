#include "conversationdata.h"
#include "stdafx.h"

using namespace pyrodactyl::event;

void ReplyChoice::Load(rapidxml::xml_node<char> *node) {
	LoadStr(text, "text", node);
	LoadNum(tone, "tone", node);
	LoadNum(nextid, "next", node);

	if (NodeValid("unlock", node, false))
		unlock.Load(node->first_node("unlock"));

	if (NodeValid("change", node, false))
		for (auto n = node->first_node("change"); n != NULL; n = n->next_sibling("change"))
			change.push_back(n);
}

void ConversationData::Load(rapidxml::xml_node<char> *node) {
	for (auto n = node->first_node("reply"); n != NULL; n = n->next_sibling("reply"))
		reply.push_back(n);
}