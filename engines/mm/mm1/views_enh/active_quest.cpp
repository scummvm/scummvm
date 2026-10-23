/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "mm/mm1/views_enh/active_quest.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/metaengine.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

namespace {

struct QuestLogEntry {
	bool _completed = false;
	Common::String _summary;
	Common::String _title;
	Common::String _details;
};

byte partyFlags(int index) {
	byte result = 0;
	for (uint i = 0; i < g_globals->_party.size(); ++i)
		result |= g_globals->_party[i]._flags[index];
	return result;
}

bool everyPartyMemberHas(int index, byte flags) {
	if (g_globals->_party.empty())
		return false;
	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		if ((g_globals->_party[i]._flags[index] & flags) != flags)
			return false;
	}
	return true;
}

void addEntry(Common::Array<QuestLogEntry> &entries, bool done,
		const Common::String &title, const Common::String &details) {
	QuestLogEntry entry;
	entry._completed = done;
	entry._title = title;
	entry._summary = Common::String::format("[%s] %s",
		done ? "Done" : "ACTIVE", title.c_str());
	entry._details = details;
	entries.push_back(entry);
}

void setFormalQuestText(Common::String &title, Common::String &details,
		byte questNum) {
	if (questNum >= 1 && questNum <= 7) {
		title = "Lord Ironfist";
		details = STRING[Common::String::format(
			"maps.map43.quests.%d", questNum)];
	} else if (questNum >= 8 && questNum <= 14) {
		title = "Lord Inspectron";
		details = STRING[Common::String::format(
			"maps.map35.quests.%d", questNum - 8)];
	} else if (questNum >= 15 && questNum <= 21) {
		title = "Lord Hacker";
		details = Common::String::format("%s %s",
			STRING["maps.map36.hacker3"].c_str(),
			STRING[Common::String::format("maps.map36.ingredients.%d",
				questNum - 15)].c_str());
	}
}

void addFormalQuest(Common::Array<QuestLogEntry> &entries) {
	const byte questNum = g_globals->_party.empty() ?
		0 : g_globals->_party[0]._quest;
	if (!questNum || questNum == 255)
		return;

	Common::String title, details;
	setFormalQuestText(title, details, questNum);
	if (!title.empty())
		addEntry(entries, false, title, details);
}

void addCourier(Common::Array<QuestLogEntry> &entries, bool done) {
	const byte flags = partyFlags(0);
	const bool dogStatueDone = (flags & CHARFLAG0_DOG_STATUE) != 0;
	if (done != dogStatueDone)
		return;
	if (!done && !(flags & (CHARFLAG0_COURIER1 | CHARFLAG0_COURIER2 |
			CHARFLAG0_COURIER3 | CHARFLAG0_ZOM_CLUE | CHARFLAG0_ZAM_CLUE |
			CHARFLAG0_FOUND_CHEST | CHARFLAG0_40)))
		return;

	// Later courier stages replace or clear some earlier flags, so infer
	// completed steps from the prerequisites of the furthest known stage.
	const bool strongholdDone = dogStatueDone || (flags & CHARFLAG0_40);
	const bool chestDone = strongholdDone || (flags & CHARFLAG0_FOUND_CHEST);
	const bool zamDone = chestDone || (flags & CHARFLAG0_ZAM_CLUE);
	const bool zomDone = chestDone || (flags & CHARFLAG0_ZOM_CLUE);
	const bool telgoranDone = zamDone || zomDone;
	const bool agarDone = telgoranDone ||
			(flags & (CHARFLAG0_COURIER2 | CHARFLAG0_COURIER3));

	const int completed = (agarDone ? 1 : 0) + (telgoranDone ? 1 : 0) +
			(zamDone ? 1 : 0) + (zomDone ? 1 : 0) +
			(chestDone ? 1 : 0) + (strongholdDone ? 1 : 0) +
			(dogStatueDone ? 1 : 0);

	Common::String details = Common::String::format(
			"Courier trail: %d/7 completed.", completed);
	if (agarDone)
		details += "\nAgar - delivered the courier's scroll.";
	if (telgoranDone)
		details += "\nTelgoran - completed the scroll delivery.";
	if (zamDone && zomDone) {
		details += "\nZam and Zom - received both clues.";
	} else {
		if (zamDone)
			details += "\nZam - received his clue.";
		if (zomDone)
			details += "\nZom - received his clue.";
	}
	if (chestDone)
		details += "\nHidden chest - recovered the Ruby Whistle.";
	if (strongholdDone)
		details += "\nAncient temple - entered the hidden stronghold.";
	if (dogStatueDone)
		details += "\nDog Statue - solved; reward claimed.";
	else
		details += "\nFollow the latest clue to its destination.";

	addEntry(entries, done, "The Courier Mission", details);
}

