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

#include "ags/plugins/ags_creditz/ags_creditz2.h"

namespace AGS3 {
namespace Plugins {
namespace AGSCreditz {

AGSCreditz2::AGSCreditz2() : AGSCreditz() {
	_version = VERSION_20;

	DLL_METHOD(AGS_GetPluginName);
	DLL_METHOD(AGS_EngineStartup);
	DLL_METHOD(AGS_EngineOnEvent);
}

const char *AGSCreditz2::AGS_GetPluginName() {
	return "AGSCreditz 2.0 (by Dima Software: AJA)";
}

void AGSCreditz2::AGS_EngineStartup(IAGSEngine *engine) {
	_engine = engine;
	_playSound = (IntFunction)_engine->GetScriptFunctionAddress("PlaySound");
	engine->RequestEventHook(AGSE_POSTSCREENDRAW);

	SCRIPT_METHOD(RunCreditSequence);
	SCRIPT_METHOD(SetCredit);
	SCRIPT_METHOD(GetCredit);
	SCRIPT_METHOD(CreditsSettings);
	SCRIPT_METHOD(SequenceSettings);
	SCRIPT_METHOD(IsSequenceFinished);
	SCRIPT_METHOD(PauseScrolling);
	SCRIPT_METHOD(SetCreditImage);
	SCRIPT_METHOD(ResetSequence);

	SCRIPT_METHOD(SetStaticCredit);
	SCRIPT_METHOD(SetStaticCreditTitle);
	SCRIPT_METHOD(SetStaticPause);
	SCRIPT_METHOD(RunStaticCreditSequence);
	SCRIPT_METHOD(IsStaticSequenceFinished);
	SCRIPT_METHOD(ShowStaticCredit);
	SCRIPT_METHOD(SetStaticImage);
	SCRIPT_METHOD(GetCurrentStaticCredit);
}

int64 AGSCreditz2::AGS_EngineOnEvent(int event, NumberPtr data) {
	if (event & AGSE_POSTSCREENDRAW)
		draw();

	return 0;
}

void AGSCreditz2::RunCreditSequence(ScriptMethodParams &params) {
	PARAMS1(int, sequence);

	startSequence(sequence);
}

void AGSCreditz2::SetCredit(ScriptMethodParams &params) {
	PARAMS7(int, sequence, int, line, string, credit, int, x_pos, int, font, int, color, int, gen_outline);

	assert(sequence >= 0 && sequence <= 10);
	if (line >= (int)_state->_credits[sequence].size())
		_state->_credits[sequence].resize(line + 1);

	Credit &c = _state->_credits[sequence][line];
	c._text = credit;
	c._fontSlot = font;
	c._colorHeight = color;
	c._x = x_pos;
	c._isSet = true;
	if (gen_outline > 0)
		c._outline = true;
}

void AGSCreditz2::GetCredit(ScriptMethodParams &params) {
	PARAMS2(int, sequence, int, ID);

	params._result = _state->_credits[sequence][ID]._text.c_str();
}

void AGSCreditz2::CreditsSettings(ScriptMethodParams &params) {
	PARAMS1(int, emptylineheight);

	if (emptylineheight >= 0)
		_state->_emptyLineHeight = emptylineheight;
}

void AGSCreditz2::SequenceSettings(ScriptMethodParams &params) {
	PARAMS6(int, sequence, int, startpoint, int, endPoint, int, speed, int, automatic, int, endwait);

	_state->_seqSettings[sequence].startpoint = startpoint;
	_state->_seqSettings[sequence].endpoint = endPoint;
	_state->_seqSettings[sequence].speed = speed;
	_state->_seqSettings[sequence].automatic = automatic;
	_state->_seqSettings[sequence].endwait = endwait;
}

void AGSCreditz2::IsSequenceFinished(ScriptMethodParams &params) {
	PARAMS1(int, sequence);

	if (_state->_seqSettings[sequence].finished) {
		_state->_seqSettings[sequence].finished = false;
		params._result = 1;
	} else {
		params._result = 0;
	}
}

void AGSCreditz2::PauseScrolling(ScriptMethodParams &params) {
	if (_state->_creditsRunning) {
		_state->_paused = !_state->_paused;
	}
}

void AGSCreditz2::SetCreditImage(ScriptMethodParams &params) {
	PARAMS5(int, sequence, int, line, int, xPos, int, slot, int, height);

	assert(sequence >= 0 && sequence <= 10);
	if (line >= (int)_state->_credits[sequence].size())
		_state->_credits[sequence].resize(line + 1);

	_state->_credits[sequence][line]._image = true;
	_state->_credits[sequence][line]._isSet = true;
	_state->_credits[sequence][line]._x = xPos;
	_state->_credits[sequence][line]._fontSlot = slot;
	_state->_credits[sequence][line]._colorHeight = height;
}

void AGSCreditz2::ResetSequence(ScriptMethodParams &params) {
	PARAMS1(int, seqtype);

	for (int i = 0; i < 10; ++i) {
		if (seqtype != 2)
			// Scrolling
			_state->_credits[i].clear();
		else
			// Static
			_state->_stCredits[i].clear();
	}
}

void AGSCreditz2::SetStaticCredit(ScriptMethodParams &params) {
	PARAMS8(int, sequence, int, id, string, credit, int, xPos, int, yPos,
		int, font, int, color, int, genOutline);

	assert(sequence >= 0 && sequence <= 10);
	if (id >= (int)_state->_stCredits[sequence].size())
		_state->_stCredits[sequence].resize(id + 1);

	_state->_stCredits[sequence][id].credit = credit;
	_state->_stCredits[sequence][id].x = xPos;
	_state->_stCredits[sequence][id].y = yPos;
	_state->_stCredits[sequence][id].font = font;
	_state->_stCredits[sequence][id].color = color;

	if (genOutline > 0)
		_state->_stCredits[sequence][id].outline = true;
}

void AGSCreditz2::SetStaticCreditTitle(ScriptMethodParams &params) {
	PARAMS8(int, sequence, int, id, string, title, int, xPos, int, yPos,
		int, font, int, color, int, genOutline);

	assert(sequence >= 0 && sequence < 10);
	if (id >= (int)_state->_stCredits[sequence].size())
		_state->_stCredits[sequence].resize(id + 1);

	_state->_stCredits[sequence][id].title = title;
	_state->_stCredits[sequence][id].title_x = xPos;
	_state->_stCredits[sequence][id].title_y = yPos;
	_state->_stCredits[sequence][id].title_font = font;
	_state->_stCredits[sequence][id].title_color = color;

	if (genOutline > 0)
		_state->_stCredits[sequence][id].title_outline = true;
}

void AGSCreditz2::SetStaticPause(ScriptMethodParams &params) {
	PARAMS3(int, sequence, int, id, int, length);

	assert(sequence >= 0 && sequence <= 10);
	if (id >= (int)_state->_stCredits[sequence].size())
		_state->_stCredits[sequence].resize(id + 1);

	_state->_stCredits[sequence][id].pause = length;
}

void AGSCreditz2::RunStaticCreditSequence(ScriptMethodParams &params) {
	PARAMS2(int, sequence, int, speed);

	if (!_state->_creditsRunning) {
		_state->_stSeqSettings[sequence].finished = false;
		_state->_stSeqSettings[sequence].speed = speed;
		_state->_creditSequence = sequence;
		_state->_staticCredits = true;
		_state->_creditsRunning = true;
		_state->_currentStatic = 1;
		_state->_timer = 0;
		draw();

	} else {
		_state->_staticCredits = false;
		_state->_creditSequence = -1;
		_state->_stSeqSettings[sequence].finished = false;
		_state->_creditsRunning = false;
		_state->_currentStatic = 0;
		_state->_timer = 0;
	}
}

void AGSCreditz2::IsStaticSequenceFinished(ScriptMethodParams &params) {
	PARAMS1(int, sequence);

	int result = (_state->_stSeqSettings[sequence].finished) ? 1 : 0;
	_state->_stSeqSettings[sequence].finished = false;

	params._result = result;
}

void AGSCreditz2::ShowStaticCredit(ScriptMethodParams &params) {
	PARAMS6(int, sequence, int, id, int, time, int, style,
		int, styleSettings1, int, styleSettings2);

	_state->_creditSequence = sequence;
	_state->_creditsRunning = true;
	_state->_staticCredits = true;
	_state->_singleStatic.id = id;
	_state->_singleStatic.time = time;
	_state->_singleStatic.style = style;
	_state->_singleStatic.settings1 = styleSettings1;
	_state->_singleStatic.settings2 = styleSettings2;
	_state->_singleStatic.bool_ = true;
	_state->_stSeqSettings[sequence].finished = false;
	_state->_timer = 0;
	_state->_timer2 = 0;
	_state->_numChars = 0;
	draw();
}

void AGSCreditz2::SetStaticImage(ScriptMethodParams &params) {
	PARAMS6(int, sequence, int, id, int, slot, int, xPos, int, yPos, int, length);

	assert(sequence >= 0 && sequence < 10);
	if (id >= (int)_state->_stCredits[sequence].size())
		_state->_stCredits[sequence].resize(id + 1);

	_state->_stCredits[sequence][id].image = true;
	_state->_stCredits[sequence][id].image_slot = slot;
	_state->_stCredits[sequence][id].x = xPos;
	_state->_stCredits[sequence][id].y = yPos;
	_state->_stCredits[sequence][id].image_time = length;
}

void AGSCreditz2::GetCurrentStaticCredit(ScriptMethodParams &params) {
	int result = -1;
	if (_state->_creditsRunning && _state->_staticCredits)
		result = _state->_currentStatic;

	params._result = result;
}

} // namespace AGSCreditz
} // namespace Plugins
} // namespace AGS3
