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
	c._color = colour;
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
	return "AGSCreditz v1.1 by AJA";
}

void AGSCreditz20::AGS_EngineStartup(IAGSEngine *engine) {
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

void AGSCreditz20::SetCredit(const ScriptMethodParams &params) {
	PARAMS7(int, sequence, int, line, string, credit, int, x_pos, int, font, int, color, int, gen_outline);

	assert(sequence >= 0 && sequence <= 10);
	if (line >= (int)_state->_credits[sequence].size())
		_state->_credits[sequence].resize(line + 1);

	Credit &c = _state->_credits[sequence][line];
	c._text = credit;
	c._fontSlot = font;
	c._color = color;
	c._x = x_pos;
	c._isSet = true;
	if (gen_outline > 0)
		c._outline = true;
}


void AGSCreditz20::ScrollCredits(const ScriptMethodParams &params) {
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

string AGSCreditz20::GetCredit(const ScriptMethodParams &params) {
	PARAMS1(int, ID);

	return (_state->_credits[0][ID]._text == IMAGE_TEXT) ?
		"image" : _state->_credits[0][ID]._text.c_str();
}

int AGSCreditz20::IsCreditScrollingFinished(const ScriptMethodParams &params) {
	return true;
}

void AGSCreditz20::SetCreditImage(const ScriptMethodParams &params) {
	//PARAMS5(int, ID, int, Slot, int, center, int, xpos, int, pixtonext);
}

void AGSCreditz20::PauseScroll(const ScriptMethodParams &params) {
	//PARAMS1(int, onoff);
}

void AGSCreditz20::ScrollReset(const ScriptMethodParams &params) {
}

void AGSCreditz20::SetEmptyLineHeight(const ScriptMethodParams &params) {
	//PARAMS1(int, Height);
}

int AGSCreditz20::GetEmptyLineHeight(const ScriptMethodParams &params) {
	return 0;
}

void AGSCreditz20::SetStaticCredit(const ScriptMethodParams &params) {
	//PARAMS8(int, ID, int, x, int, y, int, creditfont, int, creditcolour, int, centered, int, generateoutline, string, credit);

}

string AGSCreditz20::GetStaticCredit(const ScriptMethodParams &params) {
	//PARAMS1(int, ID);
	return nullptr;
}

void AGSCreditz20::StartEndStaticCredits(const ScriptMethodParams &params) {
	//PARAMS2(int, onoff, int, res);
}

int AGSCreditz20::GetCurrentStaticCredit(const ScriptMethodParams &params) {
	return 0;
}

void AGSCreditz20::SetDefaultStaticDelay(const ScriptMethodParams &params) {
	//PARAMS1(int, Cyclesperchar);
}

void AGSCreditz20::SetStaticPause(const ScriptMethodParams &params) {
	//PARAMS2(int, ID, int, length);
}

void AGSCreditz20::SetStaticCreditTitle(const ScriptMethodParams &params) {
	//PARAMS8(int, ID, int, x, int, y, int, titlefont, int, titlecolour, int, centered, int, generateoutline, string, title);
}

void AGSCreditz20::ShowStaticCredit(const ScriptMethodParams &params) {
	//PARAMS6(int, ID, int, time, int, style, int, transtime, int, sound, int, resolution);
}

void AGSCreditz20::StaticReset(const ScriptMethodParams &params) {
}

string AGSCreditz20::GetStaticCreditTitle(const ScriptMethodParams &params) {
	//PARAMS1(int, ID);
	return nullptr;
}

void AGSCreditz20::SetStaticCreditImage(const ScriptMethodParams &params) {
	//int ID, int x, int y, int Slot, int Hcentered, int Vcentered, int time) {
}

int AGSCreditz20::IsStaticCreditsFinished(const ScriptMethodParams &params) {
	return true;
}

} // namespace AGSCreditz
} // namespace Plugins
} // namespace AGS3
