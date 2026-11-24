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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "common/endian.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "dune/dune.h"
#include "dune/event_scripts.h"

namespace Dune {

bool DuneEngine::isStageEventMenu() const {
	return _menuKind == kMenuPalaceGathering && !_menuParts.empty() &&
		_menuParts[0] == kStageEventContinuation;
}

void DuneEngine::showStageEventMenu(bool withWhat, bool armTransition) {
	Common::Array<Common::String> choices;
	choices.push_back(command(161));
	if (withWhat)
		choices.push_back(command(148));
	WRITE_LE_UINT16(_state.begin() + 0x2220, withWhat ? 0x1fae : 0x1fba);
	_menuParts.clear();
	_menuParts.resize(choices.size(), kStageEventContinuation);
	_menuKind = kMenuPalaceGathering;
	showMenu(choices, nullptr, false, armTransition);
	drawMenu();
	finishPanelMenuTransition();
	_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0, kScreenWidth, kScreenHeight);
	_system->updateScreen();
}

/** 9761/9F40/9F9E: a caller's event part is searched, not a fixed PHRASE. */
bool DuneEngine::prepareStageEventParagraph(uint who) {
	WRITE_LE_UINT16(_state.begin() + 0x47c4, who);
	if (who == 14) {
		const uint16 troop = READ_LE_UINT16(_state.begin() + 0x4756);
		if (troop >= 0x8aa && (troop - 0x8aa) % 27 == 0)
			prepareTroopContactContext((troop - 0x8aa) / 27);
	}
	preparePersonMusterContext(who);
	WRITE_LE_UINT16(_state.begin() + 0x47a2, 0xfd8 + 16 * who);
	preparePersonDialogueContext(who);
	_state[0x47c2] = 0x80;
	const uint part = who * 8 + 7;
	if (part >= _dialogue.count())
		return false;
	Common::Array<Dialogue::Record> &records = _dialogue.entry(part);
	for (uint i = 0; i < records.size(); i++) {
		Dialogue::Record &record = records[i];
		if (!dialogueRecordAvailable(record, 0x80) || !conditionHolds(record.condition))
			continue;
		_dialoguePart = part;
		_dialogueLine = i;
		_dialogueMask = 0x80;
		prepareDialogueLine(record, who);
		runLineAction(record);
		_state[0x19] = 0xff;
		rememberBookRecord(record, who);
		record.flags |= 0x80;
		WRITE_LE_UINT16(_state.begin() + 0x47ba, _state[0x47a8] ? 0xffff : 0);
		_state[0x47a8] = 0;
		if (_state[0x46eb] && _orderingTroop < _map.troops().size())
			drawTroopStatus(_orderingTroop);
		else {
			_speakerPerson = who;
			_speakerFrame = 0;
			prepareLocalSpeakerParagraph();
			drawScene(false);
		}
		playSpeech(record.line, who);
		showStageEventMenu(true);
		return true;
	}
	// 9F9C returns without A042/88AF: retain the old text, recording,
	// active menu, read marks and portrait. Only the raw caller changed.
	return false;
}

/** 1707: execute until a paragraph, explicit pause, or terminal FF. */
void DuneEngine::continueStageEvent() {
	_state[0xce9d] = 0;
	for (uint limit = 0; limit < 32; limit++) {
		const uint16 cursor = READ_LE_UINT16(_state.begin() + 0x477a);
		StageEventInstruction op;
		if (!stageEventInstruction(_dosCode.begin(), cursor, op)) {
			warning("Dune: invalid stage-event instruction %04x", cursor);
			return;
		}
		if (op.opcode == 255) {
			// 1736 reads FF without advancing477A. Neither this nor00
			// changes Paul's room mirrors, followers, or physical place.
			const uint16 saved = READ_LE_UINT16(_state.begin() + 0x4776);
			_state[4] = (byte)saved;
			_state[0x46e0] = saved >> 8;
			releaseEventScript();
			if ((int8)_state[0xfb] < 0 && _orderingTroop < _map.troops().size()) {
				_troopInfoPage = 0;
				// 68EB/780A restores commands over the retained tableau;
				// it does not repaint the map or replay the old report.
				showTroopMenu(false, true);
				drawMenu();
				finishPanelMenuTransition();
				_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0, kScreenWidth, kScreenHeight);
				_system->updateScreen();
			} else {
				releaseEventSpeaker();
				const byte *layout = roomRecord(_state[5], _state[4]);
				if (layout)
					_place = layout[0] - 1;
				drawScene();
				showRoomMenu();
				raisePanelFace();
			}
			return;
		}
		WRITE_LE_UINT16(_state.begin() + 0x477a, op.next);
		switch (op.opcode) {
		case 0:
			_state[4] = op.argument;
			WRITE_LE_UINT16(_state.begin() + 0x4778, op.roster);
			// Fall through:13C8 ends at13DB's real room redraw.
			// Do not substitute palace geometry for a sietch caller.
			// fall through
		case 4: {
			releaseEventSpeaker();
			_speech.clear(); // A room frame has no paragraph, even without a local speaker.
			const byte *layout = roomRecord(_state[5], _state[4]);
			if (layout)
				_place = layout[0] - 1;
			showStageEventMenu(false);
			drawScene();
			break;
		}
		case 2:
			// 1399 changes the QUEUED table. If9761 finds no paragraph,
			// the previously displayed single-Continue menu stays intact.
			WRITE_LE_UINT16(_state.begin() + 0x2220, 0x1fae);
			WRITE_LE_UINT16(_state.begin() + 0x479e, 0); // 8C8A.
			prepareStageEventParagraph(op.argument);
			return;
		case 6:
			showStageEventMenu(false);
			WRITE_LE_UINT16(_state.begin() + 0x47c4, op.argument);
			_speakerPerson = op.argument;
			prepareLocalSpeakerIdle();
			_speakerIdleActive = false; // 9B8B freezes the prepared portrait.
			drawSpeakerFigure(op.argument, 0);
			_speakerPerson = -1;
			break;
		case 8:
			return;
		case 14: {
			const uint16 line = prepareProspectorBriefingLine();
			_hasSpiceDensityMap = true;
			_troopInfoPage = 1;
			openTroopSurvey();
			playSpeech(line, 15);
			showStageEventMenu(true);
			return;
		}
		case 16: {
			closeSurveyWindow();
			const uint16 line = prepareProspectorBriefingLine();
			_troopInfoPage = 2;
			drawTroopStatus(_orderingTroop);
			playSpeech(line, 15);
			showStageEventMenu(true);
			return;
		}
		default:
			break;
		}
	}
}

} // namespace Dune
