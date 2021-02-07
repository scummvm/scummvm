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

#include "ags/plugins/ags_creditz/ags_creditz.h"

namespace AGS3 {
namespace Plugins {
namespace AGSCreditz {

AGSCreditz::Version AGSCreditz::_version;
State *AGSCreditz::_state;
IAGSEngine *AGSCreditz::_engine;

AGSCreditz::AGSCreditz() {
	_state = new State();
	_engine = nullptr;
}

AGSCreditz::~AGSCreditz() {
	delete _state;
}

/*------------------------------------------------------------------*/

const char *IMAGE_TEXT = "*i*m*a*g*e*";

AGSCreditz11::AGSCreditz11() : AGSCreditz() {
	_version = VERSION_11;

	DLL_METHOD(AGS_GetPluginName);
	DLL_METHOD(AGS_EngineStartup);
}

const char *AGSCreditz11::AGS_GetPluginName() {
	return "AGSCreditz v1.1 by AJA";
}

void AGSCreditz11::AGS_EngineStartup(IAGSEngine *engine) {
	_engine = engine;

	SCRIPT_METHOD(SetCredit);
	SCRIPT_METHOD(ScrollCredits);
	SCRIPT_METHOD(GetCredit);
	SCRIPT_METHOD(IsCreditScrollingFinished);
	SCRIPT_METHOD(SetCreditImage);
	SCRIPT_METHOD(PauseScroll);
	SCRIPT_METHOD(ScrollReset);
	SCRIPT_METHOD(SetEmptyLineHeight);
	SCRIPT_METHOD(GetEmptyLineHeight);
	SCRIPT_METHOD(SetStaticCredit);
	SCRIPT_METHOD(GetStaticCredit);
	SCRIPT_METHOD(StartEndStaticCredits);
	SCRIPT_METHOD(GetCurrentStaticCredit);
	SCRIPT_METHOD(SetDefaultStaticDelay);
	SCRIPT_METHOD(SetStaticPause);
	SCRIPT_METHOD(SetStaticCreditTitle);
	SCRIPT_METHOD(ShowStaticCredit);
	SCRIPT_METHOD(StaticReset);
	SCRIPT_METHOD(GetStaticCreditTitle);
	SCRIPT_METHOD(SetStaticCreditImage);
	SCRIPT_METHOD(IsStaticCreditsFinished);
}

void AGSCreditz11::SetCredit(const ScriptMethodParams &params) {
	PARAMS7(int, ID, string, credit, int, colour, int, font, int, center, int, xpos, int, generateoutline);

	if (ID >= (int)_state->_credits[0].size())
		_state->_credits[0].resize(ID + 1);

	Credit &c = _state->_credits[0][ID];
	c._text = credit;
	c._fontSlot = font;
	c._center = center;
	c._x = xpos;
	c._isSet = true;
	c._outline = generateoutline;
	c._colorHeight = colour;
}

void AGSCreditz11::ScrollCredits(const ScriptMethodParams &params) {
	PARAMS7(int, onoff, int, speed, int, fromY, int, toY, int, isautom, int, wait, int, resolution);

	if (onoff == 1) {
		_state->_creditsRunning = true;
		_state->_seqSettings[0].speed = speed;
		_state->_seqSettings[0].endwait = wait;
		_state->_seqSettings[0].startpoint = fromY;
		_state->_seqSettings[0].endpoint = toY;
		_state->_seqSettings[0].automatic = isautom;

		_engine->GetScreenDimensions(&_state->_screenWidth,
			&_state->_screenHeight, &_state->_screenColorDepth);
		if (_state->_screenWidth == 320) {
			_state->_resolutionFlag = (resolution != 2) ? 1 : 0;
		} else if (_state->_screenWidth == 640) {
			_state->_resolutionFlag = (resolution != 1) ? 1 : 0;
		}

	} else if (onoff == 0) {
		_state->_creditsRunning = false;

	} else {
		_engine->AbortGame("ScrollCredits: OnOff value must be 1 or 0!");
	}
}

string AGSCreditz11::GetCredit(const ScriptMethodParams &params) {
	PARAMS1(int, ID);

	return (_state->_credits[0][ID]._text == IMAGE_TEXT) ?
		"image" : _state->_credits[0][ID]._text.c_str();
}

int AGSCreditz11::IsCreditScrollingFinished(const ScriptMethodParams &params) {
	return true;
}

void AGSCreditz11::SetCreditImage(const ScriptMethodParams &params) {
	//PARAMS5(int, ID, int, Slot, int, center, int, xpos, int, pixtonext);
}

void AGSCreditz11::PauseScroll(const ScriptMethodParams &params) {
	//PARAMS1(int, onoff);
}

void AGSCreditz11::ScrollReset(const ScriptMethodParams &params) {
}

void AGSCreditz11::SetEmptyLineHeight(const ScriptMethodParams &params) {
	//PARAMS1(int, Height);
}

int AGSCreditz11::GetEmptyLineHeight(const ScriptMethodParams &params) {
	return 0;
}

void AGSCreditz11::SetStaticCredit(const ScriptMethodParams &params) {
	//PARAMS8(int, ID, int, x, int, y, int, creditfont, int, creditcolour, int, centered, int, generateoutline, string, credit);

}

string AGSCreditz11::GetStaticCredit(const ScriptMethodParams &params) {
	//PARAMS1(int, ID);
	return nullptr;
}

void AGSCreditz11::StartEndStaticCredits(const ScriptMethodParams &params) {
	//PARAMS2(int, onoff, int, res);
}

int AGSCreditz11::GetCurrentStaticCredit(const ScriptMethodParams &params) {
	return 0;
}

void AGSCreditz11::SetDefaultStaticDelay(const ScriptMethodParams &params) {
	//PARAMS1(int, Cyclesperchar);
}

void AGSCreditz11::SetStaticPause(const ScriptMethodParams &params) {
	//PARAMS2(int, ID, int, length);
}

void AGSCreditz11::SetStaticCreditTitle(const ScriptMethodParams &params) {
	//PARAMS8(int, ID, int, x, int, y, int, titlefont, int, titlecolour, int, centered, int, generateoutline, string, title);
}

void AGSCreditz11::ShowStaticCredit(const ScriptMethodParams &params) {
	//PARAMS6(int, ID, int, time, int, style, int, transtime, int, sound, int, resolution);
}

void AGSCreditz11::StaticReset(const ScriptMethodParams &params) {
}

string AGSCreditz11::GetStaticCreditTitle(const ScriptMethodParams &params) {
	//PARAMS1(int, ID);
	return nullptr;
}

void AGSCreditz11::SetStaticCreditImage(const ScriptMethodParams &params) {
//int ID, int x, int y, int Slot, int Hcentered, int Vcentered, int time) {
}

int AGSCreditz11::IsStaticCreditsFinished(const ScriptMethodParams &params) {
	return true;
}

/*------------------------------------------------------------------*/

AGSCreditz20::AGSCreditz20() : AGSCreditz() {
	_version = VERSION_20;

	DLL_METHOD(AGS_GetPluginName);
	DLL_METHOD(AGS_EngineStartup);
}

const char *AGSCreditz20::AGS_GetPluginName() {
	return "AGSCreditz 2.0 (by Dima Software: AJA)";
}

void AGSCreditz20::AGS_EngineStartup(IAGSEngine *engine) {
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

void AGSCreditz20::RunCreditSequence(const ScriptMethodParams &params) {
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

void AGSCreditz20::SetCredit(const ScriptMethodParams &params) {
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

string AGSCreditz20::GetCredit(const ScriptMethodParams &params) {
	PARAMS2(int, sequence, int, ID);

	return _state->_credits[sequence][ID]._text.c_str();
}

void AGSCreditz20::CreditsSettings(const ScriptMethodParams &params) {
	PARAMS1(int, emptylineheight);

	if (emptylineheight >= 0)
		_state->_emptyLineHeight = emptylineheight;
}

void AGSCreditz20::SequenceSettings(const ScriptMethodParams &params) {
	PARAMS6(int, sequence, int, startpoint, int, endpoint, int, speed, int, automatic, int, endwait);

	_state->_seqSettings[sequence].startpoint = startpoint;
	_state->_seqSettings[sequence].endpoint = endpoint;
	_state->_seqSettings[sequence].speed = speed;
	_state->_seqSettings[sequence].automatic = automatic;
	_state->_seqSettings[sequence].endwait = endwait;
}

int AGSCreditz20::IsSequenceFinished(const ScriptMethodParams &params) {
	PARAMS1(int, sequence);

	if (_state->_seqSettings[sequence].finished) {
		_state->_seqSettings[sequence].finished = false;
		return 1;
	}

	return 0;
}

void AGSCreditz20::PauseScrolling(const ScriptMethodParams &params) {
	if (_state->_creditsRunning) {
		_state->_paused = !_state->_paused;
	}
}

void AGSCreditz20::SetCreditImage(const ScriptMethodParams &params) {
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

void AGSCreditz20::ResetSequence(const ScriptMethodParams &params) {
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

void AGSCreditz20::SetStaticCredit(const ScriptMethodParams &params) {
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

void AGSCreditz20::SetStaticCreditTitle(const ScriptMethodParams &params) {
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

void AGSCreditz20::SetStaticPause(const ScriptMethodParams &params) {
	PARAMS3(int, sequence, int, id, int, length);

	assert(sequence >= 0 && sequence <= 10);
	if (id >= (int)_state->_stCredits[sequence].size())
		_state->_stCredits[sequence].resize(id + 1);

	_state->_stCredits[sequence][id].pause = length;
}

void AGSCreditz20::RunStaticCreditSequence(const ScriptMethodParams &params) {
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

int AGSCreditz20::IsStaticSequenceFinished(const ScriptMethodParams &params) {
	PARAMS1(int, sequence);

	int result = (_state->_stSeqSettings[sequence].finished) ? 1 : 0;
	_state->_stSeqSettings[sequence].finished = false;

	return result;
}

void AGSCreditz20::ShowStaticCredit(const ScriptMethodParams &params) {
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

void AGSCreditz20::SetStaticImage(const ScriptMethodParams &params) {
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

int AGSCreditz20::GetCurrentStaticCredit(const ScriptMethodParams &params) {
	int result = -1;
	if (_state->_creditsRunning && _state->_staticCredits)
		result = _state->_currentStatic;

	return result;
}

void AGSCreditz20::calculateSequenceHeight(int sequence) {
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

int AGSCreditz20::VGACheck(int value) {
	int32 screenX, dum;
	_engine->GetScreenDimensions(&screenX, &dum, &dum);

	if (screenX == 640)
		return value * 2;
	else
		return value;
}

void AGSCreditz20::draw() {
}

} // namespace AGSCreditz
} // namespace Plugins
} // namespace AGS3
