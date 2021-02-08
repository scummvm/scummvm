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
}

const char *AGSCreditz2::AGS_GetPluginName() {
	return "AGSCreditz 2.0 (by Dima Software: AJA)";
}

void AGSCreditz2::AGS_EngineStartup(IAGSEngine *engine) {
	_engine = engine;

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

void AGSCreditz2::RunCreditSequence(const ScriptMethodParams &params) {
	PARAMS1(int, sequence);

	if (!_state->_creditsRunning) {
		_state->_seqSettings[sequence].finished = false;
		_state->_creditsRunning = true;
		_state->_creditSequence = sequence;

		_engine->GetScreenDimensions(&_state->_screenWidth, &_state->_screenHeight,
			&_state->_screenColorDepth);

		if (_state->_seqSettings[sequence].automatic) {
			calculateSequenceHeight(sequence);
			_state->_yPos = _state->_screenHeight + 1;
		} else {
			_state->_yPos = _state->_seqSettings[sequence].startpoint;
		}

		_state->_speedPoint = 0;
		_state->_timer = 0;
		draw();
	} else {
		_state->_paused = false;
		_state->_creditsRunning = false;
		_state->_creditSequence = -1;
		_state->_seqSettings[sequence].finished = true;
	}
}

void AGSCreditz2::SetCredit(const ScriptMethodParams &params) {
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

string AGSCreditz2::GetCredit(const ScriptMethodParams &params) {
	PARAMS2(int, sequence, int, ID);

	return _state->_credits[sequence][ID]._text.c_str();
}

void AGSCreditz2::CreditsSettings(const ScriptMethodParams &params) {
	PARAMS1(int, emptylineheight);

	if (emptylineheight >= 0)
		_state->_emptyLineHeight = emptylineheight;
}

void AGSCreditz2::SequenceSettings(const ScriptMethodParams &params) {
	PARAMS6(int, sequence, int, startpoint, int, endpoint, int, speed, int, automatic, int, endwait);

	_state->_seqSettings[sequence].startpoint = startpoint;
	_state->_seqSettings[sequence].endpoint = endpoint;
	_state->_seqSettings[sequence].speed = speed;
	_state->_seqSettings[sequence].automatic = automatic;
	_state->_seqSettings[sequence].endwait = endwait;
}

int AGSCreditz2::IsSequenceFinished(const ScriptMethodParams &params) {
	PARAMS1(int, sequence);

	if (_state->_seqSettings[sequence].finished) {
		_state->_seqSettings[sequence].finished = false;
		return 1;
	}

	return 0;
}

void AGSCreditz2::PauseScrolling(const ScriptMethodParams &params) {
	if (_state->_creditsRunning) {
		_state->_paused = !_state->_paused;
	}
}

void AGSCreditz2::SetCreditImage(const ScriptMethodParams &params) {
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

void AGSCreditz2::ResetSequence(const ScriptMethodParams &params) {
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

void AGSCreditz2::SetStaticCredit(const ScriptMethodParams &params) {
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

void AGSCreditz2::SetStaticCreditTitle(const ScriptMethodParams &params) {
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

void AGSCreditz2::SetStaticPause(const ScriptMethodParams &params) {
	PARAMS3(int, sequence, int, id, int, length);

	assert(sequence >= 0 && sequence <= 10);
	if (id >= (int)_state->_stCredits[sequence].size())
		_state->_stCredits[sequence].resize(id + 1);

	_state->_stCredits[sequence][id].pause = length;
}

void AGSCreditz2::RunStaticCreditSequence(const ScriptMethodParams &params) {
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

int AGSCreditz2::IsStaticSequenceFinished(const ScriptMethodParams &params) {
	PARAMS1(int, sequence);

	int result = (_state->_stSeqSettings[sequence].finished) ? 1 : 0;
	_state->_stSeqSettings[sequence].finished = false;

	return result;
}

void AGSCreditz2::ShowStaticCredit(const ScriptMethodParams &params) {
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

void AGSCreditz2::SetStaticImage(const ScriptMethodParams &params) {
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

int AGSCreditz2::GetCurrentStaticCredit(const ScriptMethodParams &params) {
	int result = -1;
	if (_state->_creditsRunning && _state->_staticCredits)
		result = _state->_currentStatic;

	return result;
}

void AGSCreditz2::calculateSequenceHeight(int sequence) {
	int32 height, creditHeight, dum;
	height = 0;

	for (uint currentCredit = 0; currentCredit < _state->_credits[sequence].size();
			++currentCredit) {

		if (_state->_credits[sequence][currentCredit]._isSet) {
			if (_state->_credits[sequence][currentCredit]._image) {
				if (_state->_credits[sequence][currentCredit]._colorHeight < 0)
					creditHeight = _engine->GetSpriteHeight(_state->_credits[sequence][currentCredit]._fontSlot);
				else
					creditHeight = _state->_credits[sequence][currentCredit]._colorHeight;
			} else {
				_engine->GetTextExtent(_state->_credits[sequence][currentCredit]._fontSlot,
					_state->_credits[sequence][currentCredit]._text.c_str(),
					&dum, &creditHeight);
			}

			height += creditHeight;
		} else {
			height += VGACheck(_state->_emptyLineHeight);
		}
	}

	_state->_calculatedSequenceHeight = height;
}

int AGSCreditz2::VGACheck(int value) {
	int32 screenX, dum;
	_engine->GetScreenDimensions(&screenX, &dum, &dum);

	if (screenX == 640)
		return value * 2;
	else
		return value;
}

void AGSCreditz2::draw() {
}

} // namespace AGSCreditz
} // namespace Plugins
} // namespace AGS3
