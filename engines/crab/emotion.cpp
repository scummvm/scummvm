#include "stdafx.h"
#include "emotion.h"

using namespace pyrodactyl::ui;
using namespace pyrodactyl::event;

void EmotionIndicator::Load(rapidxml::xml_node<char> *node)
{
	if (NodeValid("text", node))
		text.Load(node->first_node("text"));
}

void EmotionIndicator::Draw(const int &select)
{
	if (select >= 0 && select < value.size())
		if (value.at(select) >= 0 && value.at(select) < gEventStore.tone.size())
		{
			text.Draw(gEventStore.tone.at(value.at(select)).text);
		}
}

void EmotionIndicator::SetUI()
{
	text.SetUI();
}