void addAlamarStory(Common::Array<QuestLogEntry> &entries, bool done) {
	const byte flags = partyFlags(13);
	const bool won = (flags & CHARFLAG13_80) != 0;

	if (done) {
		if (won) {
			addEntry(entries, true, "The Inner Sanctum",
				"Completed the final quest within the Inner Sanctum.");
		}
		return;
	}

	if (won)
		return;

	const byte questNum = g_globals->_party.empty() ?
		0 : g_globals->_party[0]._quest;
	const byte projectorFlags[5] = {
		CHARFLAG13_1, CHARFLAG13_2, CHARFLAG13_4,
		CHARFLAG13_8, CHARFLAG13_10
	};
	const byte projectors = CHARFLAG13_1 | CHARFLAG13_2 | CHARFLAG13_4 |
		CHARFLAG13_8 | CHARFLAG13_10;
	int projectorCount = 0;
	for (uint i = 0; i < 5; ++i) {
		if (everyPartyMemberHas(13, projectorFlags[i]))
			++projectorCount;
	}
	const bool defeatedImposter = (flags & CHARFLAG13_ALAMAR) != 0;
	const bool inSoulMaze = g_maps && g_maps->_currentMap &&
		g_maps->_currentMap->getMapIndex() == 48;
	const bool hasEye = g_globals->_party.hasItem(EYE_OF_GOROS_ID);

	// Later story evidence takes precedence over the stale quest byte 255.
	// The original game does not clear 255 automatically after the fake
	// Crypt of Carmenca assignment.
	if (defeatedImposter || (flags & projectors)) {
		if (projectorCount < 5) {
			addEntry(entries, false, "The Inner Sanctum",
				Common::String::format(
					"Find and activate all five magical projectors.\nProgress: %d/5.",
					projectorCount));
		} else {
			addEntry(entries, false, "The Inner Sanctum",
				"All five projectors are active.\nProgress: 5/5.\nContinue into the Inner Sanctum.");
		}
		return;
	}

	if (inSoulMaze) {
		addEntry(entries, false, "Soul Maze",
			"Find your captor's name hidden within the Soul Maze, then report it to Orango.");
		return;
	}

	if (hasEye) {
		addEntry(entries, false, "The Eye of Goros",
			"Confront King Alamar with the Eye of Goros.");
		return;
	}

	if (questNum == 255) {
		addEntry(entries, false, "King Alamar",
			"Find the Crypt of Carmenca.");
	}
}

void addRanalou(Common::Array<QuestLogEntry> &entries, bool done) {
	struct PrisonerJudgmentRecord {
		byte _flag;
		const char *_location;
		const char *_name;
	};
	static const PrisonerJudgmentRecord PRISONERS[] = {
		{ CHARFLAG1_2,  "Dragadune Ruins",         "Mutated prisoner" },
		{ CHARFLAG1_4,  "Castle Doom",             "Child prisoner" },
		{ CHARFLAG1_8,  "Castle Alamar",           "Maiden prisoner" },
		{ CHARFLAG1_10, "Castle White Wolf",       "Demon prisoner" },
		{ CHARFLAG1_20, "Castle Blackridge North", "Man in shackles" },
		{ CHARFLAG1_40, "Castle Blackridge South", "Cloaked prisoner" }
	};

	const byte flags = partyFlags(1);
	const bool started = (flags & CHARFLAG1_1) != 0;
	const bool finished = everyPartyMemberHas(1, CHARFLAG1_WORTHY);
	int judged = 0;

	for (uint i = 0; i < ARRAYSIZE(PRISONERS); ++i) {
		if (flags & PRISONERS[i]._flag)
			++judged;
	}

	// A prisoner may be encountered before the party meets Wizard Ranalou.
	// Existing character flags let the log show that visit without adding any
	// new save data. The exact judgment choice is not stored by MM1.
	if (done != finished || (!done && !started && !judged))
		return;

	Common::String details = done ?
		"Reward received. Prisoners judged:" :
		Common::String::format("Prisoners judged: %d/6.", judged);
	for (uint i = 0; i < ARRAYSIZE(PRISONERS); ++i) {
		if (flags & PRISONERS[i]._flag) {
			details += Common::String::format("\n%s - %s",
				PRISONERS[i]._location, PRISONERS[i]._name);
		}
	}

	addEntry(entries, done,
		started || done ? "Wizard Ranalou's Prisoners" : "Prisoner Judgments",
		details);
}

byte partyQuestHistory(int mapIndex) {
	const Maps::Map &map = *g_maps->getMap(mapIndex);
	byte result = 0;
	for (int i = 0; i < 7; ++i) {
		if (map._visited[15 * MAP_W + i] & 0x80)
			result |= 1 << i;
	}
	return result;
}

