#pragma once
#include "common_header.h"

#include "EventSequence.h"

namespace pyrodactyl {
namespace event {
class EventSeqGroup {
	typedef std::unordered_map<unsigned int, EventSequence> SeqMap;

	// The event sequences in this group
	SeqMap seq;

	// The sequences that have ended in this group
	std::vector<unsigned int> end;

public:
	EventSeqGroup(void) {}
	~EventSeqGroup(void) {}

	void AddSeq(const unsigned int &id, std::string &path);
	void EndSeq(const unsigned int &id);
	bool EventInProgress(const unsigned int &id);
	bool ActiveSeq(unsigned int &active_seq);

	GameEvent *CurEvent(const unsigned int &id);
	void NextEvent(const unsigned int &id, Info &info, const std::string &player_id, std::vector<EventResult> &result,
				   std::vector<EventSeqInfo> &end_seq, const int choice = -1);

	void InternalEvents(Info &info);

	void SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root);
	void LoadState(rapidxml::xml_node<char> *node);
};
} // End of namespace event
} // End of namespace pyrodactyl