/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"

#include "zvision/zvision.h"
#include "zvision/scripting/script_manager.h"

#include "zvision/scripting/puzzle.h"
#include "zvision/scripting/actions.h"
#include "zvision/scripting/controls/push_toggle_control.h"
#include "zvision/scripting/controls/lever_control.h"
#include "zvision/scripting/controls/slot_control.h"
#include "zvision/scripting/controls/save_control.h"
#include "zvision/scripting/controls/input_control.h"
#include "zvision/scripting/controls/safe_control.h"
#include "zvision/scripting/controls/hotmov_control.h"
#include "zvision/scripting/controls/fist_control.h"
#include "zvision/scripting/controls/paint_control.h"
#include "zvision/scripting/controls/titler_control.h"

#include "common/textconsole.h"
#include "common/file.h"
#include "common/tokenizer.h"

namespace ZVision {

void ScriptManager::parseScrFile(const Common::String &fileName, ScriptScope &scope) {
	Common::File file;
	if (!_engine->getSearchManager()->openFile(file, fileName)) {
		error("Script file not found: %s", fileName.c_str());
	}

	while (!file.eos()) {
		Common::String line = file.readLine();
		if (file.err()) {
			error("Error parsing scr file: %s", fileName.c_str());
		}

		trimCommentsAndWhiteSpace(&line);
		if (line.empty())
			continue;

		if (line.matchString("puzzle:*", true)) {
			Puzzle *puzzle = new Puzzle();
			sscanf(line.c_str(), "puzzle:%u", &(puzzle->key));
			if (getStateFlag(puzzle->key) & Puzzle::ONCE_PER_INST)
				setStateValue(puzzle->key, 0);
			parsePuzzle(puzzle, file);
			scope.puzzles.push_back(puzzle);

		} else if (line.matchString("control:*", true)) {
			Control *ctrl = parseControl(line, file);
			if (ctrl)
				scope.controls.push_back(ctrl);
		}
	}
	scope.procCount = 0;
}

void ScriptManager::parsePuzzle(Puzzle *puzzle, Common::SeekableReadStream &stream) {
	Common::String line = stream.readLine();
	trimCommentsAndWhiteSpace(&line);

	while (!stream.eos() && !line.contains('}')) {
		if (line.matchString("criteria {", true)) {
			parseCriteria(stream, puzzle->criteriaList, puzzle->key);
		} else if (line.matchString("results {", true)) {
			parseResults(stream, puzzle->resultActions);

			// WORKAROUND for a script bug in Zork Nemesis, room ve5e (tuning
			// fork box closeup). If the player leaves the screen while the
			// box is open, puzzle 19398 shows the animation where the box
			// closes, but the box state (state variable 19397) is not updated.
			// We insert the missing assignment for the box state here.
			// Fixes bug #6803.
			if (_engine->getGameId() == GID_NEMESIS && puzzle->key == 19398)
				puzzle->resultActions.push_back(new ActionAssign(_engine, 11, "19397, 0"));

			// WORKAROUND for bug #10604. If the player is looking at the
			// cigar box when Antharia Jack returns to examine the lamp,
			// pp1f_video_flag remains 1. Later, when the player returns
			// to pick up the lantern, the game will try to play the
			// cutscene again, but since that script has already been
			// run the player gets stuck in a dark room instead. We have
			// to add the assignment action to the front, or it won't be
			// reached because changing the location terminates the script.
			//
			// Fixing it this way only keeps the bug from happening. It
			// will not repair old savegames.
			//
			// Note that the bug only affects the DVD version. The CD
			// version doesn't have a separate room for the cutscene.
			else if (_engine->getGameId() == GID_GRANDINQUISITOR && (_engine->getFeatures() & GF_DVD) && puzzle->key == 10836)
				puzzle->resultActions.push_front(new ActionAssign(_engine, 11, "10803, 0"));
		} else if (line.matchString("flags {", true)) {
			setStateFlag(puzzle->key, parseFlags(stream));
		}

		line = stream.readLine();
		trimCommentsAndWhiteSpace(&line);
	}

	puzzle->addedBySetState = false;
}

bool ScriptManager::parseCriteria(Common::SeekableReadStream &stream, Common::List<Common::List<Puzzle::CriteriaEntry> > &criteriaList, uint32 key) const {
	// Loop until we find the closing brace
	Common::String line = stream.readLine();
	trimCommentsAndWhiteSpace(&line);

	// Skip any commented out criteria. If all the criteria are commented out,
	// we might end up with an invalid criteria list (bug #6776).
	while (line.empty()) {
		line = stream.readLine();
		trimCommentsAndWhiteSpace(&line);
	}

	// Criteria can be empty
	if (line.contains('}')) {
		return false;
	}

	// Create a new List to hold the CriteriaEntries
	criteriaList.push_back(Common::List<Puzzle::CriteriaEntry>());

	// WORKAROUND for a script bug in Zork: Nemesis, room td9e (fist puzzle)
	// Since we patch the script that triggers when manipulating the left fist
	// (below), we add an additional check for the left fist sound, so that it
	// doesn't get killed immediately when the left fist animation starts.
	// Together with the workaround below, it fixes bug #6783.
	if (_engine->getGameId() == GID_NEMESIS && key == 3594) {
		Puzzle::CriteriaEntry entry;
		entry.key = 567;
		entry.criteriaOperator = Puzzle::NOT_EQUAL_TO;
		entry.argumentIsAKey = false;
		entry.argument = 1;

		criteriaList.back().push_back(entry);
	}

	// WORKAROUND for a script bug in Zork: Grand Inquisitor, room me2j
	// (Closing the Time Tunnels). When the time tunnel is open the game
	// shows a close-up of only the tunnel, instead of showing the entire
	// booth. However, the scripts that draw the lever in its correct
	// state do not test this flag, causing it to be drawn when it should
	// not be. This fixes bug #6770.
	if (_engine->getGameId() == GID_GRANDINQUISITOR && key == 9536) {
		Puzzle::CriteriaEntry entry;
		entry.key = 9404; // me2j_time_tunnel_open
		entry.criteriaOperator = Puzzle::EQUAL_TO;
		entry.argumentIsAKey = false;
		entry.argument = 0;

		criteriaList.back().push_back(entry);
	}

	while (!stream.eos() && !line.contains('}')) {
		Puzzle::CriteriaEntry entry;

		// Split the string into tokens using ' ' as a delimiter
		Common::StringTokenizer tokenizer(line);
		Common::String token;

		// Parse the id out of the first token
		token = tokenizer.nextToken();
		sscanf(token.c_str(), "[%u]", &(entry.key));

		// WORKAROUND for a script bug in Zork: Nemesis, room td9e (fist puzzle)
		// Check for the state of animation 567 (left fist) when manipulating
		// the fingers of the left fist (puzzle slots 3582, 3583).
		// Together with the workaround above, it fixes bug #6783.
		if (_engine->getGameId() == GID_NEMESIS && (key == 3582 || key == 3583) && entry.key == 568)
			entry.key = 567;

		// Parse the operator out of the second token
		token = tokenizer.nextToken();
		if (token.c_str()[0] == '=')
			entry.criteriaOperator = Puzzle::EQUAL_TO;
		else if (token.c_str()[0] == '!')
			entry.criteriaOperator = Puzzle::NOT_EQUAL_TO;
		else if (token.c_str()[0] == '>')
			entry.criteriaOperator = Puzzle::GREATER_THAN;
		else if (token.c_str()[0] == '<')
			entry.criteriaOperator = Puzzle::LESS_THAN;

		// There are supposed to be three tokens, but there is no
		// guarantee that there will be a space between the second and
		// the third one (bug #6774)
		if (token.size() == 1) {
			token = tokenizer.nextToken();
		} else {
			token.deleteChar(0);
		}

		// First determine if the last token is an id or a value
		// Then parse it into 'argument'
		if (token.contains('[')) {
			sscanf(token.c_str(), "[%u]", &(entry.argument));
			entry.argumentIsAKey = true;
		} else {
			sscanf(token.c_str(), "%u", &(entry.argument));
			entry.argumentIsAKey = false;
		}

		// WORKAROUND for a script bug in Zork: Grand Inquisitor. If the
		// fire timer is killed (e.g. by the inventory screen) with less
		// than 10 units left, it will get stuck and never time out. We
		// work around that by changing the condition from "greater than
		// 10" to "greater than 0 but not 2 (the magic time-out value)".
		//
		// I have a sneaking suspicion that there may be other timer
		// glitches like this, but this one makes the game unplayable
		// and is easy to trigger.
		if (_engine->getGameId() == GID_GRANDINQUISITOR && key == 17162) {
			Puzzle::CriteriaEntry entry0;
			entry0.key = 17161; // pe_fire
			entry0.criteriaOperator = Puzzle::GREATER_THAN;
			entry0.argumentIsAKey = false;
			entry0.argument = 0;

			criteriaList.back().push_back(entry0);

			entry.criteriaOperator = Puzzle::NOT_EQUAL_TO;
			entry.argument = 2;
		}

		criteriaList.back().push_back(entry);

		line = stream.readLine();
		trimCommentsAndWhiteSpace(&line);
	}

	return true;
}

void ScriptManager::parseResults(Common::SeekableReadStream &stream, Common::List<ResultAction *> &actionList) const {
	// Loop until we find the closing brace
	Common::String line = stream.readLine();
	trimCommentsAndWhiteSpace(&line);
	line.toLowercase();

	// TODO: Re-order the if-then statements in order of highest occurrence
	while (!stream.eos() && !line.contains('}')) {
		if (line.empty()) {
			line = stream.readLine();
			trimCommentsAndWhiteSpace(&line);
			line.toLowercase();
			continue;
		}

		const char *chrs = line.c_str();
		uint pos;
		for (pos = 0; pos < line.size(); pos++)
			if (chrs[pos] == ':')
				break;

		if (pos < line.size()) {

			uint startpos = pos + 1;

			for (pos = startpos; pos < line.size(); pos++)
				if (chrs[pos] == ':' || chrs[pos] == '(')
					break;

			if (pos < line.size()) {
				int32 slot = 11;
				Common::String args = "";
				Common::String act(chrs + startpos, chrs + pos);

				startpos = pos + 1;

				if (chrs[pos] == ':') {
					for (pos = startpos; pos < line.size(); pos++)
						if (chrs[pos] == '(')
							break;
					Common::String strSlot(chrs + startpos, chrs + pos);
					slot = atoi(strSlot.c_str());

					startpos = pos + 1;
				}

				if (pos < line.size()) {
					for (pos = startpos; pos < line.size(); pos++)
						if (chrs[pos] == ')')
							break;

					args = Common::String(chrs + startpos, chrs + pos);
				}

				// Parse for the action type
				if (act.matchString("add", true)) {
					actionList.push_back(new ActionAdd(_engine, slot, args));
				} else if (act.matchString("animplay", true)) {
					actionList.push_back(new ActionPlayAnimation(_engine, slot, args));
				} else if (act.matchString("animpreload", true)) {
					actionList.push_back(new ActionPreloadAnimation(_engine, slot, args));
				} else if (act.matchString("animunload", true)) {
					// Only used by ZGI (locations cd6e, cd6k, dg2f, dg4e, dv1j)
					actionList.push_back(new ActionUnloadAnimation(_engine, slot, args));
				} else if (act.matchString("attenuate", true)) {
					actionList.push_back(new ActionAttenuate(_engine, slot, args));
				} else if (act.matchString("assign", true)) {
					actionList.push_back(new ActionAssign(_engine, slot, args));
				} else if (act.matchString("change_location", true)) {
					actionList.push_back(new ActionChangeLocation(_engine, slot, args));
				} else if (act.matchString("crossfade", true)) {
					actionList.push_back(new ActionCrossfade(_engine, slot, args));
				} else if (act.matchString("cursor", true)) {
					actionList.push_back(new ActionCursor(_engine, slot, args));
				} else if (act.matchString("debug", true)) {
					// Not used. Purposely left empty
				} else if (act.matchString("delay_render", true)) {
					actionList.push_back(new ActionDelayRender(_engine, slot, args));
				} else if (act.matchString("disable_control", true)) {
					actionList.push_back(new ActionDisableControl(_engine, slot, args));
				} else if (act.matchString("disable_venus", true)) {
					// Not used. Purposely left empty
				} else if (act.matchString("display_message", true)) {
					actionList.push_back(new ActionDisplayMessage(_engine, slot, args));
				} else if (act.matchString("dissolve", true)) {
					actionList.push_back(new ActionDissolve(_engine));
				} else if (act.matchString("distort", true)) {
					// Only used by Zork: Nemesis for the "treatment" puzzle in the Sanitarium (aj30)
					actionList.push_back(new ActionDistort(_engine, slot, args));
				} else if (act.matchString("enable_control", true)) {
					actionList.push_back(new ActionEnableControl(_engine, slot, args));
				} else if (act.matchString("flush_mouse_events", true)) {
					actionList.push_back(new ActionFlushMouseEvents(_engine, slot));
				} else if (act.matchString("inventory", true)) {
					actionList.push_back(new ActionInventory(_engine, slot, args));
				} else if (act.matchString("kill", true)) {
					// Only used by ZGI
					actionList.push_back(new ActionKill(_engine, slot, args));
				} else if (act.matchString("menu_bar_enable", true)) {
					actionList.push_back(new ActionMenuBarEnable(_engine, slot, args));
				} else if (act.matchString("music", true)) {
					actionList.push_back(new ActionMusic(_engine, slot, args, false));
				} else if (act.matchString("pan_track", true)) {
					actionList.push_back(new ActionPanTrack(_engine, slot, args));
				} else if (act.matchString("playpreload", true)) {
					actionList.push_back(new ActionPlayPreloadAnimation(_engine, slot, args));
				} else if (act.matchString("preferences", true)) {
					actionList.push_back(new ActionPreferences(_engine, slot, args));
				} else if (act.matchString("quit", true)) {
					actionList.push_back(new ActionQuit(_engine, slot));
				} else if (act.matchString("random", true)) {
					actionList.push_back(new ActionRandom(_engine, slot, args));
				} else if (act.matchString("region", true)) {
					// Only used by Zork: Nemesis
					actionList.push_back(new ActionRegion(_engine, slot, args));
				} else if (act.matchString("restore_game", true)) {
					// Only used by ZGI to load the restart game slot, r.svr.
					// Used by the credits screen.
					actionList.push_back(new ActionRestoreGame(_engine, slot, args));
				} else if (act.matchString("rotate_to", true)) {
					actionList.push_back(new ActionRotateTo(_engine, slot, args));
				} else if (act.matchString("save_game", true)) {
					// Not used. Purposely left empty
				} else if (act.matchString("set_partial_screen", true)) {
					actionList.push_back(new ActionSetPartialScreen(_engine, slot, args));
				} else if (act.matchString("set_screen", true)) {
					actionList.push_back(new ActionSetScreen(_engine, slot, args));
				} else if (act.matchString("set_venus", true)) {
					// Not used. Purposely left empty
				} else if (act.matchString("stop", true)) {
					actionList.push_back(new ActionStop(_engine, slot, args));
				} else if (act.matchString("streamvideo", true)) {
					actionList.push_back(new ActionStreamVideo(_engine, slot, args));
				} else if (act.matchString("syncsound", true)) {
					actionList.push_back(new ActionSyncSound(_engine, slot, args));
				} else if (act.matchString("timer", true)) {
					actionList.push_back(new ActionTimer(_engine, slot, args));
				} else if (act.matchString("ttytext", true)) {
					actionList.push_back(new ActionTtyText(_engine, slot, args));
				} else if (act.matchString("universe_music", true)) {
					actionList.push_back(new ActionMusic(_engine, slot, args, true));
				} else if (act.matchString("copy_file", true)) {
					// Not used. Purposely left empty
				} else {
					warning("Unhandled result action type: %s", line.c_str());
				}
			}
		}

		line = stream.readLine();
		trimCommentsAndWhiteSpace(&line);
		line.toLowercase();
	}

	return;
}

uint ScriptManager::parseFlags(Common::SeekableReadStream &stream) const {
	uint flags = 0;

	// Loop until we find the closing brace
	Common::String line = stream.readLine();
	trimCommentsAndWhiteSpace(&line);

	while (!stream.eos() && !line.contains('}')) {
		if (line.matchString("ONCE_PER_INST", true)) {
			flags |= Puzzle::ONCE_PER_INST;
		} else if (line.matchString("DO_ME_NOW", true)) {
			flags |= Puzzle::DO_ME_NOW;
		} else if (line.matchString("DISABLED", true)) {
			flags |= Puzzle::DISABLED;
		}

		line = stream.readLine();
		trimCommentsAndWhiteSpace(&line);
	}

	return flags;
}

Control *ScriptManager::parseControl(Common::String &line, Common::SeekableReadStream &stream) {
	uint32 key;
	char controlTypeBuffer[20];

	sscanf(line.c_str(), "control:%u %s {", &key, controlTypeBuffer);

	Common::String controlType(controlTypeBuffer);

	if (controlType.equalsIgnoreCase("push_toggle")) {
		// WORKAROUND for a script bug in ZGI: There is an invalid hotspot
		// at scene em1h (bottom of tower), which points to a missing
		// script em1n. This is a hotspot at the right of the screen.
		// In the original, this hotspot doesn't lead anywhere anyway,
		// so instead of moving to a missing scene, we just remove the
		// hotspot altogether. The alternative would be to just process
		// and ignore invalid scenes, but I don't think it's worth the
		// effort. Fixes bug #6780.
		if (_engine->getGameId() == GID_GRANDINQUISITOR && key == 5653)
			return NULL;
		return new PushToggleControl(_engine, key, stream);
	} else if (controlType.equalsIgnoreCase("flat")) {
		Control::parseFlatControl(_engine);
		return NULL;
	} else if (controlType.equalsIgnoreCase("pana")) {
		Control::parsePanoramaControl(_engine, stream);
		return NULL;
	} else if (controlType.equalsIgnoreCase("tilt")) {
		// Only used in Zork Nemesis, handles tilt controls (ZGI doesn't have a tilt view)
		Control::parseTiltControl(_engine, stream);
		return NULL;
	} else if (controlType.equalsIgnoreCase("slot")) {
		return new SlotControl(_engine, key, stream);
	} else if (controlType.equalsIgnoreCase("input")) {
		return new InputControl(_engine, key, stream);
	} else if (controlType.equalsIgnoreCase("save")) {
		return new SaveControl(_engine, key, stream);
	} else if (controlType.equalsIgnoreCase("lever")) {
		// Only used in Zork Nemesis, handles draggable levers (te2e, tm7e, tp2e, tt2e, tz2e)
		return new LeverControl(_engine, key, stream);
	} else if (controlType.equalsIgnoreCase("safe")) {
		// Only used in Zork Nemesis, handles the safe in the Asylum (ac4g)
		return new SafeControl(_engine, key, stream);
	} else if (controlType.equalsIgnoreCase("hotmovie")) {
		// Only used in Zork Nemesis, handles movies where the player needs to click on something (mj7g, vw3g)
		return new HotMovControl(_engine, key, stream);
	} else if (controlType.equalsIgnoreCase("fist")) {
		// Only used in Zork Nemesis, handles the door lock puzzle with the skeletal fingers (td9e)
		return new FistControl(_engine, key, stream);
	} else if (controlType.equalsIgnoreCase("paint")) {
		// Only used in Zork Nemesis, handles the painting puzzle screen in Lucien's room in Irondune (ch4g)
		return new PaintControl(_engine, key, stream);
	} else if (controlType.equalsIgnoreCase("titler")) {
		// Only used in Zork Nemesis, handles the death screen with the Restore/Exit buttons (cjde)
		return new TitlerControl(_engine, key, stream);
	}
	return NULL;
}

} // End of namespace ZVision