void addRulerHistory(Common::Array<QuestLogEntry> &entries,
		byte flags, int firstQuest) {
	if (flags & 0x80)
		flags |= 0x7f;
	for (int i = 0; i < 7; ++i) {
		if (!(flags & (1 << i)))
			continue;
		Common::String title, details;
		setFormalQuestText(title, details, firstQuest + i);
		addEntry(entries, true,
			Common::String::format("%s #%d", title.c_str(), i + 1),
			details + " Completed.");
	}
}

void addCompletedRulerQuests(Common::Array<QuestLogEntry> &entries) {
	addRulerHistory(entries, partyFlags(10) | partyQuestHistory(43), 1);
	addRulerHistory(entries, partyFlags(8) | partyQuestHistory(35), 8);
	addRulerHistory(entries, partyFlags(9) | partyQuestHistory(36), 15);
}

void addOtherReliableCompletions(Common::Array<QuestLogEntry> &entries) {
	if (g_globals->_party.hasItem(KINGS_PASS_ID))
		addEntry(entries, true, "The King's Pass", "Obtained the King's Pass from Percella.");
	if (g_globals->_party.hasItem(KEY_CARD_ID))
		addEntry(entries, true, "The Volcano God's Riddle", "Solved the Volcano God's riddle and obtained the Key Card.");
}

void buildQuestList(Common::Array<QuestLogEntry> &entries) {
	// Active objectives always appear first
	addFormalQuest(entries);
	addCourier(entries, false);
	addAlamarStory(entries, false);
	addRanalou(entries, false);

	// Completed history follows and remains selectable with Up/Down
	addCompletedRulerQuests(entries);
	addCourier(entries, true);
	addAlamarStory(entries, true);
	addRanalou(entries, true);
	addOtherReliableCompletions(entries);
}

} // namespace

ActiveQuest::ActiveQuest() : ScrollView("ActiveQuest") {
	setBounds(Common::Rect(0, 0, 320, 200));
	addButton(&g_globals->_escSprites, Common::Point(134, 170),
		0, KEYBIND_ESCAPE, true);
}

bool ActiveQuest::msgFocus(const FocusMessage &msg) {
	MetaEngine::setKeybindingMode(KeybindingMode::KBMODE_QUEST_LOG);
	_selectedQuest = 0;
	return ScrollView::msgFocus(msg);
}

void ActiveQuest::draw() {
	ScrollView::draw();
	setReduced(false);
	writeLine(0, "Quest Log", ALIGN_MIDDLE);
	writeString(152, 172, STRING["enhdialogs.misc.go_back"]);

	Common::Array<QuestLogEntry> entries;
	buildQuestList(entries);
	if (entries.empty()) {
		setReduced(true);
		writeLine(8, "No tracked quests.", ALIGN_MIDDLE);
		return;
	}

	if (_selectedQuest >= (int)entries.size())
		_selectedQuest = entries.size() - 1;
	if (_selectedQuest < 0)
		_selectedQuest = 0;

	const int first = (_selectedQuest / 5) * 5;
	setReduced(true);
	for (int row = 0; row < 5 && first + row < (int)entries.size(); ++row) {
		const int index = first + row;
		writeString(8, 28 + row * 8, Common::String::format("%s%s",
			index == _selectedQuest ? "> " : "  ", entries[index]._summary.c_str()));
	}

	const QuestLogEntry &selected = entries[_selectedQuest];
	setReduced(false);
	writeString(0, 76, selected._title, ALIGN_MIDDLE);
	setReduced(true);
	Common::StringArray lines = splitLines(selected._details);
	for (uint i = 0, y = 100; i < lines.size() && y <= 148; ++i, y += 8)
		writeString(0, y, lines[i], ALIGN_MIDDLE);
	writeString(0, 158, "Up/Down: select", ALIGN_MIDDLE);
}

void ActiveQuest::moveSelection(int delta) {
	Common::Array<QuestLogEntry> entries;
	buildQuestList(entries);
	if (entries.empty())
		return;
	_selectedQuest += delta;
	if (_selectedQuest < 0)
		_selectedQuest = entries.size() - 1;
	else if (_selectedQuest >= (int)entries.size())
		_selectedQuest = 0;
	redraw();
}

bool ActiveQuest::msgAction(const ActionMessage &msg) {
	switch (msg._action) {
	case KEYBIND_QUEST_PREVIOUS:
		moveSelection(-1);
		return true;
	case KEYBIND_QUEST_NEXT:
		moveSelection(1);
		return true;
	case KEYBIND_ESCAPE:
	case KEYBIND_SELECT:
		close();
		return true;
	default:
		break;
	}
	return false;
}

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
