#include "stdafx.h"

#include "EventSeqGroup.h"

using namespace pyrodactyl::event;

void EventSeqGroup::EndSeq(const unsigned int &id)
{
	seq.erase(id);
	end.push_back(id);
}

void EventSeqGroup::AddSeq(const unsigned int &id, std::string &path)
{
	seq[id].Load(path);
}

bool EventSeqGroup::EventInProgress(const unsigned int &id)
{
	return seq.count(id) > 0 && seq.at(id).EventInProgress();
}

GameEvent* EventSeqGroup::CurEvent(const unsigned int &id)
{
	return seq.at(id).CurrentEvent();
}

void EventSeqGroup::NextEvent(const unsigned int &id, Info &info, const std::string &player_id,
	std::vector<EventResult> &result, std::vector<EventSeqInfo> &end_seq, const int choice)
{
	return seq.at(id).NextEvent(info, player_id, result, end_seq, choice);
}

void EventSeqGroup::InternalEvents(Info &info)
{
	for (auto it = seq.begin(); it != seq.end(); ++it)
		it->second.InternalEvents(info);
}

bool EventSeqGroup::ActiveSeq(unsigned int &active_seq)
{
	for (auto i = seq.begin(); i != seq.end(); ++i)
		if (i->second.EventInProgress())
		{
			active_seq = i->first;
			return true;
		}

	active_seq = UINT_MAX;
	return false;
}

void EventSeqGroup::SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root)
{
	for (auto i = end.begin(); i != end.end(); ++i)
	{
		rapidxml::xml_node<char> *child = doc.allocate_node(rapidxml::node_element, "end");
		child->value(gStrPool.Get(*i));
		root->append_node(child);
	}

	for (auto i = seq.begin(); i != seq.end(); ++i)
		i->second.SaveState(doc, root, gStrPool.Get(i->first));
}

void EventSeqGroup::LoadState(rapidxml::xml_node<char> *node)
{
	for (rapidxml::xml_node<char> *i = node->first_node("end"); i != NULL; i = i->next_sibling("end"))
		EndSeq(StringToNumber<unsigned int>(i->value()));

	for (auto n = node->first_node("set"); n != NULL; n = n->next_sibling("set"))
		if (n->first_attribute("name") != NULL)
		{
			unsigned int id = StringToNumber<unsigned int>(n->first_attribute("name")->value());
			if (seq.count(id) > 0)
				seq[id].LoadState(n);
		}
}