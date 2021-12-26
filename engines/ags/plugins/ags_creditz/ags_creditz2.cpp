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

#include "ags/plugins/ags_creditz/ags_creditz2.h"

namespace AGS3 {
namespace Plugins {
namespace AGSCreditz {

AGSCreditz2::AGSCreditz2() : AGSCreditz() {
	_version = VERSION_20;
}

const char *AGSCreditz2::AGS_GetPluginName() {
	return "AGSCreditz 2.0 (by Dima Software: AJA)";
}

void AGSCreditz2::AGS_EngineStartup(IAGSEngine *engine) {
	PluginBase::AGS_EngineStartup(engine);
	_playSound = _engine->GetScriptFunctionAddress("PlaySound");
	engine->RequestEventHook(AGSE_POSTSCREENDRAW);

	SCRIPT_METHOD(RunCreditSequence, AGSCreditz2::RunCreditSequence);
	SCRIPT_METHOD(SetCredit, AGSCreditz2::SetCredit);
	SCRIPT_METHOD(GetCredit, AGSCreditz2::GetCredit);
	SCRIPT_METHOD(CreditsSettings, AGSCreditz2::CreditsSettings);
	SCRIPT_METHOD(SequenceSettings, AGSCreditz2::SequenceSettings);
	SCRIPT_METHOD(IsSequenceFinished, AGSCreditz2::IsSequenceFinished);
	SCRIPT_METHOD(PauseScrolling, AGSCreditz2::PauseScrolling);
	SCRIPT_METHOD(SetCreditImage, AGSCreditz2::SetCreditImage);
	SCRIPT_METHOD(ResetSequence, AGSCreditz2::ResetSequence);

	SCRIPT_METHOD(SetStaticCredit, AGSCreditz2::SetStaticCredit);
	SCRIPT_METHOD(SetStaticCreditTitle, AGSCreditz2::SetStaticCreditTitle);
	SCRIPT_METHOD(SetStaticPause, AGSCreditz2::SetStaticPause);
	SCRIPT_METHOD(RunStaticCreditSequence, AGSCreditz2::RunStaticCreditSequence);
	SCRIPT_METHOD(IsStaticSequenceFinished, AGSCreditz2::IsStaticSequenceFinished);
	SCRIPT_METHOD(ShowStaticCredit, AGSCreditz2::ShowStaticCredit);
	SCRIPT_METHOD(SetStaticImage, AGSCreditz2::SetStaticImage);
	SCRIPT_METHOD(GetCurrentStaticCredit, AGSCreditz2::GetCurrentStaticCredit);
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
	if (line >= (int)_credits[sequence].size())
		_credits[sequence].resize(line + 1);

	Credit &c = _credits[sequence][line];
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

	params._result = _credits[sequence][ID]._text.c_str();
}

void AGSCreditz2::CreditsSettings(ScriptMethodParams &params) {
	PARAMS1(int, emptylineheight);

	if (emptylineheight >= 0)
		_emptyLineHeight = emptylineheight;
}

void AGSCreditz2::SequenceSettings(ScriptMethodParams &params) {
	PARAMS6(int, sequence, int, startpoint, int, endPoint, int, speed, int, automatic, int, endwait);

	_seqSettings[sequence].startpoint = startpoint;
	_seqSettings[sequence].endpoint = endPoint;
	_seqSettings[sequence].speed = speed;
	_seqSettings[sequence].automatic = automatic;
	_seqSettings[sequence].endwait = endwait;
}

void AGSCreditz2::IsSequenceFinished(ScriptMethodParams &params) {
	PARAMS1(int, sequence);

	if (_seqSettings[sequence].finished) {
		_seqSettings[sequence].finished = false;
		params._result = 1;
	} else {
		params._result = 0;
	}
}

void AGSCreditz2::PauseScrolling(ScriptMethodParams &params) {
	if (_creditsRunning) {
		_paused = !_paused;
	}
}

void AGSCreditz2::SetCreditImage(ScriptMethodParams &params) {
	PARAMS5(int, sequence, int, line, int, xPos, int, slot, int, height);

	assert(sequence >= 0 && sequence <= 10);
	if (line >= (int)_credits[sequence].size())
		_credits[sequence].resize(line + 1);

	_credits[sequence][line]._image = true;
	_credits[sequence][line]._isSet = true;
	_credits[sequence][line]._x = xPos;
	_credits[sequence][line]._fontSlot = slot;
	_credits[sequence][line]._colorHeight = height;
}

void AGSCreditz2::ResetSequence(ScriptMethodParams &params) {
	PARAMS1(int, seqtype);

	for (int i = 0; i < 10; ++i) {
		if (seqtype != 2)
			// Scrolling
			_credits[i].clear();
		else
			// Static
			_stCredits[i].clear();
	}
}

void AGSCreditz2::SetStaticCredit(ScriptMethodParams &params) {
	PARAMS8(int, sequence, int, id, string, credit, int, xPos, int, yPos,
	        int, font, int, color, int, genOutline);

	assert(sequence >= 0 && sequence <= 10);
	if (id >= (int)_stCredits[sequence].size())
		_stCredits[sequence].resize(id + 1);

	_stCredits[sequence][id].credit = credit;
	_stCredits[sequence][id].x = xPos;
	_stCredits[sequence][id].y = yPos;
	_stCredits[sequence][id].font = font;
	_stCredits[sequence][id].color = color;

	if (genOutline > 0)
		_stCredits[sequence][id].outline = true;
}

void AGSCreditz2::SetStaticCreditTitle(ScriptMethodParams &params) {
	PARAMS8(int, sequence, int, id, string, title, int, xPos, int, yPos,
	        int, font, int, color, int, genOutline);

	assert(sequence >= 0 && sequence < 10);
	if (id >= (int)_stCredits[sequence].size())
		_stCredits[sequence].resize(id + 1);

	_stCredits[sequence][id].title = title;
	_stCredits[sequence][id].title_x = xPos;
	_stCredits[sequence][id].title_y = yPos;
	_stCredits[sequence][id].title_font = font;
	_stCredits[sequence][id].title_color = color;

	if (genOutline > 0)
		_stCredits[sequence][id].title_outline = true;
}

void AGSCreditz2::SetStaticPause(ScriptMethodParams &params) {
	PARAMS3(int, sequence, int, id, int, length);

	assert(sequence >= 0 && sequence <= 10);
	if (id >= (int)_stCredits[sequence].size())
		_stCredits[sequence].resize(id + 1);

	_stCredits[sequence][id].pause = length;
}

void AGSCreditz2::RunStaticCreditSequence(ScriptMethodParams &params) {
	PARAMS2(int, sequence, int, speed);

	if (!_creditsRunning) {
		_stSeqSettings[sequence].finished = false;
		_stSeqSettings[sequence].speed = speed;
		_creditSequence = sequence;
		_staticCredits = true;
		_creditsRunning = true;
		_currentStatic = 1;
		_timer = 0;
		draw();

	} else {
		_staticCredits = false;
		_creditSequence = -1;
		_stSeqSettings[sequence].finished = false;
		_creditsRunning = false;
		_currentStatic = 0;
		_timer = 0;
	}
}

void AGSCreditz2::IsStaticSequenceFinished(ScriptMethodParams &params) {
	PARAMS1(int, sequence);

	int result = (_stSeqSettings[sequence].finished) ? 1 : 0;
	_stSeqSettings[sequence].finished = false;

	params._result = result;
}

void AGSCreditz2::ShowStaticCredit(ScriptMethodParams &params) {
	PARAMS6(int, sequence, int, id, int, time, int, style,
	        int, styleSettings1, int, styleSettings2);

	_creditSequence = sequence;
	_creditsRunning = true;
	_staticCredits = true;
	_singleStatic.id = id;
	_singleStatic.time = time;
	_singleStatic.style = style;
	_singleStatic.settings1 = styleSettings1;
	_singleStatic.settings2 = styleSettings2;
	_singleStatic.bool_ = true;
	_stSeqSettings[sequence].finished = false;
	_timer = 0;
	_timer2 = 0;
	_numChars = 0;
	draw();
}

void AGSCreditz2::SetStaticImage(ScriptMethodParams &params) {
	PARAMS6(int, sequence, int, id, int, slot, int, xPos, int, yPos, int, length);

	assert(sequence >= 0 && sequence < 10);
	if (id >= (int)_stCredits[sequence].size())
		_stCredits[sequence].resize(id + 1);

	_stCredits[sequence][id].image = true;
	_stCredits[sequence][id].image_slot = slot;
	_stCredits[sequence][id].x = xPos;
	_stCredits[sequence][id].y = yPos;
	_stCredits[sequence][id].image_time = length;
}

void AGSCreditz2::GetCurrentStaticCredit(ScriptMethodParams &params) {
	int result = -1;
	if (_creditsRunning && _staticCredits)
		result = _currentStatic;

	params._result = result;
}

} // namespace AGSCreditz
} // namespace Plugins
} // namespace AGS3
