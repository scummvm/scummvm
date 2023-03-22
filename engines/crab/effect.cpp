#include "effect.h"
#include "MusicManager.h"
#include "stdafx.h"

using namespace pyrodactyl::event;
using namespace pyrodactyl::music;

void Effect::Load(rapidxml::xml_node<char> *node) {
	std::string ty;
	LoadStr(ty, "type", node);

	// Should we throw a warning about missing fields? Depends on the type of effect
	bool echo_op = true, echo_sub = true, echo_val = true;

	if (ty == "var")
		type = EFF_VAR;
	else if (ty == "journal")
		type = EFF_JOURNAL;
	else if (ty == "person")
		type = EFF_OBJ;
	else if (ty == "item")
		type = EFF_ITEM;
	else if (ty == "like")
		type = EFF_LIKE;
	else if (ty == "fear")
		type = EFF_FEAR;
	else if (ty == "respect")
		type = EFF_RESPECT;
	else if (ty == STATNAME_HEALTH)
		type = EFF_HEALTH;
	else if (ty == "sound")
		type = EFF_SOUND;
	else if (ty == "money")
		type = EFF_MONEY;
	else if (ty == "end") {
		type = EFF_END;
		echo_sub = false;
		echo_val = false;
	} else if (ty == "map")
		type = EFF_MAP;
	else if (ty == "dest")
		type = EFF_DEST;
	else if (ty == "img") {
		type = EFF_IMG;
		echo_op = false;
		echo_sub = false;
	} else if (ty == "trait") {
		type = EFF_TRAIT;
		echo_op = false;
	} else if (ty == "move") {
		type = EFF_MOVE;
		echo_op = false;
	} else if (ty == "level")
		type = EFF_LEVEL;
	else if (ty == "player") {
		type = EFF_PLAYER;
		echo_op = false;
		echo_sub = false;
	} else if (ty == "save") {
		type = EFF_SAVE;
		echo_op = false;
		echo_sub = false;
		echo_val = false;
	} else if (ty == "quit") {
		type = EFF_QUIT;
		echo_op = false;
		echo_sub = false;
		echo_val = false;
	} else
		type = EFF_VAR;

	LoadStr(subject, "subject", node, echo_sub);
	LoadStr(operation, "operation", node, echo_op);
	LoadStr(val, "val", node, echo_val);
}

void Effect::ChangeOpinion(pyrodactyl::event::Info &info, pyrodactyl::people::OpinionType type) {
	int old_op = 0;

	// Only bother if the person exists and has a valid opinion
	if (info.OpinionGet(subject, type, old_op)) {
		if (operation == "=")
			info.OpinionSet(subject, type, StringToNumber<int>(val));
		else if (operation == "+")
			info.OpinionChange(subject, type, StringToNumber<int>(val));
		else if (operation == "-")
			info.OpinionChange(subject, type, -1 * StringToNumber<int>(val));

		int new_op = 0;
		info.OpinionGet(subject, type, new_op);

		if (new_op > old_op)
			info.sound.rep_inc = true;
		else if (new_op < old_op)
			info.sound.rep_dec = true;
	}
}

bool Effect::Execute(pyrodactyl::event::Info &info, const std::string &player_id,
					 std::vector<EventResult> &result, std::vector<EventSeqInfo> &end_seq) {
	if (type < EFF_MOVE) {
		switch (type) {
		case EFF_VAR:
			if (operation == "=")
				info.VarSet(subject, val);
			else if (operation == "del")
				info.VarDel(subject);
			else if (operation == "+")
				info.VarAdd(subject, StringToNumber<int>(val));
			else if (operation == "-")
				info.VarSub(subject, StringToNumber<int>(val));
			else if (operation == "*")
				info.VarMul(subject, StringToNumber<int>(val));
			else if (operation == "/")
				info.VarDiv(subject, StringToNumber<int>(val));
			break;

		case EFF_JOURNAL:
			if (subject == "finish")
				info.journal.Move(player_id, operation, true);
			else if (subject == "start")
				info.journal.Move(player_id, operation, false);
			else
				info.journal.Add(player_id, subject, operation, val);

			// Update unread status of journal
			info.unread.journal = true;

			// used so we only play one notify sound per event
			info.sound.notify = true;
			break;

		case EFF_OBJ:
			if (operation == "type")
				info.Type(subject, pyrodactyl::people::StringToPersonType(val));
			else if (operation == "state")
				info.State(subject, pyrodactyl::people::StringToPersonState(val));
			break;

		case EFF_ITEM:
			if (operation == "del")
				info.inv.DelItem(subject, val);
			else
				info.inv.LoadItem(subject, val);

			// Update unread status of inventory
			info.unread.inventory = true;

			// used so we only play one notify sound per event
			info.sound.notify = true;
			break;

		case EFF_LIKE:
			ChangeOpinion(info, pyrodactyl::people::OPI_LIKE);
			break;

		case EFF_FEAR:
			ChangeOpinion(info, pyrodactyl::people::OPI_FEAR);
			break;

		case EFF_RESPECT:
			ChangeOpinion(info, pyrodactyl::people::OPI_RESPECT);
			break;

		case EFF_HEALTH: {
			using namespace pyrodactyl::stat;
			int num = StringToNumber<int>(val);

			if (operation == "=")
				info.StatSet(subject, STAT_HEALTH, num);
			else if (operation == "+")
				info.StatChange(subject, STAT_HEALTH, num);
			else if (operation == "-")
				info.StatChange(subject, STAT_HEALTH, -1 * num);
		} break;

		case EFF_SOUND:
			if (subject == "music") {
				if (operation == "play") {
					MusicKey m = StringToNumber<MusicKey>(val);
					gMusicManager.PlayMusic(m);
				} else if (operation == "stop")
					gMusicManager.Stop();
				else if (operation == "pause")
					gMusicManager.Pause();
				else if (operation == "resume")
					gMusicManager.Resume();
			} else
				gMusicManager.PlayEffect(StringToNumber<ChunkKey>(val), 0);
			break;

		case EFF_MONEY:
			info.money_var = val;
			break;

		case EFF_END:
			if (operation == "cur")
				end_seq.push_back(true);
			else {
				EventSeqInfo seqinfo;
				seqinfo.loc = subject;
				seqinfo.val = val;
				end_seq.push_back(seqinfo);
			}
			break;

		default:
			break;
		}

		return true;
	} else {
		EventResult r;
		r.val = subject;
		r.x = StringToNumber<int>(operation);
		r.y = StringToNumber<int>(val);

		switch (type) {
		case EFF_MOVE:
			r.type = ER_MOVE;
			break;
		case EFF_MAP:
			r.type = ER_MAP;
			break;
		case EFF_DEST:
			r.type = ER_DEST;
			break;
		case EFF_IMG:
			r.type = ER_IMG;
			info.PlayerImg(StringToNumber<int>(val));
			break;
		case EFF_TRAIT:
			r.type = ER_TRAIT;
			info.unread.trait = true;
			info.sound.notify = true;
			break;
		case EFF_LEVEL:
			r.type = ER_LEVEL;
			break;
		case EFF_PLAYER:
			r.type = ER_PLAYER;
			break;
		case EFF_SAVE:
			r.type = ER_SAVE;
			break;
		case EFF_QUIT:
			r.type = ER_QUIT;
			break;
		default:
			break;
		}

		result.push_back(r);
	}

	return false;
}