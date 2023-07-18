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

/*
 * This code is based on the CRAB engine
 *
 * Copyright (c) Arvind Raja Yadav
 *
 * Licensed under MIT
 *
 */

#include "crab/crab.h"
#include "crab/event/effect.h"
#include "crab/music/MusicManager.h"

namespace Crab {

using namespace pyrodactyl::event;
using namespace pyrodactyl::music;

void Effect::load(rapidxml::xml_node<char> *node) {
	Common::String ty;
	loadStr(ty, "type", node);

	// Should we throw a warning about missing fields? Depends on the type of effect
	bool echoOp = true, echoSub = true, echoVal = true;

	if (ty == "var")
		_type = EFF_VAR;
	else if (ty == "journal")
		_type = EFF_JOURNAL;
	else if (ty == "person")
		_type = EFF_OBJ;
	else if (ty == "item")
		_type = EFF_ITEM;
	else if (ty == "like")
		_type = EFF_LIKE;
	else if (ty == "fear")
		_type = EFF_FEAR;
	else if (ty == "respect")
		_type = EFF_RESPECT;
	else if (ty == STATNAME_HEALTH)
		_type = EFF_HEALTH;
	else if (ty == "sound")
		_type = EFF_SOUND;
	else if (ty == "money")
		_type = EFF_MONEY;
	else if (ty == "end") {
		_type = EFF_END;
		echoSub = false;
		echoVal = false;
	} else if (ty == "map")
		_type = EFF_MAP;
	else if (ty == "dest")
		_type = EFF_DEST;
	else if (ty == "img") {
		_type = EFF_IMG;
		echoOp = false;
		echoSub = false;
	} else if (ty == "trait") {
		_type = EFF_TRAIT;
		echoOp = false;
	} else if (ty == "move") {
		_type = EFF_MOVE;
		echoOp = false;
	} else if (ty == "level")
		_type = EFF_LEVEL;
	else if (ty == "player") {
		_type = EFF_PLAYER;
		echoOp = false;
		echoSub = false;
	} else if (ty == "save") {
		_type = EFF_SAVE;
		echoOp = false;
		echoSub = false;
		echoVal = false;
	} else if (ty == "quit") {
		_type = EFF_QUIT;
		echoOp = false;
		echoSub = false;
		echoVal = false;
	} else
		_type = EFF_VAR;

	loadStr(_subject, "_subject", node, echoSub);
	loadStr(_operation, "operation", node, echoOp);
	loadStr(_val, "_val", node, echoVal);
}

void Effect::changeOpinion(pyrodactyl::event::Info &info, pyrodactyl::people::OpinionType opType) {
	int oldOp = 0;

	// Only bother if the person exists and has a valid opinion
	if (info.opinionGet(_subject, opType, oldOp)) {
		if (_operation == "=")
			info.opinionSet(_subject, opType, StringToNumber<int>(_val));
		else if (_operation == "+")
			info.opinionChange(_subject, opType, StringToNumber<int>(_val));
		else if (_operation == "-")
			info.opinionChange(_subject, opType, -1 * StringToNumber<int>(_val));

		int newOp = 0;
		info.opinionGet(_subject, opType, newOp);

		if (newOp > oldOp)
			info._sound._repInc = true;
		else if (newOp < oldOp)
			info._sound._repDec = true;
	}
}

bool Effect::execute(pyrodactyl::event::Info &info, const Common::String &playerId,
					 Common::Array<EventResult> &result, Common::Array<EventSeqInfo> &endSeq) {
	if (_type < EFF_MOVE) {
		switch (_type) {
		case EFF_VAR:
			if (_operation == "=")
				info.varSet(_subject, _val);
			else if (_operation == "del")
				info.varDel(_subject);
			else if (_operation == "+")
				info.varAdd(_subject, StringToNumber<int>(_val));
			else if (_operation == "-")
				info.varSub(_subject, StringToNumber<int>(_val));
			else if (_operation == "*")
				info.varMul(_subject, StringToNumber<int>(_val));
			else if (_operation == "/")
				info.varDiv(_subject, StringToNumber<int>(_val));
			break;

		case EFF_JOURNAL:
			if (_subject == "finish")
				info._journal.Move(playerId, _operation, true);
			else if (_subject == "start")
				info._journal.Move(playerId, _operation, false);
			else
				info._journal.Add(playerId, _subject, _operation, _val);

			// Update unread status of journal
			info._unread._journal = true;

			// used so we only play one notify sound per event
			info._sound._notify = true;
			break;

		case EFF_OBJ:
			if (_operation == "type")
				info.type(_subject, pyrodactyl::people::stringToPersonType(_val));
			else if (_operation == "state")
				info.state(_subject, pyrodactyl::people::stringToPersonState(_val));
			break;

		case EFF_ITEM:
			if (_operation == "del")
				info._inv.DelItem(_subject, _val);
			else
				info._inv.LoadItem(_subject, _val);

			// Update unread status of inventory
			info._unread._inventory = true;

			// used so we only play one notify sound per event
			info._sound._notify = true;
			break;

		case EFF_LIKE:
			changeOpinion(info, pyrodactyl::people::OPI_LIKE);
			break;

		case EFF_FEAR:
			changeOpinion(info, pyrodactyl::people::OPI_FEAR);
			break;

		case EFF_RESPECT:
			changeOpinion(info, pyrodactyl::people::OPI_RESPECT);
			break;

		case EFF_HEALTH: {
			using namespace pyrodactyl::stat;
			int num = StringToNumber<int>(_val);

			if (_operation == "=")
				info.statSet(_subject, STAT_HEALTH, num);
			else if (_operation == "+")
				info.statChange(_subject, STAT_HEALTH, num);
			else if (_operation == "-")
				info.statChange(_subject, STAT_HEALTH, -1 * num);
		} break;

		case EFF_SOUND:
			if (_subject == "music") {
				if (_operation == "play") {
					MusicKey m = StringToNumber<MusicKey>(_val);
					g_engine->_musicManager->playMusic(m);
				} else if (_operation == "stop")
					g_engine->_musicManager->stop();
				else if (_operation == "pause")
					g_engine->_musicManager->pause();
				else if (_operation == "resume")
					g_engine->_musicManager->resume();
			} else
				g_engine->_musicManager->playEffect(StringToNumber<ChunkKey>(_val), 0);
			break;

		case EFF_MONEY:
			info._moneyVar = _val;
			break;

		case EFF_END:
			if (_operation == "cur")
				endSeq.push_back(true);
			else {
				EventSeqInfo seqinfo;
				seqinfo._loc = _subject;
				seqinfo._val = _val;
				endSeq.push_back(seqinfo);
			}
			break;

		default:
			break;
		}

		return true;
	} else {
		EventResult r;
		r._val = _subject;
		r._x = StringToNumber<int>(_operation);
		r._y = StringToNumber<int>(_val);

		switch (_type) {
		case EFF_MOVE:
			r._type = ER_MOVE;
			break;
		case EFF_MAP:
			r._type = ER_MAP;
			break;
		case EFF_DEST:
			r._type = ER_DEST;
			break;
		case EFF_IMG:
			r._type = ER_IMG;
			info.playerImg(StringToNumber<int>(_val));
			break;
		case EFF_TRAIT:
			r._type = ER_TRAIT;
			info._unread._trait = true;
			info._sound._notify = true;
			break;
		case EFF_LEVEL:
			r._type = ER_LEVEL;
			break;
		case EFF_PLAYER:
			r._type = ER_PLAYER;
			break;
		case EFF_SAVE:
			r._type = ER_SAVE;
			break;
		case EFF_QUIT:
			r._type = ER_QUIT;
			break;
		default:
			break;
		}

		result.push_back(r);
	}
	return false;
}

} // End of namespace Crab
