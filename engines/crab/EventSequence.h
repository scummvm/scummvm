#pragma once
#include "common_header.h"

#include "ImageManager.h"
#include "GameEventInfo.h"
#include "gameevent.h"
#include "journal.h"
#include "Inventory.h"

namespace pyrodactyl
{
	namespace event
	{
		class EventSequence
		{
			std::vector<GameEvent> events;
			bool event_in_progress;

			//The event currently in execution - updated only when all trigger conditions are met in InternalEvents
			unsigned int cur;

			//The events that can happen next - these are updated when the cur event is over
			//This means cur and next operate in an alternating way
			//scan next until find event, make it cur, end cur and update next, repeat
			std::vector<unsigned int> next;

		public:

			EventSequence(){ event_in_progress = false; next.push_back(0); cur = 0; }
			~EventSequence(){}

			GameEvent* CurrentEvent() { return &events[cur]; }

			//See if we should trigger any event
			void InternalEvents(pyrodactyl::event::Info &info);
			void NextEvent(Info &info, const std::string &player_id, std::vector<EventResult> &result,
				std::vector<EventSeqInfo> &end_seq, int NextEventChoice = -1);

			bool EventInProgress() { return event_in_progress; }
			void EventInProgress(bool val) { event_in_progress = val; }

			//Load and save
			void Load(const std::string &filename);

			void SaveState(rapidxml::xml_document<char> &doc, rapidxml::xml_node<char> *root, const char* name);
			void LoadState(rapidxml::xml_node<char> *node);
		};
	}
}