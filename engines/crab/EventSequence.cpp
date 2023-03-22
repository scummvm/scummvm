#include "stdafx.h"

#include "EventSequence.h"

using namespace pyrodactyl::event;

//------------------------------------------------------------------------
// Purpose: Load
//------------------------------------------------------------------------
void EventSequence::Load(const std::string &filename) {
	XMLDoc conf(filename);
	if (conf.ready()) {
		rapidxml::xml_node<char> *node = conf.Doc()->first_node("events");
		for (auto n = node->first_node("event"); n != NULL; n = n->next_sibling("event")) {
			GameEvent e(n);
			events.push_back(e);
		}
	}
}

//------------------------------------------------------------------------
// Purpose: Check for events happening
//------------------------------------------------------------------------
void EventSequence::InternalEvents(pyrodactyl::event::Info &info) {
	for (auto nxe = next.begin(); nxe != next.end(); ++nxe)
		if (*nxe < events.size()) {
			if (events[*nxe].trig.Evaluate(info)) {
				event_in_progress = true;
				cur = *nxe;
				break;
			}
		}
}

//------------------------------------------------------------------------
// Purpose: Go to next event
//------------------------------------------------------------------------
void EventSequence::NextEvent(pyrodactyl::event::Info &info, const std::string &player_id, std::vector<EventResult> &result,
							  std::vector<EventSeqInfo> &end_seq, int NextEventChoice) {
	bool sync = false;
	event_in_progress = false;

	// Execute all effects associated with the event
	for (auto i = events[cur].effect.begin(); i != events[cur].effect.end(); ++i)
		if (i->Execute(info, player_id, result, end_seq))
			sync = true;

	// Play a notification sound
	using namespace pyrodactyl::music;

	if (info.sound.rep_dec)
		gMusicManager.PlayEffect(gMusicManager.rep_dec, 0);
	else if (info.sound.rep_inc)
		gMusicManager.PlayEffect(gMusicManager.rep_inc, 0);
	else if (info.sound.notify)
		gMusicManager.PlayEffect(gMusicManager.notify, 0);

	info.sound.notify = false;
	info.sound.rep_dec = false;
	info.sound.rep_inc = false;

	if (!result.empty() || sync) {
		EventResult r;
		r.type = ER_SYNC;
		result.push_back(r);
	}

	// Clear the next event list
	next.clear();

	// Add the next event to the event list
	if (NextEventChoice != -1)
		next.push_back(NextEventChoice);
	else {
		for (auto i = events[cur].next.begin(); i != events[cur].next.end(); ++i)
			next.push_back(*i);
	}
}

//------------------------------------------------------------------------
// Purpose: Save the state of the object
//------------------------------------------------------------------------
void EventSequence::SaveState(rapidxml::xml_document<char> &doc, rapidxml::xml_node<char> *root, const char *name) {
	rapidxml::xml_node<char> *seqnode = doc.allocate_node(rapidxml::node_element, "set");

	// Write current event id and name to node
	seqnode->append_attribute(doc.allocate_attribute("name", name));
	seqnode->append_attribute(doc.allocate_attribute("current", gStrPool.Get(cur)));

	// Prepare strings of next events and write them
	for (unsigned int i = 0; i < next.size(); i++) {
		rapidxml::xml_node<char> *child = doc.allocate_node(rapidxml::node_element, "next");
		child->value(gStrPool.Get(next[i]));
		seqnode->append_node(child);
	}

	root->append_node(seqnode);
}

//------------------------------------------------------------------------
// Purpose: Load the state of the object
//------------------------------------------------------------------------
void EventSequence::LoadState(rapidxml::xml_node<char> *node) {
	rapidxml::xml_attribute<char> *curid = node->first_attribute("current");
	if (curid != NULL)
		cur = StringToNumber<unsigned int>(curid->value());

	next.clear();
	for (auto n = node->first_node("next"); n != NULL; n = n->next_sibling("next"))
		next.push_back(StringToNumber<unsigned int>(n->value()));

	if (next.empty())
		next.push_back(0);
}